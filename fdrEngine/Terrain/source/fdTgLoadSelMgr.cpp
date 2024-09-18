//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTgLoadSelMgr.cpp
 *
 *  His:      River created @ 2005-5-18
 *
 *  Des:      对fdMgr内的TileGround进行管理,得到哪个需要置换出内存,哪一个应该调入到内存.
 *            在场景内所有的TG都调入到内存的情况下,此文件中函数管理了哪个tg应该是激活的
 *            需要处理的tg ,哪些是在内存中等待处理的tg .
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"


# include "../../backPipe/include/osCamera.h"

# include  "../../mfpipe/include/osMesh.h"

# include "../include/fdNewFieldStruct.h"


//! 全局的场景调入进度
int    g_iLoadProgress = 0;

osc_tgLoadReleaseMgr::osc_tgLoadReleaseMgr()
{
	
	m_ptrMidPipe = NULL; 
	m_ptrRealTg = NULL;
	m_iCurTgNum = 0;

	memset( m_ptrActTg,NULL,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );

	m_bTLoadInMem = false;

}


// 
osc_tgLoadReleaseMgr::~osc_tgLoadReleaseMgr()
{

	SAFE_DELETE_ARRAY( m_ptrRealTg );
	m_sThreadMgr.end_thread();
	m_sThreadMgr.End();

}


//  得到地图管理器初始化的位置信息.
void osc_tgLoadReleaseMgr::init_insertInfo( os_sceneLoadStruct* _sl )
{
	guard;

	osassert( _sl );
	osassert( _sl->m_iX>=0 );
	osassert( _sl->m_iY >= 0 );

	// 
	// 所在的图块.
	m_sLFCamInIdx.m_iX = _sl->m_iX / GROUNDMASK_WIDTH;
	m_sLFCamInIdx.m_iY = _sl->m_iY / GROUNDMASK_WIDTH;

	// 图块内的位置.
	osc_newTileGround::frame_moveCamInTgPos( _sl->m_iX,_sl->m_iY ); 
	m_eLFTgPos = osc_newTileGround::get_camInTgPos();

	unguard;
}



//! 整理出上层可用的地图列表,即激活的地图列表.
void osc_tgLoadReleaseMgr::get_activeTgList( void )
{
	guard;

	memset( m_ptrActTg,0,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );

	int    t_idx = 0;

	// 如果是把全部的地图都调入到内存
	if( m_bTLoadInMem )
		osassert( false );

	// 
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		if( m_sThreadMgr.is_activeTg( t_i ) )
			m_ptrActTg[t_idx++] = &m_ptrRealTg[t_i];
	}

	unguard;
}


//@{


//! 得到当前tg周围一圈的tg,共8个，这些tg肯定需要调入到内存。
void osc_tgLoadReleaseMgr::get_surroundTG( os_tileIdx* _mapIdx,int _my,int _mx )
{
	guard;


	int    t_iNameIdx = 1;

	osassert( _mapIdx );

	for( int t_y=_my-1;t_y<=_my+1;t_y ++ )
	{
		for( int t_x=_mx-1;t_x<=_mx+1;t_x ++ )
		{
			if( (t_y<0)||(t_y>=m_iMaxZTgNum) )
			{
				// River @ 2009-5-6: 没有必须置-1,直接进入下一轮.
				//_mapIdx[t_iNameIdx].m_iX = -1,_mapIdx[t_iNameIdx].m_iY = -1;			
				//t_iNameIdx ++;
				continue;
			}
			if( (t_x<0)||(t_x>=m_iMaxXTgNum) )
			{
				// River @ 2009-5-6: 没有必须置-1,直接进入下一轮.
				//_mapIdx[t_iNameIdx].m_iX = -1,_mapIdx[t_iNameIdx].m_iY = -1;		
				//t_iNameIdx ++;
				continue;
			}

			//! 已经在头一个位置放置了此地图的文件名
			if( (t_x == _mx)&&(t_y == _my) )
				continue;

			_mapIdx[t_iNameIdx].m_iX = t_x;
			_mapIdx[t_iNameIdx].m_iY = t_y;

			t_iNameIdx ++;

			// River added @ 2009-6-5:用于低端硬件的处理
			if( t_iNameIdx >= MAXTG_INTGMGRMACRO )
				break;
		}
	}

	return;

	unguard;
}


