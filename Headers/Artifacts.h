#pragma once
#include "comms.h"
#include "Logger.h"
#include "Memory.h"


class Artifacts
{
	friend class Bot;	// TO CALL GetEntityList();

public:
	Artifacts();

	enum class Entity
	{
		// TWINCITY

		Turtledoves = 0x0130,

		APPARETON = 0xE9,
		ROBIN = 0x84,

		POLTERGEIST = 0xE9,
		HEAVYGHOSTL23 = 0x01fc,

		// BIRDISLAND
		BIRDMAN = 0xCD,
		HAWKKING = 0xCD,
		HAWKINGL93 = 0x5D,


		// FROZEN 
		ICYBLADEDEVIL = 0x149,
		DARKELF = 0x148,
		ICYSERPANT = 0xFF,

		//DRAGON ISLAND?
		DARKSOUL = 0x146,
		INFERNALGENERAL = 0xA8,

		//DEITY LAND
		WATERDEVIL = 0x393


	};

	enum class Packet
	{
		DROP = 0x874
	};

	enum class Drop
	{
		GOLD =  0x10A1E4,
#pragma region items
		/*
		 if the ID - base <= 0x2 then its of the required type.
		*/
		// ARMORS [bases]
		PHOENIXARMOR0 = 0x20015,
		PHOENIXARMOR1 = 0x20016,
		PHOENIXARMOR2 = 0x20017,
		
		// HELMETS
		PHOENIXHELMET0 = 0x1B1F5,
		PHOENIXHELMET1 = 0x1B1F6,
		PHOENIXHELMET2 = 0x1B1F7,

		SHIELD0 = 0xDBC05,
		SHIELD01 = 0xDBC06,
		SHIELD02 = 0xDBC07,

		SHIELD1 = 0xDBBFB,
		SHIELD11 = 0xDBBFC,
		SHIELD12 = 0xDBBFD,

		SHARKHAT0 = 0x1B9C5,
		SHARKHAT1 = 0x1B9C6,
		SHARKHAT2 = 0x1B9C7
#pragma endregion
		

	};

	enum class Skill
	{
		SUPERMAN = 0x401,
		FATALSTRIKE = 0x177B
	};




	struct sEntity
	{
		Entity type;
		uint32_t id;
		uint16_t x;
		uint16_t y;
		LPVOID Address;

		bool operator==(const sEntity& rhs) const
		{
			return this->id == rhs.id;
		}
	};

	struct sDrop
	{
		DWORD ValueFromRecv;
		Artifacts::Drop ID;
		SHORT X;
		SHORT Y;
		DWORD OwnerID;


		bool operator==(const sDrop& rhs) const
		{
			return this->ValueFromRecv == rhs.ValueFromRecv;
		}
	};


	struct sAABuffer_Helper
	{
		DWORD SecondDword;
		BYTE Filler[0x200];
	};

	struct sAABuffer
	{
		DWORD FirstDword;
		BYTE Filler[0x400];	// NOT ACTUALLY A FILLER, ITS THE SERIALIZED MSG BUFFER.
		sAABuffer_Helper* pBuffer;

		sAABuffer(DWORD First, DWORD Second)
		{
			FirstDword = First;
			pBuffer = new sAABuffer_Helper{};
			memset(Filler, 0, 0x400);
			
			pBuffer->SecondDword = Second;
			memset(pBuffer->Filler, 0, 0x200);
		}

		~sAABuffer()
		{
			delete pBuffer;
		}

	};


	struct sPickDropBuffer
	{
		DWORD OnlyDword{};
		BYTE Filler[0x400]{};
		BYTE* pBuffer{};

		sPickDropBuffer(DWORD __dword)
		{
			OnlyDword = __dword;
			pBuffer = Filler;	// ADDRESS OF FIRST BYTE IN SERIALIZED MSG.
			
		}

	};

	// DATA MEMBERS
public:
	LPVOID m_lpPlayerPtr = 0x0;
	LPVOID m_lpNetworkClass = 0x0;
	LPVOID m_lpEntityVTable = 0x0;
	LPVOID m_lpEntityList = 0x0;
	LPVOID m_lpAAPacketFirstDword = 0x0;
	LPVOID m_lpAAPacketSecondDword = 0x0;
	LPVOID m_lpPickDropDword = 0x0;
	LPVOID m_AutoVTable = 0x0;
	LPVOID m_lpReviveFirstDword = 0x0;
	LPVOID m_lpReviveSecondDword = 0x0;

	sAABuffer* m_AABuffer = 0x0;
	sAABuffer* m_ReviveBuffer = 0x0;
	sPickDropBuffer* m_PickDropBuffer = 0x0;

	// FOR MOBS...
	static std::vector<uint32_t> AlreadyHitEntities;
	static uint32_t AlreadyHitMax;

	// FOR DROPS...
	static std::vector<sDrop> Drops;
	static std::unordered_set<DWORD> DropsID;
	

	// FUNCTIONS
public:
	LPVOID m_SendFunction = 0x0;
	LPVOID m_RecvFunction = 0x0;
	LPVOID m_ChatFunction = 0x0;
	LPVOID m_PathFunction = 0x0;
	LPVOID m_AAPacketFunction = 0x0;
	LPVOID m_EntityFunction = 0x0;
	LPVOID m_PickDropFunction = 0x0;
	LPVOID m_ActionFunction = 0x0;
	LPVOID m_SelfSkillFunction = 0x0;
	LPVOID m_AutoFunction = 0x0;
	LPVOID m_IsBadPtrFunction = 0x0;
	LPVOID m_ReviveFunction = 0x0;

	

	// PRIVATE FUNCTIONALITY
private:
	BOOL GetPlayerAddress();
	BOOL GetNetworkClass();
	BOOL GetEntityVTable();
	BOOL GetEntityList();
	BOOL GetAAPacketDwords();
	BOOL GetReviveDword();
	BOOL GetPickDropDword();
	BOOL GetAutoVTable();


	// ALL THESE FUNCTIONS ARE PACKET BUILDERS.
	// WELL MAYBE SOME I CANT REMEMBER.
	BOOL GetSendFunction();
	BOOL GetRecvFunction();
	BOOL GetChatFunction();
	BOOL GetPathFunction();
	BOOL GetAAPacketFunction();
	BOOL GetEntityFunction();
	BOOL GetPickDropFunction();
	BOOL GetActionFunction();
	BOOL GetSelfSkillFunction(); // NOT A PACKET BUILDER, THE ACTUAL FUNCTION.
	BOOL GetAutoFunction();
	BOOL GetIsBadPtrFunction();
	BOOL GetReviveFunction();

	


	// ADDRESS OF CALL: ORIG CHAT TO REPLACE IT WITH CALL: CBK CHAT
	BOOL GetChatFunction_hk();
	BOOL GetEntityFunction_hk();
	BOOL GetRecvFunction_hk();

	VOID Init();

	// HOOKS
public:
	// ADDRESSES TO HOOK.
	static LPVOID m_hk_ChatFunction;
	static LPVOID m_hk_EntityFunction;
	static LPVOID m_hk_RecvFunction;

	static sEntity TheEntity;



	~Artifacts()
	{
		delete m_AABuffer;
		delete m_PickDropBuffer;
		delete m_ReviveBuffer;
	}

};
