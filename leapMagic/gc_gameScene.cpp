///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_gameScene.h
 *
 *  Desc:     ��Ϸ�����Ļ��࣬��Ϸ���õ���ÿһ������������Ҫ�����������������������ͬ
 *            �ĳ�����ص���Ϣ��
 * 
 *  His:      River created @ 2004-4-12 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include ".\gc_gamescene.h"
# include "gc_creature.h"
# include "lmc_infoDisplay.h"
# include "lm_soundMgr.h"
# include "gameApp.h"
# include "lm_magicEffect.h"
# include "lm_configInfoReader.h"
# include "gc_itemShapeMgr.h"
# include "lm_stateMagic.h"

//@{
//!  ���ڵĳ�����Ϣ��
HWND		gc_gameScene::m_hWnd = NULL;
HINSTANCE	gc_gameScene::m_hIns = NULL;
int			gc_gameScene::m_iScrWidth = 800;
int			gc_gameScene::m_iScrHeight = 600;
//@} 

static os_sceneCursorInit     g_sCursorInit;


//! ����ײ㳡����ָ��,ȫ��ֻ��һ��ʵ��.
I_fdScene*	 gc_gameScene::m_ptrScene = NULL;

//! TEST CODE:
I_skinMeshObj* gc_gameScene::m_ptrHeroSkinObj = NULL;
I_skinMeshObj* gc_gameScene::m_ptrHeroSkinObj2 = NULL;

//! ��������ܵĽӿڡ�
I_resource*	 gc_gameScene::m_ptrResource = NULL;

//! ��ǰ��socket������.
I_socketMgr* gc_gameScene::m_ptrSocket = NULL;

//! ȫ�ֵ������
gc_camera*   gc_gameScene::m_ptrCamera = NULL;

//! ȫ�ֵ����������
gc_creatureMgr* gc_gameScene::m_ptrCreatureMgr = NULL;

//! ȫ�ֵ��������Ź�����
lm_soundMgr*    gc_gameScene::m_ptrSoundMgr = NULL;
//! ȫ�ֵļ��ܹ�����
lm_meManager*   gc_gameScene::m_ptrMeMgr = NULL;

//! ȫ�ֵ�װ��������ָ��
gc_itemMgr* gc_gameScene::m_ptrItemMgr = NULL;
//! ȫ�ֵ�����״ָ̬��
CStateIconMgr* gc_gameScene::m_ptrStateMgr = NULL;


//! ��ͼѰ·�㷨���
CFindPathMap* gc_gameScene::m_ptrPathFinder = NULL;




gc_gameScene::gc_gameScene(void)
{
# if __COLL_MOD__

	for( int t_i=0;t_i<__COLL_DISGRID__*__COLL_DISGRID__;t_i ++ )
		m_vecColDecalId[t_i] = -1;



	m_sLastPickIdx.m_tgIdx.m_iX = 0xffff;
	m_sLastPickIdx.m_tgIdx.m_iY = 0xffff;

# endif 

}

gc_gameScene::~gc_gameScene(void)
{
	release_scene();
}


//! ��ʱִ������ķ���ֵ
int gc_gameScene::m_iCmdImmResCode = -1;

//! ���ú͵õ���ʱ����ķ��ؽ��
void gc_gameScene::set_immCmdCode( int _code )
{
	m_iCmdImmResCode = _code;
}

int gc_gameScene::get_immCmdCode( void )
{
	return m_iCmdImmResCode;
}


/** \brief
*  ȫ�ֵĳ�ʼ����ǰ���泡��ָ��ͽ��������ָ��ġ�
*
*  Ҫ���������ʼ���ĵط������������
* \param gcs_sceneOnceInit& _init  ��ʼ���ṹ
* \return bool
*/
bool gc_gameScene::init_gameScene( gcs_sceneOnceInit& _init )
{
	guard;

	osassert( _init.m_hWnd );
	osassert( _init.m_hIns );
	osassert( _init.m_iScrWidth > 0 );
	osassert( _init.m_iSceHeight > 0 );

	m_hWnd = _init.m_hWnd;
	m_hIns = _init.m_hIns;
	m_iScrWidth = _init.m_iScrWidth;
	m_iScrHeight = _init.m_iSceHeight;
	
	//
	// �õ���Ⱦ����ĳ���ָ�롣
	m_ptrScene = ::get_sceneMgr();
	osassert( m_ptrScene );
	
	// ʵ����ȫ�ֵ����
	m_ptrCamera = new gc_camera;

	// 
	m_ptrCreatureMgr = new gc_creatureMgr;

	// ����������
	m_ptrSoundMgr = new lm_soundMgr;
	m_ptrSoundMgr->init_soundMgr( _init.m_hWnd );

	// ���ܹ�����
	m_ptrMeMgr = new lm_meManager;
	m_ptrMeMgr->init_skillList( NULL );

	// װ��������
	m_ptrItemMgr = gc_itemMgr::Instance();
	m_ptrItemMgr->init_itemMgr();

	// ״̬������
	/*
	m_ptrStateMgr = new CStateIconMgr();
	m_ptrStateMgr->init();
	*/

	// Ѱ����ص�����
    m_ptrPathFinder = new CFindPathMap;


	// �����ڳ����ڻ���ͼ��
	g_ptrConfigInfoReader->read_cursorDrawInScene( g_sCursorInit );


	return true;

	unguard;
}


