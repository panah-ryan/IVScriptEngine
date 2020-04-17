#pragma once

#include "stdafx.h"

#define MAGIC 0xE726373
#define COMPRESSED 0xE726353
#define UNCOMPRESSED 0xD524353

typedef struct scoHeader
{
	int magic;
	int codeSize;
	int globalCount;
	int publicCount;
	int codeFlags;
	int signature;
};

typedef struct scoScript
{
	char* name;
	int hash;
	char* codeSection;
	int* globalSection;
	int codeSize;
	short globalCount;
	short codeFlags;
	short state;
};

typedef struct native_s
{
	int* retBuffer;
	int count;
	int* callBuffer;
};

BYTE aesContext[0x1E4];
bool printedInfo = false;

scoScript* GetScriptFromTable(int hash)
{
	return ((scoScript*(*)(int))0x82846900)(hash);
}

int FileStreamOpen(int r3, char* path, char* fileType, int r6, int r7)
{
	return ((int(*)(int, char*, char*, int, int))0x8284F468)(r3, path, fileType, r6, r7);
}

void FileReadInts(int fileStream, void* buffer, int count)
{
	((void(*)(int, void*, int))0x8285B168)(fileStream, buffer, count);
}

void CloseFileStream(int fileStream)
{
	((void(*)(int))0x8285B088)(fileStream);
}

void FileReadBytes(int fileStream, void* buffer, int count)
{
	((void(*)(int, void*, int))0x8285AD08)(fileStream, buffer, count);
}

void SetupAESKey(void* aesKey)
{
	((void(*)(void*))0x8286B58C)(aesKey);
}

void DecryptAES(void* aesKey, void* buffer, int size)
{
	((void(*)(void*, void*, int))0x8286B5C0)(aesKey, buffer, size);
}

int inflateInit(void* zlib, char* version, int size)
{
	return ((int(*)(void*, char*, int))0x8286EF10)(zlib, version, size);
}

void inflate(void* zlib, int type)
{
	((void(*)(void*, int))0x8286F078)(zlib, type);
}

void inflateEnd(void* zlib)
{
	((void(*)(void*))0x82870830)(zlib);
}

bool ReplaceHashes(void* codeSection, int codeSize)
{
	return ((bool(*)(void*, int))0x828453F8)(codeSection, codeSize);
}

scoScript* SetupScript(scoScript* buffer, char* scriptName, void* codeSection, int codeSize, int* globalSection, int globalCount, int* publicSection, int publicCount, int codeFlags, int signature)
{
	return ((scoScript*(*)(scoScript*, char*, void*, int, int*, int, int*, int, int, int))0x82846780)(buffer, scriptName, codeSection, codeSize, globalSection, globalCount, publicSection, publicCount, codeFlags, signature);
}

bool InjectHash(int scriptPool, int hash, scoScript* buffer)
{
	return ((bool(*)(int, int, scoScript*))0x82846948)(scriptPool, hash, buffer);
}

int LoadScript(int hash, void* pArgs, int argCount, int stackSize)
{
	return ((int(*)(int, void*, int, int))0x828458C8)(hash, pArgs, argCount, stackSize);
}

int FinishUpLoadingScript(scoScript* buffer)
{
	return ((int(*)(scoScript*))0x82846A20)(buffer);
}

void KillScriptThread(ScriptThread* thread)
{
	((void(*)(ScriptThread*))0x82845AB8)(thread);
}

char* GetScriptPointerFromPool(int hash)
{
	int value = (hash / *(int*)0x82B07024) * *(int*)0x82B07024;
	value = (hash - value) << 3;
	return (char*)(*(int*)0x82B07020 + value);
}

static __forceinline scoScript* SCOAlloc()
{
	return (scoScript*)XMemAlloc(sizeof(scoScript), MAKE_XALLOC_ATTRIBUTES(0, 0, 0, 0, eXALLOCAllocatorId_GameMax, XALLOC_ALIGNMENT_DEFAULT, XALLOC_MEMPROTECT_READWRITE, TRUE, XALLOC_MEMTYPE_HEAP));
}

