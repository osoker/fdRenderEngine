//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osCamera.cpp
 *
 *   Desc:     ʵ��Camera�Ľӿ�,ͬʱ����osok�����ڲ���Ҫ��Camera�ӿ�.
 *          
 *   His:      River Created @ 4/18 2003.
 *
 *   "��Ҫ���׸ı仨�˺ܳ�ʱ�������ľ���,�����͵����Ǳ������Լ�������".
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osCamera.h"
# include "../../interface/miskFunc.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"

//! ������������Ƿ����Զ���ĵ��Σ������Ϊfalse,��Զ���ĵ���һ�����������������ļ���
OSENGINE_API BOOL                     g_bClipFarTerr = TRUE;

//! �������Ƿ��Զ���������ر�����ײ���,Ĭ�������������⡣
OSENGINE_API BOOL                     g_bCamTerrCol = TRUE;


//��River added @ 2008-4-28:�Ƿ����������ڳ�����ͼ��
OSENGINE_API BOOL                     g_bClipCamInMap = FALSE;


//! ���λ���������������ľ���
# define MINDIS_TOFOCUS               0.2f

# define  MIN_CAMLIMITVAL     -0.1f


os_ViewFrustum::os_ViewFrustum()
{
	memset( this,0,sizeof( os_ViewFrustum ) );
}

//! ���������Ϣ
void os_ViewFrustum::output_debugInfo( void )
{
}


/** \brief
*  �鿴һ�����Ƿ���һ��view frustum��
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
*  �鿴һ��bounding sphere�Ƿ���һ��view frustum��
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
*  �鿴һ��bounding sphere�Ƿ���һ��view frustum��
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
*  �鿴һ��bounding box�Ƿ���һ��view frustum��
*/
bool os_ViewFrustum::objInVF( os_bbox& _bb )
{
	guard;

	const osVec3D*    t_vec3WorldPt;
	// ʹ��һ�ַ����ź�׼���ķ�����
	// os_bbox�ڵ�8�����㴦������ռ�󣬺�VF��6������бȽϣ�
	// �����8��������һ����ĺ��棬���������ڡ������������ڣ�
	// ���ִ�������ĳЩ���������ڵ�box�������ڵ����Σ��Թ���Щ����
	t_vec3WorldPt = _bb.get_worldVertexPtr(); 
	for( int t_i=0;t_i<5;t_i ++ )
	{
		int t_j;
		for( t_j =0;t_j<8;t_j++ )
		{
			if( osn_mathFunc::distance_V2P(t_vec3WorldPt[t_j],m_Frustum[t_i])>0 )
				break;
		}

		// bbox��8�����㶼�ڵ�t_i����ĺ���
		if( t_j == 8 )
			return false;
	}

	return true;

	unguard;
}

//! �鿴һ��aabbox�Ƿ���һ��vf��(���Ա�VF����.)
bool os_ViewFrustum::objInVF( const os_aabbox& _box )
{
	osVec3D   t_vec3;

	// ���AABBOX�����ཻ��ֱ�ӷ���
	if( !m_sAabbox.collision( _box ) )
		return false;

	for( int t_i=0;t_i<5;t_i ++ )
	{
		// �ҵ���������������ڵĶ���
		t_vec3.x = ((m_Frustum[t_i].a < 0.0f) ? _box.m_vecMin.x : _box.m_vecMax.x );
		t_vec3.y = ((m_Frustum[t_i].b < 0.0f) ? _box.m_vecMin.y : _box.m_vecMax.y );
		t_vec3.z = ((m_Frustum[t_i].c < 0.0f) ? _box.m_vecMin.z : _box.m_vecMax.z );

		// ֻҪ��һ����������������ڵĶ�����������֮�⣬���aabboxʹ�ô��ַ�ʽ��⣬���������ڡ�
		if( osn_mathFunc::distance_V2P( t_vec3,m_Frustum[t_i] ) < 0.0f )
			return false;
	}

	return true;
}

//! ��ȷ���vf�Ƿ���aabbox�ཻ,ATTENTION TO FIX:�˺����Ƿ��Ҫ��
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
*  ���ڼ��һ��aabbox��vf�Ĺ�ϵ
*
*  \return bool aabboxȫ����vf�ڣ�����true.�ཻ����false.
*                     ȫ����vf�������objInVF�����ж�
*  
*  �㷨����ÿһ��vf�棬�ҵ�������������֮��Ķ��㣬��⣬������е���Щ
*        ���㶼��vf���ǰ�棬������aabbox��vf֮��.
*/
bool os_ViewFrustum::objCompleteInVF( const os_aabbox& _box )
{
	osVec3D   t_vec3;

	for( int t_i=0;t_i<5;t_i ++ )
	{
		// ÿһ�����㶼���������������ǰ��Ķ���
		t_vec3.x = ((m_Frustum[t_i].a < 0.0f) ? _box.m_vecMax.x : _box.m_vecMin.x );
		t_vec3.y = ((m_Frustum[t_i].b < 0.0f) ? _box.m_vecMax.y : _box.m_vecMin.y );
		t_vec3.z = ((m_Frustum[t_i].c < 0.0f) ? _box.m_vecMax.z : _box.m_vecMin.z );

		if( osn_mathFunc::distance_V2P( t_vec3,m_Frustum[t_i] ) < 0.0f )
			return false;
	}

	return true;
}

//! ���һ�������Ƿ�����ң��ϣ��£�Զ�������ཻ���������εľ�ȷ�ཻ
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


//! ���һ�������Ƿ������Ľ��������ཻ���Ǿ�ȷ�������������������Σ����ǽ��������ƽ�档
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

//! һ��������㣬һ�����ȣ�һ�����򣬵����µĶ���
void os_ViewFrustum::get_pt( osVec3D& _s,osVec3D& _dir,float _len,osVec3D& _res )
{
	_res = _s + _dir*_len;
}


