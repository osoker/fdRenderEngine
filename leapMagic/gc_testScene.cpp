///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_testScene.cpp
 *
 *  Desc:     ħ��Demo�Ĳ��Գ��������ڲ��Ե��볡�����������������
 * 
 *  His:      River created @ 2006-4-13
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include ".\gc_testscene.h"
# include "lm_configInfoReader.h"
# include "gc_creature.h"
# include "lm_soundMgr.h"
# include "lm_magicEffect.h"
# include "gameApp.h"
# include "deployMonster_mod.h"


LPDIRECT3DTEXTURE9    g_pCharTex = NULL;
LPDIRECT3DSURFACE9    g_pCharTarget = NULL;
LPDIRECT3DSURFACE9    g_pOriTarget = NULL;

//! gameApp.cpp�ڴ���
extern  os_screenPicDisp   g_scrPicDisp;




gc_autoSave::gc_autoSave()
{
	m_fAutoTime = 0.0f;
	m_bHaveSaved = FALSE;
}

//! ÿһ֡����Ҫ���Զ��洢�����frameMove
BOOL gc_autoSave::frame_move( float _etime )
{
	guard;

	BOOL    t_bRes = FALSE;

	m_fAutoTime += _etime;

	if( m_fAutoTime >= __AUTOSAVE_TIME__ )
	{
		t_bRes = TRUE;
		m_fAutoTime = float_mod( m_fAutoTime,__AUTOSAVE_TIME__ );
		m_bHaveSaved = TRUE;
	}

	return t_bRes;

	unguard;
}

//! �õ��Զ��洢Ҫ��ʾ����Ϣ
const char* gc_autoSave::get_displayInfo( void )
{
	static s_string t_szRes = "�Զ��洢ˢ���ĵ�...";

	if( (m_fAutoTime < __INFODIS_TIME__)&&m_bHaveSaved )
		return t_szRes.c_str();
	else
	{
		m_bHaveSaved = FALSE;
		return NULL;
	}
}

gc_testScene::gc_testScene(void)
{
	m_bLoadScene = false;

	m_sSnapshotCamera = NULL;

	m_bFadeInFightScene = false;

# if __DEPLOY_MONSTERMOD__
	m_iPtNum = 0;
	m_iCurDeployAreaIdx = -1;

	m_iCurEditMonsterHandle = -1;
	m_iCurEditMonsterId = -1;

# endif 

}

gc_testScene::~gc_testScene(void)
{
	gc_gameScene::release_scene();
}

//! �����һ��Ĭ�ϵ����λ��
void gc_testScene::cal_defaultCam( void )
{
	guard;

	os_cameraInit   t_sCamInit;

	osVec3D   t_vec3;
	m_ptrScene->get_tilePos( m_iInsertX,m_iInsertZ,t_vec3 );
	t_sCamInit.camFocus = t_vec3;
	t_sCamInit.camFocus.y += 2.0f;

	t_sCamInit.curPos = t_sCamInit.camFocus;
	t_sCamInit.curPos += osVec3D( 10.0f,5.0f,10.0f );

	t_sCamInit.faspect = float(m_iScrWidth)/m_iScrHeight;
	t_sCamInit.tlx = 0;
	t_sCamInit.tly = 0;
	t_sCamInit.wid = this->m_iScrWidth;
	t_sCamInit.hei = this->m_iScrHeight;
	t_sCamInit.min_z = 0.0f;
	t_sCamInit.max_z = 1.0f;

	g_ptrConfigInfoReader->read_camInitData( t_sCamInit );

	osassert( this->m_ptrCamera );
	m_ptrCamera->init_camera( &t_sCamInit );

	unguard;
}



//! ����ˢ����ص���Ϣ
void gc_testScene::load_deployMonsterInfo( const char* _map )
{
	guard;

	gc_monsterAppearance::Instance()->load_globalMaInfo( 
		"ˢ������\\npcMonster\\g_npc_iddir.mol" );
	gc_monsterIdNameMgr::Instance()->init_monstData( 
		"ˢ������\\npcMonster\\g_monsterInfo.mst" );

# if __DEPLOY_MONSTERMOD__
	m_strMapName = _map;
# endif 

	// �Զ�����������ص���Ϣ
	s_string   t_szDmmFileName = "ˢ������\\";
	t_szDmmFileName += _map;
	t_szDmmFileName += ".md";
	//! ����md�ļ����룬������������
	if( file_exist( (char*)t_szDmmFileName.c_str() ) )
		gc_deployMonsterMod::Instance()->lod_dmmData( t_szDmmFileName.c_str() );
	else
		gc_deployMonsterMod::Instance()->reset_dmmData();

	unguard;
}


//! ���������ƶ�������
void gc_testScene::process_spaceMove( lm_command& _cmd )
{
	guard;

	gc_gameScene::process_spaceMove( _cmd );

	lm_spaceMoveCmd*    t_ptrMove = (lm_spaceMoveCmd*)_cmd.m_ptrCommandContent;

	// ������˹����ˢ����Ϣ
# if __DEPLOY_MONSTERMOD__

	// ��ͼ�л�,�����������ݱ��
	if( m_strMapName != t_ptrMove->m_szMapName )
		m_iCurDeployAreaIdx = -1;

	load_deployMonsterInfo( t_ptrMove->m_szMapName );

# endif 

	unguard;
}



