#include "Bot.h"


 //INTERFACE

Bot::Bot(Artifacts* artifacts, Functions* functions, Hooks* hooks)
	:m_Artifacts{ artifacts }, m_Functions{ functions }, m_Hooks{ hooks }
{
	logger.log_raw("\n==================\n");
	logger.log("Bot::Bot");

	logger.log<TRUE>("Bot::m_Artifacts: ", (DWORD)m_Artifacts);
	logger.log<TRUE>("Bot::m_Functions: ", (DWORD)m_Functions);
	logger.log<TRUE>("Bot::m_Hooks: ", (DWORD)m_Hooks);


	Init();

}


// STATIC HELPERS
static void Test(Bot* __this)
{
	// TESTING PATHFINDER
	constexpr auto X = 531;
	constexpr auto Y = 298;

	__this->PathFinder(X, Y);
	

	logger.log("Im done");
}

static double Distance(POINT p1, POINT p2)
{
	// CALCULATES THE DISTANCE BETWEEN TWO ""FIXED"" POINTS
	auto XDiff = abs(p1.x - p2.x);
	auto YDiff = abs(p1.y - p2.y);

	auto distance = sqrt(XDiff * XDiff + YDiff * YDiff);
	return distance;
}

static inline uint8_t GetState(LPVOID lpPlayer)
{
	auto Player = memory.Read<DWORD>((lpPlayer));
	constexpr auto StateOffset = 0x70;

	auto state = memory.Read<uint8_t>((LPVOID)(Player + StateOffset));

	return state;
}

static inline void AddToAlreadyHit(uint32_t id)
{
	EnterCriticalSection(&EntityCS);

	Artifacts::AlreadyHitEntities.push_back(id);

	LeaveCriticalSection(&EntityCS);
}

static inline bool IsSuperActive(LPVOID lpPlayer)
{
	auto Player = memory.Read<DWORD>(lpPlayer);
	constexpr auto SuperOffset = 0x878;
	auto lpAddress = reinterpret_cast<LPVOID>(Player + SuperOffset);

	auto Super = memory.Read<uint8_t>(lpAddress);

	if (Super == 100)
	{
		return TRUE;
	}

	return FALSE;
}

static inline void SetSpeed(LPVOID lpPlayer)
{
	auto Player = memory.Read<DWORD>(lpPlayer);
	constexpr auto SpeedOffset = 0x103;

	auto lpStateAddress = reinterpret_cast<LPVOID>(Player + SpeedOffset);
	constexpr auto MaxSpeed = 0x04;

	memory.Write<BYTE, FALSE>(lpStateAddress, MaxSpeed);
	
}


// MAIN FUNCTIONALITY
void Bot::Run()
{
	std::cout << "\nMode: " << this->m_Functions->GetMode() << '\n';

label_UpdateEntity:

	
	Sleep(50);


	if (IsDead())
	{
		Resurrect();
	}

	
	Mode = this->m_Functions->GetMode();

	if (this->m_Functions->isEnable() == FALSE)
	{
		return;
	}

	// ACTIVATE SUPER
	CastSuper(Mode);

	
	// SETTING MAX SPEED WHILE IN ULT
	if (isUlti && !IsSuperActive(this->m_Artifacts->m_lpPlayerPtr))
	{
		SetSpeed(this->m_Artifacts->m_lpPlayerPtr);
		isUlti = FALSE;

		using clock_t = std::chrono::high_resolution_clock;
		m_UltBeginTimer = clock_t::now();

	}

	// GET NEXT MOB TO ATTACK
	SetEntities();

	if (this->m_TheEntity.id == 0x00)
	{
		std::cout << "\nNOPE\n";
		goto label_UpdateEntity;
	}

	std::cout << "\nAttacking... ";

	AttackEntity(&this->m_TheEntity, Mode);

	


	if (Artifacts::Drops.size() != 0) {

		if(Mode == 'n')
			Sleep(750); // for ninja

		std::cout << "\nClearing\n";
		
		ClearMapDrops();

	}

	
	

	goto label_UpdateEntity;


	

}

void Bot::PathFinder(DWORD x, DWORD y)
{

	auto Player = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);
	if (Player == 0x00) return;
	this->m_Functions->Org_PathFunction(Player, x, y);

}

