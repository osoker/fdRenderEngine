//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdBillboard.h
 *
 *  Desc:     ���������е�billBoard,������ʾ�����е���Ч.
 *
 *  His:      River created @ 2004-2-10
 *
 *  "The people who are crazy enough to think
 *   they can change the world, are the ones who do."
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"                 // ʹ��ShaderMgr������ص��豸�����Դ��
# include "../../mfpipe/include/middlePipe.h"
//


// ���ڹ�������ʹ�ú���Ⱦ���Բο�ObjectManager�е�������
//
// ע��������bill Board,һ����������billBoard,һ���ǽ���һ���������õ�billBoard.
// billBoardÿһ���׶�Ӧ�ö�Ӧһ����ͼ.ÿһ����ͼ������ʱ��Ͷ����ķ�ʽ.
// billBoardÿһ���׶εĴ�С.billBoard��Ӧ��ͼ���ۺϷ�ʽ�ȵ�.
// 
// billBoard�Ĵ�����Ϊ��������,һ���������Ե��ڳ����д���,һ����ֻ����һ�����Ƕ������
// ���ڵ�billBoard.�����Դ��ڵ�billBoardӦ���ڵ��볡����ʱ�����,��ֻ���ż�֡��billBoard
// Ӧ���ڳ����ж�̬�ļ���.
// 
// Ӧ����һ��billboard�༭��ȥ�༭billBoard��ص���Ϣ��
// �����ʹ�õ������ļ�����xxx1.tga��ʼ��ֱ���õ�������Ҫ��֡�������������Ҫ��֡�����Ļ���
// ����xxx1.tga,xxx2.tga,xxx3.tga
//
// �������Ⱦʱ��״̬��������ɫ��������ɫ = ������Ҫ��Ⱦ����ɫ��
//                     



/** \brief
 *  BillBoard��Ӧ������.
 */
enum ose_BillBoardType
{
	/** \brief
	 *   ��xzƽ������ʾ��billBoard,���ַ�ʽ��������������billBoard.
	 *
	 *   ����������������ʾ��ħ������Ч����
	 */
	BILLBOARD_XZ = 0,

	/** \brief
	 *  ��Y������ת,����Զ���������billboard,�򵥵�billBoard,
	 *  ����billBoard��������⣬���磬����Ч����
	 */
	BILLBOARD_Y = 1,


	/** \brief
	 *  ��Զ���������billBoard.�������Ĺ���塣
	 *  
	 *  ��������ħ��,�����Ч����
	 */
	BILLBOARD_ALL = 2,


	/** \brief
	 *   ʮ�ֽ�����Ƭ��billBoard��ʵ�����㲻������ȫ��BillBoard.
	 *   
	 *   �������͡�Ŀǰ�����Ȳ�֧�֡�
	 */ 
	BILLBOARD_CROSS = 3,


	/** \brief
	 *  ��ĳ��������ת��billBoard,��Ҫ���ڵ���Ч����������
	 *
	 *  �����������ȫ�Զ��������,�ڱ༭���ڵ���ʱ��Ĭ��ʹ��Y��
	 *  �ڳ����ڣ���������������������
	 */
	 BILLBOARD_AXIS = 4,


};


/** \brief
 *  ������ļ����ļ�ͷ�ṹ��
 *  
 *  
 */
struct os_bbHeader
{
	char  m_chFlag[3];
	DWORD m_dwVersion;
};


//! ��������֧�ֵĶ���֡��.
# define MAX_BBTEXNUM    12


//! ���͹����Ŀ�ʼid
# define HUGEBB_IDSTART  4096


/** \brief
 *  ����������ɺͲ��ų��������еĹ������ص���Ʒ��
 * 
 *  ��Ҫ���������ʹ�õĽӿڡ�billBoard��Ĭ��λ������(0,0,0)�㡣
 * 
 *  ��Ҫʹ���������ʹ�÷����ͷ��������ʹ�õ�billboard����Ҫ������ռ�õ��Դ档
 *  
 */
struct osc_billBoard : public os_lruEle
{
	//! ��ǰbillBoard��Ӧ���ļ�����
	char      m_szBFilename[128];
	DWORD     m_dwHash;

	//! ѭ�����ŵ�billBoard������һ�ַ�ʽ��ֻ����һ�ε�billBoard.
	BOOL      m_bLoopBB;

	//! ����һ֡�õ���ʱ�䡣
	float     m_fFrameTime;

	//! �ܹ��Ĳ���֡����
	int       m_iTotalFrame;

	//! ���ŵ�ǰ�Ĺ�����ܹ���Ҫ��ʱ��.
	float     m_fTotalPlayTime;