/** \brief
*  ���볡����Դ�Ľӿ�.
*
*  \param _lsData  ����һ���������뵽��ǰ���������ݣ���ͬ�ĳ�������Ϊ��ͬ��
*                  ���ݡ�
*/
bool gc_testScene::load_scene( const char* _lsData/* = NULL*/ )
{
	guard;

	os_sceneLoadStruct   t_sSl;
	os_sceneLoadStruct*  t_ptrS = (os_sceneLoadStruct*)_lsData;

	strcpy( t_sSl.m_szMapName,t_ptrS->m_szMapName );
	strcpy( t_sSl.m_szSkyName,t_ptrS->m_szSkyName );
	t_sSl.m_iX = t_ptrS->m_iX;
	t_sSl.m_iY = t_ptrS->m_iY;

	m_iInsertX = t_sSl.m_iX;
	m_iInsertZ = t_sSl.m_iY;

	osassert( m_ptrScene );

	if( !m_ptrScene->load_scene( &t_sSl ) )
		osassert( false );

	//! �����һ��Ĭ�ϵ����λ��
	cal_defaultCam();

	m_bLoadScene = true;

	if( !lm_gvar::g_bSpaceMoveWholeMap )
		gc_gameApp::m_3dMgr->start_fullscrFade( 0xff000000,0x0000,5.0f );
	else
	{
		// ���볡�����Ƿ���Ҫ�õ����������ĵ�ͼ������Ȼ���ƽ����������һ�����ı�����������
		process_travelWholeMap( t_ptrS->m_szMapName );
		lm_gvar::g_bSpaceMoveWholeMap = false;
	}


	// ������˹����ˢ����Ϣ
# if __DEPLOY_MONSTERMOD__
	
	load_deployMonsterInfo( t_ptrS->m_szMapName );

# endif 

	// �����ͼ����ײ��Ϣ
	gc_gameScene::m_ptrPathFinder->LoadMapInfo( t_ptrS->m_szMapName );

	if( gc_gameScene::m_ptrPathFinder->GetCollisionInfo() )
	{
		gc_gameScene::m_ptrScene->set_sceneCollInfo( 
			gc_gameScene::m_ptrPathFinder->GetCollisionInfo() );
	}

	int t_i = gc_gameApp::m_3dMgr->get_availableTextureMem();

	// ��GrowЧ��.
	//gc_gameScene::m_ptrScene->gala_actGrowScene( true );
	//osVec3D   t_vec3Pick;
	//gc_gameScene::m_ptrScene->get_tilePos( 129,128,t_vec3Pick,true );


	return true;

	unguard;
}

/** \brief
*  �ͷų�����Դ�Ľӿ�.
* 
*/
void gc_testScene::release_scene( void )
{
	guard;

	unguard;
}

//! �����snopShot�Ĵ���
void gc_testScene::process_cameraSnapshot( void )
{
	guard;

	// 
	if( !m_bLoadScene )
		return;

	if( m_sSnapshotCamera )
	{
		::get_resourceMgr()->delete_camera( m_sSnapshotCamera );
		m_sSnapshotCamera = NULL;
	}

	m_sSnapshotCamera = m_ptrCamera->snapshot_curCamera();

	return;

	unguard;
}


//! �Գ�������FrameMove
void gc_testScene::frame_moveScene( void )
{
	guard;

	// ������Ч��ص�����
	osassert( m_ptrMeMgr );
	m_ptrMeMgr->frame_moveMeMgr();

	//
	// Creature Manger.
	if( !m_ptrCreatureMgr->frame_move() ) 
		osassert( false );

	os_sceneFM    t_fm;

	t_fm.m_ptrCam = m_ptrCamera->m_ptrCamera;
	m_ptrCamera->m_ptrCamera->get_camFocus( t_fm.m_vecChrPos );
	t_fm.m_iCx = WORD(t_fm.m_vecChrPos.x/1.5f);
	t_fm.m_iCy = WORD(t_fm.m_vecChrPos.z/1.5f);

	m_ptrScene->frame_move( &t_fm );

	osVec3D  t_vec3test = t_fm.m_vecChrPos;
	m_ptrScene->get_detailPos( t_fm.m_vecChrPos,true );

	// �������ڵ���ά����������λ��
	osVec3D   t_vec3Pos,t_vec3Ori;
	gc_camera::m_ptrCamera->get_curpos( &t_vec3Pos );
	gc_camera::m_ptrCamera->get_godLookVec( t_vec3Ori );
	m_ptrSoundMgr->frame_moveListener( t_vec3Pos,t_vec3Ori );

# if __DEPLOY_MONSTERMOD__
	if( m_iCurDeployAreaIdx >= 0 )
	{
		if( m_sAutoSave.frame_move( sg_timer::Instance()->get_lastelatime() ) )
		{
			// ����.
			change_actAreaMonsterPos();
			// �洢ˢ�������ļ�.
			gc_deployMonsterMod::Instance()->save_dmmData();
		}
	}
# endif 


	// TEST CODE:���ԣ�
	framemove_fightSceneFade( sg_timer::Instance()->get_lastelatime() );


	return;

	unguard;
}


/** \brief
 *  �л����غ��ƵĲ��Գ���
 *
 *  �л��ֳɼ������裺
 *  1: ��������
 *  2: ��������ս������������
 *  3: �����ƶ������λ�á�
 *  4: �������������ҿ�ʼս����
 *  5: ����ս����Ч���෴��
 */
