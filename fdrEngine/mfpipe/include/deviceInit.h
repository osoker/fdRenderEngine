//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: deviceInit.h
 *  Desc:     处理3d Device的初始化,得到一个可以被应用程序使用的3d device.
 *  His:      River created @ 4/14 2003
 *
 * “有做得到的事，也有做不到的事。”
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

//! 初始化临时显示纹理的数目.
//!
//!  ATTENTION TO FIX:临时改变，这个值过大肯定会有问题,以后处理。
# define MIN_TMPTEXTURE   (196*2)


//! 引擎中可以调入的最多的光标的数目。
# define  MAX_CURSORNUM        32

//! 定义光标的大小,在屏幕上占用的象素数目。
# define  CURSOR_PIXELSIZE     32


//! 用于确认开始和结束渲染3d场景的配对。
# define  BEGIN_DRAWTEST    1

//-------------------------------------------------------------------------
/** \brief
 *   用于临时显示纹理,比如调入场景时的结构.
 *  
 *   这个类直接使用3dDevice的Texture指针,在显示的时候可能需要使用多线程.s
 */
//-------------------------------------------------------------------------
struct os_tmpdisplayTex
{
public:
	LPDIRECT3DTEXTURE9      tex;
	bool                    inuse;

	int                   m_iRef;

	//! 创建 的纹理的名字.
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
 *  定义一个std::vector用于使用os_tmpdisplayTex.
 */
typedef  std::vector<os_tmpdisplayTex>  VEC_TMPDISPLAYTEX;

//! 定义显示到屏幕的数据结构。
typedef  std::vector<os_screenPicDisp>  VEC_spDisp;

//! 定义os_screenPicDisp结构的指针列表.
typedef std::vector<os_screenPicDisp*>  VEC_scrPicPtr;

//! 光标最多可以有8帧的动画
# define MAX_ANICURSORFRAME  8

//! 光标在设备内部的数据结构
class osc_mouseCursor
{
private:

	//! 每一个光标对应的纹理和设备相关数据
	VEC_TMPDISPLAYTEX    m_vecCursorTex;
	//! 光标动画的帧间切换时间
	float                m_fFrameTime;


	//! 上一帧对应的光标id
	int                  m_iLastFrameCursorId;
	//! 切换上帧光标动画已经过去的时间
	float                m_fEvaSwitchTime;

	//! 创建光标的名字.
	char                 m_szTexName[128];

	//! 光标的位置偏移量
	UINT				 m_cursorOffset_x;
	UINT				 m_cursorOffset_y;

private:
	//! 根据光标的名字，查找目录下对应的光标文件
	bool    process_cursorNum( const char* _name );

public:
	osc_mouseCursor();
	

	//! 重设当前的cursor数据结构
	void    reset( void );

	//! 根据传入的文件名，来创建光标序列文件
	BOOL    create_cursor( LPDIRECT3DDEVICE9 _dev,const char* _name );

	//! 上层对光标的设置调用
	void    set_cursor( LPDIRECT3DDEVICE9 _dev, int _centerX =0, int _centerY =0 );

	//! 每一帧对设备相关光标的设置
	void    frame_setCursor( float _eletime,LPDIRECT3DDEVICE9 _dev );

};



//! 调入等待画面上的提示信息最大长度。
# define  MAX_BKINFOLEN      256

//! 渲染屏幕多边形需要的数据结构。
class osc_scrPicRenderData
{
private:

	//! 屏幕多边形渲染时，使用的动态顶点缓冲区.
	LPDIRECT3DVERTEXBUFFER9  m_ptrVerBuf;

	//! 屏幕多边形渲染的时候，使用的静态顶点缓冲区.
	LPDIRECT3DINDEXBUFFER9   m_ptrIdxBuf;

	//! 全局三维设备的指针　。
	LPDIRECT3DDEVICE9        m_pd3dDevice;

	//! 要渲染的三角形的数量.
	int                      m_iRenderVerNum;

	//! 存放顶点的临时缓冲区.
	os_screenVertex*         m_ptrScrVer;

public:
	osc_scrPicRenderData();
	~osc_scrPicRenderData();

	void        init_scrPicRenderData( LPDIRECT3DDEVICE9 _dev );
	void        release_scrPicR( void );

	void        push_scrRenderVer( os_screenVertex* _ver );
	void        draw_scrRenderVer( void );