//  
//! 根据当前在tg中的位置,来调入当前tg第二圈的数据.
void osc_tgLoadReleaseMgr::get_secSurTG( os_tileIdx* _mapIdx,int _my,int _mx )
{
	guard;


	osassert( _mapIdx );
	osassert( osc_newTileGround::m_seCamTgPos != OSN_TGPOS_ERROR );

	os_tileIdx        t_idx[MAXTG_INTGMGRMACRO];
	int              t_iNum;


	t_iNum = round_float( sqrt( float(MAXTG_INTGMGR) ) );

	switch( osc_newTileGround::m_seCamTgPos )
	{
	case OSN_TGPOS_TOPLEFT:
		t_idx[0].m_iX = _mx- (t_iNum-2);t_idx[0].m_iY = _my + (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X相同。
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY - t_i;

			//! Y相同
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX + t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	case OSN_TGPOS_TOPRIGHT:
		t_idx[0].m_iX = _mx + (t_iNum-2);t_idx[0].m_iY = _my + (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X相同。
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY - t_i;

			//! Y相同
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX - t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	case OSN_TGPOS_BOTLEFT:
		t_idx[0].m_iX = _mx - (t_iNum-2);t_idx[0].m_iY = _my - (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X相同。
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY + t_i;

			//! Y相同
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX + t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	case OSN_TGPOS_BOTRIGHT:
		t_idx[0].m_iX = _mx + (t_iNum-2);t_idx[0].m_iY = _my - (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X相同。
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY + t_i;

			//! Y相同
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX - t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	}

	// 调入第二圈的TileGround
	int        t_iBaseIdx = (t_iNum-1)*(t_iNum-1);
	os_tileIdx* t_ptrIdx;
	for( int t_i=t_iBaseIdx;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrIdx = &t_idx[t_i-t_iBaseIdx];

		if( (t_ptrIdx->m_iX <0)||(t_ptrIdx->m_iX>=m_iMaxXTgNum) )
		{
			_mapIdx[t_i].m_iX = -1,_mapIdx[t_i].m_iY = -1;
			continue;
		}

		if( (t_ptrIdx->m_iY<0)||(t_ptrIdx->m_iY>=m_iMaxZTgNum) )
		{
			_mapIdx[t_i].m_iX = -1,_mapIdx[t_i].m_iY = -1;
			continue;
		}

		_mapIdx[t_i].m_iX = t_ptrIdx->m_iX;
		_mapIdx[t_i].m_iY = t_ptrIdx->m_iY;
	}


	return;

	unguard;
}


//@}

/** \brief
*  从客户端填充的场景调入结构中得我们要调入的场景文件名。
*
*  \param  _nameArr 传入的4个长度的s_string字串，我们最多静态
*                   调入4张地图，在这个数据中返回我们要调入的地图名，
*                   如果地图名为空，则不需要调入这个地图。
*  
* 
*/
bool osc_tgLoadReleaseMgr::get_loadSceneName( 
			s_string* _nameArr,os_sceneLoadStruct* _sl )
{
	guard;

	int      t_iX,t_iY;
	int      t_iMX,t_iMY;
	char     t_str[32];
	s_string  t_mapName;

	osassert( _nameArr );
	osassert( _sl );
	osassert( _sl->m_szMapName[0] );


	t_iX = _sl->m_iX/2;
	t_iY = _sl->m_iY/2;

	//
	// 如果位置大于地图的宽度和高度。
	if( (t_iX>m_iMaxX)||(t_iY>m_iMaxZ) )
	{
		osDebugOut( 
			"位置大于地图的宽度和高度:t_iX:<%d>,m_iMaxX:<%d>, t_iY:<%d>,m_iMaxZ:<%d>\n",
			t_iX,m_iMaxX,t_iY,m_iMaxZ  );
		osassert( false );
		return false;
	}

	// 
	// 得到当前相机所在的地图。
	t_iMX = t_iX/MAX_GROUNDWIDTH;
	t_iMY = t_iY/MAX_GROUNDWIDTH;

	_nameArr[0] = SCENEMAP_DIR;
	_nameArr[0] += _sl->m_szMapName;
	_nameArr[0] += "\\";
	_nameArr[0] += _sl->m_szMapName;
	t_mapName = _nameArr[0];

	// catch this name.
	m_strMapName = t_mapName;


	osc_newTileGround::frame_moveCamInTgPos( _sl->m_iX,_sl->m_iY );

	// River added @ 2009-5-6:先置为-1;
	for( int t_i=0;t_i<MAXTG_INTGMGRMACRO;t_i ++ )
	{
		m_vecNeedLoadTile[t_i].m_iX = WORD(-1);
		m_vecNeedLoadTile[t_i].m_iY = WORD(-1);

	}

	// 当前所在的位置,必须调入.
	m_vecNeedLoadTile[0].m_iX = t_iMX;
	m_vecNeedLoadTile[0].m_iY = t_iMY;
	get_surroundTG( m_vecNeedLoadTile,t_iMY,t_iMX );

	//! 根据当前在tg中的位置，来调入当前tg第二圈的数据
	// 在内存充足的情况下,根据当前在tg中的位置，来调入当前tg第二圈的数据
	if( MAXTG_INTGMGR > MINTG_INTGMGRMACRO )
		get_secSurTG( m_vecNeedLoadTile,t_iMY,t_iMX ); 

	//
	// 从我们计算的tg列表中,计算应该调入的地图名字列表.
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		// 不合法的地图，在地图的边界以外
		if( (m_vecNeedLoadTile[t_i].m_iX==WORD(-1)) ||
			(m_vecNeedLoadTile[t_i].m_iY==WORD(-1)) )
		{
			_nameArr[t_i] = "";
			continue;
		}

		_nameArr[t_i] = t_mapName;	

		sprintf( t_str,"%02d%02d.geo",
			m_vecNeedLoadTile[t_i].m_iX,m_vecNeedLoadTile[t_i].m_iY );
		_nameArr[t_i] += t_str;

		osDebugOut( "The <%d> map's name is:<%s>..\n",
			t_i,_nameArr[t_i].c_str() );
	}

	return true;

	unguard;
}



//! 
void osc_tgLoadReleaseMgr::init_tgLRMgr( osc_TGManager* _tgMgr )
{
	guard;

	osassert( _tgMgr );

	m_ptrMidPipe = &_tgMgr->m_middlePipe;

	// 初始化线程管理并运行。
	m_sThreadMgr.init_mtLoadMapMgr(  _tgMgr,this );
	m_sThreadMgr.Begin();

	//@{
	// Windy mod @ 2006-1-11 12:11:41
	//Desc: MapTile只初始化一次的操作
	MapTile::OneTimeInit( m_ptrMidPipe );
	//@}



	//! 根据是否全部把地图调入内存来分配不同的存储空间。　
	if( m_bTLoadInMem )
	{
		osassertex( false,"全部调入地图" );
	}
	else
	{
		m_ptrRealTg = new osc_newTileGround[MAXTG_INTGMGR];

		m_iCurTgNum = MAXTG_INTGMGR;
	}

	unguard;
}


/** \brief
*  根据相机的位置，计算出当前帧需要调入的图块列表.
*
*  每一个TG分成了四个区域，如果相机的位置跟上一帧在同一个tg内，并且
*  跟上一帧在同样的tg区域内，则不需要地图置换。
*  
*  \param return bool 返回值表示了是否需要对当前的显示地图块进行置换.
*                     如果返回真,则需要对地图置换.
*/
bool osc_tgLoadReleaseMgr::get_tgNeedInMemList( osVec3D& _pos )
{
	guard;


	osVec3D         t_vec3CamPos;
	osn_inTgPos     t_sInTgPos;
	os_tileIdx      t_sCamInTg;
	bool            t_bRes;

	t_vec3CamPos = _pos;


	m_iCamInX = int( t_vec3CamPos.x / HALF_TWIDTH );
	m_iCamInZ = int( t_vec3CamPos.z / HALF_TWIDTH );

	osc_newTileGround::frame_moveCamInTgPos( m_iCamInX,m_iCamInZ );
	t_sInTgPos = osc_newTileGround::get_camInTgPos();

	t_sCamInTg.m_iX = WORD( t_vec3CamPos.x / (MAX_GROUNDWIDTH*TILE_WIDTH) );
	t_sCamInTg.m_iY = WORD( t_vec3CamPos.z / (MAX_GROUNDWIDTH*TILE_WIDTH) );

	// 
	if( (t_sInTgPos == this->m_eLFTgPos)&&
		(t_sCamInTg.m_iX == this->m_sLFCamInIdx.m_iX)&&
		(t_sCamInTg.m_iY == this->m_sLFCamInIdx.m_iY ) )
	{
		t_bRes = false;
	}
	else
	{
		t_bRes = true;

		// 
		// 得到当前应该在内存中存在的地图的列表.
		m_vecNeedLoadTile[0].m_iX = t_sCamInTg.m_iX;
		m_vecNeedLoadTile[0].m_iY = t_sCamInTg.m_iY;
		get_surroundTG( m_vecNeedLoadTile,t_sCamInTg.m_iY,t_sCamInTg.m_iX );
		// 在内存充足的情况下,根据当前在tg中的位置，来调入当前tg第二圈的数据
		if( MAXTG_INTGMGR > MINTG_INTGMGRMACRO )
			get_secSurTG( m_vecNeedLoadTile,t_sCamInTg.m_iY,t_sCamInTg.m_iX );

		m_eLFTgPos = t_sInTgPos;
		m_sLFCamInIdx.m_iX = t_sCamInTg.m_iX;
		m_sLFCamInIdx.m_iY = t_sCamInTg.m_iY;

	}

	return t_bRes;

	unguard;
}

//! 如果另外的线程正在调入地图,终止当前的地图调入状态.
void osc_tgLoadReleaseMgr::stop_threadMapLoading( void )
{
	guard;

	m_sThreadMgr.reset_loadMapMgr();
	return;

	unguard;
}


/** \brief
*  根据传入的地图名字列表，来计算出需要释放的地图名字
*
*  此函数用于多线程状态下的人物自由移动.
*/
void osc_tgLoadReleaseMgr::spacemove_swapOutMap( s_string* _mapList )
{
	guard;

	osc_TileGroundPtr      t_ptrTg;
	int                   t_i,t_j;


	osassert( _mapList );
	// River 2005-10-22:
	// 可以出现没有使用的地图，但目前不能处于地图调入的状态
	osassert( m_sThreadMgr.is_freeStatus() );

	m_iNeedRelTgNum = 0;

	// 没有使用的地图做为最优先的调入等级放在队列最前端
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];

		if( !t_ptrTg->is_tgInuse() )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}
	}

	// 
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];
		if( !t_ptrTg->is_tgInuse() )
			continue;

		for( t_j = 0;t_j<MAXTG_INTGMGR;t_j ++ )
		{
			if( t_ptrTg->m_strMapFName == _mapList[t_j] )
			{
				_mapList[t_j] = "";
				break;
			}
		}
		// 在要调入的名字列表中，不存在当前的地图，当前的地图可以释放
		if( t_j == MAXTG_INTGMGR )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}	
	}

	return;

	unguard;
}


