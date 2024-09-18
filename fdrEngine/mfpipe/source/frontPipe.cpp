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
# include "stdafx.h"
# include "../include/frontPipe.h"
# include "../include/osShader.h"
# include "../include/osEffect.h"

//  TEST CODE:在软件渲染的机器上可以速度更快？
//! 是否Effect之后记录当前的设备状态
# define BEGINPASS_FLAG   D3DXFX_DONOTSAVESTATE

osc_render*  g_frender = NULL;



D3DPRIMITIVETYPE osc_render::m_arrPrimitive[8] =
{ D3DPT_FORCE_DWORD,D3DPT_POINTLIST,D3DPT_LINELIST,D3DPT_LINESTRIP,
D3DPT_TRIANGLELIST,D3DPT_TRIANGLESTRIP,D3DPT_TRIANGLEFAN,D3DPT_FORCE_DWORD };


osc_render::osc_render()
{
	g_frender = this;
	m_idxBufLast = NULL;
	m_bufLast = NULL;
}


/// 初始化渲染器.
void osc_render::init_render( int _renlevel,LPDIRECT3DDEVICE9 _device )
{
	guard;

	osassert( _device );

	this->m_pd3dDevice = _device;
	this->m_iRenderLevel = _renlevel;
	
	this->frameset_render();
	g_shaderMgr->get_effectMgr()->set_techniqueLevel( m_iRenderLevel );

	unguard;
}



/** \brief
*  Real render function.
*
*  这个函数设置3d device 要使用的Vertex buffer&Indices buffer,调用DIP,使用
*  我们已经设置好的Effect进行渲染.
*/
bool osc_render::render( void )
{
	guard;

	HRESULT   hr;
	UINT      uPasses;
	bool      tmpb;

	// ATTENTION: 当出现奇怪的渲染问题，比如fvf跟vertexBuf的大小不一致等问题，
	//            一定要先查看这个函数内部是否正确设置了Vertexbuffer.
	tmpb = set_streamsource( 0,m_iVerSize,m_bufPointer );
	osassertex( tmpb,"Set stream source failed" );	

	if( m_bUseIndexbuf )
	{
		hr = m_pd3dDevice->SetIndices( m_IndexPointer );
		osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

		uPasses = m_effPointer->begin();
		for( UINT uPass = 0; uPass<uPasses; ++uPass )
		{
			// WORK START: ***BUG:不同的dx版本，对pass的处理不同????测试!!!
			if( m_effPointer->pass( uPass ) )continue;

			hr = m_pd3dDevice->DrawIndexedPrimitive( this->m_primitiveType,
				m_iStartVer,this->m_iMinindex,this->m_iVertexNum,
				this->m_iStartIndex,m_iPrimitiveNum );
			//! River @ 2010-10-9:更平缓的错误处理方式，直接不处理。
			//osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

			m_effPointer->endPass();
		}
		m_effPointer->end();

		//reset minindex as zero.
		m_iMinindex = 0;
	
	}
	else
	{
		uPasses = m_effPointer->begin();
		for(UINT uPass = 0; uPass < uPasses; ++uPass )
		{
			if( m_effPointer->pass( uPass ) )continue;

			hr = m_pd3dDevice->DrawPrimitive( this->m_primitiveType,
				this->m_iStartVer,this->m_iPrimitiveNum );	
			osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

			m_effPointer->endPass();
		}

		m_effPointer->end();
	}

	//  Set the render vertex and triangle num
	this->m_iRenderVerNum += this->m_iVertexNum;
	this->m_iRenderTriNum += this->m_iPrimitiveNum;

	return true;

	unguard;
}




/** \brief 
*  准备一大堆要渲染的资源.并且渲染这些资源.
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
*
*/
bool osc_render::prepare_andrender( LPDIRECT3DVERTEXBUFFER9 _pbuf,   
							   LPDIRECT3DINDEXBUFFER9 _pindex,
							   osc_effect* _peff,
							   DWORD _pri_type,
							   int   _versize,
							   int   _startver,
							   int   _startindex,
							   int   _pri_num,int _ver_num,
							   bool  _useindex,
							   int   _minIndex )
{
	guard;

	osassert( _pbuf );
	osassert( _peff );
	osassert( _pri_type );
	// River @ 2010-10-9:更缓和的处理方式，不渲染就可以了
	if( _useindex&&(!_pindex) )
		//osassert( false );
		return true;

	m_bufPointer = _pbuf;
	m_IndexPointer = _pindex;
	m_effPointer = _peff;
	m_iVerSize = _versize;
	m_iPrimitiveNum = _pri_num;
	m_bUseIndexbuf = _useindex;
	m_iVertexNum = _ver_num;
	m_iStartVer = _startver;
	m_iStartIndex = _startindex;
	m_iMinindex = _minIndex;


	//  得到d3d 格式的Primitive type.
	m_primitiveType = get_pritype_fromdword( _pri_type );

	//  Real render function.
	if( render() )
		return true;
	else
		return false;

	unguard;
}



