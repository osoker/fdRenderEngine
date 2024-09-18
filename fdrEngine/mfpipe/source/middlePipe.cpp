//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: middlePipe.cpp
 *
 *   Desc:     O.S.O.K ������м����,�м�ܵ����������е��豸��ص���Դ.
 *
 *   His:      River Created @ 4/29 2003
 *
 *  ����Ŀ�ɰܵĹؼ�����ӵ�����õ�׼������,�����м�ʶ����Ŀ�������˽�,
 *    ��Ŀ��ԱͶע���㹻�ľ�����׼������,�Լ��ٺ�������ķ�������
 * 
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/middlePipe.h"
# include "../include/deviceInit.h"
# include "../../interface/miskFunc.h"
# include "../../backpipe/include/osCamera.h"
# include "../include/osStateBlock.h"
# include "../include/osEffect.h"
# include "../include/osTexture.h" 
# include "../../fdrEngine/light.h"


//    
// ATTENTION TO OPP:
// Ŀǰÿһ��mesh��aniNodeʹ����һ�����㻺������Ӧ��
// ÿ�δ������㻺������ʱ�򣬴���һ���ڵĶ��㻺������
// �����䲻�����´���Ҫʹ��ʹ�ö��㻺��ʱ������ʹ��
// ��ǰ�Ķ��㻺������
# define  INIT_VBUFNUM     128
# define  INIT_IBUFNUM     (768*2)//syq *2


# define  DYNAMIC_IDXNUM   4096*64
# define  DYNAMIC_VBSIZE   4096*128


//! ȫ�ֹ����ɫֵ,��g_fAmbiR,g_fAmbiG,g_fAmbiB�еõ�
DWORD osc_middlePipe::m_dwAmbientColor = 0xffffffff;


//! ��̬���㻺�����Ŀ�ʼid.
# define  DYNAMICVB_IDSTART 5096

//! ATTENTION: ��ʱ�õ���ȫ�ֵ�view*proj Matrix.
OSENGINE_API osMatrix        g_matView;
OSENGINE_API osMatrix        g_matProj;
OSENGINE_API osVec3D		 g_vec3ViewPos;

osc_meshMgr*     g_ptrMeshMgr = NULL;;

void os_verticesBuf::reset_buf( void )
{
	if( m_bInuse )
		SAFE_RELEASE( m_verbuf );

	m_bInuse = false;
	m_iCapVerNum = 0;
	m_iVerNum = 0;
	m_iVerSize = 0;
	m_vbFlag = 0;
	m_bNewVB = false;


}


osc_middlePipe::osc_middlePipe()
{
	m_vecIdxBuf.resize( INIT_IBUFNUM );
	m_vecVerBuf.resize( INIT_VBUFNUM );
	m_vecDynamicVB.resize( INIT_VBUFNUM );

	m_pd3dDevice = NULL;

	m_dynamicIdxBuf = NULL;

	g_ptrMeshMgr = &m_meshMgr;
}


osc_middlePipe::~osc_middlePipe()
{
	// River @ 2011-2-23:�˳�ʱ�ͷ������塣
	//release_middelePipe();
}


//===========================================================================================
//
//  �м�ܵ���ʼ���׶εĹ���:
//
//===========================================================================================
/** \brief
*  ��ʼ���м�ܵ�.
*
*  �м�ܵ���Ҫ��ʼ���ڲ�����������Դģ��.
*  ��deviceMgr�ӿڵõ���ʼ���м�ܵ���Ҫ�����е���Ϣ.
*
*/
bool osc_middlePipe::init_middlePipe( I_deviceManager* _devmgr )
{
	guard;

	osc_d3dManager*  t_devmgr;

	osassert( _devmgr );

	t_devmgr = (osc_d3dManager*)_devmgr;

	this->m_pd3dDevice = t_devmgr->get_d3ddevice();

	m_shaderMgr.init_shaderMgr( m_pd3dDevice );
	m_frontRender.init_render( 0,m_pd3dDevice );
	

	// ��ʼ��meshMgr.
	m_meshMgr.init_meshMgr( m_pd3dDevice,this );

	//
	// ����Ⱦ�����¼�ƣ�
	record_frameSetRSB();

	if( !create_dynamicIB() )
		osassertex( false,"����ȫ�ֵĶ�̬���������ʧ��..\n" );

	return true;

	unguard;
}


/** \brief
*  �ͷ��м�ܵ�.
*
*  ��Ҫ�ͷŵ��м�ܵ���Դ����:
*  1: shaderMgr& frontRender. 
*  2: ��Ⱦ����εı���ṹ.
*  3: Vertex&Index Buffer.
* 
*/
void osc_middlePipe::release_middelePipe( void )
{
	guard;

	// �ͷ��豸�������.
	release_deviceDepData();

	if( m_iFrameSetRsId>=0 )
		os_stateBlockMgr::Instance()->release_stateBlock( m_iFrameSetRsId );
		

	// �ͷŶ�̬�Ķ��������������.
	this->release_dynamicIB();
	this->release_dynamicVB();

	unguard;
}


//===========================================================================================
//
//  �м�ܵ���������׶εĹ���:
//
//===========================================================================================