	//! ��������ܹ��õ�����ͼ����Ŀ��
	int       m_iTotalTexNum;

	/** \brief
	 *  ������õ�����������֡�
	 *  ��ͼ������ʹ�õ�һ֡��ͼ�����֣�ֻ��ʹ��*.tga��Ϊ��ͼ��
	 */
	char      m_strTextureName[128];

	//! �����ÿһ֡��Ӧ����ͼ��id.
	VEC_int       m_vecTexId;

	//! �����Ĵ�С,ÿһ֡��һ����С��λ����֮֡��ʱ��ʹ����֡�����Բ�ֵ��
	VEC_float     m_vecBBSize;


	/** \brief
	 *  ������С�ĵ���ֵ����һֵΪ��ȵĵ������ڶ���ֵΪ�߶ȵĵ�����
	 *  
	 */
	osVec2D          m_vecBBSizeAdj;


	//! ��������ɫ��ÿһ֡��һ����ɫ��
	//! �ڲ��ŵ�ʱ����֮֡�����ɫʹ��������ɫ�����Բ�ֵ��
	VEC_dword       m_vecBBColor;


	//! �����ظ�����Ŀ������һ�Ź��������ʾ������ظ�Ч��,���������򵥵�ˮ��Ч����
	float          m_fTexScale;

	//
	/** \brief
	 *  ��Ⱦ��ʽ����������Ⱦ��ʽ��1���ͱ�����ϡ�2���ͱ�����ӡ�3:�ͱ���������
	 *
	 *  �ͱ�����ӵķ�ʽ����alpha״̬����Ϊ�� 
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	 *   m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
	 *   m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );
	 * 
	 *  �ͱ�����ϵķ�ʽ����Alpha��״̬����Ϊ:
	 *
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1 );
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	 *   m_pd3dDevice->SetTextureStageState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
     *   m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA  );
	 *   
	 *  �ͱ��������ӵ�״̬��
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	 *   m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
     *   m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );
	 *  
	 */
	WORD      m_wAlphaType;


	//! ���������͡�
	ose_BillBoardType  m_billBoardType;

	//! ��ǰ��billboard�����Ƿ���ʹ���С�
	BOOL              m_bInuse;

	//@{
	//  ʹ��billBoard���԰���һ����·���˶���
	bool              m_bUseRunTrack;


	/** \brief
	 *  ���billBoard����·���ļ������ƶ��Ļ�����Ӧ���ļ�����
	 *
	 *  Track�ļ���������ԭ��Ϊ��׼��
	 *  
	 */
	char          m_strTrackFname[128];

	//! billBoard��·���ϵ��ƶ��ٶȡ�
	float         m_fTrackMoveSpeed;
	//@}


	/** \brief
	 *  �豸��ص�shaderId���˴��洢�����ʼ��shaderId,��һ��billBoard
	 *  ���ݽṹ�У�Ӧ����m_strTextureName��shaderId.
	 */
	int           m_iShaderId[MAX_BBTEXNUM];

	//@{
	//! �������ת��ص����ݡ�
	BOOL          m_bRotOrNot;
	float         m_fRotSpeed;
	int           m_iRotDir;
	//@} 

	//! �Ƿ�����û����ڴ�.��Ҫ�Ĺ����,��Ҫ�����Ե�ͣ�����ڴ���.
	bool          m_bCanSwap;

	//! �Ƿ�ZEnable.
	BOOL          m_bZEnable;

private:
	//! ��ʼ����ǰ�Ĺ��������
	void          reset_bb( void );


public:
	osc_billBoard();

	/** \brief
	 *  ���ļ��е���һ��billBoard.
	 */
	bool      load_billBoardFromFile( const char* _fname );


	//! �ͷ�һ��������ļ�.
	void      release_billBoard( void );

	//! �Ƿ��Ǿ��ι����
	bool             is_hugeBillBoard( void );


};

//! ʹ��std::vector����֯billBoard. 
typedef std::vector<osc_billBoard>  VEC_billBoard;



//! �����м�ܵ��ࡣ
class      osc_middlePipe;



/** \brief
 *  ��㴴����billBoard��ʵ����billBoard Instance.
 *
 *  �ϲ��������billBoardʵ����BillBoard�����ǿ������õ����ݣ�
 *���������ṹΪ�������õĽṹ����һ��billBoard������ɺ󣬿���ʹ������ṹ
 *  ȥ������һ��BillBoard.
 */
class osc_billBoardIns : public os_createVal
{
	friend class         osc_bbManager;
private:

