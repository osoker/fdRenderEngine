//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldStruct.cpp
 *
 *  His:      River created @ 2006-1-10
 *
 *  Des:      地图数据类相关的结构。
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


//! TG内部保存的他所在的管理器的指针。
osc_TGManager* osc_newTileGround::m_ptrTgMgr = NULL;

//! 调整水面高度时用 snowwin add
std::vector<float> osc_newTileGround::m_vecWaterHeight;

osc_newTileGround::osc_newTileGround()
{
	m_bInuse = FALSE;
	m_strMapFName = "";
	

	m_bInMTQueue = false;

}

osc_newTileGround::~osc_newTileGround()
{

}


//! 得到物品索引的BoundingBox.
void osc_newTileGround::get_objBBox( int _objIdx,os_bbox& _box )
{
	guard;

	os_aabbox*      t_ptrBox;
	os_newTGObject*     t_ptrObj;
	osVec3D         t_vec3Tmp;
	
	osassert( _objIdx <= m_iTGObjectNum );

	t_ptrObj = &m_vecTGObject[_objIdx];
	if( t_ptrObj->m_dwType == 0 )
	{
		t_ptrBox = g_ptrMeshMgr->get_meshBBox( m_vecObjectDevIdx[_objIdx] );

		//! river mod @ 2009-4-25:准备的物品缩放信息
		t_vec3Tmp.x = t_ptrBox->m_vecMax.x * t_ptrObj->m_vec3ObjectScale.x;
		t_vec3Tmp.y = t_ptrBox->m_vecMax.y * t_ptrObj->m_vec3ObjectScale.y;
		t_vec3Tmp.z = t_ptrBox->m_vecMax.z * t_ptrObj->m_vec3ObjectScale.z;
		_box.set_bbMaxVec( t_vec3Tmp   );

		t_vec3Tmp.x = t_ptrBox->m_vecMin.x * t_ptrObj->m_vec3ObjectScale.x;
		t_vec3Tmp.y = t_ptrBox->m_vecMin.y * t_ptrObj->m_vec3ObjectScale.y;
		t_vec3Tmp.z = t_ptrBox->m_vecMin.z * t_ptrObj->m_vec3ObjectScale.z;
		_box.set_bbMinVec( t_vec3Tmp );

		_box.set_bbYRot( t_ptrObj->m_fAngle );
		_box.set_bbPos( t_ptrObj->m_vec3ObjPos );	

		return;
	}
	
	_box.set_bbMaxVec( osVec3D( 0.0f,0.0f,0.0f ) );
	_box.set_bbMinVec( osVec3D( 0.0f,0.0f,0.0f ) );

	return;
	
	unguard;
}


/** \brief
*  查看一个一条射线是否和一个Tile的os_polygon相交。
*
*  \param _tileIdx  如果和当前格子的三角形相交，返回和当前格子内的哪
*                   一个碰撞格子相交。碰撞格子的索引：z值最大的，x值最小的方位为0,
*                   x,z最大的方位为1,z最小，x最大的方位为2。
*  \param _poly     传入的格子对应的渲染三角形。
*  
*/
bool osc_newTileGround::intersect_polygon( 
					const osVec3D* _arrVerData,osVec3D& _s,osVec3D& _e )
{
	guard;

	float           t_u,t_v,t_dis;
	osassert( _arrVerData );

	// 如果和第一个三角形相交。
	if( D3DXIntersectTri( &_arrVerData[0],
		&_arrVerData[1],
		&_arrVerData[2],&_s,&_e,&t_u,&t_v,&t_dis ) )
	{
		_e = _arrVerData[0] + 
			t_u*(_arrVerData[1]-_arrVerData[0] )
			+t_v*(_arrVerData[2] - _arrVerData[0] );
		return true;
	}

	//
	// 如果和第二个三角形相交,改变了三角形三个顶点的顺序，用于更好的使用
	// 相交函数返回的重心uv坐标。
	if( D3DXIntersectTri( &_arrVerData[3],
		&_arrVerData[2],
		&_arrVerData[1],&_s,&_e,&t_u,&t_v,&t_dis ) )
	{
		_e = _arrVerData[3] + 
			t_u*(_arrVerData[2]-_arrVerData[3] )
			+t_v*(_arrVerData[1] - _arrVerData[3] );
		return true;
	}

	return false;

	unguard;
}


