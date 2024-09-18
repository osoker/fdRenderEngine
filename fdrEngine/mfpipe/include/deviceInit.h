//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: deviceInit.h
 *  Desc:     ����3d Device�ĳ�ʼ��,�õ�һ�����Ա�Ӧ�ó���ʹ�õ�3d device.
 *  His:      River created @ 4/14 2003
 *
 * �������õ����£�Ҳ�����������¡���
 * 
 */
//--------------------------------------------------------------------------------------------------------
# pragma  once

# include <iostream>
# include <vector>
# include "../../interface/osInterface.h"
# include "../../dxutil/include/d3denumeration.h"
# include "../../dxutil/include/d3dsettings.h"
# include "../../dxutil/include/dxutil.h"
# include "../../dxutil/include/d3dutil.h"

# include "osMoveRecord.h"
# include "font.h"
# include "pixelfont.h"


extern CGfxFont   g_song12x12;
extern CGfxFont   g_song14x14;

//! ��ʼ����ʱ��ʾ�������Ŀ.
//!
//!  ATTENTION TO FIX:��ʱ�ı䣬���ֵ����϶���������,�Ժ���
# define MIN_TMPTEXTURE   (196*2)


//! �����п��Ե�������Ĺ�����Ŀ��
# define  MAX_CURSORNUM        32

//! ������Ĵ�С,����Ļ��ռ�õ�������Ŀ��
# define  CURSOR_PIXELSIZE     32


//! ����ȷ�Ͽ�ʼ�ͽ�����Ⱦ3d��������ԡ�
# define  BEGIN_DRAWTEST    1

//-------------------------------------------------------------------------
/** \brief
 *   ������ʱ��ʾ����,������볡��ʱ�Ľṹ.
 *  
 *   �����ֱ��ʹ��3dDevice��Textureָ��,����ʾ��ʱ�������Ҫʹ�ö��߳�.s
 */
//-------------------------------------------------------------------------
struct os_tmpdisplayTex
{
public:
	LPDIRECT3DTEXTURE9      tex;
	bool                    inuse;

	int                   m_iRef;

	//! ���� �����������.
	char                   m_szTexName[128];

public:
	os_tmpdisplayTex()
	{
		tex = NULL;
		inuse = false;
		m_iRef = 0;
		m_szTexName[0] = NULL;
	}

	void                  add_ref(){ m_iRef ++;}
	void                  release_texture( int _curId );


	~os_tmpdisplayTex()
	{
		if( tex )
			SAFE_RELEASE( tex );
	}
};


/**
 *  ����һ��std::vector����ʹ��os_tmpdisplayTex.
 */
typedef  std::vector<os_tmpdisplayTex>  VEC_TMPDISPLAYTEX;

//! ������ʾ����Ļ�����ݽṹ��
typedef  std::vector<os_screenPicDisp>  VEC_spDisp;

//! ����os_screenPicDisp�ṹ��ָ���б�.
typedef std::vector<os_screenPicDisp*>  VEC_scrPicPtr;

//! �����������8֡�Ķ���
# define MAX_ANICURSORFRAME  8

//! ������豸�ڲ������ݽṹ
class osc_mouseCursor
{
private:

	//! ÿһ������Ӧ��������豸�������
	VEC_TMPDISPLAYTEX    m_vecCursorTex;
	//! ��궯����֡���л�ʱ��
	float                m_fFrameTime;


	//! ��һ֡��Ӧ�Ĺ��id
	int                  m_iLastFrameCursorId;
	//! �л���֡��궯���Ѿ���ȥ��ʱ��
	float                m_fEvaSwitchTime;

	//! ������������.
	char                 m_szTexName[128];

	//! ����λ��ƫ����
	UINT				 m_cursorOffset_x;
	UINT				 m_cursorOffset_y;

private:
	//! ���ݹ������֣�����Ŀ¼�¶�Ӧ�Ĺ���ļ�
	bool    process_cursorNum( const char* _name );

public:
	osc_mouseCursor();
	

	//! ���赱ǰ��cursor���ݽṹ
	void    reset( void );

	//! ���ݴ�����ļ�������������������ļ�
	BOOL    create_cursor( LPDIRECT3DDEVICE9 _dev,const char* _name );