/** \brief 
*  使用接口drawprimitiveup来渲染参数的资源.
*
*  \param _pstreamzerodata  要渲染的顶点的内存.
*  \param _peff             传入的EFFECT的指针.
*  \param _pri_type         要渲染的Primitve的type.
*  \param _versize          要渲染顶点的Size.
*  \param _pri_num          要渲染的Primitive的数目.
*  \param _ver_num          要渲染顶点的数目.
* 
*  
*/
bool osc_render::prepare_andrenderup( const void* _pstreamzerodata,  
								 osc_effect* _peff,
								 DWORD _pri_type,
								 int   _versize,
								 int   _pri_num,int _ver_num )
{
	guard;

	HRESULT   hr;
	UINT      uPasses;

	osassert( _pstreamzerodata );
	osassert( _peff );
	osassert( _pri_type );

	this->m_effPointer = _peff;
	this->m_iVerSize = _versize;
	this->m_iPrimitiveNum = _pri_num;
	this->m_iVertexNum = _ver_num;
	this->m_primitiveType = get_pritype_fromdword( _pri_type );

	uPasses = m_effPointer->begin();
	for(UINT uPass = 0; uPass < uPasses; uPass++)
	{
		if( m_effPointer->pass( uPass ) )continue;

		hr = m_pd3dDevice->DrawPrimitiveUP( m_primitiveType,
			this->m_iPrimitiveNum,_pstreamzerodata,this->m_iVerSize );
		
		if( FAILED( hr ) )
		{
			osDebugOut( DXGetErrorDescription9( hr ) );
			osassert( false );
			return false;
		}

		m_effPointer->endPass();
	}
	
	m_effPointer->end();

	//set the render vertex and triangle num
	this->m_iRenderVerNum += this->m_iVertexNum;
	this->m_iRenderTriNum += this->m_iPrimitiveNum;

	m_bufLast = NULL;

	return true;

	unguard;
}


/** \brief
*  用于调试的接口,真正的渲染不能这个函数.
*
*/
bool osc_render::prepare_andrenderIndexedUp( const void* _pstreamzerodata,
										const WORD* _pidxdata,
										osc_effect* _peff,
										DWORD _pri_type,
										int   _versize,
										int   _pri_num,int _ver_num )
{
	guard;

	HRESULT   hr;
	UINT      uPasses;

	osassert( _pstreamzerodata );
	osassert( _peff );
	osassert( _pri_type );

	this->m_effPointer = _peff;
	this->m_iVerSize = _versize;
	this->m_iPrimitiveNum = _pri_num;
	this->m_iVertexNum = _ver_num;
	this->m_primitiveType = get_pritype_fromdword( _pri_type );

	uPasses = m_effPointer->begin();
	for(UINT uPass = 0; uPass < uPasses; uPass++)
	{
		if( m_effPointer->pass( uPass ) )continue;

		/*
		// TEST CODE:
		osMatrix t_mat;
		m_pd3dDevice->GetTransform( D3DTS_WORLD,&t_mat );
		*/

		hr = m_pd3dDevice->DrawIndexedPrimitiveUP( m_primitiveType,0,
			m_iVertexNum,m_iPrimitiveNum,_pidxdata ,D3DFMT_INDEX16 ,
			_pstreamzerodata ,m_iVerSize  );
		
		if( FAILED( hr ) )
		{
			osDebugOut( DXGetErrorDescription9( hr ) );
			osassert( false );
			return false;
		}

		m_effPointer->endPass();
	}
	m_effPointer->end();

	this->m_iRenderVerNum += this->m_iVertexNum;
	this->m_iRenderTriNum += this->m_iPrimitiveNum;

	m_bufLast = NULL;
	

	return true;

	unguard;
}




