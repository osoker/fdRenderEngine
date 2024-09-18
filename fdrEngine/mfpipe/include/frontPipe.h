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
# pragma   once

# include "../../interface/osInterface.h"

class osc_effect;

/** \brief
 *  OSOK �����ǰ����Ⱦ����,�����Ⱦ��ʹ����֯�õ�Device Dependent���ݽ�����Ⱦ.
 *
 *  ��������µĹ���:
 *  1������dx9 effect file's technique.
 *	   ��Ϊos Engine�ǻ���dx9.0 effect file�ģ�ÿһ��effect�����м�����Ⱦ����ÿһ�ּ�
 *	   �����һ��technique,�������������Ⱦ����Ҫ���豸״̬�����Ӳ��ˮƽ�ߵĻ�������ʹ
 *     �ø�������ͼ�͸����ӵ�shader��
 *	2������vertex buffer&index buffer.
 *	   ������ÿ����Ⱦ����Ҫ�������Ⱦ����Ҫ�����еĶ����index���buffer��Ҳ������
 *	   �̶���vertex buffer&index buffer���ڳ�ʼ��������ʱ�����Щ��õĶඥ�����ݴ���
 *	   �̶���vertex buffer&index buffer���������ﶥ�����ݡ�
 *	3����Ⱦ��ʹ��DrawPrimitiveUp&DrawIndexedPrimitive�ȼ�����Ľӿڡ�
 */
class osc_render
{
	friend class osc_middlePipe;

public:
	osc_render();

	/// ��ʼ����Ⱦ��.
	void         init_render( int _renlevel,LPDIRECT3DDEVICE9 _device );

	//================================================================================//
	//
	//----------------������Ϊ�ϲ��ṩ����õļ���render�ӿ�------------------------//
	//
	//================================================================================//

	/** ׼��һ���Ҫ��Ⱦ����Դ.������Ⱦ��Щ��Դ.
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

	/** ׼����Ⱦd3dxMesh������; �Ժ�������������ݣ�����lod�ȡ�
	 *
	 *  \param _mesh    Ҫ��Ⱦ��mesh.
	 *  \param _subset  Ҫ��Ⱦmesh����һ��subset.
	 *  \param _peff    ��Ⱦmeshʹ�õ�effect.
	 */
	bool         prepare_andrender( LPD3DXMESH  _mesh, int _subset, osc_effect* _peff );


	/** ʹ�ýӿ�drawprimitiveup����Ⱦ��������Դ.
	*
	*  \param _pstreamzerodata  Ҫ��Ⱦ�Ķ�����ڴ�.
	*  \param _peff             �����EFFECT��ָ��.
	*  \param _pri_type         Ҫ��Ⱦ��Primitve��type.
	*  \param _versize          Ҫ��Ⱦ�����Size.
	*  \param _pri_num          Ҫ��Ⱦ��Primitive����Ŀ.
	*  \param _ver_num          Ҫ��Ⱦ�������Ŀ.
	*/
	bool         prepare_andrenderup( const void* _pstreamzerodata,  
		                           osc_effect* _peff,
								   DWORD _pri_type,
								   int   _versize,
								   int   _pri_num,int _ver_num );

	/// ���ڵ��ԵĽӿ�,��������Ⱦ�����������
	bool         prepare_andrenderIndexedUp( const void* _pstreamzerodata,
		                           const WORD* _pidxdata,
		                           osc_effect* _peff,
								   DWORD _pri_type,
								   int   _versize,
								   int   _pri_num,int _ver_num );


	//---------------------------------------------------------

	/** ׼��һ���Ҫ��Ⱦ����Դ.������Ⱦ��Щ��Դ.
	*
	*  \param _pbuf       Ҫ��Ⱦ�Ķ��㻺����.
	*  \param _pindex     ��Ⱦʹ�õ�Index������. 
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

	//! ���õײ��DIP����,�ϲ����ú�һ�У���Ҫ��Mesh���Ż���Ⱦ
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

	/// ÿһ֡ʹ��Render֮ǰ�����������.����ÿһ֡�������õ�����
	void         frameset_render( void );

	/** \brief
	 *  ����vertex Buffer CatchΪ�ա�
	 *
	 *  ǰ��ܵ��ڲ���vbufCatch���ƣ����Լ�������Vertex stream�Ĵ�����
	 *  ����ĳЩ����£���Ȼbuf��ָ����ͬ����buf�����������Ѿ���ͬ������
	 *  ��ҪResetBufCatch,ʹ����Ⱦ�豸��������vertex buffer.
	 */
	void         reset_vbufCatch( void );

	//@{ 
	/**  \brief 
	 *   �õ���ǰ֡��Ⱦ�������κͶ������Ŀ.
	 */
	int          get_renderTrinum( void );
	int          get_renderVernum( void );
	//@}

	//! �õ���Ⱦ�豸��
	LPDIRECT3DDEVICE9   get_d3dDevice( void );   


