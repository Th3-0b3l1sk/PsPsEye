#include "Patch.h"


void Patch::JumpPatch(LPVOID lpAddress, LPVOID lpTarget) const
{
	DWORD dwOldProtect{};
	constexpr DWORD dwJumpLength = 5;
	VirtualProtect(lpAddress, dwJumpLength, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// WRITE JMP OPCODE
	memory.Write<BYTE, FALSE>(lpAddress, 0xE9);
	DWORD dwPatchAddress = (DWORD)lpAddress + 1;

	// RELATIVE OFFSET: NEW - CURRENT - 5
	DWORD dwRelativeOffset = (DWORD)lpTarget - (DWORD)lpAddress - 5;
	memory.Write<DWORD, FALSE>((LPVOID)dwPatchAddress, dwRelativeOffset);

	VirtualProtect(lpAddress, dwJumpLength, dwOldProtect, &dwOldProtect);
}

LPVOID Patch::TrampolinePatch(LPVOID lpAddress, LPVOID lpTarget, DWORD dwSize) const
{
	auto lpGate = VirtualAlloc(0, dwSize + 0x5, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpGate == 0x00)
	{
		logger.log_windows_error("TrampolinePatch::VirtualAlloc failed");
		return 0;
	}

	// SET GATE BYTES: STOLEN BYTES + NEW JMP ADDRESS
	memcpy_s(lpGate, dwSize, lpAddress, dwSize);

	// NEW JMP ADDRESS = lpAddress + dwSize;
	LPVOID lpJmpAddress = reinterpret_cast<LPVOID>((DWORD)lpGate + dwSize);
	LPVOID lpNewJmpAddress = reinterpret_cast<LPVOID>((DWORD)lpAddress + dwSize);

	// SET THE JMP BACK
	JumpPatch(lpJmpAddress, lpNewJmpAddress);

	InlinePatch(lpAddress, 0x90, dwSize);

	// SET THE TRAMPOLINE
	JumpPatch(lpAddress, lpTarget);
	

	return lpGate;
}

void Patch::InlinePatch(LPVOID lpAddress, BYTE Value, DWORD dwCount) const
{
	DWORD dwOldProtect{};
	VirtualProtect(lpAddress, dwCount, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memset(lpAddress, Value, dwCount);
	VirtualProtect(lpAddress, dwCount, dwOldProtect, &dwOldProtect);

}

void Patch::CallPatch(LPVOID lpAddress, LPVOID lpTarget) const
{
	DWORD dwOldProtect{};
	constexpr auto PatchSize = 5;
	VirtualProtect(lpAddress, PatchSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memory.Write<BYTE, FALSE>(lpAddress, 0xE8);

	auto lpCallPatchAddress = reinterpret_cast<LPVOID>((DWORD)lpAddress + 1);
	auto dwRelativePatchAddress = (DWORD)lpTarget - (DWORD)lpAddress - 5;

	memory.Write<DWORD>(lpCallPatchAddress, dwRelativePatchAddress);


	VirtualProtect(lpAddress, PatchSize, dwOldProtect, &dwOldProtect);



}


