#pragma once
#include "stdafx.h"

#define ZLIB1 0x11726373
#define ZLIB2 0x11726353
#define MAGIC1 0x10726373
#define MAGIC2 0x10726353

typedef struct z_stream_s
{
	char* next_in; //0x00
	int avail_in; //0x04
	int total_in; //0x08
	char* next_out; //0x0C
	int avail_out; //0x10
	int total_out; //0x14
	char* msg; //0x18
	void* state; //0x1C
	void* zalloc; //0x20
	void* zfree; //0x24
	void* opaque; //0x28
	int data_type; //0x2C
	int adler; //0x30
};

typedef struct native_s
{
	ScriptThread* thread;
	int* retBuffer;
	int count;
	int* callBuffer;
};

typedef struct scoHeader
{
	int magic;
	int codeSize;
	int staticCount;
	int globalCount;
	int scriptFlags;
	int signature;
	int compressedSize;
};

typedef struct scoScript
{
	char* name;
	int hash;
	int _0x08;
	char* codeSection;
	int* staticSection;
	int codeSize;
	short staticCount;
	short scriptFlags;
	short state;
	short _0x1E;
};

BYTE aesContext[0x1E4];

int LoadScript(int hash, void* pArgs, int argCount, int stackSize, int r7)
{
	return ((int(*)(int, void*, int, int, int))0x82FDA928)(hash, pArgs, argCount, stackSize, r7);
}

scoScript* GetScriptFromTable(int hash)
{
	return ((scoScript*(*)(int))0x82FDC4B0)(hash);
}

int FinishLoadingScript(scoScript* script)
{
	return ((int(*)(scoScript*))0x82FDC5D0)(script);
}

char* GetScriptPointerFromPool(int hash)
{
	int value = (hash / *(int*)0x831F5358) * *(int*)0x831F5358;
	value = (hash - value) << 3;
	return (char*)(*(int*)0x831F5354 + value);
}

int FileStreamOpen(int r3, char* path, char* type, int r6, int r7)
{
	return ((int(*)(int, char*, char*, int, int))0x82CC28B0)(r3, path, type, r6, r7);
}

void FileReadInts(int file, void* buffer, int count)
{
	((void(*)(int, void*, int))0x82CB7068)(file, buffer, count);
}

void CloseFileStream(int file)
{
	((void(*)(int))0x82CB6F78)(file);
}

void FileReadBytes(int file, void* buffer, int count)
{
	((void(*)(int, void*, int))0x82CB6BC0)(file, buffer, count);
}

void DecryptAES(BYTE* context, void* buffer, int size)
{
	((void(*)(BYTE*, void*, int))0x82CB87C8)(context, buffer, size);
}

int inflateInit(char* zlib, char* version, size_t size)
{
	return ((int(*)(char*, char*, size_t))0x82CEEB88)(zlib, version, size);
}

int inflate(char* zlib, int type)
{
	return ((int(*)(char*, int))0x82CEECE8)(zlib, type);
}

void inflateEnd(char* zlib)
{
	((void(*)(char*))0x82CF0460)(zlib);
}

scoScript* SetupScript(scoScript* buffer, char* scriptName, char* codeBuffer, int codeSize, int* staticBuffer, int staticCount, int* globalBuffer, int globalCount, bool r11, int scriptFlags, int globalSignature)
{
	return ((scoScript*(*)(scoScript*, char*, char*, int, int*, int, int*, int, bool, int, int))0x82FDC2C8)(buffer, scriptName, codeBuffer, codeSize, staticBuffer, staticCount, globalBuffer, globalCount, r11, scriptFlags, globalSignature);
}

BOOL ReplaceHashes(void* buffer, int size)
{
	return ((BOOL(*)(void*, int))0x82FDA0C8)(buffer, size);
}

int InjectHash(int table, int hash, void* pointer)
{
	return ((int(*)(int, int, void*))0x82FDC4F8)(table, hash, pointer);
}

