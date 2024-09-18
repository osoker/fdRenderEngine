//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTerrainDecal.cpp
 *
 *  His:      River created @ 2006-3-6
 *
 *  Des:      用于在当前的地表上创建Decal.
 *   
 * “具有最高的智商的标志是，具有两个完全相反的想法在头脑中，而行为丝毫不受影响的能力”
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdTerrainDecal.h"
# include "../include/fdFieldMgr.h"

# include "../../backPipe/include/mainEngine.h"


//! 定义decal消失使用的时间
# define  DECAL_FADEINSTART    0.6f

//! Decal和地图的偏移矩离，为了使decal之间减少zFighting 效果，每一个Decal使用不同的值
# define  MINDECAL_OFFSET         0.05f
# define  MAXDECAL_OFFSET         0.10f


//! 场景管理和中间管道的指针.
osc_TGManager* osc_decal::m_ptrTGMgr = NULL;
osc_middlePipe* osc_decal::m_ptrMPipe = NULL;
//! 创建decal时，存放decal剪切tile格子的四个面。
osPlane osc_decal::m_arrDecalPlane[4];

//! 旋转后的u和v source Plane,每一个顶点到此面的矩离，好为u.
osPlane osc_decal::m_sUSourcePlane;      
osPlane osc_decal::m_sVSourcePlane;

// 渲染Decal的meshSet.
os_polygonSet osc_decalMgr::m_sPolySet;


//! 初始化的decal顶点数目和索引数目
# define INIT_DECALVER_NUM  768
# define INIT_DECALIDX_NUM  INIT_DECALVER_NUM*2


osc_decal::osc_decal()
{
	m_vec3DecalVer.resize( INIT_DECALVER_NUM );
	m_vecDecalIdx.resize( INIT_DECALIDX_NUM );

	//! 得到此Decal的offset值，用于使Decal之间不出现Z Fightting 的现象
	m_fDecalOffset = getrand_fromfloatscope( MINDECAL_OFFSET,MAXDECAL_OFFSET );
	
	reset_decal();
}

osc_decal::~osc_decal()
{

}

void osc_decal::reset_decal( void )
{
	m_sRenderData.m_iVerNum = 0;
	m_iDecalIdxnum = 0;
	
	// 释放？
	m_sRenderData.m_iShaderId = -1;
	m_bAniDecal = false;
	m_fEtime = 0.0f;
	m_fRotAgl = 0.0f;
	m_fVanishTime = 0.0f;

	m_vec3DecalPos = osVec3D( -1.0f,0.0f,-1.0f );


	// 纹理动画和实例数据
	m_wXFrameNum = 1;
	m_wYFrameNum = 1;
	m_fFrameTime = DEFAULT_ANITIME;

	m_iCurXFrame = 0;
	m_iCurYFrame = 0;
	m_fAcctFrameTime = 0.0f;

	m_bHide = false;

	m_bInRender = false;

}

/** \brief
 *   处理得到decal用到的四个剪切面,在decal没有旋转分量时，使用这个
 *   函数，如果有旋转，则在decalMgr内指定相应的剪切面
 *
 *   \param _planeArray 如果此decal有旋转，则此参数传入剪切面
 */
void osc_decal::get_decalClipPlane( osPlane* _planeArray/* = NULL*/ )
{
	guard;


	if( _planeArray )
	{
		osassert( !float_equal( m_fRotAgl,0.0f ) );
		memcpy( m_arrDecalPlane,_planeArray,sizeof( osPlane )*4 );
	}
	else
	{
		osVec3D     t_vec3PlanePoint;
		osVec3D     t_vec3PlaneNormal;

		osassert( float_equal( m_fRotAgl,0.0f ) );

		// -X方向上的面,朝向x方向.
		t_vec3PlanePoint = this->m_vec3DecalPos;
		t_vec3PlanePoint.x -= this->m_fDecalSize;
		t_vec3PlaneNormal = osVec3D( 1.0f,0.0f,0.0f );
		osPlaneFromPointNormal( &m_arrDecalPlane[0],
			     &t_vec3PlanePoint,&t_vec3PlaneNormal );

		// x方向上的面。
		t_vec3PlanePoint = this->m_vec3DecalPos;
		t_vec3PlanePoint.x += this->m_fDecalSize;
		t_vec3PlaneNormal = osVec3D( -1.0f,0.0f,0.0f );
		osPlaneFromPointNormal( &m_arrDecalPlane[1],
			     &t_vec3PlanePoint,&t_vec3PlaneNormal );

		// -z方向上的面。
		t_vec3PlanePoint = this->m_vec3DecalPos;
		t_vec3PlanePoint.z -= this->m_fDecalSize;
		t_vec3PlaneNormal = osVec3D( 0.0f,0.0f,1.0f );
		osPlaneFromPointNormal( &m_arrDecalPlane[2],
			     &t_vec3PlanePoint,&t_vec3PlaneNormal );

		// z方向上的面.
		t_vec3PlanePoint = this->m_vec3DecalPos;
		t_vec3PlanePoint.z += this->m_fDecalSize;
		t_vec3PlaneNormal = osVec3D( 0.0f,0.0f,-1.0f );
		osPlaneFromPointNormal( &m_arrDecalPlane[3],
			     &t_vec3PlanePoint,&t_vec3PlaneNormal );

	}



	unguard;
}


