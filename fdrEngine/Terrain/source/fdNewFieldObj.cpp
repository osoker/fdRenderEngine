//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldObj.cpp
 *
 *  His:      River created @ 2006-1-14
 *
 *  Des:      地图内物品处理的杂项函数
 *   
 * “吾恐季氏之忧，不在颛臾，而在萧墙之内也。” 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdNewFieldStruct.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../effect/include/fdEleBolt.h"
# include "../../effect/include/osHDRLight.h"
# include "../../interface/miskFunc.h"
# include "../GeoMipTerrain/osMapTerrain.h"



//! 检测射线是否和物品索引相交.
BOOL osc_newTileGround::is_intersectObj( int _objIdx,
					osVec3D& _rayS,float _dis,osVec3D& _rayDir,float* _resDis/* = NULL*/ )
{
	guard;

	os_newTGObject*   t_ptrObj;
	
	osassert( _objIdx <= m_iTGObjectNum );

	t_ptrObj = &m_vecTGObject[_objIdx];

	// 
	// 必须是第零类物品才能处理。
	if( t_ptrObj->m_dwType == 0 )
	{
		float         t_fXoffset,t_fZoffset;
		osVec3D       t_vec3CamS,t_vec3CamDir;
		osMatrix      t_mat,t_matScale;

		t_fXoffset = m_sTgHeader.m_wMapPosX*MAX_GROUNDWIDTH*TILE_WIDTH;
		t_fZoffset = m_sTgHeader.m_wMapPosZ*MAX_GROUNDHEIGHT*TILE_WIDTH;

		bool t_b;
		bool t_bScale = !vec3_equal( t_ptrObj->m_vec3ObjectScale,osVec3D( 1.0f,1.0f,1.0f ) );

		// 
		//! River @ 2009-5-4: 缩放和无缩放使用两条代码分支，简单优化
		if( t_bScale )
		{
			osMatrixScaling( &t_matScale,t_ptrObj->m_vec3ObjectScale.x,
				t_ptrObj->m_vec3ObjectScale.y,t_ptrObj->m_vec3ObjectScale.z );

			osMatrixRotationY( &t_mat,t_ptrObj->m_fAngle );
			t_mat = t_matScale * t_mat;

			// 设置物品的位置
			t_mat._41 = t_ptrObj->m_vec3ObjPos.x;
			t_mat._42 = t_ptrObj->m_vec3ObjPos.y;
			t_mat._43 = t_ptrObj->m_vec3ObjPos.z;

			float  t_fTmp;
			osMatrixInverse( &t_matScale,&t_fTmp,&t_mat );

			// 旋转相机位置和方向到物品空间
			t_vec3CamS = _rayS;
			osVec3Transform( &t_vec3CamS,&t_vec3CamS,&t_matScale );
			osVec3TransformNormal( &t_vec3CamDir,&_rayDir,&t_matScale );
			osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );

			// 
			// River mod @ 2009-4-24:处理缩放后物品的碰撞检测信息
			float   t_f = osVec3Length( &t_vec3CamS ); 

			t_b = g_ptrMeshMgr->intersect_mesh( 
				m_vecObjectDevIdx[_objIdx],t_vec3CamS,_dis,t_vec3CamDir,_resDis );

			//! 计算碰撞点
			if( t_b )
			{
				osVec3D    t_vec3Tmp;
				t_vec3Tmp = t_vec3CamS + t_vec3CamDir*(*_resDis);
				osVec3Transform( &t_vec3CamS,&t_vec3Tmp,&t_mat );

				*_resDis = osVec3Length( &(t_vec3CamS - _rayS ) );
			}
		}
		else
		{
			// 把相机处理到当前的Mesh空间，然后调用Mesh的处理函数
			osMatrixRotationY( &t_mat,-t_ptrObj->m_fAngle );

			t_vec3CamS.y  = _rayS.y - t_ptrObj->m_vec3ObjPos.y;
			t_vec3CamS.x  = _rayS.x - t_ptrObj->m_vec3ObjPos.x;
			t_vec3CamS.z  = _rayS.z - t_ptrObj->m_vec3ObjPos.z;

			// 旋转相机位置和方向到物品空间
			osVec3Transform( &t_vec3CamS,&t_vec3CamS,&t_mat );
			osVec3TransformNormal( &t_vec3CamDir,&_rayDir,&t_mat );
			osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );

			t_b = g_ptrMeshMgr->intersect_mesh( 
				m_vecObjectDevIdx[_objIdx],t_vec3CamS,_dis,t_vec3CamDir,_resDis );
		}


		return t_b;	
	}

	return false;


	unguard;
}