void Bot::AttackEntity(Artifacts::sEntity* pEntity, CHAR mode)
{
	
	// GO TO ENTITY
	std::cout << "EntityID: " << pEntity->id << ", X: " << pEntity->x << ", Y: " << pEntity->y ; // std::endl;
	
	// LOOKS LIKE AS SOON AS THE PATHFINDER IS FIRED, THE PLAYER COORDS ARE AUTOMATICALLY UPDATED
	// JUST GOES THROUGH ANIMATION.
	POINT Ent{ pEntity->x, pEntity->y };

	// COMMENT OUT WHILE LOOP TO ACHIEVE MAX SPEED. ONLY FOR LVLING
	auto constexpr RandomDistance = 1.001f;
																								 															 
	// calculate time for Ult
	if (Mode == 'n') {
		
		using clock_t = std::chrono::high_resolution_clock;
		auto time = clock_t::now();
		auto delta = time - m_UltBeginTimer;
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(delta);
		constexpr auto UltDuration = 57; // Seconds


		if (duration.count() <= UltDuration) {
			Sleep(100);	// TONE DOWN ULT 200 is too much
			goto lable_NinjaSkip;
		}
		else
			// RESET THE TIMER
			m_UltBeginTimer = {};


	}
	
	//
	PathFinder(pEntity->x, pEntity->y); 



	// FOR NINJA
	while (Distance(Ent) >= RandomDistance)
	{

		if (!this->m_Functions->isEnable())
		{
			return;
		}

		Sleep(20);


		if (IsDead())
		{
			Resurrect();
			PathFinder(pEntity->x, pEntity->y);
		}

		// ANTI-STUCk?
		auto state = GetState(this->m_Artifacts->m_lpPlayerPtr);

		if (state == 0xA || state == 0x1E)
		{
			// BEING ATTACKED, RE-PATH FIND.
			PathFinder(pEntity->x, pEntity->y);
		}
	}
	
	
	std::cout << ", Distance: " << Distance(Ent);
	std::cout << ", VecSize: " << Artifacts::AlreadyHitEntities.size() << std::endl;

lable_NinjaSkip:
	// CREATE THE BUFFER
	auto AAPacketBuffer = reinterpret_cast<DWORD>(this->m_Artifacts->m_AABuffer);

	
	auto PlayerID = GetPlayerID();
	
	if (PlayerID == 0x00)
	{
		logger.log_error("Bot::AttackEntity PlayerID is Zero.");
		return;
	
	}


	POINT PlayerCoords;
	GetPlayerCoords(&PlayerCoords);
	auto res = this->m_Functions->Org_AAPacketFunction(AAPacketBuffer, 2, PlayerID, pEntity->id, PlayerCoords.x, PlayerCoords.y, 0);

	if (res == 0)
	{
		// ?
		return;
	}

	// SEND THE PACKET
	constexpr auto SkipDword = 0x4;

	auto lpPacket = reinterpret_cast<LPVOID>((DWORD)AAPacketBuffer + SkipDword);
	auto PacketLength = memory.Read<SHORT>(lpPacket);

	
	auto NetworkClass = this->m_Artifacts->m_lpNetworkClass;

	if (IsDead())
	{
		Resurrect();
		AddToAlreadyHit(this->m_TheEntity.id);
		return;
	}
	
	this->m_Functions->Org_SendFunction(NetworkClass, lpPacket, PacketLength);

	//// TRYING NEW FUNCTION...


	//// CHECK ENTITY

	//auto res = Functions::Org_IsBadPtrFunction(pEntity->Address);

	//if (res == 0)
	//{
	//	Sleep(200);
	//	// INVALID ENTITY PTR, RETURN
	//	return;
	//}


	//auto Player = memory.Read<LPVOID>(this->m_Artifacts->m_lpPlayerPtr);
	//this->m_Functions->Org_AutoFunction(Player, pEntity->Address, &this->m_Artifacts->m_AutoVTable, 0, 1);

	AddToAlreadyHit(this->m_TheEntity.id);

	Sleep(50);
	//UpdatePlayerCoords(&Ent);


}


LPVOID Bot::GetNextEntity()
{
	auto constexpr NextEntityOffset = 0x11B8;
	
	if (this->m_Artifacts->m_lpEntityList == 0x00)
	{
		// INITIALIZE ENTITY LIST
		this->m_Artifacts->GetEntityList();
	}

	
	auto constexpr MaxEntities = 5;
	for (int i{}; i < MaxEntities; i++)
	{
		auto lpCurrentEntity = reinterpret_cast<LPVOID>((DWORD)this->m_Artifacts->m_lpEntityList + i * NextEntityOffset);
		
		auto temp = memory.Read<DWORD>((LPVOID)((DWORD)lpCurrentEntity + 0x10));

		if (temp == 0)
		{
			return 0x00;
		}

		if (IsMob(lpCurrentEntity))
		{
			return lpCurrentEntity;
		}

		Sleep(10);
	}

	// NO "SUITABLE" ENTITY FOUND.
	return 0x00;
}

