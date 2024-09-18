//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: frontPipe.h
 *
 *   Desc:     O.S.O.K �����ǰ�˹ܵ�,ֻ�账���㻺������Effect��Ϻ������Ⱦ.
 *
 *   His:      River Created @ 4/25 2003
 *
 *   "��Զ��Ҫ��ס'����'����ɱ�,��Ϊ�ϵ۲���������������һ����"
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/frontPipe.h"
# include "../include/osShader.h"
# include "../include/osEffect.h"

//  TEST CODE:�������Ⱦ�Ļ����Ͽ����ٶȸ��죿
//! �Ƿ�Effect֮���¼��ǰ���豸״̬
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


/// ��ʼ����Ⱦ��.
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
*  �����������3d device Ҫʹ�õ�Vertex buffer&Indices buffer,����DIP,ʹ��
*  �����Ѿ����úõ�Effect������Ⱦ.
*/
bool osc_render::render( void )
{
	guard;

	HRESULT   hr;
	UINT      uPasses;
	bool      tmpb;

	// ATTENTION: ��������ֵ���Ⱦ���⣬����fvf��vertexBuf�Ĵ�С��һ�µ����⣬
	//            һ��Ҫ�Ȳ鿴��������ڲ��Ƿ���ȷ������Vertexbuffer.
	tmpb = set_streamsource( 0,m_iVerSize,m_bufPointer );
	osassertex( tmpb,"Set stream source failed" );	

	if( m_bUseIndexbuf )
	{
		hr = m_pd3dDevice->SetIndices( m_IndexPointer );
		osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

		uPasses = m_effPointer->begin();
		for( UINT uPass = 0; uPass<uPasses; ++uPass )
		{
			// WORK START: ***BUG:��ͬ��dx�汾����pass�Ĵ���ͬ????����!!!
			if( m_effPointer->pass( uPass ) )continue;

			hr = m_pd3dDevice->DrawIndexedPrimitive( this->m_primitiveType,
				m_iStartVer,this->m_iMinindex,this->m_iVertexNum,
				this->m_iStartIndex,m_iPrimitiveNum );
			//! River @ 2010-10-9:��ƽ���Ĵ�����ʽ��ֱ�Ӳ�����
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
*  ׼��һ���Ҫ��Ⱦ����Դ.������Ⱦ��Щ��Դ.
*
*  ����������Effect's Technique, ��������Ⱦ����Ĳ�������.
*
*  \param _pbuf       Ҫ��Ⱦ�Ķ��㻺����.
*  \param _pindex     ��Ⱦʹ�õ�Index������. 
*  \param _peff       ��Ⱦʹ�õ�Effect��ָ��.
*  \param _pri_type   �����Primitive�ĸ�ʽ.
*  \param _versize    ���㻺�����ж����size.
*  \param _startver   �����ڶ��㻺�����еĿ�ʼλ��.
*  \param _startindex Indices��Indices�������еĿ�ʼλ��.
*  \param _pri_num    Ҫ��Ⱦ��Primitive����Ŀ.
*  \param _ver_num    ��Ⱦ��ʹ�ö������Ŀ.
*  \param _useindex   �����Ⱦ�ǲ���ʹ��Index buffer.
*  \param _minIndex   ���е�Indexָ��Ķ���������,���λ�Ķ�����
*                     VertexBuffer�е�λ��,���������_ver_numһ��
*                     ΪSOFTWARE_VERTEX_PROCESSINGָ��Ҫ������Ķ���
*                     ���ݿ�.
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
	// River @ 2010-10-9:�����͵Ĵ���ʽ������Ⱦ�Ϳ�����
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


	//  �õ�d3d ��ʽ��Primitive type.
	m_primitiveType = get_pritype_fromdword( _pri_type );

	//  Real render function.
	if( render() )
		return true;
	else
		return false;

	unguard;
}



/** \brief 
*  ʹ�ýӿ�drawprimitiveup����Ⱦ��������Դ.
*
*  \param _pstreamzerodata  Ҫ��Ⱦ�Ķ�����ڴ�.
*  \param _peff             �����EFFECT��ָ��.
*  \param _pri_type         Ҫ��Ⱦ��Primitve��type.
*  \param _versize          Ҫ��Ⱦ�����Size.
*  \param _pri_num          Ҫ��Ⱦ��Primitive����Ŀ.
*  \param _ver_num          Ҫ��Ⱦ�������Ŀ.
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
*  ���ڵ��ԵĽӿ�,��������Ⱦ�����������.
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
*  ׼����Ⱦd3dxMesh�����ݡ�
*  
*  ǰ����Ⱦ�ܵ�֧��d3dxMesh����Ⱦ���Ժ�������������ݣ�����lod�ȡ�
*
*  \param _mesh    Ҫ��Ⱦ��mesh.
*  \param _subset  Ҫ��Ⱦmesh����һ��subset.
*  \param _peff    ��Ⱦmeshʹ�õ�effect.
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
			
			//! river @ 2010-10-9:���ӻ��͵Ĵ�����ʽ��ֱ�ӷ��ء�
			//osassert( false );
			_peff->endPass();
			_peff->end();

			return false;
		}

		_peff->endPass();
	}

	_peff->end();

	// ��ȾdxMesh�õ���ͬ�Ķ��㻺����������ʹ��ǰvertexBuffer CatchʧЧ��
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
		osassertex( false,"����ٿͻ��˿����������Խ��ʹ�������" );
		return false;
	}

	m_bufLast = NULL;
	return true;

	unguard;
}

/** ׼��һ���Ҫ��Ⱦ����Դ.������Ⱦ��Щ��Դ�������ڲ�ָ��Effectָ�룬effct���ⲿ����
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
		// River added @ 2011-4-1:��γ��ԣ��Ƿ�һֱʧ��.
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


/// ��������ָ��effect�����ⲿ�趨
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
