//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: middlePipe.h
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
#  pragma   once

#  include "../../interface/osInterface.h"
#  include "osShader.h"
#  include "frontPipe.h"
#  include "osPolygon.h"
#  include "osMesh.h"


class  osc_camera;
struct os_mirror;


//! ��䶥�㻺����������ָ�롣
struct os_vbufFillData
{
	char*   m_ptrDstart;
	int     m_iVerNum;

public:
	os_vbufFillData( void )
	{
		m_ptrDstart = NULL;
		m_iVerNum = 0;
	}
};

/// �м�ܵ�����vertex buffer�Ľṹ
struct os_verticesBuf
{
	//! 3d �豸�Ķ��㻺����.
	LPDIRECT3DVERTEXBUFFER9    m_verbuf;

	//! ��ǰ�Ķ��㻺�����ǲ����Ѿ�ʹ��.
	bool                       m_bInuse;

	//!  ���㻺������ÿ������ĳ���.
	int                        m_iVerSize;

	//!  ���㻺�����������ɶ������Ŀ.
	int                        m_iCapVerNum;

	//!  ���㻺�����ж������Ŀ.
	int                        m_iVerNum;

	//!  ���㻺�����ı�־λ.
	DWORD                      m_vbFlag;

	//!  ��ǰ���㻺�����Ѿ�ʹ�õĴ�С��
	int                        m_iUsedSize;

	//!  ��ǰ���㻺�����ܵĴ�С.
	int                        m_iVBSize;

	//!  ��ǰ�Ķ��㻺�����Ƿ��Ƕ����Ķ��㻺����
	bool                       m_bNewVB;

public:
	os_verticesBuf()
	{
		m_verbuf = NULL;
		m_bNewVB = false;
		reset_buf();
	}

	~os_verticesBuf()
	{
		reset_buf();
	}

	void reset_buf( void );
};

/// �м�ܵ�ʹ�õ�Indices�������Ľṹ.������ʱ�̶�ʹ��WORD Indices
struct os_indicesBuf
{
	//! 3d �豸�õ���Index buffer.
	LPDIRECT3DINDEXBUFFER9     m_idxbuf;

	//! ��ǰ�ṹ�ǲ����Ѿ�ʹ��.
	bool                       m_bInuse;

	//! ��ǰ�����������ɵ�Indices��Ŀ.
	int                        m_iCapIdxNum;

	//! ��ǰ�������е�Indices��Ŀ.
	int                        m_iIdxNum;

	//! ��ǰ�������ı�־λ.
	DWORD                      m_ibFlag;

public:
	os_indicesBuf()
	{
		m_idxbuf = NULL;
		m_bInuse = false;

		m_iCapIdxNum = 0;
		m_iIdxNum = 0;
		m_ibFlag = 0;

	}
	~os_indicesBuf()
	{
		reset_buf();
	}

	void reset_buf( void )
	{
		if( m_bInuse )
		{
			SAFE_RELEASE( m_idxbuf );
		
			m_idxbuf = NULL;
			m_bInuse = false;

			m_iCapIdxNum = 0;
			m_iIdxNum = 0;
			m_ibFlag = 0;
		}
	}

};

/** \brief 
 *  �豸��صĶ��㻺����ָ���Vector.
 */
typedef std::vector<os_verticesBuf>          VEC_verBuffer;
typedef std::vector<os_indicesBuf>           VEC_idxBuffer;


//*********************************************************************

