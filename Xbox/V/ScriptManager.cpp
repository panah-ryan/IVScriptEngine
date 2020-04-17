#include "ScriptManager.h"

ScriptManagerThread g_ScriptManagerThread;

static LPVOID mainFiber;
static Script* currentScript;

extern "C" int GetNativeFunction(int hash)
{
	int dwCount;
	int dwPoolHash;
	int dwPtr;

	dwPtr = *(int*)(0x83DDCD08 + ((hash << 2) & 0x3FC));

	if (dwPtr == 0)
		return 0;

GetCount:
	dwCount = *(int*)(dwPtr + 0x20);

	if (dwCount)
	{
		dwPoolHash = dwPtr + 0x24;
		for (int i = 0; i < dwCount; i++, dwPoolHash += 4)
		{
			if (*(int*)dwPoolHash == hash)
			{
				int dwRes = *(int*)(dwPtr + ((i + 1) << 2));
				if (dwRes)
					return dwRes;
				return 0;
			}
		}
	}
	dwPtr = *(int*)dwPtr;
	goto GetCount;
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