// HELPER FUNCTIONS

void Bot::PatchPathFunctionInstruction()
{
	/*
		Patches instruction at offset 0x4D from PathFunction beginning.
		The instuction is 6 bytes. It should be NOP'ed for the PathFunction to work
		automatically
	*/

	constexpr auto PatchInstructionOffset = 0x4D;
	auto lpPatchAddress = reinterpret_cast<LPVOID>((DWORD)m_Artifacts->m_PathFunction + PatchInstructionOffset);

	// NOP the address
	constexpr auto NOP = 0x90;
	constexpr auto Size = 6;
	patch.InlinePatch(lpPatchAddress, NOP, Size);

}

bool Bot::IsMob(LPVOID lpEntity)
{
	if (lpEntity == 0x00) return FALSE;

	// VALIDATE WHETHER IT'S A VALID ENTITY PTR.
	auto MagicDword = memory.Read<DWORD>(lpEntity);

	// MAGICDWORD SHOULD BE EQUAL TO m_Artifacts->m_lpEntityVTable
	if (MagicDword != (DWORD)m_Artifacts->m_lpEntityVTable)
	{
		return FALSE;
	}

	// CHECK IF THE MOB IS ALIVE
	// IF THE MOB IS DEAD, THE BYTE VALUE IS 0x5A
	constexpr auto DeadAliveOffset = 0x70;
	auto lpStateAddress = reinterpret_cast<LPVOID>((DWORD)lpEntity + DeadAliveOffset);
	auto state = memory.Read<uint8_t>(lpStateAddress);

	if (state == 0x3A)
	{
		// DEAD MOB. RETURN FALSE
		return FALSE;
	}

	std::cout << std::hex << "\nState: 0x" << (int)state << '\n';

	auto constexpr TYPEOFFSET = 0x1BC;
	auto lpTypeAddress = reinterpret_cast<LPVOID>((DWORD)lpEntity + TYPEOFFSET);

	auto EntityType = memory.Read<Artifacts::Entity>(lpTypeAddress);
	
	switch (EntityType)
	{
		// TWINCITY MOBS
	case Artifacts::Entity::APPARETON:

		// BIRD-ISLAND MOBS
	case Artifacts::Entity::BIRDMAN:
	case Artifacts::Entity::HAWKINGL93:
	//	case Artifacts::Entity::HAWKKING: HAS THE SAME NUMERIC TYPE AS BIRDMAN

		return TRUE;

		// NOT SUPPORTED TYPE YET.
	default:
		return FALSE;

	}
}

void Bot::MakeEntityStruct(LPVOID lpEntityAddress, Artifacts::sEntity* lpEntity)
{
	constexpr auto TYPEOFFSET = 0x1BC;
	auto lpTypeAddress = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + TYPEOFFSET);

	constexpr auto IDOFFSET = 0x190;
	auto lpEntityID = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + IDOFFSET);

	constexpr auto XOFFSET = 0x4;
	auto lpEntityX = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + XOFFSET);

	constexpr auto YOFFSET = 0x8;
	auto lpEntityY = reinterpret_cast<LPVOID>((DWORD)lpEntityAddress + YOFFSET);

	// HOPING A SIZE OF 4 :P
	auto EntityType = memory.Read<Artifacts::Entity>(lpTypeAddress);
	auto EntityX = memory.Read<DWORD>(lpEntityX);
	auto EntityY = memory.Read<DWORD>(lpEntityY);
	auto EntityID = memory.Read<DWORD>(lpEntityID);

	constexpr auto ConversionRatio = 6;

	lpEntity->id = EntityID;
	lpEntity->type = EntityType;
	lpEntity->x = (uint16_t)(EntityX >> ConversionRatio);
	lpEntity->y = (uint16_t)(EntityY >> ConversionRatio);

}

void Bot::GetPlayerCoords(POINT* p)
{
	// GET PLAYER COORDS
	// 0x32c and 0x330 are changed immediately once the PathFinder is initiated
	constexpr auto XOFFSET = 0x32C; 
	constexpr auto YOFFSET = 0x330;

	auto PlayerAddress = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);
	auto XAddress = reinterpret_cast<LPVOID>(PlayerAddress + XOFFSET);
	auto YAddress = reinterpret_cast<LPVOID>(PlayerAddress + YOFFSET);

	auto PlayerX = memory.Read<DWORD>(XAddress);
	auto PlayerY = memory.Read<DWORD>(YAddress);

	p->x = PlayerX;
	p->y = PlayerY;
}