//! 对要调入的地图找到空间，然后推进到另外的线程中，进行处理。
void osc_tgLoadReleaseMgr::process_needSwapOutTgList( void )
{
	guard;

	osc_TileGroundPtr      t_ptrTg;
	WORD                  t_x,t_y;
	int                   t_i,t_j;



	// River 2005-10-22:
	// 可以出现没有使用的地图，但目前不能处于地图调入的状态
	osassert( m_sThreadMgr.is_freeStatus() );

	// 没有使用的地图做为最优先的调入等级放在队列最前端
	m_iNeedRelTgNum = 0;
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];

		if( !t_ptrTg->is_tgInuse() )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}
	}


	// 
	// 然后找不需要在新的地图块列表内出现的地图块
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];

		if( !t_ptrTg->is_tgInuse() )
			continue;

		t_x = (WORD)t_ptrTg->m_sTgHeader.m_wMapPosX;
		t_y = (WORD)t_ptrTg->m_sTgHeader.m_wMapPosZ;

		// 
		// 如果此tg在 <<<内存中应该存在的>>> 图块列表中不存在,则标注此图块为要释放的图块.
		for( t_j = 0;t_j<MAXTG_INTGMGR;t_j ++ )
		{
			if( (t_x == m_vecNeedLoadTile[t_j].m_iX)&&
				(t_y == m_vecNeedLoadTile[t_j].m_iY) )
				break;
		}

		if( t_j == MAXTG_INTGMGR )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}	
	}

	return;

	unguard;
}