//! ���ݴ�������ݴ�С����һ���̶���С�Ķ��㻺������
int osc_middlePipe::create_staticVB( int _vsize,int _vnum,bool _sysmem )
{
	guard;

	int       t_i;
	D3DPOOL    t_flag;
	HRESULT    t_hr;

	t_i = m_vecVerBuf.get_freeNode();

	// 
	// Ŀǰֻʹ��managed��־.
	t_flag = D3DPOOL_MANAGED;

	t_hr = m_pd3dDevice->CreateVertexBuffer( 
		          _vsize*_vnum,D3DUSAGE_WRITEONLY,
				  0, t_flag ,&m_vecVerBuf[t_i].m_verbuf,NULL );

	if( FAILED( t_hr ) )
	{
		osassert( false );
		return -1;
	}

	// 
	// ���ö��㻺������һЩ���ԡ�
	m_vecVerBuf[t_i].m_bInuse = true;
	m_vecVerBuf[t_i].m_iVerSize = _vsize;
	m_vecVerBuf[t_i].m_iCapVerNum = _vnum;
	m_vecVerBuf[t_i].m_iVerNum = 0;
	m_vecVerBuf[t_i].m_vbFlag = 0;

	return t_i;

	unguard;
}



/** \brief
*  ����Vertexbuffer��id,���VBʹ�õ����ݡ�
*
*  \param _vstart ��䶥�㻺������ƫ��λ��.���Զ��DIPʹ��һ�����㻺����
*/
bool osc_middlePipe::fill_staticVB( int _vbIdx,
				os_vbufFillData* _vdata,int _num,int _vstart/* = 0*/  )
{
	guard;


	HRESULT     t_hr;
	BYTE*       t_vstart;
	int         t_iOffset = 0;
	int         t_iVFSize = 0;

	osassert( _vbIdx >= 0 );
	osassert( _vdata );


	if( _num<=0 )
		return false;

	if( 0 != _vstart )
	{
		t_iOffset = m_vecVerBuf[_vbIdx].m_iVerSize*_vstart;
		for( int t_i=0;t_i<_num;t_i ++ )
			t_iVFSize += _vdata[t_i].m_iVerNum * m_vecVerBuf[_vbIdx].m_iVerSize;
	}

	// 
	// ATTENTION TO OPP:�������ֶ��㻺��������������Ļ�������ʲô���𣿣���
	t_hr = m_vecVerBuf[_vbIdx].m_verbuf->Lock( t_iOffset,t_iVFSize,(void**)&t_vstart,0 );
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return false;
	}

	//t_vstart += (m_vecVerBuf[_vbIdx].m_iVerNum*m_vecVerBuf[_vbIdx].m_iVerSize);


	for( int t_i=0;t_i<_num;t_i ++ )
	{	
		memcpy( t_vstart,_vdata[t_i].m_ptrDstart,
			m_vecVerBuf[_vbIdx].m_iVerSize*_vdata[t_i].m_iVerNum );
		
		m_vecVerBuf[_vbIdx].m_iVerNum += _vdata[t_i].m_iVerNum;
		t_vstart += (m_vecVerBuf[_vbIdx].m_iVerSize*_vdata[t_i].m_iVerNum);

		int VerNum = m_vecVerBuf[_vbIdx].m_iVerNum;
		int intCapVerNum = m_vecVerBuf[_vbIdx].m_iCapVerNum;
		osassertex( m_vecVerBuf[_vbIdx].m_iVerNum <= 
			m_vecVerBuf[_vbIdx].m_iCapVerNum,
			va( "The ver num is:<%d,%d>..\n",m_vecVerBuf[_vbIdx].m_iVerNum,m_vecVerBuf[_vbIdx].m_iCapVerNum ) );
	}


	t_hr = m_vecVerBuf[_vbIdx].m_verbuf->Unlock();
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return false;
	}


	// 
	return true;

	unguard;
}



/** \brief
*  ���Ѿ�������static vb����������䣬�������������Ѿ�������static vb
*
*  
*/
bool osc_middlePipe::refill_staticVB( const void* _vdata,
							int _pvsize,int _vnum,int _id )
{
	guard;

	HRESULT     t_hr;
	BYTE*       t_vstart;

	// �����ǰ���㻺����û�����ã����ء�
	if( !m_vecVerBuf[_id].m_bInuse )
		return false;

	osassert( _vdata );
	osassert( m_vecVerBuf[_id].m_iVerSize == _pvsize );
	osassert( m_vecVerBuf[_id].m_iVerNum == _vnum );

	t_hr = m_vecVerBuf[_id].m_verbuf->Lock( 0,0,(void**)&t_vstart,0 );
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return false;
	}

	memcpy( t_vstart,_vdata,_pvsize*_vnum );
	t_hr = m_vecVerBuf[_id].m_verbuf->Unlock();
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return false;
	}

	return true;

	unguard;
}






/** \brief
*  Ϊ�ϲ㴴��static Index buffer.
*
*  ���е�IndexΪWORD����,��̬�Ķ���ʹ��һ��Index buffer.
*  Index data��Ҫ���ϲ㴦��,ʹ��Ҫ��һ����Ⱦ��Index&Vertex
*  �ڻ�������Ҳ�Ǽ�����һ���.
*
*/
int osc_middlePipe::create_staticIB( const void* _idata,int _inum )
{
	guard;


	HRESULT     t_hr;
	BYTE*       t_istart;

	osassert( _idata );
	osassert( _inum>0 );
	osassert( m_pd3dDevice );

	int t_i = m_vecIdxBuf.get_freeNode();

	t_hr = m_pd3dDevice->CreateIndexBuffer( 
		          sizeof( WORD )*_inum,D3DUSAGE_WRITEONLY,
				  D3DFMT_INDEX16, D3DPOOL_MANAGED,
				  &m_vecIdxBuf[t_i].m_idxbuf,NULL );
	if( FAILED( t_hr ) )
	{
		// ATTENTION TO FIX: �˴��ĳ���,���ܻ����ʲô�ط�?
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return -1;
	}

	t_hr = m_vecIdxBuf[t_i].m_idxbuf->Lock( 0,0,(void**)&t_istart,0 );
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return -1;
	}

	memcpy( t_istart,_idata,sizeof( WORD )*_inum );

	t_hr = m_vecIdxBuf[t_i].m_idxbuf->Unlock();
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return -1;
	}

	m_vecIdxBuf[t_i].m_bInuse = true;
	m_vecIdxBuf[t_i].m_iCapIdxNum = _inum;
	m_vecIdxBuf[t_i].m_iIdxNum = _inum;
	m_vecIdxBuf[t_i].m_ibFlag = 0;

	return t_i;

	unguard;
}

