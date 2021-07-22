#include "Artifacts.h"

extern Logger logger;
extern Memory memory;

Artifacts::Artifacts()
{
	Init();
}

//----------------------------------------------
//----------------- IMMEDIATES -----------------
//----------------------------------------------

BOOL Artifacts::GetPlayerAddress()
{
	std::vector<uint8_t> bytes = { 0x6A, 0x04, 0xB8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x56, 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, 0x33, 0xC0 };
	std::string_view mask = "xxx????x????x????xxx????xx";
	LPVOID lpStartAddress = reinterpret_cast<LPVOID>(0x00402C46);

	// USING DEFAULT SEARCH SIZE OF 4096
	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x0)
	{
		logger.log_error("Artifacts::GetPlayerAddress returned null lpScanAddress.");
		return FALSE;
	}

	// OFFSET FROM START OF lpScanAddress
	constexpr auto HookAddressOffset = 0x1D;
	auto lpReadPlayerPtrAddress = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + HookAddressOffset);

	// logger.log<TRUE>("lpReadPlayerPtrAddress", (DWORD)lpReadPlayerPtrAddress);
	
	// ADDRESS FORMAT: 
	// MOV ECX, DWORD PTR:[XXXX]
	// FF FF XX XX XX XX
	auto lpPlayerPtrAddress = reinterpret_cast<LPVOID>((DWORD)lpReadPlayerPtrAddress + 2);
	this->m_lpPlayerPtr = reinterpret_cast<LPVOID>(memory.Read<DWORD>(lpPlayerPtrAddress));

	if (this->m_lpPlayerPtr == 0x0)
	{
		logger.log_error("Artifacts::GetPlayerAddress returned null lpPlayerPtr.");
		return FALSE;

	}


	return TRUE;

}

BOOL Artifacts::GetNetworkClass()
{
	std::vector<uint8_t> bytes = { 0x51, 0x50, 0xB9, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5E };
	std::string_view mask = "xxx????x????x";
	LPVOID lpStartAddress = reinterpret_cast<LPVOID>(0x009063F0);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetNetworkClass returned null lpScanAddress.");
		return FALSE;
	}

	// logger.log<TRUE>("lpScanAddress", (DWORD)lpScanAddress);

	// 0x2 TO SKIP 2 INSTUCTIONS, 0x1 TO GET TO IMMEDIATE
	constexpr auto HookAddressOffset = 0x2 + 0x1;
	auto lpNetworkClassAddress = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + HookAddressOffset);

	this->m_lpNetworkClass = reinterpret_cast<LPVOID>(memory.Read<DWORD>(lpNetworkClassAddress));
	
	if (this->m_lpNetworkClass == 0x00)
	{
		logger.log_error("Artifacts::GetPlayerAddress returned null lpNetworkClass.");
		return FALSE;
	}


	return TRUE;

}

BOOL Artifacts::GetEntityVTable()
{
	std::vector<uint8_t> bytes{ 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8,
		0x00, 0x00, 0x00, 0x00, 0xC6, 0x45, 0xFC, 0x01, 0x8D, 0x8E, 0x38, 0x06, 0x00, 0x00, 0xE8 };
	std::string_view mask{"xx????xx?????x????xxxxxxxxxxx"};
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x009184C4);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetEntityVTable returned null lpScanAddress.");
		return FALSE;
	}

	// READ THE DWORD PTR 
	
	// lpScanAddress = C706 FC37F600
	constexpr uint8_t MOVOFFSET = 0x2;
	auto lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + MOVOFFSET);

	auto EntityVTable = memory.Read<DWORD>(lpReadAddress);
	this->m_lpEntityVTable = reinterpret_cast<LPVOID>(EntityVTable);

	return TRUE;
}

BOOL Artifacts::GetEntityList()
{
	// THIS FUNCTION MUST BE CALLED FROM BOT AFTER THE GAME IS LOADED.

	if (this->m_lpPlayerPtr == 0x00) return FALSE;
	
	constexpr auto EntityBaseOffsetFromPlayerBase = 0x5488;
	auto PlayerBase = memory.Read<DWORD>(this->m_lpPlayerPtr);

	this->m_lpEntityList = reinterpret_cast<LPVOID>(PlayerBase + EntityBaseOffsetFromPlayerBase);


	return TRUE;
}

