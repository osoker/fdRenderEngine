// smManager.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "smManager.h"

// ����dx9.0��Error�����
# pragma comment(lib,"dxerr9" )

#pragma comment (lib,"dsound.lib" )


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

