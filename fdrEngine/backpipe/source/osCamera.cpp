//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osCamera.cpp
 *
 *   Desc:     实现Camera的接口,同时加入osok引擎内部需要的Camera接口.
 *          
 *   His:      River Created @ 4/18 2003.
 *
 *   "不要轻易改变花了很长时间做出的决定,那样就等于是背叛了自己的信念".
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osCamera.h"
# include "../../interface/miskFunc.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"

//! 这个变量控制是否剪切远处的地形，如果设为false,则远处的地形一块块跳出。否则，整齐的剪切
OSENGINE_API BOOL                     g_bClipFarTerr = TRUE;

//! 引擎内是否自动把相机跟地表做碰撞检测,默认情况下做出检测。
OSENGINE_API BOOL                     g_bCamTerrCol = TRUE;


//！River added @ 2008-4-28:是否把相机限制在场景地图内
OSENGINE_API BOOL                     g_bClipCamInMap = FALSE;


//! 相机位置离相机焦点最近的矩离
# define MINDIS_TOFOCUS               0.2f

# define  MIN_CAMLIMITVAL     -0.1f


os_ViewFrustum::os_ViewFrustum()
{
	memset( this,0,sizeof( os_ViewFrustum ) );
}

//! 输出调试信息
void os_ViewFrustum::output_debugInfo( void )
{
}


/** \brief
*  查看一个点是否在一个view frustum内
*/
bool os_ViewFrustum::objInVF( osVec3D& _vec )
{
	int    t_i;
	float  t_fdis;
	for( t_i=0;t_i<5;t_i++ )
	{
		t_fdis = osn_mathFunc::distance_V2P( _vec,m_Frustum[t_i] );
		if( t_fdis < 0 )
			return false;
	}

	return true;

}
 

/** \brief
*  查看一个bounding sphere是否在一个view frustum内
*/
bool os_ViewFrustum::objInVF( os_bsphere& _bs )
{
	int      t_i;
	float    t_fdis;
	for( t_i=0;t_i<5;t_i++ )
	{
		t_fdis = osn_mathFunc::distance_V2P( _bs.veccen,m_Frustum[t_i] );
		t_fdis += _bs.radius;
		if( t_fdis < 0.0f )
			return false;
	}

	return true;
}


/** \brief
*  查看一个bounding sphere是否在一个view frustum内
*/
bool os_ViewFrustum::objInVF( osVec3D& _vec,float _rad )
{
	int      t_i;
	float    t_fdis;
	for( t_i=0;t_i<5;t_i++ )
	{
		t_fdis = osn_mathFunc::distance_V2P( _vec,m_Frustum[t_i] );
		t_fdis += _rad;
		if( t_fdis < 0 )
			return false;
	}

	return true;
}



/** \brief
*  查看一个bounding box是否在一个view frustum内
*/
bool os_ViewFrustum::objInVF( os_bbox& _bb )
{
	guard;

	const osVec3D*    t_vec3WorldPt;
	// 使用一种非最优和准备的方法：
	// os_bbox内的8个顶点处理到世界空间后，和VF的6个面进行比较，
	// 如果这8个顶点任一个面的后面，则不在视域内。否则在视域内，
	// 这种处理会出现某些不在视域内的box在视域内的情形，略过这些错误
	t_vec3WorldPt = _bb.get_worldVertexPtr(); 
	for( int t_i=0;t_i<5;t_i ++ )
	{
		int t_j;
		for( t_j =0;t_j<8;t_j++ )
		{
			if( osn_mathFunc::distance_V2P(t_vec3WorldPt[t_j],m_Frustum[t_i])>0 )
				break;
		}

		// bbox的8个顶点都在第t_i个面的后面
		if( t_j == 8 )
			return false;
	}

	return true;

	unguard;
}

//! 查看一个aabbox是否在一个vf内(可以被VF看到.)
bool os_ViewFrustum::objInVF( const os_aabbox& _box )
{
	osVec3D   t_vec3;

	// 如果AABBOX都不相交，直接返回
	if( !m_sAabbox.collision( _box ) )
		return false;

	for( int t_i=0;t_i<5;t_i ++ )
	{
		// 找到最可能在视域面内的顶点
		t_vec3.x = ((m_Frustum[t_i].a < 0.0f) ? _box.m_vecMin.x : _box.m_vecMax.x );
		t_vec3.y = ((m_Frustum[t_i].b < 0.0f) ? _box.m_vecMin.y : _box.m_vecMax.y );
		t_vec3.z = ((m_Frustum[t_i].c < 0.0f) ? _box.m_vecMin.z : _box.m_vecMax.z );

		// 只要有一个最可能在视域面内的顶点在视域面之外，则此aabbox使用此种方式检测，不在视域内。
		if( osn_mathFunc::distance_V2P( t_vec3,m_Frustum[t_i] ) < 0.0f )
			return false;
	}

	return true;
}

//! 精确检测vf是否与aabbox相交,ATTENTION TO FIX:此函数是否必要？
bool os_ViewFrustum::objInVFAccurate( const os_aabbox& _box )
{
	if( !objInVF( _box ) )
		return false;

	if( m_sAabbox.collision( _box ) )
		return true;
	else
		return false;

}



/** \brief
*  用于检测一个aabbox跟vf的关系
*
*  \return bool aabbox全部在vf内，返回true.相交返回false.
*                     全部在vf外情况由objInVF函数判断
*  
*  算法：对每一个vf面，找到最可能在这个面之后的顶点，检测，如果所有的这些
*        顶点都在vf面的前面，则整个aabbox在vf之内.
*/
bool os_ViewFrustum::objCompleteInVF( const os_aabbox& _box )
{
	osVec3D   t_vec3;

	for( int t_i=0;t_i<5;t_i ++ )
	{
		// 每一个顶点都是最不可能在视域面前面的顶点
		t_vec3.x = ((m_Frustum[t_i].a < 0.0f) ? _box.m_vecMax.x : _box.m_vecMin.x );
		t_vec3.y = ((m_Frustum[t_i].b < 0.0f) ? _box.m_vecMax.y : _box.m_vecMin.y );
		t_vec3.z = ((m_Frustum[t_i].c < 0.0f) ? _box.m_vecMax.z : _box.m_vecMin.z );

		if( osn_mathFunc::distance_V2P( t_vec3,m_Frustum[t_i] ) < 0.0f )
			return false;
	}

	return true;
}

//! 检测一条射线是否和左，右，上，下，远剪切面相交，是三角形的精确相交
BOOL os_ViewFrustum::ray_intersectLeftClipPlane( osVec3D& _start,osVec3D _dir,float* _dis/* = NULL*/  )
{
	guard;

	float   t_fDis,t_fU,t_fV;

	osPlane  t_plane;
	osPlaneFromPoints( &t_plane,&m_FrustumVer[0],&m_FrustumVer[2],&m_FrustumVer[4] );
	if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) != OSE_FRONT )
		return FALSE;


	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[0],m_FrustumVer[2],m_FrustumVer[4],t_fDis,t_fU,t_fV ) )
	{

		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}
	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[2],m_FrustumVer[6],m_FrustumVer[4],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}

	return FALSE;

	unguard;
}
BOOL os_ViewFrustum::ray_intersectRightClipPlane( osVec3D& _start,osVec3D _dir,float* _dis/* = NULL*/ )
{
	guard;

	float   t_fDis,t_fU,t_fV;

	osPlane  t_plane;
	osPlaneFromPoints( &t_plane,&m_FrustumVer[7],&m_FrustumVer[3],&m_FrustumVer[1] );
	if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) != OSE_FRONT )
		return FALSE;


	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[7],m_FrustumVer[3],m_FrustumVer[1],t_fDis,t_fU,t_fV ) )
	{
		if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) == OSE_FRONT )
		{
			if( _dis )  *_dis = t_fDis;
			return TRUE;
		}
	}
	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[7],m_FrustumVer[1],m_FrustumVer[5],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}

	return FALSE;

	unguard;
}

BOOL os_ViewFrustum::ray_intersectTopClipPlane( osVec3D& _start,osVec3D _dir,float* _dis/* = NULL*/ )
{
	guard;

	float   t_fDis,t_fU,t_fV;

	osPlane  t_plane;
	osPlaneFromPoints( &t_plane,&m_FrustumVer[2],&m_FrustumVer[3],&m_FrustumVer[7] );
	if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) != OSE_FRONT )
		return FALSE;


	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[2],m_FrustumVer[3],m_FrustumVer[7],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}
	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[2],m_FrustumVer[7],m_FrustumVer[6],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}

	return FALSE;

	unguard;
}

BOOL os_ViewFrustum::ray_intersectBottomClipPlane( osVec3D& _start,osVec3D _dir,float* _dis/* = NULL*/ )
{
	guard;

	float   t_fDis,t_fU,t_fV;

	osPlane  t_plane;
	osPlaneFromPoints( &t_plane,&m_FrustumVer[0],&m_FrustumVer[4],&m_FrustumVer[1] );
	if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) != OSE_FRONT )
		return FALSE;

	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[0],m_FrustumVer[4],m_FrustumVer[1],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}
	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[4],m_FrustumVer[5],m_FrustumVer[1],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}

	return FALSE;

	unguard;
}

BOOL os_ViewFrustum::ray_intersectFarClipPlane( osVec3D& _start,osVec3D _dir,float* _dis/* = NULL*/ )
{
	guard;

	float   t_fDis,t_fU,t_fV;

	osPlane  t_plane;
	osPlaneFromPoints( &t_plane,&m_FrustumVer[4],&m_FrustumVer[6],&m_FrustumVer[7] );
	if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) != OSE_FRONT )
		return FALSE;


	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[4],m_FrustumVer[6],m_FrustumVer[7],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}
	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[4],m_FrustumVer[7],m_FrustumVer[5],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}

	return FALSE;

	unguard;
}


//! 检测一条射线是否跟相机的近剪切面相交。是精确的两个近剪切面三角形，而非近剪切面的平面。
BOOL os_ViewFrustum::ray_intersectNearClipPlane( osVec3D& _start,osVec3D _dir,float* _dis/* = NULL*/ )
{
	guard;

	float   t_fU,t_fV;
	float   t_fDis;

	osPlane  t_plane;
	osPlaneFromPoints( &t_plane,&m_FrustumVer[0],&m_FrustumVer[1],&m_FrustumVer[2] );
	if( osn_mathFunc::classify_PointByPlane( t_plane,_start ) != OSE_FRONT )
		return FALSE;

	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[0],m_FrustumVer[1],m_FrustumVer[2],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}
	if( osn_mathFunc::intersect_tri( _start,_dir,
		m_FrustumVer[0],m_FrustumVer[2],m_FrustumVer[3],t_fDis,t_fU,t_fV ) )
	{
		if( _dis )  *_dis = t_fDis;
		return TRUE;
	}

	return FALSE;

	unguard;
}

//! 一个射线起点，一个长度，一个方向，但到新的顶点
void os_ViewFrustum::get_pt( osVec3D& _s,osVec3D& _dir,float _len,osVec3D& _res )
{
	_res = _s + _dir*_len;
}


