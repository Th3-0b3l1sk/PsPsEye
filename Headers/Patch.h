#pragma once
#include "comms.h"
#include "Memory.h"

extern Memory memory;

// NOTE
// ALL OF PATCHES ASSUME A SIZE OF 5, ONLY TRAMPOLINE PATCH IS ADJUSTABLE!!
// EVERY PATCH METHOD CALLS VIRTUALPROTECT

class Patch
{
public:
	// PUBLIC INTERFACE

	// INPLACE JMP PATCH, NO RETURN.
	void JumpPatch(LPVOID lpAddress, LPVOID lpTarget) const;

	// ALLOCATES A GATE MEMORY, COPIES OLD BYTES, SETS THE NEW JMP ADDRESS IN THE GATE, PLACES THE INPLACE JMP & RETURNS
	// THE GATE ADDRESS
	LPVOID TrampolinePatch(LPVOID lpAddress, LPVOID lpTarget, DWORD dwSize) const;

	// SETS A REGION OF MEMORY TO A Value
	void InlinePatch(LPVOID lpAddress, BYTE Value, DWORD dwCount) const;

	// SETS AN INPLACE CALL PATCH
	void CallPatch(LPVOID lpAddress, LPVOID lpTarget) const;

};