	//@{ River @ 2006-3-30:���ϲ����ö����������������Ϊ�˾�̬Mesh����Ⱦ���Ӹ�Ч
	/** \brief
	 *  ��Ϊprepare_andrender�Ĳ��亯�������ó���һ����vertex buffer.
	 *
	 *  ���Ҫ���õڶ������㻺����,��Ҫ����������������ϲ�ʹ���������..
	 */
	bool         set_streamsource( int _index,int _versize,
	                        LPDIRECT3DVERTEXBUFFER9 _verbuf ); 

	//! ����Index������.
	BOOL         set_indexStream( LPDIRECT3DINDEXBUFFER9 _pindex );
	//@} 

private:
	/// ��dword�еõ���Ⱦ3d primitive��type.
	D3DPRIMITIVETYPE          get_pritype_fromdword( DWORD _type );



	//! Real render function.
	bool                      render( void ); 


private:
	//! ��ǰ����Ⱦ�ȼ�.
	int                       m_iRenderLevel;

	//! ��ǰ֡��Ⱦ�����ε���Ŀ.
	int                       m_iRenderTriNum;
	//! ��ǰ֡��Ⱦ�������Ŀ.
	int                       m_iRenderVerNum;

	//! 3D�豸ָ�롣
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

	//! ��һ����Ⱦ����ʹ�õ�Vertex Buffer.
	LPDIRECT3DVERTEXBUFFER9   m_bufLast;
	//! ��һ����Ⱦ����ʹ�õ�IndexBuffer
	LPDIRECT3DINDEXBUFFER9    m_idxBufLast;

	//! The vertex buffer pointer.
	LPDIRECT3DVERTEXBUFFER9   m_bufPointer;                        
	//! Index buffer pointer.
	LPDIRECT3DINDEXBUFFER9    m_IndexPointer;                     
	//! Effect file pointer,control how to render these vertex.
	osc_effect*               m_effPointer;     

	//! ʹ����������õ�������Ⱦ��Ҫ����vertex buf�ж���Ŀ�ʼλ��.
	int                       m_iMinindex;

	static D3DPRIMITIVETYPE   m_arrPrimitive[8];
};

/// ȫ�ֵ�ǰ�˹ܵ���ָ��
extern osc_render*  g_frender;



//==========================================================================
//==========================================================================
//
//                     Front render �� Inline ����.
//
//==========================================================================
//==========================================================================

/// ÿһ֡ʹ��Render֮ǰ�����������.����ÿһ֡�������õ�����.
inline void osc_render::frameset_render( void )
{

	m_bufLast = NULL;
	m_idxBufLast = NULL;
	this->m_iRenderVerNum = 0;
	this->m_iRenderTriNum = 0;

}
//! ����vertex Buffer CatchΪ��.
inline void osc_render::reset_vbufCatch( void )
{
	m_bufLast = NULL;
	m_idxBufLast = NULL;
	m_pd3dDevice->SetStreamSource( 0,NULL,0,0 );
	m_pd3dDevice->SetIndices( NULL );
}


/// �õ���ǰ֡��Ⱦ�������κͶ������Ŀ.
inline int osc_render::get_renderTrinum( void )
{
	return m_iRenderTriNum;
}
inline int osc_render::get_renderVernum( void )
{
	return m_iRenderVerNum;
}


//! �õ���Ⱦ�豸��
inline LPDIRECT3DDEVICE9 osc_render::get_d3dDevice( void )
{
	return m_pd3dDevice;
}

/** ��Ϊprepare_andrender�Ĳ��亯�������ó���һ����vertex buffer.
 *
 * ���Ҫ���õڶ������㻺����,����Ҫʹ���������..
 */
inline bool osc_render::set_streamsource( int _index,int _versize, LPDIRECT3DVERTEXBUFFER9 _verbuf )
{
	HRESULT t_hr;

	osassert( _index == 0 );
	osassert( _versize > 0 );
	osassert( _verbuf );

	//  
	//! River @ 2009-9-22:ȥ������Ż����ܶ�ĵط�������Ϊ���Ż������Ѿ�
	//                    �Ƕ�γ����ˡ�
	/* 
	// �Լ�����״̬��ʹ��d3d��С��״̬�ı�.
	if( _verbuf == m_bufLast )
		return true;
	else
		m_bufLast = _verbuf;
	*/
	t_hr = m_pd3dDevice->SetStreamSource( _index,_verbuf,0,_versize );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr) );

	return true;
}

//! ����Index������.
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

/// ��dword�еõ���Ⱦ3d primitive��type, ��Щdword  type��sgdefine.h�ж���.
inline D3DPRIMITIVETYPE osc_render::get_pritype_fromdword( DWORD _type )
{
	osassertex( (_type>0)&&(_type<7),"Error primitive type...\n" );
	return m_arrPrimitive[_type];
}


//! ���õײ��DIP����,�ϲ����ú�һ�У���Ҫ��Mesh���Ż���Ⱦ
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
		va( "����DIP ʧ��<%s>...\n",osn_mathFunc::get_errorStr( t_hr ) ) );
	return ;
}