//! 用于存储decal影响格子的临时缓冲区,存储了tile在世界空间的坐标，会超过64
os_tileIdx osc_decal::m_arrTileIdx[MAX_TILE_PERDECAL];
int osc_decal::m_iAffectNum = -1;


//! 根据一个中心点和一个Size,得到受影响的Tg格子索引
int osc_decal::get_affectTile( osVec3D& _pos,float _size )
{
	guard;

	osVec2D    t_vec2S,t_vec2E;

	// River mod @ 2007-4-18:
	if( (_pos.x < 0.0f)||(_pos.z<0.0f)||
		(_pos.x > g_fMapMaxX) || (_pos.z > g_fMapMaxZ) )
		return 0;

	m_iAffectNum = 0;

	t_vec2S.x = _pos.x - _size;
	t_vec2S.y = _pos.z - _size;
	if( t_vec2S.x < 0.0f )
		t_vec2S.x = 0.0f;
	if( t_vec2S.y < 0.0f )
		t_vec2S.y = 0.0f;
	t_vec2E.x = _pos.x + _size;
	t_vec2E.y = _pos.z + _size;

	t_vec2S.x /= TILE_WIDTH;
	t_vec2S.y /= TILE_WIDTH;
	t_vec2E.x /= TILE_WIDTH;
	t_vec2E.y /= TILE_WIDTH;

	// 遍历得到每一个受影响的Tile.
	for( int t_z = (int)t_vec2S.y;t_z <= int(t_vec2E.y);t_z ++ )
	{
		for( int t_x = (int)t_vec2S.x;t_x <= int(t_vec2E.x);t_x ++ )
		{
			osassertex( MAX_TILE_PERDECAL > m_iAffectNum,
				va( "Decal影响的格子数目过多,decal size is<%f>..",_size ) );
			m_arrTileIdx[m_iAffectNum].m_iX = t_x;
			m_arrTileIdx[m_iAffectNum].m_iY = t_z;
			m_iAffectNum ++;
		}
	}

	return m_iAffectNum;
	
	unguard;
}
int osc_decal::get_affectTileWithRot( osVec3D& _pos,float _size,float _rotAgl )
{
	guard;

	osassert( _pos.x>=0.0f );
	osassert( _pos.z>=0.0f );

	m_iAffectNum = 0;

	os_aabbox  t_tileAABBox;
	osVec3D    t_tileVer[4];
	os_bbox    t_tileBox;
	osVec3D    t_vec3Tmp;
	osVec2D     t_vec2S,t_vec2E;


	// 先算出decal的aabbox。
	t_vec3Tmp.x =  - _size;
	t_vec3Tmp.z =  - _size;
	t_vec3Tmp.y = -1.0f;
	t_tileBox.set_bbMinVec( t_vec3Tmp );

	t_vec3Tmp.x = _size;
	t_vec3Tmp.z = _size;
	t_vec3Tmp.y = 1.0f;
	t_tileBox.set_bbMaxVec( t_vec3Tmp );
	m_vec3DecalPos.y = 0.0f;
	t_tileBox.set_bbPos( m_vec3DecalPos );
	t_tileBox.set_bbYRot( _rotAgl );

	// 认定Decal的clipPlane,第0,1个Plane是top和bottom面
	osPlane   t_boxPlane[6];
	t_tileBox.get_boxPlane( t_boxPlane );
	get_decalClipPlane( &t_boxPlane[2] );

	// 
	t_tileBox.to_aabbox( t_tileAABBox );


	// 得到decal可能覆盖的tile.
	if( t_tileAABBox.m_vecMin.x < 0.0f )
		t_tileAABBox.m_vecMin.x = 0.0f;
	if( t_tileAABBox.m_vecMin.z < 0.0f )
		t_tileAABBox.m_vecMin.z = 0.0f;
	t_tileAABBox.m_vecMin.x /= TILE_WIDTH;
	t_tileAABBox.m_vecMin.z /= TILE_WIDTH;
	t_tileAABBox.m_vecMax.x /= TILE_WIDTH;
	t_tileAABBox.m_vecMax.z /= TILE_WIDTH;


	// 遍历得到每一个受影响的Tile.
	os_aabbox   t_sTestBox;
	for( int t_z=int(t_tileAABBox.m_vecMin.z);t_z<=int(t_tileAABBox.m_vecMax.z);t_z++ )
	{
		for( int t_x=int(t_tileAABBox.m_vecMin.x);t_x<=int(t_tileAABBox.m_vecMax.x);t_x++ )
		{
			// 检测此tile的顶点的顶点构成的aabbox是否
			t_sTestBox.m_vecMin = osVec3D( t_x*TILE_WIDTH,-1.0f,t_z*TILE_WIDTH );
			t_sTestBox.m_vecMax = osVec3D( (t_x+1)*TILE_WIDTH,1.0f,(t_z+1)*TILE_WIDTH );

			if( t_tileBox.collision( t_sTestBox ) )
			{
				m_arrTileIdx[m_iAffectNum].m_iX = t_x;
				m_arrTileIdx[m_iAffectNum].m_iY = t_z;
				m_iAffectNum ++;

				// 处理完一个tile，接下来处理
				continue;

			}
		}
	}

	// Catch the decal box sourcePlane
	osPlane    t_plane[6];
	t_tileBox.get_boxPlane( t_plane );
	m_sUSourcePlane = t_plane[2];
	m_sVSourcePlane = t_plane[4];


	osassert( m_iAffectNum > 0 );

	return m_iAffectNum;

	unguard;
}