//! 当前相机在TG内的哪一大块。
osn_inTgPos osc_newTileGround::m_seCamTgPos = OSN_TGPOS_ERROR;

/** \brief
*  根据传入的位置，判断相机所在的TG的哪一大块。
*
*  传入的参数为碰撞坐标在整个大地图上的位置。
*/
void osc_newTileGround::frame_moveCamInTgPos( int _camx,int _camy )
{
	guard;

	int t_x,t_y;


	
	t_x = _camx%GROUNDMASK_WIDTH;
	t_y = _camy%GROUNDMASK_WIDTH;

	m_seCamTgPos = OSN_TGPOS_ERROR;

	if( (t_x<MAX_GROUNDWIDTH)&&(t_y>=MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_TOPLEFT;
	
	if( (t_x>=MAX_GROUNDWIDTH)&&(t_y>=MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_TOPRIGHT;

	if( (t_x<MAX_GROUNDWIDTH)&&(t_y<MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_BOTLEFT;

	if( (t_x>=MAX_GROUNDWIDTH)&&(t_y<MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_BOTRIGHT;

	osassert( m_seCamTgPos != OSN_TGPOS_ERROR );

	return;
	unguard;
}

//! 得到相机在tg内的哪一块.
osn_inTgPos osc_newTileGround::get_camInTgPos( void )
{
	guard;

	return m_seCamTgPos;

	unguard;
}

/** \brief
*  把当前帧可见的object放到渲染列表中.
*/ 
void osc_newTileGround::push_objIdx( int _oidx )
{
	guard;

	osassert( _oidx<m_iTGObjectNum );
	osassert( _oidx>=0 );
	m_vecObjInView[_oidx] = true;

	return ;

	unguard;
}


/** \brief
*  得到可见的物品列表。
*
*  慢慢的优化这个算法，就简单的算法是只要是视域内物品，都是可见的物品。
*/
void osc_newTileGround::get_inviewObjList( void )
{
	guard;

	float           t_fRadius;
	osVec3D          t_vec3Pos;
	int             t_idx;
	os_newTGObject*     t_ptrObj;



	//
	// 根据物品的大小对物品的可见性进行处理。
	for( int t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		// windy mod 12-27
		if( !m_vecObjInView[t_i] )
			continue;

		// 从相机得到当前的物品是否应该显示到视野内。
		t_ptrObj = &m_vecTGObject[t_i];
		if( (t_ptrObj->m_dwType ==0)||
			((t_ptrObj->m_dwType == 1)&&
			 (!g_bUseAnimationObj) )     )
		{
			t_idx = m_vecObjectDevIdx[t_i];
			// 创建新的mesh.
			if( t_idx == -1 )
			{
				create_meshImmediate( t_i );
				t_idx = m_vecObjectDevIdx[t_i];
			}


			t_fRadius = g_ptrMeshMgr->get_meshRadius( t_idx );

			t_vec3Pos = m_vecTGObject[t_i].m_vec3ObjPos;

			// 
			//　相机做完可见性判断，如果不在视野内，则当前的物品为不可见。
			//   只有静态的物品才使用keyObj属性
			if( !g_ptrMeshMgr->is_keyObj( t_idx ) )
			{
				m_vecObjInView[t_i] = 
					g_ptrCamera->is_viewByCamera( t_vec3Pos,t_fRadius );
			}
			else
			{
				m_vecObjInView[t_i] = MASK_HALF_VIEW;
			}

		}
		// 如果是skinMeshObj,加入到skinMeshObj队列。
		else if( t_ptrObj->m_dwType == 1 )
		{

			osc_skinMesh*    t_smObj;
				
			t_smObj = (osc_skinMesh*)m_vecObjectDevIdx[t_i];
			t_smObj->get_smPos( t_vec3Pos );
			t_fRadius = t_smObj->get_smBSRadius();
			m_vecObjInView[t_i] = 
				g_ptrCamera->is_viewByCamera( t_vec3Pos,t_fRadius );

		}
		else if( t_ptrObj->m_dwType == 4 )
		{
			// 先不处理osaMesh动画的情况;
			m_vecObjInView[t_i] = MASK_HALF_VIEW;
		}
		// 处理水面.
		else if( t_ptrObj->m_dwType == 2 )
		{
			m_vecObjInView[t_i] = MASK_HALF_VIEW;
		}
		else if( t_ptrObj->m_dwType == 8 )
		{
			m_vecObjInView[t_i] = MASK_HALF_VIEW;
		}
		else
		{
			m_vecObjInView[t_i] = false;
		}
	}

	// TEST CODE:
	//osassert( m_ivoMgr.m_vecFadeOutObj[0] < (int)m_vecTGObject.size() );
	//osassert( m_vecObjInView.size() <= m_vecTGObject.size() );

	// 
	// 使用当前TG内物品的可见性列表，对物品进行处理。
	m_ivoMgr.frame_processIvo( m_vecObjInView );

	// TEST CODE:
	//osassert( m_ivoMgr.m_vecFadeOutObj[0] < (int)m_vecTGObject.size() );

	return;

	unguard;
}


/** \brief
*   得到场景内的某个格子的顶点.
*  
*  场景中渲染多边形的示意：4个顶点的位置索引
*  
*    Z方向
*    |
*    |    1--------3
*    |    |        |
*    |    |        |
*    |    |        |
*    |    0--------2
*    |   
*   Origin-------------X方向
 */
void osc_newTileGround::get_polyVertex( osVec3D* _vec,int _x,int _z )
{
	guard;

	osassert( _x<64 );
	osassert( _z<64 );

	float   t_fXoffset = m_sTgHeader.m_wMapPosX*64*3.0f;
	float   t_fZoffset = m_sTgHeader.m_wMapPosZ*64*3.0f;

	_vec[0].x = _x*3.0f + t_fXoffset;
	_vec[0].z = _z*3.0f + t_fZoffset;
	_vec[0].y = m_vecHeight[_z*65 + _x];

	_vec[1].x = _vec[0].x;
	_vec[1].z = (_z+1)*3.0f + t_fZoffset;
	_vec[1].y = m_vecHeight[(_z+1)*65 + _x];

	_vec[2].x = (_x+1)*3.0f + t_fXoffset;
	_vec[2].z = _vec[0].z;
	_vec[2].y = m_vecHeight[_z*65 + _x + 1];

	_vec[3].x = _vec[2].x;
	_vec[3].z = _vec[1].z;
	_vec[3].y = m_vecHeight[(_z+1)*65 + _x + 1];

	return;

	unguard;
}

//! 设置一个地表的数据Chunk为可画的状态
void osc_newTileGround::set_chunkDraw( int _idx,BOOL _draw/* = TRUE*/ )
{
	guard;

	int   t_x,t_z;

	osassert( _idx < CHUNKNUM_PERTG );

	t_z = _idx / MapTile::CHUNKS;
	t_x = _idx - t_z*MapTile::CHUNKS;

	m_sMapTerrain.SetChunkVisible( t_x,t_z,_draw );

	m_bEntireCullOff = FALSE;

	return;

	unguard;
}

//! 设置一个数据Chunk受动态光的影响
void osc_newTileGround::set_chunkAffectByDL( int _idx )
{
	int   t_x,t_z;

	osassert( _idx < CHUNKNUM_PERTG );

	t_z = _idx / MapTile::CHUNKS;
	t_x = _idx - t_z*MapTile::CHUNKS;

	m_sMapTerrain.setChunkAffectByDl( t_x,t_z );

	return;

}



//! FrameSet,每一帧重置当前tg内的数据
void osc_newTileGround::frame_setTG( void )
{
	guard;

	m_bEntireCullOff = TRUE;

	// 重设当前TG内所有的Object为不可见。
	if( m_iTGObjectNum > 0 )
		memset( &m_vecObjInView[0],0,sizeof( BYTE )*m_iTGObjectNum );

	//　重设所有的obj的动态光为零
	for( int t_i=0;t_i<m_iTGObjectNum;t_i ++ )
		m_vecTGObjDL[t_i].reset_dl();

	
	//
	// 设置当前TG的InviewObjMgr。
	m_ivoMgr.frame_setObjMgr();
	    
	osMatrix   t_sMatReflect;
    osPlane    t_sClipPlane;

	osPlaneFromPointNormal( &t_sClipPlane,
		&osVec3D( 0.0f,m_ptrTgMgr->m_sSceneTexData.get_waterHeight(),0.0f ),
		&osVec3D( 0.0f,1.0f,0.0f ) );

	osMatrixReflect( &t_sMatReflect,&t_sClipPlane );

	m_sQuadTree.cal_visualTileSet( t_sMatReflect );

	// 
	// 使用当前TG内物品的可见性列表，对物品进行处理。
	get_inviewObjList();


	return;

	unguard;
}

//! 构建Chunk四周的四个面
void osc_fdChunkPick::construct_clipPlanes( float _minX,float _minZ )
{
	guard;

	osVec3D         t_vec3Pt[4];
	osVec3D         t_vec3Temp;
	float           t_fMaxX,t_fMaxZ;

	t_fMaxX = _minX + TILE_WIDTH*MapTile::GRIDS;
	t_fMaxZ = _minZ + TILE_WIDTH*MapTile::GRIDS;

	t_vec3Pt[0].x = _minX;t_vec3Pt[0].y = 0.0f;t_vec3Pt[0].z = _minZ;
	t_vec3Pt[1].x = _minX;t_vec3Pt[1].y = 0.0f;t_vec3Pt[1].z = t_fMaxZ;
	t_vec3Pt[2].x = t_fMaxX;t_vec3Pt[2].y = 0.0f;t_vec3Pt[2].z = t_fMaxZ;
	t_vec3Pt[3].x = t_fMaxX;t_vec3Pt[3].y = 0.0f;t_vec3Pt[3].z = _minZ;

	
	t_vec3Temp = t_vec3Pt[0]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[0],&t_vec3Pt[0],&t_vec3Temp,&t_vec3Pt[1] );

	t_vec3Temp = t_vec3Pt[1]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[1],&t_vec3Pt[1],&t_vec3Temp,&t_vec3Pt[2] );

	t_vec3Temp = t_vec3Pt[2]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[2],&t_vec3Pt[2],&t_vec3Temp,&t_vec3Pt[3] );

	t_vec3Temp = t_vec3Pt[3]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[3],&t_vec3Pt[3],&t_vec3Temp,&t_vec3Pt[0] );

	unguard;
}

void osc_fdChunkPick::push_testTile( int _x,int _z )
{
	guard;

	osassert( this->m_iTestTileNum > 0 );

	// River mod @ 2007-10-8:
	// 加入处理,如果传入的值有问题,则不处理这个格子.
	if( _x>=MapTile::GRIDS )
		return;
	if( _z>=MapTile::GRIDS )
		return;

	osassertex( _x<MapTile::GRIDS,va( "x val is:<%d>",_x) );
	osassertex( _z<MapTile::GRIDS,va( "z val is:<%d>",_z) );
	
	/*
	osassert( _x>=0 );
	osassert( _z>=0 );
	*/
	// River @ 2011-10-14:如果有问题，则不处理此格子.
	if( _x < 0 )
		return;
	if( _z < 0 )
		return;

	if( (m_arrTestTile[m_iTestTileNum-1].m_iX == _x)&&
		(m_arrTestTile[m_iTestTileNum-1].m_iY == _z) )
		return;
	m_arrTestTile[m_iTestTileNum].m_iX = _x;
	m_arrTestTile[m_iTestTileNum].m_iY = _z;
	m_iTestTileNum ++;
	return;

	unguard;
}




//
//! 根据传入的射线和开始格子坐标，依次查找可能相关的格子
int osc_fdChunkPick::find_testTileArr( float _minZ,int _startX )
{
	guard;
	osVec3D   t_vec3IntsetPt;
	osVec3D*  t_ptr;
	static const int t_iClip1 = 1,t_iClip2 = 3;

	osassert( m_arrTestTile );
	osassert( m_arrPlane );

	// 
	// 在Ｘ方向上，对每一个Chunk内的Z方向边,构建垂直于XZ平面的面，
	// 然后计算传入的射线跟这个面的交点，根据结果进行不同的处理
	for( int t_i=_startX;t_i<=MapTile::GRIDS;t_i ++ )
	{
		osPlane   t_tmpPlane;
		t_tmpPlane = m_arrPlane[0];
		t_tmpPlane.d -= (t_i*TILE_WIDTH);

		t_ptr = osPlaneIntersectLine( &t_vec3IntsetPt,
			&t_tmpPlane,&this->m_vec3RayStart,&this->m_vec3RayEnd );
		
		// 
		// 如果Ray射线跟Z方向平行,则加入所有的Z方向上的格子
		if( NULL == t_ptr )
		{
			for( int t_z=m_arrTestTile[m_iTestTileNum-1].m_iY;t_z < 16;t_z ++ )
				push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_z );

			return m_iTestTileNum;
		}

		// 
		// 交点跟第1,3个面的clip数据
		ose_geopos  t_pos1 = osn_mathFunc::classify_PointByPlane(
			m_arrPlane[t_iClip1],t_vec3IntsetPt );
		ose_geopos  t_pos2 = osn_mathFunc::classify_PointByPlane(
			m_arrPlane[t_iClip2],t_vec3IntsetPt );

		if( ( OSE_BACK != t_pos1 ) && ( OSE_BACK != t_pos2 ) )
		{
			int t_iZTileIdx = int((t_vec3IntsetPt.z-_minZ)/TILE_WIDTH);

			// 如果非相邻相交　
			if( t_iZTileIdx > m_arrTestTile[m_iTestTileNum-1].m_iY )
			{
				for( int t_idx = m_arrTestTile[m_iTestTileNum-1].m_iY+1;
					t_idx < t_iZTileIdx;t_idx ++ )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_idx );
				}
			}
			else
			{
				for( int t_idx = m_arrTestTile[m_iTestTileNum-1].m_iY-1;
					t_idx > t_iZTileIdx;t_idx -- )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_idx );
				}
			}

			//! River @ 2006-3-14:因为浮点运算的不精确，此处可能会大于等于16。
			if( t_iZTileIdx < MapTile::GRIDS )
				push_testTile( t_i-1,t_iZTileIdx );

			
			if( (t_i < MapTile::GRIDS)&&(t_iZTileIdx < MapTile::GRIDS) )
				push_testTile( t_i,t_iZTileIdx );

		}
		else
		{
			// 
			// 与竖线的交点在clip面之外的处理
			// 如果在编号为1的clip面之外,则加入最后一个可能相交tile在此Chunk
			// 内Z方向上所有的Tile.
			if( OSE_BACK == t_pos1 )
			{
				for( int t_i=m_arrTestTile[m_iTestTileNum-1].m_iY+1;
					t_i < MapTile::GRIDS;t_i ++ )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_i );
				}
			}
			// 
			// 如果在编号为3的clip面之外,则加入最后一个可能相交tile在此Chunk
			// 内-Z方向上所有的Tile.
			if( OSE_BACK == t_pos2 )
			{
				for( int t_i=m_arrTestTile[m_iTestTileNum-1].m_iY-1;
					t_i >= 0;t_i -- )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_i );
				}
			}

			break;
		}
	}

	return m_iTestTileNum;


	unguard;
}



