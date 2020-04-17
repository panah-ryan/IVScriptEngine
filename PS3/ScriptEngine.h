#pragma once

enum eThreadState
{
	ThreadStateIdle,
	ThreadStateRunning,
	ThreadStateKilled,
	ThreadState3,
	ThreadState4
};

struct scrThreadContext
{
	int threadID;
	int scriptHash;
	eThreadState state;
	int PC; //0x10
	int currentFuncSP; //0x14
	int SP; //0x18
	int TimerA; //0x1C
	int TimerB; //0x20
	int TimerC; //0x24
	float WaitTime; //0x28
	int _0x2C;
	int _0x30;
	int _0x34;
	int _0x38;
	int _0x3C;
	int stackSize;
	int _0x44;
	int _0x48;
	int _0x4C;
};

struct ScriptThread
{
	int vftable;
	scrThreadContext m_context;
	int* stack;
	int _0x54;
	char* exitMessage;
	char scriptName[24];
	char _0x74[0xD];
	bool networkSafe; //0x81
	bool shouldBeSaved; //0x82
	char _0x83[5];
	bool scriptPaused; //0x88
	bool _0x89;
	char _0x8A[0x26];
};

struct pgPtrCollection
{
	ScriptThread** m_pData;
	UINT16 m_pCount;
	UINT16 m_pSize;
};

int scrThread_vftable[6];
int scrThread_run[2];

static pgPtrCollection* scrThreadCollection;

DWORD wakeAt;

bool InitalizeScriptEngine()
{
#ifdef _BLES
	scrThreadCollection = (pgPtrCollection*)0x17D1428;
#else
	scrThreadCollection = (pgPtrCollection*)0x17D12A8;
#endif
	printf("Found Thread Collection %i Available Threads!\n", scrThreadCollection->m_pCount);
	return true;
}

pgPtrCollection* GetThreadCollection()
{
	return scrThreadCollection;
}

void SetActiveThread(ScriptThread* thread)
{
#ifdef _BLES
	*(int*)0x17D141C = (int)thread;
#else
	*(int*)0x17D129C = (int)thread;
#endif
}

ScriptThread* GetActiveThread()
{
#ifdef _BLES
	return (ScriptThread*)*(int*)0x017D141C;
#else
	return (ScriptThread*)*(int*)0x17D129C;
#endif
}

eThreadState ScriptThread_Reset(ScriptThread* thread, int scriptHash, void* pArgs, int argCount)
{
	thread->m_context.threadID = scriptHash;
	thread->m_context.state = ThreadStateIdle;
	return thread->m_context.state;
}

DWORD GetTickCount()
{
#ifdef _BLES
	return *(DWORD*)0x1576824;
#else
	return *(DWORD*)0x15766E4;
#endif
}

opd_s MainScriptThread_t = { 0, 0 };
void(*MainThreadScript)() = (void(*)())&MainScriptThread_t;
char* ScriptName;

eThreadState ScriptThread_Run(ScriptThread* thread, int opsToExecute)
{
	ScriptThread* activeThread = GetActiveThread();
	SetActiveThread(thread);
	if (thread->m_context.state != ThreadStateKilled)
	{
		if (MainScriptThread_t.sub != 0)
			if (GetTickCount() >= wakeAt)
				MainThreadScript();
	}
	SetActiveThread(activeThread);
	return thread->m_context.state;
}

extern "C" void SetupSPRXScript(char* scriptName, int funcAddr, int _TOC)
{
	MainScriptThread_t.sub = funcAddr;
	MainScriptThread_t.toc = _TOC;
	ScriptName = scriptName;
	printf("Loaded SPRX Script %s\n", scriptName);
}

extern "C" void ScriptThread_Yield(DWORD time)
{
	wakeAt = GetTickCount() + time;
}

extern "C" void UnloadSPRXScript()
{
	MainScriptThread_t.sub = 0;
	MainScriptThread_t.toc = 0;
	printf("Unloaded SPRX Script %s!\n", ScriptName);
}

void ScriptEngine_CreateThread(ScriptThread* thread)
{
	pgPtrCollection* collection = GetThreadCollection();
	thread->exitMessage = "Normal exit.";
	thread->m_context.state = ThreadStateIdle;
	int slot = collection->m_pCount - 1;
	thread->m_context.threadID = slot;
	write_process(scrThread_vftable, (void*)0xEDF670, 0x18);
	scrThread_run[0] = *(int*)ScriptThread_Run;
	scrThread_run[1] = TOC;
	scrThread_vftable[3] = (int)scrThread_run;
	thread->vftable = (int)&scrThread_vftable;
	collection->m_pData[slot] = thread;
	ScriptThread_Reset(thread, slot, 0, 0);
	printf("Created Thread, id %i\n", thread->m_context.threadID);
}

struct Native_s
{
	int hash;
	int func;
};

struct NativeTable
{
	Native_s* pTable;
	int size;
};

extern "C" int GetNativeFunction(int hash)
{
#ifdef _BLES
	NativeTable table = *(NativeTable*)0x17D1438;
#else
	NativeTable table = *(NativeTable*)0x17D12B8;
#endif
	for (int i = 0; i < table.size; i++)
	{
		if (table.pTable[i].hash == hash)
			return table.pTable[i].func;
	}
	return 0;
}

extern "C" uint32_t jenkinsHash(char *key)
{
	size_t len = strlen(key);
	unsigned int hash, i;
	for (hash = i = 0; i < len; ++i)
	{
		hash += tolower(key[i]);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}