	//! 设备管理相关.
	void        scrpic_onLostDevice( void );
	void        scrpic_onResetDevice( void );

	//@{ 以下的变量用于在d3dMgr中对推进到渲染管道的屏幕多边形进行处理
	//! 用于批量的渲染客户端携进到管道中的屏幕多边形数据指针。
	VEC_scrPicPtr       m_vecScrPicPtr;

	//! 渲染屏幕多边形用到的临时变量.
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
	//! 高度和宽度
	int   m_iWidth;
	int   m_iHeight;
	D3DFORMAT format;

	//! 
	void  on_lostDevice( void );
	void  on_restoreDevice( LPDIRECT3DDEVICE9 _dev );

	//! 后端缓冲区的renderTarget.
	static LPDIRECT3DSURFACE9   m_pBackBufSurface;
};

class osc_middlePipe;

class osc_skinMeshMgr;

//-------------------------------------------------------------------------
//d3d初始化和release的类.这个类主要保存了当前渲染设备的兼容性等等信息.
//从这个类可以得到一个d3d渲染设备和这个设备的兼容性.
//如果要使用ogl初始化,替代这个类.
//
//
//d3d manager应该维持一个关于当前d3d device的兼容性列表.
//
//
//关于回调函数:
//因为每一个使用这个引擎的应用程序都需要设置一个确认设备的回调函数,
//把这个函数送入d3dmanager中进行调用,这个要在引擎文档中清晰写出.
//
//使用I_d3dmanager这个类来得到d3dmanager类的使用接口.
//
//-------------------------------------------------------------------------
/** \brief
 *  3d 设备的管理器类.
 *
 *  这个类是3d设备的真正的管理器,它从I_deviceManager接口继承而来,用
 *  direct3D的3d设备实现了I_deviceManager接口集的接口.
 * 
 */
class osc_d3dManager : public I_deviceManager
{
private:
	//! 是否全屏模式.
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
	//! 当前设备的3d Setting.
    CD3DSettings        m_d3dSettings;
	//! Parameters for CreateDevice/Reset
	D3DPRESENT_PARAMETERS  m_d3dpp;   


	//! 用于devicelost时,对device的 Reset.
	osc_middlePipe*       m_ptrMiddlePipe;
	osc_skinMeshMgr*      m_ptrSmMgr;


	//! 当前d3d Manager的窗口句柄.
	HWND                m_hWnd;
	//! Saved window bounds for mode switches
	RECT                m_rcWindowBounds;   
	
	//! Windows client用到的rect.
	RECT                m_rcClientBounds;

	//! 当前窗口的style.
	DWORD               m_dwWindowStyle;




	//! 当前屏幕的宽和高.
	int                 m_iScrWidth,m_iScrHeight;  
	//! 当前屏幕的色彩深度.
	int                 m_iScrBpp;    

	//! 当前已创建设备的stencil位数,设备初始化时创建最多位数的stencil,如果失败，
	//! 创建无stencil的设备。
	int                 m_iStencilBitsNum;

	//! 当前设备的最大clip plane数目。
	int                 m_iMaxClipPlane;




	//! 确认设备的函数指针,一定要由应该程序来设置这个指针.
	CFMPTR              *cfm_deviceptr;


	//!  Whether to show cursor when fullscreen
    bool                m_bShowCursorWhenFullscreen; 


	//!  用于显示文字的类.
	//CWFont               m_fontMgr;

	//!  是否隐藏硬件光标。
	bool                 m_bHideCursor;


	//! River added @ 2007-2-26:得到后端缓冲区并处理
	LPDIRECT3DSWAPCHAIN9    m_ptrSwapChain;

	//! River added @ 2010-3-1: 用于渲染多窗口。
	LPDIRECT3DSWAPCHAIN9    m_ptrAddSwapChain;
	D3DPRESENT_PARAMETERS   m_sAddSwapPara;

	//! River mod @ 2008-3-28:
	IDirect3DQuery9*     m_pEventQuery;

private:

	//!  当前设备的顶点处理状态.
	ose_VertexProcessingType   m_eVPT;

	//!  当前的设备是否可以使用fogTable
	bool                 m_bUseFogTable;

	//!  当前的设备是否支持DXT2纹理压缩.
	bool                 m_bSupportDXT2;

	/** \brief
	 *  得到设备的兼容性信息.
	 */
	void               get_deviceCaps( void );