/** \brief
*  �ͷų�����Դ�Ľӿ�.
* 
*/
void gc_gameScene::release_scene( void )
{

	SAFE_DELETE( m_ptrCamera );
	SAFE_DELETE( m_ptrCreatureMgr );
	SAFE_DELETE( m_ptrSoundMgr );

	return;
}


/** \brief
*  ���볡����Դ�Ľӿ�.
*
*  \param _lsData  ����һ���������뵽��ǰ���������ݣ���ͬ�ĳ�������Ϊ��ͬ��
*                  ���ݡ�
*/
bool gc_gameScene::load_scene( const char* _lsData/* = NULL*/ )
{
	return true;
}


/** \brief
*  ��Ⱦ�ӿ�.
* 
*/
bool gc_gameScene::render_scene( void )
{
	return true;
}


# if __COLL_MOD__
//! ������ײ��Ϣ��ʾ��decal�б�
void gc_gameScene::construct_decalList( os_pickTileIdx& _pickIdx )
{
	guard;


	int t_iOffset = __COLL_DISGRID__/2;
	os_tileIdx      t_sMousePos;
	float t_fTgOffsetX = _pickIdx.m_tgIdx.m_iX * TGMAP_WIDTH;
	float t_fTgOffsetZ = _pickIdx.m_tgIdx.m_iY * TGMAP_WIDTH;
	os_decalInit       t_init;

	// �м򵥵Ķ���
	t_init.m_fVanishTime = 10000.0f;
	t_init.m_fRotAgl = 0.0f;
	t_init.m_fSize = 0.7f;
	strcpy( t_init.m_szDecalTex,"s_animCursor" );
	t_init.m_wXFrameNum = 3 ;
	t_init.m_wYFrameNum = 3;
	t_init.m_fFrameTime = 0.15f;


	// ��ɾ����ǰdecal�б������е�decal.
	for( int t_i =0;t_i<__COLL_DISGRID__*__COLL_DISGRID__;t_i ++ )
	{
		if( m_vecColDecalId[t_i]>= 0 )
		{
			m_ptrScene->delete_decal( m_vecColDecalId[t_i] );
			m_vecColDecalId[t_i] = -1;
		}
	}


	for( int t_x=_pickIdx.m_colTileIdx.m_iX-t_iOffset;
		 t_x<=_pickIdx.m_colTileIdx.m_iX+t_iOffset;t_x ++ )
	{	
		if( t_x < 0 )
			continue;
		if( t_x >= GROUNDMASK_WIDTH )
			continue;

		for( int t_z=_pickIdx.m_colTileIdx.m_iY-t_iOffset;
			 t_z<=_pickIdx.m_colTileIdx.m_iY+t_iOffset;t_z ++ )
		{
			if( t_z < 0 )
				continue;
			if( t_z >= GROUNDMASK_WIDTH )
				continue;

			// ����λ��������Decal.
			t_init.m_vec3Pos.x = t_x*HALF_TWIDTH + t_fTgOffsetX + HALF_CTWIDTH;
			t_init.m_vec3Pos.z = t_z*HALF_TWIDTH + t_fTgOffsetZ + HALF_CTWIDTH;

			// ���ݴ�λ���Ƿ���ײ�����õ���Ӧ�Ŀ�ͨ����Ϣ
			t_sMousePos.m_iX = _pickIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + t_x;
			t_sMousePos.m_iY = _pickIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + t_z;
			if( m_ptrScene->collision_detection( t_sMousePos ) )
				t_init.m_dwDecalColor = 0xffffff00;
			else
				t_init.m_dwDecalColor = 0xffff0000;


			// �õ�����:
			int t_iIdxX = t_x - _pickIdx.m_colTileIdx.m_iX + t_iOffset;
			int t_iIdxZ = t_z - _pickIdx.m_colTileIdx.m_iY + t_iOffset;

			m_vecColDecalId[t_iIdxZ*__COLL_DISGRID__+t_iIdxX] = 
				m_ptrScene->create_decal( t_init );

			osassert( m_vecColDecalId[t_iIdxZ*__COLL_DISGRID__+t_iIdxX] >= 0 );
		}
	}

# if __COLL_MOD__
	// ���浱ǰ�ĵ��λ�ã������޸ĺ������ײ��Ϣ����ʾ
	memcpy( &m_sLastPickIdx,&_pickIdx,sizeof( os_pickTileIdx ) );
# endif 

	unguard;
}
# endif 

//! ɾ�������ڿɼ�����ײDecal�б�
void gc_gameScene::reset_colDecal( void )
{
	guard;

# if __COLL_MOD__
	//! ɾ�����е�decalId.
	for( int t_i=0;t_i<__COLL_DISGRID__*__COLL_DISGRID__;t_i++ )
	{
		if( m_vecColDecalId[t_i]>= 0 )
		{
			m_ptrScene->delete_decal( m_vecColDecalId[t_i] );
			m_vecColDecalId[t_i] = -1;
		}
	}
# endif 

	unguard;
}




