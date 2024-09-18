///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_gameScene.h
 *
 *  Desc:     游戏场景的基类，游戏中用到的每一个场景，都需要从这个基类中派生，并处理不同
 *            的场景相关的消息。
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
//!  窗口的常规信息。
HWND		gc_gameScene::m_hWnd = NULL;
HINSTANCE	gc_gameScene::m_hIns = NULL;
int			gc_gameScene::m_iScrWidth = 800;
int			gc_gameScene::m_iScrHeight = 600;
//@} 

static os_sceneCursorInit     g_sCursorInit;


//! 引擎底层场景的指针,全局只有一个实例.
I_fdScene*	 gc_gameScene::m_ptrScene = NULL;

//! TEST CODE:
I_skinMeshObj* gc_gameScene::m_ptrHeroSkinObj = NULL;
I_skinMeshObj* gc_gameScene::m_ptrHeroSkinObj2 = NULL;

//! 处理杂项功能的接口。
I_resource*	 gc_gameScene::m_ptrResource = NULL;

//! 当前的socket管理器.
I_socketMgr* gc_gameScene::m_ptrSocket = NULL;

//! 全局的相机类
gc_camera*   gc_gameScene::m_ptrCamera = NULL;

//! 全局的人物管理类
gc_creatureMgr* gc_gameScene::m_ptrCreatureMgr = NULL;

//! 全局的声音播放管理类
lm_soundMgr*    gc_gameScene::m_ptrSoundMgr = NULL;
//! 全局的技能管理类
lm_meManager*   gc_gameScene::m_ptrMeMgr = NULL;

//! 全局的装备管理器指针
gc_itemMgr* gc_gameScene::m_ptrItemMgr = NULL;
//! 全局的人物状态指针
CStateIconMgr* gc_gameScene::m_ptrStateMgr = NULL;


//! 地图寻路算法相关
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


//! 即时执行命令的返回值
int gc_gameScene::m_iCmdImmResCode = -1;

//! 设置和得到即时命令的返回结果
void gc_gameScene::set_immCmdCode( int _code )
{
	m_iCmdImmResCode = _code;
}

int gc_gameScene::get_immCmdCode( void )
{
	return m_iCmdImmResCode;
}


/** \brief
*  全局的初始化当前引擎场景指针和界面管理器指针的。
*
*  要在主程序初始化的地方调用这个程序。
* \param gcs_sceneOnceInit& _init  初始化结构
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
	// 得到渲染引擎的场景指针。
	m_ptrScene = ::get_sceneMgr();
	osassert( m_ptrScene );
	
	// 实例化全局的相机
	m_ptrCamera = new gc_camera;

	// 
	m_ptrCreatureMgr = new gc_creatureMgr;

	// 声音管理器
	m_ptrSoundMgr = new lm_soundMgr;
	m_ptrSoundMgr->init_soundMgr( _init.m_hWnd );

	// 技能管理器
	m_ptrMeMgr = new lm_meManager;
	m_ptrMeMgr->init_skillList( NULL );

	// 装备管理器
	m_ptrItemMgr = gc_itemMgr::Instance();
	m_ptrItemMgr->init_itemMgr();

	// 状态管理器
	/*
	m_ptrStateMgr = new CStateIconMgr();
	m_ptrStateMgr->init();
	*/

	// 寻咱相关的数据
    m_ptrPathFinder = new CFindPathMap;


	// 读入在场景内画的图标
	g_ptrConfigInfoReader->read_cursorDrawInScene( g_sCursorInit );


	return true;

	unguard;
}


/** \brief
*  释放场景资源的接口.
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
*  调入场景资源的接口.
*
*  \param _lsData  从上一个场景传入到当前场景的数据，不同的场景解释为不同的
*                  数据。
*/
bool gc_gameScene::load_scene( const char* _lsData/* = NULL*/ )
{
	return true;
}


/** \brief
*  渲染接口.
* 
*/
bool gc_gameScene::render_scene( void )
{
	return true;
}