//! �ͷž�̬��IndexBuffer���ݡ�
void osc_middlePipe::release_staticIB( int _id )
{
	guard;

	osassert( (_id >= 0) );

	if( m_vecIdxBuf[_id].m_bInuse )
	{
		m_vecIdxBuf[_id].reset_buf();
		m_vecIdxBuf.release_node( _id );
	}

	unguard;
}

# ifdef _DEBUG
int osc_middlePipe::get_inuseSibNum( void )
{
	int t_num = 0;

	return t_num;
}
# endif 





//! ¼���м����ÿһ֡��Ҫ���õ���Ⱦ״̬�顣
void osc_middlePipe::record_frameSetRSB( void )
{
	guard;

	osassert( m_pd3dDevice );

	m_iFrameSetRsId = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iFrameSetRsId >= 0 );

	os_stateBlockMgr::Instance()->start_stateBlock();

	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW ); 
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );

	// ������õ���Ⱦ״̬��
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );

	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );

	set_ssRepeat();

	os_stateBlockMgr::Instance()->end_stateBlock( m_iFrameSetRsId );

	unguard;
}


/** \brief 
*  ÿһ֡����Ҫ�м�ܵ����õ����ú���,����һЩ���õĲ���.
*/
void osc_middlePipe::frame_setMiddlePipe( osc_camera* _cam )
{
	guard;// osc_middlePipe::frame_setMiddlePipe() );

	if( _cam )
	{
		_cam->get_viewmat( g_matView );
		_cam->get_projmat( g_matProj );
		_cam->get_curpos( &g_vec3ViewPos);
	}

	if( m_pd3dDevice )
	{
		if( (m_iFrameSetRsId >= 0)&&
			os_stateBlockMgr::Instance()->validate_stateBlockId( m_iFrameSetRsId ) )
		{
			os_stateBlockMgr::Instance()->apply_stateBlock( m_iFrameSetRsId );
		}
		else
		{
			this->record_frameSetRSB();
			os_stateBlockMgr::Instance()->apply_stateBlock( m_iFrameSetRsId );
		}
	}

	m_frontRender.frameset_render();
	m_meshMgr.frame_moveLru();

	//
	// �����Mixed Device,��˴�����ΪӲ������
	if( m_pd3dDevice )
	{
		osc_d3dManager*     t_ptrDevMgr;
		t_ptrDevMgr = (osc_d3dManager*)::get_deviceManagerPtr();
		if( t_ptrDevMgr->is_hardwareTL() )
			m_pd3dDevice->SetSoftwareVertexProcessing( FALSE );
		else
			m_pd3dDevice->SetSoftwareVertexProcessing( TRUE );
	}

	unguard;
}



/** \brief
*  Ϊ�ϲ��ṩ��䶯̬Index buffer�ĺ���.
*
*  \return int �����˶�̬����Index������buf�е�λ��.�������-1,����.
*
*/
int osc_middlePipe::fill_dynamicIB( const WORD* _idata,int _inum )
{
	guard;

	int           t_i;
	HRESULT       t_hr;
	DWORD         t_flag;
	void*         t_indexPtr;

	osassert( _idata );
	osassert( m_dynamicIdxBuf->m_bInuse );
	osassertex( _inum < m_dynamicIdxBuf->m_iCapIdxNum,"һ�����������������...\n" );

	t_i = m_dynamicIdxBuf->m_iIdxNum + _inum;
	if( t_i>m_dynamicIdxBuf->m_iCapIdxNum )
	{
		t_flag = D3DLOCK_DISCARD;
		m_dynamicIdxBuf->m_iIdxNum = 0;
	}
	else
		t_flag = D3DLOCK_NOOVERWRITE;


	//
	// Lock index buffer
	t_hr = m_dynamicIdxBuf->m_idxbuf->Lock( 
		m_dynamicIdxBuf->m_iIdxNum*sizeof( WORD ),
		_inum*sizeof( WORD ),&t_indexPtr,t_flag );
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return -1;
	}

	memcpy( t_indexPtr,_idata,sizeof( WORD )*_inum );

	m_dynamicIdxBuf->m_idxbuf->Unlock();
	m_dynamicIdxBuf->m_iIdxNum += _inum;

	return m_dynamicIdxBuf->m_iIdxNum-_inum;

	unguard;
}