//  使用现有的主程序，模型，场景，特效,人物，动作等等。
//  需要增加的内容：一些界面和细节,还有新的玩法。
//! TriangleFan的索引变成TriangleStrip的索引后，索引数目的变化。
int osc_decal::triFanNum2TriStripNum( int _tfIdxNum )
{
	guard;

	osassert( _tfIdxNum >= 3 );

	int   t_iTriangleNum = _tfIdxNum - 2;
	int   t_iTriStripIdxNum = 3 + 2; // 加入了两个是为了把离散的strip连接成一个

	if( t_iTriangleNum == 1 )
		return t_iTriStripIdxNum;
	else
	{
		// ATTENTION TO OPP:可以改进
		// 从第1个三角形开始,增加奇数三角形的时候，
		// 增加1个索引，增加偶数三角形的时候，增加两个索引。
		for( int t_i=1;t_i<t_iTriangleNum;t_i ++ )
		{
			if( 0 == (t_i % 2) )
				t_iTriStripIdxNum += 2;
			else
				t_iTriStripIdxNum ++;
		}
	}

	return t_iTriStripIdxNum;

	unguard;
}


/** \brief
*  Clip一个Tile格子三角形，生成新的Decal使用的三角形列表。
*
*  \param osVec3D* _pos 传入的三角形三个顶点，顺时针排列。
*/
void osc_decal::clip_tileTriangle( const osVec3D* _pos )
{
	guard;

	osassert( _pos );
	// 使用两个缓冲区用于存放切割后的三角形顶点
	static osVec3D       t_arrDecalVer[MAX_SPLITNUM];
	static osVec3D       t_arrDecalVerTmp[MAX_SPLITNUM];
	int                 t_iClipVerNum = 3;
	int                 t_iResVerNum = 0;

	memcpy( t_arrDecalVer,_pos,sizeof( osVec3D )*3 );
	for( int t_i=0;t_i<4;t_i ++ )
	{
		osn_mathFunc::split_PolyByPlane( m_arrDecalPlane[t_i],t_arrDecalVer,
			t_iClipVerNum,t_arrDecalVerTmp,t_iResVerNum );

		// 此三角形完全在clip面之外,直接返回
		if( 0 == t_iResVerNum )
			return;

		memcpy( t_arrDecalVer,t_arrDecalVerTmp,sizeof( osVec3D )*t_iResVerNum );
		t_iClipVerNum = t_iResVerNum;
	}

	// 先把顶点考到顶点缓冲区
	while( (t_iClipVerNum + m_sRenderData.m_iVerNum )>(int)m_vec3DecalVer.size() )
	{
		m_vec3DecalVer.resize( m_vec3DecalVer.size()*2 );
		osassertex( false,"decal需要处理的顶点过多" );
	}
	for( int t_i=0;t_i<t_iClipVerNum;t_i ++ )
		m_vec3DecalVer[m_sRenderData.m_iVerNum+t_i].m_vec3Pos = t_arrDecalVer[t_i];


	// 
	// 得到顶点是顺时针方向排列，从零个顶点开始，是triangleFan结构，
	// 把TriangleFan变成TriangleStrip比较复杂。
	// 构建一个triangleStrip的索引,在triangleStrip的索引内，
	// 把第一个顶点和最后一个顶点都重复两次，以构建Degenerate Triangle，
	// 把分散的三角形连接起来。
	int t_iIdxNum = triFanNum2TriStripNum( t_iClipVerNum );
	while( (t_iIdxNum + m_iDecalIdxnum)>(int)m_vecDecalIdx.size() )
		m_vecDecalIdx.resize( m_vecDecalIdx.size()*2 );

	// 第零个三角形
	m_vecDecalIdx[m_iDecalIdxnum++] = m_sRenderData.m_iVerNum + 1;
	m_vecDecalIdx[m_iDecalIdxnum++] = m_sRenderData.m_iVerNum + 1;
	m_vecDecalIdx[m_iDecalIdxnum++] = m_sRenderData.m_iVerNum + 2;
	m_vecDecalIdx[m_iDecalIdxnum++] = m_sRenderData.m_iVerNum;

	int  t_iStripIdx = m_sRenderData.m_iVerNum + 3;
	for( int t_iTriIdx = 1;t_iTriIdx < t_iClipVerNum-2;t_iTriIdx ++ )
	{
		if( 1 == (t_iTriIdx % 2) )
			m_vecDecalIdx[m_iDecalIdxnum++] = t_iStripIdx;
		else
		{
			// 插入triangleFan的第零个顶点
			m_vecDecalIdx[m_iDecalIdxnum++] = m_sRenderData.m_iVerNum;

			m_vecDecalIdx[m_iDecalIdxnum++] = t_iStripIdx;
		}

		t_iStripIdx ++;
	}

	// 最后一个索引
	m_vecDecalIdx[m_iDecalIdxnum] = m_vecDecalIdx[m_iDecalIdxnum-1];
	m_iDecalIdxnum ++;

	osassertex( m_iDecalIdxnum <=  INIT_DECALIDX_NUM,
		"Decal处理的索引过多" );

	m_sRenderData.m_iVerNum += t_iClipVerNum;

	return;

	unguard;
}


