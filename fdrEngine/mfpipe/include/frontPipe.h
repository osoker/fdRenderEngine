//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: frontPipe.h
 *
 *   Desc:     O.S.O.K 引擎的前端管道,只需处理顶点缓冲区和Effect结合后进行渲染.
 *
 *   His:      River Created @ 4/25 2003
 *
 *   "永远都要记住'机会'这个成本,因为上帝不可能老是青睐你一个人"
 */
//--------------------------------------------------------------------------------------------------------
# pragma   once

# include "../../interface/osInterface.h"

class osc_effect;

/** \brief
 *  OSOK 引擎的前端渲染器类,这个渲染器使用组织好的Device Dependent数据进行渲染.
 *
 *  它完成如下的功能:
 *  1：设置dx9 effect file's technique.
 *	   因为os Engine是基于dx9.0 effect file的，每一个effect可能有几种渲染级别，每一种级
 *	   别就是一个technique,它包含了这次渲染所需要的设备状态。如果硬件水平高的话，可以使
 *     用更多层的贴图和更复杂的shader。
 *	2：设置vertex buffer&index buffer.
 *	   可以是每次渲染都需要对这次渲染所需要的所有的顶点和index填充buffer。也可以是
 *	   固定的vertex buffer&index buffer，在初始化场景的时候把那些最常用的多顶点数据存入
 *	   固定的vertex buffer&index buffer，比如人物顶点数据。
 *	3：渲染。使用DrawPrimitiveUp&DrawIndexedPrimitive等几个最常的接口。
 */
class osc_render
{
	friend class osc_middlePipe;

public:
	osc_render();

	/// 初始化渲染器.
	void         init_render( int _renlevel,LPDIRECT3DDEVICE9 _device );

	//================================================================================//
	//
	//----------------以下是为上层提供的最常用的几个render接口------------------------//
	//
	//================================================================================//

	/** 准备一大堆要渲染的资源.并且渲染这些资源.
	*
	*  函数先设置Effect's Technique, 接下来渲染传入的参数数据.
	*
	*  \param _pbuf       要渲染的顶点缓冲区.
	*  \param _pindex     渲染使用的Index缓冲区. 
	*  \param _peff       渲染使用的Effect的指针.
	*  \param _pri_type   传入的Primitive的格式.
	*  \param _versize    顶点缓冲区中顶点的size.
	*  \param _startver   顶点在顶点缓冲区中的开始位置.
	*  \param _startindex Indices在Indices缓冲区中的开始位置.
	*  \param _pri_num    要渲染的Primitive的数目.
	*  \param _ver_num    渲染中使用顶点的数目.
	*  \param _useindex   这次渲染是不是使用Index buffer.
	*  \param _minIndex   所有的Index指向的顶点数据中,最底位的顶点在
	*                     VertexBuffer中的位置,这个参数和_ver_num一起
	*                     为SOFTWARE_VERTEX_PROCESSING指明要批处理的顶点
	*                     数据块.
	*/
	bool         prepare_andrender( LPDIRECT3DVERTEXBUFFER9 _pbuf,   
		                            LPDIRECT3DINDEXBUFFER9 _pindex,
		                            osc_effect* _peff,
					   			    DWORD _pri_type,
								    int   _versize,
								    int   _startver,
								    int   _startindex,
								    int   _pri_num,int _ver_num,
								    bool  _useindex,
								    int   _minIndex  = 0 );

	/** 准备渲染d3dxMesh的数据; 以后加入其它的数据，比如lod等。
	 *
	 *  \param _mesh    要渲染的mesh.
	 *  \param _subset  要渲染mesh的哪一个subset.
	 *  \param _peff    渲染mesh使用的effect.
	 */
	bool         prepare_andrender( LPD3DXMESH  _mesh, int _subset, osc_effect* _peff );


	/** 使用接口drawprimitiveup来渲染参数的资源.
	*
	*  \param _pstreamzerodata  要渲染的顶点的内存.
	*  \param _peff             传入的EFFECT的指针.
	*  \param _pri_type         要渲染的Primitve的type.
	*  \param _versize          要渲染顶点的Size.
	*  \param _pri_num          要渲染的Primitive的数目.
	*  \param _ver_num          要渲染顶点的数目.
	*/
	bool         prepare_andrenderup( const void* _pstreamzerodata,  
		                           osc_effect* _peff,
								   DWORD _pri_type,
								   int   _versize,
								   int   _pri_num,int _ver_num );

	/// 用于调试的接口,真正的渲染不能这个函数
	bool         prepare_andrenderIndexedUp( const void* _pstreamzerodata,
		                           const WORD* _pidxdata,
		                           osc_effect* _peff,
								   DWORD _pri_type,
								   int   _versize,
								   int   _pri_num,int _ver_num );