static __forceinline void SCOFree(scoScript* data)
{
	XMemFree(data, MAKE_XALLOC_ATTRIBUTES(0, 0, 0, 0, eXALLOCAllocatorId_GameMax, XALLOC_ALIGNMENT_DEFAULT, XALLOC_MEMPROTECT_READWRITE, TRUE, XALLOC_MEMTYPE_HEAP));
}

int ReadScript(char* scriptPath, char* scriptName)
{
	scoHeader header;
	int compressedSize;

	unsigned int hash = jenkinsHash(scriptName);
	if (GetScriptFromTable(hash) != 0)
	{
		printf("Script already loaded with that name!\n");
		return 0;
	}
	int scoFile = FileStreamOpen(0x82B07278, scriptPath, "sco", 0, 1);
	if (scoFile == NULL)
	{
		printf("Error Opening Script File!\n");
		return 0;
	}
	printf("Loading script program '%s'...\n", scriptPath);
	FileReadInts(scoFile, &header, 6);
	if (header.magic != MAGIC && header.magic != COMPRESSED && header.magic != UNCOMPRESSED)
	{
		printf("%s - Old version of script format (header should be %x but is %x)?\n", scriptPath, MAGIC, header.magic);
		CloseFileStream(scoFile);
		return 0;
	}
	if (header.magic == COMPRESSED)
		FileReadInts(scoFile, &compressedSize, 1);
	void* codeBuffer = malloc(header.codeSize);
	int* globalBuffer = (int*)malloc((header.globalCount <= 0x3FFFFFFF) ? header.globalCount << 2 : -1);
	int* publicBuffer = (int*)malloc((header.publicCount <= 0x3FFFFFFF) ? header.publicCount << 2 : -1);
	if (header.magic == COMPRESSED)
	{
		printf("%s is compressed!\n", scriptPath);
		void* compressedBuffer = malloc(compressedSize);
		FileReadBytes(scoFile, compressedBuffer, compressedSize);
		printf("Read all Compressed Data!\n");
		CloseFileStream(scoFile);
		DecryptAES(&aesContext, compressedBuffer, compressedSize);
		printf("Decrypted Script File!\n");
		char* zlibBuffer = (char*)malloc(0x38);
		ZeroMemory(zlibBuffer, 0x38);
		if (inflateInit(zlibBuffer, "1.2.3", 0x38) < 0)
		{
			printf("Error in inflateInit");
			free(zlibBuffer);
			free(compressedBuffer);
			free(publicBuffer);
			free(globalBuffer);
			free(codeBuffer);
			return 0;
		}
		*(int*)zlibBuffer = (int)compressedBuffer;
		*(int*)(zlibBuffer + 0x0C) = (int)codeBuffer;
		*(int*)(zlibBuffer + 4) = compressedSize;
		*(int*)(zlibBuffer + 0x10) = header.codeSize;
		inflate(zlibBuffer, 2);
		printf("Decompressed Code Section!\n");
		*(int*)(zlibBuffer + 0x0C) = (int)globalBuffer;
		*(int*)(zlibBuffer + 0x10) = header.globalCount << 2;
		inflate(zlibBuffer, 2);
		printf("Decompressed Global Section!\n");
		*(int*)(zlibBuffer + 0x0C) = (int)publicBuffer;
		*(int*)(zlibBuffer + 0x10) = header.publicCount << 2;
		inflate(zlibBuffer, 2);
		printf("Decompressed Public Section!\n");
		inflateEnd(zlibBuffer);
		free(compressedBuffer);
		free(zlibBuffer);
	}
	else if (header.magic == MAGIC)
	{
		printf("%s is uncompressed!\n", scriptPath);
		FileReadBytes(scoFile, codeBuffer, header.codeSize);
		FileReadBytes(scoFile, globalBuffer, header.globalCount << 2);
		FileReadBytes(scoFile, publicBuffer, header.publicCount << 2);
		CloseFileStream(scoFile);
		DecryptAES(&aesContext, codeBuffer, header.codeSize);
		printf("Decrypted Code Section!\n");
		DecryptAES(&aesContext, globalBuffer, header.globalCount << 2);
		printf("Decrypted Global Section!\n");
		DecryptAES(&aesContext, publicBuffer, header.publicCount << 2);
		printf("Decrypted Public Section!\n");
	}
	else
	{
		printf("%s is uncompressed and unencrypted!\n", scriptPath);
		FileReadBytes(scoFile, codeBuffer, header.codeSize);
		FileReadBytes(scoFile, globalBuffer, header.globalCount << 2);
		FileReadBytes(scoFile, publicBuffer, header.publicCount << 2);
		CloseFileStream(scoFile);
	}
	if (!ReplaceHashes(codeBuffer, header.codeSize))
	{
		printf("Script '%s' failed validation, it's using unsupported NATIVE commands\n", scriptPath);
		free(codeBuffer);
		free(globalBuffer);
		free(publicBuffer);
		return 0;
	}
	for (int i = 0; i < header.globalCount; i++)
		globalBuffer[i] = _byteswap_ulong(globalBuffer[i]);
	for (int i = 0; i < header.publicCount; i++)
		publicBuffer[i] = _byteswap_ulong(publicBuffer[i]);
	scoScript* scoBuffer = SCOAlloc();
	SetupScript(scoBuffer, scriptName, codeBuffer, header.codeSize, globalBuffer, header.globalCount, publicBuffer, header.publicCount, header.codeFlags, header.signature);
	if (scoBuffer->codeSection[0] == 0)
	{
		printf("Error setting up the script!\n");
		free(codeBuffer);
		free(globalBuffer);
		free(publicBuffer);
		SCOFree(scoBuffer);
		return 0;
	}
	if (!InjectHash(0x82B07020, hash, scoBuffer))
		printf("Script Failed to inject!\n");
	free(codeBuffer);
	free(globalBuffer);
	free(publicBuffer);
	if (header.signature != 0 && *(int*)0x831927BC == 0)
		printf("Script '%s' requires globals that are not available.  Load the script with the GLOBALS TRUE directive first!\n", scriptName);
	return hash;
}