//! 计算需要在内存中存在，目前在内存中又不存在的图块列表。 
void osc_tgLoadReleaseMgr::process_needSwapInTgList( void )
{
	guard;

	osc_TileGroundPtr      t_ptrTg;
	WORD                  t_x,t_y;
	int                   t_i,t_j;

	m_iNeedSwapInNum = 0;

	// 
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_x = m_vecNeedLoadTile[t_i].m_iX;
		t_y = m_vecNeedLoadTile[t_i].m_iY;

		// 
		// 如果此tg在<内存中应该存在的>图块列表中不存在,则标注此图块为要调入到内存中的地图块.
		for( t_j = 0;t_j<MAXTG_INTGMGR;t_j ++ )
		{
			t_ptrTg = &m_ptrRealTg[t_j];

			// 
			// River @ 2005-10-31: 如果此地图没有启用，则进入下一轮的处理
			if( !t_ptrTg->m_bInuse )
				continue;

			if( (t_ptrTg->m_sTgHeader.m_wMapPosX == t_x)&&
				(t_ptrTg->m_sTgHeader.m_wMapPosZ == t_y) )
				break;
		}

		// 在目前的内存tg列表中没有这个<<<内存中应该存在的>>>　图块。
		if( t_j == MAXTG_INTGMGR )
		{
			// River mod @ 2007-9-1:确认不调入不存在的地图。
			if( (t_x>=m_iMaxXTgNum)||(t_y>=m_iMaxZTgNum) )
				continue;

			m_vecTgNeedSwapIn[m_iNeedSwapInNum].m_iX = t_x;
			m_vecTgNeedSwapIn[m_iNeedSwapInNum].m_iY = t_y;
			m_iNeedSwapInNum ++;
		}	
	}

	return;

	unguard;
}


