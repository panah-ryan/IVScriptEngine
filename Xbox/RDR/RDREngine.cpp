// CoDEngine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Detour.h"
#include "ScriptEngine.h"
#include "ScriptManager.h"

HANDLE hModule;
DWORD dwTitleID;
BOOL fUnloadXEX, fGoodToUnload;

Detour<void> *WAITDetour;

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

bool IsOnGame()
{
	return (dwTitleID == 0x5454082B && MmIsAddressValid((PVOID)0x826E4638) && *(int*)0x826E4638 == 0x91230014);
}

void Init()
{
	WAITDetour = new Detour<void>;
	WAITDetour->SetupDetour(0x822ADBA0, WAITHook);
	initalized = false;
	XNotify(L"Red Dead Redemption - Script Engine Loaded!");
}

void Uninit()
{
	if (!WAITDetour)
		return;
	WAITDetour->TakeDownDetour();
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