	/** \brief
	 *  是否处于清空缓冲区，正在渲染状态。
	 *
	 *  清空缓冲区后，此时处于Inrender状态。Present后，处于非Inrender状态。
	 */
	bool               m_bInRender;


	//! 当前处于3d渲染状态。
	bool               m_bInDrawScene;

private:
	//@{
	/**
	 *  Used for display temp texture when we have not scene
	 *	to display.
	 *  
	 *  调入场景时的调入画面使用这个vector内的贴图。
	 *  
	 */
	VEC_TMPDISPLAYTEX   m_vecTexture;
	VEC_spDisp          m_vecSPDisp;
	float              m_fCurProgress;


	/** \brief
	 *  场景中用到的屏幕显示多边形。
	 */
	VEC_TMPDISPLAYTEX   m_vecSceneScrTexture;

	//syq_mask 场景中渲染目标列表
	std::vector< RENDERTARGET > m_vecRenderTarget;

	//! 渲染屏幕多边形用到的帮助类.
	osc_scrPicRenderData  m_sPicRD;



	//! 背景提示信息以及信息的显示位置。
	int                m_iInfoX,m_iInfoY;
	char               m_szBkInfo[MAX_BKINFOLEN];
	DWORD              m_dwBkInfoColor;

	//! 调入等待信息的相关显示
	int                m_iWaitInfoX,m_iWaitInfoY;
	char               m_szWaitInfo[128];
	DWORD              m_dwWaitInfoColor;

	//! 
	void               bkset_process( const char* _bgName,
		                      int _x,int _y,const char* _info,DWORD _infoColor  );
	//@}

	CSpaceMgr<osc_mouseCursor>   m_vecCursor;

	//! 当前激活的在屏幕上显示的光标id.
	int                m_iActiveCursorId;


	//! 渲染二维纹理用到的渲染状态块id.
	int                m_iScrTexRSId;
	int                m_iScrTexRSId_rt;

	//! River @ 2008-5-27:加入alpha和背景相加的功能
	int                m_iScrTexRSAddId;

	//@{
	// Windy mod @ 2005-9-20 16:05:38
	//Desc: 渲染二维纹理LensFlare用的渲染状态块ID
	int					m_iLensFlareRSId;

	//@}


	//@{
	//! 全屏幕fade需要的数据.
	bool               m_bFullScrFadeState;
	float              m_fFadeTime;
	float              m_fElaFadeTime;
	DWORD              m_dwStartColor;
	DWORD              m_dwEndColor;
	int                m_iFullScrFadeRSId;
	//@} 

	//@{
	//! 画屏幕多边形，让屏幕的alpha值为全白
	int                m_iFullscrAlphaId;
	//@}

private:
	/** \brief
	 *  初始化类中的变量.
	 */
	void                re_initvar( void );

	/** \brief
	 *  在窗口发生变化时调整窗口的style,比如从全屏变到局屏需要调整窗口style.
	 *
	 */
	void                AdjustWindowForChange( void );

	//! create the d3d query 
	void				CreateD3DQuery(void);


	//! 释放当前设备的additional swap chain.
	void                release_addSwapChain( void );


protected:

	/** \brief
	 *  找到一个可满足要求的device.
	 *
	 *  从已经建立的设备列表中找到一个可以满足要求的Device,对于不要求的项,比如SampleType
	 *  或是framerate,我们不需要在寻找Init device时考虑.如果找到设备,
	 *  填充变量m_d3dSettings.
	 * 
	 *  \param _initdata   要寻找设备的指标数据.
	 *  \return bool 如果找到设备填充内部变量m_d3dSettings,返回真.
	 *  \param int _times  第多少次查找设备
	 */
	bool                find_initdevice( const os_deviceinit* _initdata,int _time = 0 );


	/** \brief 
	 *  find_initdevice函数的帮助函数.
	 *
	 *  如果要寻找的设备是窗口设备,find_initdevice函数就调用这个函数来寻找适合的窗口
	 *  设备.
	 */
	bool                find_windowedDevice( const os_deviceinit* _initdata );

	//! 处理一些集成显卡的名字,使玩家知道为什么不能玩边缘.
	bool                is_onboardGraphicCard( const char* _des );