	//! �ϲ�Թ������õ���
	void    set_cursor( LPDIRECT3DDEVICE9 _dev, int _centerX =0, int _centerY =0 );

	//! ÿһ֡���豸��ع�������
	void    frame_setCursor( float _eletime,LPDIRECT3DDEVICE9 _dev );

};



//! ����ȴ������ϵ���ʾ��Ϣ��󳤶ȡ�
# define  MAX_BKINFOLEN      256

//! ��Ⱦ��Ļ�������Ҫ�����ݽṹ��
class osc_scrPicRenderData
{
private:

	//! ��Ļ�������Ⱦʱ��ʹ�õĶ�̬���㻺����.
	LPDIRECT3DVERTEXBUFFER9  m_ptrVerBuf;

	//! ��Ļ�������Ⱦ��ʱ��ʹ�õľ�̬���㻺����.
	LPDIRECT3DINDEXBUFFER9   m_ptrIdxBuf;

	//! ȫ����ά�豸��ָ�롡��
	LPDIRECT3DDEVICE9        m_pd3dDevice;

	//! Ҫ��Ⱦ�������ε�����.
	int                      m_iRenderVerNum;

	//! ��Ŷ������ʱ������.
	os_screenVertex*         m_ptrScrVer;

public:
	osc_scrPicRenderData();
	~osc_scrPicRenderData();

	void        init_scrPicRenderData( LPDIRECT3DDEVICE9 _dev );
	void        release_scrPicR( void );

	void        push_scrRenderVer( os_screenVertex* _ver );
	void        draw_scrRenderVer( void );

	//! �豸�������.
	void        scrpic_onLostDevice( void );
	void        scrpic_onResetDevice( void );

	//@{ ���µı���������d3dMgr�ж��ƽ�����Ⱦ�ܵ�����Ļ����ν��д���
	//! ������������Ⱦ�ͻ���Я�����ܵ��е���Ļ���������ָ�롣
	VEC_scrPicPtr       m_vecScrPicPtr;

	//! ��Ⱦ��Ļ������õ�����ʱ����.
	VEC_BYTE            m_vecHaveRender;
	//@}

};


//syq_mask
struct RENDERTARGET
{
	RENDERTARGET():
		texid( -1 ),
		pRenderTarget( NULL ),
		m_pTargetSurface( NULL )
	{
	}

	int   id;
	LPDIRECT3DTEXTURE9   pRenderTarget;
	LPDIRECT3DSURFACE9   m_pTargetSurface;
	int texid;

	// 
	//! �߶ȺͿ��
	int   m_iWidth;
	int   m_iHeight;
	D3DFORMAT format;

	//! 
	void  on_lostDevice( void );
	void  on_restoreDevice( LPDIRECT3DDEVICE9 _dev );

	//! ��˻�������renderTarget.
	static LPDIRECT3DSURFACE9   m_pBackBufSurface;
};

class osc_middlePipe;

class osc_skinMeshMgr;

//-------------------------------------------------------------------------
//d3d��ʼ����release����.�������Ҫ�����˵�ǰ��Ⱦ�豸�ļ����Եȵ���Ϣ.
//���������Եõ�һ��d3d��Ⱦ�豸������豸�ļ�����.
//���Ҫʹ��ogl��ʼ��,��������.
//
//
//d3d managerӦ��ά��һ�����ڵ�ǰd3d device�ļ������б�.
//
//
//���ڻص�����:
//��Ϊÿһ��ʹ����������Ӧ�ó�����Ҫ����һ��ȷ���豸�Ļص�����,
//�������������d3dmanager�н��е���,���Ҫ�������ĵ�������д��.
//
//ʹ��I_d3dmanager��������õ�d3dmanager���ʹ�ýӿ�.
//
//-------------------------------------------------------------------------
/** \brief
 *  3d �豸�Ĺ�������.
 *
 *  �������3d�豸�������Ĺ�����,����I_deviceManager�ӿڼ̳ж���,��
 *  direct3D��3d�豸ʵ����I_deviceManager�ӿڼ��Ľӿ�.
 * 
 */
class osc_d3dManager : public I_deviceManager
{
private:
	//! �Ƿ�ȫ��ģʽ.
    bool                m_bWindowed;

