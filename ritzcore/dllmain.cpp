// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#ifdef _DEBUG
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>
#endif

#include <windows.h>
#include "mame/mame_dmy.h"
#include "mame/emu.h"

static int C;
const HANDLE gProcessHeap = GetProcessHeap();
extern LARGE_INTEGER pf_freq;

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
){
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		QueryPerformanceFrequency(&pf_freq);
		::InitializeCriticalSection(&ritz_critical_section);
		C = 0;
		break;
	case DLL_THREAD_ATTACH:
		C++;
		break;
	case DLL_THREAD_DETACH:
		C--;
		break;
	case DLL_PROCESS_DETACH:
		::DeleteCriticalSection(&ritz_critical_section);
#ifdef _DEBUG
		DBG_OFF();
		_CrtDumpMemoryLeaks();
#endif
		break;
	}
	return TRUE;
}

