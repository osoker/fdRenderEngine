//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdSphereColl.cpp
 *
 *  His:      River created @ 2006-7-4
 *
 *  Des:      对一个场景内的bounding Sphere进行碰撞检测.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"



//! 最大使用10*10个格子的bsphere. 
//! River @ 2010-7-8:从10修改为13,如果还不够，则研究如何处理
# define MAX_BSTILENUM    13

//!
# define MAX_TILEIDXSUM   64

//! 为节省查找地表高度信息的工作量
struct os_tgHeightInfo
{
	const float*  m_ptrHeightInfo;
	WORD          m_wXIndex;
	WORD          m_wZIndex;
};

/** \brief
 *  对tgMgr的球形碰撞函数进行管理的类
 */
class osc_tgSphereColl
{
private:
	osc_TGManager*    m_ptrTGMgr;

	//! bsphere影响的地表格子
	os_tileIdx        m_sTileIdxMin;
	os_tileIdx        m_sTileIdxMax;

	//! 受影响的格子在x和z方向上的数目
	int               m_iTileNumInXZDir;

	//! bsphere影响的tileGround.
	os_tileIdx        m_sTgIdxMin;
	os_tileIdx        m_sTgIdxMax;

	//! bsphere影响的地表格子的高度数据集合
	osVec3D*          m_vec3TilePos;
	float             m_fMinHeight;
	float             m_fMaxHeight;

	//! 当前bsphere影响的地表格子所构成的aabbox.
	os_aabbox         m_sTileAABBox;

	//! 最多有四个tg受到影响
	os_tgHeightInfo   m_arrHeightPtr[4];

	//! 
	osVec2D           m_vec3NegaXzDir;
	osVec2D           m_vec3PosiXzDir;

	//! 当前碰撞信息用到的三角形的索引数组
	VEC_word          m_vecTriIdx;
	int               m_iTriangleNum;
	//! 每一个三角形对应的面
	osPlane*          m_sTriPlane;

	//! 跟sphere碰撞的三角形的索引
	VEC_word          m_vecCollTriIdx;
	int               m_iCollTriNum;

private:
	//! 得到某一个tg的指针
	const float*      find_height( WORD _x,WORD _z )
	{
		guard;

		if( (m_arrHeightPtr[0].m_wXIndex == _x)&&
			(m_arrHeightPtr[0].m_wZIndex == _z) )
			return m_arrHeightPtr[0].m_ptrHeightInfo;
		if( (m_arrHeightPtr[1].m_wXIndex == _x)&&
			(m_arrHeightPtr[1].m_wZIndex == _z) )
			return m_arrHeightPtr[1].m_ptrHeightInfo;
		if( (m_arrHeightPtr[2].m_wXIndex == _x)&&
			(m_arrHeightPtr[2].m_wZIndex == _z) )
			return m_arrHeightPtr[2].m_ptrHeightInfo;
		if( (m_arrHeightPtr[3].m_wXIndex == _x)&&
			(m_arrHeightPtr[3].m_wZIndex == _z) )
			return m_arrHeightPtr[3].m_ptrHeightInfo;

		//! River mod @ 2007-3-9:此处的assert去掉,上一次进行处理
		//osassertex( false,"出事了!!!球形地表碰撞找不到高度信息...\n" );

		return NULL;

		unguard;
	}

	void              construct_triangleList( void )
	{
		guard;

		int    t_iPlaneIdx = 0;
		m_iTriangleNum = 0;

		for( WORD t_z = m_sTileIdxMin.m_iY;t_z<=m_sTileIdxMax.m_iY;t_z ++ )
		{
			for( WORD t_x = m_sTileIdxMin.m_iX;t_x<=m_sTileIdxMax.m_iX;t_x ++ )
			{
				int     t_iXIdx = t_x - m_sTileIdxMin.m_iX;
				int     t_iZIdx = t_z - m_sTileIdxMin.m_iY;

				// 三角形的索引
				int     t_iTriIdx = m_iTriangleNum*3;
				m_vecTriIdx[t_iTriIdx]   = t_iZIdx*m_iTileNumInXZDir + t_iXIdx;
				m_vecTriIdx[t_iTriIdx+1] = (t_iZIdx+1)*m_iTileNumInXZDir + t_iXIdx;
				m_vecTriIdx[t_iTriIdx+2] = t_iZIdx*m_iTileNumInXZDir + t_iXIdx + 1;

				osPlaneFromPoints( &m_sTriPlane[t_iPlaneIdx],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+1]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+2]]  );