# if __COLL_MOD__
//! 构建碰撞信息显示的decal列表
void gc_gameScene::construct_decalList( os_pickTileIdx& _pickIdx )
{
	guard;


	int t_iOffset = __COLL_DISGRID__/2;
	os_tileIdx      t_sMousePos;
	float t_fTgOffsetX = _pickIdx.m_tgIdx.m_iX * TGMAP_WIDTH;
	float t_fTgOffsetZ = _pickIdx.m_tgIdx.m_iY * TGMAP_WIDTH;
	os_decalInit       t_init;

	// 有简单的动画
	t_init.m_fVanishTime = 10000.0f;
	t_init.m_fRotAgl = 0.0f;
	t_init.m_fSize = 0.7f;
	strcpy( t_init.m_szDecalTex,"s_animCursor" );
	t_init.m_wXFrameNum = 3 ;
	t_init.m_wYFrameNum = 3;
	t_init.m_fFrameTime = 0.15f;


	// 先删除当前decal列表内所有的decal.
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

			// 根据位置来构建Decal.
			t_init.m_vec3Pos.x = t_x*HALF_TWIDTH + t_fTgOffsetX + HALF_CTWIDTH;
			t_init.m_vec3Pos.z = t_z*HALF_TWIDTH + t_fTgOffsetZ + HALF_CTWIDTH;

			// 根据此位置是否碰撞，来得到相应的可通过信息
			t_sMousePos.m_iX = _pickIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + t_x;
			t_sMousePos.m_iY = _pickIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + t_z;
			if( m_ptrScene->collision_detection( t_sMousePos ) )
				t_init.m_dwDecalColor = 0xffffff00;
			else
				t_init.m_dwDecalColor = 0xffff0000;


			// 得到索引:
			int t_iIdxX = t_x - _pickIdx.m_colTileIdx.m_iX + t_iOffset;
			int t_iIdxZ = t_z - _pickIdx.m_colTileIdx.m_iY + t_iOffset;

			m_vecColDecalId[t_iIdxZ*__COLL_DISGRID__+t_iIdxX] = 
				m_ptrScene->create_decal( t_init );

			osassert( m_vecColDecalId[t_iIdxZ*__COLL_DISGRID__+t_iIdxX] >= 0 );
		}
	}

# if __COLL_MOD__
	// 保存当前的点击位置，用于修改后更新碰撞信息的显示
	memcpy( &m_sLastPickIdx,&_pickIdx,sizeof( os_pickTileIdx ) );
# endif 

	unguard;
}
# endif 

//! 删除场景内可见的碰撞Decal列表
void gc_gameScene::reset_colDecal( void )
{
	guard;

# if __COLL_MOD__
	//! 删除所有的decalId.
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




//! 场景内鼠标移动消息的处理
void gc_gameScene::on_mousemoveInScene( lm_command& _cmd )
{
	guard;

	// 发须是部署怪物的编辑器才有这个功能.
# if __DEPLOY_MONSTERMOD__

	//! 如果显示碰撞信息的开关关掉,则什么都不需要处理.
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
	// 画地表的显示光标
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
	// 构建此格子周围碰撞信息的decal列表，此列表为全局变量
	construct_decalList( t_pickIdx );
# endif

# endif 

	unguard;
}

//! 左键点击下，显示场景内的Decal
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
	// 先用怪物管理器处理
	m_ptrCreatureMgr->process_command( _cmd );

# endif 


	osassert( m_ptrScene );
	if( !m_ptrScene->get_pickTileIdx( t_pickIdx,t_sMousePos,&t_vec3Pick ) )
		return;

	// 
	// 画地表的显示光标
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
		// River @ 2007-1-5: 构建碰撞修改相关的功能。
		// 碰撞修改功能算法：
		// 1: 点击位置显示在碰撞格子的中心。
		// 2: 显示点击位置周围格子的碰撞信息。此碰撞信息的处理限制在一张地图的内部。
		// 3: 可以使用鼠标右键点击来修改格子的碰撞信息。
		// 4: 可以存储修改地图的碰撞信息。
		g_sCursorInit.m_vec3CursorPos.x = t_sMousePos.m_iX * 1.5f + 0.75f;
		g_sCursorInit.m_vec3CursorPos.z = t_sMousePos.m_iY * 1.5f + 0.75f;

		// 构建此格子周围碰撞信息的decal列表，此列表为全局变量
		construct_decalList( t_pickIdx );
	}


# endif 

	m_ptrScene->draw_cursorInScene( g_sCursorInit ); 


