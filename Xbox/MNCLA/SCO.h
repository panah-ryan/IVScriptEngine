#pragma once

#include "stdafx.h"

#define COMPRESSED 0xE726353
#define MAGIC1 0xE524353
#define MAGIC2 0xE726373

typedef struct scoScript
{
	char* name;
	int hash;
	char* codeSection;
	int* staticSection;
	int codeSize;
	short staticCount;
	short codeFlags;
	short state;
};

typedef struct scoHeader
{
	int magic;
	int codeSize;
	int staticCount;
	int globalCount;
	int codeFlags;
	int signature;
};

typedef struct z_stream_s
{
	char* next_in;
	int avail_in;
	int total_in;
	char* next_out;
	int avail_out;
	int total_out;
	char* msg;
	void* state;
	void* zalloc;
	void* zfree;
	void* opaque;
	int data_type;
	int adler;
	int reserved;
};

typedef struct native_s
{
	int* retBuffer;
	int count;
	int* callBuffer;
};

BYTE aesContext[0x1E4];

scoScript* GetScriptFromTable(int hash)
{
	return ((scoScript*(*)(int))0x825578B8)(hash);
}

int FileStreamOpen(int r3, char* path, char* fileType, int r6, int r7)
{
	return ((int(*)(int, char*, char*, int, int))0x821CA6A8)(r3, path, fileType, r6, r7);
}

void FileReadInts(int file, void* buffer, int count)
{
	((void(*)(int, void*, int))0x821BE710)(file, buffer, count);
}

void CloseFileStream(int file)
{
	((void(*)(int))0x821BE610)(file);
}

void FileReadBytes(int file, void* buffer, int count)
{
	((void(*)(int, void*, int))0x821BE250)(file, buffer, count);
}

void DecryptAES(BYTE* context, void* buffer, int size)
{
	((void(*)(BYTE*, void*, int))0x821E1658)(context, buffer, size);
}

int inflateInit(z_stream_s* zlib, char* version, size_t size)
{
	return ((int(*)(z_stream_s*, char*, size_t))0x821DECE0)(zlib, version, size);
}

int inflate(z_stream_s* zlib, int type)
{
	return ((int(*)(z_stream_s*, int))0x821DEE40)(zlib, type);
}

void inflateEnd(z_stream_s* zlib)
{
	((void(*)(z_stream_s*))0x821E05E0)(zlib);
}

BOOL ReplaceHashes(void* buffer, int size)
{
	return ((BOOL(*)(void*, int))0x82554630)(buffer, size);
}

scoScript* SetupScript(scoScript* buffer, char* scriptName, char* codeBuffer, int codeSize, int* staticBuffer, int staticCount, int* globalBuffer, int globalCount, int codeFlags, int signature)
{
	return ((scoScript*(*)(scoScript*, char*, char*, int, int*, int, int*, int, int, int))0x82557748)(buffer, scriptName, codeBuffer, codeSize, staticBuffer, staticCount, globalBuffer, globalCount, codeFlags, signature);
}

int InjectHash(int table, int hash, void* pointer)
{
	return ((int(*)(int, int, void*))0x825578C8)(table, hash, pointer);
}

int LoadScript(int hash, void* pArgs, int argCount, int stackSize)
{
	return ((int(*)(int, void*, int, int))0x82554A88)(hash, pArgs, argCount, stackSize);
}

int FinishLoadingScript(scoScript* script)
{
	return ((int(*)(scoScript*))0x825579A0)(script);
}

char* GetScriptPointerFromPool(int hash)
{
	int value = (hash / *(int*)0x82811138) * *(int*)0x82811138;
	value = hash - value;
	return (char*)(*(int*)0x82811134 + value);
}