//! 把应该存在于内存,又不在内存的tg推进给另外的线程,调入.
bool osc_tgLoadReleaseMgr::push_needSwapInMap( void )
{
	guard;

	int          t_i;
	int          t_x,t_y;
	s_string      t_strMapname;
	char          t_sz[16];

	for( t_i=0;t_i<m_iNeedSwapInNum;t_i ++ )
	{
		if( (m_vecTgNeedSwapIn[t_i].m_iX == (WORD)-1)||
			(m_vecTgNeedSwapIn[t_i].m_iY == (WORD)-1) )
			continue;


		osDebugOut( "地图置换发生...\n" );

		t_x = m_vecTgNeedSwapIn[t_i].m_iX;
		t_y = m_vecTgNeedSwapIn[t_i].m_iY;

		//
		// 要存在于内存中的地图块,目前不存在于内存中,推进到另外的线程中.
		t_strMapname = m_strMapName;

		// 调入不同的地图
		sprintf( t_sz,"%02d%02d.geo",t_x,t_y );

		t_strMapname += t_sz;

		osassert( m_vecTgNeedRelease[t_i]<MAXTG_INTGMGR );

		m_sThreadMgr.push_loadMap( 
			(char*)t_strMapname.c_str(),m_vecTgNeedRelease[t_i] );

	}


	return true;

	unguard;
}


/** \brief
*  对计算出的调入图块列表，根据调入优化级别进行排序。	
*
*  排序分为以下级别：
*  1: tg的bbox与视域相交的，tg的位置位于当前相机所在tg相连接的位置的两种情形，
*     需要使用阻塞式的地图调入方式。
*  2: 其它的地图使用正常的地图调入方式。
*  
*  如果当前正处于地图调入状态，则每一帧可以根据传入的相机指针，来重新设定
*  地图调入线程管理器内部的地图调入队列，慢慢优化的这个有意思算法.....
* 
*/
bool osc_tgLoadReleaseMgr::sort_needSwapInMapList( osc_camera* _cam )
{
	guard;

	osassert( _cam );

	os_tileIdx           t_vecTgNeedSwapIn[MAXTG_INTGMGRMACRO];

	m_iBlockLoad = 0;

	// 
	// 直接的操作我们暂存的数据
	memcpy( t_vecTgNeedSwapIn,m_vecTgNeedSwapIn,
		   sizeof( os_tileIdx )*m_iNeedSwapInNum );

	for( int t_i=0;t_i<m_iNeedSwapInNum;t_i ++ )
	{

	}

	return true;

	unguard;
}