inline void Bot::UpdatePlayerCoords(POINT* p)
{
	// 0x32c and 0x330 are changed immediately once the PathFinder is initiated
	constexpr auto XOFFSET = 0x4;
	constexpr auto YOFFSET = 0x8;

	auto PlayerAddress = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);
	auto XAddress = reinterpret_cast<LPVOID>(PlayerAddress + XOFFSET);
	auto YAddress = reinterpret_cast<LPVOID>(PlayerAddress + YOFFSET);

	// CONVERSION RATIO...
	memory.Write<DWORD, FALSE>(XAddress, p->x << 6);
	memory.Write<DWORD, FALSE>(YAddress, p->y << 6);

}

inline DWORD Bot::GetPlayerID()
{
	auto PlayerAddress = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);
	
	if (PlayerAddress == 0x00)
	{
		return 0x00;
	}

	constexpr auto IDOffset = 0x190;
	auto lpIDAddress = reinterpret_cast<LPVOID>(PlayerAddress + IDOffset);
	
	auto PlayerID = memory.Read<DWORD>(lpIDAddress);

	return PlayerID;
}

bool Bot::OkToGo(POINT p)
{
	// GET PLAYER COORDS...
	auto Player = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);


	// NOTE: THESE OFFSETS MUST BE USED INSTEAD OF 330, 32c as they are
	// CHANGED INCREMENTALLY.
	auto constexpr XOFFSET = 0x4;
	auto constexpr YOFFSET = 0x8;

	auto x = (float)(memory.Read<DWORD>((LPVOID)(Player + XOFFSET)) >> 6);
	auto y = (float)(memory.Read<DWORD>((LPVOID)(Player + YOFFSET)) >> 6);


	/*if (x != p.x || y != p.y)
	{
		return FALSE;
	}*/

	auto d = Distance(p);
	if (d >= 0.001f)
	{
	
		return FALSE;
	}

	

	
	return TRUE;

	
}


float Bot::Distance(POINT p)
{
	// CALCULATES THE DISTANCE BETWEEN THE PLAYER AND A FIXED POINT.
	// THE PLAYER POINT IS CHANGING.
	auto Player = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);
	

	// NOTE: THESE OFFSETS MUST BE USED INSTEAD OF 330, 32c as they are
	// CHANGED INCREMENTALLY.
	auto constexpr XOFFSET = 0x4;
	auto constexpr YOFFSET = 0x8;

	auto x = (float)(memory.Read<DWORD>((LPVOID)(Player + XOFFSET)) >> 6);
	auto y = (float)(memory.Read<DWORD>((LPVOID)(Player + YOFFSET)) >> 6);


	auto XDiff = abs(x - p.x);
	auto YDiff = abs(y - p.y);

	auto distance = sqrt(XDiff * XDiff + YDiff * YDiff);

	// std::cout << "\nDistance: " << distance << '\n';

	return distance;
}

inline void Bot::SetEntities()
{
	
	auto res = std::find(Artifacts::AlreadyHitEntities.begin(), Artifacts::AlreadyHitEntities.end(), Artifacts::TheEntity.id) == Artifacts::AlreadyHitEntities.end();
	if (res)
	{
		EnterCriticalSection(&EntityCS);
		
		// THE ENTITY WAS NOT HIT BEFORE.
		this->m_TheEntity = Artifacts::TheEntity;
		
		LeaveCriticalSection(&EntityCS);
	}
	else
	{
		logger.log<TRUE>("HitBefore: ", Artifacts::TheEntity.id);
		this->m_TheEntity.id = 0x00;
	}

	
}

