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
	int IP; //0x10
	int FrameSP; //0x14
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
	int _0x50;
};

class ScriptThread
{
public:
	scrThreadContext m_context;
	void* stack;
	char _0x58[0xC];
	char* exitMessage;
	char scriptName[20];
	char _0x7C[0x114];
	virtual void DoRun() = 0;
	virtual eThreadState Reset(int scriptHash, void* pArgs, int argCount);
	virtual eThreadState Run(int opsToExecute);
	virtual eThreadState Tick(int opsToExecute);
	virtual void Kill();
	virtual ScriptThread* GetMainScriptThread();
};