/** \brief
 *  O.S.O.K������м�ܵ�.���������е��豸�����Դ,���ҰѺ�˹ܵ�����Դ
 *  ��Ⱦ��3D�豸.
 *
 *  �м����������Դ: 
 *  1: �������й�������Ҫ�����е�shader.
 *  2: �����˹ܵ������еĶ���&����������Դ.
 *  3: �����ǰ�˹ܵ�.
 *  4: ��ǰ֡������Ҫ��Ⱦ�Ķ����ָ���б�.
 *
 *  ������������ڲ���Դ������,�����ϲ�Ӧ�ó��򹫿�.
 *  �м�ܵ���һ���Ƚϸ��ӵĴ������,��ǰ�˹ܵ��������Ķ���ν��������,
 *  ���͵�ǰ�˹ܵ�������Ⱦ.
 *
 *  �м�ܵ���������:
 *      �м�ܵ���ʼ���׶�:
 *                  ��Ҫ��ʼ��ǰ�˹ܵ�,���Ұѳ�ʼ���м�ܵ��ڲ�����ĸ�����Դ��.
 *      ��������׶�:
 *                  �м�ܵ���Ҫ��ǰ�˹ܵ��͹������ڴ��������ݴ������豸��ص�����.
 *  				���Ұ��豸��ص�һЩ���ݷ�����Ҫ�����豸������ݵĶ����.�м�ܵ�
 *  				ֻ��Ҫ����.�����ʹ���豸�������,�豸������ݵ���Ϣ�����ṩ����
 *  				��Դ�ĵ�λ���м�¼.���糡���еĶ������Ҫ��¼��������������Shader
 *  				��ID.  
 *      ���н׶�: 
 *                  ���н׶��м�ܵ����Խ���ǰ�˹ܵ��͹����Ķ����ָ��.��ǰ�˹ܵ�Ҫ��Ⱦ
 *                  ��ʱ��,�м�ܵ����Ѿ��յ��Ķ���ΰ�shader����,����Ⱦ��Щ������Ķ�
 *                  ����.
 *                  �����豸��Ⱦ�豸��һЩ��״̬,��������ת��Matrix.
 *                  ���Եõ�����Ⱦ����ε���Ϣ.
 *                  ��̬�Ĵ���һЩ��Ⱦ��Դ.
 *                  ����һЩ�ײ��3d�豸��Ϣ,����shader constants.
 *                  Ӧ�ÿ���֧���豸Reset.......
 * 
 *                  ��������.......
 */
class osc_middlePipe
{
public:
	osc_middlePipe();
	~osc_middlePipe();


	//! ȫ�ֵĶ�̬������������ID.
	enum {
		DYNAMIC_IB_ID = 0,
	};

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
	 */
	bool                init_middlePipe( I_deviceManager* _devmgr );

	/// �ͷ��м�ܵ�
	void                release_middelePipe( void );


	//===========================================================================================
	//
	//          �м�ܵ���mesh��صĺ���,���ڵ��룬��Ⱦd3dxMesh.
	//
	//===========================================================================================
	/** \brief
	 *  ����aniMesh���ݵĶ�̬��
	 *
	 *  ��������������Ʒ�ϵĶ�̬���ǼٵĶ�̬�⣬����Ⱦ����Ʒ��ʱ����Ϊ����Ʒ��
	 *  ambient�����ô���Ʒ�Եĸ������ж�̬���Ч��.
	 */
	void                process_amDlight( os_dLightData& _dlight );

	/** Ϊ�ϲ��ṩmesh����Ⱦ����
	 *  \param _mr ��Ⱦmesh�����Ľṹ
	 */
	bool                render_mesh( os_meshRender* _mr,bool _alpha = false );


	//! ÿһ֡�ϲ���Ҫ���õ�keyAniMesh��Ⱦ����
	bool                frame_renderKeyAniMesh( 
		                  os_ViewFrustum* _vf,bool _alpha,os_FightSceneRType _rtype = OS_COMMONRENDER );

	//! ����ˮ�����Ⱦ
	bool                render_aniWater( os_ViewFrustum* _vf );

	//! ���������Ⱦ
	bool                render_aniFog(  int ShaderId,int dwtype = 2);
	
		                  

	/** \brief 
     *  Ϊ�ϲ㴴��static vertex buffer.
	 *
	 *  ���ݴ�������ݴ�С����һ���̶���С�Ķ��㻺������
	 *  ���ش�����VB���м�ܵ�buf����.�������-1,����ʧ��.
	 *
	 *  \param �Ƿ�ʹ��ϵͳ�ڴ�����Ǵ����Ķ��㻺������
	 *  
	 */
	int                 create_staticVB( int _vsize,int _vnum,bool _sysmem );
	/** \brief
	 *  ����Vertexbuffer��id,���VBʹ�õ����ݡ�
	 *
	 *  \param _vstart ��䶥�㻺������ƫ��λ��.���Զ��DIPʹ��һ�����㻺����
	 */
	bool                fill_staticVB( int _vbIdx,
		                 os_vbufFillData* _vdata,int _num,int _vstart = 0 );

	//! ���Ѿ�������static vb����������䣬�������������Ѿ�������static vb
	bool                refill_staticVB( const void* _vdata,int _pvsize,int _vnum,int _id );
	//! �ͷŵ�ǰ�ľ�̬���㻺������
	void                release_staticVB( int _vbIdx );


	/** \brief
	 *  Ϊ�ϲ㴴��static Index buffer.
	 *
	 *  ���е�IndexΪWORD����,��̬�Ķ���ʹ��һ��Index buffer.
	 *  Index data��Ҫ���ϲ㴦��,ʹ��Ҫ��һ����Ⱦ��Index&Vertex
	 *  �ڻ�������Ҳ�Ǽ�����һ���.
	 *
	 */
    int                 create_staticIB( const void* _idata,int _inum );