/** \brief
 *  计算切分顶点后，每一个decal顶点的uv坐标
 *
 *  算法描述：无旋转的算法：
 *  1: 算出uv原点
 *  2: 每一个顶点在uv方向上的位置减去uv原点，再除以整体uv的数值。
 *  
 *  有旋转的算法描述：
 *  算出uv原点对应的两个平面，即u平面和V平面，然后对每一个顶点,
 *  计算此顶点到这两个平面的矩离，然后除以Decal的二倍大小。
 *
 *  优化decal,动态填充顶点缓冲区。
 */
void osc_decal::cal_decalVerUv( void )
{
	guard;

	float        t_fUvLength = m_fDecalSize*2.0f;

	if( !float_equal( m_fRotAgl,0.0f ) )
	{
		for( int t_i=0;t_i<this->m_sRenderData.m_iVerNum;t_i ++ )
		{
			m_vec3DecalVer[t_i].m_vec2Uv.x = 
				osn_mathFunc::distance_V2P( m_vec3DecalVer[t_i].m_vec3Pos,m_sUSourcePlane );
			m_vec3DecalVer[t_i].m_vec2Uv.x /= t_fUvLength;

			m_vec3DecalVer[t_i].m_vec2Uv.y = 
				osn_mathFunc::distance_V2P( m_vec3DecalVer[t_i].m_vec3Pos,m_sVSourcePlane );
			m_vec3DecalVer[t_i].m_vec2Uv.y /= t_fUvLength;

			// Decal offset.
			m_vec3DecalVer[t_i].m_vec3Pos.y += m_fDecalOffset;

		}
	}
	else
	{
		float        t_fUSource,t_fVSource;

		t_fUSource = this->m_vec3DecalPos.x - m_fDecalSize;
		t_fVSource = this->m_vec3DecalPos.z - m_fDecalSize;

		for( int t_i=0;t_i<this->m_sRenderData.m_iVerNum;t_i ++ )
		{
			m_vec3DecalVer[t_i].m_vec2Uv.x = 
				(m_vec3DecalVer[t_i].m_vec3Pos.x - t_fUSource)/t_fUvLength;

			m_vec3DecalVer[t_i].m_vec2Uv.y = 
				(m_vec3DecalVer[t_i].m_vec3Pos.z - t_fVSource)/t_fUvLength;

			// Decal offset.
			m_vec3DecalVer[t_i].m_vec3Pos.y += m_fDecalOffset;

		}

	}

	return;

	unguard;
}

//! 计算有纹理动画的顶点uv坐标
void osc_decal::cal_decalVerUvTexAnim( void )
{
	guard;

	// 需要根据动画纹理相关的数据，来确定当前的纹理坐标范围
	float        t_fUStart,t_fVStart;
	float        t_fULength;
	float        t_fVLength;
	
	// 
	// 开始uv处理
	t_fULength = 1.0f/m_wXFrameNum;
	t_fVLength = 1.0f/m_wYFrameNum;

	t_fUStart = m_iCurXFrame*t_fULength;
	t_fVStart = m_iCurYFrame*t_fVLength;

	t_fULength = m_wXFrameNum*(m_fDecalSize*2.0f);
	t_fVLength = m_wYFrameNum*(m_fDecalSize*2.0f);

	if( !float_equal( m_fRotAgl,0.0f ) )
	{
		for( int t_i=0;t_i<this->m_sRenderData.m_iVerNum;t_i ++ )
		{
			m_vec3DecalVer[t_i].m_vec2Uv.x = 
				osn_mathFunc::distance_V2P( m_vec3DecalVer[t_i].m_vec3Pos,m_sUSourcePlane );
			m_vec3DecalVer[t_i].m_vec2Uv.x /= t_fULength;
			m_vec3DecalVer[t_i].m_vec2Uv.x += t_fUStart;

			m_vec3DecalVer[t_i].m_vec2Uv.y = 
				osn_mathFunc::distance_V2P( m_vec3DecalVer[t_i].m_vec3Pos,m_sVSourcePlane );
			m_vec3DecalVer[t_i].m_vec2Uv.y /= t_fVLength;
			m_vec3DecalVer[t_i].m_vec2Uv.y += t_fVStart;

		}
	}
	else
	{
		float        t_fUSource,t_fVSource;

		t_fUSource = this->m_vec3DecalPos.x - m_fDecalSize;
		t_fVSource = this->m_vec3DecalPos.z - m_fDecalSize;

		for( int t_i=0;t_i<this->m_sRenderData.m_iVerNum;t_i ++ )
		{
			m_vec3DecalVer[t_i].m_vec2Uv.x = 
				(m_vec3DecalVer[t_i].m_vec3Pos.x - t_fUSource)/t_fULength;
			m_vec3DecalVer[t_i].m_vec2Uv.x += t_fUStart;

			m_vec3DecalVer[t_i].m_vec2Uv.y = 
				(m_vec3DecalVer[t_i].m_vec3Pos.z - t_fVSource)/t_fVLength;
			m_vec3DecalVer[t_i].m_vec2Uv.y += t_fVStart;

		}
	}

	return;

	unguard;
}