/** \brief
*  准备渲染d3dxMesh的数据。
*  
*  前端渲染管道支持d3dxMesh的渲染，以后加入其它的数据，比如lod等。
*
*  \param _mesh    要渲染的mesh.
*  \param _subset  要渲染mesh的哪一个subset.
*  \param _peff    渲染mesh使用的effect.
*  
*/
bool osc_render::prepare_andrender( LPD3DXMESH  _mesh, int _subset, osc_effect* _peff )
{
	guard;


	HRESULT   hr;
	UINT      uPasses;

	osassert( _mesh );
	osassert( _peff );


	uPasses = _peff->begin();
	for(UINT uPass = 0; uPass < uPasses; uPass++)
	{
		if( _peff->pass( uPass ) )continue;
		hr = _mesh->DrawSubset( _subset );

		if( FAILED( hr ) )
		{
			osDebugOut( DXGetErrorDescription9( hr ) );
			
			//! river @ 2010-10-9:更加缓和的错误处理方式，直接返回。
			//osassert( false );
			_peff->endPass();
			_peff->end();

			return false;
		}

		_peff->endPass();
	}

	_peff->end();

	// 渲染dxMesh用到不同的顶点缓冲区，所以使当前vertexBuffer Catch失效。
	m_bufLast = NULL;
	m_idxBufLast = NULL;

	return true;

	unguard;
}


bool osc_render::render( LPD3DXMESH _mesh, int _subset )
{
	guard;

	HRESULT   hr;
	osassert( _mesh );	
	hr = _mesh->DrawSubset( _subset );
	if( FAILED( hr ) )
	{
		::Sleep( 5 );
		int t_k = 0;
		while( t_k<4 )
		{
			hr = _mesh->DrawSubset( _subset );
			if( FAILED( hr ) )
			{
				t_k ++;
				::Sleep( 5 );
			}
			else
			{
				m_bufLast = NULL;
				return true;
			}
		}
	
		osDebugOut( DXGetErrorDescription9( hr ) );
		osassertex( false,"请减少客户端开启数量，以降低错误发生率" );
		return false;
	}

	m_bufLast = NULL;
	return true;

	unguard;
}

/** 准备一大堆要渲染的资源.并且渲染这些资源，不在内部指定Effect指针，effct在外部设置
 */
bool osc_render::render( LPDIRECT3DVERTEXBUFFER9 _pbuf,   
		                        LPDIRECT3DINDEXBUFFER9 _pindex,
					   			DWORD _pri_type,
								int   _versize,
								int   _startver,
								int   _startindex,
								int   _pri_num,int _ver_num,
								bool  _useindex,
								int   _minIndex  /*= 0*/ )
{
	guard;

	HRESULT   hr;

	set_streamsource( 0,_versize,_pbuf );

	if( _useindex )
	{
		set_indexStream( _pindex );
		hr = m_pd3dDevice->DrawIndexedPrimitive(get_pritype_fromdword(_pri_type),
			_startver,_minIndex,_ver_num,_startindex,_pri_num );

		//@{
		// River added @ 2011-4-1:多次尝试，是否一直失败.
		if( FAILED( hr ) )
		{
			int t_iTimes = 0;
			while( t_iTimes < 4 )
			{
				::Sleep( 5 );
				hr = m_pd3dDevice->DrawIndexedPrimitive(get_pritype_fromdword(_pri_type),
					_startver,_minIndex,_ver_num,_startindex,_pri_num );
				if( FAILED( hr ) )
				{
					t_iTimes ++;
				}
				else
					return true;
			}
		}
		//@} 

		osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );
	}
	else
	{
		hr = m_pd3dDevice->DrawPrimitive( m_primitiveType,m_iStartVer,m_iPrimitiveNum );	
		osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );
	}

# ifdef _DEBUG
	m_iRenderVerNum += _ver_num;
	m_iRenderTriNum += _pri_num;
# endif 

	return true;

	unguard;
}


/// 不在里面指定effect，在外部设定
bool osc_render::prepare_andrenderIndexedUp( const void* _pstreamzerodata,
	                           const WORD* _pidxdata,
							   DWORD _pri_type,
							   int   _versize,
							   int   _pri_num,int _ver_num )
{
	guard;

	HRESULT   hr;

	osassert( _pstreamzerodata );
	osassert( _pri_type );

	this->m_iVerSize = _versize;
	this->m_iPrimitiveNum = _pri_num;
	this->m_iVertexNum = _ver_num;
	this->m_primitiveType = get_pritype_fromdword( _pri_type );

	hr = m_pd3dDevice->DrawIndexedPrimitiveUP( m_primitiveType,0,
		m_iVertexNum,m_iPrimitiveNum,_pidxdata ,D3DFMT_INDEX16 , _pstreamzerodata ,m_iVerSize  );

	osassertex( !FAILED( hr ),osn_mathFunc::get_errorStr( hr ) ) ;
	
	this->m_iRenderVerNum += this->m_iVertexNum;
	this->m_iRenderTriNum += this->m_iPrimitiveNum;

	m_bufLast = NULL;

	return true;

	unguard;
}