void gc_testScene::change_fightScene( void )
{
	guard;

	m_bFadeInFightScene = true;
	m_fFadeTime = 0.0f;

	gc_gameApp::m_3dMgr->start_fullscrFade( 0x00,0xff000000,FIGHT_SCENE_FADETIME/2.0f );


	unguard;
}

//! fadeInս�������Ĵ���
void gc_testScene::framemove_fightSceneFade( float _time )
{
	guard;


	if( !m_bFadeInFightScene )
		return;

	m_fFadeTime += _time;
	
	// �м���̣�����decal,Ȼ�������������
	if( m_fFadeTime >= FIGHT_SCENE_FADETIME/2.0f )
	{

		// TEST CODE:
		static bool t_binit = true;
		if( t_binit )
		{
			os_decalInit       t_init;
			
			t_binit = false;
			t_init.m_bFakeDecal = true;
			t_init.m_dwDecalColor = 0xccffffff;
			t_init.m_fFrameTime = 0.0f;
			t_init.m_fRotAgl = 0.0f;
			t_init.m_fSize = 13.0f;
			t_init.m_fVanishTime = 0.0f;
			strcpy( t_init.m_szDecalTex,"taiji" );
			m_ptrCreatureMgr->get_heroEntity()->get_entityPos( t_init.m_vec3Pos );
			t_init.m_wXFrameNum = 1;
			t_init.m_wYFrameNum = 1;
			
			t_init.m_fYOffset = 0.0f;

			t_init.m_fRotAgl = -OS_PI/4.0f;


			m_ptrScene->create_decal( t_init );

			t_init.m_vec3Pos.y += t_init.m_fYOffset;
			m_ptrCreatureMgr->reset_heroPos( t_init.m_vec3Pos );
		}

		m_bFadeInFightScene = false;
		// ��������ʼfadeOut
		gc_gameApp::m_3dMgr->start_fullscrFade( 0xff000000,0x0,FIGHT_SCENE_FADETIME/2.0f );


	}




	unguard;
}



/** \brief
*  ��Ⱦ�ӿ�.
* 
*/
bool gc_testScene::render_scene( void )
{
	guard;


	// 
	// ����һ����Ϣ����ʼ�����ĵ�����ԣ�
	if( m_bLoadScene )
	{
		osassert( m_ptrScene );

		frame_moveScene();

		m_ptrScene->render_scene( false,false,OS_COMMONRENDER );


# if 0
		// ����������snapShot,�򻭳������snapShot.
		if( m_sSnapshotCamera )
			gc_camera::draw_camera( m_sSnapshotCamera,m_ptrScene );

		// ������ǰ����Ĺ���
		if( m_ptrCreatureMgr )
			m_ptrCreatureMgr->draw_creatureBBox();

# if __DEPLOY_MONSTERMOD__


		//! ���������ڵ��߶�
		if( m_iPtNum > 0 )
		{
			//! ÿһ֡�õ������λ����Ϣ
			for( int t_i=0;t_i<m_iPtNum;t_i ++ )
			{
				m_ptrScene->get_detailPos( m_vec3AreaPt[t_i] );
				m_vec3AreaPt[t_i].y += 1.3f; 
			}
			m_ptrScene->draw_line( m_vec3AreaPt,m_iPtNum,0xffffff00,3.0f );
		}

		// ��������ʾ�����λ�ú���Ұ��Ϣ
		draw_monsterArea();

		//! ��ʾ�Զ��洢ˢ����Ϣ.
		const char*   t_szInfo = m_sAutoSave.get_displayInfo();
		if(  t_szInfo != NULL )
		{
			gc_gameApp::push_debugInfo( t_szInfo,500,54,0xffffff00 );
		}


# endif 

		// �������ڹ��������
		if( lm_gvar::g_bDispMonsterName )
			m_ptrCreatureMgr->draw_creatureName( m_ptrScene );

		// 
		// ��Ⱦһ����ά�Ľ�����ԣ�Ȼ���Ƿ���Լ�����ʾ�������Ч
		if( g_scrPicDisp.tex_id>=0 )
		{
			gc_gameScene::m_ptrScene->push_scrDisEle( &g_scrPicDisp );
			gc_gameScene::m_ptrScene->render_scene( true );
		}

# endif 

		// ��������Ķ�ά��ʾ
		if( lm_gvar::g_bDisplayCharInRect )
		{
			
			RECT   t_rect;
			t_rect.left = 0;
			t_rect.right = 600;
			t_rect.top = 0;
			t_rect.bottom = 450;
            
			gc_gameApp::m_3dMgr->d3d_clear( 0,NULL,D3DCLEAR_ZBUFFER, 
				0x0/*g_dwFarTerrColor*/, 1.0f, 0L );

			gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();
			if( gc_gameScene::m_ptrHeroSkinObj )
				gc_gameScene::m_ptrHeroSkinObj->render_charInScrRect( t_rect,0.1f,2.0f,true );

			t_rect.left = 600;
			t_rect.right = 1000;
			t_rect.bottom = 300;
			if( gc_gameScene::m_ptrHeroSkinObj2 )
				gc_gameScene::m_ptrHeroSkinObj2->render_charInScrRect( t_rect,0.1f,2.0f,true );


# if 0
			// ����.
			if( g_pCharTex == NULL )
			{
				if( FAILED(D3DXCreateTexture(
					gc_gameApp::m_3dMgr->get_d3ddevice(),800,600,1,
					D3DUSAGE_RENDERTARGET,D3DFMT_A8B8G8R8,D3DPOOL_DEFAULT,&g_pCharTex)))
					osassert( false );
				
				if( FAILED( g_pCharTex->GetSurfaceLevel( 0,&g_pCharTarget ) ) )
					osassert( false );				
			}
			// ��Ⱦ
			if( gc_gameScene::m_ptrHeroSkinObj )
			{
				// set device states
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_FOGENABLE, FALSE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


				// setup texture addressing settings
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

				// setup colour calculations
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

				// setup alpha calculations
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

				// setup filtering
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

				// disable texture stages we do not need.
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

				// setup scene alpha blending
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

				// setup ambient color is full value
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_AMBIENT,-1);

				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderState(D3DRS_LIGHTING,FALSE);


				// 
				gc_gameApp::m_3dMgr->get_d3ddevice()->GetRenderTarget( 0,&g_pOriTarget );
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderTarget( 0,g_pCharTarget );

				gc_gameApp::m_3dMgr->d3d_clear( 0,
					NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,0x0,1.0f,0L );

				gc_gameScene::m_ptrHeroSkinObj->render_charInScrRect( 
					t_rect,0.0f,1.0f,true,NULL );
				gc_gameApp::m_3dMgr->get_d3ddevice()->SetRenderTarget( 0,g_pOriTarget );

			}
# endif 

		}

	}



	return true;

	unguard;
}