//! 给出一个开始点和结束点，找到跟clip面的交点
BOOL os_ViewFrustum::get_clipPt( osVec3D& _s,osVec3D& _e,osVec3D& _res )
{
	osVec3D  t_dir = _e - _s;
	float    t_fDis;
	
	osVec3Normalize( &t_dir,&t_dir );

	if( ray_intersectLeftClipPlane( _s,t_dir,&t_fDis ) )
	{
		if( t_fDis > 0.0f )
		{
			get_pt( _s,t_dir,t_fDis,_res );
			return true;
		}
	}
	if( ray_intersectRightClipPlane( _s,t_dir,&t_fDis ) )
	{
		if( t_fDis > 0.0f )
		{
			get_pt( _s,t_dir,t_fDis,_res );
			return true;
		}
	}
	if( ray_intersectTopClipPlane( _s,t_dir,&t_fDis ) )
	{
		if( t_fDis > 0.0f )
		{
			get_pt( _s,t_dir,t_fDis,_res );
			return true;
		}
	}
	if( ray_intersectBottomClipPlane( _s,t_dir,&t_fDis ) )
	{
		if( t_fDis > 0.0f )
		{
			get_pt( _s,t_dir,t_fDis,_res );
			return true;
		}
	}
	if( ray_intersectFarClipPlane( _s,t_dir,&t_fDis ) )
	{
		if( t_fDis > 0.0f )
		{
			get_pt( _s,t_dir,t_fDis,_res );
			return true;
		}
	}
	if( ray_intersectNearClipPlane( _s,t_dir,&t_fDis ) )
	{
		if( t_fDis > 0.0f )
		{
			get_pt( _s,t_dir,t_fDis,_res );
			return true;
		}
	}

	return false;
}


//! 此函数把一个线段clip到视域内,可能修改两个顶点
bool os_ViewFrustum::clip_lineSeg( osVec3D& _s,osVec3D& _e )
{
	guard;

# if 0
	//! 构建近剪切面
	osPlane   t_sNearClip;

	osVec3D  t_vec3Look;

	g_ptrCamera->get_godLookVec( t_vec3Look );
	osVec3D  t_vec3Pt[3];

	osVec3Normalize( &t_vec3Look,&t_vec3Look );
	t_vec3Pt[0] = m_FrustumVer[0] + t_vec3Look*0.1f;
	t_vec3Pt[1] = m_FrustumVer[1] + t_vec3Look*0.1f;
	t_vec3Pt[2] = m_FrustumVer[2] + t_vec3Look*0.1f;

	osPlaneFromPoints( &t_sNearClip,&t_vec3Pt[0],&t_vec3Pt[1],&t_vec3Pt[2] );

	osVec3D     t_vec3Tmp;
	ose_geopos  t_sPos,t_ePos;

	//! 如果两个点都在近剪切面之前,则返回真
	t_sPos = osn_mathFunc::classify_PointByPlane( t_sNearClip,_s );
	t_ePos = osn_mathFunc::classify_PointByPlane( t_sNearClip,_e );
	if( (t_sPos == OSE_FRONT)&&(t_ePos == OSE_FRONT ) )
		return true;

	if( (t_sPos == OSE_BACK)&&
		(t_ePos == OSE_FRONT) )
	{
		osPlaneIntersectLine( &t_vec3Tmp,&t_sNearClip,&_e,&_s );
		_s = t_vec3Tmp;
		return true;
	}

	if( (t_sPos == OSE_FRONT)&&
		(t_ePos == OSE_BACK ) )
	{
		osPlaneIntersectLine( &t_vec3Tmp,&t_sNearClip,&_s,&_e );
		_e = t_vec3Tmp;
		return true;
	}
# else

	//! 两个顶点都在视域内的情况
	if( objInVF( _s )&&objInVF( _e ) )
		return true;

	osVec3D  t_resA,t_resB;

	//! 两个顶点都在视域外
	if( (!objInVF( _s ))&&(!objInVF( _e ) ) )
	{
		if( get_clipPt( _s,_e,t_resA ) )
		{
			if( get_clipPt( _e,_s,t_resB ) )
			{
				_s = t_resA;
				_e = t_resB;
				return true;
			}
		}
		return false;
	}

	//! 一个是视域外，一个内部,剪切
	if( objInVF( _s ) && (!objInVF( _e ) ) )
	{
		if( get_clipPt( _s,_e,t_resA ) )
		{
			_e = t_resA;
			return true;
		}
	}
	if( (!objInVF( _s ) )&& objInVF( _e ) )
	{
		if( get_clipPt( _e,_s,t_resA ) )
		{
			_s = t_resA;
			return true;
		}
	}


	return false;


# endif 

	return false;

	unguard;
}

/*
//! 用于水面以下的渲染，镜象当前的vf.
void os_ViewFrustum::reflect_vf( osPlane& _plane )
{
	guard;

	osMatrix    t_sReflMat;

	osMatrixReflect( &t_sReflMat,&t_sClipPlane );


	unguard;
}*/




//! 画出Vf的远剪切面，要比实际的远剪切面近一点
void os_ViewFrustum::draw_farPlane( osc_middlePipe* _pipe )
{
	guard;

	os_polygon       t_rPolygon;

	//
	// 构造Bounding Box的渲染数据。
	os_sceVerDiffuse     t_pt[6];
	WORD                 t_idx[12];

	t_idx[0] = 0;
	t_idx[1] = 1;
	t_idx[2] = 2;

	t_idx[3] = 1;
	t_idx[4] = 3;
	t_idx[5] = 2;

	t_idx[6] = 0;
	t_idx[7] = 2;
	t_idx[8] = 6;

	t_idx[9] = 0;
	t_idx[10] = 6;
	t_idx[11] = 4;

	t_pt[0].m_vecPos = m_FrustumVer[4];
	t_pt[1].m_vecPos = m_FrustumVer[6];
	t_pt[2].m_vecPos = m_FrustumVer[5];
	t_pt[3].m_vecPos = m_FrustumVer[7];
	t_pt[4].m_vecPos = m_FrustumVer[0];
	t_pt[5].m_vecPos = m_FrustumVer[2];


	osVec3D  t_vec3LookVec;
	g_ptrCamera->get_godLookVec( t_vec3LookVec );

	osVec3Normalize( &t_vec3LookVec,&t_vec3LookVec );
	t_vec3LookVec *= 0.2f;
	for( int t_i=0;t_i<6;t_i ++ )
	{
		t_pt[t_i].m_color = 0xffff0000;
		t_pt[t_i].m_vecPos -= t_vec3LookVec;
	}

	osVec3D  t_vec3Test,t_vec3CamPos;
	t_vec3Test = m_FrustumVer[4] + m_FrustumVer[5] + m_FrustumVer[6] + m_FrustumVer[7];
	t_vec3Test /= 4.0f;
	g_ptrCamera->get_curpos( &t_vec3CamPos );
	t_vec3Test -= t_vec3CamPos;
	float t_f = osVec3Length( &t_vec3Test );
	

	t_rPolygon.m_iPriNum = 4;
	t_rPolygon.m_bSelfReleaseVI = false;
	t_rPolygon.m_bUseStaticVB = false;
	t_rPolygon.m_iPriType = TRIANGLELIST;
	t_rPolygon.m_iShaderId = g_shaderMgr->
		add_shader( "line", osc_engine::get_shaderFile( "line" ).c_str() );
	if( t_rPolygon.m_iShaderId < 0 )
		t_rPolygon.m_iShaderId = g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );
	t_rPolygon.m_iVerNum = 6;
	t_rPolygon.m_iVerSize = sizeof( os_sceVerDiffuse );
	t_rPolygon.m_sRawData.m_arrIdxData = t_idx;
	t_rPolygon.m_sRawData.m_arrVerData = t_pt;

	_pipe->render_polygon( &t_rPolygon );

	unguard;
}




///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Start of class osc_camera implements.
//
//! 相机数据文件的文件头。
const char* CAMFILE_HEADER  =   "cam";
const DWORD CAMFILE_VERSION =   0x100;
const DWORD CAMFILE_VERSION101 =   0x101;

//! 上下范围: 0 至 -(OS_PI/2),   旋转范围:以osVec3D(-1,0,0)为基, 逆时针,0-2*OS_PI
osc_camera::osc_camera()
{
	m_fCurpitch = 0.0f;
	m_fCuryaw = 0.0f;
	m_bShouldUpdate = true;

	m_bInCAState = false;
	m_fCATime = 0.0f;
	m_fEleTime = 0.0f;

	m_bInRotPitchLerpState = false;
	m_bPlayingCamEffect = false;
	m_fPlayEffectTime = 0.f;
	m_offset = osVec3D(0,0,0);

	m_bLockPitch = false;
	m_pScene = NULL;

	// 上层程序会限制这个数据到达一个合理的程度
	m_fMaxDis = 60.f;
	m_fMinDis = 0.f;
	m_bCammerLimitY = true;
	m_NeedCollsionTerrain = true;

	m_vec3CamFocus = osVec3D(0,0,0);

}

//! 得到相机可看到的远剪切面的矩离，即场景可见的最远矩离
float osc_camera::get_farClipPlaneDis( void )
{
	guard;

	return m_fFarPlane;

	unguard;
}
float osc_camera::get_nearClipPlaneDis( void )
{
	guard;

	return m_fNearPlane;

	unguard;
}

/**********************************************************************/
//
//  初始化Camera的函数.
//
/**********************************************************************/
/** \brief 
*  从一个结构中初始化一个Camera.
*
*  \param _cdata  从这个结构中初始化我们的Camera.
*
*/
void osc_camera::init_camera( const os_cameraInit* _cdata )
{
	guard;

	m_pScene = (osc_TGManager*)get_sceneMgr();
	osassert( m_pScene );

	osVec3D     t_vec;
	osassert( _cdata );
	osassertex( _cdata->camFocus != _cdata->curPos, "相机初始化时位置与焦点不能相同!\n" );
	osassert( _cdata->fov > 0.0f );
	osassert( _cdata->faspect > 0.0f );
	osassert( _cdata->nearp > 0.0f );
	osassert( _cdata->farp > _cdata->nearp );


	// River add 2009-2-26: 切换场景后，不至于相机还存在动画状态
	m_bInRotPitchLerpState = false;
	m_bInCAState = false;
	m_sAutoResetMgr.end_autoResetState();

	// 初始化Camera的位置信息.
	m_curpos = _cdata->curPos;

	m_vec3CamFocus = _cdata->camFocus;
	
	osVec3D   t_vec3Temp;
	t_vec3Temp = m_vec3CamFocus - m_curpos;
	m_fCamDisFocus = osVec3Length( &t_vec3Temp );
	osassert( m_fCamDisFocus >= 0.0f );

	osVec3Normalize( &t_vec3Temp, &t_vec3Temp );
	m_curLookVec = t_vec3Temp;


	m_fCurpitch = 0.f;
	pitch( /*_cdata->initPitch*/-1.0f );


	// 设置Projection Matrix的信息.
	// 近剪切面最近也要0.35以上
	float t_fNear = _cdata->nearp;
	if( t_fNear < 0.35f )
		t_fNear = 0.35f;

	set_projmat( _cdata->nearp,_cdata->farp,
		_cdata->fov,_cdata->faspect );


	// 设置Viewport.
	this->set_viewport( _cdata->tlx,_cdata->tly,
		_cdata->wid,_cdata->hei,_cdata->min_z,_cdata->max_z );

	// Set updata flag to true.
	this->m_bShouldUpdate = true;

	//
	m_fTADis = float(m_fFarPlane*TA_SCALE);
	// 使物品的可见度近于地表透明的格子
	m_fHalfFar = m_fTADis/2.0f;
	m_fObjFullView = float(m_fHalfFar*2.0f-(1.0f-TA_SCALE)*m_fFarPlane);

	//syq 新数据初始化
	m_dwVersion = CAMFILE_VERSION101;
	m_vec3OldCamPos = m_vec3NewCamPos = m_curpos;
	m_vec3OldCamFocus = m_vec3NewCamFocus = m_vec3CamFocus;

	modify_pitch_yaw();
	
	
	//@{ tzz added for galaxy shaking camera
	m_fCurrTime			= 0.0f;
	m_fDamp				= 0.0f;
	m_bShaking			= false;
	//@}

	update_camera();

	

	return;

	unguard;
}

