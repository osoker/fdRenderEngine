// common.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "common.h"
#include "bugCommit.h"


# pragma comment( lib ,"winmm" )
# pragma comment( lib ,"d3d9" )
# pragma comment( lib ,"d3dx9" )
# pragma comment( lib ,"dxerr9" )
# pragma comment( lib ,"dsound" )	


#ifdef _BUG_COMMITTER
extern BugManage	g_bugMgr;
#endif //_BUG_COMMITER


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
		
	

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

#ifdef _BUG_COMMITTER

		BugManage::InitCommonCtrl();
		g_bugMgr.CreateDlg((HINSTANCE)hModule);
#endif //_BUG_COMMITER

		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