				m_vecTriIdx[t_iTriIdx+3] = t_iZIdx*m_iTileNumInXZDir + t_iXIdx + 1;
				m_vecTriIdx[t_iTriIdx+4] = (t_iZIdx+1)*m_iTileNumInXZDir + t_iXIdx;
				m_vecTriIdx[t_iTriIdx+5] = (t_iZIdx+1)*m_iTileNumInXZDir + t_iXIdx + 1;

				osPlaneFromPoints( &m_sTriPlane[t_iPlaneIdx+1],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+3]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+4]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+5]]  );

				m_iTriangleNum += 2;
				t_iPlaneIdx += 2;
			}
		}

		unguard;
	}



	//! 得到所有受影响格子高度值的函数
	bool              get_affectTileHeight( void )
	{
		guard;

		m_fMinHeight = 10000.0f;
		m_fMaxHeight = -10000.0f;

		int   t_idx = 0;
		for( WORD t_z = m_sTgIdxMin.m_iY;t_z<=m_sTgIdxMax.m_iY;t_z ++ )
		{
			for( WORD  t_x = m_sTgIdxMin.m_iX;t_x<=m_sTgIdxMax.m_iX;t_x ++ )
			{
				os_tileIdx   t_sTileIdx;
				t_sTileIdx.m_iX = t_x;
				t_sTileIdx.m_iY = t_z;
 
				// River @ 2010-9-30:加入缓和的客户端的出错处理。
				if( (t_x + t_z)>=MAX_TILEIDXSUM )
					return false;

				m_arrHeightPtr[t_idx].m_ptrHeightInfo = 
					m_ptrTGMgr->get_sceneColHeightData( t_sTileIdx );

				// River @ 2007-3-7:更加温柔的错误处理方式:
				if( NULL == m_arrHeightPtr[t_idx].m_ptrHeightInfo )
					continue;

				m_arrHeightPtr[t_idx].m_wXIndex = t_x;
				m_arrHeightPtr[t_idx].m_wZIndex = t_z;
				t_idx ++;
			}
		}
		for( int t_iNotUsed = t_idx;t_iNotUsed<4;t_iNotUsed ++ )
		{
			m_arrHeightPtr[t_iNotUsed].m_wXIndex = -1;
			m_arrHeightPtr[t_iNotUsed].m_wZIndex = -1;
		}

		//
		// 对每一个格子,计算此格子的高度信息
		for( WORD t_z = m_sTileIdxMin.m_iY;t_z<=(m_sTileIdxMax.m_iY+1);t_z ++ )
		{
			for( WORD t_x = m_sTileIdxMin.m_iX;t_x<=(m_sTileIdxMax.m_iX+1);t_x ++ )
			{
				// 高度信息每一个TG有65*65个
				const float* t_ptrHeight = find_height( t_x/64,t_z/64 );

				// River mod @ 2006-9-11:此处可能正常出现空的高度信息，因为actTglist使用的是上一帧的数据
				// WORK START:ATTENTION TO FIX:此处有一些必须处理的东西，这个空指针只要出现，程序必定要死
				// 尽量在此处加入更加好用的出错处理机制：
				if( !t_ptrHeight )
					continue;

				int   t_iXOffset = t_x%64;
				int   t_iZOffset = t_z%64;
				int   t_iTileHeightIdx = 
					(m_iTileNumInXZDir)*(t_z-m_sTileIdxMin.m_iY) + (t_x-m_sTileIdxMin.m_iX);

				m_vec3TilePos[t_iTileHeightIdx].y = t_ptrHeight[t_iZOffset*65+t_iXOffset];
				m_vec3TilePos[t_iTileHeightIdx].x = t_x*TILE_WIDTH;
				m_vec3TilePos[t_iTileHeightIdx].z = t_z*TILE_WIDTH;

				if( m_vec3TilePos[t_iTileHeightIdx].y < m_fMinHeight )
					m_fMinHeight = m_vec3TilePos[t_iTileHeightIdx].y;

				if( m_vec3TilePos[t_iTileHeightIdx].y > m_fMaxHeight )
					m_fMaxHeight = m_vec3TilePos[t_iTileHeightIdx].y;
			}
		}

		// 
		// River mod 2008-4-17:得到相机出错的更多信息
		if( m_fMinHeight > 9999.0f )
		{
			osDebugOut( va( "非正常的数据出现<%f,%f>.tile min<%d,%d>,tile Max<%d,%d>...",m_fMinHeight,m_fMaxHeight,
				m_sTileIdxMin.m_iX,m_sTileIdxMin.m_iY,m_sTileIdxMax.m_iX,m_sTileIdxMax.m_iY  ) );
			return false;
		}
		else
			return true;

		unguard;
	}