/** \brief
*  传入相机，根据相机来得到应该可见的地图列表, 激活的地图烈表.
*
*  每一帧需要调用这个函数，在后台来处理地图的调入.
*  传回上层当前可用的地图指针列表和可用的地图数目.
*
*  1: 根据相机的位置，算出是否需要置换地图.
*  2: 处理置换的地图，推进到地图调入线程。
*  3: 算出上层可以使用的地图列表。 返回地图个数。
*/
int osc_tgLoadReleaseMgr::frame_getInViewTg( 
	osc_camera* _cam,osc_TileGroundPtr* _tgList,int _maxNum )
{
	guard;

	osassert( _cam );
	osassert( _tgList );
	osassert( _maxNum );

	int   t_idx = 0;

# if __QTANG_VERSION__

	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		osassert( t_idx < _maxNum );

		if( !m_ptrRealTg[t_i].is_tgInuse() )
			continue;
		
		_tgList[t_idx] = &m_ptrRealTg[t_i];
		t_idx ++;
	}

# else

	osVec3D   t_vec3CamPos;

	_cam->get_curpos( &t_vec3CamPos );

	//
	// ATTENTION TO OPP: 此段的代码有问题。
	// 根据位置,查看是否需要地图置换.
	if( get_tgNeedInMemList( t_vec3CamPos ) )
	{		
		// 重设线程的调入.
		stop_threadMapLoading();
		
		// 计算需要从内存中置换出的地图图块列表
		process_needSwapOutTgList();

		// 计算需要在内存中存在，目前在内存中又不存在的图块列表。
		process_needSwapInTgList();
		osassertex( m_iNeedSwapInNum <= m_iNeedRelTgNum,
			va( "要调入的地图块数目必须小于等要移出的地图块数目!!调入:<%d>,移出:<%d>..\n",
			    m_iNeedSwapInNum,m_iNeedRelTgNum ) );

		// 对计算出的调入图块列表，根据调入优化级别进行排序。	
		sort_needSwapInMapList( _cam );

		// 推进要调到的图块列表到图块调入线程中。
		push_needSwapInMap();

	}

	// 得到激活的地图块列表，即可以在主线程内渲染处理的地图块列表
	get_activeTgList();

	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		osassert( t_idx < _maxNum );

		if( !m_ptrActTg[t_i] )
			continue;

		_tgList[t_idx] = m_ptrActTg[t_i];
		t_idx ++;
	}

# endif 

	return t_idx;

	unguard;
}

//! 初始化地图后，得到当前激活的地图列表，此时还没有相机加入，所以加入单独的函数。
int osc_tgLoadReleaseMgr::init_getActMapList( osc_TileGroundPtr* _tgList,int _maxNum )
{
	guard;

	osassert( _tgList );
	osassert( _maxNum );

	osassertex( m_sThreadMgr.is_freeStatus(),"当前地图调入线程应该处于空闲状态...\n" );

	get_activeTgList();

	int   t_idx = 0;
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		osassert( t_idx < _maxNum );

		if( !m_ptrActTg[t_i] )
			continue;

		_tgList[t_idx] = m_ptrActTg[t_i];
		t_idx ++;

	}

	return t_idx;


	return 0;

	unguard;
}



//! 整体释放当前的地图内存管理器。
void osc_tgLoadReleaseMgr::release_tgLRMgr( void )
{
	guard;

	// 先终止多线程相关的调入.
	m_sThreadMgr.reset_loadMapMgr();

	//! 把所有可操作的地图全部释放。
	for( int t_i=0;t_i<this->m_iCurTgNum;t_i ++ )
	{
		if( m_ptrRealTg[t_i].is_tgInuse() )
		{
			if( !m_ptrRealTg[t_i].release_TG() )
				osassert( false );
		}
	}

	
	// 全部的释放静态的mesh和osa文件资源
	g_ptrMeshMgr->reset_staticMesh();
	g_ptrMeshMgr->reset_osaMesh();
	

	return;

	unguard;
}




/** \brief
*  多线程安全的地图调入代码.
*
*  替换TGManager中的相关函数.
*/
bool osc_tgLoadReleaseMgr::threadSafe_loadNMap( 
	           char* _nmapFname,osc_TileGroundPtr _tg )
{
	guard;

	osassert( _nmapFname && (_nmapFname[0]) );
	osassert( _tg );
	osassert( !_tg->is_tgInuse() );

	// TEST CODE:
	g_bMapLoadding = true;

	if( !_tg->load_TGData( _nmapFname,this->m_ptrMidPipe ) )
	{
		osassertex( false,"地图管理器,  多线程调入地图失败...\n" );
		return false;
	}

	osDebugOut( "地图管理器,  多线程调入地图<%s>成功...\n",_nmapFname );
	
	// TEST CODE:
	g_bMapLoadding = false;


	return true;

	unguard;
}



