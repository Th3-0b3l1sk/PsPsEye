#include "comms.h"
#include "Globals.h"
#include "Artifacts.h"
#include "Functions.h"
#include "Hooks.h"
#include "Bot.h"

// MAINTAIN CLASS CREATION ORDER!!
Artifacts artifacts{};
Functions functions{};
Hooks hooks{};



// FOR MOBS...
CRITICAL_SECTION EntityCS;

// FOR DROPS...
CRITICAL_SECTION DropCS;

Bot* CBot = new Bot{ &artifacts, &functions, &hooks };


VOID Run()
{
	while (1) {

		if (functions.isEnable() == TRUE) {
			CBot->Run();
		}

		Sleep(100);
	}
}


BOOL WINAPI ThreadProc()
{
	
	InitializeCriticalSection(&EntityCS);
	InitializeCriticalSection(&DropCS);

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Run, NULL, NULL, NULL);
	return TRUE;
	
}





BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
	
		logger.log_raw("Dll Loaded!\n");
		ThreadProc();
		
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