# if __DEPLOY_MONSTERMOD__
	// 执行怪物移动的消息
	if( (m_ptrCreatureMgr->get_selCreatureEntity() != NULL)&&
		(!m_ptrCreatureMgr->get_monsterPicked() )  )
	{
		// Ctrl键按下时,没有移动功能
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
		// Ctrl键按下时,没有移动功能 
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
			//! 如果目标点不可能通过，则需要设置一个靠谱的目标点。
			if( !gc_gameScene::m_ptrPathFinder->CanMove( t_vec3Pick ) )
			{
				// 如果找不到最近的可移动顶点，不做任何处理
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
			// 设置鼠标点击的真实目标信息
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
				// 83,123到(138,99)
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
				else // 继续原来的处理,什么都不做.
				{
					osDebugOut( "寻路失败...\n" );
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

	// 再发送主角人物移动的消息????
	lm_command   t_cmd;
	t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
	t_cmd.m_eCommandId = LM_HEROWALK;
	t_cmd.m_ptrCommandContent = &t_vec3Pick;
	lmc_commandMgr::Instance()->push_command( t_cmd );

# endif 

	unguard;
}

//! 右键点击下，修改场景内的碰撞信息
void gc_gameScene::on_rbuttonDownInScene( lm_command& _cmd )
{
	guard;

	// 如果是场景内的怪物刷新相关的编辑,则右键去除怪物的选定
# if __DEPLOY_MONSTERMOD__

	m_ptrCreatureMgr->set_nullSelCreature();

# endif 


# if __COLL_MOD__
	os_tileIdx      t_sMousePos;
	os_pickTileIdx  t_pickIdx;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Pick;


	// 如果没有点击，则不开始
	if( (m_sLastPickIdx.m_tgIdx.m_iX == 0xffff)&&
		(m_sLastPickIdx.m_tgIdx.m_iY == 0xffff)  )
		return;


	t_sMousePos = *(os_tileIdx*)_cmd.m_ptrCommandContent;
	osassert( (t_sMousePos.m_iX>=0)&&(t_sMousePos.m_iY>=0) );

	osassert( m_ptrScene );
	if( !m_ptrScene->get_pickTileIdx( t_pickIdx,t_sMousePos,&t_vec3Pick ) )
		return;


	// 如果右键点击的位置在我们要修改的碰撞区域内，则修改，否则返回。
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


	//! 修改此位置的碰撞信息
	if( !m_ptrScene->modify_colAtMousePick( t_pickIdx ) )
	{
		MessageBox( NULL,"修改碰撞信息出错...","ERROR",MB_OK );
		return;
	}

	construct_decalList( m_sLastPickIdx );

# endif 

	return;

	unguard;
}

//! 处理自动遍历整个地图的命令列表
void gc_gameScene::on_ControlDownInScene( lm_command& _cmd )
{
	guard;


# if __COLL_MOD__

	//os_tileIdx      t_sMousePos;
	os_pickTileIdx  t_sMousePos;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Pick;


	// 如果没有点击，则不开始
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

			// 根据位置来构建Decal.
		//	t_init.m_vec3Pos.x = t_x*HALF_TWIDTH + t_fTgOffsetX + HALF_CTWIDTH;
		//	t_init.m_vec3Pos.z = t_z*HALF_TWIDTH + t_fTgOffsetZ + HALF_CTWIDTH;

			// 根据此位置是否碰撞，来得到相应的可通过信息
			t_sMousePos.m_tgIdx = m_sLastPickIdx.m_tgIdx;
			t_sMousePos.m_colTileIdx.m_iX = t_x;
			t_sMousePos.m_colTileIdx.m_iY = t_z;
			//t_sMousePos.m_iX = m_sLastPickIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + t_x;
			//t_sMousePos.m_iY = m_sLastPickIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + t_z;

			if( !m_ptrScene->modify_colAtMousePick( t_sMousePos ,TRUE,(bool)bPass) )
			{
				MessageBox( NULL,"修改碰撞信息出错...","ERROR",MB_OK );
				return;
			}
		}
	}


	////! 修改此位置的碰撞信息
	//if( !m_ptrScene->modify_colAtMousePick( t_pickIdx ) )
	//{
	//	MessageBox( NULL,"修改碰撞信息出错...","ERROR",MB_OK );
	//	return;
	//}

	construct_decalList( m_sLastPickIdx );

# endif 
	return;

	unguard;
}
//! 处理自动遍历整个地图的命令列表
void gc_gameScene::process_travelWholeMap( const char* _mapName )
{
	guard;

	osVec2D   t_vec2MapSize;
	int       t_iXSize,t_iZSize;

	// 再得到地图在x,z方向上的地图块数目。
	t_vec2MapSize = m_ptrScene->get_mapSize();
	t_iXSize = round_float( t_vec2MapSize.x/(TILE_WIDTH*64) );
	t_iZSize = round_float( t_vec2MapSize.y/(TILE_WIDTH*64) );

	// 循环推进每一个地图块的spaceMove命令
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
	sprintf( t_sAutoCmd.m_szCmdStr,"say %s","我跑完整张地图了!!!!" );
	lmc_commandMgr::Instance()->push_autoCmd( t_sAutoCmd );

	unguard;
}

