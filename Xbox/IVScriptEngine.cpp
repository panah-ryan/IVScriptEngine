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
Detour<void> *NativeDetour;
Detour<void*> *AESDetour;
Detour<int> *ScriptDebuggerDetour;

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
HANDLE autoloadxex;

void WAITHook(void* r3)
{
	if (!initalized)
	{
		ScriptEngine::Initalize();
		ScriptEngine::CreateThread(&g_ScriptManagerThread);
		StartScriptFromHdd("GAME:\\autoload.sco", "autoload", NULL, 0, 0x400);
		initalized = true;
	}
	WAITDetour->CallOriginal(r3);
}

int debugScriptHash = -1;
bool debuggingascript = false;

int ScriptDebuggerHook(ScriptThread* thread, int opsToExecute)
{
	if (thread->m_context.scriptHash == debugScriptHash && debuggingascript)
	{
		scoScript* script = GetScriptFromTable(thread->m_context.scriptHash);
			//printf("Thread - 0x%X codeSection - 0x%X pScript = 0x%X\n", thread, script->codeSection, script);
		thread->scriptPaused = true;
	}
	return ScriptDebuggerDetour->CallOriginal(thread, opsToExecute);
}

byte originalOpcode = 0x00;
scoScript* debugSCO;
int BreakpointPC = 0;
ScriptThread* debugThread;

void BreakpointHandle(ScriptThread* thread, int PC, int* currentStackLoc)
{
	BreakpointPC = PC - 1;
	debugThread = thread;
	debugSCO = GetScriptFromTable(thread->m_context.scriptHash);
	printf("%s script stopped with breakpoint at 0x%0x\n", thread->scriptName, (int)(debugSCO->codeSection + PC));
	printf("Code Section: 0x%X - Size %i\n", debugSCO->codeSection, debugSCO->codeSize);
	printf("Current Stack Pointer: 0x%X & SP = %i Func SP = %i & Script Stack Pointer = 0x%X\n", currentStackLoc, thread->m_context.SP, thread->m_context.currentFuncSP, thread->stack);
	int* funcStackLoc = (int*)((int)currentStackLoc - (thread->m_context.currentFuncSP << 2));
	printf("Stack Dump:\n");
	for (int i = 0; i < thread->m_context.currentFuncSP; i++)
		printf("\t0x%X\n", funcStackLoc[i]);
	thread->scriptPaused = true;
}

extern "C" void SetBreakPoint(char* scriptName, int offset)
{
	scoScript* sco = GetScriptFromTable(jenkinsHash(scriptName));
	if (sco)
	{
		originalOpcode = sco->codeSection[offset];
		sco->codeSection[offset] = 0x4B;
		debuggingascript = true;
		printf("Breakpoint Enabled!\n");
	}
	else
		printf("Error couldn't find that script!\n");
}

extern "C" void RemoveBreakPoint()
{
	debugSCO->codeSection[BreakpointPC] = originalOpcode;
	debugThread->scriptPaused = false;
	debuggingascript = false;
	printf("Breakpoint Disabled!\n");
}

__inline __declspec(naked) int BreakPointOpcode()
{
	__asm
	{
		mr r5, r31
		mr r4, r27
		mr r3, r30
		bl BreakpointHandle
		addi r27, r27, -1
		lis r10, 0x8284
		ori r10, r10, 0x5308
		mtctr r10
		bctr
	}
}

int scriptPrintHash = 0;

extern "C" void EnableScriptPrints(char* scriptName)
{
	scriptPrintHash = jenkinsHash(scriptName);
	printf("Script prints enabled for: 0x%X\n", scriptName);
}

void PRINTSTRING(char* string)
{
	if(*(int*)(*(int*)0x8319277C +8) == scriptPrintHash)
		printf("%s", string);
}

void PRINTFLOAT(int r3, int r4, float value)
{
	if (*(int*)(*(int*)0x8319277C + 8) == scriptPrintHash)
	{
		if (r3 || r4)
			printf("%*.*f", r3, r4, value);
		else
			printf("%f", value);
	}
}

void PRINTINT(int r3, int r4)
{
	if (*(int*)(*(int*)0x8319277C + 8) == scriptPrintHash)
	{
		if (r3)
			printf("%*d", r3, r4);
		else
			printf("%d", r4);
	}
}

void PRINTNL()
{
	if (*(int*)(*(int*)0x8319277C + 8) == scriptPrintHash)
		printf("\n");
}

void PRINTVECTOR(float* r3)
{
	if (*(int*)(*(int*)0x8319277C + 8) == scriptPrintHash)
		printf("<< %g, %g, %g >>", r3[0], r3[1], r3[2]);
}

bool IsOnGame()
{
	return (dwTitleID == 0x545407F2 && MmIsAddressValid((PVOID)0x826E4638) && *(int*)0x826E4638 == 0x4816EF49);
}

void AddNativeFunction(char* name, void* function)
{
	((void(*)(char*, void*))0x82845600)(name, function);
}

void NativeInject()
{
	AddNativeFunction("START_NEW_SCRIPT_FROM_HDD", START_NEW_SCRIPT_FROM_HDD);
	AddNativeFunction("TERMINATE_SCRIPT_FROM_HDD", TERMINATE_SCRIPT_FROM_HDD);
	NativeDetour->CallOriginal();
}

bool aesGrabbed;

void* AESSteal(void* aesCxt)
{
	AESDetour->CallOriginal(aesCxt);
	if (!aesGrabbed)
	{
		memcpy(aesContext, aesCxt, sizeof(aesContext));
		aesGrabbed = true;
	}
	return aesCxt;
}

void Init()
{
	WAITDetour = new Detour<void>;
	NativeDetour = new Detour<void>;
	AESDetour = new Detour<void*>;
	ScriptDebuggerDetour = new Detour<int>;
	WAITDetour->SetupDetour(0x82843908, WAITHook);
	NativeDetour->SetupDetour(0x8257D0B8, NativeInject);
	AESDetour->SetupDetour(0x8286B588, AESSteal);
	*(int*)0x82B07004 = (int)PRINTSTRING;
	*(int*)0x82B07008 = (int)PRINTFLOAT;
	*(int*)0x82B0700C = (int)PRINTINT;
	*(int*)0x82B07010 = (int)PRINTNL;
	*(int*)0x82B07014 = (int)PRINTVECTOR;
	ScriptDebuggerDetour->SetupDetour(0x825E87C8, ScriptDebuggerHook);
	initalized = false;
	*(short*)0x821B55E0 = 0x7F03;
	*(short*)0x828442A2 = 0x004B;
	*(int*)0x828443E8 = (int)BreakPointOpcode;
	XNotify(L"Grand Theft Auto IV - Script Engine Loaded!");
}


PCHAR va(PCHAR Text, ...)
{
	char Buffer[2048];
	va_list VA_ARGS;
	va_start(VA_ARGS, Text);
	vsprintf(Buffer, Text, VA_ARGS);
	return Buffer;
}

void Uninit()
{
	if (!WAITDetour || !NativeDetour || !AESDetour || !ScriptDebuggerDetour)
		return;
	WAITDetour->TakeDownDetour();
	NativeDetour->TakeDownDetour();
	AESDetour->TakeDownDetour();
	ScriptDebuggerDetour->TakeDownDetour();
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