//! ɾ��������ѡ�еĹ���
void gc_testScene::delete_selMonster( void )
{
	guard;

# if __DEPLOY_MONSTERMOD__	

	if( m_ptrCreatureMgr->get_activeSelMonster() == NULL )
		return;

	gc_deployMonsterMod*   t_ptrDmm = gc_deployMonsterMod::Instance();
	int  t_iMonNum = t_ptrDmm->get_monsterNum( m_iCurDeployAreaIdx );
	gc_creature*           t_ptrCre;
	gc_monsterInsert*      t_ptrMi;
	int                    t_iHandleBack;
	
	for( int t_i=0;t_i<t_iMonNum;t_i ++ )
	{
		t_ptrMi = t_ptrDmm->get_monsterInfoFromIdx( m_iCurDeployAreaIdx,t_i );
		osassert( t_ptrMi );
		t_ptrCre = m_ptrCreatureMgr->get_creaturePtrByHandle( t_ptrMi->m_iMonsterHandle );	

		if( t_ptrCre == m_ptrCreatureMgr->get_activeSelMonster() )
		{
			t_iHandleBack = t_ptrMi->m_iMonsterHandle;
			// ��������ɾ��
			t_ptrDmm->delete_monsterInArea( m_iCurDeployAreaIdx,t_i,t_ptrMi->m_iMonsterHandle );

			// �����豸�������ɾ��
			m_ptrCreatureMgr->del_creatureByHandle( t_iHandleBack );
			m_ptrCreatureMgr->set_nullSelCreature();

			return;
		}
	}

	osassertex( false,"ɾ����������������ڵĹ������...\n" );


# endif 

	unguard;
}

	//! ��ά�������������صĺ���
# if __DEPLOY_MONSTERMOD__

//! �������������Ϣ
void gc_testScene::draw_monsterQuad( osVec3D& _cenPos,float _dis,DWORD _color,int _width/*= 3.0f*/ )
{
	guard;

	int                    t_iPtNum = 0;
	osVec3D                t_vec3Pt[5];

	// ������Χ.
	for( int t_i=0;t_i<4;t_i ++ )
		t_vec3Pt[t_i] = _cenPos;
	t_vec3Pt[0].x -= _dis;
	t_vec3Pt[0].z -= _dis;
	t_vec3Pt[1].x += _dis;
	t_vec3Pt[1].z -= _dis;
	t_vec3Pt[2].x += _dis;
	t_vec3Pt[2].z += _dis;
	t_vec3Pt[3].x -= _dis;
	t_vec3Pt[3].z += _dis;
	for( int t_i=0;t_i<4;t_i ++ )
	{
		m_ptrScene->get_detailPos( t_vec3Pt[t_i] );
		t_vec3Pt[t_i].y += 1.5f;
	}
	t_vec3Pt[4] = t_vec3Pt[0];
	m_ptrScene->draw_line( t_vec3Pt,5,_color,(float)_width );

	unguard;
}