public:
	osc_tgSphereColl()
	{
		m_ptrTGMgr = NULL;
	
		// -xz方向和xy方向的常量
		m_vec3NegaXzDir = osVec2D( -0.70710671f,-0.70710671f );
		m_vec3PosiXzDir = osVec2D(  0.70710671f, 0.70710671f );

		m_vec3TilePos = new osVec3D[(MAX_BSTILENUM+1)*(MAX_BSTILENUM+1)];
		m_sTriPlane = new osPlane[2*(MAX_BSTILENUM+1)*(MAX_BSTILENUM+1)];
		
		// 
		m_vecTriIdx.resize( (MAX_BSTILENUM*MAX_BSTILENUM*2)*3 );
		m_vecCollTriIdx.resize( MAX_BSTILENUM*MAX_BSTILENUM );
	}
	~osc_tgSphereColl()
	{
		release();
	}

	void release(void)
	{
		m_vecTriIdx.clear();
		m_vecCollTriIdx.clear();
		SAFE_DELETE_ARRAY( m_vec3TilePos );
		SAFE_DELETE_ARRAY( m_sTriPlane );
	}

	//! 初始化管理器
	void         init_tgSphereColl( osc_TGManager* _tgMgr )
	{
		guard;

		osassert( _tgMgr );
		m_ptrTGMgr = _tgMgr;
		
		unguard;
	}


	//! 计算出当前的bsphere影响的地表格子.
	void         cal_affectTile( os_bsphere* _bs )
	{
		guard;

		// 
		// 计算x,z方向的最小值和最大值.直角三角形 a*a + b*b = c*c
		osVec2D        t_vec2Min,t_vec2Max;
		float          t_fBoundRadius = sqrt( 2*_bs->radius*_bs->radius );


		// River mod @ 2010-7-8:此处会出错，确认此值的大小。
		osassertex( (t_fBoundRadius/TILE_WIDTH+1)<MAX_BSTILENUM,
			va( "%s<%f>...\n","传入的bsphere超出了程序所能处理的大小",t_fBoundRadius ) );

		t_vec2Min.x = _bs->veccen.x;
		t_vec2Min.y = _bs->veccen.z;
		t_vec2Max   = t_vec2Min;

		t_vec2Min = t_vec2Min + m_vec3NegaXzDir*t_fBoundRadius;
		t_vec2Max = t_vec2Max + m_vec3PosiXzDir*t_fBoundRadius;
	
		m_sTileIdxMin.m_iX = int( t_vec2Min.x/TILE_WIDTH );
		m_sTileIdxMin.m_iY = int( t_vec2Min.y/TILE_WIDTH );
		m_sTileIdxMax.m_iX = int( t_vec2Max.x/TILE_WIDTH );
		m_sTileIdxMax.m_iY = int( t_vec2Max.y/TILE_WIDTH );

		m_iTileNumInXZDir = m_sTileIdxMax.m_iX + 2 - m_sTileIdxMin.m_iX;

		m_sTgIdxMin.m_iX = m_sTileIdxMin.m_iX/64;
		m_sTgIdxMin.m_iY = m_sTileIdxMin.m_iY/64;
		m_sTgIdxMax.m_iX = (m_sTileIdxMax.m_iX+1)/64;
		m_sTgIdxMax.m_iY = (m_sTileIdxMax.m_iY+1)/64;

		osassert( (m_sTgIdxMax.m_iX-m_sTgIdxMin.m_iX)<2 );
		osassert( (m_sTgIdxMax.m_iY-m_sTgIdxMin.m_iY)<2 );

		return;

		unguard;
	}



	//! 计算出这些地表格子的AABBox
	os_aabbox*   cal_etileAABBox( void )
	{
		//! 主要得到所有受影响格子的最高高度和最低高度
		// River mod @ 2008-4-17:得到更多的相机出错数据 
		if( !get_affectTileHeight() )
			return NULL;

		//
		m_sTileAABBox.m_vecMax.x = m_sTileIdxMax.m_iX*TILE_WIDTH+TILE_WIDTH;
		m_sTileAABBox.m_vecMax.z = m_sTileIdxMax.m_iY*TILE_WIDTH+TILE_WIDTH;
		m_sTileAABBox.m_vecMax.y = m_fMaxHeight;

		m_sTileAABBox.m_vecMin.x = m_sTileIdxMin.m_iX*TILE_WIDTH;
		m_sTileAABBox.m_vecMin.z = m_sTileIdxMin.m_iY*TILE_WIDTH;
		m_sTileAABBox.m_vecMin.y = m_fMinHeight;

		return &m_sTileAABBox;
	}

	//! 计算出哪一个三角面片跟此bsphere相交,得到三角形列表
	int         cal_intersectTriList( os_bsphere* _bs )
	{
		//! 根据每一个高度信息来构建三角形列表
		construct_triangleList();


		// 计算相交
		m_iCollTriNum = 0;
		for( int t_i=0;t_i<m_iTriangleNum;t_i ++ )
		{
			if( _bs->collision( &m_vec3TilePos[m_vecTriIdx[t_i*3]],
				&m_vec3TilePos[m_vecTriIdx[t_i*3+1]],
				&m_vec3TilePos[m_vecTriIdx[t_i*3+2]],&m_sTriPlane[t_i] ) )
			{
				m_vecCollTriIdx[m_iCollTriNum] = t_i;
				m_iCollTriNum ++;
			}
		}

		return m_iCollTriNum;
	}

	//!  对每一个TriList内的三角形,计算出bsphere在posVector上移动多远才能使用bsphere不跟
	//!  此三角形碰撞,得到一个矩离列表.并返回最大矩离值.
	float        cal_passDistance( os_bsphere* _bs,osVec3D& _moveDir )
	{

		return 0.0f;
	}

};