//! ����������ƶ���Ϣ�Ĵ���
void gc_gameScene::on_mousemoveInScene( lm_command& _cmd )
{
	guard;

	// �����ǲ������ı༭�������������.
# if __DEPLOY_MONSTERMOD__

	//! �����ʾ��ײ��Ϣ�Ŀ��عص�,��ʲô������Ҫ����.
	if( lm_gvar::g_bDisplayColInfo == FALSE )
		return;

	os_tileIdx      t_sMousePos;
	os_pickTileIdx  t_pickIdx;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Pick;

	t_sMousePos = *(os_tileIdx*)_cmd.m_ptrCommandContent;
	osassert( (t_sMousePos.m_iX>=0)&&(t_sMousePos.m_iY>=0) );

	osassert( m_ptrScene );
	if( !m_ptrScene->get_pickTileIdx( t_pickIdx,t_sMousePos,&t_vec3Pick ) )
		return;

	// 
	// ���ر����ʾ���
	t_sMousePos.m_iX = t_pickIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + t_pickIdx.m_colTileIdx.m_iX;
	t_sMousePos.m_iY = t_pickIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + t_pickIdx.m_colTileIdx.m_iY;

	g_sCursorInit.m_vec3CursorPos = t_vec3Pick;
	if( m_ptrScene->collision_detection( t_sMousePos ) )
		g_sCursorInit.m_dwCursorColor = 0xffffff00;
	else
		g_sCursorInit.m_dwCursorColor = 0xffff0000;

	g_sCursorInit.m_vec3CursorPos.x = t_sMousePos.m_iX * 1.5f + 0.75f;
	g_sCursorInit.m_vec3CursorPos.z = t_sMousePos.m_iY * 1.5f + 0.75f;
# if __COLL_MOD__
	// �����˸�����Χ��ײ��Ϣ��decal�б����б�Ϊȫ�ֱ���
	construct_decalList( t_pickIdx );
# endif

# endif 

	unguard;
}

