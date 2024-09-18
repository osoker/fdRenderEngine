//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osPolygon.h
 *
 *   Desc:     O.S.O.K ����ĵĻ�����Ⱦ��λ,�м�ܵ���������Ⱦ�Ļ�����λ.
 *
 *   His:      River Created @ 4/30 2003
 *
 *  "The world is a fine place,and worth fighting for"��
 */
//--------------------------------------------------------------------------------------------------------
# pragma   once

# include "../../interface/osInterface.h"
# include "osShader.h"
# include <algorithm>

//  
//  River @ 2009-9-17:
//! һ���ر�chunk�ڣ��������е�hardTile��Ŀ
# define  MAX_HARDTILE_PERCHUNK 3


/** \brief 
 *  O.S.O.K������Ⱦ�Ļ�����λ֮һ.
 *
 *  ������֯��Ⱦ���ݵ�O.S.O.K����Ļ�����λ,�����˶�������ݺ�shader����,
 *  �������л���Ҫ�����м�ܵ��������豸������ݵ���Ϣ.
 *
 *  ����polygon��ʹ�ö��㻺����.��һЩҪʹ�ö�̬�Ķ��㻺����.
 * 
 *  ATTENTION TO OPP:������ݽṹ���Ծ���
 */
struct os_polygon
{
public:
	os_polygon();
	~os_polygon();

	int   get_indexCount( void )const{ return osn_mathFunc::get_indexCount( m_iPriType, m_iPriNum ); }

	void  set_rawData( void* verData, void* indData );
	void  set_bufData( int vBufId, int iBufId, int vStart, int iStart, int miniIdx );


public:
	union			//!< Polygon��Ⱦ�ṹ�ڣ�vertex��index����
	{
		struct		//!< ���ʹ��ȫ�ֵĶ�������Ļ�,�Ͳ�ʹ�����������. 
		{
			void*     m_arrVerData;  //!< ����������Ϣ
			void*     m_arrIdxData;  //!< Index������Ϣ.
		} m_sRawData;

		struct
		{
			int       m_iVbufId;     //!< ���㻺������id
			int       m_iIbufId;     //!< ������������id

			int       m_iVertexStart;//!< ��ȫ�ֶ��㻺�����еĶ��㿪ʼλ��.
			int       m_iIbufStart;

			int       m_iMiniIdx;    //!< ����������Ⱦ����ֵ������ȷ�Ļ�������߲��ٵ�Ч��
		} m_sBufData;
	};

	//! ��ǰpoly�õ��Ķ������Ŀ.
	int        m_iVerNum;
	//! ��ǰpoly�õĶ���ĳ���.
	int        m_iVerSize;

	//! Primitive type.
	int        m_iPriType;
	//! Primitive number.
	int        m_iPriNum;

	int        m_iShaderId;

	//! �ǲ���ʹ�þ�̬��vertex �� index buffer.
	bool       m_bUseStaticVB;


	//! �Ƿ��Լ��ͷŶ�����������ݡ�
	bool       m_bSelfReleaseVI;
};


typedef std::vector< os_polygon* >  VEC_polyptr;
typedef std::vector< os_polygon >   VEC_polygon;


//----------------------------------------------------------

class os_polygonSet
{
	friend class osc_middlePipe;
public:
	os_polygonSet( void ):m_usedSize(0){}
	~os_polygonSet( void ){}

	void add_polygon( const os_polygon* poly );
	void clear( void );


private:
	//! ֻ��middlePipeʹ�ô˺���
	void sort_polygon( void );


	std::vector<const os_polygon*> m_polyList;
	int                            m_usedSize;
};

inline void os_polygonSet::add_polygon( const os_polygon* poly )
{
	if( m_usedSize == m_polyList.size() )
	{
		m_polyList.push_back( poly );
		++m_usedSize;
	}
	else
	{
		m_polyList[m_usedSize] = poly;
		++m_usedSize;
	}
}

inline void os_polygonSet::clear( void )
{
	m_usedSize = 0;
}

//----------------------------------------------------------

//! ���ݽṹ
# define MAX_TEXNUM  6