BOOL ReadScript(char* path, char* name)
{
	scoHeader header;
	int compressedSize = 0;
	char zlib[0x38];

	int hash = jenkinsHash(name);
	if (GetScriptFromTable(hash) != NULL)
	{
		printf("Script already exists in the hash table!\n");
		return FALSE;
	}
	int file = FileStreamOpen(0x831C9E90, path, "sco", 0, 1);
	if (file == NULL)
	{
		printf("Failed to open script file!\n");
		return FALSE;
	}
	FileReadInts(file, &header, 7);
	if (header.magic != ZLIB1 && header.magic != ZLIB2 && header.magic != MAGIC1 && header.magic != MAGIC2)
	{
		printf("%s - Old version of script format. Please get latest compiler, recompile, and try again. (header is 0x%08x, but should be one of 0x%08x, 0x%08x, 0x%08x, 0x%08x)?\n", path, header.magic, ZLIB1, ZLIB2, MAGIC1, MAGIC2);
		return FALSE;
	}
	char* codeBuffer = (char*)malloc(header.codeSize);
	int* staticBuffer = (int*)malloc((header.staticCount <= 0x3FFFFFFF) ? header.staticCount << 2 : -1);
	int* globalBuffer = (int*)malloc((header.globalCount <= 0x3FFFFFFF) ? header.globalCount << 2 : -1);
	FileReadBytes(file, codeBuffer, header.codeSize);
	FileReadBytes(file, staticBuffer, header.staticCount << 2);
	FileReadBytes(file, globalBuffer, header.globalCount << 2);
	CloseFileStream(file);
	DecryptAES(aesContext, codeBuffer, header.codeSize);
	DecryptAES(aesContext, staticBuffer, header.staticCount << 2);
	DecryptAES(aesContext, globalBuffer, header.globalCount << 2);
	for (int i = 0; i < header.staticCount; i++)
		staticBuffer[i] = _byteswap_ulong(staticBuffer[i]);
	for (int i = 0; i < header.globalCount; i++)
		globalBuffer[i] = _byteswap_ulong(globalBuffer[i]);
	scoScript* script = (scoScript*)malloc(sizeof(scoScript));
	SetupScript(script, name, codeBuffer, header.codeSize, staticBuffer, header.staticCount, globalBuffer, header.globalCount, false, header.scriptFlags, header.signature);
	if (script->codeSection[0] == 0)
	{
		printf("Failed to setup script!\n");
		free(codeBuffer);
		free(staticBuffer);
		free(globalBuffer);
		free(script);
		return FALSE;
	}
	if (!ReplaceHashes(&script->codeSection, header.codeSize))
	{
		printf("Script '%s' failed validation: it's using unsupported NATIVE commands. You can ignore this assertion for now but you will crash later if that function is called.\n", name);
		free(codeBuffer);
		free(staticBuffer);
		free(globalBuffer);
		free(script);
		return FALSE;
	}
	if (!InjectHash(0x831F5354, hash, script))
	{
		printf("Failed to inject script into hash table!\n");
		free(codeBuffer);
		free(staticBuffer);
		free(globalBuffer);
		free(script);
		return FALSE;
	}
	free(codeBuffer);
	free(staticBuffer);
	free(globalBuffer);
	if (header.signature != 0 && *(int*)0x83881348 == 0)
		printf("Script '%s' requires globals that are not available.  Load the script with the GLOBALS TRUE directive first!\n", name);
	return hash;
}

/*
Param 1 - const char* script location
Param 2 - const char* script name
Param 3 - void* pArgs
Param 4 - int argCount
Param 5 - int stackSize
*/
void START_NEW_SCRIPT_FROM_HDD(native_s* call)
{
	char* scriptPath = (char*)call->callBuffer[0];
	char* scriptName = (char*)call->callBuffer[1];
	void* pArgs = (void*)call->callBuffer[2];
	int argCount = call->callBuffer[3];
	int stackSize = call->callBuffer[4];
	ScriptThread* oldThread;
	if (stackSize == 0)
		printf("Stack size needs to be greater than 0!\n");
	else
	{
		int hash = ReadScript(scriptPath, scriptName);
		if (hash == 0)
		{
			printf("Error Reading Script! Skipped Loading!\n");
			return;
		}
		pgPtrCollection<ScriptThread>* collection = ScriptEngine::GetThreadCollection();
		for (int i = 0; i < collection->count(); i++)
			if (collection->at(i)->m_context.scriptHash == hash && collection->at(i)->m_context.state == ThreadStateKilled)
				oldThread = collection->at(i);
		if (oldThread)
			oldThread->m_context.scriptHash = 0;
		if (!LoadScript(hash, pArgs, argCount, stackSize, 0))
		{
			printf("Error Loading Script!\n");
			return;
		}
		if (oldThread)
			oldThread->m_context.scriptHash = hash;
		for (int i = 0; i < collection->count(); i++)
			if (collection->at(i)->m_context.scriptHash == hash)
				printf("Thread Index: %i Location: 0x%X\n", i, collection->at(i));
		scoScript* script = GetScriptFromTable(hash);
		FinishLoadingScript(script);
		printf("%s loaded from %s into this address 0x%X\n", scriptName, scriptPath, script);
	}
}

//Param 1 - const char* script name
void TERMINATE_SCRIPT_FROM_HDD(native_s* call)
{
	char* scriptName = (char*)call->callBuffer[0];
	int hash = jenkinsHash(scriptName);
	ScriptThread* thread;
	pgPtrCollection<ScriptThread>* collection = ScriptEngine::GetThreadCollection();
	for (int i = 0; i < collection->count(); i++)
	{
		thread = collection->at(i);
		if (thread->m_context.scriptHash == hash)
		{
			thread->m_context.state = ThreadStateKilled;
			printf("Killed script thread for %s\n", scriptName);
			free(GetScriptFromTable(hash));
			printf("Cleaned up memory for %s!\n", scriptName);
			char* poolAddr = GetScriptPointerFromPool(hash);
			ZeroMemory(poolAddr, 4);
			memset(poolAddr + 4, 0xCD, 4);
			printf("Removed %s from hash table!\n", scriptName);
			return;
		}
	}
}
