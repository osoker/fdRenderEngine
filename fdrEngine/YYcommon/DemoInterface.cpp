#include "stdafx.h"
#include "TcpClient.h"
#include "DemoInterface.h"

CDemoInterface g_yyDemo;


CDemoInterface::CDemoInterface(void)
{
    m_hFactory = NULL;
    m_pfnGetInterface = NULL;
	m_pfnGameWndMsg = NULL;
	m_errorCode= 0;
	m_isInit = FALSE;
}

CDemoInterface::~CDemoInterface(void)
{

}

BOOL CDemoInterface::InitFactory()
{
    WCHAR wzSelfPath[MAX_PATH]; memset( wzSelfPath, 0, sizeof(wzSelfPath) );
    GetModuleFileNameW( NULL, wzSelfPath, sizeof(wzSelfPath) );
    LPWSTR lpInsertPos = wcsrchr( wzSelfPath, L'\\' );
    *lpInsertPos = L'\0';
    lstrcatW( wzSelfPath, L"\\pipFactory.dll" );
	m_hFactory = LoadLibraryW( wzSelfPath );
	if( m_hFactory == NULL ) {
		m_errorCode = 1;
		return FALSE;
	}

    m_pfnInitInterface = (PFN_INITYYINTERFACE)GetProcAddress( m_hFactory, "YYPIP_InitInterface" );
	if( m_pfnInitInterface == NULL ){
		m_errorCode = 2;
		return FALSE;
	}

	m_pfnGetInterface = (PFN_GETYYINTERFACE)GetProcAddress( m_hFactory, "YYPIP_GetInterface" );
	if( m_pfnGetInterface == NULL ){
		m_errorCode = 3;
		return FALSE;
	}

    //call factory init func
	if( m_pfnInitInterface() == -1 ) {
		m_errorCode = 4;
		return FALSE;
	}

    /////////////////////////////////////////////////////////////////////////
    //all game must be call this func to show yy window
	m_pfnRunService = (PFN_RUNSERVICE)m_pfnGetInterface( "YYPIP_RunService" );
    if( m_pfnRunService == NULL ) return FALSE;

	m_pfnLoadInGame = (PFN_LOADINGAME)m_pfnGetInterface( "YYPIP_LoadInGame" );
    if( m_pfnLoadInGame == NULL ) return FALSE;

    m_pfnFreeGame = (PFN_FREEINGAME)m_pfnGetInterface( "YYPIP_FreeInGame" );
    if( m_pfnFreeGame == NULL ) return FALSE;

    m_pfnCheckClient = (PFN_CHECKCLIENT)m_pfnGetInterface( "YYPIP_CheckYYClient" );
    if( m_pfnCheckClient == NULL ) return FALSE;

    m_pfnIsPipSuccess = (PFN_ISPIPSUCCESS)m_pfnGetInterface( "YYPIP_IsPipRunSuccess" );
    if( m_pfnIsPipSuccess == NULL ) return FALSE;

    /////////////////////////////////////////////////////////////////////////
    //sometimes call this func to show yy window
    m_pfnMouseInput = (PFN_MOUSEINPUT)m_pfnGetInterface( "YYPIP_MouseInput" );
    if( m_pfnMouseInput == NULL ) return FALSE;

    m_pfnSetMainWnd = (PFN_SETMAINWND)m_pfnGetInterface( "YYPIP_SetMainWnd" );
    if( m_pfnSetMainWnd == NULL ) return FALSE;

    m_pfnCreateUI = (PFN_CREATEUI)m_pfnGetInterface( "YYPIP_CreateUI" );
    if( m_pfnCreateUI == NULL ) return FALSE;

    m_pfnDestoryUI = (PFN_DESTORYUI)m_pfnGetInterface( "YYPIP_DestoryUI" );
    if( m_pfnDestoryUI == NULL ) return FALSE;

    m_pfnRenderGUI = (PFN_RENDERGUI)m_pfnGetInterface( "YYPIP_RenderGUI" );
    if( m_pfnRenderGUI == NULL ) return FALSE;

    m_pfnGameWndMsg = (PFN_GAMEWNDMSG)m_pfnGetInterface( "YYPIP_GameWndMessage" );
    if( m_pfnGameWndMsg == NULL ) return FALSE;


    /////////////////////////////////////////////////////////////////////////
    //game used yy voice channel
    m_pfnJoinChannel = (PFN_JOINCHANNEL)m_pfnGetInterface( "YYPIP_JoinChannel" );
    if( m_pfnJoinChannel == NULL ) return FALSE;

    m_pfnSetTeamAdmin = (PFN_SETTEAMADMIN)m_pfnGetInterface( "YYPIP_SetTeamAdmin" );
    if( m_pfnSetTeamAdmin == NULL ) return FALSE;

    m_pfnSetUserName = (PFN_SETUSERNAME)m_pfnGetInterface( "YYPIP_SetUserName" );
    if( m_pfnSetUserName == NULL ) return FALSE;

    m_pfnJoinTeam = (PFN_JOINTEAM)m_pfnGetInterface( "YYPIP_JoinTeam" );
    if( m_pfnJoinTeam == NULL ) return FALSE;

    m_pfnSetTeamDevice = (PFN_SETTEAMDEVICE)m_pfnGetInterface( "YYPIP_SetTeamDevice" );
    if( m_pfnSetTeamDevice == NULL ) return FALSE;

    m_pfnSetTeamVoice = (PFN_SETTEAMVOICE)m_pfnGetInterface( "YYPIP_SetTeamVoice" );
    if( m_pfnSetTeamVoice == NULL ) return FALSE;

    m_pfnLockTeamVoice = (PFN_LOCKTEAMVOICE)m_pfnGetInterface( "YYPIP_LockTeamVoice" );
    if( m_pfnLockTeamVoice == NULL ) return FALSE;


    /////////////////////////////////////////////////////////////////////////
    //game to channel yy voice window
    m_pfnGetPipShow = (PFN_GETPIPSHOW)m_pfnGetInterface( "YYPIP_GetPipShow" );
    if( m_pfnGetPipShow == NULL ) return FALSE;

    m_pfnSetPipShow = (PFN_SETPIPSHOW)m_pfnGetInterface( "YYPIP_SetPipShow" );
    if( m_pfnSetPipShow == NULL ) return FALSE;

    m_pfnSetMsgShow = (PFN_SETMSGSHOW)m_pfnGetInterface( "YYPIP_SetMsgShow" );
    if( m_pfnSetMsgShow == NULL ) return FALSE;

    m_pfnMouseShow = (PFN_SETMOUSESHOW)m_pfnGetInterface( "YYPIP_SetMouseShow" );
    if( m_pfnMouseShow == NULL ) return FALSE;

    m_pfnLockWnd = (PFN_LOCKWINDOW)m_pfnGetInterface( "YYPIP_LockWindow" );
    if( m_pfnLockWnd == NULL ) return FALSE;

    m_pfnMoveWnd = (PFN_MOVEWINDOW)m_pfnGetInterface( "YYPIP_MoveWindow" );
    if( m_pfnMoveWnd == NULL ) return FALSE;


    /////////////////////////////////////////////////////////////////////////
    //game to used open id func 
    m_pfnShowOpid = (PFN_SHOWOPENID)m_pfnGetInterface( "YYPIP_ShowOpenId" );
    if( m_pfnShowOpid == NULL ) return FALSE;

    m_pfnLoginSucceed = (PFN_LOGINSUCCEED)m_pfnGetInterface( "YYPIP_LoginSucceed" );
    if( m_pfnLoginSucceed == NULL ) return FALSE;

    m_pfnLoginFailure = (PFN_LOGINFAILURE)m_pfnGetInterface( "YYPIP_LoginFailure" );
    if( m_pfnLoginFailure == NULL ) return FALSE;

    m_pfnSetLoginKey = (PFN_SETLOGINKEY)m_pfnGetInterface( "YYPIP_SetLoginKey" );
    if( m_pfnSetLoginKey == NULL ) return FALSE;

    m_pfnSetCallBack = (PFN_SETCALLBACK)m_pfnGetInterface( "YYPIP_SetCallBackFunc" );
    if( m_pfnSetCallBack == NULL ) return FALSE;

	m_isInit = TRUE;

    return TRUE;
}
BOOL CDemoInterface::IsInit()
{
	guard;

	return m_isInit;

	unguard;
}
int CDemoInterface::GetErrorCode()
{
	guard;

	return m_errorCode;

	unguard;
}