//! ����һ����ʼ��ͽ����㣬�ҵ���clip��Ľ���
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


//! �˺�����һ���߶�clip��������,�����޸���������
bool os_ViewFrustum::clip_lineSeg( osVec3D& _s,osVec3D& _e )
{
	guard;

# if 0
	//! ������������
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

	//! ��������㶼�ڽ�������֮ǰ,�򷵻���
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

	//! �������㶼�������ڵ����
	if( objInVF( _s )&&objInVF( _e ) )
		return true;

	osVec3D  t_resA,t_resB;

	//! �������㶼��������
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

	//! һ���������⣬һ���ڲ�,����
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
//! ����ˮ�����µ���Ⱦ������ǰ��vf.
void os_ViewFrustum::reflect_vf( osPlane& _plane )
{
	guard;

	osMatrix    t_sReflMat;

	osMatrixReflect( &t_sReflMat,&t_sClipPlane );


	unguard;
}*/




//! ����Vf��Զ�����棬Ҫ��ʵ�ʵ�Զ�������һ��
void os_ViewFrustum::draw_farPlane( osc_middlePipe* _pipe )
{
	guard;

	os_polygon       t_rPolygon;

	//
	// ����Bounding Box����Ⱦ���ݡ�
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
//! ��������ļ����ļ�ͷ��
const char* CAMFILE_HEADER  =   "cam";
const DWORD CAMFILE_VERSION =   0x100;
const DWORD CAMFILE_VERSION101 =   0x101;

//! ���·�Χ: 0 �� -(OS_PI/2),   ��ת��Χ:��osVec3D(-1,0,0)Ϊ��, ��ʱ��,0-2*OS_PI
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

	// �ϲ���������������ݵ���һ������ĳ̶�
	m_fMaxDis = 60.f;
	m_fMinDis = 0.f;
	m_bCammerLimitY = true;
	m_NeedCollsionTerrain = true;

	m_vec3CamFocus = osVec3D(0,0,0);

}

//! �õ�����ɿ�����Զ������ľ��룬�������ɼ�����Զ����
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
//  ��ʼ��Camera�ĺ���.
//
/**********************************************************************/
/** \brief 
*  ��һ���ṹ�г�ʼ��һ��Camera.
*
*  \param _cdata  ������ṹ�г�ʼ�����ǵ�Camera.
*
*/
void osc_camera::init_camera( const os_cameraInit* _cdata )
{
	guard;

	m_pScene = (osc_TGManager*)get_sceneMgr();
	osassert( m_pScene );

	osVec3D     t_vec;
	osassert( _cdata );
	osassertex( _cdata->camFocus != _cdata->curPos, "�����ʼ��ʱλ���뽹�㲻����ͬ!\n" );
	osassert( _cdata->fov > 0.0f );
	osassert( _cdata->faspect > 0.0f );
	osassert( _cdata->nearp > 0.0f );
	osassert( _cdata->farp > _cdata->nearp );


	// River add 2009-2-26: �л������󣬲�������������ڶ���״̬
	m_bInRotPitchLerpState = false;
	m_bInCAState = false;
	m_sAutoResetMgr.end_autoResetState();

	// ��ʼ��Camera��λ����Ϣ.
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


	// ����Projection Matrix����Ϣ.
	// �����������ҲҪ0.35����
	float t_fNear = _cdata->nearp;
	if( t_fNear < 0.35f )
		t_fNear = 0.35f;

	set_projmat( _cdata->nearp,_cdata->farp,
		_cdata->fov,_cdata->faspect );


	// ����Viewport.
	this->set_viewport( _cdata->tlx,_cdata->tly,
		_cdata->wid,_cdata->hei,_cdata->min_z,_cdata->max_z );

	// Set updata flag to true.
	this->m_bShouldUpdate = true;

	//
	m_fTADis = float(m_fFarPlane*TA_SCALE);
	// ʹ��Ʒ�Ŀɼ��Ƚ��ڵر�͸���ĸ���
	m_fHalfFar = m_fTADis/2.0f;
	m_fObjFullView = float(m_fHalfFar*2.0f-(1.0f-TA_SCALE)*m_fFarPlane);

	//syq �����ݳ�ʼ��
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

//! 2009-9-4:һ��������λ�ú�focus,���ڽ��һ���ܱ�̬����Ļ�ζ���bug.
void osc_camera::set_curPosFocus( osVec3D& _pos,osVec3D& _focus,bool _force/* = false*/ )
{
	guard;

	// River mod @ 2008-4-28:��������ڵ�ͼ֮�������
	if( g_bClipCamInMap )
	{
		// ���뱣֤�����λ����������ͼ֮��,��������
		if( (_pos.x<0.0f)||(_pos.z<0.0f)||
			(_pos.x>g_fMapMaxX)||(_pos.z>g_fMapMaxZ) )
			return;
	}


	// 
	// River added @ 2009-5-19:��׼���������ײ���ص���
	if( !vec3_equal( m_vec3CamFocus,(osVec3D &)_focus ) )
		m_sAutoResetMgr.pause_autoReset( false );
	else
		return;

	m_vec3CamFocus = _focus;	
	//�������ϲ��趨��ֵ�п��ܻᳬ���䶨�����Զ���룬�ײ㲻���д��ж�
	m_curpos = _pos;


	// 
	// River @ 2006-7-14:�жϵ�ǰ���õĽ����Ƿ�Ϸ�,ֻ������ײ�����������ʹ��
	if( m_pScene && g_bRenderTerr && g_bCamTerrCol )
	{
		osassertex( _focus.x >= 0.0f,va( "�����������:<%f>..\n",_focus.x ) );
		osassertex( _focus.z >= 0.0f,va( "�����������:<%f>..\n",_focus.z ) );

		osVec3D   t_vec3Pos;

		//! �ȼ�⽹���Ƿ�Ϸ�
		t_vec3Pos = _focus;
		m_pScene->get_detailPos( t_vec3Pos );
		if( m_vec3CamFocus .y <= (t_vec3Pos.y + 0.2f) )
			m_vec3CamFocus .y = (t_vec3Pos.y + 0.2f);


		//fixme:river����λ�ø߶���ر���ײ�ж�,����ᶶ��	
		//! �ټ��λ���Ƿ�Ϸ�,ǿ��ʹ��һ���ڵر�֮�ϵ�λ��
		t_vec3Pos = m_curpos;
		m_pScene->get_detailPos( t_vec3Pos,true );
		
		bool t_bColl = false;

		if( m_curpos.y < (t_vec3Pos.y-1.0f) )
			t_bColl = true;

		if( t_bColl )
		{
			// River mod @ 2008-11-5: ��ʱ��ת��������Ƿ���Ե���һ��û����ײ��λ��
			update_camera( false,false );
			
			int   t_iTryRotNum = 0;
			int   t_iCanRotNum = int(OS_PI*2/0.05f);
			while( t_bColl )
			{
				// 
				// ����force rot����Ȼ��ʱ���ܴ�����ת��ֵ״̬���Ͳ�����ת��
				rotate_horizon( m_vec3CamFocus,0.05f,true );
				t_vec3Pos = m_curpos;

				// �˴�ʹ�õر�ĸ߶ȾͿ����ˣ��������Ʒ���Ա�İ�͸��
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
			// River Added @ 2008-11-17:�������Ĳ��ԣ���ֹ��ѭ��
			if( t_iTryRotNum > t_iCanRotNum )
			{
				while( t_bColl )
				{
					m_curpos.y = t_vec3Pos.y + 5.0f;
					t_vec3Pos = m_curpos;

					// �˴�ʹ�õر�ĸ߶ȾͿ����ˣ��������Ʒ���Ա�İ�͸��
					m_pScene->get_detailPos( t_vec3Pos );
					if( m_curpos.y < (t_vec3Pos.y-1.0f) )
						t_bColl = true;
					else
						t_bColl = false;
				}
			}
		}
	}
	
	//! �޸�pitch��yaw��ֵ
	modify_pitch_yaw();

	m_bShouldUpdate = true;

	unguard;
}



/** \brief
*  ���������ǰ��λ��.
*/
void osc_camera::set_curpos( osVec3D* _vec,bool _force/* = false*/  )
{
	guard;

	if( !_force )
	{
		if( m_bInCAState  )
			return;
	}

	// River mod @ 2008-4-28:��������ڵ�ͼ֮�������
	if( g_bClipCamInMap )
	{
		// ���뱣֤�����λ����������ͼ֮��,��������
		if( (_vec->x<0.0f)||(_vec->z<0.0f)||
			(_vec->x>g_fMapMaxX)||(_vec->z>g_fMapMaxZ) )
			return;
	}

	//�������ϲ��趨��ֵ�п��ܻᳬ���䶨�����Զ���룬�ײ㲻���д��ж�
	m_curpos = *_vec;
	
	//! �޸�pitch��yaw��ֵ
	modify_pitch_yaw();

	m_bShouldUpdate = true;


	unguard;
}


//! �����Ƕ�ֵ,ʹ�Ƕ�ֵ����ȷ��
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
//  ����Camera��Ҫ�Ľӿ�.
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
*   ʹ�����Godģʽ��ǰ�ƶ�һ�ξ���(��������ɸ�).
*
*   ִ�в���:
*
*   1: �����Ҫ����Look vector,�ȹ����µ�Look vector.
*
*   2: ��Look vector���ƶ�Ҫ�ƶ��ľ���.
* 
*/
void osc_camera::move_forward_godmod( float _dis )
{
	guard;


	//
	// �����Ҫ����Look Vector,�����µ�look vector.
	//
	if( this->m_bShouldUpdate )
		update_camera();
	
	//
	//  ������������ƶ�Ҫ�ƶ��ľ���.
	//
	m_curpos.x -= m_curLookVec.x*_dis;
	m_curpos.y -= m_curLookVec.y*_dis;
	m_curpos.z -= m_curLookVec.z*_dis;
	
	m_bShouldUpdate = true;
	
	unguard;
}

/** \brief
*   ʹ�����Generalģʽ��ǰ�ƶ�һ�ξ���.
*
*   ִ�в���:
*
*   1: �ȹ���Camera Look vector��ˮƽ�����ϵ�Vector.
*
*   2: ��Look vector��ˮƽ�������ƶ�Ҫ�ƶ��ľ���.
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
	// ����Camera Look vector��ˮƽ�����ϵ�Vector.
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
	//��Ϊ������ģʽ�����Ծ���camera��pitch,�ƶ�Ҳ������pitch.
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
	//  �����ˮƽ�������ƶ���Ӧ�ľ���.
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
	// ����һ��ֵ��ʹ�ó����в�������������һ�����Ʒ��
	// River @ 2--6-3-22: Ϊ��ʹ�ϲ�ĵ�����Ⱦ��������ļ��У�
	//                    ����g_bClipFarTerr��ȫ�ֱ���
	if( _offset && (!g_bClipFarTerr) )
		t_fFar += CLIPDIS_ADD;


	// 
	//now only use the hfov....use d3dx helper func.fov_horiz1.5*SG_PI
	osMatrixPerspectiveFovLH( &_mat,this->m_fFov,this->m_fAspect,
		m_fNearPlane,t_fFar );

}


/***************************************************************************************/
//
//  ʹ������е����ݽ���һЩ�������ļ���.
//
/***************************************************************************************/
//@{
/** \brief
*   �õ���Ļ��һ����ά�����������ά�ռ��е�����.
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
//  ����������Щ���к���ֻ�������������ڲ�,���ṩ�ϲ�ʹ����Щ�����Ľӿ�.
//
//=====================================================================================================


/** \brief 
*  ʹ������������õ�ViewPort
* 
*  Set_viewport�ǳ�ʼ���Ժ����Ҫ���õĵĺ�����ʹ������������õ�ViewPort.
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
*  ����һ������ռ�Ķ���,������ǰ����ռ䶥���Ӧ����Ļ�ռ�Ķ���.
*
*  \param _wpos ���������ռ�Ķ�������,������Ϊ��������.
*  \param _spos ����������ռ䶥���Ӧ����Ļ����.
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

	// River mod @ 2009-2-5: �жϴ���Ķ����Ƿ��������������֮��
	osVec3Normalize( &t_vec3PtDir,&(t_dir) );
	float t_fDot = osVec3Dot( &t_vec3PtDir,&m_curLookVec );
	if( t_fDot <= 0.0f )
		return false;

	// tzz ��������������������ģ��Ͳ�������ʾ
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


//!  �õ�ˮƽ�����ϵ�look vec.
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
	
	// ��Ϊ������ģʽ�����Ծ���camera��pitch,�ƶ�Ҳ������pitch.
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
*  Update the camera.����Camera�еĸ���Matrix.
*/
void osc_camera::update_camera( bool _buildFrus/* = true*/,
		bool _colwithTerr/* = true*/,bool _reflect/* = false*/ )
{
	guard;
	
	osVec3D up( 0.0f,1.0f,0.0f );
	osVec3D right( 1.0f,0.0f,0.0f );
	osVec3D look =  m_vec3CamFocus - m_curpos;
	osVec3Normalize( &look,&look );

	// River @ 2011-10-18�������أ����ٱ�֤������
	//osassert( !vec3_equal( m_vec3CamFocus,m_curpos ) );
	if( vec3_equal( m_vec3CamFocus,m_curpos ) )
		return;

	// River mod @ 2008-12-8:Ϊ����ˮ�浹Ӱ,�õ���� 
	if( _reflect )
	{
		up = osVec3D( 0.0f,-1.0f,0.0f );
		right = osVec3D( -1.0f,0.0f,0.0f );
	}


	// TEST CODE: river @ 2006.5.8,��ĳ��������������
	if( g_bRenderTerr )
	{
		float t_f = 0.0f;

		// river mod 2008-4-28:
		if( g_bClipCamInMap )
		{
			// River mod @ 2007-4-18:ʹ������ٳ���
			if( m_curpos.x < 0.0f )
				m_curpos.x = 0.1f;
			if( m_curpos.z < 0.0f )
				m_curpos.z = 0.1f;
		}

		// River mod @ 2007-12-5:Ϊ�˲�ʹ������˶�������ѭ��
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
	// �������ȫ�Ķ��ӽǣ���up Vector����ʹ��( 0,0,1 )
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
*  ������ǰCamera��frustum.
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

	//! River @ 2009-4-14:���ȶ��ĳ�����
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

	// �ع�vf��aabbox
	osn_mathFunc::cal_aabboxFromVArray( 
		t_resFrus->m_FrustumVer,8,t_resFrus->m_sAabbox );	

	return;

	unguard;
}

/** \brief
*  �õ������ǰ��ָ��ķ���,����������ں���ģ��,�����˷����
*  ����.
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

	// River mod @ 2009-4-14:Ϊ�˸��ȶ��ĳ�����
	if( m_fCamDisFocus < 0.1f )
		m_fCamDisFocus = 0.1f;

	update_lockPitch();

	cam_rot( m_fCurpitch, m_fCuryaw );
	
	m_bShouldUpdate = true;


}

/** \brief
 *  ��������λ�õ�����ľ���
 */
float osc_camera::get_disFromCamposToFocus()
{
	osVec3D v = m_curpos - m_vec3CamFocus;
	return osVec3Length( &v );
}

/** \brief
*  ʹ�����һ���������ˮƽ��ת��
*
*  �㷨������
*  1: �ѵ�ǰ�������λ�ö���ת����ԭ�����ꡣ
*  2: ��y����ת��ǰ���ƽ�ƺ��λ�ö��㡣
*  3: ƽ�Ƶ�ǰ��ת��Ķ��㵽�������ռ䡣
*  4: ���������Ҫ�����ݡ�
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
*  ʹ�����һ���������������ת��
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


	// ������̧��ֱ̫,����.
	if( (m_fCurpitch + _radians)>=(MIN_CAMLIMITVAL)  )		
		return OSE_CAMPITCH_TOO_HIGH;

	// �������������������洩��
	if( fabs( m_fCurpitch + _radians ) >= OS_PI )
		return OSE_CAMPITCH_TOO_LOW;


	// ��lockPitch״̬����force״̬
	if( (!m_bLockPitch) || _force )
	{
		pitch( _radians );

		cam_rot( m_fCurpitch, m_fCuryaw );
	}


	return OSE_CAMOPE_SUCCESS;

	unguard;
}



// ���������λ���뽹���������
void osc_camera::set_maxDistance( float _maxDis )
{
	m_fMaxDis = _maxDis;

	m_bShouldUpdate = true;
}

// ���������λ���뽹�����С����
void osc_camera::set_minDistance( float _minDis )
{
	m_fMinDis = _minDis;

	m_bShouldUpdate = true;
}

// �޸����λ���뽹��ľ�������Ч����������
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

//! ATTENTION��TO��FIX����
//! ����ɼ�һ�����ӣ���ɼ���������ϵ���Ʒ����Ʒ�Ϳ��ܻ�����������󡣡�
//! ���崦����Ʒ�Ƿ���ʾ��Factor,��ֵԽ����ʾ����ƷԽ��,��ֵСһЩ��ʱ��,Զ����һЩС��Ʒ�Ͳ������.
# define  INVIEW_FACTOR   0.6f

//
//!  �������Ʒ��bounding Sphere���������Ұ��Χ�ڡ�
//!  �Ƿ��������������һ���뾶��λ�á�
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

	// ȷ�Ͽɼ���Ʒ���������Զ��͸���ĸ�����
	if( (t_fDistance-_radius)>m_fObjFullView )
		return MASK_OUTHALF_NOTVIEW;

	// �����Ʒ�뾶��ƽ������factor����������룬��ɼ������򲻿ɼ���
	t_fDistance -= m_fHalfFar;
	if( ((_radius*_radius)*INVIEW_FACTOR)>t_fDistance )
	{
		return MASK_OUTHALF_VIEW;
	}


	return MASK_OUTHALF_NOTVIEW;

	unguard;
}

//! �õ����е�����ӽǿ��.
float osc_camera::get_cameraFov( void )
{
	return this->m_fFov;
}






//! ����������������в�ֵ��
void osc_camera::lerp_cameraFile( void )
{
	guard;

	float       t_fLerp;
	// 
	// �õ�LerpVector.
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
		// �����ʹ�õ�ÿһ���������в�ֵ.
		osVec3Lerp( &m_curpos,&m_vec3OldPos,&m_vec3NewPos,t_fLerp );
		osVec3Lerp( &m_curLookVec,&m_vec3OldLook,&m_vec3NewLook,t_fLerp );
		osVec3Lerp( &m_curRightVec,&m_vec3OldRight,&m_vec3NewRight,t_fLerp );

		m_fCurpitch = m_fOldPitch + (m_fNewPitch-m_fOldPitch)*t_fLerp;


		//���㽹��
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

//! �����֡�������Զ���λʱ���Ž����Զ���λ���������������������Ķ���.
# define RELAXAUTORESET_TIMES  8

//! ���Զ���λ������λ�ý��д���,�������,������Ϊ�ɵ����λ��.
void osc_camera::process_camAutoReset( void )
{
	guard;

	if( (!m_sAutoResetMgr.is_autoResetState()) || (!g_bCamTerrCol) )
		return ;


	float  t_fMove;

	m_sAutoResetMgr.process_camAutoResetPos( this,t_fMove );

	osVec3D   t_vec3Dir;

	// �Զ���λ��
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

	// �����ײ,���������ķ������˶����
	if( (t_bSphereColl || t_bRayInter) )
	{	
		// ʹ����һ֡��ȷ�����λ��,��ֹ����Ķ�������.
		m_curpos = m_vec3LastFrameCamPos;

		update_camera( true,false );


		m_sAutoResetMgr.set_autoResetPass( false );
	}
	else
	{
		m_sAutoResetMgr.set_autoResetPass( true );

		if( !m_sAutoResetMgr.pass_autoResetTime() )
		{
			// ʹ����һ֡��ȷ�����λ��,��ֹ����Ķ�������.
			m_curpos = m_vec3LastFrameCamPos;
			update_camera( true,false );
		}
	}

	return;

	unguard;
}


//!  �����frameMove���������ڴ�������Ķ�����
void osc_camera::frame_moveCamera( float _etime )
{
	guard;
	
	// River added @ 2008-6-17������ײ״̬�£���������
	if( (!g_bCamTerrCol)&&(!m_bInCAState) )
		return;
	
	// ���picth lock,���.
	if( m_bLockPitch )
		update_camera_toLockPitch( _etime );


	m_bRotInCurFrame = false;

	if( m_bPlayingCamEffect )
	{
		update_camEffect();
		return;
	}

	// 
	// River @ 2006-9-9:������λ����focus֮�£����պ���Y���ϣ�������ײ����
	//                  �����������������������������ݡ�
	osVec3D   t_vec3Nor;
	osVec3Normalize( &t_vec3Nor,&(m_vec3CamFocus-m_curpos) );
	if( fabs(t_vec3Nor.y - 1.0f)<0.0001f )
		return;


	if( (!m_bInCAState) && (!m_bInRotPitchLerpState) )
	{ 

		update_camera();

		// �������Ľ����λ�����ڵĵ�ͼû�е��룬�������������ײ���
		int   t_iXPos,t_iZPos,t_iXFocus,t_iZFocus;
		t_iXPos = int( m_curpos.x / TGMAP_WIDTH );
		t_iZPos = int( m_curpos.z / TGMAP_WIDTH );
		t_iXFocus = int( m_vec3CamFocus.x / TGMAP_WIDTH);
		t_iZFocus = int( m_vec3CamFocus.z / TGMAP_WIDTH);
		if( (!m_pScene->is_tgLoaded( t_iXPos,t_iZPos )) ||
			(!m_pScene->is_tgLoaded( t_iXFocus,t_iZFocus) ) )
			return;


		//! ��������ܵ���ײ���
		modify_CollsionTerrain();

		m_vec3LastFrameCamPos = m_curpos;

		// �����ǰ֡���û�и�������ײ,�����Զ���λ����
		process_camAutoReset();

		return;
	}

	m_fEleTime += _etime;

	if( m_bInCAState )
	{
		lerp_cameraFile();
		return; 
	}



	// �����������ת��pitch��ֵ��
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

//! �õ��������λ�úͽ��������γɵ�bsphere.
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


	// ��xyz �� XYz������������.
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



//! TEST CODE:����ȷ����������ر�����ײ����.
void osc_camera::assert_camera( void )
{
	guard;

	os_bsphere   t_sBs;

	get_camNearClipBSphere( t_sBs );

	bool       t_bSphereColl = m_pScene->get_sphereInterTerrPos( &t_sBs );

	osassert( !t_bSphereColl );

	unguard;
}

//!  ���ú͵õ�����Ľ���.
void osc_camera::set_camFocus( const osVec3D& _focus )
{
	// 
	// River added @ 2009-5-19:��׼���������ײ���ص���
	if( !vec3_equal( m_vec3CamFocus,(osVec3D &)_focus ) )
		m_sAutoResetMgr.pause_autoReset( false );
	else
		return;

	m_vec3CamFocus = _focus;	

	// 
	// River @ 2006-7-14:�жϵ�ǰ���õĽ����Ƿ�Ϸ�,ֻ������ײ�����������ʹ��
	if( m_pScene && g_bRenderTerr && g_bCamTerrCol )
	{
		osassertex( _focus.x >= 0.0f,va( "�����������:<%f>..\n",_focus.x ) );
		osassertex( _focus.z >= 0.0f,va( "�����������:<%f>..\n",_focus.z ) );

		osVec3D   t_vec3Pos;

		//! �ȼ�⽹���Ƿ�Ϸ�
		t_vec3Pos = _focus;
		m_pScene->get_detailPos( t_vec3Pos );
		if( m_vec3CamFocus .y <= (t_vec3Pos.y + 0.2f) )
			m_vec3CamFocus .y = (t_vec3Pos.y + 0.2f);


		//fixme:river����λ�ø߶���ر���ײ�ж�,����ᶶ��	
		//! �ټ��λ���Ƿ�Ϸ�,ǿ��ʹ��һ���ڵر�֮�ϵ�λ��
		t_vec3Pos = m_curpos;
		m_pScene->get_detailPos( t_vec3Pos,true );
		
		bool t_bColl = false;

		if( m_curpos.y < (t_vec3Pos.y-1.0f) )
			t_bColl = true;

		if( t_bColl )
		{
			// River mod @ 2008-11-5: ��ʱ��ת��������Ƿ���Ե���һ��û����ײ��λ��
			update_camera( false,false );
			
			int   t_iTryRotNum = 0;
			int   t_iCanRotNum = int(OS_PI*2/0.05f);
			while( t_bColl )
			{
				// 
				// ����force rot����Ȼ��ʱ���ܴ�����ת��ֵ״̬���Ͳ�����ת��
				rotate_horizon( m_vec3CamFocus,0.05f,true );
				t_vec3Pos = m_curpos;

				// �˴�ʹ�õر�ĸ߶ȾͿ����ˣ��������Ʒ���Ա�İ�͸��
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
			// River Added @ 2008-11-17:�������Ĳ��ԣ���ֹ��ѭ��
			if( t_iTryRotNum > t_iCanRotNum )
			{
				while( t_bColl )
				{
					m_curpos.y = t_vec3Pos.y + 5.0f;
					t_vec3Pos = m_curpos;

					// �˴�ʹ�õر�ĸ߶ȾͿ����ˣ��������Ʒ���Ա�İ�͸��
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


//! pitchLock�����pitch�б䶯��һ��ʱ����Զ��ָ�Ϊlockʱ��Pitch.
void osc_camera::update_camera_toLockPitch( float _time )
{
	guard;

	static float t_sfTime = 0.0f;

	t_sfTime += _time;

	
	// 
	// ÿ������һ��
	if( t_sfTime > 2.0f )
	{
		float  t_fPitchBackup = m_fCurpitch;
		
		update_lockPitch();

		//! River added @ 2009-4-14:���㣬�����pitchֵ��λ��Ϊ��������ڵر�֮��
		//����������²��ܶ������
		if( fabs( m_fCurpitch - t_fPitchBackup ) > 0.05f )
		{	
			cam_rot( m_fCurpitch,m_fCuryaw );
			osVec3D  t_vec3Pos = m_curpos;
			m_pScene->get_detailPos( t_vec3Pos );

			// 
			// ������pitch�󣬸��ر�����ײ���򷵻ء�
			// ������������ر�����ײ�������λ�úͽ���֮������߸��ر�����ײ.
			if( t_vec3Pos.y > (m_curpos.y + 0.1f) )
			{
				// �ر�߶ȴ�������ĸ߶�
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


//! �������λ��������ɵķ�Χ��
void osc_camera::pull_camPosToMaxDis( void )
{
	guard;

	// River mod @ 2007-10-13:Ϊ��ʹ�������Խ��ԽԶ
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



//! ���������֮����,��λ�������ͷ��
void osc_camera::pitch_camAboveFocus( void )
{
	guard;

	osVec3D    t_vec3Intersect,t_vec3RayDir;
	os_bsphere   t_sBs;
	bool       t_bRayInter,t_bSphereColl;

	//! River added @ 2009-7-31:��һ�β��ԣ����û����ײ����ֱ�ӷ��ء�
	get_camNearClipBSphere( t_sBs );
	// ������������ر�����ײ�������λ�úͽ���֮������߸��ر�����ײ.
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


	// River mod @ 2008-11-5:���ڰ������ת��û����ײ��λ��
	do
	{
		//! �����ת�Ĺ�ֱ��������ײ,����Ҫ��Զ���,�Է�ֹ������ѭ��,��Ϊ�������������
		//  ���Ա������force��ת����
		if( OSE_CAMPITCH_TOO_HIGH == rotate_vertical( m_vec3CamFocus,0.05f,true ) )
			return;

		update_camera( true,false );

		get_camNearClipBSphere( t_sBs );

		// ������������ر�����ײ�������λ�úͽ���֮������߸��ر�����ײ.
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


//! ������ر�����ײ���,�����������,����ݵ��ζ������λ�ý����Զ�λ�ô���.
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



	// �����ײ,���������ķ������˶����
	if( t_bSphereColl || t_bRayInter )
	{
		// �˱�������ȷ�ϵ�ǰ����Ƿ���Y���Ϸ������ƶ�
		bool       t_bInYAxisMoveStatus = false;
		
		// ���ݴ������λ�ã����ڼ����Զ���λʱ���Ӧ���뽹��ľ��롣
		osVec3D    t_vec3BackPos = m_curpos;
		

		// 
		// ATTENTION TO FIX:�˴����ܻ�����̧���ܸߵ�λ��,Ȼ�����
		do 
		{
			osVec3D   t_vec3Look;
			float     t_fDisToFocus,t_fStdFactor;



			get_godLookVec( t_vec3Look );
			t_fDisToFocus = osVec3Length( &(m_curpos-m_vec3CamFocus) );

			// �����λ���뽹��ľ���������ֵʱ���������̧
			t_fStdFactor = MINDIS_TOFOCUS;

			// 
			// ������ر��ཻ,��ӽ��㴦��ʼ�²��µĿɵ�������λ��.
			// �����ʱ�Ѿ������˶�ȥ�õ����õ����λ�ã�����Ҫ�ٴ���
			if( t_bRayInter && (!t_bInYAxisMoveStatus) )
			{
				m_curpos = t_vec3Intersect;
				// River @ 2006-7-14:���뱣֤�����λ�ò����ܵ�����ȥ
				if( t_fDisToFocus > t_sBs.radius )
					m_curpos += (t_vec3Look*t_sBs.radius);
				else
					// ��ʱ��������˶�������õ��˶�����
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
			// ʹ����ײsphere�İ뾶��Ϊ���ı�׼
			if( t_fDisToFocus < t_fStdFactor )
			{
				if( !t_bRayInter )
					m_curpos -= (t_vec3Look*0.1f);

				//! �����ת�Ĺ�ֱ��������ײ,����Ҫ��Զ���,�Է�ֹ������ѭ��.
				if( OSE_CAMPITCH_TOO_HIGH == rotate_vertical( m_vec3CamFocus,0.05f,true ) )
				{
					t_bInYAxisMoveStatus = true;
					move_close( 0.1f );
				}
			}

			update_camera( true,false );
			get_camNearClipBSphere( t_sBs );

			// ������������ر�����ײ�������λ�úͽ���֮������߸��ر�����ײ.
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

			// River mod @ 20079-17:ȷ�����λ�úͽ���֮�䲻�ܹ���ľ���
			if( t_fDisToFocus > m_fMaxDis )
				break;

		}while( t_bSphereColl || t_bRayInter );

		// River mod @ 2007-9-17:Ϊ�����������Զ�������ϲ�涨��������λ�úͽ������
		// ������ϵ����ܼ�ⲻ�ܴﵽҪ��,��������ת���,ֱ�����������ͷ��
		if( t_bSphereColl || t_bRayInter )
		{
			// River mod @ 2007-10-9:
			//pitch_camAboveFocus();

			// River mod @ 2007-10-13:
			pull_camPosToMaxDis();
		}


		// 
		// River @ 2007-2-9:��ĳ�ֳ����������£����ܻᴫ��ܴ�ľ��������
		// �����ڴ˴�����������룬���Ա�֤��������Զ���λ��Զ�������
		float   t_fResetDis = osVec3Length( &(t_vec3BackPos-m_vec3CamFocus) );
		if( t_fResetDis > m_fMaxDis )
			t_fResetDis = m_fMaxDis;

		// ��Ϊ�޸������,�����Զ���λ�Ĺ���
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
	// �õ�LerpVector.
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
*  ����һ�������Ƿ��������������
*/
bool osc_camera::is_ptInCamVF( osVec3D& _pos )
{
	guard;

	os_ViewFrustum*    m_frus;
    
	osassert( g_ptrCamera );

	//
	// Ŀǰ��⵱ǰ�Ķ����Ƿ��������ڡ�
	m_frus = g_ptrCamera->get_frustum();
	if( m_frus->objInVF( _pos ) )
		return true;
	else
		return false;

	unguard;
}



//! �������λ������������ͨ��;�������ر���ײʱ�����������͵ĵ�����λ��
bool osc_camera::animate_camRotPicth( float _rotRad,float _pitchRad,float _time )
{
	guard;

	// �����������״̬ʱ�������ٽ������������
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
	//m_bInRotPitchLerpState����������Ƿ���
	m_bInRotPitchLerpState = !_autoReset;
}


# if __EFFECT_EDITOR__

//! �ӳ����е���camera,����ʾ�����λ�ú���ת��������ݣ����ӿ��޹ء�
bool osc_camera::load_camera( const char* _cname,
							 bool _ani/* = false*/,float _time/* = 0.0f*/  )
{
	guard;
	int     t_iSize;
	DWORD   t_dwFileVersion;
	char    t_szFileHeader[4];
	BYTE*   t_ptrStart;

	osassert( (_cname)&&(_cname[0]) );

	// �鿴��ǰ���ļ��Ƿ����
	if( !file_exist( (char*)_cname ) )
		return false;

	// 
	// ����camera��ȫ�ֵĻ�������
	int   t_iGBufIdx = -1;
	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)_cname,t_ptrStart,TMP_BUFSIZE );
	osassert( t_iSize > 0 );

	// �����ļ�ͷ�Ͱ汾�š�
	READ_MEM_OFF( t_szFileHeader,t_ptrStart,sizeof( char )*4 );
	READ_MEM_OFF( &t_dwFileVersion,t_ptrStart,sizeof( DWORD ) );
	m_dwVersion = t_dwFileVersion;

	if( t_dwFileVersion == CAMFILE_VERSION )
	{
		// 
		// ���е�����У�������ֵ�������??????
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

		if( !_ani )//�޶���
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


//! �洢�����е�cameraλ�����ݡ�
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

	//�洢�޶�������,ֻ�洢��ͬ�ĵ�
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

	//���������Ч�ļ�(*.csf)
	int   t_iGBufIdx = -1;
	BYTE* p = START_USEGBUF( t_iGBufIdx );
	read_fileToBuf( _camEffectName,p,TMP_BUFSIZE );
	char header[4] = {0};
	READ_MEM_OFF( header,p,sizeof(char)*4);
	if( strcmp( header, "csf")!=0){
		osDebugOut( "camera: �����Ч�ļ�<%s>��ʽ����ȷ!\n", _camEffectName );
		return; 
	}
	int ver=0;
	READ_MEM_OFF( &ver,p,sizeof(DWORD));
	int vertexCount=0;
	READ_MEM_OFF( &vertexCount,p,sizeof(DWORD));
	END_USEGBUF( t_iGBufIdx );
	if(vertexCount<2)
	{
		osDebugOut( "camera: �����Ч�ļ�<%s>������!\n", _camEffectName );
		return;
	}


	m_camEffectPoint.clear();
	m_camEffectPoint.resize(vertexCount);
	memcpy(&m_camEffectPoint[0],p,sizeof(osVec3D)*vertexCount );
	if(m_camEffectPoint[0]!=m_camEffectPoint[vertexCount-1]){
		osDebugOut( "camera: �����Ч�ļ�<%s>��Ҫ�պϵ�����!\n", _camEffectName );
		return;
	}

	m_oldCamPos = m_curpos;
	m_oldCamFocus = m_vec3CamFocus;
	m_oldLookVec = m_curLookVec;
	//��������˶���Ч
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

	//�������仯,�����ʱ����Ҳ���˶�
	//osVec3D offset;
	m_offset = m_vec3CamFocus - m_oldCamFocus; 
	m_offset.y = 0.f;

	//���浱ǰ��λ��
	vFocus = m_oldCamFocus;


	m_curpos.x = m_oldCamPos.x + m_v3.x + m_offset.x;
	m_curpos.y = m_oldCamPos.y + m_v3.y + m_offset.y;
	m_curpos.z = m_oldCamPos.z + m_v3.z + m_offset.z;

	m_vec3CamFocus.x = vFocus.x + m_v3.x + m_offset.x;
	m_vec3CamFocus.y = vFocus.y + m_v3.y + m_offset.y;
	m_vec3CamFocus.z = vFocus.z + m_v3.z + m_offset.z;


	update_camera();

}


//! River @ 2009-4-9:������Զ������������ǰӦ�õ�pitchֵ
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
*  ���Ƹ�����Ϊһ�̶�����ֵ
*
*  \param _rad     ��ֱ������ӽ�.
*  \param _radMin  ��ƽ������ӽ�.
*  \param _minFDis ����뽹����Դﵽ��������룬�ڴ�λ���ϣ����ʹ����ƽ���ӽǡ�
*  \param _maxFDis ����뽹����Դﵽ����Զ���룬�ڴ�λ���ϣ����ʹ����ֱ���ӽǡ�
*/
void osc_camera::limit_pitch( float _rad,float _radMin,float _minFDis,float _maxFDis )
{
	guard;

	if( this->m_bShouldUpdate )
		update_camera();

	m_bLockPitch = true;

	if( _rad > 0.0f )
	{
		// ��ײ��pitchһֱ�Ǹ�ֵ,�о�һ��
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
 *  ���Ƹ�����Ϊһ�̶�ֵ
 *
 *  \param _rad �̶��ĸ����Ƕ�ֵ(����)
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
		// ��ײ��pitchһֱ�Ǹ�ֵ,�о�һ��
		m_fCurpitch = -_rad;
	}

	// River @ 2009-4-9:pitch���ֵ����Сֵʹ��ͬһ������
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
 *  ȡ�����Ƹ�����Ϊһ�̶�ֵ
 *
 *  \param _rad �̶��ĸ����Ƕ�ֵ(����)
 *  
 */
void osc_camera::unlimit_pitch( )
{
	m_bLockPitch = false;

}


//! ����autoResetState.
void osc_camera::save_autoResetState( void )
{
	m_sBackUpARM = m_sAutoResetMgr;
}

//! �ָ�autoResetState.
void osc_camera::restore_autoResetState( void )
{
	m_sAutoResetMgr = m_sBackUpARM;
}


//! �õ����viewFrustum��8������,����˸�����ֱ���: xyz,Xyz,xYz,XYz,xyZ,XyZ,xYZ,XYZ
const osVec3D* osc_camera::get_vfPt( void )
{
	os_ViewFrustum*  t_ptrVf;

	t_ptrVf = get_frustum();
	if( t_ptrVf )
		return t_ptrVf->m_FrustumVer;
	else
		return NULL;
}



//! ���ݵ�ǰ�����λ�úͷ�������е��������
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

//! ��������ı�������
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
*  ʹ����񶯵Ľӿ�
*  
*  \param float _fDamp		������.
*  \param float _swing      ��������.
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



//! �ƶ������һ��λ����,�ܿ��ϰ���,
//! �������Ĳ���ֵС���㣬�����ʹ�����δ��һ��ʱ�䲻�Զ���λ��
bool osc_camera::move_cameraToAvoidShelter( float _disToFocus )
{
	guard;

	//! ���ƶ������ָ����λ��....
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
			
			// ����һ��״̬�����rotֵû���޸ģ��򲻻ص�
			m_sAutoResetMgr.pause_autoReset( true );
		}
		else
# endif 
		{
			move_close( _disToFocus - t_fCurLength );
			//! �����Զ���λ�Ĺ���,���û�������ĵط����ƶ����,�������ʼ�Զ���λ.
			m_sAutoResetMgr.start_autoResetState( t_fCurLength,0 );
		}
	}

	//! ��һ֡�����Զ���λ����
	m_sAutoResetMgr.set_autoResetPass( false );

	return true;

	unguard;
}