//! 只需要在这个文件内使用
static osc_tgSphereColl   t_sTgSC;

/** \brief
*  计算一个包围球跟地表的碰撞检测.
*
*  算法结果描述:
*  每一次计算bsphere的碰撞时,需要传入上一次没有碰撞的bsphere所在的位置.如果当前的bsphere跟
*  地表之间有碰撞,则找到此bsphere最可能到达当前的位置,而又没有跟地表碰撞的位置.
*  
*  算法描述:
*  1: 先计算出当前的bsphere影响的地表格子.
*  2: 计算出这些地表格子的AABBox.
*  3: 如果此AABBox跟bsphere不碰撞,则返回 false
*  4: 计算出哪一个三角面片跟此bsphere相交,得到三角形列表.TriList
*  5: 计算出bsphere当前位置跟上一帧没有碰撞地表的位置的向量. posVector
*  6: 对每一个TriList内的三角形,计算出bsphere在posVector上移动多远才能使用bsphere不跟
*     此三角形碰撞,得到一个矩离列表 distanceList
*  7: 从distanceList内,找出最远的矩离,使用posVector计算出这个位置,赋值给_resPos,返回true.
*   
*  \return bool 如果碰撞,返回true,否则返回false.
*  \param  os_bsphere* _bs  要跟地形的碰撞信息做碰撞检测的bounding sphere
*/
bool osc_TGManager::get_sphereInterTerrPos( os_bsphere* _bs )
{
	guard;

	os_aabbox*    t_sAABBox = NULL;

	t_sTgSC.init_tgSphereColl( this );

	// River mod @ 20109-9-18：确认不会底层出错.
	if( _bs->radius > 10000.0 )
		return true;

	t_sTgSC.cal_affectTile( _bs );
	t_sAABBox = t_sTgSC.cal_etileAABBox();

	// River mod @ 2008-4-17:加入更多的相机出错数据
	if( !t_sAABBox )
	{
		// 出错处理???得到sphere的位置和相关数据？
		osDebugOut( "The ERROR sphere is:<%f,%f,%f,%f>....\n",_bs->veccen.x,
			_bs->veccen.y,_bs->veccen.z,_bs->radius );
		//osassertex( false,"在此位置上，相机出错!!!!\n" );

		// River @ 2010-8-21:很少出错，直接返回不处理。
		return false;
	}

	// 如果跟aabbox不相交,直接返回false.
	if( !t_sAABBox->collision( *_bs ) )
		return false;


	// 如果实际上没有三角形相交,返回不碰撞.
	if( 0 == t_sTgSC.cal_intersectTriList( _bs ) )
		return false;

	return true;

	unguard;
}

