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

Detour<void> *WAITDetour;
Detour<void*> *AESDetour;
Detour<void> *NativeDetour;

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

typedef struct injectNative
{
	int value;
	void* func;
};

injectNative customNatives[2];

void PushNative(char* name, injectNative* native)
{
	((void(*)(char*, injectNative*))0x82FDA368)(name, native);
}

void NativeHook()
{
	customNatives[0].value = 1;
	customNatives[0].func = START_NEW_SCRIPT_FROM_HDD;
	PushNative("START_NEW_SCRIPT_FROM_HDD", &customNatives[0]);
	customNatives[1].value = 2;
	customNatives[1].func = TERMINATE_SCRIPT_FROM_HDD;
	PushNative("TERMINATE_SCRIPT_FROM_HDD", &customNatives[1]);
	NativeDetour->CallOriginal();
}

void AddNative(char* name, void* func)
{
	printf("%s\n", name);
	NativeDetour->CallOriginal(name, func);
}

bool IsOnGame()
{
	return (dwTitleID == 0x5454086B && MmIsAddressValid((PVOID)0x826E4638) && *(int*)0x826E4638 == 0x8181FFF8);
}

void Init()
{
	WAITDetour = new Detour<void>;
	AESDetour = new Detour<void*>;
	NativeDetour = new Detour<void>;
	WAITDetour->SetupDetour(0x82FD7500, WAITHook);
	AESDetour->SetupDetour(0x82CB8720, AESSteal);
	//NativeDetour->SetupDetour(0x82823D98, NativeHook);
	NativeDetour->SetupDetour(0x82FDA368, AddNative);
	initalized = false;
	XNotify(L"Max Payne 3 - Script Engine Loaded!");
}

void Uninit()
{
	if (!WAITDetour || !AESDetour || !NativeDetour)
		return;
	WAITDetour->TakeDownDetour();
	AESDetour->TakeDownDetour();
	NativeDetour->TakeDownDetour();
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