	//---------------------------------------------------------

	/** 准备一大堆要渲染的资源.并且渲染这些资源.
	*
	*  \param _pbuf       要渲染的顶点缓冲区.
	*  \param _pindex     渲染使用的Index缓冲区. 
	*  \param _pri_type   传入的Primitive的格式.
	*  \param _versize    顶点缓冲区中顶点的size.
	*  \param _startver   顶点在顶点缓冲区中的开始位置.
	*  \param _startindex Indices在Indices缓冲区中的开始位置.
	*  \param _pri_num    要渲染的Primitive的数目.
	*  \param _ver_num    渲染中使用顶点的数目.
	*  \param _useindex   这次渲染是不是使用Index buffer.
	*  \param _minIndex   所有的Index指向的顶点数据中,最底位的顶点在
	*                     VertexBuffer中的位置,这个参数和_ver_num一起
	*                     为SOFTWARE_VERTEX_PROCESSING指明要批处理的顶点
	*                     数据块.
	*/
	bool         render( LPDIRECT3DVERTEXBUFFER9 _pbuf,   
		                            LPDIRECT3DINDEXBUFFER9 _pindex,
					   			    DWORD _pri_type,
								    int   _versize,
								    int   _startver,
								    int   _startindex,
								    int   _pri_num,int _ver_num,
								    bool  _useindex,
								    int   _minIndex  = 0 );

	//! 调用底层的DIP函数,上层设置好一切，主要用Mesh的优化渲染
	void        render( D3DPRIMITIVETYPE _pritype,
		                int _BaseVertexIndex,
						DWORD _MinIndex,
						DWORD _NumVertices,
						DWORD _StartIndex,
						DWORD _PrimitiveCount );


	bool         render( LPD3DXMESH _mesh, int _subset );

	bool         prepare_andrenderIndexedUp( const void* _pstreamzerodata,
		                           const WORD* _pidxdata,
								   DWORD _pri_type,
								   int   _versize,
								   int   _pri_num,int _ver_num );


	//-----------------------------------------------------------

	/// 每一帧使用Render之前调用这个函数.设置每一帧必须设置的数据
	void         frameset_render( void );

	/** \brief
	 *  设置vertex Buffer Catch为空。
	 *
	 *  前凋管道内部的vbufCatch机制，可以减少设置Vertex stream的次数，
	 *  但在某些情况下，虽然buf的指针相同，但buf的其它机制已经不同，所以
	 *  需要ResetBufCatch,使用渲染设备重新设置vertex buffer.
	 */
	void         reset_vbufCatch( void );

	//@{ 
	/**  \brief 
	 *   得到当前帧渲染的三角形和顶点的数目.
	 */
	int          get_renderTrinum( void );
	int          get_renderVernum( void );
	//@}

	//! 得到渲染设备。
	LPDIRECT3DDEVICE9   get_d3dDevice( void );   


	//@{ River @ 2006-3-30:在上层设置顶点和索引缓冲区，为了静态Mesh的渲染更加高效
	/** \brief
	 *  做为prepare_andrender的补充函数来设置超过一个的vertex buffer.
	 *
	 *  如果要设置第二个顶点缓冲区,需要公开这个函数，由上层使用这个函数..
	 */
	bool         set_streamsource( int _index,int _versize,
	                        LPDIRECT3DVERTEXBUFFER9 _verbuf ); 

	//! 设置Index缓冲区.
	BOOL         set_indexStream( LPDIRECT3DINDEXBUFFER9 _pindex );
	//@} 

private:
	/// 从dword中得到渲染3d primitive的type.
	D3DPRIMITIVETYPE          get_pritype_fromdword( DWORD _type );



	//! Real render function.
	bool                      render( void ); 


private:
	//! 当前的渲染等级.
	int                       m_iRenderLevel;

	//! 当前帧渲染三角形的数目.
	int                       m_iRenderTriNum;
	//! 当前帧渲染顶点的数目.
	int                       m_iRenderVerNum;

	//! 3D设备指针。
	LPDIRECT3DDEVICE9         m_pd3dDevice;                       

	//! Which type the primitive will use.line,triangle fan?
	D3DPRIMITIVETYPE          m_primitiveType;

	//! Vertex num of the vertex buffer.
	int                       m_iVertexNum;                         
	//! Vertex stride.
	int                       m_iVerSize;                           
	//! Primitive num in index buffer .
	int                       m_iPrimitiveNum;                      
	//! Start vertex of vertex stream.
	int                       m_iStartVer;
	//! Start Index.
	int                       m_iStartIndex;
	//! Whether we use index buffer.
	bool                      m_bUseIndexbuf;                       