//! 更新此decal的纹理动画信息
void osc_decal::frame_texAnim( float _etime )
{
	guard;

	m_fAcctFrameTime += _etime;
	if( m_fAcctFrameTime >= m_fFrameTime )
	{
		int     t_iFrameNum;

		t_iFrameNum = int(m_fAcctFrameTime/m_fFrameTime);
		m_fAcctFrameTime -= (t_iFrameNum*m_fFrameTime);

		for( int t_i = t_iFrameNum;t_i > 0;t_i -- )
		{
			m_iCurXFrame ++;
			if( m_iCurXFrame >= m_wXFrameNum )
			{
				m_iCurXFrame = 0;
				m_iCurYFrame ++;
			}

			if( m_iCurYFrame >= m_wYFrameNum )
				m_iCurYFrame = 0;
		}
	}

	return;

	unguard;
}


//! 对Decal进行FrameMove.计算decal的animation信息
bool osc_decal::frame_move( float _etime )
{
	guard;

	// 非AniDecal
	if( !m_bAniDecal )
		return true;

	//  完成了decal的显示周期
	this->m_fEtime += _etime;
	if( m_fEtime >= m_fVanishTime )
	{
		//! 当前decal完成
		reset_decal();
		return false;
	}

	float   t_fFadePercent,t_fAlphaVal = 1.0f;
	t_fFadePercent = m_fEtime / m_fVanishTime;
	if( t_fFadePercent >= DECAL_FADEINSTART )
	{
		t_fAlphaVal = (t_fFadePercent- DECAL_FADEINSTART)/(1.0f-DECAL_FADEINSTART);
		t_fAlphaVal = 1.0f - t_fAlphaVal;
	}

	// 更新每一个顶点的Alpha值
	DWORD   t_dwOrVal = BYTE(0xff*t_fAlphaVal) << 24;
	for( int t_i=0;t_i<m_sRenderData.m_iVerNum;t_i ++ )
	{
		m_vec3DecalVer[t_i].m_dwColor = m_dwDecalColor;
		m_vec3DecalVer[t_i].m_dwColor |= t_dwOrVal;
	}


	if( (m_wXFrameNum>1 )||(m_wYFrameNum>1) )
	{
		frame_texAnim( _etime );
		cal_decalVerUvTexAnim();
	}

	return true;

	unguard;
}

//! 重设decal的旋转和位置信息
void osc_decal::move_decal( osVec3D& _pos,float _rotAgl )
{
	guard;

	osVec3D   t_vec3Tmp = _pos;
	t_vec3Tmp.y = 0.0f;

	if( vec3_equal( _pos,this->m_vec3DecalPos ) &&
		float_equal( _rotAgl,this->m_fRotAgl )  )
		return ;

	osVec3D             t_vecPolyVer[4];
	osc_TileGroundPtr    t_ptrTg = NULL;
	BOOL                t_bBridgeDecal = FALSE;

	// 重设几何数据
	m_vec3DecalPos = t_vec3Tmp;
	m_fRotAgl = _rotAgl;
	m_sRenderData.m_iVerNum = 0;
	m_iDecalIdxnum = 0; 

	// 
	if( float_equal( m_fRotAgl,0.0f ) )
	{
		// 如果没有影响到的格子,直接返回.
		int  t_iAffectNum = 0;
		t_iAffectNum = get_affectTile( m_vec3DecalPos,m_fDecalSize );
		if( t_iAffectNum>0 )
			get_decalClipPlane();
		else
			return;
	}
	else
	{
		get_affectTileWithRot( m_vec3DecalPos,m_fDecalSize,m_fRotAgl );
	}

	osVec3D   t_vec3DetailPos = _pos,t_pos = _pos;
	m_ptrTGMgr->get_detailPos( t_vec3DetailPos,true );
	m_ptrTGMgr->get_detailPos( t_pos );
	if( t_vec3DetailPos.y - t_pos.y > 0.1f )
		t_bBridgeDecal = TRUE;


	// 得到每一个格子对应的三角形，然后使用decal的clipPlane，clip出新的三角形。
	static int t_iSLastX,t_iSLastZ;
	t_iSLastX = -1;
	t_iSLastZ = -1;

	for( int t_i=0;t_i<m_iAffectNum;t_i ++ )
	{
		int t_iXIdx,t_iZIdx;
		
		t_iXIdx = m_arrTileIdx[t_i].m_iX/MAX_GROUNDWIDTH;
		t_iZIdx = m_arrTileIdx[t_i].m_iY/MAX_GROUNDWIDTH;

		if( (t_iXIdx!=t_iSLastX)||(t_iZIdx!=t_iSLastZ) )
			t_ptrTg = m_ptrTGMgr->get_tground( t_iXIdx,t_iZIdx );

		if( !t_ptrTg )
			continue;

		// 得到顶点，使用Decal的clipPlane切割。
		t_ptrTg->get_polyVertex( t_vecPolyVer,m_arrTileIdx[t_i].m_iX%MAX_GROUNDWIDTH,
			                     m_arrTileIdx[t_i].m_iY%MAX_GROUNDWIDTH );

		// clip两个三角形，顶点索引：(0,1,2) 顶点索引 (1,3,2).
		clip_tileTriangle( t_vecPolyVer );
		t_vecPolyVer[0] = t_vecPolyVer[1];
		t_vecPolyVer[1] = t_vecPolyVer[3];
		clip_tileTriangle( t_vecPolyVer );

		// Catch.
		t_iSLastX = t_iXIdx;
		t_iSLastZ = t_iZIdx;
	}

	// 
	// River mod @ 2008-10-16:如果是bridgeDecal,使用bridge的高度
	if( t_bBridgeDecal )
	{
		for( int t_i=0;t_i<m_sRenderData.m_iVerNum;t_i ++ )
			m_vec3DecalVer[t_i].m_vec3Pos.y = t_vec3DetailPos.y + 0.07f;
	}


	// 对每一个顶点计算decal的offset
	m_sRenderData.m_bUseStaticVB = false;
	m_sRenderData.m_sRawData.m_arrIdxData = &m_vecDecalIdx[0];
	m_sRenderData.m_sRawData.m_arrVerData = &m_vec3DecalVer[0];
	m_sRenderData.m_iVerSize = sizeof( os_billBoardVertex );
	m_sRenderData.m_iPriNum = m_iDecalIdxnum-2;
	m_sRenderData.m_iPriType = TRIANGLESTRIP;


	// 计算每一个顶点的uv坐标。
	cal_decalVerUv();

	// 处理Decal的顶点颜色
	if( !m_bAniDecal )
	{
		for( int t_i=0;t_i<m_sRenderData.m_iVerNum;t_i ++ )
			m_vec3DecalVer[t_i].m_dwColor = m_dwDecalColor;
	}

	unguard;
}