// 
# define CAMCOLL_OFFSET  0.1f

//! River added @ 2009-5-3:用于处理相机与人物之间的物品半透明的偏移值。
OSENGINE_API float g_fCamFocusOffset = 0.0f;

/** \brief
*  对得到的物品的列表进行处理。
*
*  算法描述：
*  把每一个物品分配到每一个tileGround.
*/
bool osc_TGManager::process_camChrObjList( void )
{
	guard;

	osVec3D            t_vec3CamPos,t_vec3CamDir,t_vec3HeroPos;
	osc_TileGroundPtr  t_ptrTG;
	float              t_fDis;
	static int t_vecObjId[MAXOBJNUM_TOCHAR];
	int        t_iFindNum = 0,t_iObjNum = 0;

	osassert( g_ptrCamera );

	//! 主角人物的大致高度
	t_vec3HeroPos = g_ptrCamera->get_camFocus();
	t_vec3HeroPos.y += g_fCamFocusOffset;
	
	// 人物所在格子的物品列表
	m_iCamChrObjNum = 0;
	for( int t_i=0;t_i<this->m_iActMapNum;t_i ++ )
	{	
		t_iFindNum += m_arrPtrTG[t_i]->m_sQuadTree.get_chunkObj(
			 t_vec3HeroPos,&t_vecObjId[0],t_iObjNum,MAXOBJNUM_TOCHAR );

		if( m_iCamChrObjNum < t_iObjNum )
		{
			for( int t_idx = m_iCamChrObjNum;t_idx<t_iObjNum;t_idx ++ )
			{
				m_vecCamChrObj[t_idx].m_iX = WORD(t_i);
				m_vecCamChrObj[t_idx].m_iY = t_vecObjId[t_idx];
			}
			m_iCamChrObjNum = t_iObjNum;
		}
	}

	//
	// 得到相机的射线。
	if( m_iCamChrObjNum > 0 )
	{
		g_ptrCamera->get_curpos( &t_vec3CamPos );
		t_vec3CamDir = t_vec3CamPos - t_vec3HeroPos;
		t_fDis = osVec3Length( &t_vec3CamDir );
		osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );

		//! River: 加大值，确认相机位置往后拉一些,减小相机每一帧的反弹
		t_fDis += CAMCOLL_OFFSET;

	}

	osassert( MAXOBJNUM_TOCHAR >= m_iCamChrObjNum );

	float      t_fClosestLength = 10000.0f;
	float      t_fTmp ;
	for( int t_i=0;t_i<m_iCamChrObjNum;t_i ++ )
	{
		t_ptrTG = m_arrPtrTG[m_vecCamChrObj[t_i].m_iX];

		// CAMERA WORK:
		// River mod @ 2009-2-5:如果有相交，则记录交点，找到离人物最近的交点
		// 处理相机到这个位置,下一步就是如何的正确回复相机到原来的的位置，跟地形
		// 一个碰撞的处理，但跟物品的回复，就会稍有一些麻烦
		if( t_ptrTG->is_intersectObj( 
			m_vecCamChrObj[t_i].m_iY,t_vec3HeroPos,t_fDis,t_vec3CamDir,&t_fTmp ) ) 
		{
			//! 如果是桥类物品，则什么都不做.
			if( t_ptrTG->is_bridgeObj( m_vecCamChrObj[t_i].m_iY ) )
				continue;

			if( t_ptrTG->is_NoAlphaObj( m_vecCamChrObj[t_i].m_iY ) )
				continue;


			// River added @ 2009-3-3:如果是树类物品，则半透明处理
			// River mod @ 2009-4-27:反向的处理一下树,默认物品为树类物品.
			if( !t_ptrTG->is_treeObj( m_vecCamChrObj[t_i].m_iY ) )
			{
				t_ptrTG->m_ivoMgr.make_halfFade( m_vecCamChrObj[t_i].m_iY );
				continue;
			}

			if( t_fTmp < t_fClosestLength )
				t_fClosestLength = t_fTmp;

		}
	}

	if( m_iCamChrObjNum > 0 )
	{
		if( t_fClosestLength <= (t_fDis-CAMCOLL_OFFSET ) )
			g_ptrCamera->move_cameraToAvoidShelter( t_fClosestLength );
		// 此处的调用，为的是不让相机出现反弹
		else if( t_fClosestLength <= t_fDis )
			g_ptrCamera->move_cameraToAvoidShelter( -1.0f );
	}


	return true;

	unguard;
}

