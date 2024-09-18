///////////////////////////////////////////////////////////////////////////////////////////
/** \file
*  Filename: consoleMisk.cpp
*
*  Desc:     魔法demoConsole的相关处理，主要是一个about的对话框内，做上各种各样的功能
* 
*  His:      River created @ 2006-6-16
* 
*/
///////////////////////////////////////////////////////////////////////////////////////////
# include "gameApp.h"
# include "gc_testScene.h"
# include "resource.h"
# include "direct.h"
# include "commdlg.h"
# include "deployMonster_mod.h"
# include "gc_creature.h"



// 记录命令到硬盘
void   write_cmdToHd( const char* _cmd )
{
	FILE*    t_file = fopen( "lastcmd.txt","wt" );

	if( !t_file ) 
		return;
	fprintf( t_file,"%s\n",_cmd ) ;
	fclose( t_file );	
}
const char* get_lastCmdFromHd( void )
{
	static char   t_szCmd[256];
	t_szCmd[0] = NULL;
	FILE*    t_file = fopen( "lastcmd.txt","rt" );
	if( !t_file ) 
		return t_szCmd;
	int  t_idx = 0;
	fgets( t_szCmd,256,t_file );
	t_szCmd[strlen(t_szCmd)-1] = NULL;
	fclose( t_file );	
	return t_szCmd;
}


//! 得到要插入怪物的命令
void  get_monsterInsertCmd( char* _sz )
{
	// 
	int   t_iXAdd = getrand_fromintscope( -8,8 );
	int   t_iZAdd = getrand_fromintscope( -8,8 );

	t_iXAdd = lm_gvar::g_iHeroPosX + t_iXAdd;
	t_iZAdd = lm_gvar::g_iHeroPosZ + t_iZAdd;

	if( t_iXAdd < 0 )	t_iXAdd = 0;
	if( t_iZAdd < 0 )   t_iZAdd = 0;

	int t_idx = OSRAND % 6;
	int t_iAngle = OSRAND % 360;

	sprintf( _sz,"%s %s %d %d %d","monster",
		g_szHeroDir[t_idx].c_str(),t_iXAdd,t_iZAdd,t_iAngle );

	return;
}

void get_lastPlaySkillCmd( char* _sz )
{
	sprintf( _sz,"testskill %s",lm_gvar::g_szLastSkill.c_str() );
	return;
}

//! 执行命令
void  exec_command( const char* _cmd )
{
	if( lmc_commandMgr::Instance()->interpret_strToCmd( _cmd ) )
		write_cmdToHd( _cmd );
	else
		MessageBox( NULL,"执行命令失败....","ERROR",MB_OK );
}

// 调入执行命令集合
void loadAndExecCmdList( HWND  _hwnd )
{
	TCHAR filenamebuffer[256];
	char   backdir[256];

	// 弹出选择文件的对话框
	filenamebuffer[0] = NULL;
	strcpy( filenamebuffer,"default.txt" );
	// Display the OpenFileName dialog. Then, try to load the specified file
	OPENFILENAME ofn ;
	memset( &ofn,0,sizeof( OPENFILENAME ) );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = _hwnd;
	ofn.lpstrFilter = _T(".txt Files (.txt)\0*.txt\0\0");
	ofn.lpstrFile = filenamebuffer;
	ofn.nMaxFile = 256;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	//back up the current working directory.
	_getcwd( backdir,256 );
	ofn.lpstrInitialDir = backdir;

	// 
	// 打开文件,把选择的命令列表文件传给命令管理器
	if( GetOpenFileName( &ofn ) )
	{
		lmc_commandMgr::Instance()->load_execCmdFile( filenamebuffer );
		::chdir( backdir );
	}

	return;

}


// 在区域部署的怪物插入下拉框内,加入内容
void  init_monsterInsertCombo( HWND _hdlg )
{
	guard;

	osassert( _hdlg );

	HWND    t_hwnd;

	t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_MONTYPE );
	::SendMessage( t_hwnd,CB_RESETCONTENT,0,0 );