//! mesh����,����Ĵ�С��Effect�С�
struct os_renderMeshData
{
	//! �������ݡ����㻺����id�����м�ܵ������ID
	int       m_iVertexNum;
	int       m_iVertexBufId;
	int       m_iVertexStart;

	//! �������ݡ�����������id�����м�ܵ������ID
	int       m_iIndexBufId;
	int       m_iStartIndex;
	int       m_iMinIdx;

	//! Primitive���
	int       m_iPrimitiveNum;
	DWORD     m_dwPriType;

	//! ����ID�Ķ���,���ڲ�ʹ�õ�id,�����-1.
	int       m_wTexId[MAX_TEXNUM];

	const D3DXMATRIX*  m_pTexMatrix[MAX_TEXNUM];

	//! ��������������������(x:left y:bottom z:right w:top)
	osVec4D		m_rectangle;

public:
	os_renderMeshData();
};


//! �Ƿ�ʹ��qsort��Mesh��������
# define QSORT_MESH   1
typedef std::vector<const os_renderMeshData*> VEC_meshDataPtr;


class os_meshSet
{
	friend class osc_middlePipe;
public:
	os_meshSet( void );
	~os_meshSet( void );


	/** maxTexId��meshNumPerTex����ָ��һ����ֵ�����������ڴ棬
	 * �ڴ治����ʱ�ڲ����Զ������ڴ��С
	 */
	bool init( int effectId, int verSize, int maxTexId, int meshNumPerTex );
	bool init( const char* fxFile, int verSize, int maxTexId, int meshNumPerTex );


	void push( const os_renderMeshData* _mesh );

# if QSORT_MESH
	//! ��Ҫ��Ⱦ��meshָ���������
	void sort_mesh( void );
# endif 

	/// �����osc_middlePipe::render_andClearMeshSet������Ⱦ�����ⲿ�����ٵ���clear
	void clear( void );

	//! �����ڲ�Effect�Ľӿڣ���ʱ���ڶ�̬��Shader����Ⱦ
	void set_effectFloat( const char* _name,float _val );
	void set_effectValue( const char* _name,void* _vec4,int _size );
	void set_effectVector( const char* _name,osVec4D* _vec4);

public:
	int	                m_effectId;
	int					m_effectTechniqueLevel;
	int                 m_vertexSize;

# if QSORT_MESH
	VEC_meshDataPtr     m_vecRenderDataPtr;
	int                 m_iUsedSize;
# else
	static_intMap< static_set<const os_renderMeshData*> > m_meshList;
# endif 
};


//////////////////////////////////////////////////////////////////////////


inline void os_polygon::set_rawData( void* verData, void* indData )
{
	m_sRawData.m_arrVerData = verData;
	m_sRawData.m_arrIdxData = indData;
	m_bUseStaticVB = false;
}

inline void os_polygon::set_bufData( int vBufId, int iBufId, int vStart, int iStart, int miniIdx )
{
	m_sBufData.m_iVbufId = vBufId;
	m_sBufData.m_iIbufId = iBufId;
	m_sBufData.m_iVertexStart = vStart;
	m_sBufData.m_iIbufStart = iStart;
	m_sBufData.m_iMiniIdx = miniIdx;
	m_bUseStaticVB = true;
}

inline void os_meshSet::push( const os_renderMeshData* _mesh )
{
# if QSORT_MESH
	if( m_iUsedSize == m_vecRenderDataPtr.size() )
	{
		m_vecRenderDataPtr.push_back( _mesh );
		m_iUsedSize ++;
	}
	else
	{
		m_vecRenderDataPtr[m_iUsedSize] = _mesh;
		m_iUsedSize ++;
	}
# else
	osassert( _mesh->m_wTexId[0] >= -1 );
	m_meshList[ _mesh->m_wTexId[0]+1 ].push_up( _mesh );
# endif 
}



inline void os_meshSet::clear( void )
{
# if QSORT_MESH
	m_iUsedSize = 0;
# else
	int k;
	for( int i = 0; i < m_meshList.size(); ++i )
		m_meshList.value_ptr( i, k )->clear();
	m_meshList.clear();
# endif 
}