//! 2009-9-4:一次性设置位置和focus,用于解决一个很变态的屏幕晃动的bug.
void osc_camera::set_curPosFocus( osVec3D& _pos,osVec3D& _focus,bool _force/* = false*/ )
{
	guard;

	// River mod @ 2008-4-28:处理相机在地图之外的情形
	if( g_bClipCamInMap )
	{
		// 必须保证相机的位置在整个地图之内,否则不设置
		if( (_pos.x<0.0f)||(_pos.z<0.0f)||
			(_pos.x>g_fMapMaxX)||(_pos.z>g_fMapMaxZ) )
			return;
	}


	// 
	// River added @ 2009-5-19:更准备的相机碰撞检测回弹。
	if( !vec3_equal( m_vec3CamFocus,(osVec3D &)_focus ) )
		m_sAutoResetMgr.pause_autoReset( false );
	else
		return;

	m_vec3CamFocus = _focus;	
	//在这里上层设定的值有可能会超出其定义的最远距离，底层不进行此判断
	m_curpos = _pos;


	// 
	// River @ 2006-7-14:判断当前设置的焦点是否合法,只有在碰撞开启的情况下使用
	if( m_pScene && g_bRenderTerr && g_bCamTerrCol )
	{
		osassertex( _focus.x >= 0.0f,va( "错误的数据是:<%f>..\n",_focus.x ) );
		osassertex( _focus.z >= 0.0f,va( "错误的数据是:<%f>..\n",_focus.z ) );

		osVec3D   t_vec3Pos;

		//! 先检测焦点是否合法
		t_vec3Pos = _focus;
		m_pScene->get_detailPos( t_vec3Pos );
		if( m_vec3CamFocus .y <= (t_vec3Pos.y + 0.2f) )
			m_vec3CamFocus .y = (t_vec3Pos.y + 0.2f);


		//fixme:river加入位置高度与地表碰撞判断,相机会抖动	
		//! 再检测位置是否合法,强制使用一个在地表之上的位置
		t_vec3Pos = m_curpos;
		m_pScene->get_detailPos( t_vec3Pos,true );
		
		bool t_bColl = false;

		if( m_curpos.y < (t_vec3Pos.y-1.0f) )
			t_bColl = true;

		if( t_bColl )
		{
			// River mod @ 2008-11-5: 此时旋转相机，看是否可以到达一个没有碰撞的位置
			update_camera( false,false );
			
			int   t_iTryRotNum = 0;
			int   t_iCanRotNum = int(OS_PI*2/0.05f);
			while( t_bColl )
			{
				// 
				// 必须force rot，不然此时可能处于旋转插值状态，就不能旋转了
				rotate_horizon( m_vec3CamFocus,0.05f,true );
				t_vec3Pos = m_curpos;

				// 此处使用地表的高度就可以了，桥类的物品可以变的半透明
				m_pScene->get_detailPos( t_vec3Pos );
				if( m_curpos.y < (t_vec3Pos.y-1.0f) )
					t_bColl = true;
				else
					t_bColl = false;

				t_iTryRotNum ++;

				if( t_iTryRotNum > t_iCanRotNum )
					break;
			}

			// 
			// River Added @ 2008-11-17:加入更多的测试，防止死循环
			if( t_iTryRotNum > t_iCanRotNum )
			{
				while( t_bColl )
				{
					m_curpos.y = t_vec3Pos.y + 5.0f;
					t_vec3Pos = m_curpos;

					// 此处使用地表的高度就可以了，桥类的物品可以变的半透明
					m_pScene->get_detailPos( t_vec3Pos );
					if( m_curpos.y < (t_vec3Pos.y-1.0f) )
						t_bColl = true;
					else
						t_bColl = false;
				}
			}
		}
	}
	
	//! 修改pitch和yaw的值
	modify_pitch_yaw();

	m_bShouldUpdate = true;

	unguard;
}



/** \brief
*  设置相机当前的位置.
*/
void osc_camera::set_curpos( osVec3D* _vec,bool _force/* = false*/  )
{
	guard;

	if( !_force )
	{
		if( m_bInCAState  )
			return;
	}

	// River mod @ 2008-4-28:处理相机在地图之外的情形
	if( g_bClipCamInMap )
	{
		// 必须保证相机的位置在整个地图之内,否则不设置
		if( (_vec->x<0.0f)||(_vec->z<0.0f)||
			(_vec->x>g_fMapMaxX)||(_vec->z>g_fMapMaxZ) )
			return;
	}

	//在这里上层设定的值有可能会超出其定义的最远距离，底层不进行此判断
	m_curpos = *_vec;
	
	//! 修改pitch和yaw的值
	modify_pitch_yaw();

	m_bShouldUpdate = true;


	unguard;
}


//! 修正角度值,使角度值是正确的
void osc_camera::modify_pitch_yaw()
{
	osVec3D up(0,1,0);
	osVec3D fz(-1,0,0);
	osVec3D zdir;
	osVec3D dir= m_curpos-m_vec3CamFocus;

	osVec3Normalize( &dir,&dir );
	m_fCurpitch = -osn_mathFunc::get_vectorAngle( up, dir );

	zdir = osVec3D(dir.x,0,dir.z);
	osVec3Normalize( &zdir,&zdir );
	m_fCuryaw = osn_mathFunc::get_vectorAngle( fz, zdir );
	if(zdir.z>0)
		m_fCuryaw = OS_PI*2-m_fCuryaw;


}

/**************************************************************************************/
//
//  操纵Camera需要的接口.
//
/**************************************************************************************/
//void osc_camera::move_forward( float _dis )
//{
//	guard;// osc_camera::move_forward() );
//
//	move_forward_genmod( _dis );
//
//	unguard;
//}

/** \brief
*   使相机以God模式往前移动一段距离(距离可正可负).
*
*   执行步骤:
*
*   1: 如果需要更新Look vector,先构建新的Look vector.
*
*   2: 在Look vector上移动要移动的距离.
* 
*/
void osc_camera::move_forward_godmod( float _dis )
{
	guard;


	//
	// 如果需要更新Look Vector,计算新的look vector.
	//
	if( this->m_bShouldUpdate )
		update_camera();
	
	//
	//  在相机方向上移动要移动的距离.
	//
	m_curpos.x -= m_curLookVec.x*_dis;
	m_curpos.y -= m_curLookVec.y*_dis;
	m_curpos.z -= m_curLookVec.z*_dis;
	
	m_bShouldUpdate = true;
	
	unguard;
}

/** \brief
*   使相机以General模式往前移动一段距离.
*
*   执行步骤:
*
*   1: 先构建Camera Look vector在水平方向上的Vector.
*
*   2: 在Look vector的水平方向上移动要移动的距离.
*/
void osc_camera::move_forward_genmod( float _dis )
{
	guard;

	osMatrix tempmatrix;
	osVec4D  tempvec;
	osVec3D  look_vec,up_vec,right_vec;
	
	look_vec    =  this->m_lookvec; 
	up_vec      =  this->m_upvec;   
	right_vec   =  this->m_rightvec;

	//
	// 构建Camera Look vector在水平方向上的Vector.
	//
	osMatrixRotationAxis( &tempmatrix,&up_vec,-this->m_fCuryaw );	
	osVec3Transform( &tempvec,&look_vec,&tempmatrix );
	look_vec.x = tempvec.x;
	look_vec.y = tempvec.y;
	look_vec.z = tempvec.z;
	osVec3Transform( &tempvec,&right_vec,&tempmatrix );
	right_vec.x = tempvec.x;
	right_vec.y = tempvec.y;
	right_vec.z = tempvec.z;

	//
	//因为是正常模式，所以就算camera有pitch,移动也不能有pitch.
	//
	osMatrixRotationAxis( &tempmatrix,&right_vec,0  ); 
	osVec3Transform( &tempvec,&look_vec,&tempmatrix );
	look_vec.x = tempvec.x;
	look_vec.y = tempvec.y;
	look_vec.z = tempvec.z;
	osVec3Transform( &tempvec,&right_vec,&tempmatrix );
	right_vec.x = tempvec.x;
	right_vec.y = tempvec.y;
	right_vec.z = tempvec.z;


	osVec3Normalize(&look_vec,&look_vec);

	//
	//  在这个水平方向上移动相应的距离.
	//
	m_curpos.x -= look_vec.x*_dis;
	m_curpos.y -= look_vec.y*_dis;
	m_curpos.z -= look_vec.z*_dis;

	
	//m_bShouldUpdate = true;
	update_camera();

	unguard;
}




//!  Get the proj Matrix.
void osc_camera::get_projmat( osMatrix& _mat,bool _offset/* = true*/ )
{
	float  t_fFar = m_fFarPlane;


	// 
	// 加入一个值，使用场景中不会出被相机剪掉一半的物品。
	// River @ 2--6-3-22: 为了使上层的地形渲染可以整齐的剪切，
	//                    加入g_bClipFarTerr的全局变量
	if( _offset && (!g_bClipFarTerr) )
		t_fFar += CLIPDIS_ADD;


	// 
	//now only use the hfov....use d3dx helper func.fov_horiz1.5*SG_PI
	osMatrixPerspectiveFovLH( &_mat,this->m_fFov,this->m_fAspect,
		m_fNearPlane,t_fFar );

}


/***************************************************************************************/
//
//  使用相机中的数据进行一些常用量的计算.
//
/***************************************************************************************/
//@{
/** \brief
*   得到屏幕上一个二维坐标在相机三维空间中的向量.
*/
osVec3D osc_camera::get_upprojvec( const osVec2D* _svec,osMatrix& _wmat )
{
	guard;//

	D3DVIEWPORT9  t_viewPort;
	osVec3D       t_vec3Out,t_vecIn;
	osMatrix      t_matProj,t_matView;

	
	osassert( _svec );
	
	get_viewport( t_viewPort );
	get_projmat( t_matProj );
	get_viewmat( t_matView );

	t_vecIn.x = _svec->x;
	t_vecIn.y = _svec->y;
	t_vecIn.z = 1.0f;

	t_vecIn.x =  ( ( ( 2.0f * _svec->x ) / t_viewPort.Width  ) - 1 ) / t_matProj._11;
	t_vecIn.y = -( ( ( 2.0f * _svec->y ) / t_viewPort.Height ) - 1 ) / t_matProj._22;
	t_vecIn.z =  1.0f;

	// Get the inverse view matrix
	D3DXMATRIX m;
	D3DXMatrixInverse( &m, NULL, &t_matView );

	// Transform the screen space pick ray into 3D space
	t_vec3Out.x  = t_vecIn.x*m._11 + t_vecIn.y*m._21 + t_vecIn.z*m._31;
	t_vec3Out.y  = t_vecIn.x*m._12 + t_vecIn.y*m._22 + t_vecIn.z*m._32;
	t_vec3Out.z  = t_vecIn.x*m._13 + t_vecIn.y*m._23 + t_vecIn.z*m._33;

	D3DXVec3Normalize(&t_vec3Out,&t_vec3Out);

	return t_vec3Out;

	unguard;
}

osVec3D osc_camera::get_upprojvec( int _x,int _y,osVec3D& _dir )
{
	guard;

	D3DVIEWPORT9  t_viewPort;
	osVec3D       t_vec3Out,t_vecIn;
	osMatrix      t_matProj,t_matView;

	
	osassert( _x>=0 );
	osassert( _y>=0 );
	
	get_viewport( t_viewPort );
	get_projmat( t_matProj );
	get_viewmat( t_matView );

	t_vecIn.x = float(_x);
	t_vecIn.y = float(_y);
	t_vecIn.z = 1.0f;

	t_vecIn.x =  ( ( ( 2.0f * _x ) / t_viewPort.Width  ) - 1 ) / t_matProj._11;
	t_vecIn.y = -( ( ( 2.0f * _y ) / t_viewPort.Height ) - 1 ) / t_matProj._22;
	t_vecIn.z =  1.0f;

	// Get the inverse view matrix
	osMatrix    m;
	D3DXMatrixInverse( &m, NULL, &t_matView );

	// Transform the screen space pick ray into 3D space
	t_vec3Out.x  = t_vecIn.x*m._11 + t_vecIn.y*m._21 + t_vecIn.z*m._31;
	t_vec3Out.y  = t_vecIn.x*m._12 + t_vecIn.y*m._22 + t_vecIn.z*m._32;
	t_vec3Out.z  = t_vecIn.x*m._13 + t_vecIn.y*m._23 + t_vecIn.z*m._33;

	osVec3Normalize( &_dir,&t_vec3Out );

	return t_vec3Out;

	unguard;
}
//@} 