# if __DEPLOY_MONSTERMOD__
	int t_iSel = gc_gameApp::m_ptrActiveScene->get_curActiveArea();
	if( t_iSel < 0 )
		return;
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();

	int t_iMonTypeNum = t_ptrDmm->get_monsterType( t_iSel );
	for( int t_i=0;t_i<t_iMonTypeNum;t_i ++ )
		SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)t_ptrDmm->get_monsterTypeDir( t_iSel,t_i ) );

	::SendMessage( t_hwnd,CB_SETCURSEL,0 ,0 );
# endif 

	return;

	unguard;
}


# define   INIT_CMDFILENAME   "initcmd.ini"



void   init_deployMonsterInterface( HWND _hDlg )
{

# if __DEPLOY_MONSTERMOD__
	//! 看区域刷怪信息是否已经存在
	HWND    t_hwnd;
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( t_ptrDmm->is_dmmDataLoaded() )
	{
		int   t_iAreaNum = t_ptrDmm->get_monsterAreaNum();
		t_hwnd = ::GetDlgItem( _hDlg,IDC_COMBO_AREALIST );

		::SendMessage( t_hwnd,CB_RESETCONTENT,0,0 );
		for( int t_i=0;t_i<t_iAreaNum;t_i ++ )
			::SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)t_ptrDmm->get_monsterAreaName( t_i ) );

		int t_iSel = gc_gameApp::m_ptrActiveScene->get_curActiveArea();
		if( t_iSel < 0 )
			t_iSel = 0;
		::SendMessage( t_hwnd,CB_SETCURSEL,t_iSel,0 );

		// 初始化怪物播放下拉框
		init_monsterInsertCombo( _hDlg );

	}

	//! 初始化显示怪物边框的复选按钮
	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_TRIGGER );
	if( lm_gvar::g_bDisTrigger )
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_CHECKED,0 );
	else
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_UNCHECKED,0 );
	// 
	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_VISION );
	if( lm_gvar::g_bDisVision )
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_CHECKED,0 );
	else
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_UNCHECKED,0 );
	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_CALL );
	if( lm_gvar::g_bDisCall )
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_CHECKED,0 );
	else
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_UNCHECKED,0 );
	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_RUN );
	if( lm_gvar::g_bDisRun )
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_CHECKED,0 );
	else
		::SendMessage( t_hwnd,(UINT) BM_SETCHECK,BST_UNCHECKED,0 );

# endif 

}

//! 重设怪物id编辑框内的内容
void  reset_monsterId( HWND _hdlg )
{
	guard;

# if __DEPLOY_MONSTERMOD__
	osassert( _hdlg );
	HWND    t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_MONSTERNAME );
	int     t_idx = (int)::SendMessage( t_hwnd,(UINT) CB_GETCURSEL,0,0 );
	char    t_sz[128];

	// 没有调入相应的数据.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;

	if( t_idx >= 0 )
	{
		SMonstData*   t_ptrMonster = 
			gc_monsterIdNameMgr::Instance()->get_monsterFromIdx( t_idx );

		if( !t_ptrMonster )
			return;

		sprintf( t_sz,"%d",t_ptrMonster->m_dwMonstId );
		::SetDlgItemText( _hdlg,IDC_EDIT_MONSTERID,t_sz );

		// 移动间隔.
		if( lm_gvar::g_bNormalMoveTime )
			sprintf( t_sz,"%d",t_ptrMonster->m_dwZhengChangYiDongJianGe );
		else
			sprintf( t_sz,"%d",t_ptrMonster->m_dwZhuiJiYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwBuFu );
		::SetDlgItemText( _hdlg,IDC_EDIT_STEPLENGTH,t_sz );

		gc_gameApp::m_ptrActiveScene->set_curEditMonsterId( t_ptrMonster->m_dwMonstId );

	}

# endif

	unguard;
}