	//! �ͷž�̬��IndexBuffer���ݡ�
	void                 release_staticIB( int _id );

	/** \brief
     *  Ϊ�ϲ��ṩ��䶯̬Index buffer�ĺ���.
	 *  
	 *  ȫ��ֻ��һ����̬������������,�����ϲ㲻��Ҫ֪���˻�����������.
	 *  
	 *  \return int �����˶�̬����Index������buf�е�λ��.
	 */
	int                 fill_dynamicIB( const WORD* _idata,int _inum );

# ifdef _DEBUG
	int                  get_inuseSibNum( void );
	void                 get_textureStageState( DWORD _Stage,D3DTEXTURESTAGESTATETYPE _Type,DWORD* _Value );
# endif 


	/** \brief
	 *  Ϊ�ϲ㴴��dynamic Vertex buffer,�����˴���dvb��������
	 *
	 *  \param _int _size  Ҫ�����Ķ�̬���㻺�����Ĵ�С.�������0,��ʹ��ϵͳĬ�ϵĴ�С.
	 *  \return int        ���ض�̬���㻺����������.
	 *  \param BOOL _newVB �Ƿ�Ҫ����һ���µĻ�����,����ϵͳ�����޴˸�ʽ������.
	 */
	int                 create_dynamicVB( DWORD _fvf,int _size = 0,BOOL _newVB = false );

	//! ʹ�ö�������������ǵĶ�̬���㻺����
	int                 fill_dnamicVB( int _id,void* _ver,int _versize,int _vernum );


	/** \brief 
	 *  �ͷ��豸�������.
	 *
	 *  ���Ҫ����һ�����ĳ���,����Ҫ��������������м�ܵ��е��豸��������ͷŵ�.
	 *  ���ҪReset�豸,��Ҫ�ͷ��м�ܵ����е���Դ,�����´����豸��صĵ��м�ܵ���Դ.
	 */
	void                release_deviceDepData( bool _finalRelease = true );


	//=========================================================================
	//
	//  ���н׶��м�ܵ��ṩ�Ĺ��ܺ���.
	//
	//=========================================================================

	/** \brief
	 *  ʹ���м�ܵ���Ⱦһ�������,�⽫ʹ���м�ܵ�����һ��DIP����,������
	 *  ������ζѵ�һ��һ������������Ⱦ.
	 */
	bool                render_polygon( const os_polygon* _poly );
	bool				render_andClearPolygonSet( os_polygonSet*  _polySet );

	/// ����Ⱦ�������һ����ɣ��ٶȽϿ�
	bool				render_andClearMeshSet( os_meshSet* meshSet );

	bool                render_meshData( int effectId, int verSize, const os_renderMeshData* mesh );


	/// �򵥵�����3d�豸״̬����.
	void                set_viewMatrix( osMatrix& _mat );
	void                set_projMatrix( osMatrix& _mat );
	void                set_worldMatrix( osMatrix& _mat );
	void                set_viewport( D3DVIEWPORT9& _vp );
	void                set_fvf( DWORD _fvf );

	//! ����sampleStateΪreapeat״̬������Ⱦ�ر��ʱ��ʹ��clamp״̬�������������
	//! ����Ϊreapeat״̬��
	void                set_ssRepeat( void );

	//! ����Ϊclamp״̬������Ⱦ��ͼ��ʱ����Ҫ����Ϊclamp״̬��
	void                set_ssClamp( void );

	/** \brief
	 *  3d ���״̬������ʹ���������,�����������߿�ģʽ����ʵ��ģʽ.
	 *
	 *  \param _mode: 1:point 2���߿�ģʽ.3Ϊʵ��ģʽ.
	 */
	void                set_deviceFillMode( int _mode );


	//! ������Ⱦ�豸�Ķ��㻺����״̬Ϊ��.
	void                set_nullStream( void );
	void                set_renderState( D3DRENDERSTATETYPE _State,DWORD _Value );
	void                get_renderState( D3DRENDERSTATETYPE _State,DWORD* _Value );
	void                set_material( D3DMATERIAL9& _mat );
	void                set_textureStageState( DWORD _Stage,D3DTEXTURESTAGESTATETYPE _Type,DWORD _Value );
	void                set_sampleState( DWORD _sample,D3DSAMPLERSTATETYPE _state,DWORD _value );
	void                set_dxLight( D3DLIGHT9* _dxL,int _num );
	void                set_dxLightDisable( int _num );


