#pragma once

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

#ifdef _BLES
opd_s GetScriptFromTable_t = { 0xA39218, TOC };
opd_s FileStreamOpen_t = { 0xA75990, TOC };
opd_s FileReadInts_t = { 0xA73CE8, TOC };
opd_s CloseFileStream_t = { 0xA73668, TOC };
opd_s FileReadBytes_t = { 0xA73AA8, TOC };
opd_s DecryptAES_t = { 0xA76F40, TOC };
opd_s inflateInit_t = { 0xA39ED8, TOC };
opd_s inflate_t = { 0xA3A210, TOC };
opd_s inflateEnd_t = { 0xA39EE8, TOC };
opd_s ReplaceHashes_t = { 0xA35798, TOC };
opd_s SetupScript_t = { 0xA38F40, TOC };
opd_s byteswap_ulong_t = { 0xA38D88, TOC };
opd_s LoadScript_t = { 0xA35F00, TOC };
opd_s FinishLoadingScript_t = { 0xA393F0, TOC };
opd_s KillScript_t = { 0x44A808, TOC };
#else
opd_s GetScriptFromTable_t = { 0xA39048, TOC };
opd_s FileStreamOpen_t = { 0xA757C0, TOC };
opd_s FileReadInts_t = { 0xA73B18, TOC };
opd_s CloseFileStream_t = { 0xA73498, TOC };
opd_s FileReadBytes_t = { 0xA738D8, TOC };
opd_s DecryptAES_t = { 0xA76D70, TOC };
opd_s inflateInit_t = { 0xA39D08, TOC };
opd_s inflate_t = { 0xA3A040, TOC };
opd_s inflateEnd_t = { 0xA39D18, TOC };
opd_s ReplaceHashes_t = { 0xA355C8, TOC };
opd_s SetupScript_t = { 0xA38D70, TOC };
opd_s byteswap_ulong_t = { 0xA38BB8, TOC };
opd_s LoadScript_t = { 0xA35D30, TOC };
opd_s FinishLoadingScript_t = { 0xA39220, TOC };
opd_s KillScript_t = { 0x44A678, TOC };
#endif


scoScript*(*GetScriptFromTable)(int hash) = (scoScript*(*)(int))&GetScriptFromTable_t;
int(*FileStreamOpen)(int r3, char* path, char* fileType, int r6, int r7) = (int(*)(int, char*, char*, int, int))&FileStreamOpen_t;
void(*FileReadInts)(int fileStream, void* buffer, int count) = (void(*)(int, void*, int))&FileReadInts_t;
void(*CloseFileStream)(int fileStream) = (void(*)(int))&CloseFileStream_t;
void(*FileReadBytes)(int fileStream, void* buffer, int count) = (void(*)(int, void*, int))&FileReadBytes_t;
void(*DecryptAES)(void* aesKey, void* buffer, int size) = (void(*)(void*, void*, int))&DecryptAES_t;
int(*inflateInit)(void* zlib, char* version, int size) = (int(*)(void*, char*, int))&inflateInit_t;
void(*inflate)(void* zlib, int type) = (void(*)(void*, int))&inflate_t;
void(*inflateEnd)(void* zlib) = (void(*)(void*))&inflateEnd_t;
bool(*ReplaceHashes)(void* codeSection, int codeSize) = (bool(*)(void*, int))&ReplaceHashes_t;
scoScript*(*SetupScript)(scoScript* buffer, char* scriptName, void* codeSection, int codeSize, int* globalSection, int globalCount, int* publicSection, int publicCount, int codeFlags, int signature) = (scoScript*(*)(scoScript*, char*, void*, int, int*, int, int*, int, int, int))&SetupScript_t;
void(*byteswap_ulong)(int* buffer, int count) = (void(*)(int*, int))&byteswap_ulong_t;
int(*LoadScript)(int hash, void* pArgs, int argCount, int stackSize) = (int(*)(int, void*, int, int))&LoadScript_t;
int(*FinishLoadingScript)(scoScript* buffer) = (int(*)(scoScript*))&FinishLoadingScript_t;
void(*KillScript)(ScriptThread* thread) = (void(*)(ScriptThread*))&KillScript_t;

