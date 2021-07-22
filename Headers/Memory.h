#pragma once
#include "comms.h"
#include "Logger.h"

extern Logger logger;

class Memory
{
public:
	Memory()
	{
		dwModuleBase = (DWORD)GetModuleHandle(NULL);
		if (dwModuleBase == 0x00)
		{
			logger.log_windows_error("Memory::GetModuleHandle failed");
		}
	}

	template <typename T>
	inline T Read(LPVOID lpAddress) const
	{
		return *(T*)lpAddress;
	}

	template<typename T, bool EnableVirtual = true>
	inline void Write(LPVOID lpAddress, T _tValue) const
	{
		if (EnableVirtual) {
			DWORD dwOldProtect{};
			VirtualProtect(lpAddress, sizeof(T), PAGE_EXECUTE_READWRITE, &dwOldProtect);
			*(T*)lpAddress = _tValue;
			VirtualProtect(lpAddress, sizeof(T), dwOldProtect, &dwOldProtect);
		}
		else
		{
			*(T*)lpAddress = _tValue;
		}
	}


	// NOTE:
	// THE MASK MUST BEGIN WITH AN X IT CAN'T BEGIN WITH A WILDCARD.
	LPVOID AOBScan(LPVOID lpStartAddress, const std::pair<std::vector<uint8_t>, std::string_view>& pair, size_t dwScanSize = 0x1000 )
	{
		const std::vector<uint8_t>& bytes = pair.first;
		std::string_view mask = pair.second;
		auto dwAddressToRead = reinterpret_cast<DWORD>(lpStartAddress);
		LPVOID lpPossibleHit = 0x00;
			
		size_t i{};
		while (i < dwScanSize) {

			if (Read<uint8_t>(reinterpret_cast<LPVOID>((DWORD)lpStartAddress + i)) == bytes[0]) {

				// FOUND A POSSIBLE HIT.
				lpPossibleHit = reinterpret_cast<LPVOID>((DWORD)lpStartAddress + i);
			
				
				for (size_t j{ 1 }; j < bytes.size(); j++) {
			
					if (mask[j] == '?') continue;
					if (Read<uint8_t>(reinterpret_cast<LPVOID>((DWORD)lpPossibleHit + j)) != bytes[j])
					{
						lpPossibleHit = 0x00;
						break;
					}

				}

				if (lpPossibleHit != 0x00) return lpPossibleHit;

			}

			
			++i;
		}

		return 0x00;

	}

	inline LPVOID CalculateAddressFromOffset(LPVOID lpAddress, size_t InstructionSize) const
	{
		// Calculates the target function from a call/jmp instruction using the offset.
		// ASSUMING THE INSTRUCTION OPCODE IS 1 BYTE
		constexpr int8_t InstructionOpcodeByte = 1;
		auto lpOffsetAddress = reinterpret_cast<LPVOID>((DWORD)lpAddress + InstructionOpcodeByte);
		auto Offset = Read<int32_t>(lpOffsetAddress);
		
		// Target = AddressOfNextInstruction + Offset
		auto lpAddressOfNextInstruction = reinterpret_cast<LPVOID>((DWORD)lpAddress + InstructionSize);
		auto lpTarget = reinterpret_cast<LPVOID>((DWORD)lpAddressOfNextInstruction + Offset);

		return lpTarget;
	}

private:
	DWORD dwModuleBase = 0x0;
};

