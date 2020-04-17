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
	int threadID; //0x24
	int scriptHash; //0x28
	eThreadState state; //0x2C
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
};

class ScriptThread
{
public:
	scrThreadContext m_context;
	void* stack;
	char* exitMessage;
	char _0x58[0x2C];
	char scriptName[24];
	char _0x9C[8];
	virtual void DoRun() = 0;
	virtual eThreadState Reset(int scriptHash, void* pArgs, int argCount);
	virtual eThreadState Run(int opsToExecute);
	virtual const char* ChangeScriptName();
	virtual const char* CallChangeScriptName();
	virtual eThreadState Tick(int opsToExecute);
	virtual void Kill();
	virtual void loc_823F84F0(int r4, int r5, int r6);
	virtual eThreadState loc_823F7B60();
	virtual eThreadState nullsub();
	virtual ScriptThread* GetMainScriptThread();
};