//! ��ѡ�й�Ϸ��������Ϣ
void gc_testScene::draw_activeMonsterQuad( void )
{
	guard;

	// ���㷨����:����ÿһ֡����ô��
	// ��Ұ��Ϣ,�õ������������ÿһ�������id,�õ���id��ָ��,�鿴�Ƿ��ѡ���ָ��һ��,
	// һ�µĻ�,������Ұ��Ϣ
	osVec3D  t_vec3Pos;
	gc_deployMonsterMod*   t_ptrDmm = gc_deployMonsterMod::Instance();
	gc_monsterInsert*      t_ptrMi;
	int  t_iMonNum = t_ptrDmm->get_monsterNum( m_iCurDeployAreaIdx );
	gc_creature*           t_ptrCre;
	int                    t_iPtNum = 0;
	osVec3D                t_vec3Pt[10];
	osVec3D                t_vec3CurPos;

	static gc_creature*    t_sptrCre[128];
	static int             t_iptrCreNum = 0;

	
	t_iptrCreNum = 0;
	t_sptrCre[0] = m_ptrCreatureMgr->get_activeSelMonster();
	t_iptrCreNum ++;

	std::list<gc_creature*>&  t_listPtrCre = m_ptrCreatureMgr->get_multiSel();
	if( t_listPtrCre.size() > 0 )
	{
		std::list<gc_creature*>::iterator  t_iter;
		for( t_iter = t_listPtrCre.begin();t_iter != t_listPtrCre.end();t_iter ++ )
		{
			t_sptrCre[t_iptrCreNum] = (gc_creature*)(*t_iter);
			t_iptrCreNum ++;
		}
	}

	// ���б��ڵ�ÿһ���������Ϣ.
	for( int t_k = 0;t_k<t_iptrCreNum;t_k ++ )
	{

		for( int t_i=0;t_i<t_iMonNum;t_i ++ )
		{
			t_ptrMi = t_ptrDmm->get_monsterInfoFromIdx( m_iCurDeployAreaIdx,t_i );
			osassert( t_ptrMi );
			t_ptrCre = m_ptrCreatureMgr->get_creaturePtrByHandle( t_ptrMi->m_iMonsterHandle );
			if( t_ptrCre == t_sptrCre[t_k] )
			{
				// ���ݵ�ǰ��λ��,���5������
				t_ptrCre->get_entityPos( t_vec3CurPos );

				for( int t_k=0;t_k<4;t_k ++ )
					t_vec3Pt[t_k] = t_vec3CurPos;

				t_vec3Pt[0].x -= t_ptrMi->m_fVisualField;
				t_vec3Pt[0].z -= t_ptrMi->m_fVisualField;
				t_vec3Pt[1].x += t_ptrMi->m_fVisualField;
				t_vec3Pt[1].z -= t_ptrMi->m_fVisualField;
				t_vec3Pt[2].x += t_ptrMi->m_fVisualField;
				t_vec3Pt[2].z += t_ptrMi->m_fVisualField;
				t_vec3Pt[3].x -= t_ptrMi->m_fVisualField;
				t_vec3Pt[3].z += t_ptrMi->m_fVisualField;

				for( int t_k=0;t_k<4;t_k ++ )
				{
					m_ptrScene->get_detailPos( t_vec3Pt[t_k] );
					t_vec3Pt[t_k].y += 1.5f;
				}
				t_vec3Pt[4] = t_vec3Pt[0];
				t_vec3Pt[5] = t_vec3CurPos;t_vec3Pt[5].y += 1.5f;
				t_vec3Pt[6] = t_vec3Pt[2];
				t_vec3Pt[7] = t_vec3Pt[1];
				t_vec3Pt[8] = t_vec3Pt[5];
				t_vec3Pt[9] = t_vec3Pt[3];

				t_iPtNum = 10;
				break;
			}
		}

		//
		if( t_iPtNum > 0 )
		{
			if( lm_gvar::g_bDisVision )
				m_ptrScene->draw_line( t_vec3Pt,t_iPtNum,0xffff0000,3 );

			// ��������
			if( lm_gvar::g_bDisTrigger )
				draw_monsterQuad( t_vec3CurPos,t_ptrMi->m_fTriggerDis,0xff00ff00,2 );

			// ���ܰ뾶.
			if( lm_gvar::g_bDisRun )
				draw_monsterQuad( t_vec3CurPos,t_ptrMi->m_fEscapeDis,0xff0000ff,2 );

			// ����ĺ��о���.
			if( lm_gvar::g_bDisCall )
				draw_monsterQuad( t_vec3CurPos,t_ptrMi->m_fCallDis,0xffffffff,2 );

		}
	}



	return;

	unguard;
}