//=====================================================================================================
//
//  The following public func is the func which will used in inner 
//  engine part.
//  接下来的这些公有函数只可以用在引擎内部,不提供上层使用这些函数的接口.
//
//=====================================================================================================


/** \brief 
*  使用这个函数来得到ViewPort
* 
*  Set_viewport是初始化以后必须要调用的的函数，使用这个函数来得到ViewPort.
*/
void osc_camera::get_viewport( D3DVIEWPORT9& _v )
{
	
	_v.X = this->m_iTopleftx;
	_v.Y = this->m_iToplefty;
	_v.Width = this->m_iWidth;
	_v.Height = this->m_iHeight;
	_v.MinZ = this->m_fMinz;
	_v.MaxZ = this->m_fMaxz;
	
	return;
}



/** \brief
*  给出一个世界空间的顶点,给出当前世界空间顶点对应的屏幕空间的顶点.
*
*  \param _wpos 传入的世界空间的顶点坐标,此坐标为绝对坐标.
*  \param _spos 返回了世界空间顶点对应的屏幕坐标.
*  
*/
bool osc_camera::get_scrCoodFromVec3( const  osVec3D& _wpos,osVec3D& _spos )
{
	guard;

	D3DVIEWPORT9   t_v;
	osMatrix       t_projMat;
	osMatrix       t_viewMat;
	osMatrix       t_matWorld;
	osVec3D        t_vec3;
	
	osVec3D        t_vec3PtDir;
	const osVec3D  t_dir  = _wpos-m_curpos;

	// River mod @ 2009-2-5: 判断传入的顶点是否在相机近剪切面之后
	osVec3Normalize( &t_vec3PtDir,&(t_dir) );
	float t_fDot = osVec3Dot( &t_vec3PtDir,&m_curLookVec );
	if( t_fDot <= 0.0f )
		return false;

	// tzz ：如果是在相机焦点下面的，就不让其显示
	if(m_vec3CamFocus.y - _wpos.y > 3.0f){
		return false;
	}

	osMatrixIdentity( &t_matWorld );
	get_viewport( t_v );
	get_projmat( t_projMat );
	get_viewmat( t_viewMat );


	osVec3Transform( &t_vec3,&_wpos,&t_viewMat );
	osVec3Transform( &t_vec3,&t_vec3,&t_projMat );


	//
	osVec3Project( &_spos,&_wpos,&t_v,&t_projMat,&t_viewMat,&t_matWorld );

	return true;

	unguard;
}




void osc_camera::get_nearplane( osPlane& _plane )
{
	guard;

	osVec3D  t_vec,t_lookvec;
	
	t_lookvec.x = m_viewmat._13;
	t_lookvec.y = m_viewmat._23;
	t_lookvec.z = m_viewmat._33;
	
	_plane.a = t_lookvec.x;
	_plane.b = t_lookvec.y;
	_plane.c = t_lookvec.z;
	
	t_vec = m_curpos + this->m_fNearPlane*t_lookvec;
	_plane.d = -osVec3Dot( &t_lookvec,&t_vec );

	unguard;
}
void osc_camera::get_farplane( osPlane& _plane )
{
	guard;

	osVec3D  t_vec,t_lookvec;
	
	t_lookvec.x = m_viewmat._13;
	t_lookvec.y = m_viewmat._23;
	t_lookvec.z = m_viewmat._33;
	
	_plane.a = t_lookvec.x;
	_plane.b = t_lookvec.y;
	_plane.c = t_lookvec.z;
	
	t_vec = m_curpos + this->m_fFarPlane*t_lookvec;
	_plane.d = osVec3Dot( &t_lookvec,&t_vec );

	unguard;
}


//!  得到水平方向上的look vec.
void osc_camera::get_horlookvec( osVec3D& _look )
{
	
	osMatrix tempmatrix;
	osVec4D  tempvec;
	osVec3D  look_vec,up_vec,right_vec;
	
	look_vec =  this->m_lookvec; //sgVec3D( 0,0,1 );
	up_vec =    this->m_upvec;   //sgVec3D( 0,1,0 );
	right_vec = this->m_rightvec;//sgVec3D( 1,0,0 );
	
	osMatrixRotationAxis( &tempmatrix,&up_vec,-this->m_fCuryaw );	
	osVec3Transform( &tempvec,&look_vec,&tempmatrix );
	look_vec.x = tempvec.x;
	look_vec.y = tempvec.y;
	look_vec.z = tempvec.z;
	osVec3Transform( &tempvec,&right_vec,&tempmatrix );
	right_vec.x = tempvec.x;
	right_vec.y = tempvec.y;
	right_vec.z = tempvec.z;
	
	// 因为是正常模式，所以就算camera有pitch,移动也不能有pitch.
    osMatrixRotationAxis( &tempmatrix,&right_vec,0  ); 
	osVec3Transform( &tempvec,&look_vec,&tempmatrix );
	look_vec.x = tempvec.x;
	look_vec.y = tempvec.y;
	look_vec.z = tempvec.z;
	osVec3Transform( &tempvec,&right_vec,&tempmatrix );
	right_vec.x = tempvec.x;
	right_vec.y = tempvec.y;
	right_vec.z = tempvec.z;
	
	osVec3Normalize(&look_vec,&look_vec);
	
	_look = look_vec;
	
}


/**
*  Update the camera.更新Camera中的各个Matrix.
*/
void osc_camera::update_camera( bool _buildFrus/* = true*/,
		bool _colwithTerr/* = true*/,bool _reflect/* = false*/ )
{
	guard;
	
	osVec3D up( 0.0f,1.0f,0.0f );
	osVec3D right( 1.0f,0.0f,0.0f );
	osVec3D look =  m_vec3CamFocus - m_curpos;
	osVec3Normalize( &look,&look );

	// River @ 2011-10-18：相机相关，至少保证不出错。
	//osassert( !vec3_equal( m_vec3CamFocus,m_curpos ) );
	if( vec3_equal( m_vec3CamFocus,m_curpos ) )
		return;

	// River mod @ 2008-12-8:为做出水面倒影,得到相机 
	if( _reflect )
	{
		up = osVec3D( 0.0f,-1.0f,0.0f );
		right = osVec3D( -1.0f,0.0f,0.0f );
	}


	// TEST CODE: river @ 2006.5.8,在某此情况下相机出错
	if( g_bRenderTerr )
	{
		float t_f = 0.0f;

		// river mod 2008-4-28:
		if( g_bClipCamInMap )
		{
			// River mod @ 2007-4-18:使相机不再出错
			if( m_curpos.x < 0.0f )
				m_curpos.x = 0.1f;
			if( m_curpos.z < 0.0f )
				m_curpos.z = 0.1f;
		}

		// River mod @ 2007-12-5:为了不使相机的运动限入死循环
		if( m_curpos.x > g_fMapMaxX )
		{
			t_f = m_curpos.x;
			m_curpos.x = g_fMapMaxX - 0.1f;
			m_vec3CamFocus.x -= ( t_f-m_curpos.x);
		}
		if( m_curpos.z > g_fMapMaxZ )
		{
			t_f = m_curpos.z;
			m_curpos.z = g_fMapMaxZ - 0.1f;
			m_vec3CamFocus.z -= (t_f-m_curpos.z);
		}
	}


	// River added @ 2006-4-14:
	// 如果是完全的顶视角，则up Vector必须使用( 0,0,1 )
	if( float_equal( fabs(look.y),up.y )  )
	{
		up.y = 0.0f;up.z = 1.0f;
	}

	osVec3Cross( &right,&up, &look);
	osVec3Normalize( &right,&right );
	osVec3Cross( &up,&look, &right );
	osVec3Normalize( &up,&up );

	osMatrix out;
	osMatrixLookAtLH( &out, &m_curpos, &m_vec3CamFocus, &up  );
	m_viewmat = out;
	
	

	m_curLookVec = look;
	m_curRightVec = right;

	osVec3D len = m_curpos - m_vec3CamFocus;
	m_fCamDisFocus = abs(osVec3Length( &len ));

	// build the view frustum.
	if( _buildFrus )
		build_camFrustum();

	// River mod @ 2007-10-13:
	if( (m_fCamDisFocus > (m_fMaxDis+0.001))&&g_bCamTerrCol )
		pull_camPosToMaxDis();


	m_bShouldUpdate = false;
	
	//@{ \brief : tzz add for shaking camera
	ShakeCamera();
	//@}


	unguard;
}

/** \brief
*  构建当前Camera的frustum.
*/
void osc_camera::build_camFrustum( bool _guaranteeFrums/* = false*/ )
{
	guard;
    D3DXMATRIXA16       t_mat;
	osVec3D             t_vec;
	osMatrix            t_projMat;
	int                 t_i;
	os_ViewFrustum*      t_resFrus;

	get_projmat( t_projMat,false );
	osMatrixMultiply( &t_mat,&m_viewmat,&t_projMat );

	//! River @ 2009-4-14:更稳定的出错处理。
	if( NULL == osMatrixInverse( &t_mat,NULL,&t_mat ) )
		return;

	t_resFrus = &m_Frustum;

	// xyz,Xyz,xYz,XYz,xyZ,XyZ,xYZ,XYZ
	t_resFrus->m_FrustumVer[0] = osVec3D( -1.0f, -1.0f,  0.0f); // xyz
	t_resFrus->m_FrustumVer[1] = osVec3D(  1.0f, -1.0f,  0.0f); // Xyz
	t_resFrus->m_FrustumVer[2] = osVec3D( -1.0f,  1.0f,  0.0f); // xYz
	t_resFrus->m_FrustumVer[3] = osVec3D(  1.0f,  1.0f,  0.0f); // XYz
	t_resFrus->m_FrustumVer[4] = osVec3D( -1.0f, -1.0f,  1.0f); // xyZ
	t_resFrus->m_FrustumVer[5] = osVec3D(  1.0f, -1.0f,  1.0f); // XyZ
	t_resFrus->m_FrustumVer[6] = osVec3D( -1.0f,  1.0f,  1.0f); // xYZ
	t_resFrus->m_FrustumVer[7] = osVec3D(  1.0f,  1.0f,  1.0f); // XYZ

	for( t_i=0;t_i<8;t_i++ )
		osVec3TransformCoord( &t_resFrus->m_FrustumVer[t_i],&t_resFrus->m_FrustumVer[t_i],&t_mat );
	
    osPlaneFromPoints( &t_resFrus->m_Frustum[0], &t_resFrus->m_FrustumVer[6], 
        &t_resFrus->m_FrustumVer[7], &t_resFrus->m_FrustumVer[5] ); // Far
    osPlaneFromPoints( &t_resFrus->m_Frustum[1], &t_resFrus->m_FrustumVer[2], 
        &t_resFrus->m_FrustumVer[6], &t_resFrus->m_FrustumVer[4] ); // Left
    osPlaneFromPoints( &t_resFrus->m_Frustum[2], &t_resFrus->m_FrustumVer[7], 
        &t_resFrus->m_FrustumVer[3], &t_resFrus->m_FrustumVer[5] ); // Right
    osPlaneFromPoints( &t_resFrus->m_Frustum[3], &t_resFrus->m_FrustumVer[2], 
        &t_resFrus->m_FrustumVer[3], &t_resFrus->m_FrustumVer[6] ); // Top
    osPlaneFromPoints( &t_resFrus->m_Frustum[4], &t_resFrus->m_FrustumVer[1], 
        &t_resFrus->m_FrustumVer[0], &t_resFrus->m_FrustumVer[4] ); // Bottom

	// 重构vf的aabbox
	osn_mathFunc::cal_aabboxFromVArray( 
		t_resFrus->m_FrustumVer,8,t_resFrus->m_sAabbox );	

	return;

	unguard;
}