	//! The main D3D object
    LPDIRECT3D9         m_pD3D;              
	//! The D3D rendering device
    LPDIRECT3DDEVICE9   m_pd3dDevice;        
	//! Caps for the device
	D3DCAPS9            m_d3dCaps;   
	//! Surface desc of the backbuffer
    D3DSURFACE_DESC     m_d3dsdBackBuffer;   
	//! Indicate sw or hw vertex processing
	DWORD                m_dwCreateFlags;     


	//! The Enumeration Ojbect.
	CD3DEnumeration     m_Enumeration;
	//! ��ǰ�豸��3d Setting.
    CD3DSettings        m_d3dSettings;
	//! Parameters for CreateDevice/Reset
	D3DPRESENT_PARAMETERS  m_d3dpp;   


	//! ����devicelostʱ,��device�� Reset.
	osc_middlePipe*       m_ptrMiddlePipe;
	osc_skinMeshMgr*      m_ptrSmMgr;


	//! ��ǰd3d Manager�Ĵ��ھ��.
	HWND                m_hWnd;
	//! Saved window bounds for mode switches
	RECT                m_rcWindowBounds;   
	
	//! Windows client�õ���rect.
	RECT                m_rcClientBounds;

	//! ��ǰ���ڵ�style.
	DWORD               m_dwWindowStyle;




	//! ��ǰ��Ļ�Ŀ�͸�.
	int                 m_iScrWidth,m_iScrHeight;  
	//! ��ǰ��Ļ��ɫ�����.
	int                 m_iScrBpp;    

	//! ��ǰ�Ѵ����豸��stencilλ��,�豸��ʼ��ʱ�������λ����stencil,���ʧ�ܣ�
	//! ������stencil���豸��
	int                 m_iStencilBitsNum;

	//! ��ǰ�豸�����clip plane��Ŀ��
	int                 m_iMaxClipPlane;




	//! ȷ���豸�ĺ���ָ��,һ��Ҫ��Ӧ�ó������������ָ��.
	CFMPTR              *cfm_deviceptr;


	//!  Whether to show cursor when fullscreen
    bool                m_bShowCursorWhenFullscreen; 


	//!  ������ʾ���ֵ���.
	//CWFont               m_fontMgr;

	//!  �Ƿ�����Ӳ����ꡣ
	bool                 m_bHideCursor;


	//! River added @ 2007-2-26:�õ���˻�����������
	LPDIRECT3DSWAPCHAIN9    m_ptrSwapChain;

	//! River added @ 2010-3-1: ������Ⱦ�ര�ڡ�
	LPDIRECT3DSWAPCHAIN9    m_ptrAddSwapChain;
	D3DPRESENT_PARAMETERS   m_sAddSwapPara;

	//! River mod @ 2008-3-28:
	IDirect3DQuery9*     m_pEventQuery;

private:

	//!  ��ǰ�豸�Ķ��㴦��״̬.
	ose_VertexProcessingType   m_eVPT;

	//!  ��ǰ���豸�Ƿ����ʹ��fogTable
	bool                 m_bUseFogTable;

	//!  ��ǰ���豸�Ƿ�֧��DXT2����ѹ��.
	bool                 m_bSupportDXT2;

	/** \brief
	 *  �õ��豸�ļ�������Ϣ.
	 */
	void               get_deviceCaps( void );


	/** \brief
	 *  �Ƿ�����ջ�������������Ⱦ״̬��
	 *
	 *  ��ջ������󣬴�ʱ����Inrender״̬��Present�󣬴��ڷ�Inrender״̬��
	 */
	bool               m_bInRender;


	//! ��ǰ����3d��Ⱦ״̬��
	bool               m_bInDrawScene;

private:
	//@{
	/**
	 *  Used for display temp texture when we have not scene
	 *	to display.
	 *  
	 *  ���볡��ʱ�ĵ��뻭��ʹ�����vector�ڵ���ͼ��
	 *  
	 */
	VEC_TMPDISPLAYTEX   m_vecTexture;
	VEC_spDisp          m_vecSPDisp;
	float              m_fCurProgress;


	/** \brief
	 *  �������õ�����Ļ��ʾ����Ρ�
	 */
	VEC_TMPDISPLAYTEX   m_vecSceneScrTexture;

	//syq_mask ��������ȾĿ���б�
	std::vector< RENDERTARGET > m_vecRenderTarget;

