#pragma once
#include "comms.h"

#include "Logger.h"
#include "Memory.h"
#include "Patch.h"

extern Logger logger;
extern Memory memory;
extern Patch patch;


#include "Artifacts.h"
extern Artifacts artifacts;

#include <algorithm> // std::find

extern CRITICAL_SECTION EntityCS;	// to sync objects.
extern CRITICAL_SECTION DropCS;

class Functions
{
private:
	// BOT ENABLED FLAG
	static BOOL isEnabled;

	// A-> Archers| N-> Ninja| W-> Warrior| M->Mage| ....
	static CHAR Mode;

	
	// FUNCTION POINTERS

	typedef void(__thiscall* p_ChatFunction)(DWORD __this, LPCWCHAR __Target, LPCWCHAR __ChatMsg, DWORD __Zero1,
		DWORD __Color, DWORD __Mode, DWORD __Zero2, DWORD __Zero3, DWORD __Zero4, DWORD __Zero5);

	typedef void(__thiscall* p_PathFunction)(DWORD __this, DWORD __X, DWORD __Y);

	typedef bool(__cdecl* p_EntityFunction)(DWORD EntityAddress);

	typedef bool(__thiscall* p_AAFunction)(DWORD __this, DWORD __Two, DWORD __PlayerID, DWORD __MobID, DWORD __PlayerX, DWORD __PlayerY, DWORD __Zero);

	typedef void(__thiscall* p_SendFunction)(LPVOID __this, LPVOID __lpBuffer, DWORD __Length);

	typedef bool(__thiscall* p_RecvFunction)(LPVOID __this, LPVOID __PacketBuffer, LPVOID __Param);

	typedef bool(__thiscall* p_PickDropFunction)(LPVOID __this, DWORD __ValueFromRevc, DWORD __X, DWORD __Y, DWORD __Three, DWORD __ValueSetToZero);

	typedef bool(__thiscall* p_ActionFunction)(LPVOID __this, DWORD __ID, DWORD __X, DWORD __Y, DWORD __Direction, DWORD __FiftyOne, DWORD __Action);

	typedef void(__thiscall* p_SelfSkillFunction)(LPVOID __this, Artifacts::Skill __SkillID, DWORD __PlayerID, DWORD __Zero, DWORD __One);

	typedef void(__thiscall* p_AutoFunction)(LPVOID __this, LPVOID __pEntity, LPVOID __pAutoVTable, DWORD __Zero, DWORD __One);

	typedef bool(__cdecl* p_IsBadPtrFunction)(LPVOID __Ptr);

	typedef bool(__thiscall* p_ReviveFunction)(LPVOID __this, DWORD __PlayerPtr, DWORD __Zero1, DWORD __Zero2, DWORD __Zero3, DWORD __x5E, BYTE __Mode, DWORD __Zero4);


public:
	
	// FUNCTION CALLBACKS

	static void __stdcall Cbk_ChatFunction(LPCWCHAR __Target, LPCWCHAR __ChatMsg, DWORD __Zero1,
		DWORD __Color, DWORD __Mode, DWORD __Zero2 = 0, DWORD __Zero3 = 0, DWORD __Zero4 = 0, DWORD __Zero5 = 0);
	
	static bool __cdecl Cbk_EntityFunction(DWORD EntityAddress);
	
	static bool __stdcall Cbk_RecvFunction(LPVOID __PacketBuffer, LPVOID __Param);


public:

	Functions()
	{
		Init();
	}

	void Init();

	inline BOOL isEnable() const { return isEnabled; }
	inline CHAR GetMode() const { return Mode; }


	
	// ORIGINAL FUNCTIONS

	static p_ChatFunction Org_ChatFunction;
	static p_PathFunction Org_PathFunction;
	static p_SendFunction Org_SendFunction;
	static p_RecvFunction Org_RecvFunction;
	static p_AAFunction Org_AAPacketFunction;
	static p_EntityFunction Org_EntityFunction;
	static p_PickDropFunction Org_PickDropFunction;
	static p_ActionFunction Org_ActionFunction;
	static p_SelfSkillFunction Org_SelfSkillFunction;
	static p_AutoFunction Org_AutoFunction;
	static p_IsBadPtrFunction Org_IsBadPtrFunction;
	static p_ReviveFunction Org_ReviveFunction;


	

};