//! 根据id框的变化,变化其它的界面
void  reset_interfaceFromId( HWND _hdlg )
{
	guard;
	osassert( _hdlg );

	char    t_szId[128];
	int     t_id;

	// 没有调入相应的数据.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;

	::GetDlgItemText( _hdlg,IDC_EDIT_MONSTERID,t_szId,128 );
	t_id = atoi( t_szId );
	if( (t_id < 0)||(t_id > 10000) )
		return;

	int     t_idx = gc_monsterIdNameMgr::Instance()->get_monsterIdxFromId( t_id );
	if( t_idx == -1 )
		return;
	SMonstData*   t_ptrMonster = 
		gc_monsterIdNameMgr::Instance()->get_monsterFromIdx( t_idx );
	if( !t_ptrMonster )
		return;


	HWND    t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_MONSTERNAME );
	SendMessage( t_hwnd,(UINT) CB_SETCURSEL,t_idx,0 );

	char    t_sz[128];
	// 移动间隔.
	if( lm_gvar::g_bNormalMoveTime )
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhengChangYiDongJianGe );
	else
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhuiJiYiDongJianGe );
	::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
	sprintf( t_sz,"%d",t_ptrMonster->m_dwBuFu );
	::SetDlgItemText( _hdlg,IDC_EDIT_STEPLENGTH,t_sz );	

# if __DEPLOY_MONSTERMOD__
	gc_gameApp::m_ptrActiveScene->set_curEditMonsterId( t_ptrMonster->m_dwMonstId );
# endif 
	unguard;
}

//! 根据是否让怪物正常移动,来处理相关的界面
void  reset_interfaceFromNormalMove( HWND _hdlg )
{
	guard;

# if __DEPLOY_MONSTERMOD__
	osassert( _hdlg );
	lm_gvar::g_bNormalMoveTime = !lm_gvar::g_bNormalMoveTime;

	// 没有调入相应的数据.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;

	int   t_id = gc_gameApp::m_ptrActiveScene->get_curEditMonsterId();
	if( t_id < 0 )
		return;

	int   t_idx = 
		gc_monsterIdNameMgr::Instance()->get_monsterIdxFromId( t_id );
	SMonstData*   t_ptrMonster = 
		gc_monsterIdNameMgr::Instance()->get_monsterFromIdx( t_idx );
	if( !t_ptrMonster )
		return;

	char    t_sz[128];
	// 移动间隔.
	if( lm_gvar::g_bNormalMoveTime )
	{
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhengChangYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"正常移动间隔:" );
	}
	else
	{
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"追击移动间隔:" );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhuiJiYiDongJianGe );
	}
	::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
# endif 

	unguard;
}

//! 从几个编辑框得到怪物的数据,并调用三维场景的相关处理
bool  see_monsterMove( HWND  _hdlg )
{
	guard;

# if __DEPLOY_MONSTERMOD__
	// 没有调入相应的数据.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return true;

	// 当前正在编辑的怪物指针
	if( gc_gameApp::m_ptrActiveScene->get_curEditMonsterId() < 0 )
	{
		MessageBox( NULL,"ID错误,请选择怪物..\n","ERROR",MB_OK );
		return false;
	}
	int   t_id = gc_gameApp::m_ptrActiveScene->get_curEditMonsterId();
	if( t_id < 0 )
	{
		MessageBox( NULL,"ID错误,请选择怪物..\n","ERROR",MB_OK );
		return false;
	}

	int   t_idx = 
		gc_monsterIdNameMgr::Instance()->get_monsterIdxFromId( t_id );
	SMonstData*   t_ptrMonster = 
		gc_monsterIdNameMgr::Instance()->get_monsterFromIdx( t_idx );
	if( !t_ptrMonster )
	{
		MessageBox( NULL,"ID错误,请选择怪物..\n","ERROR",MB_OK );
		return false;
	}

	// 先得到移动间隔.
	char    t_sz[128];
	int     t_iTmp;
	::GetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz,128 );
	t_iTmp = atoi( t_sz );
	if( t_iTmp < 0 )
	{
		MessageBox( NULL,"移动间隔数据出错","ERROR",MB_OK );
		return false;
	}
	if( lm_gvar::g_bNormalMoveTime )
		t_ptrMonster->m_dwZhengChangYiDongJianGe = t_iTmp;
	else
		t_ptrMonster->m_dwZhuiJiYiDongJianGe = t_iTmp;

	// 得到步幅数据
	::GetDlgItemText( _hdlg,IDC_EDIT_STEPLENGTH,t_sz,128 );
	t_iTmp = atoi( t_sz );
	if( t_iTmp < 0 )
	{
		MessageBox( NULL,"移动间隔数据出错","ERROR",MB_OK );
		return false;
	}
	t_ptrMonster->m_dwBuFu = t_iTmp;

	// 调用场景的处理函数:
	gc_gameApp::m_ptrActiveScene->see_monsterMoveInScene( t_idx );