/** \brief
*  Ϊ�ϲ㴴��dynamic Index buffer.
*
*  ���е�IndexΪWORD����,��̬�Ķ���ʹ��һ��Index buffer.
*  Index data��Ҫ���ϲ㴦��,ʹ��Ҫ��һ����Ⱦ��Index&Vertex
*  �ڻ�������Ҳ�Ǽ�����һ���.
*  ���ϲ��ṩ��䶯̬Indexbuf�ĺ���.
*  �м�ܵ�ֻ��Ҫһ����̬��Index buffer�Ϳ�����.
*/
bool osc_middlePipe::create_dynamicIB( void )
{
	guard;

	HRESULT    t_hr;

	//! һ��Ҫʹ��m_vecIdxBuf�е�һ��Ԫ��.
	if( !m_vecIdxBuf.get_nodeIndexAndUse( 
		osc_middlePipe::DYNAMIC_IB_ID, &m_dynamicIdxBuf ) )
	{
		osassert( false  );
	}

	t_hr = m_pd3dDevice->CreateIndexBuffer( DYNAMIC_IDXNUM*sizeof(WORD),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_dynamicIdxBuf->m_idxbuf,NULL );
		
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return false;
	}

	m_dynamicIdxBuf->m_iCapIdxNum = DYNAMIC_IDXNUM;
	m_dynamicIdxBuf->m_bInuse = true;
	m_dynamicIdxBuf->m_iIdxNum = 0;


	return true;

	unguard;
}

/** \brief
*  �ͷ�dynamic index buffer.
*
*  reset �м�ܵ����ݵ�ʱ����Ҫ�����������.
*/
bool osc_middlePipe::release_dynamicIB( void )
{
	guard;

	if( m_dynamicIdxBuf )
	{
		SAFE_RELEASE( m_dynamicIdxBuf->m_idxbuf );

		m_dynamicIdxBuf->m_iCapIdxNum = 0;
		m_dynamicIdxBuf->m_bInuse = false;
		m_dynamicIdxBuf->m_iIdxNum = 0;
	}

	// �ڹ��������ͷ�
	m_vecIdxBuf.release_node( osc_middlePipe::DYNAMIC_IB_ID );


	return true;

	unguard;
}

/** \brief 
*  Reset�м�ܵ�.��ǰdeviceMgr Reset��,�м�ܵ�Ӧ�õ����������.
*
*  ���ӵĴ�����,ÿ�ε�deviceManager��reset��,������������õ����������
*  ���ؽ����������е��豸�����Դ.
*
*/
bool osc_middlePipe::mdpipe_onResetDevice( void )
{

	m_shaderMgr.shaderMgr_onResetDevice( m_pd3dDevice );

	// ���´�����̬������������.
	if( !create_dynamicIB() )
		return false;

	// ���´�����̬�Ķ��㻺����.
	if( !reset_dynamicVB() )
		return false;

	osDebugOut( "�����豸Reset�ɹ�.....\n" );

	return true;

}

/** \brief
*  Ϊ�ϲ㴴��dynamic Vertex buffer,�����˴���dvb��������
*
*  \param _int _size Ҫ�����Ķ�̬���㻺�����Ĵ�С.�������0,��ʹ��ϵͳĬ�ϵĴ�С.
*/
int osc_middlePipe::create_dynamicVB( DWORD _fvf,int _size/* = 0*/,BOOL _newVB/* = false*/  )
{
	guard;

	HRESULT          t_hr;
	os_verticesBuf*   t_ptrVB;
	int              t_iVBSize;

	osassert( _size >= 0 );

	if( _size == 0 )
		t_iVBSize = DYNAMIC_VBSIZE;
	else
		t_iVBSize = _size;

	// 
	// ���Ŀǰ�Ķ�̬���㻺������������Ҫ��fvf���͡�
	if( !_newVB )
	{
		CSpaceMgr<os_verticesBuf>::NODE  t_ptrNode;
		for( t_ptrNode = m_vecDynamicVB.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = m_vecDynamicVB.next_validNode( &t_ptrNode ) )
		{
			if( !t_ptrNode.p->m_bInuse )
				continue;	
			if( (t_ptrNode.p->m_vbFlag == _fvf)&&
				(t_ptrNode.p->m_iVBSize >= t_iVBSize) )
			{
				if( !t_ptrNode.p->m_bNewVB )
					return t_ptrNode.idx + DYNAMICVB_IDSTART;
			}
		}
	}

	//
	// �����µĶ��㻺������
	int t_i = m_vecDynamicVB.get_freeNodeAUse( &t_ptrVB );
	t_hr = m_pd3dDevice->CreateVertexBuffer( t_iVBSize,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		_fvf,D3DPOOL_DEFAULT,&t_ptrVB->m_verbuf,NULL );
	if( FAILED( t_hr ) )
	{
		osassertex( false,va( "����:<%s>,�뵥����Ϸ�������������Ϸ����...\n",
			(char*)osn_mathFunc::get_errorStr( t_hr ) ) );
		return -1;
	}

	t_ptrVB->m_bNewVB = _newVB;
	t_ptrVB->m_vbFlag = _fvf;
	t_ptrVB->m_iVerNum = 0;
	t_ptrVB->m_iVBSize = t_iVBSize;
	t_ptrVB->m_iUsedSize = 0;
	t_ptrVB->m_bInuse = true;

	return t_i+DYNAMICVB_IDSTART;

	unguard;
}