	/** \brief 
	 *  ���м�ܵ��õ�һЩ��Ⱦ��Ϣ.
	 */
	int                 get_renderTriNum( void );
	int                 get_renderVerNum( void );
	int                 get_meshFaceNum( void );

	/** \brief 
	 *  ÿһ֡����Ҫ�м�ܵ����õ����ú���,����һЩ���õĲ���.
	 */
	void              frame_setMiddlePipe( osc_camera* _cam = NULL );

	//! ���м�ܵ��õ�device.
	LPDIRECT3DDEVICE9  get_device( void ) { return m_pd3dDevice; } 

	//! ȫ�ֹ����ɫֵ,��g_fAmbiR,g_fAmbiG,g_fAmbiB�еõ�
	static DWORD       m_dwAmbientColor;


	//@{   �һ�lost device�õ��ĺ�����
	/** \brief
	 *  reset MiddlePipeǰ��Ҫ�ͷŵ�����.
	 *
	 *  �����һ�ʧȥ���豸���Ը�ctl alt del���ֵ�����
	 */
	void                mdpipe_onLostDevice( void );


	/** \brief 
	 *  Reset�м�ܵ�.��ǰdeviceMgr Reset��,�м�ܵ�Ӧ�õ����������.
	 */
	bool                mdpipe_onResetDevice( void );
	//@} 



private:
	/** \brief
	 *  Ϊ�ϲ㴴��dynamic Index buffer.
	 *
	 *  ���е�IndexΪWORD����,��̬�Ķ���ʹ��һ��Index buffer.
	 *  Index data��Ҫ���ϲ㴦��,ʹ��Ҫ��һ����Ⱦ��Index&Vertex
	 *  �ڻ�������Ҳ�Ǽ�����һ���.
	 *  ���ϲ��ṩ��䶯̬Indexbuf�ĺ���.
	 *  �м�ܵ�ֻ��Ҫһ����̬��Index buffer�Ϳ�����.
	 */
    bool              create_dynamicIB( void );

	/** \brief
	 *  �ͷ�dynamic index buffer.
	 *
	 *  reset �м�ܵ����ݵ�ʱ����Ҫ�����������.
	 */
	bool              release_dynamicIB( void );


	/** \brief
	 *  �ͷ�dynamic vertex buffer.
	 *
	 *  \param bool _release ���������false,��ǰ�ͷ���device lost���豸�ͷ�.
	 */
	bool              release_dynamicVB( bool _release = true );

	//! device lost��Զ�̬�Ķ��㻺�������д���,���´�����Щ���㻺����
	bool              reset_dynamicVB( void );

	/** Ϊ�ϲ��ṩ��̬��䶥�㻺�����ĺ���
	 */
	//int                 fill_dynamicVB( VEC_polyptr& _polyptr, int _pnum,int _vernum );


	//! ¼���м����ÿһ֡��Ҫ���õ���Ⱦ״̬�顣
	void                record_frameSetRSB( void );


private:
	//!  �м�ܵ������������й�������Ҫ��shader.
	osc_shaderMgr          m_shaderMgr;


	//!  �м�ܵ����Զ�3d Deviceֱ�ӽ��в���,3d�豸ָ��.
	LPDIRECT3DDEVICE9      m_pd3dDevice;   


	osc_meshMgr           m_meshMgr;


	//!  �м�ܵ�������ǰ�˹ܵ�.
	osc_render            m_frontRender;


	//@{
	//! �м�ܵ�������豸�������,����Vertices&Indices buffer. 
	CSpaceMgr<os_verticesBuf>  m_vecVerBuf;
	//! �����IdxBufΪȫ�ֵĶ�̬����������.������������������Ϊ��̬
	CSpaceMgr<os_indicesBuf>   m_vecIdxBuf;
	//! ��̬��������������������Ⱦÿһ֡���ı������,ָ��m_vecIdxBuf��һ��Զ��
	os_indicesBuf*        m_dynamicIdxBuf;

	int                  m_iTileVbIdx;
	CSpaceMgr<os_verticesBuf>  m_vecDynamicVB;
	//@}

	//@{
	//  �м�ܵ���Ⱦʹ�õ���Ⱦ״̬�顣
	//! ÿһ֡��Ҫ���õ�״̬��
	int                  m_iFrameSetRsId;
};


extern osc_meshMgr*     g_ptrMeshMgr;

//! �豸����ʱ����Ӱ���ֵ��룬ʹ�õ��ٽ���
extern CRITICAL_SECTION    g_sShadowSec;


// �����м�ܵ���inline ������
# include "middlePipe.inl"
