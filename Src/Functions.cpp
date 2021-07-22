#include "Functions.h"



BOOL Functions::isEnabled = FALSE;
CHAR Functions::Mode = ' ';


//----------------------------------------------------
//---------------- Original Functions-----------------
//----------------------------------------------------


Functions::p_ChatFunction Functions::Org_ChatFunction = 0x0;
Functions::p_PathFunction Functions::Org_PathFunction = 0x0;
Functions::p_SendFunction Functions::Org_SendFunction = 0x0;
Functions::p_RecvFunction Functions::Org_RecvFunction = 0x0;

Functions::p_AAFunction  Functions::Org_AAPacketFunction = 0x0;
Functions::p_EntityFunction Functions::Org_EntityFunction = 0x0;
Functions::p_PickDropFunction Functions::Org_PickDropFunction = 0x0;
Functions::p_ActionFunction Functions::Org_ActionFunction = 0x0;
Functions::p_SelfSkillFunction Functions::Org_SelfSkillFunction = 0x0;
Functions::p_AutoFunction Functions::Org_AutoFunction = 0x0;
Functions::p_IsBadPtrFunction Functions::Org_IsBadPtrFunction = 0x0;
Functions::p_ReviveFunction Functions::Org_ReviveFunction = 0x0;

//----------------------------------------------------
//---------------- Static H Functions-----------------
//----------------------------------------------------

static bool __stdcall ValidateEntityAndEnqueue(LPVOID lpEntityAddress, Artifacts::sEntity* Entity)
{
	// NULL PTR
	if (lpEntityAddress == 0x00) return false;


	// CHECK IF DEAD ENTITY
	constexpr auto DeadAliveOffset = 0x70;
	auto lpStateAddress = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + DeadAliveOffset);
	auto state = memory.Read<uint8_t>(lpStateAddress);

	if (state == 0x3A)
	{
		// DEAD MOB. RETURN FALSE
		return FALSE;
	}

	// CREATE AN ENTITY STRUCT
	constexpr auto TYPEOFFSET = 0x1BC;
	auto lpTypeAddress = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + TYPEOFFSET);

	constexpr auto IDOFFSET = 0x190;
	auto lpEntityID = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + IDOFFSET);

	constexpr auto XOFFSET = 0x4;
	auto lpEntityX = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + XOFFSET);

	constexpr auto YOFFSET = 0x8;
	auto lpEntityY = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + YOFFSET);

	// HOPING A SIZE OF 4 :P
	Artifacts::Entity EntityType = memory.Read<Artifacts::Entity>(lpTypeAddress);
	auto EntityX = memory.Read<DWORD>(lpEntityX);
	auto EntityY = memory.Read<DWORD>(lpEntityY);
	auto EntityID = memory.Read<DWORD>(lpEntityID);

	constexpr auto ConversionRation = 6;

	EntityX >>= ConversionRation;
	EntityY >>= ConversionRation;

	Entity->id = EntityID;
	Entity->type = EntityType;
	Entity->x = EntityX;
	Entity->y = EntityY;
	Entity->Address = lpEntityAddress;


	

	switch (EntityType)
	{
	case Artifacts::Entity::BIRDMAN:
	case Artifacts::Entity::APPARETON:
	// case Artifacts::Entity::POLTERGEIST: SAME AS APPARETON
	case Artifacts::Entity::HAWKINGL93:
	// case Artifacts::Entity::HAWKKING: SAME AS BIRDMAN
	case Artifacts::Entity::ROBIN:
	case Artifacts::Entity::Turtledoves:
	case Artifacts::Entity::HEAVYGHOSTL23:
	case Artifacts::Entity::ICYBLADEDEVIL:
	case Artifacts::Entity::DARKELF:
	case Artifacts::Entity::INFERNALGENERAL:
	case Artifacts::Entity::DARKSOUL:
	case Artifacts::Entity::WATERDEVIL:
	case Artifacts::Entity::ICYSERPANT:
	{
		// IF THE ENTITY IS NOT IN THE VECTOR.
		if (std::find(Artifacts::AlreadyHitEntities.begin(), Artifacts::AlreadyHitEntities.end(), Entity->id) == Artifacts::AlreadyHitEntities.end())
			return true;
	}
	default:
		return FALSE;
	}

	
}