//! River added @ 2007-6-8:加入地表高度的信息处理。
//! 此处的高度，为纯地表高度，即渲染地表的数据高度。
bool osc_TGManager::get_earthHeight( osVec3D& _pos )
{
	guard;

	os_tileIdx                    t_tgIdx;
	float                         t_fU,t_fV,t_fOrigX,t_fOrigZ;
	int                           t_iColX,t_iColZ;

	_pos.y = 0.0f;

	// River mod @ 2007-4-18:
	if( (_pos.x < 0.0f)||(_pos.z<0.0f)||
		(_pos.x > g_fMapMaxX) || (_pos.z > g_fMapMaxZ) )
	{
		_pos.y = 0.0f;
		return false;
	}


	// 先算出整张大地图格子的索引
	t_fU = _pos.x / TILE_WIDTH;
	t_fV = _pos.z / TILE_WIDTH;
	t_iColX = int(t_fU);
	t_iColZ = int(t_fV);

	// 得到碰撞信息
	t_tgIdx.m_iX = WORD(t_iColX / MAX_GROUNDWIDTH);
	t_tgIdx.m_iY = WORD(t_iColZ / MAX_GROUNDWIDTH);

	// windy add 4-27
	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		_pos.y = 0.0f;
		return false;
	}
	const float* t_tileColHeightInfo;
	t_tileColHeightInfo = osc_mapTerrain::
		get_mapTerrainIns()->get_mapTileHeightInfo( t_tgIdx.m_iX,t_tgIdx.m_iY );
	if( NULL ==  t_tileColHeightInfo )
	{
		_pos.y = 0.0f;
		return false;
	}
	
	// 算传入坐标在一个显示格子内的uv.
	t_fU = t_fU - t_iColX;
	t_fV = t_fV - t_iColZ;

	// 单独显示格子的原点坐标x,z值
	t_fOrigX = t_iColX*TILE_WIDTH;
	t_fOrigZ = t_iColZ*TILE_WIDTH;

	t_iColX -= (t_tgIdx.m_iX*MAX_GROUNDWIDTH);
	t_iColZ -= (t_tgIdx.m_iY*MAX_GROUNDWIDTH);

	osVec3D v00( t_fOrigX,
		t_tileColHeightInfo[65*t_iColZ+t_iColX],
				 t_fOrigZ );
	osVec3D v10( t_fOrigX +TILE_WIDTH,
		t_tileColHeightInfo[65*t_iColZ+t_iColX+1],
				t_fOrigZ );
	osVec3D v01( t_fOrigX,
		t_tileColHeightInfo[65*(t_iColZ+1)+t_iColX],
				t_fOrigZ + TILE_WIDTH );
	osVec3D v11( t_fOrigX +TILE_WIDTH,
		t_tileColHeightInfo[65*(t_iColZ+1)+t_iColX+1],
				t_fOrigZ+TILE_WIDTH );

	osPlane t_plane;
	if( (t_fU+t_fV)<1.0f )
	{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v00);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
	}
	else{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v11);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
	}
    
	osassertex( !_isnan( _pos.y ),"有可能是地图资料的高度不是浮点数造成的问题" );

	return true;

	unguard;
}

/** \brief 
*	根据一个世界坐标，返回一个DWORD的值，表明这个世界坐标的光照图的颜色。
*
*	\param osVec3D& _pos		x,z 有作用,为世界坐标
*/
DWORD osc_TGManager::get_lightMapColor(const osVec3D& _pos)
{
	guard;

	//! river @ 2009-12-15:释放地图后，有可能某些地方会调用此值，确保不出错
	if( !m_bLoadedMap )
		return 0xff000000;

	// River mod @ 2007-4-18:
	if( (_pos.x < 0.0f)||(_pos.z<0.0f)||
		(_pos.x > g_fMapMaxX) || (_pos.z > g_fMapMaxZ) )
		return 0xff000000;

	const os_tileIdx t_tgIdx(WORD((int)(_pos.x / TILE_WIDTH) / MAX_GROUNDWIDTH),
							 WORD((int)(_pos.z / TILE_WIDTH) / MAX_GROUNDWIDTH));

	// River mod @ 2007-8-26:
	// 对于激活地图列表相关的处理，使用临界区，界为上层使用了多线程
	::EnterCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

	// windy add 4-27
	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

		return 0xff000000;
	}

	osVec3D t_tmpPos = _pos;
	DWORD t_color = 0xff000000;
	if(get_objHeightInfo(t_tmpPos) <= -9999.0f){
		// 不在桥类物品上面,就计算阴影
		//
		t_color = osc_mapTerrain::get_mapTerrainIns()->get_mapTileLightInfo(_pos);
	}
	
	::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

	return t_color;


	unguard;
}
# define INTERSECT_TESTOFFSET 1000.0f