	//! ��Ⱦ��Ļ������õ��İ�����.
	osc_scrPicRenderData  m_sPicRD;



	//! ������ʾ��Ϣ�Լ���Ϣ����ʾλ�á�
	int                m_iInfoX,m_iInfoY;
	char               m_szBkInfo[MAX_BKINFOLEN];
	DWORD              m_dwBkInfoColor;

	//! ����ȴ���Ϣ�������ʾ
	int                m_iWaitInfoX,m_iWaitInfoY;
	char               m_szWaitInfo[128];
	DWORD              m_dwWaitInfoColor;

	//! 
	void               bkset_process( const char* _bgName,
		                      int _x,int _y,const char* _info,DWORD _infoColor  );
	//@}

	CSpaceMgr<osc_mouseCursor>   m_vecCursor;

	//! ��ǰ���������Ļ����ʾ�Ĺ��id.
	int                m_iActiveCursorId;


	//! ��Ⱦ��ά�����õ�����Ⱦ״̬��id.
	int                m_iScrTexRSId;
	int                m_iScrTexRSId_rt;

	//! River @ 2008-5-27:����alpha�ͱ�����ӵĹ���
	int                m_iScrTexRSAddId;

	//@{
	// Windy mod @ 2005-9-20 16:05:38
	//Desc: ��Ⱦ��ά����LensFlare�õ���Ⱦ״̬��ID
	int					m_iLensFlareRSId;

	//@}


	//@{
	//! ȫ��Ļfade��Ҫ������.
	bool               m_bFullScrFadeState;
	float              m_fFadeTime;
	float              m_fElaFadeTime;
	DWORD              m_dwStartColor;
	DWORD              m_dwEndColor;
	int                m_iFullScrFadeRSId;
	//@} 

	//@{
	//! ����Ļ����Σ�����Ļ��alphaֵΪȫ��
	int                m_iFullscrAlphaId;
	//@}

private:
	/** \brief
	 *  ��ʼ�����еı���.
	 */
	void                re_initvar( void );

	/** \brief
	 *  �ڴ��ڷ����仯ʱ�������ڵ�style,�����ȫ���䵽������Ҫ��������style.
	 *
	 */
	void                AdjustWindowForChange( void );

	//! create the d3d query 
	void				CreateD3DQuery(void);


	//! �ͷŵ�ǰ�豸��additional swap chain.
	void                release_addSwapChain( void );


protected:

	/** \brief
	 *  �ҵ�һ��������Ҫ���device.
	 *
	 *  ���Ѿ��������豸�б����ҵ�һ����������Ҫ���Device,���ڲ�Ҫ�����,����SampleType
	 *  ����framerate,���ǲ���Ҫ��Ѱ��Init deviceʱ����.����ҵ��豸,
	 *  ������m_d3dSettings.
	 * 
	 *  \param _initdata   ҪѰ���豸��ָ������.
	 *  \return bool ����ҵ��豸����ڲ�����m_d3dSettings,������.
	 *  \param int _times  �ڶ��ٴβ����豸
	 */
	bool                find_initdevice( const os_deviceinit* _initdata,int _time = 0 );


	/** \brief 
	 *  find_initdevice�����İ�������.
	 *
	 *  ���ҪѰ�ҵ��豸�Ǵ����豸,find_initdevice�����͵������������Ѱ���ʺϵĴ���
	 *  �豸.
	 */
	bool                find_windowedDevice( const os_deviceinit* _initdata );

	//! ����һЩ�����Կ�������,ʹ���֪��Ϊʲô�������Ե.
	bool                is_onboardGraphicCard( const char* _des );


	/** \brief 
	 *  ����3d��Ⱦ����.
	 *  
	 *  ���Ѿ�����CD3DSettings�ṹ�еõ�������Ҫ����3D device��Ҫ
	 *  �ĸ��ֲ���,����3d Device.
	 *
	 *  ��������:
	 *
	 *    1: ����� D3DPRESENT_PARAMETERS �ṹ.
	 *	 
	 *	  2: ����3d device.
	 *
	 */
	bool                create_3denv( BOOL _bStartQuery );  