void StartScriptFromHdd(char* scriptPath, char* scriptName, void* pArgs, int argCount, int stackSize)
{
	int hash = ReadScript(scriptPath, scriptName);
	if (hash == 0)
	{
		printf("Error Reading Script! Skipped Loading!\n");
		return;
	}
	if (!LoadScript(hash, pArgs, argCount, stackSize))
	{
		printf("Error Loading Script!\n");
		return;
	}
	pgPtrCollection<ScriptThread>* collection = ScriptEngine::GetThreadCollection();
	for (int i = 0; i < collection->count(); i++)
	{
		if (collection->at(i)->m_context.scriptHash == hash)
			printf("Thread Index: %i Location: 0x%X\n", i, collection->at(i));
	}
	scoScript* script = GetScriptFromTable(hash);
	FinishUpLoadingScript(script);
	printf("%s Loaded from %s into this address 0x%X\n", scriptName, scriptPath, script);
}

/*
Param 1 - const char* Script Location
Param 2 - const char* Script Name
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
	(stackSize == 0) ? printf("Stack Size needs to be greater than 0!\n") : StartScriptFromHdd(scriptPath, scriptName, pArgs, argCount, stackSize);
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
			KillScriptThread(thread);
			printf("Killed Script Thread for %s\n", scriptName);
			SCOFree(GetScriptFromTable(hash));
			printf("Cleaned up memory for %s!\n", scriptName);
			char* poolAddr = GetScriptPointerFromPool(hash);
			ZeroMemory(poolAddr, 4);
			memset(poolAddr + 4, 0xCD, 4);
			printf("Removed %s from Hash Table!\n", scriptName);
			return;
		}
	}
}