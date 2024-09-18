//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osPolygon.cpp
 *
 *   Desc:     O.S.O.K ����ĵĻ�����Ⱦ��λ,�м�ܵ���������Ⱦ�Ļ�����λ.
 *
 *   His:      River Created @ 4/30 2003
 *
 *  "The world is a fine place,and worth fighting for"��
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osPolygon.h"
# include "../include/osEffect.h"

os_polygon::os_polygon()
{
	m_iVerNum = 0;
	m_iVerSize = 0;
	m_iPriType = 0;
	m_iPriNum = 0;
	m_bUseStaticVB = true;
	m_iShaderId = -1;

	memset( &this->m_sBufData, 0, sizeof(m_sBufData) );

	// Ĭ���Լ����ͷŶ�����������ݡ�
	m_bSelfReleaseVI = false;
}

os_polygon::~os_polygon()
{
	if( m_bSelfReleaseVI && !m_bUseStaticVB )
	{
		SAFE_DELETE_ARRAY( m_sRawData.m_arrVerData );
		SAFE_DELETE_ARRAY( m_sRawData.m_arrIdxData );
	}
}

static int poly_cmp( const void* _a,const void* _b )
{
	const os_polygon*   t_a = (*(const os_polygon**)_a);
	const os_polygon*   t_b = (*(const os_polygon**)_b);

	int a_eff = g_shaderMgr->get_effectId( t_a->m_iShaderId );
	int b_eff = g_shaderMgr->get_effectId( t_b->m_iShaderId );

	if( a_eff < b_eff )
		return -1;
	else if( a_eff > b_eff )
		return 1;
	else // a_eff == b_eff
	{
		if( t_a->m_iShaderId < t_b->m_iShaderId )
			return -1;
		else if( t_a->m_iShaderId > t_b->m_iShaderId )
			return 1;
		else
			return 0;
	}
}

void os_polygonSet::sort_polygon( void )
{
	if( m_usedSize >= 2 )
		qsort( &m_polyList[0],this->m_usedSize, sizeof( const os_polygon* ), poly_cmp );
}


//--------------------------------------------------------------------


os_renderMeshData::os_renderMeshData()
{
	memset( this,0,sizeof( os_renderMeshData ) );	
	m_iVertexBufId = -1;
	m_iIndexBufId = -1;	
}


os_meshSet::os_meshSet( void )
{
# if QSORT_MESH
	m_iUsedSize = 0;
# endif 
	m_effectTechniqueLevel = 0;
}

os_meshSet::~os_meshSet( void )
{
}

// River @ 2006-1-19:��ǰ�Ļ���,�����Ҫ�������Ļ�,��Ƚ��鷳.
//                   �����Ҫ�������(�����Ȱ�Effect,�ٰ�����1,�ٰ�����2),
//                   ���Բ���ʹ��qsort
bool os_meshSet::init( int effectId,int verSize,int maxTexId,int meshNumPerTex )
{
	guard;

	osassert( effectId >= 0 );

	m_effectId = effectId;
	osc_effect* eff = g_shaderMgr->get_effectMgr()->get_effectfromid( m_effectId );
	m_effectTechniqueLevel = eff->m_curTechLevel;

# if (!QSORT_MESH)
	m_meshList.resize( maxTexId+1, static_set<const os_renderMeshData*>( meshNumPerTex, 0 ) );
	clear();
# endif 

	m_vertexSize = verSize;

	return true;

	unguard;
}

//! �����ڲ�Effect�Ľӿڣ���ʱ���ڶ�̬��Shader����Ⱦ
void os_meshSet::set_effectFloat( const char* _name,float _val )
{
	osc_effect*   t_ptrEffect = g_shaderMgr->get_effectMgr()->get_effectfromid( m_effectId );
	osassert( t_ptrEffect );
	t_ptrEffect->set_float( _name,_val );
}

void os_meshSet::set_effectValue( const char* _name,void* _vec4,int _size )
{
	osc_effect*   t_ptrEffect = g_shaderMgr->get_effectMgr()->get_effectfromid( m_effectId );
	osassert( t_ptrEffect );
	t_ptrEffect->set_value( _name,_vec4,_size );
}
void os_meshSet::set_effectVector( const char* _name,osVec4D* _vec4)
{
	osc_effect*   t_ptrEffect = g_shaderMgr->get_effectMgr()->get_effectfromid( m_effectId );
	osassert( t_ptrEffect );
	t_ptrEffect->m_peffect->SetVector(_name,_vec4);
}


bool os_meshSet::init( const char* fxFile, int verSize, int maxTexId, int meshNumPerTex )
{
	guard;

	m_vertexSize = verSize;
	m_effectId = g_shaderMgr->get_effectMgr()->dadd_effecttomanager((char*)fxFile);
	osc_effect* eff = g_shaderMgr->get_effectMgr()->get_effectfromid( m_effectId );
	m_effectTechniqueLevel = eff->m_curTechLevel;

	if( m_effectId == -1 )
	{
		osassert( false );
		return false;
	}

	bool   t_b = true;

# if QSORT_MESH
	m_vecRenderDataPtr.resize( 256 );
# else
	t_b = init( m_effectId, verSize, maxTexId, meshNumPerTex );
# endif 

	return t_b;

	unguard;
}

//! �Զ���ν���������㷨
static int mesh_cmp( const void* _a,const void* _b )
{
	const os_renderMeshData*   t_a = (*(const os_renderMeshData**)_a);
	const os_renderMeshData*   t_b = (*(const os_renderMeshData**)_b);

	// ��ʱֻ����������,�Ժ���Ը��ݼ�����ͬ����Դ���в�ͬ������
	if( t_a->m_wTexId[0] > t_b->m_wTexId[0] )
		return -1;
	else if( t_a->m_wTexId[0] < t_b->m_wTexId[0] )
		return 1;

	return 0;

}

# if QSORT_MESH
//! ��Ҫ��Ⱦ��meshָ���������
void os_meshSet::sort_mesh( void )
{
	qsort( &m_vecRenderDataPtr[0],this->m_iUsedSize,
		sizeof( const os_renderMeshData* ),mesh_cmp ); 
}
# endif 