static void __stdcall AddDrop(LPVOID lpPacket)
{
	constexpr auto TypeOffset = 0x2;
	auto lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpPacket + TypeOffset);
	auto PacketType = memory.Read<uint16_t>(lpReadAddress);

	constexpr auto ValueFromRecvOffset = 0x4;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpPacket + ValueFromRecvOffset);
	auto ValueFromRecv = memory.Read<uint32_t>(lpReadAddress);

	constexpr auto IDOffset = 0x8;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpPacket + IDOffset);
	auto ID = memory.Read<Artifacts::Drop>(lpReadAddress);

	constexpr auto XOffset = 0xC;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpPacket + XOffset);
	auto X = memory.Read<uint16_t>(lpReadAddress);

	constexpr auto YOffset = 0xE;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpPacket + YOffset);
	auto Y = memory.Read<uint16_t>(lpReadAddress);

	constexpr auto OwnerIDOffset = 0x18;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpPacket + OwnerIDOffset);
	auto OwnerID = memory.Read<uint32_t>(lpReadAddress);

	Artifacts::sDrop Drop;
	
	Drop.ID = ID;
	Drop.ValueFromRecv = ValueFromRecv;
	Drop.X = X;
	Drop.Y = Y;
	Drop.OwnerID = OwnerID;


	//logger.log<TRUE>("ID: ", Drop.ID);
	// ONLY LOOT MY DROPS FOR NOW, EXCLUDE DRAGON COAT L8ER

	auto Player = memory.Read<DWORD>(artifacts.m_lpPlayerPtr);
	auto PlayerID = memory.Read<DWORD>((LPVOID)(Player + 0x190));

	 
	
	// pick items even if i didnt drop'em
	switch (Drop.ID)
	{

	case Artifacts::Drop::PHOENIXARMOR0:
	case Artifacts::Drop::PHOENIXARMOR1:
	case Artifacts::Drop::PHOENIXARMOR2:
	
	case Artifacts::Drop::PHOENIXHELMET0:
	case Artifacts::Drop::PHOENIXHELMET1:
	case Artifacts::Drop::PHOENIXHELMET2:

	case Artifacts::Drop::SHARKHAT0:
	case Artifacts::Drop::SHARKHAT1:
	case Artifacts::Drop::SHARKHAT2:

	case Artifacts::Drop::SHIELD0:
	case Artifacts::Drop::SHIELD01:
	case Artifacts::Drop::SHIELD02:

	case Artifacts::Drop::SHIELD1:
	case Artifacts::Drop::SHIELD11:
	case Artifacts::Drop::SHIELD12:

	{
		EnterCriticalSection(&DropCS);

		auto res = Artifacts::DropsID.insert(Drop.ValueFromRecv);

		if (res.second == true)
		{
			Artifacts::Drops.push_back(Drop);
		}

		LeaveCriticalSection(&DropCS);

	}

	default:
		break;

	}


		
	
	
	// PICK GOLD ONLY IF I DROPED IT..
	if ((PlayerID == Drop.OwnerID)) {
#define NOTHING 0
		switch (Drop.ID)
		{
		case (Artifacts::Drop)NOTHING:	// DISAPLE LOOTING
		//case Artifacts::Drop::GOLD:
		//default:
		

		{
			EnterCriticalSection(&DropCS);

			auto res = Artifacts::DropsID.insert(Drop.ValueFromRecv);

			if (res.second == true)
			{
				Artifacts::Drops.push_back(Drop);
			}

			LeaveCriticalSection(&DropCS);

		}

	
		}
	}


}

//----------------------------------------------------
//---------------- Callbacks -------------------------
//----------------------------------------------------