//! 创建非跟随地形的普通decal.
void osc_decal::move_fakeDecal( osVec3D& _pos,float _rot,float _offset/* = 0.0f*/ )
{
	guard;

	osVec3D   t_vec3DetailPos;

	t_vec3DetailPos = _pos;
	m_ptrTGMgr->get_detailPos( t_vec3DetailPos,true );

	// 旋转4个顶点的位移到我们需要的位置
	osMatrix   t_sRotMat;
	osVec3D    t_vec3Offset[4];
	osMatrixRotationY( &t_sRotMat,_rot );
	t_vec3Offset[0] = osVec3D( -m_fDecalSize,_offset,-m_fDecalSize );
	t_vec3Offset[1] = osVec3D( -m_fDecalSize,_offset,m_fDecalSize );;
	t_vec3Offset[2] = osVec3D( m_fDecalSize,_offset,m_fDecalSize );
	t_vec3Offset[3] = osVec3D( m_fDecalSize,_offset,-m_fDecalSize );
	for( int t_i=0;t_i<4;t_i ++ )
	{
		osVec3Transform( &t_vec3Offset[t_i],&t_vec3Offset[t_i],&t_sRotMat );
		m_vec3DecalVer[t_i].m_vec3Pos = t_vec3DetailPos + t_vec3Offset[t_i];
	}

	m_iDecalIdxnum = 6;
	m_vec3DecalVer[0].m_dwColor = m_dwDecalColor;
	m_vec3DecalVer[1].m_dwColor = m_dwDecalColor;
	m_vec3DecalVer[2].m_dwColor = m_dwDecalColor;
	m_vec3DecalVer[3].m_dwColor = m_dwDecalColor;

	m_vec3DecalVer[0].m_vec2Uv = osVec2D( 0.0f,0.0f );
	m_vec3DecalVer[1].m_vec2Uv = osVec2D( 1.0f,0.0f );
	m_vec3DecalVer[2].m_vec2Uv = osVec2D( 1.0f,1.0f);
	m_vec3DecalVer[3].m_vec2Uv = osVec2D( 0.0f,1.0f );

	m_vecDecalIdx[0] = 0;
	m_vecDecalIdx[1] = 1;
	m_vecDecalIdx[2] = 2;
	m_vecDecalIdx[3] = 0;
	m_vecDecalIdx[4] = 2;
	m_vecDecalIdx[5] = 3;


	// 对每一个顶点计算decal的offset
	m_sRenderData.m_bUseStaticVB = false;
	m_sRenderData.m_sRawData.m_arrIdxData = &m_vecDecalIdx[0];
	m_sRenderData.m_sRawData.m_arrVerData = &m_vec3DecalVer[0];
	m_sRenderData.m_iVerSize = sizeof( os_billBoardVertex );
	m_sRenderData.m_iPriNum = 2;
	m_sRenderData.m_iVerNum = 6;
	m_sRenderData.m_iPriType = TRIANGLELIST;


	unguard;
}