void Bot::ClearMapDrops()
{
	EnterCriticalSection(&DropCS);

	this->m_Drops = Artifacts::Drops;
	
	LeaveCriticalSection(&DropCS);

	for (auto& Drop : m_Drops)
	{

		if (!this->m_Functions->isEnable())
		{
			goto Clear_N_Exit;
		}

		logger.log<TRUE>("LootID: ", Drop.ID);


		// PATHFIND TO DROP
		PathFinder(Drop.X, Drop.Y);

		// W8 TILL I REACH
		POINT D{ Drop.X, Drop.Y };

		auto Player = memory.Read<DWORD>(this->m_Artifacts->m_lpPlayerPtr);

		auto lpState = reinterpret_cast<LPVOID>(Player + 0x70);
		auto state = memory.Read<uint32_t>(lpState);



		while (!OkToGo(D))
		{
			if (!this->m_Functions->isEnable())
			{
				goto Clear_N_Exit;
			}

			if (IsDead())
			{
				Resurrect();
				PathFinder(D.x, D.y);
			}

			Sleep(20);
			PathFinder(D.x, D.y);

		}



		// CALL ORG_MAKEPACKET



		// Sit();



		auto PickDropBuffer = new Artifacts::sPickDropBuffer{ (DWORD)this->m_Artifacts->m_lpPickDropDword };
		
		this->m_Functions->Org_PickDropFunction((LPVOID)PickDropBuffer, Drop.ValueFromRecv, Drop.X, Drop.Y, 3, 0);

		// SEND THE PACKET

		constexpr auto SkipDword = 0x4;

		auto lpPacket = reinterpret_cast<LPVOID>((DWORD)PickDropBuffer + SkipDword);
		auto PacketLength = memory.Read<SHORT>(lpPacket);

		auto NetworkClass = this->m_Artifacts->m_lpNetworkClass;




		this->m_Functions->Org_SendFunction(NetworkClass, lpPacket, PacketLength);


		delete PickDropBuffer;
	}

Clear_N_Exit:

	this->m_Drops.clear();

	EnterCriticalSection(&DropCS);

	

	Artifacts::Drops.clear();
	Artifacts::DropsID.clear();

	LeaveCriticalSection(&DropCS);


	return;


}

void Bot::Sit()
{
	// NOTE: Sit uses the same structure as AAPacket with different DWORDS
	// TODO:
		// Programatically fnd dword if it works :P [ not working ]

	auto PacketBuffer = new Artifacts::sAABuffer{ 0x00F6FEE0, 0x00F6FEF8 };
	auto Player = GetPlayerID();
	POINT p;
	GetPlayerCoords(&p);
	Functions::Org_ActionFunction(PacketBuffer, Player, p.x, p.y, 1, 51, 0xFA);

	// PREPARE THE MSG
	auto lpBuffer = reinterpret_cast<LPVOID>((DWORD)PacketBuffer + 0x4);
	auto length = memory.Read<SHORT>(lpBuffer);

	Functions::Org_SendFunction(this->m_Artifacts->m_lpNetworkClass, lpBuffer, length);



}

void Bot::CastSuper(CHAR mode)
{
	if (IsSuperActive(this->m_Artifacts->m_lpPlayerPtr))
	{
		auto lpPlayer = this->m_Artifacts->m_lpPlayerPtr;
		auto Player = memory.Read<LPVOID>(lpPlayer);
		auto PlayerID = GetPlayerID();
		Artifacts::Skill Skill{};

		// PICKING SUPER BASED ON THE HERO
		switch (mode)
		{
		case 'w':
			Skill = Artifacts::Skill::SUPERMAN;
			break;

		case 'n':
			Skill = Artifacts::Skill::FATALSTRIKE;
			break;
		default:
			isUlti = FALSE;
			return;
		}
		
		Functions::Org_SelfSkillFunction(Player, Skill, PlayerID, 0, 1);

		isUlti = TRUE;
	}
}

inline bool Bot::IsDead()
{
	auto State = GetState(this->m_Artifacts->m_lpPlayerPtr);

	if (State == 0x3A)
	{
		return TRUE;
	}

	return FALSE;
}

void Bot::Resurrect()
{
	// 30 Seconds
#define REVIVE_WAIT_DURATION 30000

	Sleep(REVIVE_WAIT_DURATION);

	// PREPARE PACKET
	auto PlayerID = GetPlayerID();
	constexpr auto _x5E = 0x5E;
	BYTE ReviveHere = 1;	// REVIVE = 0
	auto Buffer = this->m_Artifacts->m_ReviveBuffer;

	Functions::Org_ReviveFunction(Buffer, PlayerID, 0, 0, 0, _x5E, ReviveHere, 0);

	// SEND THE PACKET
	constexpr auto SkipDword = 0x4;
	auto Packet = reinterpret_cast<LPVOID>((DWORD)Buffer + SkipDword);
	auto length = memory.Read<SHORT>(Packet);
	auto NetworkClass = this->m_Artifacts->m_lpNetworkClass;

	Functions::Org_SendFunction(NetworkClass, Packet, length);


}






// IMPLEMENTATION

void Bot::Init()
{
	Mode = this->m_Functions->GetMode();

	// SetUp PathFinder.
	PatchPathFunctionInstruction();
	logger.log_idented<4>("PatchPathFunction installed.");

}
