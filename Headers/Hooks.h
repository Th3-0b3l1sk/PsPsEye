#pragma once
#include "comms.h"

#include "Logger.h"
#include "Memory.h"
#include "Patch.h"
#include "Functions.h"
#include "Artifacts.h"

extern Logger logger;
extern Memory memory;
extern Patch patch;

class Hooks
{
public:
	Hooks()
	{
		Init();
	}

private:
	void Init();
	bool HookChatFunction();
	bool HookEntityFunction();
	bool HookRecvFunction();

};