	/** \brief 
	 *  创建3d渲染环境.
	 *  
	 *  从已经填充的CD3DSettings结构中得到我们需要创建3D device需要
	 *  的各种参数,创建3d Device.
	 *
	 *  创建步骤:
	 *
	 *    1: 先填充 D3DPRESENT_PARAMETERS 结构.
	 *	 
	 *	  2: 创建3d device.
	 *
	 */
	bool                create_3denv( BOOL _bStartQuery );  


	/** \brief 
	 *  Fill the D3DPRESENT_PARAMETERS struct.
	 *
	 *  使用我们准备好的CD3DSettings结构去填充创建3D设备必须的D3DPRESENT_PARAMETERS
	 *  结构.要操作的变量为:m_d3dpp
	 */
	void                BuildPresentParamsFromSettings( void );


	/** \brief 
	 *  得到当前的d3d格式是16位还是32位.
	 *
	 *  从不同的d3d格式中得到这个显示模式是16位还是32位.
	 *  \param _format 要检测的格式.
	 *  \return int 返回16或是32,如果是返回0,出错.
	 */
	int                 formatlength( D3DFORMAT _format );

	//! 记录当前d3dMgr中需要的渲染状态块。
	void                record_scrTexRBlock( void );

	//! 记录当前d3dMgr中跟背景相加的渲染块
	void                record_scrAlphaAddBlock( void );

	
	//! 画renderTarget的状态
	void                record_scrTexRBlock_rt( void );//syq test code

	//! 记录全屏alpha时,需要的渲染状态块.
	void                record_fullScrFadeRBlock( void );

	//! 记录画全屏幕alpha的状态块渲染
	void                record_fullScrAlpha( void );

	//@{
	// Windy mod @ 2005-9-20 16:07:46
	//Desc: Lens Flare RS 
	void				record_LensFlareRBlock( void );

	//@}


	//! 初始化font相关数据,在初始化完全局的StateBlockMgr后调用此函数。
	void                        init_miskRes( void );


	//! 渲染一组屏幕多边形列表，列表中的数据应该有同样的纹理id.
	bool                        disp_scrPicArr( os_screenPicDisp** _tdisp,
		                                    int _disnum,int _texIdx );

	//! 如果是窗口模式，创建设备前，先调整窗口的大小和位置。
	void                        adjust_windowSize( HWND _hwnd,int _width,int _height ,bool _windowed);


public:
	osc_d3dManager();
	~osc_d3dManager();


	/** \brief 
	 *  创建我们需要的3d Device.
	 *
	 *  这个函数是I_deviceManager中的接口,实现步履:
	 *  
	 *  1: 创direct3d object.
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


	//! 重新设置d3dDevice的宽度，高度,用于上层调整游戏的分辨率
	virtual bool         reset_device( int _width,int _height,bool _windowed = true );

	/* \brief
	 * 创建额外的swapChain,用于多窗口渲染
	 *
	 * 当上层的窗口大小改变的时候，调用这个接口
	 * 如果此时内部已经有一个swapChain，并且大小跟当前的不一致，则释放内部已
	 * 存在的swapChain,然后重新创建一个swapChain.
	 *
	 * 如果 _hwnd == NULL ，就是释放交换链资源
	 */
	virtual bool         create_addSwapChain( int _width,int _height,HWND _hwnd );

	//! 设置中间管道的指针和人物管理器的指针.用于处理device reset的情况.
	void                 set_deviceResetNeedRes( 
		                     osc_middlePipe* _mptr,osc_skinMeshMgr* _cptr );

	
	/** \brief 
	 *  释放我们创建的3d Device.
	 *
	 *  在释放3d Device之前要把所有的3d资源都释放完.
	 *  
	 */
	virtual void         release_device( void );



	/** \brief 
	 *  使用d3d的硬件光标.
	 *
	 *  这个函数在主程序的消息WM_MOUSEMOVE消息中调用,调用这个函数使用d3d的硬件光标.
	 *
	 */
	virtual void         set_hwCursor( void );



	//  Uper layer should control these func??????如果需要再公开.
	//@{
	/**
	 *  Render control functions.所有的渲染都在beginDraw后发生,
	 *  并且在Present后结束(Present中包含有enddraw );
	 */
	virtual void         d3d_begindraw( void );
	virtual void         d3d_enddraw( void );
	//! river added @ 2010-3-1:用于多窗口渲染
	virtual void         d3d_addSwapClear( DWORD _Count,
                                     CONST D3DRECT* _pRects,
                                     DWORD _Flags,
                                     D3DCOLOR _Color );