/** \brief
*  得到相机当前的指向的方向,不管相机处于何种模型,加入了方向的
*  分量.
*/
void osc_camera::get_godLookVec( osVec3D& _vec )
{
	guard;

	if( m_bShouldUpdate )
		update_camera();
	osVec3Normalize( &m_curLookVec,&m_curLookVec );
	_vec = m_curLookVec;
	return;

	unguard;
}

void osc_camera::move_close( float _length )
{
	m_fCamDisFocus += _length;

	// River mod @ 2009-4-14:为了更稳定的出错处理
	if( m_fCamDisFocus < 0.1f )
		m_fCamDisFocus = 0.1f;

	update_lockPitch();

	cam_rot( m_fCurpitch, m_fCuryaw );
	
	m_bShouldUpdate = true;


}

/** \brief
 *  获得相机的位置到焦点的距离
 */
float osc_camera::get_disFromCamposToFocus()
{
	osVec3D v = m_curpos - m_vec3CamFocus;
	return osVec3Length( &v );
}

/** \brief
*  使相机绕一个顶点进行水平旋转。
*
*  算法描述：
*  1: 把当前的相机的位置顶点转换到原点坐标。
*  2: 绕y轴旋转当前相机平移后的位置顶点。
*  3: 平移当前旋转后的顶点到相机世界空间。
*  4: 构建相机需要的数据。
*/
ose_camOperateRS osc_camera::rotate_horizon( const osVec3D& _ver,
						float _radians,bool _force/* = false*/  )
{
	guard;

	//! 
	m_sAutoResetMgr.pause_autoReset( false );

	if( !_force )
	{
		if( m_bInCAState || m_bInRotPitchLerpState || m_bPlayingCamEffect )
			return OSE_CAMIN_ANIMATION;
	}
	
	yaw( -_radians );
	cam_rot( m_fCurpitch,  m_fCuryaw );

	return OSE_CAMOPE_SUCCESS;

	unguard;
}

/** \brief
*  使相机绕一个顶点进行上下旋转。
*/
ose_camOperateRS osc_camera::rotate_vertical( const osVec3D& _ver,
						float _radians,bool _force/* = false*/  )
{
	guard;

	if( !_force )
	{
		if( m_bInCAState || m_bInRotPitchLerpState || m_bPlayingCamEffect )
			return OSE_CAMIN_ANIMATION;
	}


	// 如果相机抬的太直,返回.
	if( (m_fCurpitch + _radians)>=(MIN_CAMLIMITVAL)  )		
		return OSE_CAMPITCH_TOO_HIGH;

	// 不能让相机随意的在下面穿过
	if( fabs( m_fCurpitch + _radians ) >= OS_PI )
		return OSE_CAMPITCH_TOO_LOW;


	// 非lockPitch状态或是force状态
	if( (!m_bLockPitch) || _force )
	{
		pitch( _radians );

		cam_rot( m_fCurpitch, m_fCuryaw );
	}


	return OSE_CAMOPE_SUCCESS;

	unguard;
}



// 设置相机的位置与焦点的最大距离
void osc_camera::set_maxDistance( float _maxDis )
{
	m_fMaxDis = _maxDis;

	m_bShouldUpdate = true;
}

// 设置相机的位置与焦点的最小距离
void osc_camera::set_minDistance( float _minDis )
{
	m_fMinDis = _minDis;

	m_bShouldUpdate = true;
}

// 修改相机位置与焦点的距离在有效距离区间内
void osc_camera::modify_disdance()
{
	if( m_fCamDisFocus > m_fMaxDis )
	{
		osVec3D dir= m_curpos - m_vec3CamFocus;
		osVec3Normalize( &dir, &dir );
		dir *= m_fMaxDis;
		m_curpos = m_vec3CamFocus + dir;
	
	}
}


void osc_camera::cam_rot( float _pitch, float _yaw )
{
	float x = m_fCamDisFocus*sin(_pitch)*cos(_yaw);
	float y = m_fCamDisFocus*sin(_pitch)*sin(_yaw);
	float z = m_fCamDisFocus*cos(_pitch);

	m_curpos = m_vec3CamFocus + osVec3D(x,z,y);

	m_bShouldUpdate = true;

}

//! ATTENTION　TO　FIX：　
//! 如果可见一个格子，则可见这个格子上的物品，物品就可能会产生悬空现象。　
//! 定义处理物品是否显示的Factor,此值越大，显示的物品越多,此值小一些的时候,远处的一些小物品就不会出现.
# define  INVIEW_FACTOR   0.6f

//
//!  传入的物品的bounding Sphere在相机的视野范围内。
//!  是否被相机看见，传入一个半径和位置。
BYTE osc_camera::is_viewByCamera( osVec3D& _pos,float _radius )
{
	guard;

	float     t_fDistance;
	osVec3D   t_vec3Temp;

	t_vec3Temp = _pos - m_vec3CamFocus;
	t_fDistance = osVec3Length( &t_vec3Temp );
	t_fDistance += m_fCamDisFocus;

	if( t_fDistance < m_fHalfFar )
	{
		return MASK_HALF_VIEW;
	}

	// 确认可见物品不会出现在远方透明的格子上
	if( (t_fDistance-_radius)>m_fObjFullView )
		return MASK_OUTHALF_NOTVIEW;

	// 如果物品半径的平方乘以factor大于这个矩离，则可见，否则不可见。
	t_fDistance -= m_fHalfFar;
	if( ((_radius*_radius)*INVIEW_FACTOR)>t_fDistance )
	{
		return MASK_OUTHALF_VIEW;
	}


	return MASK_OUTHALF_NOTVIEW;

	unguard;
}

//! 得到现有的相机视角宽度.
float osc_camera::get_cameraFov( void )
{
	return this->m_fFov;
}






//! 对两种相机动画进行插值。
void osc_camera::lerp_cameraFile( void )
{
	guard;

	float       t_fLerp;
	// 
	// 得到LerpVector.
	if( m_fEleTime > m_fCATime )
	{
		t_fLerp = 1.0f;
		m_bInCAState = false ;
	}
	else
		t_fLerp = (m_fEleTime / m_fCATime);

	if( m_dwVersion == CAMFILE_VERSION )
	{
		//
		// 对相机使用的每一个参数进行插值.
		osVec3Lerp( &m_curpos,&m_vec3OldPos,&m_vec3NewPos,t_fLerp );
		osVec3Lerp( &m_curLookVec,&m_vec3OldLook,&m_vec3NewLook,t_fLerp );
		osVec3Lerp( &m_curRightVec,&m_vec3OldRight,&m_vec3NewRight,t_fLerp );

		m_fCurpitch = m_fOldPitch + (m_fNewPitch-m_fOldPitch)*t_fLerp;


		//计算焦点
		osVec3D focus = m_curLookVec * 4;
		m_vec3CamFocus = m_curpos + focus;


	}
	else if( m_dwVersion == CAMFILE_VERSION101 )
	{
		osVec3Lerp( &m_curpos,&m_vec3OldCamPos,&m_vec3NewCamPos,t_fLerp );
		osVec3Lerp( &m_vec3CamFocus,&m_vec3OldCamFocus,&m_vec3NewCamFocus,t_fLerp );


	}
	
	update_camera();

	unguard;
}

//! 必须多帧都可以自动归位时，才进行自动归位处理，否则很容易引起相机的抖动.
# define RELAXAUTORESET_TIMES  8

//! 对自动回位后的相机位置进行处理,如果不对,则重设为旧的相机位置.
void osc_camera::process_camAutoReset( void )
{
	guard;

	if( (!m_sAutoResetMgr.is_autoResetState()) || (!g_bCamTerrCol) )
		return ;


	float  t_fMove;

	m_sAutoResetMgr.process_camAutoResetPos( this,t_fMove );

	osVec3D   t_vec3Dir;

	// 自动回位。
	move_close( t_fMove );
	update_camera( true,false );

	osVec3D    t_vec3Intersect,t_vec3RayDir;
	os_bsphere   t_sBs;

	get_camNearClipBSphere( t_sBs );

	bool       t_bRayInter;
	bool       t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );

	t_vec3RayDir = t_sBs.veccen - m_vec3CamFocus;
	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );
	t_bRayInter   = m_pScene->get_lineSegInterTerrPos( m_vec3CamFocus,m_curpos,t_vec3Intersect );
	if( t_bRayInter )
	{
		if( osVec3Length( &(t_vec3Intersect - m_vec3CamFocus ) ) >=
			osVec3Length( &(t_sBs.veccen    - m_vec3CamFocus ) )  )
			t_bRayInter = false;
	}

	// 如果碰撞,往相机焦点的方向上运动相机
	if( (t_bSphereColl || t_bRayInter) )
	{	
		// 使用上一帧正确的相机位置,防止相机的抖动发生.
		m_curpos = m_vec3LastFrameCamPos;

		update_camera( true,false );


		m_sAutoResetMgr.set_autoResetPass( false );
	}
	else
	{
		m_sAutoResetMgr.set_autoResetPass( true );

		if( !m_sAutoResetMgr.pass_autoResetTime() )
		{
			// 使用上一帧正确的相机位置,防止相机的抖动发生.
			m_curpos = m_vec3LastFrameCamPos;
			update_camera( true,false );
		}
	}

	return;

	unguard;
}


//!  相机的frameMove函数，用于处理相机的动画。
void osc_camera::frame_moveCamera( float _etime )
{
	guard;
	
	// River added @ 2008-6-17：非碰撞状态下，不做处理
	if( (!g_bCamTerrCol)&&(!m_bInCAState) )
		return;
	
	// 如果picth lock,检测.
	if( m_bLockPitch )
		update_camera_toLockPitch( _etime );


	m_bRotInCurFrame = false;

	if( m_bPlayingCamEffect )
	{
		update_camEffect();
		return;
	}

	// 
	// River @ 2006-9-9:如果相机位置在focus之下，而刚好在Y轴上，不做碰撞处理
	//                  正常的相机操作，不会出这样的数据。
	osVec3D   t_vec3Nor;
	osVec3Normalize( &t_vec3Nor,&(m_vec3CamFocus-m_curpos) );
	if( fabs(t_vec3Nor.y - 1.0f)<0.0001f )
		return;


	if( (!m_bInCAState) && (!m_bInRotPitchLerpState) )
	{ 

		update_camera();

		// 如果相机的焦点和位置所在的地图没有调入，则不能做相机的碰撞检测
		int   t_iXPos,t_iZPos,t_iXFocus,t_iZFocus;
		t_iXPos = int( m_curpos.x / TGMAP_WIDTH );
		t_iZPos = int( m_curpos.z / TGMAP_WIDTH );
		t_iXFocus = int( m_vec3CamFocus.x / TGMAP_WIDTH);
		t_iZFocus = int( m_vec3CamFocus.z / TGMAP_WIDTH);
		if( (!m_pScene->is_tgLoaded( t_iXPos,t_iZPos )) ||
			(!m_pScene->is_tgLoaded( t_iXFocus,t_iZFocus) ) )
			return;


		//! 做相机智能的碰撞检测
		modify_CollsionTerrain();

		m_vec3LastFrameCamPos = m_curpos;

		// 如果当前帧相机没有跟地形碰撞,则处理自动回位功能
		process_camAutoReset();

		return;
	}

	m_fEleTime += _etime;

	if( m_bInCAState )
	{
		lerp_cameraFile();
		return; 
	}



	// 对相机进行旋转和pitch插值。
	float    t_fUsedTime,t_fRotRad,t_fPitchRad;
	if( m_fEleTime >= m_fCATime )
	{
		m_bInRotPitchLerpState = false ;
		t_fUsedTime = m_fCATime - (m_fEleTime-_etime);
	}
	else
		t_fUsedTime = _etime;

	t_fRotRad = t_fUsedTime * m_fRotPerSec;
	t_fPitchRad = t_fUsedTime * m_fPitchPerSec;

	if( m_bShouldUpdate )
		update_camera();

	rotate_horizon( m_vec3CamFocus,t_fRotRad,true );	
	rotate_vertical( m_vec3CamFocus,t_fPitchRad,true );


	update_camera();



	return;

	unguard;
}