//! �������£���ʾ�����ڵ�Decal
void gc_gameScene::on_lbuttonDownInScene( lm_command& _cmd )
{
	guard;

	os_tileIdx      t_sMousePos;
	os_pickTileIdx  t_pickIdx;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Pick;

	t_sMousePos = *(os_tileIdx*)_cmd.m_ptrCommandContent;
	osassert( (t_sMousePos.m_iX>=0)&&(t_sMousePos.m_iY>=0) );


# if __DEPLOY_MONSTERMOD__

	// River mod @ 2007-3-9:
	// ���ù������������
	m_ptrCreatureMgr->process_command( _cmd );

# endif 


	osassert( m_ptrScene );
	if( !m_ptrScene->get_pickTileIdx( t_pickIdx,t_sMousePos,&t_vec3Pick ) )
		return;

	// 
	// ���ر����ʾ���
	t_sMousePos.m_iX = t_pickIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + t_pickIdx.m_colTileIdx.m_iX;
	t_sMousePos.m_iY = t_pickIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + t_pickIdx.m_colTileIdx.m_iY;

	g_sCursorInit.m_vec3CursorPos = t_vec3Pick;
	if( m_ptrScene->collision_detection( t_sMousePos ) )
		g_sCursorInit.m_dwCursorColor = 0xffffff00;
	else
		g_sCursorInit.m_dwCursorColor = 0xffff0000;
	
# if __COLL_MOD__

	if( lm_gvar::g_bAdjMapCollInfo )
	{
		//
		// River @ 2007-1-5: ������ײ�޸���صĹ��ܡ�
		// ��ײ�޸Ĺ����㷨��
		// 1: ���λ����ʾ����ײ���ӵ����ġ�
		// 2: ��ʾ���λ����Χ���ӵ���ײ��Ϣ������ײ��Ϣ�Ĵ���������һ�ŵ�ͼ���ڲ���
		// 3: ����ʹ������Ҽ�������޸ĸ��ӵ���ײ��Ϣ��
		// 4: ���Դ洢�޸ĵ�ͼ����ײ��Ϣ��
		g_sCursorInit.m_vec3CursorPos.x = t_sMousePos.m_iX * 1.5f + 0.75f;
		g_sCursorInit.m_vec3CursorPos.z = t_sMousePos.m_iY * 1.5f + 0.75f;

		// �����˸�����Χ��ײ��Ϣ��decal�б����б�Ϊȫ�ֱ���
		construct_decalList( t_pickIdx );
	}


# endif 

	m_ptrScene->draw_cursorInScene( g_sCursorInit ); 


# if __DEPLOY_MONSTERMOD__
	// ִ�й����ƶ�����Ϣ
	if( (m_ptrCreatureMgr->get_selCreatureEntity() != NULL)&&
		(!m_ptrCreatureMgr->get_monsterPicked() )  )
	{
		// Ctrl������ʱ,û���ƶ�����
		if( !lm_gvar::g_bCtrlDown )
		{
			lm_gameEntity*  t_ptrEnt = m_ptrCreatureMgr->get_selCreatureEntity();

			lm_command   t_cmd;
			t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
			t_cmd.m_eCommandId = LM_MONSTERWALK;
			t_cmd.m_ptrCommandContent = &t_vec3Pick;

			t_ptrEnt->process_command( t_cmd );
		}
	}
	else
	{
		// Ctrl������ʱ,û���ƶ����� 
		if( (m_ptrCreatureMgr->get_selCreatureEntity() == NULL)&&(!lm_gvar::g_bCtrlDown) )
		{
			lm_command   t_cmd;
			t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
			t_cmd.m_eCommandId = LM_HEROWALK;

			// TEST CODE:
			int  t_iSrcPtx,t_iSrcPtz;
			int  t_iDstPtx,t_iDstPtz;
			int  t_iResPtx,t_iResPtz;

			osVec3D   t_vec3Src,t_vec3RealTarget;

			m_ptrCreatureMgr->get_heroEntity()->get_entityPos( t_vec3Src );
			t_iSrcPtx = int(t_vec3Src.x * 1000);
			t_iSrcPtz = int(t_vec3Src.z * 1000);

			
			t_iDstPtx = int(t_vec3Pick.x * 1000);
			t_iDstPtz = int(t_vec3Pick.z * 1000);
		
			bool t_bRes;

			t_vec3RealTarget = t_vec3Pick;

# if 0
			//! ���Ŀ��㲻����ͨ��������Ҫ����һ�����׵�Ŀ��㡣
			if( !gc_gameScene::m_ptrPathFinder->CanMove( t_vec3Pick ) )
			{
				// ����Ҳ�������Ŀ��ƶ����㣬�����κδ���
				if( !gc_gameScene::m_ptrPathFinder->FindNearestMovePt( 
					t_iSrcPtx,t_iSrcPtz,t_iDstPtx,t_iDstPtz,t_iResPtx,t_iResPtz ) )
					return ;

				t_iDstPtx = t_iResPtx;
				t_iDstPtz = t_iResPtz;

				t_vec3RealTarget.x = t_iDstPtx/1000.0f;
				t_vec3RealTarget.z = t_iDstPtz/1000.0f;
			}
# else

# endif 

			

# if 1
			// 
			// �������������ʵĿ����Ϣ
			((gc_hero*)m_ptrCreatureMgr->get_heroEntity())->set_realTarget( t_vec3RealTarget );

			osDebugOut( "The pt before check<%d,%d,%d,%d>...\n",
				t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz );

			if( !gc_gameScene::m_ptrPathFinder->CheckLine( t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz,true ) )
			{
				/*
				t_iSrcPtx = 163580;
				t_iSrcPtz = 217444;
				t_iDstPtx = 156355;
				t_iDstPtz = 224858;
				*/
				// 83,123��(138,99)
				t_iSrcPtx = 124500;
				t_iSrcPtz = 184500;
				t_iDstPtx = 207000;
				t_iDstPtz = 148500;

				t_bRes = gc_gameScene::m_ptrPathFinder->FindPathMap(
					t_iSrcPtx,t_iSrcPtz,t_iDstPtx,t_iDstPtz,t_iResPtx,t_iResPtz );


				if( t_bRes )
				{
					t_vec3Pick.x = t_iResPtx/1000.0f;
					t_vec3Pick.z = t_iResPtz/1000.0f;
				}
				else // ����ԭ���Ĵ���,ʲô������.
				{
					osDebugOut( "Ѱ·ʧ��...\n" );
					//return;
				}

			}
			else
			{
				t_vec3Pick = t_vec3RealTarget;
				osDebugOut( "check line success....\n" );
			}

# else
			osVec3D  t_vecDst;
			osVec3Normalize( &t_vecDst,&(t_vec3Pick-t_vec3Src) );

			gc_gameScene::m_ptrPathFinder->FindLinePath( t_vec3Src,t_vecDst );
			t_vec3Pick.x = t_vec3Src.x;
			t_vec3Pick.z = t_vec3Src.z;
# endif 

			t_cmd.m_ptrCommandContent = &t_vec3Pick;
			lmc_commandMgr::Instance()->push_command( t_cmd );
		}
	}
# else

	// �ٷ������������ƶ�����Ϣ????
	lm_command   t_cmd;
	t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
	t_cmd.m_eCommandId = LM_HEROWALK;
	t_cmd.m_ptrCommandContent = &t_vec3Pick;
	lmc_commandMgr::Instance()->push_command( t_cmd );

# endif 

	unguard;
}

//! �Ҽ�����£��޸ĳ����ڵ���ײ��Ϣ
void gc_gameScene::on_rbuttonDownInScene( lm_command& _cmd )
{
	guard;

	// ����ǳ����ڵĹ���ˢ����صı༭,���Ҽ�ȥ�������ѡ��
# if __DEPLOY_MONSTERMOD__

	m_ptrCreatureMgr->set_nullSelCreature();

# endif 


# if __COLL_MOD__
	os_tileIdx      t_sMousePos;
	os_pickTileIdx  t_pickIdx;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Pick;


	// ���û�е�����򲻿�ʼ
	if( (m_sLastPickIdx.m_tgIdx.m_iX == 0xffff)&&
		(m_sLastPickIdx.m_tgIdx.m_iY == 0xffff)  )
		return;


	t_sMousePos = *(os_tileIdx*)_cmd.m_ptrCommandContent;
	osassert( (t_sMousePos.m_iX>=0)&&(t_sMousePos.m_iY>=0) );

	osassert( m_ptrScene );
	if( !m_ptrScene->get_pickTileIdx( t_pickIdx,t_sMousePos,&t_vec3Pick ) )
		return;


	// ����Ҽ������λ��������Ҫ�޸ĵ���ײ�����ڣ����޸ģ����򷵻ء�
	if( (t_pickIdx.m_tgIdx.m_iX != m_sLastPickIdx.m_tgIdx.m_iX)||
		(t_pickIdx.m_tgIdx.m_iY != m_sLastPickIdx.m_tgIdx.m_iY) )
		return;
	int t_iOffset = __COLL_DISGRID__/2;
	int t_iDis;
	t_iDis = t_pickIdx.m_colTileIdx.m_iX - m_sLastPickIdx.m_colTileIdx.m_iX;
	if( abs(t_iDis) > t_iOffset )
		return;
	t_iDis = t_pickIdx.m_colTileIdx.m_iY - m_sLastPickIdx.m_colTileIdx.m_iY;
	if( abs(t_iDis) > t_iOffset )
		return;


	//! �޸Ĵ�λ�õ���ײ��Ϣ
	if( !m_ptrScene->modify_colAtMousePick( t_pickIdx ) )
	{
		MessageBox( NULL,"�޸���ײ��Ϣ����...","ERROR",MB_OK );
		return;
	}

	construct_decalList( m_sLastPickIdx );

# endif 

	return;

	unguard;
}

