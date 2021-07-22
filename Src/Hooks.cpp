#include "Hooks.h"

void Hooks::Init()
{


	logger.log_raw("\n=========================\n");
	logger.log("Hooks::Init.");

	HookChatFunction();
	logger.log("Cbk_ChatFunctionInstalled.");

	HookEntityFunction();
	logger.log("Cbk_EntityFunctionInstalled.");

	HookRecvFunction();
	logger.log("Cbk_RecvFunctionInstalled.");


}

bool Hooks::HookChatFunction()
{
	auto Cbk_ChatFunction = Functions::Cbk_ChatFunction;
	auto lpHookAddress = Artifacts::m_hk_ChatFunction;

	// ADDRESS TO HOOK: E8 FB ED FF FF
	patch.CallPatch(lpHookAddress, Cbk_ChatFunction);


	return TRUE;
}

bool Hooks::HookEntityFunction()
{
	auto Cbk_EntityFunction = Functions::Cbk_EntityFunction;
	auto lpHookAddress = Artifacts::m_hk_EntityFunction;

	// ADDRESS TO HOOK: E8 F0C1FBFF
	patch.CallPatch(lpHookAddress, Cbk_EntityFunction);

	return TRUE;

}

bool Hooks::HookRecvFunction()
{
	auto Cbk_RecvFunction = Functions::Cbk_RecvFunction;
	auto lpHookAddress = Artifacts::m_hk_RecvFunction;

	patch.CallPatch(lpHookAddress, Cbk_RecvFunction);

	return TRUE;
}