BOOL Artifacts::GetAAPacketDwords()
{
	// FIRST DWORD.

	std::vector<uint8_t> bytes{ 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x68, 0x94, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x59 };
	std::string_view mask{"xx????x????xxxxxx????x"};
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x0098E440);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetAAPacketFirstDword returned null lpScanAddress.");
		return FALSE;
	}

	// Address to read: C706 {E4 A1 F6 00}

	auto constexpr OFFSET = 0x2;
	auto lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + OFFSET);

	this->m_lpAAPacketFirstDword = memory.Read<LPVOID>(lpReadAddress);

	// SECOND DWORD.
	// IS AN OFFSET -0xA8 FROM FIRST DWORD

	constexpr auto SecondDwordOffset = -0xa8;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpReadAddress + SecondDwordOffset);	// the instruciton offset is already calculated.

	this->m_lpAAPacketSecondDword = memory.Read<LPVOID>(lpReadAddress);

	return TRUE;
}

BOOL Artifacts::GetReviveDword()
{
	std::vector<uint8_t> bytes{ 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0x33, 0xFF, 0x89, 0x7D, 0xFC, 0x8D, 0x86, 0x04, 0x04, 0x00, 0x00, 0x39, 0x38,
		0x74, 0x00, 0xC6, 0x45, 0xFC, 0x01, 0x8B, 0x00, 0x3B, 0xC7, 0x74, 0x00, 0x8B, 0x10, 0x6A, 0x01, 0x8B, 0xC8, 0xFF, 0x12, 0x89, 0x7D, 0xFC,
		0xEB, 0x00, 0x6A, 0x3F, 0x68, 0x00, 0x00, 0x00, 0x00, 0x68 };
	std::string_view mask{ "xx????xxxxxxxxxxxxxx?xxxxxxxxx?xxxxxxxxxxxx?xxx????x" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x009F2C68);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetReviveDword returned null lpScanAddress.");
		return FALSE;
	}

	// FIRST DWORD
	constexpr auto InstructionOffset = 0x2;
	auto lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + InstructionOffset);

	this->m_lpReviveFirstDword = memory.Read<LPVOID>(lpReadAddress);

	// SECOND DWORD AT AN OFFSET 0x860 FROM FIRST DWORD
	constexpr auto SecondDwordOffset = 0x85e;
	lpReadAddress = reinterpret_cast<LPVOID>((DWORD)lpReadAddress + SecondDwordOffset);	// instruction offset already calculated.

	this->m_lpReviveSecondDword = memory.Read<LPVOID>(lpReadAddress);

	return TRUE;

}

BOOL Artifacts::GetPickDropDword()
{
	std::vector<uint8_t> bytes = { 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x46, 0x04, 0x89, 0x86, 0x04, 0x04, 0x00, 0x00, 0x83, 0x4D, 0xFC, 0xFF, 0x8B, 0xC6 };
	std::string_view mask{ "xx????x????xxxxxxxxxxxxxxx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x00BEF816);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetPickDropDword returned null lpScanAddress.");
		return FALSE;
	}

	// VALUE TO READ:  C706 [DC58F900]

	constexpr auto OpcodeLength = 0x2;
	auto lpDwordAddress = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + OpcodeLength);

	this->m_lpPickDropDword = memory.Read<LPVOID>(lpDwordAddress);

	return TRUE;


}

BOOL Artifacts::GetAutoVTable()
{
	std::vector<uint8_t> bytes{ 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x4D, 0xFC, 0xFF, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xC2, 0x04, 0x00 };
	std::string_view mask{ "xx????xxxxx????xxx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x0049139C);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetAutoVTable returned null lpScanAddress.");
	}

	// ADDRESS TO READ => C700 2433E700 

	constexpr auto InstructionLength = 0x2;
	auto lpAddressToRead = reinterpret_cast<LPVOID>((DWORD)lpScanAddress + InstructionLength);

	this->m_AutoVTable = memory.Read<LPVOID>(lpAddressToRead);
	
	return TRUE;
	
}


//---------------------------------------------
//----------------- FUNCTIONS -----------------
//---------------------------------------------