/** \brief
*  初次调入地图的函数。
*
*  相当于接替了原来的osc_TGManager中的load_scene函数.
*
*  \param _sl 调入地图的结构。
*  \param _maxX,_maxZ 地图在x,z方向上的最大地图块数。
*  
*/
bool osc_tgLoadReleaseMgr::load_scene( os_sceneLoadStruct* _sl,int _maxX,int _maxZ )
{
	guard;

	osassert( _sl );

	osassert( _maxX>0 );
	osassert( _maxZ>0 );

	VEC_string     t_vecMapName;

	m_iMaxXTgNum = _maxX;
	m_iMaxZTgNum = _maxZ;
	m_iMaxX = m_iMaxXTgNum*MAX_GROUNDWIDTH;
	m_iMaxZ = m_iMaxZTgNum*MAX_GROUNDWIDTH;

	// 全部调入地图到内存中
	if( m_bTLoadInMem )
	{
		osassertex( false,"全部调入地图" );
	}
	else
	{
		// 先得到要调入地图的名字。
		t_vecMapName.resize( MAXTG_INTGMGR );

		if( !get_loadSceneName( &t_vecMapName[0],_sl ) )
			osassertex( false,"取得初始化地图名字失败" );

		// syq 5-20 add
		if( osc_TGManager::m_pUpdate_uiLoadingStep )
			(*osc_TGManager::m_pUpdate_uiLoadingStep)( 25,0);
		g_iLoadProgress = 25;

		// 循环调入我们得到的所有地图名字,此处为阻塞调入,必须调入完成后才能返回.
		for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
		{
			if( t_vecMapName[t_i] == "" )
				continue;

			// 推进地图到另外的线程,并等待调入完成.
			m_sThreadMgr.push_loadMap((char*)t_vecMapName[t_i].c_str(),t_i,TRUE );

			// syq 5-20 add
			g_iLoadProgress += int((float(t_i+1)/float(MAXTG_INTGMGR))*30);
			if( osc_TGManager::m_pUpdate_uiLoadingStep )
				(*osc_TGManager::m_pUpdate_uiLoadingStep)( g_iLoadProgress,0);

		}

	}

    // 
	// 主动终止当前的线程处理
	// River @ 2006-2-23:在地图快要调入完的时候，此函数有可能死锁
	stop_threadMapLoading();

	// 初始化初始的位置信息.
	init_insertInfo( _sl );

	return true;

	unguard;
}



//! 自由移动的处理函数跟此函数。
bool osc_tgLoadReleaseMgr::space_moveMapLoad( os_sceneLoadStruct* _sl,int _maxX,int _maxZ )
{
	guard;

	int         t_idx = 0;
	s_string     t_szMapNameList[16];

	osassert( _sl );

	m_iMaxX = _maxX;
	m_iMaxZ = _maxZ;
	m_iMaxXTgNum = m_iMaxX/MAX_GROUNDWIDTH;
	m_iMaxZTgNum = m_iMaxZ/MAX_GROUNDWIDTH;

	// 先终止线程的地图调入
	stop_threadMapLoading();

	if( !get_loadSceneName( t_szMapNameList,_sl ) )
		osassert( false );

	spacemove_swapOutMap( t_szMapNameList );
	osDebugOut( "Swap out tg num is:<%d>..\n",m_iNeedRelTgNum );


	// 循环调入我们得到的所有地图名字,此处为阻塞调入,必须调入完成后才能返回.
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		if( t_szMapNameList[t_i] == "" )
			continue;

		// 推进地图到另外的线程,并等待调入完成.
		m_sThreadMgr.push_loadMap( (char*)t_szMapNameList[t_i].c_str(),
			          m_vecTgNeedRelease[t_idx],TRUE  );

		t_idx ++;
	}

    // 
	// 主动终止当前的线程处理
	stop_threadMapLoading();
	init_insertInfo( _sl );

	return true;

	unguard;
}