	//!交换链是否已经准备好了
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
	 *  设置3d Device的Gamma 值.
	 *
	 *  Gamma只有真的三维全屏下才有效，而目前因为输入法的原因，我们使用了假的全屏。
	 *  
	 */
	virtual void         set_gamma( const os_gamma* _gamma );



	//@{
	/**
	 *  得到3d Deivce.
	 */
	virtual LPDIRECT3DDEVICE9   get_d3ddevice( void );
	virtual LPDIRECT3D9         get_d3d( void );
	//@}

	virtual int          get_availableTextureMem( void );
	
	//syq
	int		get_curUsedSceneScrTextureNum();


	/** \brief 
	 *  Reset 3d 渲染环境.
	 *
	 *  重新设置3d 渲染环境,比如从窗口模式到全屏模式,或是全屏模式不同Mode
	 *  的切换.
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
	 *  \param _tdisp   要显示的数据结构。
	 *  \param _disnum  要显示的数据结构为指针，这个数据控制有多少个元素需要显示。
	 *  \param _texId   要使用哪一个纹理array: 目前有三种Array 0: deviceMgr.
	 *                  1:场景中创建的纹理。  1: 图标纹理。
	 *  
	 */
	virtual bool         disp_inscreen( const os_screenPicDisp* _tdisp,
		                      int _disnum = 1,int _texIdx = 0 ,bool _LensPic = false);

	/** \brief
	 *  渲染和背景混合相加的图片
	 */
	virtual bool         disp_alphaAddScrPic( const os_screenPicDisp* _tdisp,int _idx,int _texIdx );


	//@{
	/** \brief
	 *  设置调入游戏或其它长时间任务的背景画面。 
	 * 
	 *  背景画面使用统一的图片规格，使用640*480的真实图片大小，把这张图片
	 *  切分成四张256*256的图加两张128*256的图。
	 *  并且使用一张256*64的调入框。
	 *
	 *  \param _bgName 传入的是界面目录的名字，在这个目录下存入了背景图片的名字。
	 *  \param _x,_y,_info 设置背景时，这些参数可以在背景的某个位置设置一个提示信息,
	 *                     这个提示信息在显示背景的过程中，一直存在.
	 *  \param _infoColor   设置背景的提示信息时，提示信息的字串颜色
	 */
	virtual bool         set_backGround( const char* _bgName,int _x = 0,
		                  int _y = 0,const char* _info = NULL,
						          DWORD _infoColor = 0xffffffff );

	/** \brief
	 *  设置符任务进展。
	 *
	 *  最大值为1.0,这个进展将拖动我们任务进展条。
	 *
	 *  \param _addps 这个数据是往原来进度上加入的进度，刚开始时进度为0,如果此值
	 *                加上系统内部的进度大于1.0,则系统使用1.0的进度。
	 *
	 *  \param _x,_y  任务等待画面可以设置提示信息，此为提示信息的位置。
	 *  \param _info  任务提示信息,为空则不显示信息。
	 *  \param _infoColor 提示信息字串的颜色.
	 */
	virtual void         set_progress( float _addps,int _x,int _y,
		                  const char* _info = NULL,DWORD _infoColor = 0xffffffff );
	//@}




	/** \brief
	 *  为场景中需要显示的二维图片创建纹理。
	 */
	int                  create_sceneScrTex( const char* _filename );
	
	LPDIRECT3DTEXTURE9   get_sceneScrTex( int _id );
	//syq_mask 创建渲染目标
	int                create_renderTarget( int _width, int _height, D3DFORMAT _format = D3DFMT_A8R8G8B8 );

	//syq_mask 获得渲染目标
	LPDIRECT3DSURFACE9 get_renderTarget( int _id );
	LPDIRECT3DTEXTURE9 get_renderTexture( int _id );




	//! test注册一个纹理,返回id
	int					register_texid( LPDIRECT3DTEXTURE9 _pTexture );
	void				unregister_texid( int _texid );
	int register_targetAsTex( int _targetIdx );

	//! 创建一个纹理
	int					 create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture );

	//! 释放单个的场景中的二维图片纹理。
	void                 release_sceneScrTex( int _texid );