BOOL Artifacts::GetSendFunction()
{
	std::vector<uint8_t> bytes = { 0x6A, 0x0C, 0xB8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xF1, 0x89, 0x75,
		0xE8, 0x33, 0xDB, 0x39, 0x5E, 0x18, 0x75, 0x04, 0x33, 0xC0, 0xEB, 0x66, 0xB8, 0x59, 0x08, 0x00, 0x00, 0x8B, 0x7D, 0x08 };
	std::string_view mask = "xxx????x????xxxxxxxxxxxxxxxxxxxxxxxx";
	LPVOID lpStartAddress = reinterpret_cast<LPVOID>(0x00906328);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetSendFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_SendFunction = lpScanAddress;


	return TRUE;
}

BOOL Artifacts::GetRecvFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x8D, 0xAC, 0x24, 0xBC, 0xDF, 0xFF, 0xFF, 0xB8, 0x44, 0x20,
		0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x6A, 0xFF, 0x68, 0x00, 0x00, 0x00, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50 };
	std::string_view mask = "xxxxxxxxxxxxxx????xxx????xxxxxxx";
	LPVOID lpStartAddress = reinterpret_cast<LPVOID>(0x0090585D);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetRecvFunction returned null lpScanAddress");
		return FALSE;
	}

	this->m_RecvFunction = lpScanAddress;

	return TRUE;
}


BOOL Artifacts::GetChatFunction()
{
	std::vector<uint8_t> bytes = { 0x55, 0x8D, 0xAC, 0x24, 0x18, 0xF1, 0xFF, 0xFF, 0x81, 0xEC, 0xE8, 0x0E,
		0x00, 0x00, 0x6A, 0xFF, 0x68, 0x00, 0x00, 0x00, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 };
	std::string_view mask = "xxxxxxxxxxxxxxxxx????xxxxxx";
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x00933AE5);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetChatFunction returned null lpScanAddress.");
		return FALSE;
	}


	this->m_ChatFunction = lpScanAddress;

	return TRUE;

}

BOOL Artifacts::GetPathFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x8B, 0xEC, 0x81, 0xEC, 0x2C, 0x01, 0x00, 0x00, 0x56, 0x57,
		0x8B, 0xF1, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xC8, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x84, 0xC0, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0xE8 };
	std::string_view mask = "xxxxxxxxxxxxxx????xxx????xxxx????x";
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x00940489);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetPathFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_PathFunction = lpScanAddress;

	return TRUE;
}

BOOL Artifacts::GetAAPacketFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x5D, 0x0C, 0x56, 0x57, 0x8B, 0xF1, 0x85, 0xDB, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x7D, 0x10, 0x85, 0xFF, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x8E, 0x04, 0x04, 0x00, 0x00 };
	std::string_view mask{"xxxxxxxxxxxxxxx????xxxxxxx????x????xxxxxx"};
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x0098DFC4);
	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetAAPacketFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_AAPacketFunction = lpScanAddress;

	return TRUE;
}

BOOL Artifacts::GetEntityFunction()
{
	// USING A SHORT CUT
	if (this->m_hk_EntityFunction == 0x00) return FALSE;

	// E8 F0 C1 FB FF  
	constexpr auto InstructionSize = 5;
	this->m_EntityFunction = memory.CalculateAddressFromOffset(m_hk_EntityFunction, InstructionSize);

	return TRUE;
}

BOOL Artifacts::GetPickDropFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x8B, 0xEC, 0x53, 0x56, 0x8B, 0x75, 0x08, 0x8B, 0xD9, 0x85, 0xF6, 0x75, 0x00, 0x33, 0xC0, 0xE9, 0x00, 0x00, 0x00, 0x00, 0x57, 0xE8 };
	std::string_view mask{ "xxxxxxxxxxxxx?xxx????xx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x00BEF89E);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetDropFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_PickDropFunction = lpScanAddress;

	return TRUE;
}

BOOL Artifacts::GetActionFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x8B, 0xEC, 0x56, 0x57, 0x8B, 0x7D, 0x08, 0x8B, 0xF1, 0x85, 0xFF, 0x75, 0x00, 0x32, 0xC0, 0xE9,
		0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x8E, 0x04, 0x04, 0x00, 0x00, 0x81, 0x49, 0x20, 0x00, 0x01, 0x00, 0x00 };
	std::string_view mask{ "xxxxxxxxxxxxx?xxx????x????x????xxxxxxxxxxxxx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x009F2B79);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetActionFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_ActionFunction = lpScanAddress;

	return TRUE;


}

