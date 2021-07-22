#pragma once
#include "comms.h"
#include "Logger.h"
#include "Patch.h"
#include "Memory.h"

extern Logger logger;
extern Patch patch;
extern Memory memory;

#include "Artifacts.h"
#include "Hooks.h"
#include "Functions.h"

extern CRITICAL_SECTION EntityCS;
extern CRITICAL_SECTION DropCS;

class Bot
{

	// MEMBERS
private:
	Artifacts* m_Artifacts = 0x00;
	Functions* m_Functions = 0x00;
	Hooks* m_Hooks = 0x00;

	

	Artifacts::sEntity m_TheEntity;
	
	std::vector<Artifacts::sDrop> m_Drops;
	
	BOOL isUlti = FALSE;

	std::chrono::steady_clock::time_point m_UltBeginTimer;
	

	// INTERFACE
public:
	Bot() = delete;
	Bot(Artifacts* artifacts, Functions* functions, Hooks* hooks);
	
	void Run();

	// BOT BEHAVIOUR
	void PathFinder(DWORD x, DWORD y);
	void AttackEntity(Artifacts::sEntity* pEntity, CHAR mode);
	LPVOID GetNextEntity();

	

	// IMPLEMENTATION
private:

	CHAR Mode;

	// SETS UP NECESSARY PATCHES FOR THE BOT TO WORK CORRECTLY
	void Init();
	


	// STATIC HELPER
	void inline PatchPathFunctionInstruction();
	bool inline IsMob(LPVOID lpEntity);
	bool inline OkToGo(POINT p);
	void inline MakeEntityStruct(LPVOID lpEntityAddress, Artifacts::sEntity* lpEntity);
	void inline GetPlayerCoords(POINT* p);	
	void inline UpdatePlayerCoords(POINT* p);
	DWORD inline GetPlayerID();
	float inline Distance(POINT p);
	void inline SetEntities();
	void ClearMapDrops();
	void Sit();
	void CastSuper(CHAR mode);
	bool inline IsDead();
	void Resurrect();
	

};