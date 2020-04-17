#pragma once

enum eThreadState
{
	ThreadStateIdle,
	ThreadStateRunning,
	ThreadStateKilled,
	ThreadState3,
	ThreadState4
};

extern "C" unsigned int jenkinsHash(char *key);

class scrThreadContext
{
public:
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

class ScriptThread
{
public:
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
	virtual void DoRun() = 0;
	virtual eThreadState Reset(int scriptHash, void* pArgs, int argCount);
	virtual eThreadState Run(int opsToExecute);
	virtual eThreadState Tick(int opsToExecute);
	virtual void Kill();
	virtual ScriptThread* GetMainScriptThread();
};
