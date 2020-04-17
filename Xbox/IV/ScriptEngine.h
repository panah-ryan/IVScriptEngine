#pragma once

#include "stdafx.h"
#include "pgCollection.h"
#include "ScriptThread.h"

#ifndef __SCRIPT_ENGINE_H__
#define __SCRIPT_ENGINE_H__

class ScriptEngine
{
public:
	static bool Initalize();
	static pgPtrCollection<ScriptThread>* GetThreadCollection();
	static ScriptThread* GetActiveThread();
	static void SetActiveThread(ScriptThread* thread);
	static void CreateThread(ScriptThread* thread);
};

#endif