//! �����Զ�����������ͼ�������б�
void gc_gameScene::on_ControlDownInScene( lm_command& _cmd )
{
	guard;


# if __COLL_MOD__

	//os_tileIdx      t_sMousePos;
	os_pickTileIdx  t_sMousePos;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Pick;


	// ���û�е�����򲻿�ʼ
	if( (m_sLastPickIdx.m_tgIdx.m_iX == 0xffff)&&
		(m_sLastPickIdx.m_tgIdx.m_iY == 0xffff)  )
		return;


	BOOL bPass  = *(BOOL*)_cmd.m_ptrCommandContent;

	int t_iOffset = __COLL_DISGRID__/2;
	float t_fTgOffsetX = m_sLastPickIdx.m_tgIdx.m_iX * TGMAP_WIDTH;
	float t_fTgOffsetZ = m_sLastPickIdx.m_tgIdx.m_iY * TGMAP_WIDTH;

	
	for( int t_x=m_sLastPickIdx.m_colTileIdx.m_iX-t_iOffset;
		 t_x<=m_sLastPickIdx.m_colTileIdx.m_iX+t_iOffset;t_x ++ )
	{	
		if( t_x < 0 )
			continue;
		if( t_x >= GROUNDMASK_WIDTH )
			continue;

		for( int t_z=m_sLastPickIdx.m_colTileIdx.m_iY-t_iOffset;
			 t_z<=m_sLastPickIdx.m_colTileIdx.m_iY+t_iOffset;t_z ++ )
		{
			if( t_z < 0 )
				continue;
			if( t_z >= GROUNDMASK_WIDTH )
				continue;

			// ����λ��������Decal.
		//	t_init.m_vec3Pos.x = t_x*HALF_TWIDTH + t_fTgOffsetX + HALF_CTWIDTH;
		//	t_init.m_vec3Pos.z = t_z*HALF_TWIDTH + t_fTgOffsetZ + HALF_CTWIDTH;

			// ���ݴ�λ���Ƿ���ײ�����õ���Ӧ�Ŀ�ͨ����Ϣ
			t_sMousePos.m_tgIdx = m_sLastPickIdx.m_tgIdx;
			t_sMousePos.m_colTileIdx.m_iX = t_x;
			t_sMousePos.m_colTileIdx.m_iY = t_z;
			//t_sMousePos.m_iX = m_sLastPickIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + t_x;
			//t_sMousePos.m_iY = m_sLastPickIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + t_z;

			if( !m_ptrScene->modify_colAtMousePick( t_sMousePos ,TRUE,(bool)bPass) )
			{
				MessageBox( NULL,"�޸���ײ��Ϣ����...","ERROR",MB_OK );
				return;
			}
		}
	}


	////! �޸Ĵ�λ�õ���ײ��Ϣ
	//if( !m_ptrScene->modify_colAtMousePick( t_pickIdx ) )
	//{
	//	MessageBox( NULL,"�޸���ײ��Ϣ����...","ERROR",MB_OK );
	//	return;
	//}

	construct_decalList( m_sLastPickIdx );

# endif 
	return;

	unguard;
}
//! �����Զ�����������ͼ�������б�
void gc_gameScene::process_travelWholeMap( const char* _mapName )
{
	guard;

	osVec2D   t_vec2MapSize;
	int       t_iXSize,t_iZSize;

	// �ٵõ���ͼ��x,z�����ϵĵ�ͼ����Ŀ��
	t_vec2MapSize = m_ptrScene->get_mapSize();
	t_iXSize = round_float( t_vec2MapSize.x/(TILE_WIDTH*64) );
	t_iZSize = round_float( t_vec2MapSize.y/(TILE_WIDTH*64) );

	// ѭ���ƽ�ÿһ����ͼ���spaceMove����
	lm_autoCmd   t_sAutoCmd;
	float t_fTimeOffset = 5.0f;
	for( int t_iZ = 0;t_iZ<t_iZSize;t_iZ++ )
	{
		for( int t_iX=0;t_iX<t_iXSize;t_iX ++ )
		{
			t_sAutoCmd.m_fOffsetExecTime = t_fTimeOffset;
			sprintf( t_sAutoCmd.m_szCmdStr,
				"move %s %d %d",
				_mapName,t_iX*128 + 64,t_iZ*128 + 64 );
			lmc_commandMgr::Instance()->push_autoCmd( t_sAutoCmd );

			t_fTimeOffset += 5.0f;
		}
	}

	t_sAutoCmd.m_fOffsetExecTime = t_fTimeOffset + 1.0f;
	sprintf( t_sAutoCmd.m_szCmdStr,"say %s","���������ŵ�ͼ��!!!!" );
	lmc_commandMgr::Instance()->push_autoCmd( t_sAutoCmd );

	unguard;
}