//! �����������й����������Ϣ
void gc_testScene::draw_allMonsterQuad( void )
{
	guard;

	// ���㷨����:����ÿһ֡����ô��
	// ��Ұ��Ϣ,�õ������������ÿһ�������id,�õ���id��ָ��,�鿴�Ƿ��ѡ���ָ��һ��,
	// һ�µĻ�,������Ұ��Ϣ
	osVec3D  t_vec3Pos;
	gc_deployMonsterMod*   t_ptrDmm = gc_deployMonsterMod::Instance();
	gc_monsterInsert*      t_ptrMi;
	int  t_iMonNum = t_ptrDmm->get_monsterNum( m_iCurDeployAreaIdx );
	gc_creature*           t_ptrCre;
	int                    t_iPtNum = 0;
	osVec3D                t_vec3Pt[10];
	osVec3D                t_vec3CurPos;

	for( int t_i=0;t_i<t_iMonNum;t_i ++ )
	{
		t_ptrMi = t_ptrDmm->get_monsterInfoFromIdx( m_iCurDeployAreaIdx,t_i );
		osassert( t_ptrMi );
		t_ptrCre = m_ptrCreatureMgr->get_creaturePtrByHandle( t_ptrMi->m_iMonsterHandle );

		// ���ݵ�ǰ��λ��,���5������
		t_ptrCre->get_entityPos( t_vec3CurPos );

		for( int t_k=0;t_k<4;t_k ++ )
			t_vec3Pt[t_k] = t_vec3CurPos;

		t_vec3Pt[0].x -= t_ptrMi->m_fVisualField;
		t_vec3Pt[0].z -= t_ptrMi->m_fVisualField;
		t_vec3Pt[1].x += t_ptrMi->m_fVisualField;
		t_vec3Pt[1].z -= t_ptrMi->m_fVisualField;
		t_vec3Pt[2].x += t_ptrMi->m_fVisualField;
		t_vec3Pt[2].z += t_ptrMi->m_fVisualField;
		t_vec3Pt[3].x -= t_ptrMi->m_fVisualField;
		t_vec3Pt[3].z += t_ptrMi->m_fVisualField;

		for( int t_k=0;t_k<4;t_k ++ )
		{
			m_ptrScene->get_detailPos( t_vec3Pt[t_k] );
			t_vec3Pt[t_k].y += 1.5f;
		}
		t_vec3Pt[4] = t_vec3Pt[0];
		t_vec3Pt[5] = t_vec3CurPos;t_vec3Pt[5].y += 1.5f;
		t_vec3Pt[6] = t_vec3Pt[2];
		t_vec3Pt[7] = t_vec3Pt[1];
		t_vec3Pt[8] = t_vec3Pt[5];
		t_vec3Pt[9] = t_vec3Pt[3];

		t_iPtNum = 10;
		if( lm_gvar::g_bDisVision )
			m_ptrScene->draw_line( t_vec3Pt,t_iPtNum,0xffff0000,3 );

		// ��������
		if( lm_gvar::g_bDisTrigger )
			draw_monsterQuad( t_vec3CurPos,t_ptrMi->m_fTriggerDis,0xff00ff00,2 );

		// ���ܰ뾶.
		if( lm_gvar::g_bDisRun )
			draw_monsterQuad( t_vec3CurPos,t_ptrMi->m_fEscapeDis,0xff0000ff,2 );

		// ����ĺ��о���.
		if( lm_gvar::g_bDisCall )
			draw_monsterQuad( t_vec3CurPos,t_ptrMi->m_fCallDis,0xffffffff,2 );

	}

	return;

	unguard;
}



//! ѡ�й�����ص���Ϣ,λ��,��Ұ�ȵ�
void gc_testScene::draw_monsterArea( void )
{
	guard;

# if 0
	os_colorInterpretStr   t_str;

	t_str.charHeight = 12;
	t_str.m_iMaxPixelX = 24;
	t_str.m_iMaxPixelY = 200;
	t_str.m_iPosX = 0;
	t_str.m_iPosY = 100;
	t_str.m_iRowDis = 1;
	t_str = "<&ffffffff:���\n����&>";
	m_ptrScene->disp_string( &t_str );
# endif 

	// ������Ŀ��ʾ:
	if( m_iCurDeployAreaIdx >= 0 )
	{
		gc_gameApp::push_debugInfo( va( "��ǰ�����������: %d",
			gc_deployMonsterMod::Instance()->get_monsterNum( m_iCurDeployAreaIdx ) ),500,38,0xffff0000 );
	}


	// û��ѡ�й���
	if( m_iCurDeployAreaIdx < 0 )
		return;

	// 
	// ��ʾѡ�еĹ������ڵ�λ��
	if( !lm_gvar::g_bDisplayAllMonInfo )
	{
		if( !m_ptrCreatureMgr->get_activeSelMonster() )
			return;
	}

	// ��������ǻ�ȫ��
	if( lm_gvar::g_bDisplayAllMonInfo )
		draw_allMonsterQuad();
	else
		draw_activeMonsterQuad();

	// λ����Ϣ
	osVec3D  t_vec3Pos;
	int      t_x,t_z;

	if( !lm_gvar::g_bDisplayAllMonInfo )
	{
		m_ptrCreatureMgr->get_activeSelMonster()->get_entityPos( t_vec3Pos );
		t_x = (int)(t_vec3Pos.x / HALF_TWIDTH);
		t_z = (int)(t_vec3Pos.z / HALF_TWIDTH);
		gc_gameApp::push_debugInfo( va( "ѡ�й���λ��:<%d,%d>",t_x,t_z ),500,24,0xffff0000 );
	}


	
	unguard;
}


/** \brief
 *  �������
 *
 *  ATTENTION: �ڲ����µ�����֮ǰ������洢��ǰ����������صĹ���ˢ����Ϣ��
 *  �㷨����:
 *  1: ���ڳ����ڻ���������
 *  2: �ƶ�����������������λ��.
 *  3: �ڴ������ڲ��������.
 *
 */
