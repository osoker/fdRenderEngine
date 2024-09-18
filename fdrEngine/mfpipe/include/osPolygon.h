//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osPolygon.h
 *
 *   Desc:     O.S.O.K 引擎的的基础渲染单位,中间管道创建和渲染的基本单位.
 *
 *   His:      River Created @ 4/30 2003
 *
 *  "The world is a fine place,and worth fighting for"。
 */
//--------------------------------------------------------------------------------------------------------
# pragma   once

# include "../../interface/osInterface.h"
# include "osShader.h"
# include <algorithm>

//  
//  River @ 2009-9-17:
//! 一个地表chunk内，最多可以有的hardTile数目
# define  MAX_HARDTILE_PERCHUNK 3


/** \brief 
 *  O.S.O.K引擎渲染的基本单位之一.
 *
 *  用于组织渲染数据的O.S.O.K引擎的基本单位,包含了多边形数据和shader数据,
 *  在运行中还需要保存中间管道创建的设备相关数据的信息.
 *
 *  所有polygon都使用顶点缓冲区.有一些要使用动态的顶点缓冲区.
 * 
 *  ATTENTION TO OPP:这个数据结构可以精减
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
	union			//!< Polygon渲染结构内，vertex和index数据
	{
		struct		//!< 如果使用全局的顶点数组的话,就不使用这个数据了. 
		{
			void*     m_arrVerData;  //!< 顶点数据信息
			void*     m_arrIdxData;  //!< Index数据信息.
		} m_sRawData;

		struct
		{
			int       m_iVbufId;     //!< 顶点缓冲区的id
			int       m_iIbufId;     //!< 索引缓冲区的id

			int       m_iVertexStart;//!< 在全局顶点缓冲区中的顶点开始位置.
			int       m_iIbufStart;

			int       m_iMiniIdx;    //!< 如果是软件渲染，此值设置正确的话，能提高不少的效率
		} m_sBufData;
	};

	//! 当前poly用到的顶点的数目.
	int        m_iVerNum;
	//! 当前poly用的顶点的长度.
	int        m_iVerSize;

	//! Primitive type.
	int        m_iPriType;
	//! Primitive number.
	int        m_iPriNum;

	int        m_iShaderId;

	//! 是不是使用静态的vertex 和 index buffer.
	bool       m_bUseStaticVB;


	//! 是否自己释放顶点和索引数据。
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
	//! 只有middlePipe使用此函数
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

//! 数据结构
# define MAX_TEXNUM  6

//! mesh数据,顶点的大小在Effect中。
struct os_renderMeshData
{
	//! 顶点数据。顶点缓冲区id是由中间管道管理的ID
	int       m_iVertexNum;
	int       m_iVertexBufId;
	int       m_iVertexStart;

	//! 索引数据。索引缓冲区id是由中间管道管理的ID
	int       m_iIndexBufId;
	int       m_iStartIndex;
	int       m_iMinIdx;

	//! Primitive相关
	int       m_iPrimitiveNum;
	DWORD     m_dwPriType;

	//! 纹理ID的队列,对于不使用的id,则填充-1.
	int       m_wTexId[MAX_TEXNUM];

	const D3DXMATRIX*  m_pTexMatrix[MAX_TEXNUM];

	//! 纹理对象的世界坐标区域(x:left y:bottom z:right w:top)
	osVec4D		m_rectangle;

public:
	os_renderMeshData();
};


//! 是否使用qsort对Mesh进行排序
# define QSORT_MESH   1
typedef std::vector<const os_renderMeshData*> VEC_meshDataPtr;


class os_meshSet
{
	friend class osc_middlePipe;
public:
	os_meshSet( void );
	~os_meshSet( void );


	/** maxTexId、meshNumPerTex可以指定一个数值，用来分配内存，
	 * 内存不够用时内部会自动扩大内存大小
	 */
	bool init( int effectId, int verSize, int maxTexId, int meshNumPerTex );
	bool init( const char* fxFile, int verSize, int maxTexId, int meshNumPerTex );


	void push( const os_renderMeshData* _mesh );

# if QSORT_MESH
	//! 对要渲染的mesh指针进行排序
	void sort_mesh( void );
# endif 

	/// 如果用osc_middlePipe::render_andClearMeshSet进行渲染，在外部不需再调用clear
	void clear( void );

	//! 设置内部Effect的接口，暂时用于动态光Shader的渲染
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

