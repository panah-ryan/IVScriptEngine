#include "ScriptEngine.h"

static pgPtrCollection<ScriptThread>* scrThreadCollection;

static std::unordered_set<ScriptThread*> g_ownedThreads;

extern int ourScriptThreadId;

bool ScriptEngine::Initalize()
{
	scrThreadCollection = (pgPtrCollection<ScriptThread>*)0x828DA3C8;
	printf("Found Thread Collection %i Available Threads!\n", scrThreadCollection->count());
	return true;
}

pgPtrCollection<ScriptThread>* ScriptEngine::GetThreadCollection()
{
	return scrThreadCollection;
}

ScriptThread* ScriptEngine::GetActiveThread()
{
	return (ScriptThread*)*(int*)0x828DA3BC;
}

void ScriptEngine::SetActiveThread(ScriptThread* thread)
{
	*(int*)0x828DA3BC = (int)thread;
}

void ScriptEngine::CreateThread(ScriptThread* thread)
{
	pgPtrCollection<ScriptThread>* collection = GetThreadCollection();
	thread->exitMessage = "Normal exit.";
	thread->m_context.state = ThreadStateIdle;
	int slot = collection->count() - 1;
	thread->m_context.threadID = slot;
	ourScriptThreadId = slot;
	collection->set(slot, thread);
	g_ownedThreads.insert(thread);
	thread->Reset(slot, nullptr, 0);
	printf("Created Thread, id %i\n", thread->m_context.threadID);
}