/** \brief
*  �ͷ�dynamic vertex buffer.
*/
bool osc_middlePipe::release_dynamicVB(  bool _release/* = true*/  )
{
	guard;


	CSpaceMgr<os_verticesBuf>::NODE t_ptrNode;
	for( t_ptrNode=m_vecDynamicVB.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode=m_vecDynamicVB.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->m_bInuse );

		SAFE_RELEASE( t_ptrNode.p->m_verbuf );
		t_ptrNode.p->m_iUsedSize = 0;
		t_ptrNode.p->m_iVerNum = 0;
		t_ptrNode.p->m_verbuf = NULL;
		

		if( _release )
		{
			t_ptrNode.p->m_bInuse = false;	
			t_ptrNode.p->m_iCapVerNum = 0;
			t_ptrNode.p->m_iVerSize = 0;
			t_ptrNode.p->m_vbFlag = 0;

			m_vecDynamicVB.release_node( t_ptrNode.idx );
		}
	}

	return true;

	unguard;
}

//! device lost��Զ�̬�Ķ��㻺�������д���,���´�����Щ���㻺����
bool osc_middlePipe::reset_dynamicVB( void )
{
	guard;

	HRESULT           t_hr;

	CSpaceMgr<os_verticesBuf>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecDynamicVB.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDynamicVB.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->m_bInuse );
	
		// ���´�����̬�Ķ��㻺����.
		t_hr = m_pd3dDevice->CreateVertexBuffer( t_ptrNode.p->m_iVBSize,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			t_ptrNode.p->m_vbFlag,D3DPOOL_DEFAULT,&t_ptrNode.p->m_verbuf,NULL );

		if( FAILED( t_hr ) )
		{
			osassertex( false,"������̬�Ķ��㻺����ʧ��...\n" );
			return false;
		}
	}

	return true;

	unguard;
}

//@}



/** \brief
*  ʹ�ö�������������ǵĶ�̬���㻺������
*/
int osc_middlePipe::fill_dnamicVB( int _id,void* _ver,int _versize,int _vernum )
{
	guard;

	int             t_i,t_iStart;
	DWORD           t_flag;
	BYTE*           t_bufStart;
	HRESULT         t_hr;
	os_verticesBuf*  t_ptrVB;

	osassert( (_id-DYNAMICVB_IDSTART)>=0 );
	osassert( m_vecDynamicVB[_id-DYNAMICVB_IDSTART].m_bInuse );

	t_ptrVB = &m_vecDynamicVB[_id-DYNAMICVB_IDSTART];

	osassertex( _vernum*_versize <= (t_ptrVB->m_iVBSize),
		va( "һ������Ķ�̬�������,ver num<%d>,ver size<%d>..vb size<%d>..\n",
		   _vernum,_versize,t_ptrVB->m_iVBSize ) );

	// �鿴���㻺�����Ŀռ��Ƿ���.
	t_i = _versize*_vernum;

	if( t_i > t_ptrVB->m_iVBSize - t_ptrVB->m_iUsedSize )
	{
		t_flag = D3DLOCK_DISCARD;
		t_ptrVB->m_iUsedSize = 0;
	}
	else
		t_flag = D3DLOCK_NOOVERWRITE;


	// �������㻺������
	osassert( t_ptrVB->m_verbuf );
	t_hr = t_ptrVB->m_verbuf->Lock( t_ptrVB->m_iUsedSize,
		                 t_i,(void**)&t_bufStart,t_flag );
	if( FAILED( t_hr ) )
	{
		//! �������ĵ�����Ϣ
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		return -1;
	}

	t_iStart = t_ptrVB->m_iUsedSize;

	
	//! ֤ʵdx��bug,�Լ���һЩ����.
	if( !IsBadWritePtr( t_bufStart,_vernum*_versize ) )
	{
		// ��ÿһ������ε����ݸ��Ƶ����㻺����.
		memcpy( t_bufStart,_ver,_vernum*_versize );
		t_ptrVB->m_iUsedSize += (_vernum*_versize);
	}
	else
	{
		//! River mod @ 2009-12-16:�����ʱ���豸��ʧ����
		//  ������Ӧ���߼��󣬽���d3d����d3d��֤dip����
		//  ʧ�ܡ�
		t_ptrVB->m_iUsedSize += (_vernum*_versize);
		t_ptrVB->m_verbuf->Unlock();

		//@{
		//! TEST CODE: 
		HRESULT   t_hrTest = m_pd3dDevice->TestCooperativeLevel();
		if( t_hrTest == D3D_OK )
		{
			osDebugOut( "D3D_OK...\n" );
			osassertex( false,"D3D_OK...Lockʧ��...\n" );
		}
		if( t_hrTest == D3DERR_DRIVERINTERNALERROR )
		{
			osDebugOut( "D3DERR_DRIVERINTERNALERROR...\n" );
			osassertex( false,"D3DERR_DRIVERINTERNALERROR...Lockʧ��...\n" );
		}

		// �ȴ�ֱ������reset�豸
		while( t_hrTest != D3DERR_DEVICENOTRESET )
		{
			::Sleep( 30 );
			t_hrTest = m_pd3dDevice->TestCooperativeLevel();
		}

		// �����豸��Ȼ��������õ�ǰ�ĺ���
		osc_d3dManager* t_ptrMgr = (osc_d3dManager*)::get_deviceManagerPtr();
		if( FAILED( t_ptrMgr->reset_3denv() ) )
			osassertex( false,"��䶥�㻺����ʱ�豸��ʧ���ָ�����....\n" );

		// 
		return fill_dnamicVB( _id,_ver,_versize,_vernum  );

	}

	// Unlock the vertex buffer.
	t_hr = t_ptrVB->m_verbuf->Unlock();
	if( FAILED( t_hr ) )
	{
		osassert( false );
		return -1;
	}

	t_iStart /= _versize;

	osassert( t_iStart>=0 );

	return t_iStart;

	unguard;
}