bool InjectHash(int hash, int pointer)
{
#ifdef _BLES
	int value = (hash / *(int*)0x17D1464) * *(int*)0x17D1464;
#else
	int value = (hash / *(int*)0x17D12E4) * *(int*)0x17D12E4;
#endif
	value = (hash - value) << 3;
#ifdef _BLES
	int offset = *(int*)0x17D12E0 + value;
#else
	int offset = *(int*)0x17D12E0 + value;
#endif
	*(int*)offset = hash;
	*(int*)(offset + 4) = pointer;
#ifdef _BLES
	*(int*)0x17D146C += 1;
#else
	*(int*)0x17D12EC += 1;
#endif
	printf("Injecting Hash at: 0x%X\n", offset);
	return true;
}

char* GetScriptPointerFromPool(int hash)
{
#ifdef _BLES
	int value = (hash / *(int*)0x17D1464) * *(int*)0x17D1464;
#else
	int value = (hash / *(int*)0x17D12E4) * *(int*)0x17D12E4;
#endif
	value = (hash - value) << 3;
#ifdef _BLES
	return (char*)(*(int*)0x17D1460 + value);
#else
	return (char*)(*(int*)0x17D12E0 + value);
#endif
}

BYTE aesContext[0x1E4];

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
#ifdef _BLES
	int scoFile = FileStreamOpen(0x1844CF8, scriptPath, "sco", 0, 1);
#else
	int scoFile = FileStreamOpen(0x1844B78, scriptPath, "sco", 0, 1);
#endif
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
	int* globalBuffer = (int*)malloc(header.globalCount << 2);
	int* publicBuffer = (int*)malloc(header.publicCount << 2);
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
		memset(zlibBuffer, 0, 0x38);
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
	byteswap_ulong(globalBuffer, header.globalCount);
	byteswap_ulong(publicBuffer, header.publicCount);
	scoScript* scoBuffer = (scoScript*)malloc(sizeof(scoScript));
	SetupScript(scoBuffer, scriptName, codeBuffer, header.codeSize, globalBuffer, header.globalCount, publicBuffer, header.publicCount, header.codeFlags, header.signature);
	if (scoBuffer->codeSection[0] == 0)
	{
		printf("Error setting up the script!\n");
		free(codeBuffer);
		free(globalBuffer);
		free(publicBuffer);
		free(scoBuffer);
		return 0;
	}
	if (!InjectHash(hash, (int)scoBuffer))
		printf("Script Failed to inject!\n");
	free(codeBuffer);
	free(globalBuffer);
	free(publicBuffer);
	if (!GetScriptFromTable(hash))
		return 0;
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
	pgPtrCollection* collection = GetThreadCollection();
	for (int i = 0; i < collection->m_pCount; i++)
	{
		if (collection->m_pData[i]->m_context.scriptHash == hash)
			printf("Thread Index: %i Location: 0x%X\n", i, collection->m_pData[i]);
	}
	scoScript* script = GetScriptFromTable(hash);
	FinishLoadingScript(script);
	printf("%s Loaded from %s into this address 0x%X\n", scriptName, scriptPath, script);
}

typedef struct native_s
{
	int* retBuffer;
	int count;
	int* callBuffer;
};

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
	if (stackSize == 0)
		printf("Stack Size needs to be greater than 0!\n");
	else
		StartScriptFromHdd(scriptPath, scriptName, pArgs, argCount, stackSize);
}

//Param 1 - const char* script name
void TERMINATE_SCRIPT_FROM_HDD(native_s* call)
{
	char* scriptName = (char*)call->callBuffer[0];
	int hash = jenkinsHash(scriptName);
	ScriptThread* thread;
	pgPtrCollection* collection = GetThreadCollection();
	for (int i = 0; i < collection->m_pCount; i++)
	{
		thread = collection->m_pData[i];
		if (thread->m_context.scriptHash == hash)
		{
			KillScript(thread);
			printf("Killed Script Thread for %s\n", scriptName);
			free(GetScriptFromTable(hash));
			printf("Cleaned up memory for %s!\n", scriptName);
			char* poolAddr = GetScriptPointerFromPool(hash);
			memset(poolAddr, 0, 4);
			memset(poolAddr + 4, 0xCD, 4);
#ifdef _BLES
			*(int*)0x17D146C -= 1;
#else
			*(int*)0x17D12EC -= 1;
#endif
			printf("Removed %s from Hash Table!\n", scriptName);
			return;
		}
	}
}