	//! ���ļ��е����billBoard������ָ�롣
	osc_billBoard*        m_ptrBBData;

	//! ��ǰbillBoard������ռ��е�λ�á�
	osVec3D              m_vec3Pos;

	//! ��ǰ��billBoard�Ƿ���ʹ���С�
	bool                 m_bInuse;

	//! ��ǰBillBoard��Ⱦʱ�õ������ݡ�
	os_billBoardVertex    m_arrRVertex[4];

	/** \brief
	 *  ���Ź�����ʱ����ٱ�.
	 *
	 *  ���ݹ���������������ʱ����ⲿҪ��Ĺ���岥��ʱ�����������ٶȱ�
	 *  ʹ������ٶȱ�������ÿһ֡frameMoveʱ�������ʱ�䣬�����������ⲿҪ��
	 *  ��ʱ���ڲ�����˹���塣
	 *  ����������ֵΪ1.0�����ǰ������༭ʱ��ʱ���������궯����
	 */
	float               m_fTimeScale;

	//! ����ʱ�����Ļ�׼����ֵ���Ժ�������ֵ���������׼֮������
	float               m_fBaseSizeScale;

	//! �����Ĵ�С����ֵ
	float               m_fSizeScale;


	//@{
	// ����Ϊ��ֵʹ�õ�����Ϣ��

	//! ��֮֡�����Բ�ֵ�õ���ϵ����
	float                m_fInteValue;

	//! �����һ֡�õ���ʱ�䲻���л�����һ֡�Ļ������ŵ�ʱ��洢�����ֵ�С�
	float                m_fAccumulateTime;

	//! ��ǰ���ŵ���֡������
	int                  m_iCurFrame;


	//! ��ǰinstance�õ���track��id.
	int                  m_iTrackId;
	//! ����track����������billBoardӦ�õ��ﵽ��λ�����ݡ�
	osVec3D              m_vec3Fpos;

	//@}


	//! ��ǰbillBoard��bounding sphere.
	os_bsphere           m_bsphere;

	//! ��ǰ�����ʵ������ת�Ƕȡ�
	float                m_fRotAgl;

	//! ʹ�ö��Ƶ���ɫ.
	bool                 m_bUseCustomColor;
	DWORD                m_dwCustomColor;

	//! ��ǰbb�Ƿ�ɼ�����Framemove���жϿɼ��ԣ���Ⱦʱ�á�
	bool                m_bInview;

	//! ��ǰ��bb�Ƿ�������״̬
	bool                m_bHide;

	//! ��ǰbb�Ƿ񶥲���Ⱦ
	bool                m_bTopLayer;

	//! River added @ 2006-6-29: 
	//! ������ĳ���ض���������ת�Ĺ���壬��ֵΪ��Ҫ�Ƶ���,Ĭ��ʹ��Y��.
	osVec3D            m_vec3BBRotAxis;

	//! River added @ 2011-1-30:�����߳̿�ָ����bb�Ƿ��ͷš�
	bool               m_bInReleaseState;
private:

	//! ��ʼ��billBoard��instance�Ķ������ݡ�
	void             init_bbAniData( void );

	/** \brief
	 *  ��ǰ��billBoard�������д���
	 *
	 *  \return bool �������false,���ʾ��ǰ��bb�Ѿ��ͷ�
	 */
	bool             frame_moveBBIns( float _etime,bool _canReleaseIns = false );

	
	//! �õ������ʵ�����ݵ�UP&Right Vector.
	void             get_bbUpRightVec( 
		               I_camera* _camptr,osVec3D& _up,osVec3D& _right );

	//! ����billBoard��instance���������������Ⱦ�õĶ������ݡ�
	bool             fill_renderVerData( I_camera* _camptr );

	//! �ͷŵ�ǰ��billboard,ʹ��ǰ��billBoardIns��Ϊ���õ�Instance.
	void             release_billBoardInsInner( void );


public:
	osc_billBoardIns();

	/** \brief
	 *  ��ʼ����ǰ��billboard Instance.
	 *
	 *  \param bool _createIns �����ֵΪfalse,�������billBoard��ص���Դ����Ӳ��,
	 *                         ʵ����������,ʹ����һ�δ���ʵ��ʱ�ӿ��ٶ�.
	 *  \param DWORD _cusColor �Ƿ�ʹ�ö��Ƶ���ɫ.�����ֵ����0,��ʹ�ù�����ļ���
	 *                         ��Ӧ�Ĺ������ɫ,ʹ�ô���ɫֵ,��ʹ�ù�����ļ��ж�Ӧ��
	 *                         �����alphaֵ.
	 *  \param float _sizeScale  ������Ƿ�����
	 *  \param bool _topLayer   �Ƿ񶥲���Ⱦ
	 */
	WORD             init_billBoardIns( osc_billBoard* _bbdata,osVec3D& _pos,
		                  bool _createIns = true, DWORD _cusColor = 0,
						  float _sizeScale = 1.0f,bool _topLayer = false );