void gc_testScene::deploy_monsterAreaInScene( int _idx )
{
	guard;

	gc_deployMonsterMod*   t_ptrDmm;
	const SPoints*         t_ptrPt;

	// ����µ�idx�͵�ǰ���ڲ����������һ�£��洢��ǰ�Ѿ���������������Ϣ
	if( (m_iCurDeployAreaIdx != _idx)&&(m_iCurDeployAreaIdx>=0) )
	{
		change_actAreaMonsterPos();

		//! ����
		reset_deviceMonster();
	
	}

	// ͬһ�������ܲ�������
	if( m_iCurDeployAreaIdx == _idx )
		return;

	// ���ڼ�����ˢ���������ͼ�Ƿ�һ��
	osVec2D    t_vec2MapSize = m_ptrScene->get_mapSize();

	//! 1: �ȴ�������߽�
	m_iCurDeployAreaIdx = _idx;
	t_ptrDmm = gc_deployMonsterMod::Instance();
	t_ptrPt = t_ptrDmm->get_monAreaGeoData( _idx );
	m_iPtNum = t_ptrPt->mCount;
	for( int t_i=0;t_i<m_iPtNum;t_i ++ )
	{
		m_vec3AreaPt[t_i].x = t_ptrPt->mPoints[t_i].x*HALF_TWIDTH;
		m_vec3AreaPt[t_i].z = t_ptrPt->mPoints[t_i].y*HALF_TWIDTH;

		if( (m_vec3AreaPt[t_i].x > t_vec2MapSize.x)||
			(m_vec3AreaPt[t_i].z > t_vec2MapSize.y)  )
		{
			MessageBox( NULL,"ˢ����Ϣ�ļ��͵�ͼ�ļ���ƥ��","����µ�ͼ�ļ�",MB_OK );
			m_iCurDeployAreaIdx = -1;
			return;
		}


		m_ptrScene->get_detailPos( m_vec3AreaPt[t_i] );

		m_vec3AreaPt[t_i].y += 1.3f;
	}
	m_vec3AreaPt[m_iPtNum] = m_vec3AreaPt[0];
	m_iPtNum ++;

	// 2: �ƶ�������������������.
	POINT   t_pt;
	t_pt.x = 0;t_pt.y = 0;
	for( int t_i=0;t_i<m_iPtNum-1;t_i ++ )
	{
		t_pt.x += t_ptrPt->mPoints[t_i].x;
		t_pt.y += t_ptrPt->mPoints[t_i].y;
	}
	t_pt.x /= (m_iPtNum-1);
	t_pt.y /= (m_iPtNum-1);

	lmc_commandMgr::Instance()->interpret_strToCmd( 
		va( "move %s %d %d",m_strMapName.c_str(),t_pt.x,t_pt.y ) );
		
	//! 3: �ڳ����ڲ�����Ӧ�Ĺ���,�ȵõ��������Ŀ��Ȼ����ÿһ������
	int   t_iMonsterNum = t_ptrDmm->get_monsterNum( _idx );
	gc_monsterInsert*  t_ptrMi;
	char               t_szCmd[128];
	
	for( int t_i=0;t_i<t_iMonsterNum;t_i ++ )
	{
		t_ptrMi = t_ptrDmm->get_monsterInfoFromIdx( _idx,t_i );
		sprintf( t_szCmd,"monster %s %d %d %f %f",t_ptrMi->m_szMonsterDir,
			t_ptrMi->m_sInsertPt.x,t_ptrMi->m_sInsertPt.y,1.0f,t_ptrMi->m_fScale );
		lmc_commandMgr::Instance()->interpret_strToCmd( t_szCmd );
		

		//! �ڳ����ڲ������󣬼�¼�����handle,�����ڱ���ˢ����Ϣʱ�����Եõ������λ����Ϣ
		t_ptrMi->m_iMonsterHandle = lm_gvar::g_iCreatureHandle;

		osassert( lm_gvar::g_iCreatureHandle>= 0 );
	}

	//! �ڳ����ڲ�������,����ѡ�����һֻ����Ϊ�������,���������
	m_ptrCreatureMgr->set_nullSelCreature();


	//! ���������,����������ƶ��ٶ��Զ���Ϊԭ����25��
	lmc_commandMgr::Instance()->interpret_strToCmd( "herospeed 25" );
	g_fInitAutoResetSpeed = 100.0f;


	return;

	unguard;
}

//! ɾ����ǰ�༭����Ĺ�����Ϣ
void gc_testScene::reset_deviceMonster( void )
{
	guard;

	gc_deployMonsterMod*   t_ptrDmm = NULL;

	//! �õ���ǰ�ļ�������
	if( m_iCurDeployAreaIdx >= 0 )
	{
		t_ptrDmm = gc_deployMonsterMod::Instance();
	}

	// ��û�е�������
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;


	//! ����ÿһ�������λ����Ϣ
	int    t_iMonNum = t_ptrDmm->get_monsterNum( m_iCurDeployAreaIdx );
	gc_monsterInsert*  t_ptrMi;
	for( int t_i=0;t_i<t_iMonNum;t_i ++ )	
	{
		t_ptrMi = t_ptrDmm->get_monsterInfoFromIdx( m_iCurDeployAreaIdx,t_i );
		t_ptrMi->m_iMonsterHandle = -1;
	}

	//! ��������������������豸��ع�������
	m_ptrCreatureMgr->del_allCreature();

	unguard;
}