//! 处理自由移动的命令
void gc_gameScene::process_spaceMove( lm_command& _cmd )
{
	guard;

	lm_spaceMoveCmd*    t_ptrMove = (lm_spaceMoveCmd*)_cmd.m_ptrCommandContent;
	
	// 先处理地图
	osassert( t_ptrMove->m_iXPos>=0 );
	osassert( t_ptrMove->m_iZPos>=0 );
	osassert( t_ptrMove->m_szMapName[0] );

	// 如果没有调入地图，则不处理，直接返回。
	if( float_equal( m_ptrScene->get_mapSize().x,0.0f ) )
		return;

	BOOL   t_bFollowBack = lm_gvar::g_bCamFollowHero;
	lm_gvar::g_bCamFollowHero = TRUE;

	m_ptrScene->spacemove_process( 
		t_ptrMove->m_szMapName,t_ptrMove->m_iXPos,t_ptrMove->m_iZPos );

	// 更新人物和相机. 因为地图还没有正确的初始化完成，所以此时的人物位置为水平面上，不准确
	m_ptrCreatureMgr->reset_heroPos( t_ptrMove->m_iXPos,t_ptrMove->m_iZPos );

	// 
	// 接下来设置准确的人物位置
	os_sceneFM      t_fm;
	t_fm.m_ptrCam = m_ptrCamera->m_ptrCamera;
	m_ptrCreatureMgr->get_heroFocusPos( t_fm.m_vecChrPos );
	t_fm.m_iCx = WORD( t_ptrMove->m_iXPos );
	t_fm.m_iCy = WORD( t_ptrMove->m_iZPos );
	m_ptrScene->frame_move( &t_fm );

	// 更新人物和相机. 此时位置正确
	m_ptrCreatureMgr->reset_heroPos( t_ptrMove->m_iXPos,t_ptrMove->m_iZPos );

	// 遍历移动整个地图
	if( lm_gvar::g_bSpaceMoveWholeMap )
	{
		// 调入场景后，是否需要得到整个场景的地图个数，然后推进命令，让人物一步步的遍历整个场景
		process_travelWholeMap( t_ptrMove->m_szMapName );
		lm_gvar::g_bSpaceMoveWholeMap = false;
	}

	lm_gvar::g_bCamFollowHero = t_bFollowBack;

	int t_i = gc_gameApp::m_3dMgr->get_availableTextureMem();

	t_i = 0;

	unguard;
}

//! 处理场景内魔法插入的命令
void gc_gameScene::process_skillPlay( lm_command& _cmd )
{
	guard;

	lm_skillPlay    t_sSp;
	lm_command      t_cmd;
	lm_cmdSkillName* t_sSkillName = 
		((lm_cmdSkillName*)_cmd.m_ptrCommandContent);

	// 如果找不到名字，则直接返回
	t_sSp.m_iSkillIndex = m_ptrMeMgr->get_skillIdFromName( t_sSkillName->m_szSkillName );
	if( t_sSp.m_iSkillIndex == -1 )
	{
		// 数字播放?
		t_sSp.m_iSkillIndex = atoi( t_sSkillName->m_szSkillName );

		if( t_sSp.m_iSkillIndex == 0 )
			return;
	}

	// 如果有start handle
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

	// River added @ 2007-6-7:加入高度调整
	t_sSp.m_fHeiAdj = t_sSkillName->m_fSkillHeight;


	t_cmd.m_dwCmdContentSize = sizeof( lm_skillPlay );
	t_cmd.m_eCommandId = LM_MESKILLPLAY;
	t_cmd.m_ptrCommandContent = &t_sSp;
	t_cmd.m_iEHandle = _cmd.m_iEHandle;
	t_cmd.m_iSHandle = _cmd.m_iSHandle;

	lmc_commandMgr::Instance()->push_command( t_cmd );

	unguard;
}