//! 初始化一个decal.
WORD osc_decal::init_decal( os_decalInit& _dinit,int _shaderId/* = -1*/ )
{
	guard;
	
	osassert( _dinit.m_fSize > 0.0f );
	osassert( _dinit.m_szDecalTex[0] );

	m_bInRender = true;

	if( float_equal( _dinit.m_fVanishTime,0.0f ) )
	{
		m_bAniDecal = false;
		m_dwDecalColor = _dinit.m_dwDecalColor;
	}
	else
	{
		m_bAniDecal = true;
		m_fEtime = 0.0f;
		m_fVanishTime = _dinit.m_fVanishTime;
		// 去掉Alpha值.
		m_dwDecalColor = _dinit.m_dwDecalColor & 0x00ffffff;
	}

	// 先生成decal的几何数据
	m_fDecalSize = _dinit.m_fSize;
	m_wXFrameNum = _dinit.m_wXFrameNum;
	m_wYFrameNum = _dinit.m_wYFrameNum;
	m_fFrameTime = _dinit.m_fFrameTime;

	m_iAlphaType = _dinit.m_iAlphaType;

	// 创建简单的or切分的decal
	osVec3D  t_vec3Pos = _dinit.m_vec3Pos;
	if( _dinit.m_bFakeDecal )
		move_fakeDecal( t_vec3Pos,_dinit.m_fRotAgl,_dinit.m_fYOffset );
	else
		move_decal( _dinit.m_vec3Pos,_dinit.m_fRotAgl );

	m_bFake = _dinit.m_bFakeDecal;


	// 
	// 创建设备相关的shader数据
	if( _shaderId < 0 )
	{
		m_sRenderData.m_iShaderId = g_shaderMgr->add_shader( 
			_dinit.m_szDecalTex,osc_engine::get_shaderFile( "decal" ).c_str() );
		osassert( m_sRenderData.m_iShaderId >= 0 );
	}
	else
		m_sRenderData.m_iShaderId = _shaderId;


	m_bHide = false;

	increase_val();

	return this->get_val();

	unguard;
}




///////////////////////////////////////////////////////////////////////////////////
//
//  DecalMgr相关的函数实现
//
///////////////////////////////////////////////////////////////////////////////////

osc_decalMgr::osc_decalMgr()
{

}

osc_decalMgr::~osc_decalMgr()
{

}



//! 把当前的decalMgr跟tgMgr相关联系起来.
void osc_decalMgr::OneTimeInit( osc_TGManager* _tgMgr,osc_middlePipe* _pipe )
{
	guard;
	osassert( _tgMgr );
	osassert( _pipe );
	
	osc_decal::m_ptrTGMgr = _tgMgr;
	osc_decal::m_ptrMPipe = _pipe;
	
	unguard;
}

//! 重设一个decal的位置
void osc_decalMgr::reset_decalPos( int _id,osVec3D& _pos )
{
	guard;

	WORD   t_wLow;
	
	t_wLow = (WORD)_id;

	if( m_arrDecalIns.validate_id( t_wLow ) )
	{
		if( m_arrDecalIns[t_wLow].validate_cval( _id ) )
			m_arrDecalIns[t_wLow].move_decal( _pos );
	}

	unguard;
}


//! 为sceneCursor创建一个特殊的Decal,为的是可以在场景内正确的显示光标
void osc_decalMgr::create_sceneCursor( os_decalInit& _dinit )
{
	guard;

	int  t_idxFree = -1;
	for( int t_i =0;t_i<MAX_SCENECURSORNUM;t_i ++ )
	{
		if( !m_sSceneCursor[t_i].is_inRender() )
		{
			t_idxFree = t_i;
			break;
		}
	}

	if( t_idxFree == -1 )
		return;
	else
		m_sSceneCursor[t_idxFree].init_decal( _dinit );

	unguard;
}

//! fake shadow的shaderID.
int osc_decalMgr::m_iFakeShadowShaId = -1;


//! River @ 2011-2-11:加入fakeShadow创建代码，走一个捷径流程，使人物出现的更快.
int osc_decalMgr::create_fakeShadow( os_decalInit& _dinit )
{
	guard;

	osc_decal*   t_ptrDecal;
	DWORD        t_dwIdx = -1;
	WORD         t_wCVal;

	// River:优化，全局只创建一次，可更快的创建fakeShadow.
	strcpy( _dinit.m_szDecalTex,g_szFakeShadowTexture );
	if( m_iFakeShadowShaId < 0 )
	{
		m_iFakeShadowShaId = g_shaderMgr->add_shader( 
			_dinit.m_szDecalTex,osc_engine::get_shaderFile( "decal" ).c_str() );
		osassert( m_iFakeShadowShaId >= 0 );
	}


	t_dwIdx = m_arrDecalIns.get_freeNodeAUse( &t_ptrDecal );
	osassert( t_ptrDecal );

	t_ptrDecal->reset_decal();
	t_wCVal = t_ptrDecal->init_decal( _dinit,m_iFakeShadowShaId );

	return osn_mathFunc::syn_dword( t_wCVal,(WORD)t_dwIdx );

	unguard;
}



/** \brief
*  创建一个decal
*
*/
int osc_decalMgr::create_decal( os_decalInit& _dinit )
{
	guard;

	osc_decal*   t_ptrDecal;
	DWORD        t_dwIdx = -1;
	WORD         t_wCVal;

	t_dwIdx = m_arrDecalIns.get_freeNodeAUse( &t_ptrDecal );
	//osassert( t_ptrDecal );
	// River mod @ 2011-9-26: 如果为空，直接返回，DECAL为非关键资源。
	if( !t_ptrDecal )
		return -1;

	t_ptrDecal->reset_decal();
	t_wCVal = t_ptrDecal->init_decal( _dinit );

	return osn_mathFunc::syn_dword( t_wCVal,(WORD)t_dwIdx );

	unguard;
}