	/** \brief 
	 *  Fill the D3DPRESENT_PARAMETERS struct.
	 *
	 *  ʹ������׼���õ�CD3DSettings�ṹȥ��䴴��3D�豸�����D3DPRESENT_PARAMETERS
	 *  �ṹ.Ҫ�����ı���Ϊ:m_d3dpp
	 */
	void                BuildPresentParamsFromSettings( void );


	/** \brief 
	 *  �õ���ǰ��d3d��ʽ��16λ����32λ.
	 *
	 *  �Ӳ�ͬ��d3d��ʽ�еõ������ʾģʽ��16λ����32λ.
	 *  \param _format Ҫ���ĸ�ʽ.
	 *  \return int ����16����32,����Ƿ���0,����.
	 */
	int                 formatlength( D3DFORMAT _format );

	//! ��¼��ǰd3dMgr����Ҫ����Ⱦ״̬�顣
	void                record_scrTexRBlock( void );

	//! ��¼��ǰd3dMgr�и�������ӵ���Ⱦ��
	void                record_scrAlphaAddBlock( void );

	
	//! ��renderTarget��״̬
	void                record_scrTexRBlock_rt( void );//syq test code

	//! ��¼ȫ��alphaʱ,��Ҫ����Ⱦ״̬��.
	void                record_fullScrFadeRBlock( void );

	//! ��¼��ȫ��Ļalpha��״̬����Ⱦ
	void                record_fullScrAlpha( void );

	//@{
	// Windy mod @ 2005-9-20 16:07:46
	//Desc: Lens Flare RS 
	void				record_LensFlareRBlock( void );

	//@}


	//! ��ʼ��font�������,�ڳ�ʼ����ȫ�ֵ�StateBlockMgr����ô˺�����
	void                        init_miskRes( void );


	//! ��Ⱦһ����Ļ������б��б��е�����Ӧ����ͬ��������id.
	bool                        disp_scrPicArr( os_screenPicDisp** _tdisp,
		                                    int _disnum,int _texIdx );

	//! ����Ǵ���ģʽ�������豸ǰ���ȵ������ڵĴ�С��λ�á�
	void                        adjust_windowSize( HWND _hwnd,int _width,int _height ,bool _windowed);


public:
	osc_d3dManager();
	~osc_d3dManager();


	/** \brief 
	 *  ����������Ҫ��3d Device.
	 *
	 *  ���������I_deviceManager�еĽӿ�,ʵ�ֲ���:
	 *  
	 *  1: ��direct3d object.
	 *  
	 *  2: Enumerate the device list.
	 *
	 *  3: Find the best device we need .
	 *
	 *  4: Create the device&return;
	 *
	 */
	virtual bool         init_device( HWND _hwnd,const os_deviceinit* _initdata,
		                              void* _cfm_ptr );


	//! ��������d3dDevice�Ŀ�ȣ��߶�,�����ϲ������Ϸ�ķֱ���
	virtual bool         reset_device( int _width,int _height,bool _windowed = true );

	/* \brief
	 * ���������swapChain,���ڶര����Ⱦ
	 *
	 * ���ϲ�Ĵ��ڴ�С�ı��ʱ�򣬵�������ӿ�
	 * �����ʱ�ڲ��Ѿ���һ��swapChain�����Ҵ�С����ǰ�Ĳ�һ�£����ͷ��ڲ���
	 * ���ڵ�swapChain,Ȼ�����´���һ��swapChain.
	 *
	 * ��� _hwnd == NULL �������ͷŽ�������Դ
	 */
	virtual bool         create_addSwapChain( int _width,int _height,HWND _hwnd );

	//! �����м�ܵ���ָ��������������ָ��.���ڴ���device reset�����.
	void                 set_deviceResetNeedRes( 
		                     osc_middlePipe* _mptr,osc_skinMeshMgr* _cptr );

	
	/** \brief 
	 *  �ͷ����Ǵ�����3d Device.
	 *
	 *  ���ͷ�3d Device֮ǰҪ�����е�3d��Դ���ͷ���.
	 *  
	 */
	virtual void         release_device( void );



	/** \brief 
	 *  ʹ��d3d��Ӳ�����.
	 *
	 *  ������������������ϢWM_MOUSEMOVE��Ϣ�е���,�����������ʹ��d3d��Ӳ�����.
	 *
	 */
	virtual void         set_hwCursor( void );