/** \brief
*  处理chunk内被射线分割的tileIdx
*
*  \param _rayStart,_rayDir 鼠标pick射线的方向
*  \param _minX,_minZ       要处理Chunk的x,z坐标最小值
*  大致的算法描述：       
*     
*         Z方向
*         |      1/
*   pt[1] -------/----------- pt[2]
*         |  |  /  |        |
*         -----/-------------
*         |  |/ |  |        |
*         ---/---------------
*      0  | /|  |           |  2
*         -/-----------------   
*         /  |  |           |
*        /-------------------
*Ray start|  |  |           | pt[3]
*   pt[0] ------------------------X方向
*               3
*
*  一：先把整个Chunk最外面的四条边编号为0,1,2,3.
*
*  二：把四边边构建四个垂直于xz平面的面,面的法向量朝内四边形的内部。
*
*  三：如果Ray跟第零个面的交点在第1,3面之前，则找出此交点对应的格子索引，然后
*      设置此格子为可能跟射线相交的格子。
*      接下来为第零边右边紧接着的Chunk内的一条边构建一个垂直于xz平面的面，然后
*      找出面跟射线的交点出来,设置相应的可能跟射线相交的格子。
*      直到以下发生：交点在面1或3之外，或者到达了编号为2的面。
*  
*  四：如果步骤三不成立，则对第3个面进行处理。找到正确的交线,因为如果一条射线跟
*      一个Chunk相关，那么必须跟至少两条边相交，所以只需要处理第零和第1,3条边就ok了。
*      
*      对第三个面的处理，首先要找到射线跟第三个面的交点在Chunk内的格子索引的X值，
*      然后从此X值开始，象步骤三那样对此值右边的Chunk内边构建面，处理。
*    
*/
int osc_fdChunkPick::get_pickTile( osVec3D&  _rayStart,
					  osVec3D& _rayDir,float _minX,float _minZ )
{
	guard;

	osVec3D      t_vec3IntsetPt;

	osassert( _minX >= 0.0f );
	osassert( _minZ >= 0.0f );

	m_iTestTileNum = 0;

	//
	m_vec3RayStart = _rayStart;
	m_vec3RayEnd = _rayStart + 100000.0f*_rayDir;
	construct_clipPlanes( _minX,_minZ );

	if( osPlaneIntersectLine( &t_vec3IntsetPt,
		&m_arrPlane[0],&m_vec3RayStart,&m_vec3RayEnd ) )
	{
		if((OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[1],t_vec3IntsetPt))&&
		  (OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[3],t_vec3IntsetPt)) )
		{
			m_arrTestTile[m_iTestTileNum].m_iX = 0;
			m_arrTestTile[m_iTestTileNum].m_iY = int((t_vec3IntsetPt.z-_minZ)/TILE_WIDTH );
			m_iTestTileNum ++;

			find_testTileArr( _minZ,1 );

			return m_iTestTileNum;
		}
	}

	//
	// 和第3个面相交处理
	if( osPlaneIntersectLine( &t_vec3IntsetPt,
		&m_arrPlane[3],&m_vec3RayStart,&m_vec3RayEnd ) )
	{
		if( (OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[0],t_vec3IntsetPt))&&
			(OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[2],t_vec3IntsetPt)) )
		{
			m_arrTestTile[m_iTestTileNum].m_iX = 
				int((t_vec3IntsetPt.x - _minX)/TILE_WIDTH);
			m_arrTestTile[m_iTestTileNum].m_iY = 0;
			m_iTestTileNum ++;

			find_testTileArr( _minZ,m_arrTestTile[m_iTestTileNum-1].m_iX+1 );

			// 
			// 在m_iTestTileNum==1时，有可能是如下的情形导致，在这种情形下，
			// find_testTileArr函数不能正确的处理出可能相交的格子列表，所以移到下一步处理。
			// 下一步计算时，会处理这条射线跟编号为１的clip面相交的情形
			/*
			* 
			*         Z方向
			*         |      1
			*   pt[1] -------\------------ pt[2]
			*         |  |  | \|  |  |   |
			*         ---------\----------
			*         |  |  |  |\ |  |   |
			*         -----------\--------
			*      0  |  |  |  |  \  |   |  2
			*         -------------\------   
			*         |  |  |  |  | \|   |
			*         ---------------\----
			*         |  |  |  |  |  |\  | pt[3]
			*   pt[0] -----------------\-------X方向
			*               3         Ray start
			*  
			*/
			if( m_iTestTileNum > 1 )
				return m_iTestTileNum;
		}

	}


	// 和第1个面相交的处理
	if( osPlaneIntersectLine( &t_vec3IntsetPt,
		&m_arrPlane[1],&m_vec3RayStart,&m_vec3RayEnd ) )
	{
		if((OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[0],t_vec3IntsetPt))&&
		   (OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[2],t_vec3IntsetPt)) )
		{
			m_arrTestTile[m_iTestTileNum].m_iX = 
				int((t_vec3IntsetPt.x - _minX)/TILE_WIDTH);
			m_arrTestTile[m_iTestTileNum].m_iY = 15;
			m_iTestTileNum ++;

			find_testTileArr( _minZ,m_arrTestTile[m_iTestTileNum-1].m_iX+1 );
	
			return m_iTestTileNum;
		}
	}

	return m_iTestTileNum;

	unguard;
}