//
//! tzz:
//		如果用osc_tgSphereColl 的析构函数来释放osc_tgSphereColl内部的资源的话，
//		就会导致 MFC 引用项目的时候对堆的内存检查问题，认为内存已经泄漏了，事实上没有
//		所以就将osc_TGManager::ReleaseStatic定义，在osc_engine::release中调用。
void osc_TGManager::ReleaseStatic(void)
{
	guard;
	t_sTgSC.release();	
	unguard;
}



/** \brief
*  根据一条射线来得到是否跟地表相交，以及相交点的位置。
*  
*  使用跟鼠标点选地表一样的核心算法。
*  \param osVec3D& _rayStart  射线的开始位置。
*  \param osVec3D& _rayDir    射线的方向
*  \param osVec3D& _resPos    如果相交，此值为返回的交点位置。
*  
*/
bool osc_TGManager::get_rayInterTerrPos( osVec3D& _rayStart,osVec3D& _rayDir,osVec3D& _resPos )
{
	guard;

	osVec3D    t_vec3RayDir = _rayDir,t_vec3RayEnd;
	osVec3D    t_vec3Tmp;
	osPlane         t_pickPlane;
	ose_geopos       t_arrPPClip[4];
	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );	
	os_Ray     pickRay(_rayStart,t_vec3RayDir );


	// 先得到当前射线顶点右方的向量,用于构建射线方向的平面
	t_vec3Tmp = osVec3D( 0.0f,1.0f,0.0f );
	osVec3Cross( &t_vec3Tmp,&t_vec3Tmp,&t_vec3RayDir );
	t_vec3Tmp = _rayStart + t_vec3Tmp;

	// 
	// 构建一个射线方向的平面，如果一个地表格子的四个顶点全在平面的一测，则
	// 这个格子肯定不和相机的射线相交。因为地表格子是凸面体。
	t_vec3RayEnd = _rayStart + t_vec3RayDir;
	osPlaneFromPoints( &t_pickPlane,&_rayStart,&t_vec3Tmp,&t_vec3RayEnd );

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// 以四叉树为基准的测试:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// 从当前TileGround的QuadTree得到相交的Chunk列表
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       _rayStart,t_vec3RayDir,t_ptrChunkIdx,t_ptrChunkXZ );
		if( 0 == t_iPickChunkNum )
			continue;

		// 对每一个ray相交的Chunk进行更细致的处理
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// 开始查找可能相交的tile格子	列表。
			int t_tileNum = m_sChunkPick.get_pickTile(
				_rayStart,t_vec3RayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// 对每一个tile格子查看是否相交
			const os_tileIdx*   t_ptrIdx = m_sChunkPick.get_testTileArr();
			int   t_iBeginx = t_ptrChunkIdx[t_i].m_iX*16;
			int   t_iBeginy = t_ptrChunkIdx[t_i].m_iY*16;
			int   t_iTx,t_iTz;

			for( int t_i=0;t_i<t_tileNum;t_i ++ )
			{
				//! 如果超出64，则不处理这个格子
				t_iTx = t_iBeginx + t_ptrIdx[t_i].m_iX;
				t_iTz = t_iBeginy + t_ptrIdx[t_i].m_iY;

				if( (t_iTx >= MAX_GROUNDWIDTH) ||
					(t_iTz >= MAX_GROUNDWIDTH) )
					continue;

				TilePtr->get_polyVertex( t_poly,t_iTx,t_iTz );


				// 
				// 先做大体的检测,根据凸多边原理,此处可以过滤掉大半的四边形
				t_arrPPClip[0] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[0] );
				t_arrPPClip[1] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[1] );
				t_arrPPClip[2] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[2] );
				t_arrPPClip[3] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[3] );
				if( (t_arrPPClip[0] == t_arrPPClip[1])&&
					(t_arrPPClip[1] == t_arrPPClip[2])&&
					(t_arrPPClip[2] == t_arrPPClip[3]) )
					continue;

				// 做三角形级别的相交测试
				if( pick_nearestTri( t_poly,pickRay,
					_resPos,_rayStart,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	return t_hr;

	unguard;
}


/** \brief
*  输入一条线段,来确认此线段是否跟地表相交,以及交点的位置.
*
*  这个函数主要用于优化相机跟地表的碰撞测试,核心算法来自函数get_rayInterTerrPos
*
*  \param osVec3D& _ls 线段的开始位置.
*  \param osVec3D& _le 线段的结束位置.
*  \param osVec3D& _resPos 如果相交,返回相交的结束位置.
*  \return bool            返回true,则跟地表相交.
*/
bool osc_TGManager::get_lineSegInterTerrPos( osVec3D& _ls,osVec3D& _le,osVec3D& _resPos )
{
	guard;


	osVec3D    t_vec3RayDir,t_vec3RayEnd;
	osVec3D    t_vec3Tmp;
	osPlane         t_pickPlane;
	ose_geopos       t_arrPPClip[4];
	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	t_vec3RayDir = _le -_ls;
	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );	
	os_Ray     pickRay( _ls,t_vec3RayDir );


	// 先得到当前射线顶点右方的向量,用于构建射线方向的平面
	t_vec3Tmp = osVec3D( 0.0f,1.0f,0.0f );
	osVec3Cross( &t_vec3Tmp,&t_vec3Tmp,&t_vec3RayDir );
	t_vec3Tmp = _ls + t_vec3Tmp;

	// 
	// 构建一个射线方向的平面，如果一个地表格子的四个顶点全在平面的一测，则
	// 这个格子肯定不和相机的射线相交。因为地表格子是凸面体。
	t_vec3RayEnd = _ls + t_vec3RayDir;
	
	// River @ 2006-9-9:此算法如果_ls和_le同在Y轴上，会出现问题。
	osassert( _ls != t_vec3Tmp );

	osPlaneFromPoints( &t_pickPlane,&_ls,&t_vec3Tmp,&t_vec3RayEnd );

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// 以四叉树为基准的测试:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// 从当前TileGround的QuadTree得到相交的Chunk列表
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       _ls,t_vec3RayDir,t_ptrChunkIdx,t_ptrChunkXZ, &_le );
		if( 0 == t_iPickChunkNum )
			continue;

		// 对每一个ray相交的Chunk进行更细致的处理
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// 开始查找可能相交的tile格子	列表。
			int t_tileNum = m_sChunkPick.get_pickTile(
				_ls,t_vec3RayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// 对每一个tile格子查看是否相交
			const os_tileIdx*   t_ptrIdx = m_sChunkPick.get_testTileArr();
			int   t_iBeginx = t_ptrChunkIdx[t_i].m_iX*16;
			int   t_iBeginy = t_ptrChunkIdx[t_i].m_iY*16;
			int   t_iTx,t_iTz;

			for( int t_i=0;t_i<t_tileNum;t_i ++ )
			{
				//! 如果超出64，则不处理这个格子
				t_iTx = t_iBeginx + t_ptrIdx[t_i].m_iX;
				t_iTz = t_iBeginy + t_ptrIdx[t_i].m_iY;

				if( (t_iTx >= MAX_GROUNDWIDTH) ||
					(t_iTz >= MAX_GROUNDWIDTH) )
					continue;

				TilePtr->get_polyVertex( t_poly,t_iTx,t_iTz );


				// 
				// 先做大体的检测,根据凸多边原理,此处可以过滤掉大半的四边形
				t_arrPPClip[0] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[0] );
				t_arrPPClip[1] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[1] );
				t_arrPPClip[2] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[2] );
				t_arrPPClip[3] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[3] );
				if( (t_arrPPClip[0] == t_arrPPClip[1])&&
					(t_arrPPClip[1] == t_arrPPClip[2])&&
					(t_arrPPClip[2] == t_arrPPClip[3]) )
					continue;

				// 做三角形级别的相交测试
				if( pick_nearestTri( t_poly,pickRay,
					_resPos,_ls,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	return t_hr;

	unguard;
}