# endif 

	return TRUE;

	unguard;
}

//! 初始化怪物移动速度查看的编辑框
void  init_monsterMoveInterface( HWND _hdlg )
{
	guard;

	osassert( _hdlg );
	char     t_sz[256];

# if __DEPLOY_MONSTERMOD__
	// 没有调入相应的数据.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;

	// 在怪物的下拉框内加入怪物的名字
	HWND    t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_MONSTERNAME );
	gc_monsterIdNameMgr*  t_ptrIdNameMgr = gc_monsterIdNameMgr::Instance();
	int                   t_iMonsterNum = t_ptrIdNameMgr->get_monsterNum();

	::SendMessage( t_hwnd,CB_RESETCONTENT,0,0 );
	for( int t_i=0;t_i<t_iMonsterNum;t_i ++ )
	{
		::SendMessage( t_hwnd,CB_ADDSTRING,0,
			(LPARAM)t_ptrIdNameMgr->get_monsterFromIdx( t_i )->m_szMonstName );
	}

	int   t_iCurSel = 0;
	if( gc_gameApp::m_ptrActiveScene->get_curEditMonsterId() >= 0 )
	{
		int  t_id = gc_gameApp::m_ptrActiveScene->get_curEditMonsterId();
		t_iCurSel = t_ptrIdNameMgr->get_monsterIdxFromId( t_id );
		if( t_iCurSel == -1 )
			t_iCurSel = 0;
	}
	::SendMessage( t_hwnd,CB_SETCURSEL,t_iCurSel,0 );

	// 左侧的怪物id编辑框
	SMonstData*   t_ptrMonster = t_ptrIdNameMgr->get_monsterFromIdx(t_iCurSel);
	osassert( t_ptrMonster );
	sprintf( t_sz,"%d",t_ptrMonster->m_dwMonstId );
	::SetDlgItemText( _hdlg,IDC_EDIT_MONSTERID,t_sz );

	// 是否是编辑正常怪物移动
	// ::SendMessage( t_hwnd,(UINT) BM_GETCHECK,0,0 ) == BST_CHECKED
	t_hwnd = ::GetDlgItem( _hdlg,IDC_CHECK_NORMALMOVE );
	if( lm_gvar::g_bNormalMoveTime )
	{
		::SendMessage( t_hwnd,(UINT)BM_SETCHECK,BST_CHECKED,0 );
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"正常移动间隔:" );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhengChangYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
	}
	else
	{
		::SendMessage( t_hwnd,(UINT)BM_SETCHECK,BST_UNCHECKED,0 );
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"追击移动间隔:" );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhuiJiYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
	}

	sprintf( t_sz,"%d",t_ptrMonster->m_dwBuFu );
	::SetDlgItemText( _hdlg,IDC_EDIT_STEPLENGTH,t_sz );
# endif 

	unguard;
}

//！显示初始化的相机fov.
void  init_cameraFov( HWND _hdlg )
{
	guard;

	//　先得到edit和上面的字符
	char   t_sz[256];
	float  t_fFov;

	t_fFov = gc_camera::m_ptrCamera->get_cameraFov();
	sprintf( t_sz,"%f",t_fFov );

	::SetDlgItemText( _hdlg,IDC_EDIT_FOVMOD,t_sz );

	unguard;
}

//! 程序初始化第一次打开控制台
static bool  t_sbFirstConsole = true;

