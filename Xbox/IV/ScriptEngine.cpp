#include "ScriptEngine.h"

static pgPtrCollection<ScriptThread>* scrThreadCollection;
static short scrThreadId;
static short scrThreadCount;

static std::unordered_set<ScriptThread*> g_ownedThreads;

bool ScriptEngine::Initalize()
{
	scrThreadCollection = (pgPtrCollection<ScriptThread>*)0x83192788;
	printf("Found Thread Collection %i Available Threads!\n", scrThreadCollection->count());
	return true;
}

pgPtrCollection<ScriptThread>* ScriptEngine::GetThreadCollection()
{
	return scrThreadCollection;
}

ScriptThread* ScriptEngine::GetActiveThread()
{
	return (ScriptThread*)*(int*)0x8319277C;
}

void ScriptEngine::SetActiveThread(ScriptThread* thread)
{
	*(int*)0x8319277C = (int)thread;
}

void ScriptEngine::CreateThread(ScriptThread* thread)
{
	pgPtrCollection<ScriptThread>* collection = GetThreadCollection();
	thread->exitMessage = "Normal exit.";
	thread->m_context.state = ThreadStateIdle;
	int slot = collection->count() - 1;
	thread->m_context.threadID = slot;
	collection->set(slot, thread);
	g_ownedThreads.insert(thread);
	thread->Reset(slot, nullptr, 0);
	printf("Created Thread, id %i\n", thread->m_context.threadID);
}