//! ���������ƶ�������
void gc_gameScene::process_spaceMove( lm_command& _cmd )
{
	guard;

	lm_spaceMoveCmd*    t_ptrMove = (lm_spaceMoveCmd*)_cmd.m_ptrCommandContent;
	
	// �ȴ����ͼ
	osassert( t_ptrMove->m_iXPos>=0 );
	osassert( t_ptrMove->m_iZPos>=0 );
	osassert( t_ptrMove->m_szMapName[0] );

	// ���û�е����ͼ���򲻴���ֱ�ӷ��ء�
	if( float_equal( m_ptrScene->get_mapSize().x,0.0f ) )
		return;

	BOOL   t_bFollowBack = lm_gvar::g_bCamFollowHero;
	lm_gvar::g_bCamFollowHero = TRUE;

	m_ptrScene->spacemove_process( 
		t_ptrMove->m_szMapName,t_ptrMove->m_iXPos,t_ptrMove->m_iZPos );

	// ������������. ��Ϊ��ͼ��û����ȷ�ĳ�ʼ����ɣ����Դ�ʱ������λ��Ϊˮƽ���ϣ���׼ȷ
	m_ptrCreatureMgr->reset_heroPos( t_ptrMove->m_iXPos,t_ptrMove->m_iZPos );

	// 
	// ����������׼ȷ������λ��
	os_sceneFM      t_fm;
	t_fm.m_ptrCam = m_ptrCamera->m_ptrCamera;
	m_ptrCreatureMgr->get_heroFocusPos( t_fm.m_vecChrPos );
	t_fm.m_iCx = WORD( t_ptrMove->m_iXPos );
	t_fm.m_iCy = WORD( t_ptrMove->m_iZPos );
	m_ptrScene->frame_move( &t_fm );

	// ������������. ��ʱλ����ȷ
	m_ptrCreatureMgr->reset_heroPos( t_ptrMove->m_iXPos,t_ptrMove->m_iZPos );

	// �����ƶ�������ͼ
	if( lm_gvar::g_bSpaceMoveWholeMap )
	{
		// ���볡�����Ƿ���Ҫ�õ����������ĵ�ͼ������Ȼ���ƽ����������һ�����ı�����������
		process_travelWholeMap( t_ptrMove->m_szMapName );
		lm_gvar::g_bSpaceMoveWholeMap = false;
	}

	lm_gvar::g_bCamFollowHero = t_bFollowBack;

	int t_i = gc_gameApp::m_3dMgr->get_availableTextureMem();

	t_i = 0;

	unguard;
}

//! ��������ħ�����������
void gc_gameScene::process_skillPlay( lm_command& _cmd )
{
	guard;

	lm_skillPlay    t_sSp;
	lm_command      t_cmd;
	lm_cmdSkillName* t_sSkillName = 
		((lm_cmdSkillName*)_cmd.m_ptrCommandContent);

	// ����Ҳ������֣���ֱ�ӷ���
	t_sSp.m_iSkillIndex = m_ptrMeMgr->get_skillIdFromName( t_sSkillName->m_szSkillName );
	if( t_sSp.m_iSkillIndex == -1 )
	{
		// ���ֲ���?
		t_sSp.m_iSkillIndex = atoi( t_sSkillName->m_szSkillName );

		if( t_sSp.m_iSkillIndex == 0 )
			return;
	}

	// �����start handle
	gc_creature*  t_ptrCrea;
	if( _cmd.m_iSHandle != -1 )
	{
		t_ptrCrea = m_ptrCreatureMgr->get_creaturePtrByHandle( _cmd.m_iSHandle );
		t_sSp.m_ptrStartPos = t_ptrCrea->get_entityPosPtr();
	}
	else
		t_sSp.m_ptrStartPos = m_ptrCreatureMgr->get_heroEntity()->get_entityPosPtr();

	if( _cmd.m_iEHandle == -1 )
	{
		if( NULL != m_ptrCreatureMgr->get_selCreatureEntity() )
			t_sSp.m_ptrAttackPos = m_ptrCreatureMgr->get_selCreatureEntity()->get_entityPosPtr();
		else
			t_sSp.m_ptrAttackPos = NULL;
	}
	else
	{
		t_ptrCrea = m_ptrCreatureMgr->get_creaturePtrByHandle( _cmd.m_iEHandle );
		t_sSp.m_ptrAttackPos = t_ptrCrea->get_entityPosPtr();
	}

	// River added @ 2007-6-7:����߶ȵ���
	t_sSp.m_fHeiAdj = t_sSkillName->m_fSkillHeight;


	t_cmd.m_dwCmdContentSize = sizeof( lm_skillPlay );
	t_cmd.m_eCommandId = LM_MESKILLPLAY;
	t_cmd.m_ptrCommandContent = &t_sSp;
	t_cmd.m_iEHandle = _cmd.m_iEHandle;
	t_cmd.m_iSHandle = _cmd.m_iSHandle;

	lmc_commandMgr::Instance()->push_command( t_cmd );

	unguard;
}