void  add_cmdToCmdList( HWND hDlg )
{
	HWND    t_hwnd;

	t_hwnd = ::GetDlgItem( hDlg,IDC_COMBOCMD );

	CIniFile   t_file;

	if( !t_file.OpenFileRead( INIT_CMDFILENAME ) )
	{
		MessageBox( NULL,"打开初始化命令文件失败","找不到文件",MB_OK );

		::SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)"move zyla 128 128" );

		::SendMessage( t_hwnd,CB_SETCURSEL,0 ,0 );
		return;
	}
	else
	{
		// 读入命令列表
		t_file.ReadSection( "initcmd" );
		int  t_iCmdNum = 0;
		t_file.ReadLine( "cmdnum",&t_iCmdNum );
		char     t_szCmdContent[128];
		char     t_szCmdLine[128];

		for( int t_i=0;t_i<t_iCmdNum;t_i ++ )
		{
			sprintf( t_szCmdLine,"%s%d","cmd",t_i+1 );
			t_file.ReadLine( t_szCmdLine,t_szCmdContent,128 );
			::SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)t_szCmdContent );
		}

		::SendMessage( t_hwnd,CB_SETCURSEL,0 ,0 );
		t_file.CloseFile();
	}


	if( t_sbFirstConsole )
		t_sbFirstConsole = false;
	else
	{
		::SetDlgItemText( hDlg,IDC_COMBOCMD,get_lastCmdFromHd() );
	}

	// 使用框侧的字体处于全选中的状态
	char  t_szCmdCont[128];
	::GetDlgItemText( hDlg,IDC_COMBOCMD,t_szCmdCont,128 );
	::SendMessage( t_hwnd,EM_SETSEL,0,strlen( t_szCmdCont ) ); 

	// 初始化相机相关的数据。
	init_cameraFov( hDlg );

# if __DEPLOY_MONSTERMOD__

	init_deployMonsterInterface( hDlg );

	//! 怪物移动速度查看的界面初始化
	init_monsterMoveInterface( hDlg );

# endif 

}

//! 得到复选框的内容,赋给全局变量
void   process_changeMonsterDis( HWND _hDlg )
{
	HWND    t_hwnd;

	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_TRIGGER );
	if( ::SendMessage( t_hwnd,(UINT) BM_GETCHECK,0,0 ) == BST_CHECKED )
		lm_gvar::g_bDisTrigger = TRUE;
	else
		lm_gvar::g_bDisTrigger = FALSE;

	// 
	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_VISION );
	if( ::SendMessage( t_hwnd,(UINT) BM_GETCHECK,0,0 ) == BST_CHECKED )
		lm_gvar::g_bDisVision = TRUE;
	else
		lm_gvar::g_bDisVision = FALSE;

	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_CALL );
	if( ::SendMessage( t_hwnd,(UINT) BM_GETCHECK,0,0 ) == BST_CHECKED )
		lm_gvar::g_bDisCall = TRUE;
	else
		lm_gvar::g_bDisCall = FALSE;


	t_hwnd = ::GetDlgItem( _hDlg,IDC_CHECK_RUN );
	if( ::SendMessage( t_hwnd,(UINT) BM_GETCHECK,0,0 ) == BST_CHECKED )
		lm_gvar::g_bDisRun = TRUE;
	else
		lm_gvar::g_bDisRun = FALSE;

	return;

}

//! 调入一个怪物刷新的区域文件
BOOL load_deployMonsterData( HWND _hDlg )
{
	guard;

	TCHAR filenamebuffer[256];
	char   backdir[256];

	HWND    t_hwnd;

	t_hwnd = ::GetDlgItem( _hDlg,IDC_COMBO_AREALIST );

	// 弹出选择文件的对话框
	filenamebuffer[0] = NULL;
	strcpy( filenamebuffer,"default.md" );
	// Display the OpenFileName dialog. Then, try to load the specified file
	OPENFILENAME ofn ;
	memset( &ofn,0,sizeof( OPENFILENAME ) );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = _hDlg;
	ofn.lpstrFilter = _T(".md Files (.md)\0*.md\0\0");
	ofn.lpstrFile = filenamebuffer;
	ofn.nMaxFile = 256;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	//back up the current working directory.
	_getcwd( backdir,256 );
	ofn.lpstrInitialDir = backdir;

	// 
	// 打开文件,把选择的命令列表文件传给命令管理器
	if( GetOpenFileName( &ofn ) )
	{
		gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
		t_ptrDmm->lod_dmmData( filenamebuffer );
		::chdir( backdir );

		// 得到区域的信息
		int   t_iAreaNum = t_ptrDmm->get_monsterAreaNum();

		::SendMessage( t_hwnd,CB_RESETCONTENT,0,0 );
		for( int t_i=0;t_i<t_iAreaNum;t_i ++ )
			::SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)t_ptrDmm->get_monsterAreaName( t_i ) );	
		::SendMessage( t_hwnd,CB_SETCURSEL,0 ,0 );

	}

	return TRUE;

	unguard;
}

