#pragma once
#include "stdafx.h"
#include "ScriptEngine.h"

#undef Yield

class Script
{
public:
	inline Script(void(*function)())
	{
		scriptFiber = nullptr;
		callbackFunction = function;
		wakeAt = GetTickCount();
	}

	inline ~Script()
	{
		if (scriptFiber)
			DeleteFiber(scriptFiber);
	}

	void Tick();

	void Yield(DWORD time);

	inline void(*GetCallbackFunction())()
	{
		return callbackFunction;
	}

	LPVOID scriptFiber;
	DWORD wakeAt;
	void(*callbackFunction)();
	void Run();
};

typedef std::map<const char*, std::shared_ptr<Script>> scriptMap;

class ScriptManagerThread : public ScriptThread
{
public:
	scriptMap m_scripts;
	virtual void DoRun() override;
	void LoadScript(const char* szName, void(*fn)());
	void UnloadScript(const char* szName);
};

extern ScriptManagerThread g_ScriptManagerThread;