BOOL Artifacts::GetSelfSkillFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x83, 0xEC, 0x54, 0x68, 0xE4, 0x01, 0x00, 0x00, 0xB8, 0x00, 0x00,
		0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xF1, 0xFF, 0x75, 0x6C, 0x8B, 0x7D, 0x60, 0x57, 0xE8 };
	std::string_view mask{ "xxxxxxxxxx????x????xxxxxxxxxx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x00A2244E);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });
	
	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetSelfSkillFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_SelfSkillFunction = lpScanAddress;

	return TRUE;
}

BOOL Artifacts::GetAutoFunction()
{
	std::vector<uint8_t> bytes{ 0x68, 0x74, 0x06, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xF1, 0x33, 0xDB };
	std::string_view mask{ "xxxxxx????x????xxxx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x0094092B);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetAutoFunction returned null lpScanAddres.");
		return FALSE;
	}

	this->m_AutoFunction = lpScanAddress;
	
	return TRUE;
	
}

BOOL Artifacts::GetIsBadPtrFunction()
{
	std::vector<uint8_t> bytes{ 0x83, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x74, 0x00, 0x8B, 0x74, 0x24, 0x08 };
	std::string_view mask{ "xx?????xx?xxxx" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x009088AF);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetIsBadPtrFunction returned null lpScanAddress.");
		return FALSE;
	}

	m_IsBadPtrFunction = lpScanAddress;

	return TRUE;

}

BOOL Artifacts::GetReviveFunction()
{
	std::vector<uint8_t> bytes{ 0x55, 0x8B, 0xEC, 0x56, 0x57, 0x8B, 0x7D, 0x08, 0x8B, 0xF1, 0x85, 0xFF, 0x75, 0x00, 0x32, 0xC0, 0xE9, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8 };
	std::string_view mask{ "xxxxxxxxxxxxx?xxx????x????x" };
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x009F2CC8);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetReviveFunction returned null lpScanAddress.");
		return FALSE;
	}

	this->m_ReviveFunction = lpScanAddress;

	return TRUE;
	
}


BOOL Artifacts::GetRecvFunction_hk()
{
	// --TODO:
	//		- UPDATE RECV DATA [*]

	std::vector<uint8_t> bytes{ 0xE8, 0x00, 0x00, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x00, 0x8B, 0x4E, 0x14, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x83, 0x4D, 0xFC, 0xFF };
	std::string_view mask = "x????xxx?xxxx????xxxx";
	LPVOID lpStartAddress = reinterpret_cast<LPVOID>(0x00905E88);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetRecvFunction returned null lpScanAddress");
		return FALSE;
	}

	this->m_hk_RecvFunction = lpScanAddress;


	return TRUE;
}

BOOL Artifacts::GetChatFunction_hk()
{
	// RETRIEVES THE ADDRESS TO REPLACE ORG_CHAT FUNCTION

	std::vector<uint8_t> bytes{ 0xE8, 0x00, 0x00, 0x00, 0x00, 0x83, 0x4D, 0xFC, 0xFF, 0x8B, 0x4D, 0x0C, 0x85, 0xC9, 0x74,
		0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xC2, 0x28, 0x00, 0x6A, 0x48, 0xB8, 0x00, 0x00,
		0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8 };
	std::string_view mask = "x????xxxxxxxxxx?x????x????xxxxxx????x????x";
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x00934FF2);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x0)
	{
		logger.log_error("Artifacts::GetChatFunction_hk returned null lpScanAddress.");
		return FALSE;
	}

	this->m_hk_ChatFunction = lpScanAddress;

	return TRUE;


}

BOOL Artifacts::GetEntityFunction_hk()
{
	std::vector<uint8_t> bytes{ 0xE8, 0x00, 0x00, 0x00, 0x00, 0x59, 0x85, 0xC0, 0x0F, 0x85, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xE8 };
	std::string_view mask{"x????xxxxx????xxx"};
	auto lpStartAddress = reinterpret_cast<LPVOID>(0x0094C5F5);

	auto lpScanAddress = memory.AOBScan(lpStartAddress, { bytes, mask });

	if (lpScanAddress == 0x00)
	{
		logger.log_error("Artifacts::GetEntityFunction_hk returned null lpScanAddress.");
		return FALSE;
	}

	
	this->m_hk_EntityFunction = lpScanAddress;
	return TRUE;


}