	/** \brief
	 *  释放所有的场景中使用二维图片纹理。
	 *
	 *  \param _realRse 是否真的释放，如果是假的释放，如果调入的纹理
	 *                  在纹理的名字内存在，则返回已经存大的纹理。
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
	 *  这个函数是引擎显示文字信息的主要的函数,必须支持Unicode.
	 *  
	 *  最后测试性能,加入高效率的字符显示函数.
	 * 
	 */
	virtual int          disp_string( os_stringDisp* _dstr,int _dnum,int _fontType = 0,bool _useZ = false, bool _bShadow = false, RECT* _clipRect = NULL );

	//! 立即在屏幕上显示字串，参数跟disp_string参数一致。
	virtual int          disp_stringImm( os_stringDisp* _dstr,int _fontType = 0,os_screenPicDisp* _cursor = NULL );
		                    

	/** \brief
	 *  对当前的屏幕进行截图.
	 *  使用日期加当前的时间做为截图的名字.
	 */
	virtual char*         catch_screen( void );

	//! 得到后端缓冲区的描述.
	const D3DSURFACE_DESC*  get_backBufDesc( void ){ return &m_d3dsdBackBuffer; } 

	//! 得到主表面.
	virtual bool get_mainRenderTarget( LPDIRECT3DSURFACE9& _target );


	//! 得到显卡的信息
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
	 *  得到创建设备的Stencil bits number.
	 */
	int                         get_stencilBitNum( void );

	/** \brief
	 *  得到创建设备的clip plane的数目。
	 */
	int                         get_maxClipPlane( void );
	//@}

	/** \brief
	 *  得到当前的设备是否是软件Vertex Shader.
	 */
	bool                        is_softwareVS( void );

	/** \brief
	 *  得到当前的普通物体处理是否是用硬件TNL
	 */
	bool                        is_hardwareTL( void );

	//! 设备是否支持纹理压缩.
	bool                        is_supportDXT2( void ) { return m_bSupportDXT2; }

	/** \brief
	 *  是否可以使用fogTable做雾化.
	 */
	bool                        is_fogTableUsed( void ) { return m_bUseFogTable; }


	//@{
	//  程序中光标相关的接口。
	/** \brief
	 *  使用文件名创建光标。
	 */
	int                         create_cursor( const char* _cursorName );
	//! 设置引擎中显示的光标。
	void                        set_cursor( int _id, int _centerX = 0, int _centerY = 0 );

	//! 获得引擎当前显示的光标
	int                         get_cursor();

	//! 设置是否隐藏光标。
	void                        hide_corsor( bool _hide );

	//! 设置光标的位置。
	virtual void                 set_cursorPos();
	//! 渲染屏幕上的光标。
	void                        render_cursor( void );
	//@}  


	/** \brief
	 *  得到 Indexed Vertex Blending Support的indexed Matrix的数目.
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

	//! 得到3d设备可用的最多blendStage数目
	int                         get_maxTextureBlendStages( void ) 
	{
		return m_d3dCaps.MaxTextureBlendStages;
	}
	//! 得到一个dip能使用最多的纹理数目
	int                         get_maxTextureNum( void )
	{
		return m_d3dCaps.MaxSimultaneousTextures;
	}

	//! 是否支持我们旗帜的多层贴图方式.
	bool                        is_supportMlFlag( void );


	/** \brief
	 *  加入全屏幕画多边的形功能，用于画一个纯白的Alpha底图，在渲染
	 *  水面的时候，不会误fraction一部分的透明物品
	 */
	bool                        draw_fullscrAlpha( float _alpha = 1.0f );

	//@{
	//! 测试全局的淡出淡入.
	void                        draw_fullscrFade( void );
	/** \brief
	 *  全屏幕的淡入淡出特效.
	 *
	 *  \param DWORD _scolor,_ecolor  淡入淡出时的全屏幕起始颜色和结束颜色,
	 *                                需要使用颜色的alpha通道.
	 *  \param float _fadetime        淡出淡入时的动画时间.
	 */
	virtual void                start_fullscrFade( 
		                            DWORD _scolor,DWORD _ecolor,float _fadetime );
	/** \brief
	 *  上层得到fade的百分比;
	 *  
	 *   用于客户端处理一些事件，如快要fadeout或是fadeIn的时候，播放特效，发送消息等。
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
//  引擎初始化需要的几个简单的全局变量.


# include "deviceInit.inl"
		