void __stdcall Functions::Cbk_ChatFunction(LPCWCHAR __Target, LPCWCHAR __ChatMsg, DWORD __Zero1, DWORD __Color, DWORD __Mode, DWORD __Zero2, DWORD __Zero3, DWORD __Zero4, DWORD __Zero5)
{
	// move ecx into __this since it's an stdcall function
	DWORD __this;
	__asm mov __this, ecx;

	auto StartMsg = L"@start";
	auto StopMsg = L"@stop";

	std::wstringstream wSS{ __ChatMsg };
	std::wstring wCommand{};

	wSS >> wCommand;
	
	auto CommandMsg = (_wcsicmp(wCommand.c_str(), StartMsg) == 0) ? isEnabled = TRUE, L"Enabled" :
		(_wcsicmp(wCommand.c_str(), StopMsg) == 0) ? isEnabled = FALSE, L"Disabled" : L"";

	if (isEnabled)
	{
		std::wstring wOption{};
		wSS >> wOption;	// GET THE OPTIONAL ARG...

		auto Warrior = L"warrior";
		auto Ninja = L"ninja";
		auto Archer = L"archer";
		auto Mage = L"mage";

		if (wOption.empty())
		{
			// DEFAULT CASE IS WARRIOR
			Mode = 'w';
		}
		else
		{
			Mode = _wcsicmp(wOption.c_str(), Warrior) == 0 ? 'w' :
				_wcsicmp(wOption.c_str(), Ninja) == 0 ? 'n' :
				_wcsicmp(wOption.c_str(), Archer) == 0 ? 'a' :
				_wcsicmp(wOption.c_str(), Mage) == 0 ? 'm' : '0';	
		}


		
	}
	

	// PROCESS THE COMMAND
	if (CommandMsg != L"")
	{
		// PATCH JNE TO JMP TO PREVENT THE MSG FROM BEING SENT TO THE SERVER
		auto lpPatchAddress = artifacts.m_ChatFunction;
		auto constexpr InstructionPatchOffset = 0xC47;
		lpPatchAddress = reinterpret_cast<LPVOID>((DWORD)lpPatchAddress + InstructionPatchOffset);

		// logger.log<TRUE>("lpPatchAddress", (DWORD)lpPatchAddress);

		// INLINE PATCH BYTE FROM JNE TO JMP
		uint8_t constexpr JMP = 0xeb;
		patch.InlinePatch(lpPatchAddress, JMP, 1);


		// DISPLAY A SYSTEM MSG
		
		constexpr auto ColorRed = 0xFFFF0000;
		constexpr auto SystemMode = 0x7D5;
		Org_ChatFunction(__this, __Target, CommandMsg, 0, ColorRed, SystemMode, 0, 0, 0, 0);

		// REMOVE THE PATCH FOR FURTHER MSG PROCESSING
		uint8_t constexpr JNE = 0x74;
		patch.InlinePatch(lpPatchAddress, JNE, 1);

		// SET ENABLED FLAG
		
	}

	else
	{
		Org_ChatFunction(__this, __Target, __ChatMsg, __Zero1, __Color, __Mode, __Zero2, __Zero3, __Zero4, __Zero5);
	}


}

bool __cdecl Functions::Cbk_EntityFunction(DWORD EntityAddress)
{
	bool res =  Functions::Org_EntityFunction(EntityAddress);
	
	// TRYING THE ARRAY METHOD INSTEAD. ONE MOB AT A TIME.

	// IF THE ADDRESS IS INVALID, NO WORK TO BE DONE.
	if (!res) return res;
	
	else
	{
		// CREATE A WORKING THREAD AT A HELPER FUNCTION
		LPVOID lpEntityAddress = 0x00;

		// THE ADDRESS IS STORED AT EDI
		__asm mov lpEntityAddress, edi;

		EnterCriticalSection(&EntityCS);

		Artifacts::sEntity Entity{};
		auto InsertEntity = ValidateEntityAndEnqueue(lpEntityAddress, &Entity);

		if (InsertEntity)
		{
			if (Artifacts::AlreadyHitEntities.size() >= Artifacts::AlreadyHitMax)
			{
				Artifacts::AlreadyHitEntities.clear();
			}

			// logger.log<TRUE>("EntID: ", Entity.id);
			Artifacts::TheEntity = Entity;

			
		}
	
		LeaveCriticalSection(&EntityCS);

		return res;

	}
	

	return res;
}

bool __stdcall Functions::Cbk_RecvFunction(LPVOID __PacketBuffer, LPVOID __Param)
{
	// TODO 
	// --Check if i own the drop


	// SETTING ORIGINAL FUNCION ARGS...
	auto constexpr RecvRelatedOffset = 0x14;
	auto lpRecvEcx = reinterpret_cast<LPVOID>((DWORD)artifacts.m_lpNetworkClass + RecvRelatedOffset);

	auto RecvEcx = memory.Read<LPVOID>(lpRecvEcx);
	
	// IF 1 THER IS A PACKET, 0 WE CAN SEND OURS
	// TODO
	//	-IMPLEMENT OURS
	auto res = Org_RecvFunction(RecvEcx, __PacketBuffer, __Param);

	if (res)
	{		
		/*
			// LOGGING PACKETS...
		auto lpType = reinterpret_cast<LPVOID>((DWORD)__PacketBuffer + 0x2);
		auto temp = memory.Read<SHORT>(lpType);
		// filter
		switch (temp)
		{
		case 0x883:
		case 0x859:
		case 0x880:
		//case 0x89d:
		//case 0x84a:
		//case 0x857:
		//case 0x8bf:
		//case 0x8a2:
		case 0x7fe:
		case 0x8b3:
		case 0x7d2:
		case 0x897:
		case 0x8d5:


			break; // do nothin
		default:

			logger.log<TRUE>("RECV: ", temp);
		}
		
		*/

		// GET PACKET TYPE
		auto constexpr TypeOffset = 0x2;
		auto lpTypeAddress = reinterpret_cast<LPVOID>((DWORD)__PacketBuffer + TypeOffset);
		auto PacketType = (Artifacts::Packet)memory.Read<uint16_t>(lpTypeAddress);

		switch (PacketType)
		{
		case Artifacts::Packet::DROP:
		{
			// IF THE VALUE IS 1, THEN ADD... 
			auto constexpr ShouldAddOffset = 0x12;
			auto lpShouldAdd = reinterpret_cast<LPVOID>((DWORD)__PacketBuffer + ShouldAddOffset);
			auto ShouldAdd = memory.Read<uint8_t>(lpShouldAdd);

			if (ShouldAdd != 1)
			{
				return res;
			}

			AddDrop(__PacketBuffer);
		}
			break;
		}
	}


	return res;

}