//! 得到此相机的位置和近剪切面形成的bsphere.
bool osc_camera::get_camNearClipBSphere( os_bsphere& _bs )
{
	guard;	

	osVec3D    t_vec3Tmp;
	os_ViewFrustum*  t_ptrFrus = get_frustum();

	// validate float value: 
	osassert( !_isnan( t_ptrFrus->m_FrustumVer[0].x  ) );
	osassert( !_isnan( t_ptrFrus->m_FrustumVer[0].y  ) );
	osassert( !_isnan( t_ptrFrus->m_FrustumVer[0].z  ) );

	osassert( !_isnan( t_ptrFrus->m_FrustumVer[3].x  ) );
	osassert( !_isnan( t_ptrFrus->m_FrustumVer[3].y  ) );
	osassert( !_isnan( t_ptrFrus->m_FrustumVer[3].z  ) );


	// 由xyz 和 XYz两个点来计算.
	t_vec3Tmp = t_ptrFrus->m_FrustumVer[0] + t_ptrFrus->m_FrustumVer[3];
	_bs.veccen = t_vec3Tmp/2.0f;
	_bs.radius = osVec3Length( &(t_ptrFrus->m_FrustumVer[0]-_bs.veccen) );
	_bs.extendedBs( m_curpos );

	if( _bs.radius < 100.0f )
		return true;
	else
		return false;


	unguard;
}



//! TEST CODE:用于确认相机不跟地表有碰撞发生.
void osc_camera::assert_camera( void )
{
	guard;

	os_bsphere   t_sBs;

	get_camNearClipBSphere( t_sBs );

	bool       t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );

	osassert( !t_bSphereColl );

	unguard;
}

//!  设置和得到相机的焦点.
void osc_camera::set_camFocus( const osVec3D& _focus )
{
	// 
	// River added @ 2009-5-19:更准备的相机碰撞检测回弹。
	if( !vec3_equal( m_vec3CamFocus,(osVec3D &)_focus ) )
		m_sAutoResetMgr.pause_autoReset( false );
	else
		return;

	m_vec3CamFocus = _focus;	

	// 
	// River @ 2006-7-14:判断当前设置的焦点是否合法,只有在碰撞开启的情况下使用
	if( m_pScene && g_bRenderTerr && g_bCamTerrCol )
	{
		osassertex( _focus.x >= 0.0f,va( "错误的数据是:<%f>..\n",_focus.x ) );
		osassertex( _focus.z >= 0.0f,va( "错误的数据是:<%f>..\n",_focus.z ) );

		osVec3D   t_vec3Pos;

		//! 先检测焦点是否合法
		t_vec3Pos = _focus;
		m_pScene->get_detailPos( t_vec3Pos );
		if( m_vec3CamFocus .y <= (t_vec3Pos.y + 0.2f) )
			m_vec3CamFocus .y = (t_vec3Pos.y + 0.2f);


		//fixme:river加入位置高度与地表碰撞判断,相机会抖动	
		//! 再检测位置是否合法,强制使用一个在地表之上的位置
		t_vec3Pos = m_curpos;
		m_pScene->get_detailPos( t_vec3Pos,true );
		
		bool t_bColl = false;

		if( m_curpos.y < (t_vec3Pos.y-1.0f) )
			t_bColl = true;

		if( t_bColl )
		{
			// River mod @ 2008-11-5: 此时旋转相机，看是否可以到达一个没有碰撞的位置
			update_camera( false,false );
			
			int   t_iTryRotNum = 0;
			int   t_iCanRotNum = int(OS_PI*2/0.05f);
			while( t_bColl )
			{
				// 
				// 必须force rot，不然此时可能处于旋转插值状态，就不能旋转了
				rotate_horizon( m_vec3CamFocus,0.05f,true );
				t_vec3Pos = m_curpos;

				// 此处使用地表的高度就可以了，桥类的物品可以变的半透明
				m_pScene->get_detailPos( t_vec3Pos );
				if( m_curpos.y < (t_vec3Pos.y-1.0f) )
					t_bColl = true;
				else
					t_bColl = false;

				t_iTryRotNum ++;

				if( t_iTryRotNum > t_iCanRotNum )
					break;
			}

			// 
			// River Added @ 2008-11-17:加入更多的测试，防止死循环
			if( t_iTryRotNum > t_iCanRotNum )
			{
				while( t_bColl )
				{
					m_curpos.y = t_vec3Pos.y + 5.0f;
					t_vec3Pos = m_curpos;

					// 此处使用地表的高度就可以了，桥类的物品可以变的半透明
					m_pScene->get_detailPos( t_vec3Pos );
					if( m_curpos.y < (t_vec3Pos.y-1.0f) )
						t_bColl = true;
					else
						t_bColl = false;
				}
			}
		}
	}

	//syq 2006.4.17
	modify_pitch_yaw();
	
	m_bShouldUpdate = true;


}


//! pitchLock后，如果pitch有变动，一定时间后自动恢复为lock时的Pitch.
void osc_camera::update_camera_toLockPitch( float _time )
{
	guard;

	static float t_sfTime = 0.0f;

	t_sfTime += _time;

	
	// 
	// 每两秒检测一次
	if( t_sfTime > 2.0f )
	{
		float  t_fPitchBackup = m_fCurpitch;
		
		update_lockPitch();

		//! River added @ 2009-4-14:计算，如果此pitch值的位置为让相机处于地表之下
		//！此种情况下不能动画相机
		if( fabs( m_fCurpitch - t_fPitchBackup ) > 0.05f )
		{	
			cam_rot( m_fCurpitch,m_fCuryaw );
			osVec3D  t_vec3Pos = m_curpos;
			m_pScene->get_detailPos( t_vec3Pos );

			// 
			// 如果相机pitch后，跟地表有碰撞，则返回。
			// 必须是相机跟地表无碰撞和相机的位置和焦点之间的连线跟地表无碰撞.
			if( t_vec3Pos.y > (m_curpos.y + 0.1f) )
			{
				// 地表高度大于相机的高度
				m_fCurpitch = t_fPitchBackup;
				cam_rot( m_fCurpitch,m_fCuryaw );
			}
			else
			{
				float   t_fPitch = m_fCurpitch - t_fPitchBackup;
				
				m_fCurpitch = t_fPitchBackup;
				// river mod @ 2009-4-24:
				//animate_camRotPicth( 0.0f,t_fPitchBackup - m_fCurpitch,1.5f );
				animate_camRotPicth( 0.0f,t_fPitch ,1.5f );
			}
		}
		
		t_sfTime = 0.0f;
	}
	/**/

	return;

	unguard;
}


//! 把相机的位置拉到许可的范围内
void osc_camera::pull_camPosToMaxDis( void )
{
	guard;

	// River mod @ 2007-10-13:为了使相机不会越来越远
	float t_fDisFocus = osVec3Length( &(m_curpos-m_vec3CamFocus) );
	if( t_fDisFocus > m_fMaxDis )
	{
		t_fDisFocus -= m_fMaxDis;
		t_fDisFocus += 0.01f;
		move_close( -t_fDisFocus );
		update_camera( true,false );
	}


	// 
	pitch_camAboveFocus();

	unguard;
}



//! 把相机焦点之上拉,即位于人物的头顶
void osc_camera::pitch_camAboveFocus( void )
{
	guard;

	osVec3D    t_vec3Intersect,t_vec3RayDir;
	os_bsphere   t_sBs;
	bool       t_bRayInter,t_bSphereColl;

	//! River added @ 2009-7-31:第一次测试，如果没有碰撞，则直接返回。
	get_camNearClipBSphere( t_sBs );
	// 必须是相机跟地表无碰撞和相机的位置和焦点之间的连线跟地表无碰撞.
	t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );
	t_vec3RayDir = t_sBs.veccen - m_vec3CamFocus;
	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );
	t_bRayInter   = m_pScene->get_lineSegInterTerrPos
		( m_vec3CamFocus,m_curpos,t_vec3Intersect );
	if( t_bRayInter )
	{
		if( osVec3Length( &( t_vec3Intersect - m_vec3CamFocus ) ) >=
			osVec3Length( &( t_sBs.veccen    - m_vec3CamFocus ) )   )
			t_bRayInter = false;
	}
	if( (!t_bSphereColl) & (!t_bRayInter) )
		return;


	// River mod @ 2008-11-5:用于把相机旋转到没有碰撞的位置
	do
	{
		//! 如果旋转的过直但还有碰撞,则需要拉远相机,以防止死机的循环,因为相机可能锁定，
		//  所以必须加入force旋转参数
		if( OSE_CAMPITCH_TOO_HIGH == rotate_vertical( m_vec3CamFocus,0.05f,true ) )
			return;

		update_camera( true,false );

		get_camNearClipBSphere( t_sBs );

		// 必须是相机跟地表无碰撞和相机的位置和焦点之间的连线跟地表无碰撞.
		t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );

		t_vec3RayDir = t_sBs.veccen - m_vec3CamFocus;
		osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );
		t_bRayInter   = m_pScene->get_lineSegInterTerrPos
			( m_vec3CamFocus,m_curpos,t_vec3Intersect );
		if( t_bRayInter )
		{
			if( osVec3Length( &( t_vec3Intersect - m_vec3CamFocus ) ) >=
				osVec3Length( &( t_sBs.veccen    - m_vec3CamFocus ) )   )
				t_bRayInter = false;
		}

	}while( t_bSphereColl || t_bRayInter );


	return;

	unguard;
}