//! 在场景内主角人物所在的位置上播放一个特效
void gc_gameScene::process_effplay( lm_command& _cmd )
{
	guard;

	lm_effectPlay    t_sEp;
	lm_effectPlay*   t_ptrEp;
	lm_command       t_cmd;

	t_ptrEp = (lm_effectPlay*)_cmd.m_ptrCommandContent;
	strcpy( t_sEp.m_szEffName,t_ptrEp->m_szEffName );

	// 如果是在指定位置播放，则在指定的位置播放，否则在主角人物所在的位置播放
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

//! 处理背景音乐的播放　
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

//! 处理人物的装备切换
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

//! 处理场景内怪物的状态魔法播放
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



//! 处理场景内的天气变化
void gc_gameScene::process_weatherChange( lm_command& _cmd )
{
	guard;

	// 调入天气文件
	lm_cmdFileName*    t_szName = (lm_cmdFileName*)_cmd.m_ptrCommandContent;

	DWORD         t_dwVersion;
	os_weather    t_sWeather;
	char          t_szMagic[4];

	// read_fileToBuf
	int t_iGBufIdx;
	BYTE*    t_fstart = START_USEGBUF( t_iGBufIdx );

	// 如果不存在文件，不处理天气
	if( !file_exist( t_szName->m_szFileName ) )
		return ;

	int t_iSize = read_fileToBuf( t_szName->m_szFileName,t_fstart,TMP_BUFSIZE );
	osassertex( t_iSize > 0,va("打开文件<%s>失败..\n",t_szName->m_szFileName ) );

	READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	if( strcmp( t_szMagic,"wea" ) != 0 )
		osassertex( false,va( "错误的天气文件<%s>..\n",t_szName->m_szFileName ) );

	READ_MEM_OFF( &t_sWeather,t_fstart,sizeof( os_weather ) - sizeof( os_rainStruct ) );

	END_USEGBUF( t_iGBufIdx );

	if (t_dwVersion < 110)
	{
		memset(t_sWeather.m_szOsaBkSkyEffect[0],0,128);
		memset(t_sWeather.m_szOsaBkSkyEffect[1],0,128);
		memset(t_sWeather.m_szOsaBolt,0,128);
		t_sWeather.m_iBoltRate = 0.0f;	//!< 闪电播放概率(0~1)
		t_sWeather.m_BoltAngle = 0.0f;	//!< 闪电角度，弧度值（0~2*PI）
		t_sWeather.m_BoltColor = osColor( 0.0f,0.0f,0.0f,0.0f );	//!< 闪电颜色
		t_sWeather.m_BoltType = 0;		//!< 闪电类型,0表示只影响方向光,1表示只影响环境光
		t_sWeather.m_BoltHeight = 0.0f;	//!< 闪电高度
		//! 第二层和第三层天空旋转速度
		t_sWeather.m_fSky1MoveSpeed = 0.0f;
		t_sWeather.m_fSky2MoveSpeed = 0.0f;

		t_sWeather.m_TerrFogDistance=0.0f;
		t_sWeather.m_SkyFogDistance=0.0f;
		t_sWeather.m_BoltTime = 0.0f;			//!< 闪电时间
		//{@ windy add 7.10 用于对闪电进行特殊处理
		t_sWeather.m_BoltFrequency=0;	//!< 先不用,如果参数不够再加
		//@}
		//{@ windy add 7.11 加入对雪花的控制 
		t_sWeather.m_bSnow=false;
		//os_snow		m_SnowSetting;
	}
	if (t_dwVersion<=112)
	{
		strcpy(t_sWeather.m_szTerrain,"keyani\\default_terr.osa");
	}
	
	// 根据当前相机的远剪切面来处理雾的开始和结束矩离
	t_sWeather.m_fFogStart = t_sWeather.m_fFogStart * 
		gc_camera::m_ptrCamera->get_farClipPlaneDis();
	t_sWeather.m_fFogEnd = t_sWeather.m_fFogEnd *
		gc_camera::m_ptrCamera->get_farClipPlaneDis();

	// 更改引擎场景内部的天气数
	m_ptrScene->change_sceneWeather( t_sWeather,3.0f );
	

	unguard;
}


//! 执行指令的接口
BOOL gc_gameScene::process_command( lm_command& _command )
{
	guard;

	// 场景内场景相关
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

	// 场景没有调入之前，所有命令不处理
	if( !is_renderEnable() )
		return true;


	// 场景内相机
	if( (_command.m_eCommandId>LM_CAMERASTART)&&
		(_command.m_eCommandId<LM_CAMERAEND) )
	{
		osassert( m_ptrCamera );
		m_ptrCamera->process_command( _command );
	}

	// 场景内人物
	if( (_command.m_eCommandId>LM_HEROSTART)&&
		(_command.m_eCommandId<LM_HEROEND) )
	{
		osassert( this->m_ptrCreatureMgr );
		m_ptrCreatureMgr->process_command( _command );
	}
	// 场景内的怪物
	if( (_command.m_eCommandId>LM_MONSTERSTART)&&
		(_command.m_eCommandId<LM_MONSTEREND) )
	{
		osassert( m_ptrCreatureMgr );
		m_ptrCreatureMgr->process_command( _command );
	}

	// 场景内的特效管理器
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