BOOL ReadScript(char* path, char* scriptName)
{
	scoHeader header;
	int compressedSize = 0;
	z_stream_s zlib;

	int hash = jenkinsHash(scriptName);
	if (GetScriptFromTable(hash) != NULL)
	{
		printf("'%s' already found in script table!\n", scriptName);
		return FALSE;
	}
	int file = FileStreamOpen(0x827D7770, path, "sco", 0, 1);
	if (file == NULL)
	{
		printf("Couldn't open script file '%s'!\n", path);
		return FALSE;
	}
	printf("Loading script program '%s'...\n", path);
	FileReadInts(file, &header, 6);
	if (header.magic != COMPRESSED && header.magic != MAGIC1 && header.magic != MAGIC2)
	{
		printf("%s - Old version of script format (header should be %x but is %x)?\n", path, MAGIC2, header.magic);
		CloseFileStream(file);
		return FALSE;
	}
	if (header.magic == COMPRESSED)
		FileReadInts(file, &compressedSize, 1);
	char* codeBuffer = (char*)malloc(header.codeSize);
	int* staticBuffer = (int*)malloc((header.staticCount <= 1073741823) ? header.staticCount << 2 : -1);
	int* globalBuffer = (int*)malloc((header.globalCount <= 1073741823) ? header.globalCount << 2 : -1);
	if (compressedSize)
	{
		void* compressedBuffer = malloc(compressedSize);
		FileReadBytes(file, compressedBuffer, compressedSize);
		CloseFileStream(file);
		DecryptAES(aesContext, compressedBuffer, compressedSize);
		ZeroMemory(&zlib, sizeof(zlib));
		if (!inflateInit(&zlib, "1.2.3", sizeof(zlib)))
		{
			printf("Error in inflateInit\n");
			free(compressedBuffer);
			free(codeBuffer);
			free(globalBuffer);
			free(staticBuffer);
			return FALSE;
		}
		zlib.next_in = (char*)compressedBuffer;
		zlib.next_out = codeBuffer;
		zlib.avail_in = compressedSize;
		zlib.avail_out = header.codeSize;
		if (!inflate(&zlib, 2))
		{
			printf("Error in script inflate to %s\n", "opcodes");
			free(compressedBuffer);
			free(codeBuffer);
			free(globalBuffer);
			free(staticBuffer);
			return FALSE;
		}
		zlib.next_out = (char*)staticBuffer;
		zlib.avail_out = header.staticCount << 2;
		if (!inflate(&zlib, 2))
		{
			printf("Error in script inflate to %s\n", "statics");
			free(compressedBuffer);
			free(codeBuffer);
			free(staticBuffer);
			free(globalBuffer);
			return FALSE;
		}
		zlib.next_out = (char*)globalBuffer;
		zlib.avail_out = header.globalCount << 2;
		if (!inflate(&zlib, 2))
		{
			printf("Error in script inflate to %s\n", "globals");
			free(compressedBuffer);
			free(codeBuffer);
			free(staticBuffer);
			free(globalBuffer);
			return FALSE;
		}
		inflateEnd(&zlib);
		free(compressedBuffer);
	}
	else
	{
		FileReadBytes(file, codeBuffer, header.codeSize);
		FileReadBytes(file, staticBuffer, header.staticCount << 2);
		FileReadBytes(file, globalBuffer, header.globalCount << 2);
		CloseFileStream(file);
		DecryptAES(aesContext, codeBuffer, header.codeSize);
		DecryptAES(aesContext, staticBuffer, header.staticCount << 2);
		DecryptAES(aesContext, globalBuffer, header.globalCount << 2);
	}
	if (!ReplaceHashes(codeBuffer, header.codeSize))
	{
		printf("Script '%s' failed validation, it's using unsupported NATIVE commands\n", path);
		free(codeBuffer);
		free(staticBuffer);
		free(globalBuffer);
		return FALSE;
	}
	for (int i = 0; i < header.staticCount; i++)
		staticBuffer[i] = _byteswap_ulong(staticBuffer[i]);
	for (int i = 0; i < header.globalCount; i++)
		globalBuffer[i] = _byteswap_ulong(globalBuffer[i]);
	scoScript* script = (scoScript*)malloc(sizeof(scoScript));
	SetupScript(script, scriptName, codeBuffer, header.codeSize, staticBuffer, header.staticCount, globalBuffer, header.globalCount, header.codeFlags, header.signature);
	if (script->codeSection[0] == 0)
	{
		printf("'%s' failed to be setup!\n", scriptName);
		free(codeBuffer);
		free(globalBuffer);
		free(staticBuffer);
		free(script);
		return FALSE;
	}
	if (!InjectHash(0x82811134, hash, script))
	{
		printf("'%s' failed to be injected into the hsah table!\n", scriptName);
		free(codeBuffer);
		free(staticBuffer);
		free(globalBuffer);
		free(script);
		return FALSE;
	}
	free(codeBuffer);
	free(staticBuffer);
	free(globalBuffer);
	if (header.signature != 0 && *(int*)0x828DA3FC == 0)
		printf("Script '%s' requires globals that are not available.  Load the script with the GLOBALS TRUE directive first!", scriptName);
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
		if (!LoadScript(hash, pArgs, argCount, stackSize))
		{
			printf("Error Loading Script!\n");
			return;
		}
		pgPtrCollection<ScriptThread>* collection = ScriptEngine::GetThreadCollection();
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
			thread->Kill();
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