//! �ڳ����������������ڵ�λ���ϲ���һ����Ч
void gc_gameScene::process_effplay( lm_command& _cmd )
{
	guard;

	lm_effectPlay    t_sEp;
	lm_effectPlay*   t_ptrEp;
	lm_command       t_cmd;

	t_ptrEp = (lm_effectPlay*)_cmd.m_ptrCommandContent;
	strcpy( t_sEp.m_szEffName,t_ptrEp->m_szEffName );

	// �������ָ��λ�ò��ţ�����ָ����λ�ò��ţ������������������ڵ�λ�ò���
	if( (t_ptrEp->m_vec3Pos.x > 0.0f)&&
		(t_ptrEp->m_vec3Pos.z > 0.0f) )
	{
		m_ptrScene->get_tilePos( 
			(int)t_ptrEp->m_vec3Pos.x,(int)t_ptrEp->m_vec3Pos.z,t_sEp.m_vec3Pos );
	}
	else
		m_ptrCreatureMgr->get_heroEntity()->get_entityPos( t_sEp.m_vec3Pos );

	t_cmd.m_dwCmdContentSize = sizeof( lm_effectPlay );
	t_cmd.m_eCommandId = LM_MEEFFECTPLAY;
	t_cmd.m_ptrCommandContent = &t_sEp;

	lmc_commandMgr::Instance()->push_command( t_cmd );


	unguard;
}

//! ���������ֵĲ��š�
void gc_gameScene::process_musicPlay( lm_command& _cmd )
{
	guard;

	lm_cmdFileName*    t_ptrCmdFileName = (lm_cmdFileName*)_cmd.m_ptrCommandContent;

	if( t_ptrCmdFileName->m_szFileName[0] != NULL )
	{
		if( m_ptrSoundMgr )
			m_ptrSoundMgr->play_mp3( t_ptrCmdFileName->m_szFileName,TRUE );		
	}

	unguard;
}

//! ���������װ���л�
void gc_gameScene::process_equipChange( lm_command& _cmd )
{
	guard;

	lm_cmdFileName*    t_ptrEquipSwitch = (lm_cmdFileName*)_cmd.m_ptrCommandContent;

	if( _cmd.m_iSHandle == -1 )
		return;

	const char*   t_szCDir;
	os_equipment  t_equip;
	lm_command    t_cmd;

	if( t_ptrEquipSwitch->m_szFileName[0] != NULL )
	{
		t_szCDir = m_ptrCreatureMgr->get_creatureStr( _cmd.m_iSHandle );
		if( !t_szCDir ) return;
		if( !t_szCDir[0] ) return;

		if( !m_ptrItemMgr->get_equipFromName( t_szCDir,
			t_ptrEquipSwitch->m_szFileName,t_equip ) )
			return;

		t_cmd.m_dwCmdContentSize = sizeof( os_equipment );
		t_cmd.m_eCommandId = LM_MONSTEREQUIP;
		t_cmd.m_ptrCommandContent = &t_equip;
		t_cmd.m_iSHandle = _cmd.m_iSHandle;

		lmc_commandMgr::Instance()->push_command( t_cmd );

	}

	return;

	unguard;
}

//! �������ڹ����״̬ħ������
void gc_gameScene::process_stateMagic( lm_command& _cmd )
{
	guard;

	lm_cmdFileName*    t_ptrStateName = (lm_cmdFileName*)_cmd.m_ptrCommandContent;

	const char*   t_ptrEffectName = NULL;

	if( m_ptrStateMgr )
		t_ptrEffectName = m_ptrStateMgr->getStateSpeffName( t_ptrStateName->m_szFileName );

	lm_command       t_cmd;
	lm_cmdFileName   t_ptrName;
	// 
	if( NULL == t_ptrEffectName )
		t_ptrName.m_szFileName[0] = NULL;
	else
		strcpy( t_ptrName.m_szFileName,t_ptrEffectName );
	t_cmd.m_iSHandle = _cmd.m_iSHandle;
	t_cmd.m_dwCmdContentSize = _cmd.m_dwCmdContentSize;
	t_cmd.m_eCommandId = LM_MESTATEEFFECT;
	t_cmd.m_ptrCommandContent = &t_ptrName;

	lmc_commandMgr::Instance()->push_command( t_cmd );

	return;

	unguard;
}