	//  Uper layer should control these func??????�����Ҫ�ٹ���.
	//@{
	/**
	 *  Render control functions.���е���Ⱦ����beginDraw����,
	 *  ������Present�����(Present�а�����enddraw );
	 */
	virtual void         d3d_begindraw( void );
	virtual void         d3d_enddraw( void );
	//! river added @ 2010-3-1:���ڶര����Ⱦ
	virtual void         d3d_addSwapClear( DWORD _Count,
                                     CONST D3DRECT* _pRects,
                                     DWORD _Flags,
                                     D3DCOLOR _Color );

	//!�������Ƿ��Ѿ�׼������
	virtual BOOL		d3d_isSwapPrepare(void)const;

	virtual void         d3d_clear(  DWORD _Count,
                                     CONST D3DRECT* _pRects,
                                     DWORD _Flags,
                                     D3DCOLOR _Color,
                                     float _Z,
                                     DWORD _Stencil );
	virtual DWORD        d3d_present( const os_present* = NULL,bool _reset = true);
    //@}



	/** \brief
	 *  ����3d Device��Gamma ֵ.
	 *
	 *  Gammaֻ�������άȫ���²���Ч����Ŀǰ��Ϊ���뷨��ԭ������ʹ���˼ٵ�ȫ����
	 *  
	 */
	virtual void         set_gamma( const os_gamma* _gamma );



	//@{
	/**
	 *  �õ�3d Deivce.
	 */
	virtual LPDIRECT3DDEVICE9   get_d3ddevice( void );
	virtual LPDIRECT3D9         get_d3d( void );
	//@}

	virtual int          get_availableTextureMem( void );
	
	//syq
	int		get_curUsedSceneScrTextureNum();


	/** \brief 
	 *  Reset 3d ��Ⱦ����.
	 *
	 *  ��������3d ��Ⱦ����,����Ӵ���ģʽ��ȫ��ģʽ,����ȫ��ģʽ��ͬMode
	 *  ���л�.
	 *
	 */
	bool                reset_3denv( bool _forceReset = false );


    /**********************************************************************/
	//@{
	/** 
	 *  Function to display temp pictures.such as start screen or
	 *  load screen.
	 */
	virtual int          load_texture( const char* _filename );
	virtual void         unload_texture( int _texid );
	//@}

	/** \brief
	 *  Display a temp picture in screen,such as load screen.
	 *
	 *  \param _tdisp   Ҫ��ʾ�����ݽṹ��
	 *  \param _disnum  Ҫ��ʾ�����ݽṹΪָ�룬������ݿ����ж��ٸ�Ԫ����Ҫ��ʾ��
	 *  \param _texId   Ҫʹ����һ������array: Ŀǰ������Array 0: deviceMgr.
	 *                  1:�����д���������  1: ͼ������
	 *  
	 */
	virtual bool         disp_inscreen( const os_screenPicDisp* _tdisp,
		                      int _disnum = 1,int _texIdx = 0 ,bool _LensPic = false);

	/** \brief
	 *  ��Ⱦ�ͱ��������ӵ�ͼƬ
	 */
	virtual bool         disp_alphaAddScrPic( const os_screenPicDisp* _tdisp,int _idx,int _texIdx );


	//@{
	/** \brief
	 *  ���õ�����Ϸ��������ʱ������ı������档 
	 * 
	 *  ��������ʹ��ͳһ��ͼƬ���ʹ��640*480����ʵͼƬ��С��������ͼƬ
	 *  �зֳ�����256*256��ͼ������128*256��ͼ��
	 *  ����ʹ��һ��256*64�ĵ����
	 *
	 *  \param _bgName ������ǽ���Ŀ¼�����֣������Ŀ¼�´����˱���ͼƬ�����֡�
	 *  \param _x,_y,_info ���ñ���ʱ����Щ���������ڱ�����ĳ��λ������һ����ʾ��Ϣ,
	 *                     �����ʾ��Ϣ����ʾ�����Ĺ����У�һֱ����.
	 *  \param _infoColor   ���ñ�������ʾ��Ϣʱ����ʾ��Ϣ���ִ���ɫ
	 */
	virtual bool         set_backGround( const char* _bgName,int _x = 0,
		                  int _y = 0,const char* _info = NULL,
						          DWORD _infoColor = 0xffffffff );