	//! 上一次渲染调用使用的Vertex Buffer.
	LPDIRECT3DVERTEXBUFFER9   m_bufLast;
	//! 上一次渲染调用使用的IndexBuffer
	LPDIRECT3DINDEXBUFFER9    m_idxBufLast;

	//! The vertex buffer pointer.
	LPDIRECT3DVERTEXBUFFER9   m_bufPointer;                        
	//! Index buffer pointer.
	LPDIRECT3DINDEXBUFFER9    m_IndexPointer;                     
	//! Effect file pointer,control how to render these vertex.
	osc_effect*               m_effPointer;     

	//! 使用这个变量得到本次渲染需要的在vertex buf中顶点的开始位置.
	int                       m_iMinindex;

	static D3DPRIMITIVETYPE   m_arrPrimitive[8];
};

/// 全局的前端管道的指针
extern osc_render*  g_frender;



//==========================================================================
//==========================================================================
//
//                     Front render 的 Inline 函数.
//
//==========================================================================
//==========================================================================

/// 每一帧使用Render之前调用这个函数.设置每一帧必须设置的数据.
inline void osc_render::frameset_render( void )
{

	m_bufLast = NULL;
	m_idxBufLast = NULL;
	this->m_iRenderVerNum = 0;
	this->m_iRenderTriNum = 0;

}
//! 设置vertex Buffer Catch为空.
inline void osc_render::reset_vbufCatch( void )
{
	m_bufLast = NULL;
	m_idxBufLast = NULL;
	m_pd3dDevice->SetStreamSource( 0,NULL,0,0 );
	m_pd3dDevice->SetIndices( NULL );
}


/// 得到当前帧渲染的三角形和顶点的数目.
inline int osc_render::get_renderTrinum( void )
{
	return m_iRenderTriNum;
}
inline int osc_render::get_renderVernum( void )
{
	return m_iRenderVerNum;
}


//! 得到渲染设备。
inline LPDIRECT3DDEVICE9 osc_render::get_d3dDevice( void )
{
	return m_pd3dDevice;
}

/** 做为prepare_andrender的补充函数来设置超过一个的vertex buffer.
 *
 * 如果要设置第二个顶点缓冲区,就需要使用这个函数..
 */
inline bool osc_render::set_streamsource( int _index,int _versize, LPDIRECT3DVERTEXBUFFER9 _verbuf )
{
	HRESULT t_hr;

	osassert( _index == 0 );
	osassert( _versize > 0 );
	osassert( _verbuf );

	//  
	//! River @ 2009-9-22:去掉这个优化，很多的地方可能因为这优化出错，已经
	//                    是多次出错了。
	/* 
	// 自己设置状态，使用d3d最小化状态改变.
	if( _verbuf == m_bufLast )
		return true;
	else
		m_bufLast = _verbuf;
	*/
	t_hr = m_pd3dDevice->SetStreamSource( _index,_verbuf,0,_versize );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr) );

	return true;
}

//! 设置Index缓冲区.
inline BOOL osc_render::set_indexStream( LPDIRECT3DINDEXBUFFER9 _pindex )
{
	osassert( _pindex );

	if( _pindex == m_idxBufLast )
		return TRUE;
	else
		m_idxBufLast = _pindex;

	HRESULT t_hr;

	t_hr = m_pd3dDevice->SetIndices( _pindex );
	osassertex( !FAILED(t_hr),osn_mathFunc::get_errorStr( t_hr ) );

	return TRUE;
}

/// 从dword中得到渲染3d primitive的type, 这些dword  type在sgdefine.h中定义.
inline D3DPRIMITIVETYPE osc_render::get_pritype_fromdword( DWORD _type )
{
	osassertex( (_type>0)&&(_type<7),"Error primitive type...\n" );
	return m_arrPrimitive[_type];
}


//! 调用底层的DIP函数,上层设置好一切，主要用Mesh的优化渲染
inline void osc_render::render( D3DPRIMITIVETYPE _pritype,
				   int _BaseVertexIndex,
				   DWORD _MinIndex,
				   DWORD _NumVertices,
				   DWORD _StartIndex,
				   DWORD _PrimitiveCount )
{
	HRESULT  t_hr = m_pd3dDevice->DrawIndexedPrimitive( _pritype,_BaseVertexIndex,
		_MinIndex,_NumVertices,_StartIndex,_PrimitiveCount );
	osassertex( !FAILED( t_hr ),
		va( "调用DIP 失败<%s>...\n",osn_mathFunc::get_errorStr( t_hr ) ) );
	return ;
}