	/** \brief
	 *  ��Ⱦ��ǰ��billBoard Instance.
	 *
	 *  ����billBoard�Ĳ�ͬ���ͣ��ó���ͬ��billBoard��Ⱦ����
	 * 
	 *  \param _pipe    ��Ⱦ��ǰ��billBoardʱʹ�õ��м�ܵ���
	 */
	bool             render_billBoardIns( osc_middlePipe* _pipe );

	//! ��bbIns�еõ�Ҫ��Ⱦ��shaderId,�����Ż���BB��Ⱦģʽ��
	int              get_bbInsRShader( void );


	//! �ͷŵ�ǰ��billboard,ʹ��ǰ��billBoardIns��Ϊ���õ�Instance.
	void             release_billBoardIns( bool _finalRelease = false );

	/** \brief
	 *  ���ݹ��������������õ�ǰbillBoard��λ�á�
	 *  
	 */
	void             set_billBoardPos( osVec3D& _pos );

	//! ���ù����Ĳ���ʱ��.
	void             set_bbPlayTime( float _timeScale );

	//! �õ������Ĳ���ʱ��.
	float            get_bbPlayTime( void );

	//! �Ƿ���ѭ�����ŵĹ����
	bool             is_loopPlayBB( void ) { return m_ptrBBData->m_bLoopBB; }

	/** \brief
	 *  ��ǰ��billBoard�Ƿ���ʹ���С�
	 */
	bool             is_billBoardInUsed( void )  { return this->m_bInuse; }

	//! ���ù�������ת��
	void             set_bbRotAxis( osVec3D& _axis ) { m_vec3BBRotAxis = _axis; }

};

//! ʹ��std::vector����֯billBoard��ʵ�����ݡ�
typedef std::vector<osc_billBoardIns>  VEC_billBoardIns;

//! �����
/** \brief
 *  billBoard�Ĺ�������
 *  
 *  ��������������˵�ǰ�������õ���billBoard��Billboardʵ����
 *
 *
 *  ��Ӱ�͹����ֿ�����Ϊ��Ӱ����Ⱦ��Ҫ�ڳ����ĵر� 
 *  ��Ⱦ֮��������Ʒ��Ⱦ֮ǰ����Ҫ�ر�ZBuffer.
 *  
 */
class osc_bbManager : public ost_Singleton<osc_bbManager>
{
	//! �����Ҫ����ʹ�ù������е�billBoard vector.
	friend  class    osc_billBoardIns;

private:
	//! �������е�billBoard���ݡ�
	CSpaceMgr<osc_billBoard> m_vecBillboard;

	//! �������е�billboardInstance
	CSpaceMgr<osc_billBoardIns> m_vecBBIns;

	//! �������ڵĲ���Զ������Ӱ��ľ��͹����ʵ��
	CSpaceMgr<osc_billBoardIns> m_vecHugeBBIns;

	//! �����µ�����Ч����billBoard.
	CSpaceMgr<osc_billBoardIns> m_vecRainEffect;


	//! ��ǰ������������м�ܵ�ָ�롣
	osc_middlePipe*   m_ptrMPipe;


	//! ��ǰ֡��Ⱦ��������
	int              m_iRenderTriNum;


	//! ��Ⱦ��ǰ��bbMgrʱ�������õ��ľ�̬������������
	int              m_iIdxBuf;
	//! ��Ⱦ��ǰ��bbMgrʱ�������õ��Ķ�̬���㻺������
	int              m_iVerBuf;


	//! ��һ���������Ч��
	bool             m_bFirstRainEff;
	//! ��һ�������������Ӱ.
	bool             m_bFristShadow;

	//! ÿ��һ��ʱ�䣬�ͷ�һЩ���õ�billboard.
	float            m_fDiscardBBTime;

	//! ���ڴ��������Чʱ�Ĺ����λ��
	static I_fdScene* m_ptrScePtr;

	//! ȫ�ֵ�cameraָ��
	I_camera*         m_ptrGlobalCam;

private:

	//! ��һ��bb�ļ���,�õ�billBoard��Ӧ�ڴ�ṹ��ָ��.
	osc_billBoard*    get_billBoardPtr( const char* _fname );