//! 相机跟地表间的碰撞检测,在这个函数内,会根据地形对相机的位置进行自动位置处理.
void osc_camera::modify_CollsionTerrain()
{
	guard;


	if( !g_bCamTerrCol )
		return;

	osVec3D    t_vec3Intersect,t_vec3RayDir;
	os_bsphere   t_sBs;

	if( !get_camNearClipBSphere( t_sBs ) )
		pitch_camAboveFocus();

	bool       t_bRayInter;
	bool       t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );

	t_vec3RayDir = t_sBs.veccen - m_vec3CamFocus;
	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );
	t_bRayInter   = m_pScene-> get_lineSegInterTerrPos( m_vec3CamFocus,m_curpos,t_vec3Intersect );
	if( t_bRayInter )
	{
		if( osVec3Length( &(t_vec3Intersect - m_vec3CamFocus ) ) >=
			osVec3Length( &(t_sBs.veccen    - m_vec3CamFocus ) )  )
			t_bRayInter = false;
	}



	// 如果碰撞,往相机焦点的方向上运动相机
	if( t_bSphereColl || t_bRayInter )
	{
		// 此变量用于确认当前相机是否在Y轴上方向上移动
		bool       t_bInYAxisMoveStatus = false;
		
		// 备份此相机的位置，用于计算自动回位时相机应该离焦点的矩离。
		osVec3D    t_vec3BackPos = m_curpos;
		

		// 
		// ATTENTION TO FIX:此处可能会把相机抬到很高的位置,然后出错
		do 
		{
			osVec3D   t_vec3Look;
			float     t_fDisToFocus,t_fStdFactor;



			get_godLookVec( t_vec3Look );
			t_fDisToFocus = osVec3Length( &(m_curpos-m_vec3CamFocus) );

			// 在相机位置离焦点的矩离低于这个值时，相机往上抬
			t_fStdFactor = MINDIS_TOFOCUS;

			// 
			// 如果跟地表相交,则从交点处开始猜测新的可到达的相机位置.
			// 如果此时已经往上运动去得到可用的相机位置，则不需要再处理。
			if( t_bRayInter && (!t_bInYAxisMoveStatus) )
			{
				m_curpos = t_vec3Intersect;
				// River @ 2006-7-14:必须保证相机的位置不能跑到对面去
				if( t_fDisToFocus > t_sBs.radius )
					m_curpos += (t_vec3Look*t_sBs.radius);
				else
					// 此时相机往上运动，是最好的运动方向
					t_fStdFactor = t_sBs.radius;
			}
			else
			{
				if( !t_bInYAxisMoveStatus )
					m_curpos += (t_vec3Look*0.1f);
				else
					move_close( 0.1f );
			}

			// 
			// 使用碰撞sphere的半径做为检测的标准
			if( t_fDisToFocus < t_fStdFactor )
			{
				if( !t_bRayInter )
					m_curpos -= (t_vec3Look*0.1f);

				//! 如果旋转的过直但还有碰撞,则需要拉远相机,以防止死机的循环.
				if( OSE_CAMPITCH_TOO_HIGH == rotate_vertical( m_vec3CamFocus,0.05f,true ) )
				{
					t_bInYAxisMoveStatus = true;
					move_close( 0.1f );
				}
			}

			update_camera( true,false );
			get_camNearClipBSphere( t_sBs );

			// 必须是相机跟地表无碰撞和相机的位置和焦点之间的连线跟地表无碰撞.
			t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );

			t_vec3RayDir = t_sBs.veccen - m_vec3CamFocus;
			osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );
			t_bRayInter   = 
				m_pScene->get_lineSegInterTerrPos( m_vec3CamFocus,m_curpos,t_vec3Intersect );
			if( t_bRayInter )
			{
				if( osVec3Length( &( t_vec3Intersect - m_vec3CamFocus ) ) >=
					osVec3Length( &( t_sBs.veccen    - m_vec3CamFocus ) )   )
					t_bRayInter = false;
			}

			// River mod @ 20079-17:确保相机位置和焦点之间不能过大的矩离
			if( t_fDisToFocus > m_fMaxDis )
				break;

		}while( t_bSphereColl || t_bRayInter );

		// River mod @ 2007-9-17:为了让相机的拉远不超过上层规定的最大相机位置和焦点矩离
		// 如果以上的智能检测不能达到要求,则向上旋转相机,直到到达人物的头顶
		if( t_bSphereColl || t_bRayInter )
		{
			// River mod @ 2007-10-9:
			//pitch_camAboveFocus();

			// River mod @ 2007-10-13:
			pull_camPosToMaxDis();
		}


		// 
		// River @ 2007-2-9:在某种出问题的情况下，可能会传入很大的矩离参数，
		// 我们在此处限制这个矩离，可以保证不会出现自动回位很远的情况。
		float   t_fResetDis = osVec3Length( &(t_vec3BackPos-m_vec3CamFocus) );
		if( t_fResetDis > m_fMaxDis )
			t_fResetDis = m_fMaxDis;

		// 因为修改了相机,启动自动回位的功能
		m_sAutoResetMgr.start_autoResetState( t_fResetDis,0.0f );

	}

	return;

	unguard;
}
void osc_camera::lerp_cameraRotPitch( float _etime )
{
	guard;

	float       t_fLerp;

	// 
	// 得到LerpVector.
	if( m_fEleTime > m_fCATime )
	{
		t_fLerp = 1.0f;
		m_bInRotPitchLerpState = false ;
	}
	else
		t_fLerp = (m_fEleTime / m_fCATime);


	unguard;
}


/** \brief
*  机测一个顶点是否在相机的视域内
*/
bool osc_camera::is_ptInCamVF( osVec3D& _pos )
{
	guard;

	os_ViewFrustum*    m_frus;
    
	osassert( g_ptrCamera );

	//
	// 目前检测当前的顶点是否在视域内。
	m_frus = g_ptrCamera->get_frustum();
	if( m_frus->objInVF( _pos ) )
		return true;
	else
		return false;

	unguard;
}



//! 对相机的位置做动画。普通用途：相机与地表碰撞时，相机可以柔和的到达新位置
bool osc_camera::animate_camRotPicth( float _rotRad,float _pitchRad,float _time )
{
	guard;

	// 处于相机动画状态时，不能再进行相机动画。
	if( (m_bInCAState)||(m_bInRotPitchLerpState) || m_bPlayingCamEffect )
		return false;

	osassert( fabs(_rotRad)<=(OS_PI*2) );
	osassert( fabs(_pitchRad)<=OS_PI );
	osassert( _time>=0 );


	m_fCATime = _time;
	m_fEleTime = 0.0f;
	m_fTotalPitch = _pitchRad;
	m_fTotalRot = _rotRad;

	m_fRotPerSec = this->m_fTotalRot/this->m_fCATime;
	m_fPitchPerSec = m_fTotalPitch / m_fCATime;

	m_bInRotPitchLerpState = true;

	return true;

	unguard;
}

void osc_camera::set_autoResetState( bool _autoReset )
{
	//m_bInRotPitchLerpState代表的意义是反的
	m_bInRotPitchLerpState = !_autoReset;
}


# if __EFFECT_EDITOR__

//! 从场景中调入camera,仅表示相机的位置和旋转方向等数据，跟视口无关。
bool osc_camera::load_camera( const char* _cname,
							 bool _ani/* = false*/,float _time/* = 0.0f*/  )
{
	guard;
	int     t_iSize;
	DWORD   t_dwFileVersion;
	char    t_szFileHeader[4];
	BYTE*   t_ptrStart;

	osassert( (_cname)&&(_cname[0]) );

	// 查看当前的文件是否存在
	if( !file_exist( (char*)_cname ) )
		return false;

	// 
	// 读入camera到全局的缓冲区。
	int   t_iGBufIdx = -1;
	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)_cname,t_ptrStart,TMP_BUFSIZE );
	osassert( t_iSize > 0 );

	// 读入文件头和版本号。
	READ_MEM_OFF( t_szFileHeader,t_ptrStart,sizeof( char )*4 );
	READ_MEM_OFF( &t_dwFileVersion,t_ptrStart,sizeof( DWORD ) );
	m_dwVersion = t_dwFileVersion;

	if( t_dwFileVersion == CAMFILE_VERSION )
	{
		// 
		// 所有的相机中，这两个值必须相等??????
		READ_MEM_OFF( &m_lookvec,t_ptrStart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_rightvec,t_ptrStart,sizeof( osVec3D ) );

		// 
		READ_MEM_OFF( &m_vec3NewPos,t_ptrStart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_vec3NewLook,t_ptrStart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_vec3NewRight,t_ptrStart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_fNewPitch,t_ptrStart,sizeof( float ) );
		READ_MEM_OFF( &m_fNewYaw,t_ptrStart,sizeof( float ) );

		if( m_fNewYaw < 0 )
			m_fNewYaw += 2*OS_PI;

		if( !_ani )
		{
			m_curpos = m_vec3NewPos;
			m_curLookVec = m_vec3NewLook;
			m_curRightVec = m_vec3NewRight;

			m_fCurpitch = m_fNewPitch;
			m_fCuryaw = m_fNewYaw;

			osVec3Normalize( &m_curLookVec, &m_curLookVec );
			osVec3D focus = m_curLookVec * 4;
			m_vec3CamFocus = m_curpos + focus;


		}
		else
		{
			m_bInCAState = true;
			m_fCATime = _time;
			m_fEleTime = 0.0f;

			m_vec3OldPos = m_curpos;	
			m_vec3OldLook = m_curLookVec;
			m_vec3OldRight = m_curRightVec;

			m_fOldPitch = m_fCurpitch;
			m_fOldYaw = m_fCuryaw;

			if( m_fOldYaw < 0 )
				m_fOldYaw += 2*OS_PI;
		}

		if( (m_fCurpitch)>=(OS_PI/2) )
			m_fCurpitch = OS_PI/2-float(0.01);
		if( (m_fCurpitch)<=-(OS_PI/2) )
			m_fCurpitch = -OS_PI/2 + float(0.01);
		
		if( (m_fCuryaw)>2*OS_PI )
			m_fCuryaw = m_fCuryaw - 2*OS_PI ;
		else if( (m_fCuryaw)<-2*OS_PI )
			m_fCuryaw = m_fCuryaw + 2*OS_PI;

		osVec3Cross( &m_upvec,&m_lookvec,&m_rightvec );

		m_bShouldUpdate = true;		
	}
	else if( t_dwFileVersion == CAMFILE_VERSION101 )
	{
		//syq
		// windy mod 5-27
		READ_MEM_OFF( &m_vec3NewCamPos,t_ptrStart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_vec3NewCamPos, t_ptrStart,sizeof( osVec3D ) );

		READ_MEM_OFF( &m_vec3NewCamPos,t_ptrStart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_vec3NewCamFocus,t_ptrStart,sizeof( osVec3D ) );

		if( !_ani )//无动画
		{
			//m_vec3NewCamPos = m_curpos;
			//m_vec3NewCamFocus = m_vec3CamFocus;
			m_curpos = m_vec3NewCamPos;
			m_vec3CamFocus = m_vec3NewCamFocus;

			//?
			//m_vec3OldCamPos =m_vec3NewCamPos ;
			//m_vec3OldCamFocus = m_vec3NewCamFocus
		}
		else
		{
			m_bInCAState = true;
			m_fCATime = _time;
			m_fEleTime = 0.0f;
			m_vec3OldCamPos = m_curpos;
			m_vec3OldCamFocus = m_vec3CamFocus;
			
		}
		m_bShouldUpdate = true;
		modify_pitch_yaw();
	}

	END_USEGBUF( t_iGBufIdx );
	update_camera();

	return true;
	unguard;
}


//! 存储场景中的camera位置数据。
bool osc_camera::save_camera( const char* _cname )
{
	guard;

	BYTE* t_ptrStart, *t_pCur;
	int   t_iGBufIdx = -1;
	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_pCur = t_ptrStart;
	WRITE_MEM_OFF( t_pCur, CAMFILE_HEADER, sizeof(char)*4 );
	WRITE_MEM_OFF( t_pCur, &CAMFILE_VERSION101, sizeof(DWORD) );

	WRITE_MEM_OFF( t_pCur, &m_curpos, sizeof(osVec3D) );
	WRITE_MEM_OFF( t_pCur, &m_vec3CamFocus, sizeof(osVec3D) );

	//存储无动画数据,只存储相同的点
	WRITE_MEM_OFF( t_pCur, &m_curpos, sizeof(osVec3D) );
	WRITE_MEM_OFF( t_pCur, &m_vec3CamFocus, sizeof(osVec3D) );

	write_file( (char*)_cname, t_ptrStart, int(t_pCur - t_ptrStart) );

	END_USEGBUF( t_iGBufIdx );


	return true;

	unguard;
}
# endif 



void osc_camera::play_camEffect( char * _camEffectName, int _nCntPerSec )
{
	m_nCntPerSec = _nCntPerSec;

	//读入相机特效文件(*.csf)
	int   t_iGBufIdx = -1;
	BYTE* p = START_USEGBUF( t_iGBufIdx );
	read_fileToBuf( _camEffectName,p,TMP_BUFSIZE );
	char header[4] = {0};
	READ_MEM_OFF( header,p,sizeof(char)*4);
	if( strcmp( header, "csf")!=0){
		osDebugOut( "camera: 相机特效文件<%s>格式不正确!\n", _camEffectName );
		return; 
	}
	int ver=0;
	READ_MEM_OFF( &ver,p,sizeof(DWORD));
	int vertexCount=0;
	READ_MEM_OFF( &vertexCount,p,sizeof(DWORD));
	END_USEGBUF( t_iGBufIdx );
	if(vertexCount<2)
	{
		osDebugOut( "camera: 相机特效文件<%s>无数据!\n", _camEffectName );
		return;
	}


	m_camEffectPoint.clear();
	m_camEffectPoint.resize(vertexCount);
	memcpy(&m_camEffectPoint[0],p,sizeof(osVec3D)*vertexCount );
	if(m_camEffectPoint[0]!=m_camEffectPoint[vertexCount-1]){
		osDebugOut( "camera: 相机特效文件<%s>需要闭合的曲线!\n", _camEffectName );
		return;
	}

	m_oldCamPos = m_curpos;
	m_oldCamFocus = m_vec3CamFocus;
	m_oldLookVec = m_curLookVec;
	//播放相机运动特效
	m_bPlayingCamEffect = true;
}