//---------------------------------------------
//----------------- Hooks ---------------------
//---------------------------------------------

LPVOID Artifacts::m_hk_ChatFunction = 0x0;
LPVOID Artifacts::m_hk_EntityFunction = 0x0;
LPVOID Artifacts::m_hk_RecvFunction = 0x0;

std::vector<uint32_t> Artifacts::AlreadyHitEntities;

Artifacts::sEntity Artifacts::TheEntity{};
uint32_t Artifacts::AlreadyHitMax = 5; // change back to 4

std::vector<Artifacts::sDrop> Artifacts::Drops{};
std::unordered_set<DWORD> Artifacts::DropsID{};


VOID Artifacts::Init()
{

	
	GetPlayerAddress();
	logger.log<TRUE>("PlayerPtr: ", (DWORD)m_lpPlayerPtr);

	GetNetworkClass();
	logger.log<TRUE>("NetworkClass: ", (DWORD)m_lpNetworkClass);

	GetSendFunction();
	logger.log<TRUE>("SendFunction: ", (DWORD)m_SendFunction);

	GetRecvFunction();
	logger.log<TRUE>("RecvFunction: ", (DWORD)m_RecvFunction);

	GetChatFunction();
	logger.log<TRUE>("ChatFunction: ", (DWORD)m_ChatFunction);

	GetPickDropFunction();
	logger.log<TRUE>("PickDropFunction: ", (DWORD)m_PickDropFunction);

	GetChatFunction_hk();
	logger.log<TRUE>("ChatFunction_hk: ", (DWORD)m_hk_ChatFunction);

	GetEntityFunction_hk();
	logger.log<TRUE>("EntityFunction_hk: ", (DWORD)m_hk_EntityFunction);

	GetRecvFunction_hk();
	logger.log<TRUE>("RecvFunction_hk: ", (DWORD)m_hk_RecvFunction);


	GetPathFunction();
	logger.log<TRUE>("PathFunction: ", (DWORD)m_PathFunction);

	GetEntityVTable();
	logger.log<TRUE>("EntityVTable: ", (DWORD)m_lpEntityVTable);

	GetEntityFunction();
	logger.log<TRUE>("EntityFunction: ", (DWORD)m_EntityFunction);

	GetAAPacketFunction();
	logger.log<TRUE>("AAPacketFunction: ", (DWORD)m_AAPacketFunction);

	GetAAPacketDwords();
	logger.log<TRUE>("AAPacketFirstDword: ", (DWORD)m_lpAAPacketFirstDword);
	logger.log<TRUE>("AAPacketSecondDword: ", (DWORD)m_lpAAPacketSecondDword);

	m_AABuffer = new sAABuffer((DWORD)m_lpAAPacketFirstDword, (DWORD)m_lpAAPacketSecondDword);

	GetPickDropDword();
	logger.log<TRUE>("PickDropDword: ", (DWORD)m_lpPickDropDword);

	m_PickDropBuffer = new sPickDropBuffer{ (DWORD)m_lpPickDropDword };
	
	logger.log<TRUE>("PickDropBuffer: ", (DWORD)m_PickDropBuffer);

	GetActionFunction();
	logger.log<TRUE>("ActionFunction: ", (DWORD)m_ActionFunction);

	GetSelfSkillFunction();
	logger.log<TRUE>("SelfSkillFunction: ", (DWORD)m_SelfSkillFunction);

	GetAutoVTable();
	logger.log<TRUE>("AutoVTable: ", (DWORD)m_AutoVTable);

	GetAutoFunction();
	logger.log<TRUE>("AutoFunction: ", (DWORD)m_AutoFunction);

	GetIsBadPtrFunction();
	logger.log<TRUE>("IsBadPtrFunction: ", (DWORD)m_IsBadPtrFunction);

	GetReviveDword();
	logger.log<TRUE>("ReviveFirstDword: ", (DWORD)m_lpReviveFirstDword);
	logger.log<TRUE>("ReviveSecondDword: ", (DWORD)m_lpReviveSecondDword);

	m_ReviveBuffer = new sAABuffer{ (DWORD)m_lpReviveFirstDword, (DWORD)m_lpReviveSecondDword };

	GetReviveFunction();
	logger.log<TRUE>("ReviveFunction: ", (DWORD)m_ReviveFunction);
}