void osc_decalMgr::hide_decal( int _id,bool _hide )
{
	guard;

	WORD    t_wLow;
	
	t_wLow = (WORD)_id;

	if( m_arrDecalIns.validate_id( t_wLow ) )
	{
		// 确认有相同的CreateVal.
		if( m_arrDecalIns[t_wLow].validate_cval( _id ) )
			m_arrDecalIns[t_wLow].set_hide( _hide );
	}

	unguard;
}

void osc_decalMgr::delete_decal( int _id )
{
	guard;

	WORD    t_wLow;
	
	t_wLow = (WORD)_id;

	if( m_arrDecalIns.validate_id( t_wLow ) )
	{
		// 确认有相同的CreateVal.
		if( m_arrDecalIns[t_wLow].validate_cval( _id ) )
			m_arrDecalIns.release_node( t_wLow );
	}

	return;

	unguard;
}


//! Release All the Decal.
void osc_decalMgr::release_allDecal( void )
{
	guard;

	CSpaceMgr<osc_decal>::NODE   t_ptrNode;

	for( t_ptrNode = this->m_arrDecalIns.begin_usedNode();
		t_ptrNode.p != NULL;t_ptrNode = m_arrDecalIns.next_validNode( &t_ptrNode ) )
	{
		m_arrDecalIns.release_node( t_ptrNode.idx );
	}

	unguard;
}




//! 渲染当前的decalMgr.
void osc_decalMgr::render_decalmgr( os_FightSceneRType _rtype )
{
	guard;

	CSpaceMgr<osc_decal>::NODE   t_ptrNode;
	float    t_fEtime = sg_timer::Instance()->get_lastelatime();

	// Decal不能写Z,因为地表上可能有非常低的物品，Decal不能挡住他们
	osc_decal::m_ptrMPipe->set_renderState( D3DRS_ZWRITEENABLE,FALSE );

	// 先渲染srcAlpha invsrcAlpha.
	osc_decal::m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	for( t_ptrNode = this->m_arrDecalIns.begin_usedNode();
		t_ptrNode.p != NULL;t_ptrNode = m_arrDecalIns.next_validNode( &t_ptrNode ) )
	{		
		if( t_ptrNode.p->m_iAlphaType != 0 )
			continue;

		// 如果动画到消失，则删除这个decalNode Ins.
		if( !t_ptrNode.p->frame_move( t_fEtime ) )
		{
			m_arrDecalIns.release_node( t_ptrNode.idx );
			continue;
		}

		// 如果此decal是隐藏的，不需要渲染
		if( t_ptrNode.p->m_bHide )
			continue;

		if( t_ptrNode.p->m_iDecalIdxnum < 3 )
			continue;

		m_sPolySet.add_polygon( &t_ptrNode.p->m_sRenderData );
	}

	// River @ 2005-8-26:设置世界矩阵为identity,否则某些情况下，光标不见.
	osc_decal::m_ptrMPipe->set_worldMatrix( osc_bbManager::m_smatMatrix );
	osc_decal::m_ptrMPipe->render_andClearPolygonSet( &m_sPolySet );

	osc_decal::m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE );
	for( t_ptrNode = this->m_arrDecalIns.begin_usedNode();
		t_ptrNode.p != NULL;t_ptrNode = m_arrDecalIns.next_validNode( &t_ptrNode ) )
	{		
		if( t_ptrNode.p->m_iAlphaType != 1 )
			continue;

		// 如果动画到消失，则删除这个decalNode Ins.
		if( !t_ptrNode.p->frame_move( t_fEtime ) )
		{
			m_arrDecalIns.release_node( t_ptrNode.idx );
			continue;
		}

		// 如果此decal是隐藏的，不需要渲染
		if( t_ptrNode.p->m_bHide )
			continue;

		if( t_ptrNode.p->m_iDecalIdxnum < 3 )
			continue;

		m_sPolySet.add_polygon( &t_ptrNode.p->m_sRenderData );
	}
	osc_decal::m_ptrMPipe->render_andClearPolygonSet( &m_sPolySet );

	return;

	unguard;
}

//! 渲染场景内的光标.
void osc_decalMgr::render_sceneCursor( void )
{
	guard;

	float    t_fEtime = sg_timer::Instance()->get_lastelatime();

	osc_decal::m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE );
	for( int t_i=0;t_i<MAX_SCENECURSORNUM;t_i ++ )
	{
		if( !m_sSceneCursor[t_i].is_inRender() )
			continue;

		if( m_sSceneCursor[t_i].frame_move( t_fEtime ) )
		{
			if( (m_sSceneCursor[t_i].m_sRenderData.m_iShaderId >= 0)&&
				(m_sSceneCursor[t_i].m_sRenderData.m_iVerNum >=3 )  )
			{
				m_sPolySet.add_polygon( &m_sSceneCursor[t_i].m_sRenderData );
				osc_decal::m_ptrMPipe->set_worldMatrix( osc_bbManager::m_smatMatrix );
				osc_decal::m_ptrMPipe->render_andClearPolygonSet( &m_sPolySet );
			}
		}
	}

	unguard;
}