/** \brief 
*  �ͷ��豸�������.
*
*  ���Ҫ����һ�����ĳ���,����Ҫ��������������м�ܵ��е��豸���
*  �����ͷŵ�.
*  ���ҪReset�豸,��Ҫ�ͷ��м�ܵ����е���Դ,�����´����豸��ص�
*  ���м�ܵ���Դ.
*
*/
void osc_middlePipe::release_deviceDepData( bool _finalRelease/* = true*/ )
{
	guard;

	// �ͷ��м�ܵ���mesh���ݺ�shader���ݡ�
	m_meshMgr.release_meshMgr( _finalRelease );
	m_shaderMgr.release_shaderMgr( _finalRelease );
	
	CSpaceMgr<os_indicesBuf>::NODE t_ptrIndiceNode;
	for( t_ptrIndiceNode=m_vecIdxBuf.begin_usedNode();
		t_ptrIndiceNode.p != NULL;
		t_ptrIndiceNode = m_vecIdxBuf.next_validNode( &t_ptrIndiceNode ) )
		release_staticIB( t_ptrIndiceNode.idx );

	CSpaceMgr<os_verticesBuf>::NODE t_ptrVerNode;
	for( t_ptrVerNode=m_vecVerBuf.begin_usedNode();
		t_ptrVerNode.p != NULL;
		t_ptrVerNode = m_vecVerBuf.next_validNode( &t_ptrVerNode ) )
		release_staticVB( t_ptrVerNode.idx );


	// �ͷŶ�̬����index buffer.
	release_dynamicIB();
	release_dynamicVB();


	//
	// ������������ͷų����������´�������
	if( !_finalRelease )
	{

		// ������̬����������.
		bool t_b = create_dynamicIB();
		osassert( t_b );

		// ������̬���㻺����.
		DWORD      t_fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2; 
		m_iTileVbIdx = create_dynamicVB( t_fvf );
		osassert( m_iTileVbIdx >= 0 );
	}

	return;

	unguard;
}



/** \brief
*  ʹ���м�ܵ���Ⱦһ�������,�⽫ʹ���м�ܵ�����һ��DIP����,������
*  ������ζѵ�һ��һ������������Ⱦ.
*/
bool osc_middlePipe::render_polygon( const os_polygon* _poly )
{
	guard;

	int   t_iPrinum;
	int   t_iVersize,t_iStartver,t_iVernum;
	DWORD t_dwPritype;
	bool  t_b;

	osc_effect*              t_eff;
	LPDIRECT3DVERTEXBUFFER9  t_verbuf;
	LPDIRECT3DINDEXBUFFER9   t_idxbuf;

	osassert( _poly );

	t_eff = m_shaderMgr.get_effect( _poly->m_iShaderId );
	if( !t_eff )
	{
		osassert( false);
		return false;
	}
	m_shaderMgr.set_EffectInfo( _poly->m_iShaderId );

	t_iVernum = _poly->m_iVerNum;
	t_iPrinum = _poly->m_iPriNum;
	t_iVersize = _poly->m_iVerSize;
	t_dwPritype = _poly->m_iPriType;

	// ʹ�ù̶��Ķ��㻺������index Buffer.
	if( _poly->m_bUseStaticVB )
	{
		int t_id = _poly->m_sBufData.m_iVbufId;
		t_iStartver = _poly->m_sBufData.m_iVertexStart;

		// ��̬VertexBuffer��id��
		if( t_id >= DYNAMICVB_IDSTART )
			t_verbuf = m_vecDynamicVB[t_id-DYNAMICVB_IDSTART].m_verbuf;
		else
			t_verbuf = m_vecVerBuf[t_id].m_verbuf;

		t_idxbuf = m_vecIdxBuf[_poly->m_sBufData.m_iIbufId].m_idxbuf;

		t_b = m_frontRender.prepare_andrender( t_verbuf,t_idxbuf,
			t_eff,t_dwPritype,t_iVersize,t_iStartver,0,t_iPrinum, t_iVernum,true );

		if( !t_b )
		{
			osassert( false );
			return false;
		}
	}
	else  // ʹ�����ݶ��㻺����.
	{
		t_b = m_frontRender.prepare_andrenderIndexedUp( _poly->m_sRawData.m_arrVerData,
			(const WORD*)_poly->m_sRawData.m_arrIdxData,t_eff,t_dwPritype,_poly->m_iVerSize,
			_poly->m_iPriNum,_poly->m_iVerNum );

		if( !t_b )
		{
			osassert( false );
			return false;
		}
	}

	return true;
	
	unguard;
}


