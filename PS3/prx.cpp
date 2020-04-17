#include <sys/prx.h>
#include <sys/syscall.h>
#include <sys/ppu_thread.h>
#include <sys/sys_time.h>
#include <sys/time_util.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/timer.h>
#include <sys/return_code.h>
#include <sys/prx.h>
#include <sys/paths.h>
#include <cellstatus.h>
#include <cell/error.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <ctype.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cstdarg>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <ctime>
#include <typeinfo>
#include <vector>
#include <pthread.h>
#include <locale.h>

#ifdef _BLES
#define TOC 0xF6CCF8
#else
#define TOC 0xF6CCE8
#endif

#include "Dialog.h"
#include "globals.h"
#include "Detour.h"
#include "ScriptEngine.h"
#include "SCO.h"

SYS_MODULE_INFO(IVScriptEngine, 0, 1, 1);
SYS_MODULE_START(PRX_ENTRY);

Detour WaitDetour;
Detour AESDetour;
Detour NativeDetour;

bool initalized;

ScriptThread MainThread;

void WaitStub()
{
	__asm("nop");
	__asm("nop");
	__asm("nop");
	__asm("nop");
	__asm("nop");
}

void AESStub()
{
	__asm("nop");
	__asm("nop");
	__asm("nop");
	__asm("nop");
	__asm("nop");
}

void NativeStub()
{
	__asm("nop");
	__asm("nop");
	__asm("nop");
	__asm("nop");
	__asm("nop");
}

sys_prx_id_t loadedScript;
int modRes;

void WaitHook(int r3)
{
	if (*(byte*)0x10051000 == 0x01)
	{
		loadedScript = sys_prx_load_module((char*)0x10051001, 0, 0);
		sys_prx_start_module(loadedScript, 0, 0, &modRes, 0, 0);
		*(byte*)0x10051000 = 0x00;
	}
	else if (*(byte*)0x10051000 == 0x02)
	{
		sys_prx_stop_module(loadedScript, 0, 0, &modRes, 0, 0);
		sys_prx_unload_module(loadedScript, 0, 0);
		*(byte*)0x10051000 = 0x00;
	}
	if (!initalized)
	{
		printf("SPRX Load/Unload Hook Running!\n");
		InitalizeScriptEngine();
		ScriptEngine_CreateThread(&MainThread);
		StartScriptFromHdd("/dev_hdd0/tmp/autoload.sco", "autoload", NULL, 0, 0x400);
		initalized = true;
	}
	WaitDetour.CallOriginal<int>(r3);
}

bool aesGrabbed = false;

void* AESHook(void* aesCxt)
{
	AESDetour.CallOriginal<void*>(aesCxt);
	if (!aesGrabbed)
	{
		memcpy(aesContext, aesCxt, sizeof(aesContext));
		aesGrabbed = true;
	}
	return aesCxt;
}
#ifdef _BLES
opd_s AddNative_t = { 0xA389B8, TOC };
#else
opd_s AddNative_t = { 0xA387E8, TOC };
#endif
void(*AddNative)(char* name, void* function) = (void(*)(char*, void*))&AddNative_t;

void NativeHook()
{
	AddNative("START_NEW_SCRIPT_FROM_HDD", START_NEW_SCRIPT_FROM_HDD);
	AddNative("TERMINATE_SCRIPT_FROM_HDD", TERMINATE_SCRIPT_FROM_HDD);
	NativeDetour.CallOriginal<int>();
}

void Thread(uint64_t nothing)
{
#ifdef _BLES
	*(short*)0x4809E2 = 0x0000; 
	WaitDetour.SetupDetour(0xA368E0, TOC, (PVOID)WaitHook, (PVOID)WaitStub); 
	AESDetour.SetupDetour(0xA76F08, TOC, (PVOID)AESHook, (PVOID)AESStub);
	NativeDetour.SetupDetour(0x42ADD0, TOC, (PVOID)NativeHook, (PVOID)NativeStub);
#else
	*(int*)0x480788 = 0x4E800020;
	WaitDetour.SetupDetour(0xA36308, TOC, (PVOID)WaitHook, (PVOID)WaitStub);
	AESDetour.SetupDetour(0xA76D38, TOC, (PVOID)AESHook, (PVOID)AESStub);
	NativeDetour.SetupDetour(0x42AC40, TOC, (PVOID)NativeHook, (PVOID)NativeStub);
#endif
	*(byte*)0x10051000 = 0x00;
	initalized = false;
	PS3MSG("Grand Theft Auto IV Script Engine Loaded!");
	sys_ppu_thread_exit(nothing);
}

extern "C" int PRX_ENTRY()
{
	printf("SPRX Loaded!\n");
	sys_ppu_thread_t ThreadModuleID;
	sys_ppu_thread_create(&ThreadModuleID, Thread, 0, 10, 0x10, 0, "IVScriptEngine");
	return SYS_PRX_RESIDENT;
}