void Functions::Init()
{
	// USING THE GLOBAL artifacts DEFINED IN DLLMAIN

	logger.log_raw("========================\n");
	logger.log("Functions::Init");
	
	Org_ChatFunction = reinterpret_cast<p_ChatFunction>(artifacts.m_ChatFunction);
	logger.log<TRUE>("Functions::Org_ChatFunction: ", (DWORD)Org_ChatFunction);
	logger.log<TRUE>("Functions::Cbk_ChatFunction: ", (DWORD)Cbk_ChatFunction);


	Org_PathFunction = reinterpret_cast<p_PathFunction>(artifacts.m_PathFunction);
	logger.log<TRUE>("Functions::Org_PathFunction: ", (DWORD)Org_PathFunction);

	Org_EntityFunction = reinterpret_cast<p_EntityFunction>(artifacts.m_EntityFunction);
	logger.log<TRUE>("Functions::Org_EntityFunction: ", (DWORD)Org_EntityFunction);
	logger.log<TRUE>("Functions::Cbk_EntityFunction: ", (DWORD)Cbk_EntityFunction);

	Org_AAPacketFunction = reinterpret_cast<p_AAFunction>(artifacts.m_AAPacketFunction);
	logger.log<TRUE>("Functions::Org_AAPacketFunction: ", (DWORD)Org_AAPacketFunction);

	Org_SendFunction = reinterpret_cast<p_SendFunction>(artifacts.m_SendFunction);
	logger.log<TRUE>("Functions::Org_SendFunction: ", (DWORD)Org_SendFunction);

	Org_RecvFunction = reinterpret_cast<p_RecvFunction>(artifacts.m_RecvFunction);
	logger.log<TRUE>("Functions::Org_RecvFunction: ", (DWORD)Org_RecvFunction);
	logger.log<TRUE>("Functions::Cbk_RecvFunction: ", (DWORD)Cbk_RecvFunction);

	Org_PickDropFunction = reinterpret_cast<p_PickDropFunction>(artifacts.m_PickDropFunction);
	logger.log<TRUE>("Functions::Org_PickDropFunction: ", (DWORD)Org_PickDropFunction);

	Org_ActionFunction = reinterpret_cast<p_ActionFunction>(artifacts.m_ActionFunction);
	logger.log<TRUE>("Functions::Org_ActionFunction: ", (DWORD)Org_ActionFunction);

	Org_SelfSkillFunction = reinterpret_cast<p_SelfSkillFunction>(artifacts.m_SelfSkillFunction);
	logger.log<TRUE>("Functions::SelfSkillFunction: ", (DWORD)Org_SelfSkillFunction);

	Org_AutoFunction = reinterpret_cast<p_AutoFunction>(artifacts.m_AutoFunction);
	logger.log<TRUE>("Functions::AutoFunction: ", (DWORD)Org_AutoFunction);

	Org_IsBadPtrFunction = reinterpret_cast<p_IsBadPtrFunction>(artifacts.m_IsBadPtrFunction);
	logger.log<TRUE>("Functions::IsBadPtrFunction: ", (DWORD)Org_IsBadPtrFunction);

	Org_ReviveFunction = reinterpret_cast<p_ReviveFunction>(artifacts.m_ReviveFunction);
	logger.log<TRUE>("Functions::ReviveFunction: ", (DWORD)Org_ReviveFunction);
}