	//! ����billBoard��lru.
	void              set_bbMgrLru( void );
	//! �ͷ����һ��ʱ��û��ʹ�õ�billboard.
	void              discard_lruBB( void );

	//! ��ȾRainEff�õ���polygon.
	os_polygon         m_sRainEffPoly;
	//! ��һ�γ�ʼ��RainEff��ص����ݡ�
	void              first_initRainEff( void );


	osc_bbManager();
	~osc_bbManager();

public:

	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static osc_bbManager*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void         DInstance( void );


	//! ��ʼ��billBoardManager.
    void             init_bbManager( osc_middlePipe* _pipe );


	//
	//! ����һ��billBoardInstance.
	int              create_billBoard( const char *_bfile,osVec3D& _pos,
		                  bool _createIns = true,float _sizeScale = 1.0f,bool _topLayer = false );

	/** \brief
	 *  ����һ�����ܼ������Ӱ���billBoard
	 *
	 *  �������͵Ĺ������Ҫ����ȫ�ֵĳ�����Ч��������������Ч��������
	 */
	int              create_hugeBillBoard( const char *_bfile,osVec3D& _pos,
		                  bool _createIns = true,float _sizeScale = 1.0f );

	/** \brief
	 *  ����billBoard��λ�á�һ�������еĹ���壬���ܿ���λ�á�
	 *
	 *  \param _id ��Ҫ���õĹ�����id��
	 *  \param _pos  Ҫ���õĹ�����λ�á�
	 */
	void              set_billBoardPos( int _id,osVec3D& _pos,bool _updateImm = false );

	/** \brief
	 *  ��ʱ���µ�ǰid�����ʵ����λ��
	 *
	 *  ���ڸ����ڳ��������FrameMove������׼ȷ�õ������λ�õ������
	 *  �����������õ��Ĺ���塣
	 */
	void              update_bbPos( int _id,I_camera* _camptr );


	//! ���ù����Ĳ���ʱ�䡣
	void              set_bbPlayTime( int _id,float _timeScale );

	//! ���ù�������ת��
	void              set_bbRotAxis( int _id,osVec3D& _axis );


	//! �õ���������������ʱ�䡣
	float             get_bbPlayTime( int _id );


	/** \brief
	 *  ɾ��billBoard.
	 *
	 *  \param _id ɾ��ָ��id��billBoard.
	 */
	void              delete_billBoard( int _id,bool _finalRelease = false );

	// River @ 2010-12-24:Ϊ���ϲ��ٽ���ɾ�������Ĳ�����
	const char*       get_bbNameFromId( int _id );

	/** \brief
	 *  ������billboard����Ҫ�����billBoard����Ϊ��Ⱦ׼�����ݡ�
	 *  
	 *  ��������ڰ�����shadow���bb��������䴦��
	 */
	void             frameMove_bbMgr( I_camera* _camptr );

	//
	/** \brief
	 *  ��Ⱦ�����������пɼ���billBoard.
	 *
	 *  \param _camptr    �����м���������
	 */
	bool             render_bbManager( I_camera*  _camptr,os_FightSceneRType _rtype = OS_COMMONRENDER );

	//! ��Ⱦ���͹���壬�������Ⱦ�Ľ׶���Ⱦ
	bool             render_mgrHugeBB( I_camera* _camptr );



	/** \brief
	 *  ��Ⱦ��Ӱ����塣
	 *
	 *  ���е���Ӱbb����xz�����ϵ�bb,���Բ���Ҫ�����
	 */
	bool             render_raindropBB( I_camera* _camptr );
	//@} 

	



	//@{
	//  ���Ч���䵽�����Ч����
	//! �������Ч����
	void            create_rainEff( osVec3D& _vec );
	//@} 

	//! ��֤��ǰ��billBoardId�Ƿ���Ч��
	bool             validate_bbId( int _id );

	//! ��ǰ�Ĺ�����Ƿ���ѭ�����ŵĹ����,����������Ч,����ʹ��ѭ�����ŵĹ����.
	bool             is_loopPlayBB( int _id );

	//! ���ػ���ʾ��ǰ��bb.
	void             hide_bb( int _id,bool _hide );

	//! ���ŵ�ǰ�Ĺ����.river added @ 2010-1-7:
	void             scale_bb( int _id,float _rate );


	//! �õ���ǰ֡��Ⱦ��bbMgr������
	int            get_bbMgrRenderTriNum( void )    { return m_iRenderTriNum; }

	//! IdentityMat
	static osMatrix   m_smatMatrix;


};



//! ������ǰbillBoardMgr��Ӧ��inline������
# include "fdBillBoard.inl"







