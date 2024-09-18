///////////////////////////////////////////////////////////////////////////////////////////
/** \file
*  Filename: consoleMisk.cpp
*
*  Desc:     ħ��demoConsole����ش�����Ҫ��һ��about�ĶԻ����ڣ����ϸ��ָ����Ĺ���
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



// ��¼���Ӳ��
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


//! �õ�Ҫ������������
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

//! ִ������
void  exec_command( const char* _cmd )
{
	if( lmc_commandMgr::Instance()->interpret_strToCmd( _cmd ) )
		write_cmdToHd( _cmd );
	else
		MessageBox( NULL,"ִ������ʧ��....","ERROR",MB_OK );
}

// ����ִ�������
void loadAndExecCmdList( HWND  _hwnd )
{
	TCHAR filenamebuffer[256];
	char   backdir[256];

	// ����ѡ���ļ��ĶԻ���
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
	// ���ļ�,��ѡ��������б��ļ��������������
	if( GetOpenFileName( &ofn ) )
	{
		lmc_commandMgr::Instance()->load_execCmdFile( filenamebuffer );
		::chdir( backdir );
	}

	return;

}


// ��������Ĺ��������������,��������
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
	//! ������ˢ����Ϣ�Ƿ��Ѿ�����
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

		// ��ʼ�����ﲥ��������
		init_monsterInsertCombo( _hDlg );

	}

	//! ��ʼ����ʾ����߿�ĸ�ѡ��ť
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

//! �������id�༭���ڵ�����
void  reset_monsterId( HWND _hdlg )
{
	guard;

# if __DEPLOY_MONSTERMOD__
	osassert( _hdlg );
	HWND    t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_MONSTERNAME );
	int     t_idx = (int)::SendMessage( t_hwnd,(UINT) CB_GETCURSEL,0,0 );
	char    t_sz[128];

	// û�е�����Ӧ������.
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

		// �ƶ����.
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

//! ����id��ı仯,�仯�����Ľ���
void  reset_interfaceFromId( HWND _hdlg )
{
	guard;
	osassert( _hdlg );

	char    t_szId[128];
	int     t_id;

	// û�е�����Ӧ������.
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
	// �ƶ����.
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

//! �����Ƿ��ù��������ƶ�,��������صĽ���
void  reset_interfaceFromNormalMove( HWND _hdlg )
{
	guard;

# if __DEPLOY_MONSTERMOD__
	osassert( _hdlg );
	lm_gvar::g_bNormalMoveTime = !lm_gvar::g_bNormalMoveTime;

	// û�е�����Ӧ������.
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
	// �ƶ����.
	if( lm_gvar::g_bNormalMoveTime )
	{
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhengChangYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"�����ƶ����:" );
	}
	else
	{
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"׷���ƶ����:" );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhuiJiYiDongJianGe );
	}
	::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
# endif 

	unguard;
}

//! �Ӽ����༭��õ����������,��������ά��������ش���
bool  see_monsterMove( HWND  _hdlg )
{
	guard;

# if __DEPLOY_MONSTERMOD__
	// û�е�����Ӧ������.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return true;

	// ��ǰ���ڱ༭�Ĺ���ָ��
	if( gc_gameApp::m_ptrActiveScene->get_curEditMonsterId() < 0 )
	{
		MessageBox( NULL,"ID����,��ѡ�����..\n","ERROR",MB_OK );
		return false;
	}
	int   t_id = gc_gameApp::m_ptrActiveScene->get_curEditMonsterId();
	if( t_id < 0 )
	{
		MessageBox( NULL,"ID����,��ѡ�����..\n","ERROR",MB_OK );
		return false;
	}

	int   t_idx = 
		gc_monsterIdNameMgr::Instance()->get_monsterIdxFromId( t_id );
	SMonstData*   t_ptrMonster = 
		gc_monsterIdNameMgr::Instance()->get_monsterFromIdx( t_idx );
	if( !t_ptrMonster )
	{
		MessageBox( NULL,"ID����,��ѡ�����..\n","ERROR",MB_OK );
		return false;
	}

	// �ȵõ��ƶ����.
	char    t_sz[128];
	int     t_iTmp;
	::GetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz,128 );
	t_iTmp = atoi( t_sz );
	if( t_iTmp < 0 )
	{
		MessageBox( NULL,"�ƶ�������ݳ���","ERROR",MB_OK );
		return false;
	}
	if( lm_gvar::g_bNormalMoveTime )
		t_ptrMonster->m_dwZhengChangYiDongJianGe = t_iTmp;
	else
		t_ptrMonster->m_dwZhuiJiYiDongJianGe = t_iTmp;

	// �õ���������
	::GetDlgItemText( _hdlg,IDC_EDIT_STEPLENGTH,t_sz,128 );
	t_iTmp = atoi( t_sz );
	if( t_iTmp < 0 )
	{
		MessageBox( NULL,"�ƶ�������ݳ���","ERROR",MB_OK );
		return false;
	}
	t_ptrMonster->m_dwBuFu = t_iTmp;

	// ���ó����Ĵ�����:
	gc_gameApp::m_ptrActiveScene->see_monsterMoveInScene( t_idx );
# endif 

	return TRUE;

	unguard;
}

//! ��ʼ�������ƶ��ٶȲ鿴�ı༭��
void  init_monsterMoveInterface( HWND _hdlg )
{
	guard;

	osassert( _hdlg );
	char     t_sz[256];

# if __DEPLOY_MONSTERMOD__
	// û�е�����Ӧ������.
	gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;

	// �ڹ�����������ڼ�����������
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

	// ���Ĺ���id�༭��
	SMonstData*   t_ptrMonster = t_ptrIdNameMgr->get_monsterFromIdx(t_iCurSel);
	osassert( t_ptrMonster );
	sprintf( t_sz,"%d",t_ptrMonster->m_dwMonstId );
	::SetDlgItemText( _hdlg,IDC_EDIT_MONSTERID,t_sz );

	// �Ƿ��Ǳ༭���������ƶ�
	// ::SendMessage( t_hwnd,(UINT) BM_GETCHECK,0,0 ) == BST_CHECKED
	t_hwnd = ::GetDlgItem( _hdlg,IDC_CHECK_NORMALMOVE );
	if( lm_gvar::g_bNormalMoveTime )
	{
		::SendMessage( t_hwnd,(UINT)BM_SETCHECK,BST_CHECKED,0 );
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"�����ƶ����:" );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhengChangYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
	}
	else
	{
		::SendMessage( t_hwnd,(UINT)BM_SETCHECK,BST_UNCHECKED,0 );
		::SetDlgItemText( _hdlg,IDC_STATIC_MOVETIME,"׷���ƶ����:" );
		sprintf( t_sz,"%d",t_ptrMonster->m_dwZhuiJiYiDongJianGe );
		::SetDlgItemText( _hdlg,IDC_EDIT_MOVETIME,t_sz );
	}

	sprintf( t_sz,"%d",t_ptrMonster->m_dwBuFu );
	::SetDlgItemText( _hdlg,IDC_EDIT_STEPLENGTH,t_sz );
# endif 

	unguard;
}

//����ʾ��ʼ�������fov.
void  init_cameraFov( HWND _hdlg )
{
	guard;

	//���ȵõ�edit��������ַ�
	char   t_sz[256];
	float  t_fFov;

	t_fFov = gc_camera::m_ptrCamera->get_cameraFov();
	sprintf( t_sz,"%f",t_fFov );

	::SetDlgItemText( _hdlg,IDC_EDIT_FOVMOD,t_sz );

	unguard;
}

//! �����ʼ����һ�δ򿪿���̨
static bool  t_sbFirstConsole = true;

void  add_cmdToCmdList( HWND hDlg )
{
	HWND    t_hwnd;

	t_hwnd = ::GetDlgItem( hDlg,IDC_COMBOCMD );

	CIniFile   t_file;

	if( !t_file.OpenFileRead( INIT_CMDFILENAME ) )
	{
		MessageBox( NULL,"�򿪳�ʼ�������ļ�ʧ��","�Ҳ����ļ�",MB_OK );

		::SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)"move zyla 128 128" );

		::SendMessage( t_hwnd,CB_SETCURSEL,0 ,0 );
		return;
	}
	else
	{
		// ���������б�
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

	// ʹ�ÿ������崦��ȫѡ�е�״̬
	char  t_szCmdCont[128];
	::GetDlgItemText( hDlg,IDC_COMBOCMD,t_szCmdCont,128 );
	::SendMessage( t_hwnd,EM_SETSEL,0,strlen( t_szCmdCont ) ); 

	// ��ʼ�������ص����ݡ�
	init_cameraFov( hDlg );

# if __DEPLOY_MONSTERMOD__

	init_deployMonsterInterface( hDlg );

	//! �����ƶ��ٶȲ鿴�Ľ����ʼ��
	init_monsterMoveInterface( hDlg );

# endif 

}

//! �õ���ѡ�������,����ȫ�ֱ���
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

//! ����һ������ˢ�µ������ļ�
BOOL load_deployMonsterData( HWND _hDlg )
{
	guard;

	TCHAR filenamebuffer[256];
	char   backdir[256];

	HWND    t_hwnd;

	t_hwnd = ::GetDlgItem( _hDlg,IDC_COMBO_AREALIST );

	// ����ѡ���ļ��ĶԻ���
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
	// ���ļ�,��ѡ��������б��ļ��������������
	if( GetOpenFileName( &ofn ) )
	{
		gc_deployMonsterMod*  t_ptrDmm = gc_deployMonsterMod::Instance();
		t_ptrDmm->lod_dmmData( filenamebuffer );
		::chdir( backdir );

		// �õ��������Ϣ
		int   t_iAreaNum = t_ptrDmm->get_monsterAreaNum();

		::SendMessage( t_hwnd,CB_RESETCONTENT,0,0 );
		for( int t_i=0;t_i<t_iAreaNum;t_i ++ )
			::SendMessage( t_hwnd,CB_ADDSTRING,0,(LPARAM)t_ptrDmm->get_monsterAreaName( t_i ) );	
		::SendMessage( t_hwnd,CB_SETCURSEL,0 ,0 );

	}

	return TRUE;

	unguard;
}

//! ����һ������ˢ������
BOOL deploy_monsterInArea( HWND _hdlg )
{
	//! ���ݵ�ǰ�������ѡ��,������ǰ��������Ϣ
	HWND   t_hwnd = ::GetDlgItem( _hdlg,IDC_COMBO_AREALIST );
	int    t_idx = (int)::SendMessage( t_hwnd,(UINT) CB_GETCURSEL,0,0 );

# if __DEPLOY_MONSTERMOD__
	//! �����ڴ���ˢ��������ص�����
	if( t_idx >= 0 )
		gc_gameApp::m_ptrActiveScene->deploy_monsterAreaInScene( t_idx );
# endif 

	return TRUE;
}

//! ��ʾĳһ��λ��ѡ�е�������
int display_seltri( HWND _hdlg )
{
	char   t_sz[64];
	int    t_iParId;
	int    t_iSel[4];

	
	::GetDlgItemText( _hdlg,IDC_EDIT_SEL1,t_sz,64 );
	t_iParId = atoi( t_sz );
	if( (t_iParId<0)||(t_iParId>7) )
	{
		MessageBox( NULL,"������ָ�����岿λID...\n","ERROR",MB_OK );
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

//! ���������fov�޸�
void change_cameraFov( HWND _hdlg )
{
	guard;

	//  
	//���ȵõ�edit��������ַ�
	char   t_sz[256];
	::GetDlgItemText( _hdlg,IDC_EDIT_FOVMOD,t_sz,256 );
	float  t_fFov = (float)atof( t_sz );

	gc_camera::m_ptrCamera->set_cameraFov( t_fFov );

	return;

	unguard;
}


// �����ڡ������Ϣ�������
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
			// �����Ի���
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

		//! ���²������İ�ť
		if( LOWORD(wParam) == IDC_BTN_INSERTMON )
		{
			get_monsterInsertCmd( t_szLastCmd );
			exec_command( t_szLastCmd );
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! �����ϴ���ȷ���ŵļ���.
		if( LOWORD(wParam) == IDC_BTN_PLAYLASTSKILL )
		{
			get_lastPlaySkillCmd( t_szLastCmd );
			exec_command( t_szLastCmd );
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! ����ִ�������.
		if( LOWORD(wParam) == IDC_BTN_EXECMDLIST )
		{
			loadAndExecCmdList( hDlg );
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;			
		}

		//  
		//! ���Ҫִ��fov���޸�
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
			// �õ��༭���ڵ��ִ������������Ӧ������,���������ȷ��
			// ��¼���Ϊ�´�console��Ĭ�����
			char     t_szCmd[256];
			::GetDlgItemText( hDlg,IDC_COMBOCMD,t_szCmd,256 );
			exec_command( t_szCmd );

			// �����Ի���
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		//! ��ʾѡ�в�λ��ѡ������
		if( LOWORD(wParam) == IDC_BUTTON_DISEL )
		{
			if( display_seltri( hDlg ) == 0 )
				return FALSE;

			// �����Ի���
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;			
		}

		//! ˢ�ֱ༭���༭��ص���Ϣ
# if __DEPLOY_MONSTERMOD__

		// ����
		if( LOWORD(wParam) == IDC_BTN_DEPLOY_CURAREA )
		{
			::deploy_monsterInArea( hDlg );
			EndDialog( hDlg,LOWORD(wParam) );
			return TRUE;
		}
		// �洢
		if( LOWORD(wParam) == IDC_BTN_SAVE_DEPLOYDATA )
		{
			gc_gameApp::m_ptrActiveScene->change_actAreaMonsterPos();
			gc_deployMonsterMod::Instance()->save_dmmData();
			MessageBox( NULL,"�洢ˢ����Ϣ�ļ��ɹ�","INFO",MB_OK );

			EndDialog( hDlg,LOWORD(wParam) );
			return true;
		}

		// ��������ڵ�ǰ�༭��������
		if( LOWORD(wParam) == IDC_BTN_INSERT_MONSTER )
		{
			//! ���ݵ�ǰ�������ѡ��,������ǰ��������Ϣ
			HWND   t_hwnd = ::GetDlgItem( hDlg,IDC_COMBO_MONTYPE );
			int    t_idx = (int)::SendMessage( t_hwnd,(UINT) CB_GETCURSEL,0,0 );

			gc_gameApp::m_ptrActiveScene->insert_monsterInScene( t_idx );
			
			EndDialog( hDlg,LOWORD(wParam) );
			return true;			
		}

		// ������ʾ
		if( LOWORD(wParam) == IDC_BUTTON_CHNAGEDIS )
		{
			process_changeMonsterDis( hDlg );

			EndDialog( hDlg,LOWORD(wParam) );
			return true;	
		}

		// ���ݹ����޸Ľ������,�����Ľ������ʾ
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

		//! �鿴�����ƶ�:�ڳ����ڼ�����ƶ�����
		if( LOWORD(wParam) == IDC_BUTTON_SEEMONMOVE )
		{
			if( see_monsterMove( hDlg ) )
				EndDialog( hDlg,LOWORD(wParam) );

			return TRUE;
		}

		//! ������������
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