bool osc_middlePipe::render_andClearPolygonSet( os_polygonSet*  _polySet )
{
	guard;

	osassert( _polySet );
	int                      t_id;
	int                      shaId = -1, perShaId = -1;
	int                      effId = -1, perEffId = -1;
	osc_effect              *eff_ptr = NULL;

	const os_polygon*        poly = 0;
	LPDIRECT3DVERTEXBUFFER9  t_verbuf;
	LPDIRECT3DINDEXBUFFER9   t_idxbuf;
	bool  t_b;

	UINT uPasses;

	_polySet->sort_polygon();

	for( int i = 0; i < _polySet->m_usedSize; ++i )
	{
		poly = _polySet->m_polyList[i];
		shaId   = poly->m_iShaderId;
		effId   = m_shaderMgr.get_effectId( shaId );

		if( shaId != perShaId )
		{
			if( effId != perEffId )
			{
				if( eff_ptr && (perEffId>=0) ) 
					eff_ptr->end();

				// ��¼�ϴ�ʹ�õ�EffectId.
				perEffId = effId;

				eff_ptr = g_shaderMgr->get_effect( shaId );
				osassert( eff_ptr );

                uPasses = eff_ptr->begin();
			}

			g_shaderMgr->set_EffectInfo( shaId );
			perShaId = shaId;
		}

		for( UINT j = 0; j < uPasses; ++j )
		{
			if( eff_ptr->pass( j ) )continue;

			if( poly->m_bUseStaticVB )
			{
				t_id = poly->m_sBufData.m_iVbufId;
				if( t_id >= DYNAMICVB_IDSTART )
					t_verbuf = m_vecDynamicVB[t_id-DYNAMICVB_IDSTART].m_verbuf;
				else
					t_verbuf = m_vecVerBuf[t_id].m_verbuf;

				t_idxbuf = m_vecIdxBuf[poly->m_sBufData.m_iIbufId].m_idxbuf;

				t_b = m_frontRender.render( t_verbuf,t_idxbuf, poly->m_iPriType,
					poly->m_iVerSize, poly->m_sBufData.m_iVertexStart, 0, poly->m_iPriNum, poly->m_iVerNum,true );
				if( !t_b )
				{
					osassert( false );
					if( eff_ptr ) 
						eff_ptr->end();
					_polySet->clear(); 
					return false;
				}
			}
			else
			{
				t_b = m_frontRender.prepare_andrenderIndexedUp( poly->m_sRawData.m_arrVerData,
					(const WORD*)poly->m_sRawData.m_arrIdxData, poly->m_iPriType, poly->m_iVerSize,
					poly->m_iPriNum, poly->m_iVerNum );
				if( !t_b )
				{
					osassert( false );
					if( eff_ptr )
						eff_ptr->end();
					_polySet->clear(); 

					return false;
				}
			}

			eff_ptr->endPass();
		}
	}

	if( eff_ptr )
		eff_ptr->end();
	_polySet->clear(); 

	return true;

	unguard;
}



# if QSORT_MESH

bool osc_middlePipe::render_andClearMeshSet( os_meshSet* meshSet )
{
	guard;

	bool  t_b;
	LPDIRECT3DVERTEXBUFFER9  t_verbuf = NULL;
	LPDIRECT3DINDEXBUFFER9   t_idxbuf = NULL;

	unsigned int      passes;

	const os_renderMeshData*   t_ptrRMesh;
	int                      t_iTexId = -1;

	// ����
	meshSet->sort_mesh();

	osc_effect* eff = m_shaderMgr.get_effectMgr()->get_effectfromid( meshSet->m_effectId );
	eff->m_curTechLevel = meshSet->m_effectTechniqueLevel;
	passes = eff->begin();

	int k;
	for( unsigned int uPass = 0; uPass < passes; ++uPass )
	{
		if( eff->pass( uPass ) )continue;

		//! ��ÿһ��RenderMeshData����DIP
		for( int t_i =0;t_i<meshSet->m_iUsedSize;t_i ++ )
		{
			t_ptrRMesh = meshSet->m_vecRenderDataPtr[t_i];

			if( (t_iTexId != t_ptrRMesh->m_wTexId[0])&&( eff->get_texCount()>0 ) )
			{
				t_iTexId = t_ptrRMesh->m_wTexId[0];

				// ��һ������
				eff->set_textureById( 0,t_iTexId );
			}

			// �ڶ�������������Ĵ���
			for( k = 1; k < eff->get_texCount(); ++k )
				eff->set_textureById( k, t_ptrRMesh->m_wTexId[ k ] );
			for( k = 0; k < eff->get_matCount(); ++k )
				eff->set_matrix( k, *( t_ptrRMesh->m_pTexMatrix[k]) );
			//@{
			// River @ 2010-4-3:�Ż�
			for( k = 0;k < eff->get_float2Count();k ++ )
			{
				if( t_ptrRMesh->m_pTexMatrix[k+eff->get_matCount()] )
				{
					osVec2D   t_vec2;
					t_vec2.x = t_ptrRMesh->m_pTexMatrix[k+eff->get_matCount()]->_11;
					t_vec2.y = t_ptrRMesh->m_pTexMatrix[k+eff->get_matCount()]->_22;
					eff->set_float2( k,(float*)&t_vec2 );
				}
			}
			//@}


			if( t_ptrRMesh->m_iVertexBufId >= DYNAMICVB_IDSTART )
			{
				t_verbuf = m_vecDynamicVB[ 
					t_ptrRMesh->m_iVertexBufId - DYNAMICVB_IDSTART ].m_verbuf;
			}
			else
				t_verbuf = m_vecVerBuf[ t_ptrRMesh->m_iVertexBufId ].m_verbuf;

			t_idxbuf = m_vecIdxBuf[ t_ptrRMesh->m_iIndexBufId ].m_idxbuf;

			
			// �жϵƹ�����Ժ����õ��Դ��HLSL����
			extern osc_camera*         g_ptrCamera;
			g_ptrLightMgr->EffectChunk(g_ptrCamera->get_camFocus(),28.0f,t_ptrRMesh->m_rectangle,eff->m_peffect);

			// River added @ 2009-3-24:effect�ڵĸ���
			eff->commit_change();

			t_b = m_frontRender.render( t_verbuf, t_idxbuf, 
										t_ptrRMesh->m_dwPriType, meshSet->m_vertexSize, 
										t_ptrRMesh->m_iVertexStart,
										t_ptrRMesh->m_iStartIndex, 
										t_ptrRMesh->m_iPrimitiveNum, 
										t_ptrRMesh->m_iVertexNum, true, t_ptrRMesh->m_iMinIdx );
			

		}

		eff->endPass();
	}

	eff->end();

	meshSet->clear();
	
	return true;

	unguard;
}