//! 得到位置射线与传入tg高度相交的高度数据，用于更精确的得到桥类物品的碰撞信息
float osc_TGManager::get_intersectHeight( osc_TileGroundPtr  _tg,
						int _objNum,const int* _objList,osVec3D& _pos )
{
	guard;

	osassert( _tg );

	if( _objNum == 0 )
		return INTERSECT_TESTOFFSET;

	osVec3D   t_vec3Start,t_vec3Dir;

	t_vec3Start = _pos;
	t_vec3Start.y += INTERSECT_TESTOFFSET;

	t_vec3Dir = osVec3D( 0.0f,-1.0f,0.0f );

	//! 对每一个物品的id进行检测,查看射线跟每一个物品的相交信息.
	float      t_fObjHeight = -10*INTERSECT_TESTOFFSET;
	float      t_fDis = INTERSECT_TESTOFFSET*10;
	BOOL       t_bRes;

	for( int t_i = 0;t_i<_objNum;t_i ++ )
	{
		t_bRes = _tg->is_intersectObj( _objList[t_i],
			t_vec3Start,INTERSECT_TESTOFFSET*10,t_vec3Dir,&t_fDis );
		if( TRUE == t_bRes )
		{
			if( t_fObjHeight < (t_vec3Start.y - t_fDis) )
				t_fObjHeight = t_vec3Start.y - t_fDis;
		}
	}

	return t_fObjHeight;

	unguard;
}


/** \brief
 *  得到场景内某个位置上跟地表物品相关的高度信息,如果没有物品,则高度信息为很小的负值.
 *
 *  算法描述:
 *  1: 得到当前位置的chunk内的物品列表.
 *  2: 查看当前位置往上的射线跟其中哪一个物品相关,找到最高的交点.
 *  3: 返回交点的Y值坐标.
 *  
 */
float osc_TGManager::get_objHeightInfo( osVec3D& _pos )
{
	guard;

	//! 根据位置得到所在tg
	osc_TileGroundPtr    t_ptrTG;
	float                t_fObjHeight = -100000.0f;

	int  t_x = int(_pos.x / (TILE_WIDTH*MAX_GROUNDWIDTH));
	int  t_y = int(_pos.z / (TILE_WIDTH*MAX_GROUNDWIDTH));

	t_ptrTG = get_tground( t_x,t_y );
	if( !t_ptrTG )
		return t_fObjHeight;

	//! 得到物品的列表.
	static int  t_iObjList[32];
	int         t_iObjNum = 0;

	// River mod @ 2007-4-24:桥类物品可能只占用一个mapTile,但体保超出了
	// 当前的tile,所以需要跟周围的mapTile的quadTree进行处理，得到更精确
	// 碰撞检测
	// 得到周围的tileGround指针，然后处理碰撞，是否有碰撞物品。
	for( int x = t_x-1;x<=t_x+1;x++ )
	{
		for( int y = t_y-1;y<=t_y+1;y++ )
		{
			if( (x<0)||(x>=m_iMaxXTgNum)||
				(y<0)||(y>=m_iMaxZTgNum) )
				continue;

			t_ptrTG = get_tground( x,y );
			if( !t_ptrTG )
				continue;

			// 不需要处理中心tileGround和超出地图范围的。
			if( (x==t_x)&&(y==t_y) )
			{
				t_iObjNum = t_ptrTG->
					m_sQuadTree.get_posInChunkObj( _pos,t_iObjList,128 );
			}
			else
			{
				t_iObjNum = t_ptrTG->
					m_sQuadTree.get_posExceedQuadObj( _pos,t_iObjList,128 );
			}

			if( t_iObjNum == 0 )
				continue;

			//! 计算要碰撞的桥类物品高度
			float   t_fNewHeight = get_intersectHeight( 
				t_ptrTG,t_iObjNum,t_iObjList,_pos );

			if( t_fNewHeight > t_fObjHeight )
				t_fObjHeight = t_fNewHeight;
		}
	}

	return t_fObjHeight;

	unguard;
}
