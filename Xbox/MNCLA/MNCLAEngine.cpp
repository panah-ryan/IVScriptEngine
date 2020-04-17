// CoDEngine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Detour.h"
#include "ScriptEngine.h"
#include "ScriptManager.h"
#include "SCO.h"

HANDLE hModule;
DWORD dwTitleID;
BOOL fUnloadXEX, fGoodToUnload;
int ourScriptThreadId;

Detour<void> *WAITDetour;
Detour<void> *RunTickLoopDetour;
Detour<void*> *AESDetour;
Detour<void> *NativeDetour;
Detour<void>* StartScriptDetour;
Detour<void>* AddNativeDetour;

void LaunchDash()
{
	XLaunchNewImage(XLAUNCH_KEYWORD_DASH, 0);
}

int ResolveFunction(CHAR* ModuleName, DWORD Ordinal) {

	// Get our module handle
	HMODULE mHandle = GetModuleHandle(ModuleName);
	if (mHandle == NULL)
		return NULL;

	// Return our address
	return (int)GetProcAddress(mHandle, (LPCSTR)Ordinal);
}

int XNotifyThread(wchar_t *Message)
{
	Sleep(600);
	((void(*)(int, int, unsigned long long, PWCHAR, PVOID))ResolveFunction("xam.xex", 656))(34, 0xFF, 2, Message, 0);
	return 0;
}

void XNotify(wchar_t *Message)
{
	ExCreateThread(0, 0, 0, 0, (LPTHREAD_START_ROUTINE)XNotifyThread, Message, 0);
}

bool initalized = false;

void WAITHook(void* r3)
{
	if (!initalized)
	{
		ScriptEngine::Initalize();
		ScriptEngine::CreateThread(&g_ScriptManagerThread);
		initalized = true;
	}
	WAITDetour->CallOriginal(r3);
}

void RunTickLoop(int r3, int timer)
{
	int threadid = *(int*)(r3 + 0x10);
	if (threadid == 2)
	{
		ScriptThread* thread = (ScriptThread*)((int(*)(int))0x82554D48)(ourScriptThreadId);
		if (thread != 0)
			thread->Tick(0x186A0);
	}
	RunTickLoopDetour->CallOriginal(r3, timer);
}

bool aesGrabbed;

void* AESSteal(void* buffer)
{
	AESDetour->CallOriginal(buffer);
	if (!aesGrabbed)
	{
		memcpy(aesContext, buffer, sizeof(aesContext));
		aesGrabbed = true;
	}
	return buffer;
}

void PushNative(char* name, void* func)
{
	((void(*)(char*, void*))0x82554798)(name, func);
}

void NativeHook()
{
	PushNative("START_NEW_SCRIPT_FROM_HDD", START_NEW_SCRIPT_FROM_HDD);
	PushNative("TERMINATE_SCRIPT_FROM_HDD", TERMINATE_SCRIPT_FROM_HDD);
	NativeDetour->CallOriginal();
}

bool IsOnGame()
{
	return (dwTitleID == 0x545407F8 && MmIsAddressValid((PVOID)0x826E4638) && *(int*)0x826E4638 == 0xE8C10058);
}

void StartScriptHook(int r3)
{
	printf("Starting Script: %s\n", (char*)*(int*)(*(int*)(r3 + 8)));
	StartScriptDetour->CallOriginal(r3);
}

void AddNative(char* name, void* func)
{
	printf("%s\n", name);
	AddNativeDetour->CallOriginal(name, func);
}

void Init()
{
	WAITDetour = new Detour<void>;
	RunTickLoopDetour = new Detour<void>;
	AESDetour = new Detour<void*>;
	NativeDetour = new Detour<void>;
	StartScriptDetour = new Detour<void>;
	AddNativeDetour = new Detour<void>;
	WAITDetour->SetupDetour(0x82553D30, WAITHook);
	RunTickLoopDetour->SetupDetour(0x82727BB0, RunTickLoop);
	StartScriptDetour->SetupDetour(0x825569E8, StartScriptHook);
	//AESDetour->SetupDetour(0x821E1610, AESSteal);
	//NativeDetour->SetupDetour(0x821E7E08, NativeHook);
	AddNativeDetour->SetupDetour(0x82554798, AddNative);
	initalized = false;
	XNotify(L"Midnight Club LA - Script Engine Loaded!");
}

void Uninit()
{
	if (!WAITDetour || !RunTickLoopDetour || !AESDetour || !NativeDetour || !StartScriptDetour || !AddNativeDetour)
		return;
	WAITDetour->TakeDownDetour();
	RunTickLoopDetour->TakeDownDetour();
	AESDetour->TakeDownDetour();
	NativeDetour->TakeDownDetour();
	StartScriptDetour->TakeDownDetour();
	AddNativeDetour->TakeDownDetour();
}

DWORD WINAPI MasterThread(LPVOID)
{
	DWORD dwTempTitleID = 0;
	for (;;)
	{
		dwTitleID = XamGetCurrentTitleId();
		if (fUnloadXEX == TRUE)
		{
			Uninit();
			Sleep(50);
			fUnloadXEX = FALSE;
			fGoodToUnload = TRUE;
			return S_OK;
		}
		if (::dwTitleID == dwTempTitleID)
			continue;
		Sleep(200);
		if (IsOnGame())
			Init();
		else
			Uninit();
		dwTempTitleID = ::dwTitleID;
	}
}

bool APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_DETACH)
	{
		fUnloadXEX = TRUE;
		while (fGoodToUnload == FALSE)
			Sleep(1);
	}
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		ExCreateThread(0, 0, 0, 0, MasterThread, 0, 2);
	}
	return true;
}