//! �������ڵ������仯
void gc_gameScene::process_weatherChange( lm_command& _cmd )
{
	guard;

	// ���������ļ�
	lm_cmdFileName*    t_szName = (lm_cmdFileName*)_cmd.m_ptrCommandContent;

	DWORD         t_dwVersion;
	os_weather    t_sWeather;
	char          t_szMagic[4];

	// read_fileToBuf
	int t_iGBufIdx;
	BYTE*    t_fstart = START_USEGBUF( t_iGBufIdx );

	// ����������ļ�������������
	if( !file_exist( t_szName->m_szFileName ) )
		return ;

	int t_iSize = read_fileToBuf( t_szName->m_szFileName,t_fstart,TMP_BUFSIZE );
	osassertex( t_iSize > 0,va("���ļ�<%s>ʧ��..\n",t_szName->m_szFileName ) );

	READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	if( strcmp( t_szMagic,"wea" ) != 0 )
		osassertex( false,va( "����������ļ�<%s>..\n",t_szName->m_szFileName ) );

	READ_MEM_OFF( &t_sWeather,t_fstart,sizeof( os_weather ) - sizeof( os_rainStruct ) );

	END_USEGBUF( t_iGBufIdx );

	if (t_dwVersion < 110)
	{
		memset(t_sWeather.m_szOsaBkSkyEffect[0],0,128);
		memset(t_sWeather.m_szOsaBkSkyEffect[1],0,128);
		memset(t_sWeather.m_szOsaBolt,0,128);
		t_sWeather.m_iBoltRate = 0.0f;	//!< ���粥�Ÿ���(0~1)
		t_sWeather.m_BoltAngle = 0.0f;	//!< ����Ƕȣ�����ֵ��0~2*PI��
		t_sWeather.m_BoltColor = osColor( 0.0f,0.0f,0.0f,0.0f );	//!< ������ɫ
		t_sWeather.m_BoltType = 0;		//!< ��������,0��ʾֻӰ�췽���,1��ʾֻӰ�컷����
		t_sWeather.m_BoltHeight = 0.0f;	//!< ����߶�
		//! �ڶ���͵����������ת�ٶ�
		t_sWeather.m_fSky1MoveSpeed = 0.0f;
		t_sWeather.m_fSky2MoveSpeed = 0.0f;

		t_sWeather.m_TerrFogDistance=0.0f;
		t_sWeather.m_SkyFogDistance=0.0f;
		t_sWeather.m_BoltTime = 0.0f;			//!< ����ʱ��
		//{@ windy add 7.10 ���ڶ�����������⴦��
		t_sWeather.m_BoltFrequency=0;	//!< �Ȳ���,������������ټ�
		//@}
		//{@ windy add 7.11 �����ѩ���Ŀ��� 
		t_sWeather.m_bSnow=false;
		//os_snow		m_SnowSetting;
	}
	if (t_dwVersion<=112)
	{
		strcpy(t_sWeather.m_szTerrain,"keyani\\default_terr.osa");
	}
	
	// ���ݵ�ǰ�����Զ��������������Ŀ�ʼ�ͽ�������
	t_sWeather.m_fFogStart = t_sWeather.m_fFogStart * 
		gc_camera::m_ptrCamera->get_farClipPlaneDis();
	t_sWeather.m_fFogEnd = t_sWeather.m_fFogEnd *
		gc_camera::m_ptrCamera->get_farClipPlaneDis();

	// �������泡���ڲ���������
	m_ptrScene->change_sceneWeather( t_sWeather,3.0f );
	

	unguard;
}


//! ִ��ָ��Ľӿ�
BOOL gc_gameScene::process_command( lm_command& _command )
{
	guard;

	// �����ڳ������
	if( (_command.m_eCommandId>LM_SCENESTART)&&
		(_command.m_eCommandId<LM_SCENEEND) )
	{
		switch(_command.m_eCommandId)
		{
		case LM_SCENELOAD:
			if( !load_scene( (const char*)_command.m_ptrCommandContent ) )
				osassert( false );
			break;
		case LM_SCENERELEASE:
			release_scene();
			break;
		case LM_SCENE_LBUTTONDOWN:
			on_lbuttonDownInScene( _command );
			break;
		case LM_SCENE_MOUSEMOVE:
			on_mousemoveInScene( _command );
			break;
		case LM_SCENE_RBUTTONDOWN:
			on_rbuttonDownInScene( _command );
			break;
		case LM_SCENE_CONTROLDOWN:
			on_ControlDownInScene( _command);
			break;
		case LM_SPACEMOVE:
			process_spaceMove( _command );
			break;
		case LM_SKILLPLAY:
			process_skillPlay( _command );
			break;
		case LM_EFFECTPLAY:
			process_effplay( _command );
			break;
		case LM_SCENEMUSIC:
			process_musicPlay( _command );
			break;
		case LM_NAMEEQUIPCHANGE:
			process_equipChange( _command );
			break;
		case LM_MAGICSTATE:
			process_stateMagic( _command );
			break;
		default:
			break;
		}
	}

	// ����û�е���֮ǰ�������������
	if( !is_renderEnable() )
		return true;


	// ���������
	if( (_command.m_eCommandId>LM_CAMERASTART)&&
		(_command.m_eCommandId<LM_CAMERAEND) )
	{
		osassert( m_ptrCamera );
		m_ptrCamera->process_command( _command );
	}

	// ����������
	if( (_command.m_eCommandId>LM_HEROSTART)&&
		(_command.m_eCommandId<LM_HEROEND) )
	{
		osassert( this->m_ptrCreatureMgr );
		m_ptrCreatureMgr->process_command( _command );
	}
	// �����ڵĹ���
	if( (_command.m_eCommandId>LM_MONSTERSTART)&&
		(_command.m_eCommandId<LM_MONSTEREND) )
	{
		osassert( m_ptrCreatureMgr );
		m_ptrCreatureMgr->process_command( _command );
	}

	// �����ڵ���Ч������
	if( (_command.m_eCommandId>LM_MEMGRSTART)&&
		(_command.m_eCommandId<LM_MEMGREND) )
	{
		osassert( m_ptrMeMgr );
		m_ptrMeMgr->process_command( _command );
	}

	if( _command.m_eCommandId == LM_WEATHERCHANGE )
		process_weatherChange( _command );
	


	return TRUE;

	unguard;
}


