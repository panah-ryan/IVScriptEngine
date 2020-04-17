#include "ScriptThread.h"
#include "ScriptEngine.h"

eThreadState ScriptThread::Tick(int opsToExecute)
{
	return ((eThreadState(*)(ScriptThread*, int))0x82553C10)(this, opsToExecute);
}

void ScriptThread::Kill()
{
	((void(*)(ScriptThread*))0x825E7C48)(this);
}

eThreadState ScriptThread::Run(int opsToExecute)
{
	ScriptThread* activeThread = ScriptEngine::GetActiveThread();
	ScriptEngine::SetActiveThread(this);
	if (m_context.state != ThreadStateKilled)
		DoRun();
	ScriptEngine::SetActiveThread(activeThread);
	return m_context.state;
}

eThreadState ScriptThread::Reset(int scriptHash, void* pArgs, int argCount)
{
	this->m_context.threadID = scriptHash;
	this->m_context.state = ThreadStateIdle;
	return m_context.state;
}

extern "C" ScriptThread* ScriptThread::GetMainScriptThread()
{
	return this;
}

extern "C" unsigned int jenkinsHash(char *key)
{
	size_t len = strlen(key);
	unsigned int hash, i;
	for (hash = i = 0; i < len; ++i)
	{
		hash += tolower(key[i]);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}