	/** \brief
	 *  ���÷������չ��
	 *
	 *  ���ֵΪ1.0,�����չ���϶����������չ����
	 *
	 *  \param _addps �����������ԭ�������ϼ���Ľ��ȣ��տ�ʼʱ����Ϊ0,�����ֵ
	 *                ����ϵͳ�ڲ��Ľ��ȴ���1.0,��ϵͳʹ��1.0�Ľ��ȡ�
	 *
	 *  \param _x,_y  ����ȴ��������������ʾ��Ϣ����Ϊ��ʾ��Ϣ��λ�á�
	 *  \param _info  ������ʾ��Ϣ,Ϊ������ʾ��Ϣ��
	 *  \param _infoColor ��ʾ��Ϣ�ִ�����ɫ.
	 */
	virtual void         set_progress( float _addps,int _x,int _y,
		                  const char* _info = NULL,DWORD _infoColor = 0xffffffff );
	//@}




	/** \brief
	 *  Ϊ��������Ҫ��ʾ�Ķ�άͼƬ��������
	 */
	int                  create_sceneScrTex( const char* _filename );
	
	LPDIRECT3DTEXTURE9   get_sceneScrTex( int _id );
	//syq_mask ������ȾĿ��
	int                create_renderTarget( int _width, int _height, D3DFORMAT _format = D3DFMT_A8R8G8B8 );

	//syq_mask �����ȾĿ��
	LPDIRECT3DSURFACE9 get_renderTarget( int _id );
	LPDIRECT3DTEXTURE9 get_renderTexture( int _id );




	//! testע��һ������,����id
	int					register_texid( LPDIRECT3DTEXTURE9 _pTexture );
	void				unregister_texid( int _texid );
	int register_targetAsTex( int _targetIdx );