//! ���ļ�������ÿһ�������λ����Ϣ
void gc_testScene::change_actAreaMonsterPos( void )
{
	guard;

	gc_deployMonsterMod*   t_ptrDmm = NULL;

	//! �õ���ǰ�ļ�������
	if( m_iCurDeployAreaIdx >= 0 )
	{
		t_ptrDmm = gc_deployMonsterMod::Instance();
	}

	// ��û�е�������
	if( !t_ptrDmm->is_dmmDataLoaded() )
		return;



	//! ����ÿһ�������λ����Ϣ
	int    t_iMonNum = t_ptrDmm->get_monsterNum( m_iCurDeployAreaIdx );
	gc_monsterInsert*  t_ptrMi;
	gc_creature*       t_ptrC;
	osVec3D            t_vec3Pos;
	
	for( int t_i=0;t_i<t_iMonNum;t_i ++ )
	{
		t_ptrMi = t_ptrDmm->get_monsterInfoFromIdx( m_iCurDeployAreaIdx,t_i );
		
		osassert( t_ptrMi->m_iMonsterHandle >= 0 );

		t_ptrC = m_ptrCreatureMgr->get_creaturePtrByHandle( t_ptrMi->m_iMonsterHandle );

		osassert( t_ptrC );

		t_ptrC->get_entityPos( t_vec3Pos );
		
		t_ptrMi->m_sInsertPt.x = int( t_vec3Pos.x / HALF_TWIDTH );
		t_ptrMi->m_sInsertPt.y = int( t_vec3Pos.z / HALF_TWIDTH );
	}

	t_ptrDmm->update_dmmPosInfo();

	return;

	unguard;
}

	
//! �ڳ����ڲ鿴������ƶ�����
void gc_testScene::see_monsterMoveInScene( int _monIdx )
{
	guard;

	osassert( _monIdx >= 0 );

	// �����ǰ��handle��Ч,��ɾ����ǰ��handle.
	if( m_iCurEditMonsterHandle >= 0 )
		m_ptrCreatureMgr->del_creatureByHandle( m_iCurEditMonsterHandle );

	// �ٴ����µ�handle
	SMonstData* t_ptrMon = gc_monsterIdNameMgr::Instance()->get_monsterFromIdx( _monIdx );
	DWORD   t_dwVF,t_dwTri,t_dwEsc,t_dwCall;
	float   t_fScale;

	//! �ӹ����id�õ������Ӧ��Ŀ¼��
	s_string  t_szMonDir = gc_monsterIdNameMgr::Instance()->get_monstDirFromId( 
		t_ptrMon->m_dwMonstId,t_dwVF,t_dwTri,t_dwEsc,t_dwCall,t_fScale );
	t_szMonDir = gc_deployMonsterMod::Instance()->
		process_monsterDir( (char*)t_szMonDir.c_str() );

	char   t_szCmd[128];

	sprintf( t_szCmd,"monster %s %d %d %f %f",t_szMonDir.c_str(),
		lm_gvar::g_iHeroPosX + 1,lm_gvar::g_iHeroPosZ,1.0f,t_fScale );


	lmc_commandMgr::Instance()->interpret_strToCmd( t_szCmd );
	
	//! �ڳ����ڲ������󣬼�¼�����handle,���ڸ�����һ֡�Ĺ���
	m_iCurEditMonsterHandle = lm_gvar::g_iCreatureHandle;
	osassert( m_iCurEditMonsterHandle >= 0 );

	float   t_fActTime;
	float   t_fRunSpeed = float(t_ptrMon->m_dwBuFu) / 1000.0f;

	gc_monster*  t_ptrCre = (gc_monster*)m_ptrCreatureMgr->get_activeSelMonster();
	osassert( t_ptrCre );

	if( lm_gvar::g_bNormalMoveTime )
	{
		t_ptrCre->set_walkActionName( "��·" ); 
		t_fActTime = float(t_ptrMon->m_dwZhengChangYiDongJianGe)/1000.0f;
	}
	else
	{
		t_ptrCre->set_walkActionName( "�ܲ�" );
		t_fActTime = float(t_ptrMon->m_dwZhuiJiYiDongJianGe)/1000.0f;
	}

	t_ptrCre->set_monstWalkInfo( t_fRunSpeed,t_fActTime );

	unguard;
}


//! �ڳ����ڹ��ﲿ���������һ������
void gc_testScene::insert_monsterInScene( int _midx )
{
	guard;

	gc_deployMonsterMod*   t_ptrDmm = NULL;

	t_ptrDmm = gc_deployMonsterMod::Instance();

	if( (m_iCurDeployAreaIdx < 0)||(!t_ptrDmm->is_dmmDataLoaded()) )
		return;

	//! �ڹ��ﲿ��༭���ڼ���˹���,�Ƚ��鷳
	gc_monsterInsert*  t_ptrMi = NULL;
	t_ptrMi = t_ptrDmm->insert_monsterInArea( 
		m_iCurDeployAreaIdx,_midx,lm_gvar::g_iHeroPosX,lm_gvar::g_iHeroPosZ );

	//! �ȴ���һ���ڳ����ڿ�����ʾ�Ĺ���.	
	char               t_szCmd[128];
	sprintf( t_szCmd,"monster %s %d %d %f %f",t_ptrMi->m_szMonsterDir,
		lm_gvar::g_iHeroPosX,lm_gvar::g_iHeroPosZ,1.0f,t_ptrMi->m_fScale );
	lmc_commandMgr::Instance()->interpret_strToCmd( t_szCmd );

	//! �ڳ����ڲ������󣬼�¼�����handle,�����ڱ���ˢ����Ϣʱ�����Եõ������λ����Ϣ
	t_ptrMi->m_iMonsterHandle = lm_gvar::g_iCreatureHandle;

	osassert( t_ptrMi->m_iMonsterHandle >= 0 );

	return;

	unguard;
}

# endif 
