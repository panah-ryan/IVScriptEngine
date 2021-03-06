#include "ScriptManager.h"

ScriptManagerThread g_ScriptManagerThread;

static LPVOID mainFiber;
static Script* currentScript;

typedef struct Native_s
{
	int hash;
	int func;
};

typedef struct NativeTable
{
	Native_s* pTable;
	int size;
};

extern "C" int GetNativeFunction(int hash)
{
	NativeTable table = *(NativeTable*)0x828DA3E0;
	for (int i = 0; i < table.size; i++)
	{
		if (table.pTable[i].hash == hash)
			return table.pTable[i].func;
	}
	return 0;
}

void Script::Tick()
{
	if (mainFiber == nullptr)
		mainFiber = ConvertThreadToFiber(nullptr);
	if (GetTickCount() < this->wakeAt)
		return;
	if (scriptFiber)
	{
		currentScript = this;
		SwitchToFiber(scriptFiber);
		currentScript = nullptr;
	}
	else
		scriptFiber = CreateFiber(NULL, (LPFIBER_START_ROUTINE)this->callbackFunction, this);
}

void Script::Run()
{
	callbackFunction();
}

void Script::Yield(DWORD time)
{
	wakeAt = GetTickCount() + time;
	SwitchToFiber(mainFiber);
}

void ScriptManagerThread::DoRun()
{
	std::map<const char*, std::shared_ptr<Script>>::iterator it = m_scripts.begin();
	while (it != m_scripts.end())
	{
		it->second->Tick();
		it++;
	}
}

void ScriptManagerThread::LoadScript(const char* szName, void(*fn)())
{
	if (m_scripts.find(szName) != m_scripts.end())
	{
		printf("Script %s is alreaady loaded!\n");
		return;
	}
	m_scripts[szName] = std::make_shared<Script>(fn);
	printf("Loaded Script %s\n", szName);
}

void ScriptManagerThread::UnloadScript(const char* szName)
{
	if (m_scripts.find(szName) == m_scripts.end())
	{
		printf("Couldn't find script %s\n", szName);
		return;
	}
	m_scripts.erase(m_scripts.find(szName));
	printf("Unloaded Script %s\n", szName);
}

extern "C" void scriptWait(DWORD waitTime)
{
	currentScript->Yield(waitTime);
}

extern "C" void scriptRegister(const char* szName, void(*function)())
{
	g_ScriptManagerThread.LoadScript(szName, function);
}

extern "C" void scriptUnregister(const char* szName)
{
	g_ScriptManagerThread.UnloadScript(szName);
}