# else

bool osc_middlePipe::render_andClearMeshSet( os_meshSet* meshSet )
{
	guard;

	bool  t_b;
	LPDIRECT3DVERTEXBUFFER9  t_verbuf;
	LPDIRECT3DINDEXBUFFER9   t_idxbuf;

	unsigned int      passes;

	osc_effect* eff = m_shaderMgr.get_effectMgr()->get_effectfromid( meshSet->m_effectId );
	passes = eff->begin();

	static_intMap< static_set<const os_renderMeshData*> >* meshList = &( meshSet->m_meshList );
	static_set< const os_renderMeshData* >* v_ptr;
	const os_renderMeshData* mesh_ptr;

	int k;
	int tex0;
	for( unsigned int uPass = 0; uPass < passes; ++uPass )
	{
		if( eff->pass( uPass ) )continue;

		for( int i = 0; i < meshList->size(); ++i )
		{
			v_ptr = meshList->value_ptr( i, tex0 );
			if( tex0 < 0 )continue;
			if( eff->get_texCount() > 0 )
			{
				eff->set_textureById( 0, tex0-1 );
				mesh_ptr = ( *( v_ptr->value_ptr( 0 ) ) );
			}

			for( int j = 0; j < v_ptr->size(); ++j )
			{
				mesh_ptr = ( *( v_ptr->value_ptr( j ) ) );
				for( k = 1; k < eff->get_texCount(); ++k )
				{
					eff->set_textureById( k, mesh_ptr->m_wTexId[ k ] );
				}
				for( k = 0; k < eff->get_matCount(); ++k )
				{
					if( mesh_ptr->m_pTexMatrix[k] )
						eff->set_matrix( k, *(mesh_ptr->m_pTexMatrix[k]) );
				}
				//@{
				// River @ 2010-4-3:�Ż�
				for( k = 0;t_i< eff->get_float2Count();k ++ )
				{
					if( mesh_ptr->m_pTexMatrix[k+eff->get_matCount()] )
					{
						osVec2D   t_vec2;
						t_vec2.x = mesh_ptr->m_pTexMatrix[k+eff->get_matCount()]._11;
						t_vec2.y = mesh_ptr->m_pTexMatrix[k+eff->get_matCount()]._22;
						eff->set_float2( k,&t_vec2 );
					}
				}
				//@}

				if( mesh_ptr->m_iVertexBufId >= DYNAMICVB_IDSTART )
					t_verbuf = m_vecDynamicVB[ mesh_ptr->m_iVertexBufId - DYNAMICVB_IDSTART ].m_verbuf;
				else
					t_verbuf = m_vecVerBuf[ mesh_ptr->m_iVertexBufId ].m_verbuf;

				t_idxbuf = m_vecIdxBuf[ mesh_ptr->m_iIndexBufId ].m_idxbuf;
				
				t_b = m_frontRender.render( t_verbuf, t_idxbuf, mesh_ptr->m_dwPriType, meshSet->m_vertexSize, mesh_ptr->m_iVertexStart,
									mesh_ptr->m_iStartIndex, mesh_ptr->m_iPrimitiveNum, mesh_ptr->m_iVertexNum, true, mesh_ptr->m_iMinIdx );
				osassert( t_b );

				if( !t_b )
				{
					meshSet->clear();
					osassert( false );
					return false;
				}
			}
			v_ptr->clear();

			eff->endPass();
		}
	}

	eff->end();

	meshList->clear();

	return true;

	unguard;
}
# endif 


bool osc_middlePipe::render_meshData( int effectId, int verSize, const os_renderMeshData* mesh )
{
	guard;

	bool  t_b;
	LPDIRECT3DVERTEXBUFFER9  t_verbuf;
	LPDIRECT3DINDEXBUFFER9   t_idxbuf;

	unsigned int      passes;

	osc_effect* eff = m_shaderMgr.get_effectMgr()->get_effectfromid( effectId );
	passes = eff->begin();

	for( unsigned int uPass = 0; uPass < passes; ++uPass )
	{
		if( eff->pass( uPass ) )continue;

		for( int k = 0; k < eff->get_texCount(); ++k )
		{
			eff->set_textureById( k, mesh->m_wTexId[ k ] );
		}

		if( mesh->m_iVertexBufId >= DYNAMICVB_IDSTART )
			t_verbuf = m_vecDynamicVB[ mesh->m_iVertexBufId - DYNAMICVB_IDSTART ].m_verbuf;
		else
			t_verbuf = m_vecVerBuf[ mesh->m_iVertexBufId ].m_verbuf;

		t_idxbuf = m_vecIdxBuf[ mesh->m_iIndexBufId ].m_idxbuf;

		t_b = m_frontRender.render( t_verbuf, t_idxbuf, mesh->m_dwPriType, verSize, mesh->m_iVertexStart,
					mesh->m_iStartIndex, mesh->m_iPrimitiveNum, mesh->m_iVertexNum, true, mesh->m_iMinIdx );

		if( !t_b )
		{
			osassert( false );
			return false;
		}

		eff->endPass();
	}

	eff->end();

	return true;

	unguard;
}