	//! ����һ������
	int					 create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture );

	//! �ͷŵ����ĳ����еĶ�άͼƬ����
	void                 release_sceneScrTex( int _texid );

	/** \brief
	 *  �ͷ����еĳ�����ʹ�ö�άͼƬ����
	 *
	 *  \param _realRse �Ƿ�����ͷţ�����Ǽٵ��ͷţ�������������
	 *                  ������������ڴ��ڣ��򷵻��Ѿ���������
	 */
	void                 release_allSceScrTex( bool _realRse = true );

	/**********************************************************************/



    /**********************************************************************/
	//
	// Use following func to display string of Engine.
	//
	/**********************************************************************/
	/** \brief
	 *  Draw text on the screen.  
	 *
	 *  ���������������ʾ������Ϣ����Ҫ�ĺ���,����֧��Unicode.
	 *  
	 *  ����������,�����Ч�ʵ��ַ���ʾ����.
	 * 
	 */
	virtual int          disp_string( os_stringDisp* _dstr,int _dnum,int _fontType = 0,bool _useZ = false, bool _bShadow = false, RECT* _clipRect = NULL );

	//! ��������Ļ����ʾ�ִ���������disp_string����һ�¡�
	virtual int          disp_stringImm( os_stringDisp* _dstr,int _fontType = 0,os_screenPicDisp* _cursor = NULL );
		                    

	/** \brief
	 *  �Ե�ǰ����Ļ���н�ͼ.
	 *  ʹ�����ڼӵ�ǰ��ʱ����Ϊ��ͼ������.
	 */
	virtual char*         catch_screen( void );

	//! �õ���˻�����������.
	const D3DSURFACE_DESC*  get_backBufDesc( void ){ return &m_d3dsdBackBuffer; } 

	//! �õ�������.
	virtual bool get_mainRenderTarget( LPDIRECT3DSURFACE9& _target );


	//! �õ��Կ�����Ϣ
	virtual const char* get_displayCardInfo(void)const;


	//@{
	/**
	 *  Not open for upper layer.inner part will use these func.
	 */
	int                         get_scrwidth( void );
	int                         get_scrheight( void );
	int                         get_scrbpp( void );
	RECT                        get_windowBounds( void );
	bool                        get_fullScreen( void );

	/** \brief
	 *  �õ������豸��Stencil bits number.
	 */
	int                         get_stencilBitNum( void );

	/** \brief
	 *  �õ������豸��clip plane����Ŀ��
	 */
	int                         get_maxClipPlane( void );
	//@}

	/** \brief
	 *  �õ���ǰ���豸�Ƿ������Vertex Shader.
	 */
	bool                        is_softwareVS( void );

	/** \brief
	 *  �õ���ǰ����ͨ���崦���Ƿ�����Ӳ��TNL
	 */
	bool                        is_hardwareTL( void );

	//! �豸�Ƿ�֧������ѹ��.
	bool                        is_supportDXT2( void ) { return m_bSupportDXT2; }

	/** \brief
	 *  �Ƿ����ʹ��fogTable����.
	 */
	bool                        is_fogTableUsed( void ) { return m_bUseFogTable; }


	//@{
	//  �����й����صĽӿڡ�
	/** \brief
	 *  ʹ���ļ���������ꡣ
	 */
	int                         create_cursor( const char* _cursorName );
	//! ������������ʾ�Ĺ�ꡣ
	void                        set_cursor( int _id, int _centerX = 0, int _centerY = 0 );

	//! ������浱ǰ��ʾ�Ĺ��
	int                         get_cursor();

	//! �����Ƿ����ع�ꡣ
	void                        hide_corsor( bool _hide );

	//! ���ù���λ�á�
	virtual void                 set_cursorPos();
	//! ��Ⱦ��Ļ�ϵĹ�ꡣ
	void                        render_cursor( void );
	//@}  


	/** \brief
	 *  �õ� Indexed Vertex Blending Support��indexed Matrix����Ŀ.
	 */
	int                         get_iVBNum( void )     { return m_d3dCaps.MaxVertexBlendMatrixIndex/2; } 

	int							get_maxTextureWidth(void)
	{
		return m_d3dCaps.MaxTextureWidth;
	}
	int							get_maxTextureHeight(void)
	{
		return m_d3dCaps.MaxTextureHeight;
	}

	//! �õ�3d�豸���õ����blendStage��Ŀ
	int                         get_maxTextureBlendStages( void ) 
	{
		return m_d3dCaps.MaxTextureBlendStages;
	}
	//! �õ�һ��dip��ʹ������������Ŀ
	int                         get_maxTextureNum( void )
	{
		return m_d3dCaps.MaxSimultaneousTextures;
	}

	//! �Ƿ�֧���������ĵĶ����ͼ��ʽ.
	bool                        is_supportMlFlag( void );


	/** \brief
	 *  ����ȫ��Ļ����ߵ��ι��ܣ����ڻ�һ�����׵�Alpha��ͼ������Ⱦ
	 *  ˮ���ʱ�򣬲�����fractionһ���ֵ�͸����Ʒ
	 */
	bool                        draw_fullscrAlpha( float _alpha = 1.0f );

	//@{
	//! ����ȫ�ֵĵ�������.
	void                        draw_fullscrFade( void );
	/** \brief
	 *  ȫ��Ļ�ĵ��뵭����Ч.
	 *
	 *  \param DWORD _scolor,_ecolor  ���뵭��ʱ��ȫ��Ļ��ʼ��ɫ�ͽ�����ɫ,
	 *                                ��Ҫʹ����ɫ��alphaͨ��.
	 *  \param float _fadetime        ��������ʱ�Ķ���ʱ��.
	 */
	virtual void                start_fullscrFade( 
		                            DWORD _scolor,DWORD _ecolor,float _fadetime );
	/** \brief
	 *  �ϲ�õ�fade�İٷֱ�;
	 *  
	 *   ���ڿͻ��˴���һЩ�¼������Ҫfadeout����fadeIn��ʱ�򣬲�����Ч��������Ϣ�ȡ�
	 */
	virtual float        get_fullscrFadePercent( void );

	//@}

	HWND  get_hWnd();

#if __YY__
	bool						yy_init(void);

	virtual int					yy_msg_process(UINT _msg,WPARAM _wParam,LPARAM _lParam);
	virtual void				yy_show_openId(BOOL _show,int _x,int _y);
	virtual void				yy_set_callback(PFN_SETCALLBACK _callback);
	virtual void				yy_set_login_key(const char* _key);
	virtual void				yy_set_login_failed(const char* _why);
	virtual void				yy_set_login_succ(void);
#endif //__YY__

};




//
//  �����ʼ����Ҫ�ļ����򵥵�ȫ�ֱ���.


# include "deviceInit.inl"
		