void osc_camera::update_camEffect()
{
	m_fPlayEffectTime += sg_timer::Instance()->get_lastelatime();
	int idx = (int)( m_fPlayEffectTime * m_nCntPerSec );
	if( idx<0 || idx>=(int)m_camEffectPoint.size() ){
		m_fPlayEffectTime   = 0.f;
		m_curpos = m_oldCamPos + m_offset + m_v3;


		m_vec3CamFocus = m_oldCamFocus + m_offset + m_v3;
		m_bPlayingCamEffect = false;
		update_camera();
		return ;
	}
	
	osVec3D v3,vFocus;
	m_v3 = m_camEffectPoint[ idx ];
	float tmp;
	tmp = m_v3.z;
	m_v3.z = m_v3.y;
	m_v3.y = tmp;
	m_v3.x = m_v3.z = 0;

	//相机焦点变化,相机震动时主角也在运动
	//osVec3D offset;
	m_offset = m_vec3CamFocus - m_oldCamFocus; 
	m_offset.y = 0.f;

	//保存当前的位置
	vFocus = m_oldCamFocus;


	m_curpos.x = m_oldCamPos.x + m_v3.x + m_offset.x;
	m_curpos.y = m_oldCamPos.y + m_v3.y + m_offset.y;
	m_curpos.z = m_oldCamPos.z + m_v3.z + m_offset.z;

	m_vec3CamFocus.x = vFocus.x + m_v3.x + m_offset.x;
	m_vec3CamFocus.y = vFocus.y + m_v3.y + m_offset.y;
	m_vec3CamFocus.z = vFocus.z + m_v3.z + m_offset.z;


	update_camera();

}


//! River @ 2009-4-9:根据最远最近矩离算出当前应该的pitch值
float osc_camera::update_lockPitch( void )
{
	guard;

	if( !m_bLockPitch )
		return m_fCurpitch;

	if( float_equal( m_fLockPitch,m_fLockPitchMin ) )
		return m_fLockPitch;

	float  t_fLerp = (m_fCamDisFocus - m_fMinFDis)/( m_fMaxFDis-m_fMinFDis );

	m_fCurpitch =  -osn_mathFunc::float_lerp( m_fLockPitchMin,m_fLockPitch,t_fLerp );

	return m_fCurpitch;

	unguard;
}


/** \brief
*  限制俯仰角为一固定区间值
*
*  \param _rad     最直的相机视角.
*  \param _radMin  最平的相机视角.
*  \param _minFDis 相机离焦点可以达到的最近矩离，在此位置上，相机使用最平的视角。
*  \param _maxFDis 相机离焦点可以达到的最远矩离，在此位置上，相机使用最直的视角。
*/
void osc_camera::limit_pitch( float _rad,float _radMin,float _minFDis,float _maxFDis )
{
	guard;

	if( this->m_bShouldUpdate )
		update_camera();

	m_bLockPitch = true;

	if( _rad > 0.0f )
	{
		// 最底层的pitch一直是负值,研究一下
		m_fCurpitch = -_rad;
	}
	m_fLockPitch = _rad;
	m_fLockPitchMin = _radMin;
	m_fMinFDis = _minFDis;
	m_fMaxFDis = _maxFDis;

	// TEST CODE:
	m_fCurpitch = update_lockPitch();

	cam_rot( m_fCurpitch, m_fCuryaw ); 

	update_camera();


	return;

	unguard;
}



/** \brief
 *  限制俯仰角为一固定值
 *
 *  \param _rad 固定的俯仰角度值(弧度)
 *  
 */
void osc_camera::limit_pitch( float _rad )
{
	guard;

	if( this->m_bShouldUpdate )
		update_camera();

	m_bLockPitch = true;

	if( _rad > 0.0f )
	{
		// 最底层的pitch一直是负值,研究一下
		m_fCurpitch = -_rad;
	}

	// River @ 2009-4-9:pitch最大值和最小值使用同一个数据
	m_fLockPitch = m_fCurpitch;
	m_fLockPitchMin = m_fLockPitch;

	cam_rot( m_fCurpitch, m_fCuryaw ); 
	update_camera();

	unguard;
}
void osc_camera::limit_pitch(void)
{
	guard;

	if( this->m_bShouldUpdate )
		update_camera();

	m_bLockPitch = true;

	cam_rot( m_fCurpitch, m_fCuryaw ); 
	update_camera();


	unguard;
}
/** \brief
 *  取消限制俯仰角为一固定值
 *
 *  \param _rad 固定的俯仰角度值(弧度)
 *  
 */
void osc_camera::unlimit_pitch( )
{
	m_bLockPitch = false;

}


//! 保存autoResetState.
void osc_camera::save_autoResetState( void )
{
	m_sBackUpARM = m_sAutoResetMgr;
}

//! 恢复autoResetState.
void osc_camera::restore_autoResetState( void )
{
	m_sAutoResetMgr = m_sBackUpARM;
}


//! 得到相机viewFrustum的8个顶点,这个八个顶点分别是: xyz,Xyz,xYz,XYz,xyZ,XyZ,xYZ,XYZ
const osVec3D* osc_camera::get_vfPt( void )
{
	os_ViewFrustum*  t_ptrVf;

	t_ptrVf = get_frustum();
	if( t_ptrVf )
		return t_ptrVf->m_FrustumVer;
	else
		return NULL;
}



//! 备份当前相机的位置和方向等所有的相机数据
void osc_camera::backup_curCam( void )
{
	guard;

	m_sCamBackup.m_fAspect =      this->m_fAspect;
	m_sCamBackup.m_fFarp =        this->m_fFarPlane;
	m_sCamBackup.m_fFov =         this->m_fFov;
	m_sCamBackup.m_fNearp =       this->m_fNearPlane;
	m_sCamBackup.m_iHei =         this->m_iHeight;
	m_sCamBackup.m_fMax_z =       this->m_fMaxz;
	m_sCamBackup.m_fMin_z =       this->m_fMinz;
	m_sCamBackup.m_iTlx =         this->m_iTopleftx;
	m_sCamBackup.m_iTly =         this->m_iToplefty;
	m_sCamBackup.m_iWid =         this->m_iWidth;
	m_sCamBackup.m_vec3CamFocus = this->m_vec3CamFocus;
	m_sCamBackup.m_vec3CamPos =   this->m_curpos;
	m_sCamBackup.m_fCurPitch    = this->m_fCurpitch;
	m_sCamBackup.m_fCurYaw      = this->m_fCuryaw;

	unguard;
}

//! 重用相机的备份数据
void osc_camera::use_backupCam( void )
{
	guard;

	m_fAspect      = m_sCamBackup.m_fAspect;  
	m_fFarPlane    = m_sCamBackup.m_fFarp;      
	m_fFov         = m_sCamBackup.m_fFov;        
	m_fNearPlane   = m_sCamBackup.m_fNearp;      
	m_iHeight      = m_sCamBackup.m_iHei;       
	m_fMaxz        = m_sCamBackup.m_fMax_z;      
	m_fMinz        = m_sCamBackup.m_fMin_z;      
	m_iTopleftx    = m_sCamBackup.m_iTlx;       
	m_iToplefty    = m_sCamBackup.m_iTly;        
	m_iWidth       = m_sCamBackup.m_iWid;        
	m_vec3CamFocus = m_sCamBackup.m_vec3CamFocus;
	m_curpos       = m_sCamBackup.m_vec3CamPos; 

	m_fCurpitch = m_sCamBackup.m_fCurPitch;
	m_fCuryaw   = m_sCamBackup.m_fCurYaw;

	//
	update_camera( false,false );

	unguard;
}

/** \brief : tzz modified
*  使相机振动的接口
*  
*  \param float _fDamp		振动阻尼.
*  \param float _swing      相机的振幅.
*/
void osc_camera::shake_camera( float _fDamp,float _swing )
{
	guard;
	
	//remember the data
	m_fShakeSwing	= _swing;
	m_fDamp			= _fDamp; 

	//set shaking
	m_bShaking		= true;
	m_fCurrTime		= 0.0f;

	return;

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: ShakeCamera
//Desc: shake camera in update camera
//--------------------------------------------------------------------------------------------------------------------
void osc_camera::ShakeCamera(void)
{
	//guard;
	
	if(m_bShaking){
		//process shaking formula

		// min swing to clear the shaking state
		const float t_fMinSwing = 1e-2f;

		//@{ \brief: get the current swing of damp shaking
		// e 
		const float e = 2.718281828f;

		// beta(damp)
		const float beta = m_fDamp;

		// time
		m_fCurrTime +=sg_timer::Instance()->get_lastelatime();

		// check the College Fundamental Physics
		float t_fCurrSwing = m_fShakeSwing * pow(e,-beta * m_fCurrTime);
		//@}

		if(t_fCurrSwing < t_fMinSwing){
			//clear the shake state
			m_bShaking = false;
			m_fCurrTime = 0.0f;
			return ;
		}

		//@{ \brief : set the camera position
		// cycle
		const float cycle = 200.0f;

		//offset position
		const float offset =  t_fCurrSwing * sin(cycle * m_fCurrTime);

		osVec3D t_offsetCamPos = m_curpos;
		osVec3D t_offsetTarPos = m_vec3CamFocus;

		t_offsetCamPos.x	+= offset;
		t_offsetCamPos.y	+= ((rand() % 2) == 0) ?(float)(rand() % 10) / 500.0f : -(float)(rand() % 10) / 500.0f;	//noise it
		t_offsetTarPos.x	+= offset;
		t_offsetTarPos.y	+= ((rand() % 2) == 0) ?(float)(rand() % 10) / 500.0f : -(float)(rand() % 10) / 500.0f;	//noise it
		
		osMatrix out;
		osVec3D up(0.0f,1.0f,0.0f);
		osMatrixLookAtLH( &out, &t_offsetCamPos, &t_offsetTarPos, &up  );
		m_viewmat = out;
	}


	//unguard;
}



//! 移动相机到一个位置上,避开障碍物,
//! 如果传入的参数值小于零，则仅仅使相机在未来一段时间不自动回位。
bool osc_camera::move_cameraToAvoidShelter( float _disToFocus )
{
	guard;

	//! 先移动相机到指定的位置....
	float t_fCurLength = osVec3Length( &(m_curpos-m_vec3CamFocus) );

	if( _disToFocus >= 0.0f )
	{

# if 1
		if( m_sAutoResetMgr.is_autoResetState() )
		{
			float t_fTmpLength = m_sAutoResetMgr.get_lastMoveLength();
			if( t_fTmpLength > 0.0f )
				move_close( -t_fTmpLength );
			else
				move_close( _disToFocus - t_fCurLength );
			
			// 设置一个状态，如果rot值没有修改，则不回弹
			m_sAutoResetMgr.pause_autoReset( true );
		}
		else
# endif 
		{
			move_close( _disToFocus - t_fCurLength );
			//! 开启自动回位的功能,如果没有其它的地方再移动相机,则相机开始自动回位.
			m_sAutoResetMgr.start_autoResetState( t_fCurLength,0 );
		}
	}

	//! 下一帧不做自动回位处理
	m_sAutoResetMgr.set_autoResetPass( false );

	return true;

	unguard;
}