//! 部署一个怪物刷新区域
BOOL deploy_monsterInArea( HWND _hdlg )
{
	//! 根据当前下拉框的选择,但到当前的区域信息
	HWND   t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_AREALIST );
	int    t_idx = (int)::SendMessage( t_hwnd,(UINT) CB_GETCURSEL,0,0 );

# if __DEPLOY_MONSTERMOD__
	//! 场景内处理刷怪区域相关的数据
	if( t_idx >= 0 )
		gc_gameApp::m_ptrActiveScene->deploy_monsterAreaInScene( t_idx );
# endif 

	return TRUE;
}

//! 显示某一部位的选中的三角形
int display_seltri( HWND _hdlg )
{
	char   t_sz[64];
	int    t_iParId;
	int    t_iSel[4];

	
	::GetDlgItemText( _hdlg,IDC_EDIT_SEL1,t_sz,64 );
	t_iParId = atoi( t_sz );
	if( (t_iParId<0)||(t_iParId>7) )
	{
		MessageBox( NULL,"请重新指定身体部位ID...\n","ERROR",MB_OK );
		return 0;
	}
	::GetDlgItemText( _hdlg,IDC_EDIT_SEL2,t_sz,64 );
	t_iSel[0] = atoi( t_sz );
	::GetDlgItemText( _hdlg,IDC_EDIT_SEL3,t_sz,64 );
	t_iSel[1] = atoi( t_sz );
	if( t_iSel[1] == 0 )
		t_iSel[1] = -1;
	::GetDlgItemText( _hdlg,IDC_EDIT_SEL4,t_sz,64 );
	t_iSel[2] = atoi( t_sz );
	if( t_iSel[2] == 0 )
		t_iSel[2] = -1;
	t_iSel[3] = -1;

	int t_idx = 0;
	for( int t_i=0;t_i<3;t_i ++ )
	{
		if( t_iSel[t_i] == -1 )
			break;
		else
			t_idx ++;
	}

# if __GLOSSY_EDITOR__
	gc_gameScene::m_ptrCreatureMgr->
		get_heroPtr()->displaytri_bysel( t_iParId,t_iSel,t_idx );
# endif 

	return 1;
}

//! 处理相机的fov修改
void change_cameraFov( HWND _hdlg )
{
	guard;

	//  
	//　先得到edit和上面的字符
	char   t_sz[256];
	::GetDlgItemText( _hdlg,IDC_EDIT_FOVMOD,t_sz,256 );
	float  t_fFov = (float)atof( t_sz );

	gc_camera::m_ptrCamera->set_cameraFov( t_fFov );

	return;

	unguard;
}


// “关于”框的消息处理程序。
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char        t_szLastCmd[256];
	switch (message)
	{
	case WM_INITDIALOG:
		{
			add_cmdToCmdList( hDlg );

			return TRUE;
		}

	case WM_DESTROY:
		{			
			// 结束对话框
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if( LOWORD(wParam) == IDOK )
			{
				char     t_szCmd[256];
				::GetDlgItemText( hDlg,IDC_COMBOCMD,t_szCmd,256 );
				exec_command( t_szCmd );
			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! 按下插入怪物的按钮
		if( LOWORD(wParam) == IDC_BTN_INSERTMON )
		{
			get_monsterInsertCmd( t_szLastCmd );
			exec_command( t_szLastCmd );
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! 播放上次正确播放的技能.
		if( LOWORD(wParam) == IDC_BTN_PLAYLASTSKILL )
		{
			get_lastPlaySkillCmd( t_szLastCmd );
			exec_command( t_szLastCmd );
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! 调入执行命令集合.
		if( LOWORD(wParam) == IDC_BTN_EXECMDLIST )
		{
			loadAndExecCmdList( hDlg );
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;			
		}

		//  
		//! 如果要执行fov的修改
		if( LOWORD(wParam) == IDC_BUTTON_FOVMOD )
		{
			change_cameraFov( hDlg );
			EndDialog( hDlg,LOWORD(wParam) );
			return TRUE;
		}

		if( LOWORD(wParam) == IDC_BTN_SAVECOLLINFO )
		{
			EndDialog( hDlg,LOWORD(wParam));

# if __COLL_MOD__
			//gc_gameApp::m_ptrActiveScene->m_ptrScene->save_colheiModInfo();
# endif 
			break;
		}

		if( LOWORD(wParam) == IDC_BUTTONEXEC )
		{
			//
			// 得到编辑框内的字串，并处理成相应的命令,如果命令正确，
			// 记录命令，为下次console的默认命令。
			char     t_szCmd[256];
			::GetDlgItemText( hDlg,IDC_COMBOCMD,t_szCmd,256 );
			exec_command( t_szCmd );

			// 结束对话框
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! 显示选中部位的选择多边形
		if( LOWORD(wParam) == IDC_BUTTON_DISEL )
		{
			if( display_seltri( hDlg ) == 0 )
				return FALSE;

			// 结束对话框
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;			
		}

		//! 刷怪编辑器编辑相关的信息
# if __DEPLOY_MONSTERMOD__

		// 部署
		if( LOWORD(wParam) == IDC_BTN_DEPLOY_CURAREA )
		{
			::deploy_monsterInArea( hDlg );
			EndDialog( hDlg,LOWORD(wParam) );
			return TRUE;
		}
		// 存储
		if( LOWORD(wParam) == IDC_BTN_SAVE_DEPLOYDATA )
		{
			gc_gameApp::m_ptrActiveScene->change_actAreaMonsterPos();
			gc_deployMonsterMod::Instance()->save_dmmData();
			MessageBox( NULL,"存储刷怪信息文件成功","INFO",MB_OK );

			EndDialog( hDlg,LOWORD(wParam) );
			return true;
		}

		// 插入怪物在当前编辑的区域内
		if( LOWORD(wParam) == IDC_BTN_INSERT_MONSTER )
		{
			//! 根据当前下拉框的选择,但到当前的区域信息
			HWND   t_hwnd = ::GetDlgItem( hDlg,IDC_COMBO_MONTYPE );
			int    t_idx = (int)::SendMessage( t_hwnd,(UINT) CB_GETCURSEL,0,0 );

			gc_gameApp::m_ptrActiveScene->insert_monsterInScene( t_idx );
			
			EndDialog( hDlg,LOWORD(wParam) );
			return true;			
		}

		// 更改显示
		if( LOWORD(wParam) == IDC_BUTTON_CHNAGEDIS )
		{
			process_changeMonsterDis( hDlg );

			EndDialog( hDlg,LOWORD(wParam) );
			return true;	
		}

		// 根据怪物修改界面操作,来更改界面的显示
		if( LOWORD(wParam) == IDC_COMBO_MONSTERNAME )
		{
			reset_monsterId( hDlg );
			return TRUE;
		}
		if( LOWORD(wParam) == IDC_EDIT_MONSTERID )
		{
			::reset_interfaceFromId( hDlg );
			return TRUE;
		}
		if( LOWORD(wParam) == IDC_CHECK_NORMALMOVE )
		{
			::reset_interfaceFromNormalMove( hDlg );
			return TRUE;
		}

		//! 查看怪物移动:在场景内加入可移动怪物
		if( LOWORD(wParam) == IDC_BUTTON_SEEMONMOVE )
		{
			if( see_monsterMove( hDlg ) )
				EndDialog( hDlg,LOWORD(wParam) );

			return TRUE;
		}

		//! 存入怪物的数据
		if( LOWORD(wParam) == IDC_BUTTON_SAVEMONSTER )
		{
			gc_monsterIdNameMgr::Instance()->save_monsterData();
			EndDialog( hDlg,LOWORD(wParam) );

			return TRUE;
		}
# endif 

		break;

	}
	return FALSE;
}

