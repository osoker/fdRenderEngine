//=======================================================================================================
/**  \file
 *   Filename: osInterface.h
 *   Desc:     Osok显示引擎的头文件,如果要使用Osok引擎,包含这个头文件.里面有向外部公开的所有的结构
 *             和接口.
 *   His:      River created @ 4/14 2003.
 *
 *   "法拉利小轿车只不过是一种物质享受,做人最重要的是真诚."
 *  
 */
//=======================================================================================================
# pragma  once

#ifndef DIRECTINPUT_VERSION 
#define DIRECTINPUT_VERSION 0x0800
#endif

# pragma warning( disable : 195 )


//! 定义导出符。
#ifdef FDRENGINE_EXPORTS
#define OSENGINE_API __declspec(dllexport)
#else
#define OSENGINE_API __declspec(dllimport)
#endif


/** 
 *  不使用common_mfc版本的common库，在memoryManager.h的第一行，把
 *   # define TRACKMEM_ALLOCK  0
 *   宏的值定义为0, 则编译可以在mfc下使用的common和渲染库，设为1,则
 *   全局的重载new和delete. 
 */
# include "../../common/com_include.h"

//! RIVER added @ 2009-5-4:针对QTang项目的一些优化和处理。
# define __QTANG_VERSION__   1


//! 是否编译为刀光编辑器的版本，在刀光编辑器的装备，才可以编辑人物的动作和装备
# define __GLOSSY_EDITOR__   0


//! 如果使用特效编辑器，需要加入一些引擎用不到的函数。
# define __EFFECT_EDITOR__   1

//@{
// Windy mod @ 2005-9-1 19:18:01
//Desc: ride 是否使用坐骑编辑器，需要加入一些引擎用不到的函数。
# define __RIDE_EDITOR__	1
//@}

//! 是否使用movie Record的功能。
# define __MOVIE_RECORD__    0

//! 用于一些文件是否打包调入，比如spe.list,xml.list等等.
# define __PACKFILE_LOAD__   1

//! 调入纹理是否受256大小的限制。
# define __TEXTURE_SIZELIMIT__   0

//! 是否编译为可以编辑人物第二层贴图的版本.
# define __CHARACTER_EFFEDITOR__  1

# define  __LENSFLARE_EDITOR__		1

# define	__YY__ 0

// 加入dx9.0有Error处理库
# pragma comment(lib,"dxerr9" )


//@{
//! 包含常用的函数和宏。
# include "vformat_macro.h"
//@}



//! 声音和背景音乐播放相关的头文件
# include "smPlayerInterface.h"

# include "fileOperate.h"
# include "../YYcommon/DemoInterface.h"


class IFontManager
{
public:
	virtual ~IFontManager(){};
	virtual int  RegisterFont(const char* _fontName,int height,int flWeight= FW_LIGHT) = 0;
};
extern OSENGINE_API IFontManager* gFontMgrPtr;

//enum E_FontType
//{
//	EFT_DefaultType = 0,
//	EFT_Font1,
//	EFT_Font2,
//	EFT_Font3,
//	//! 可添加新的
//	EFT_COUNT
//};


//==================================================================================================
//
//  接口或是用于接口的结构类型,使用纯虚函数来定义接口,再在其它的类中实现这些接口.
//  但外部的程序或是使用这个库的程序只能使用或是操作接口.
//
//==================================================================================================

// Class declear.
struct I_camera;



// 在miskFunc.h头文件中定义
class OSENGINE_API os_aabbox;
class OSENGINE_API os_bsphere;
class OSENGINE_API os_bbox;

//! 用于银河项目的小功能
//
//1: 平面阴影投射功能，仅用于地表是平面的项目
//2: 
# define GALAXY_VERSION  0


//--------------------------------------------------------------------------------
//@{	tzz added for galaxy exploding shader effect
//--------------------------------------------------------------------------------
#if  GALAXY_VERSION
/*! \struct galaxy explode struct
*	\brief : initialize the explode data in geomipmap.fx
*	this struct exports for galaxy shader effect to generate
*	exploding in 
*
*	I_fdScene::gala_genExplode(...) 
*
*	function
*
*	caution!!
*	this struct must align with fx file's struct(geomipmap.fx)
*	can NOT change one and not change the other...
*/
typedef struct OSENGINE_API _gala_explode
{	
	//! explode position
	osVec3D		vecExplodePos;

	//! speed of effect(animation?)
	float		fSpeed;

	//! radius of wave expand
	float		fRadiusOffset;

	//! swing of explode wave
	float		fSwing;

	//! the cycle of explode wave
	float		fCycle;

	//! timer
	float		fTime;

}gala_explode;

/*!	\struct galaxy space hole (suck power) struct
*	\brief : initialize the bullet shader effect
*	this struct will store in osc_mapTerrain by array
*	and refresh every loop in fx file
*	calling :
*
*	I_fdScene::gala_refreshSpaceHole(...) 
*
*	function
*
*	caution!!
*	this struct must align with fx file's struct(geomipmap.fx)
*	can NOT change one and not change the other...
*/

typedef struct OSENGINE_API _gala_spaceHole
{	
	//! this space hole position
	//! caution!! 
	//! this bullet position need NOT refresh every 
	//! game loop
	osVec3D		vecHolePos;
	
	//! radius of wave expand
	float		fRadiusOffset;
	
	//! the depth of part space
	float		fDepth;

}gala_spaceHole;

#endif //GALAXY_VERSION
//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------

/**  \brief
 *   Gamma set used struct.
 */
struct OSENGINE_API os_gamma
{
	//@{
	/**
	 * 和D3D的Gamma结构数据相同.
	 */
	WORD        m_arrRed[256];
	WORD        m_arrGreen[256];
	WORD        m_arrBlue[256];
	//@}
};







# if __CHARACTER_EFFEDITOR__
//! 外层使用的，用于更新人物第二层纹理特效的数据结构。
struct OSENGINE_API os_chaSecEffStruct
{
	/** \brief
	 *  第二层贴图的shader方式。
	 *
	 *  目前只支持两种id: 
	 *  5: 球形环境贴图。
	 *  6: 移动纹理贴图。  
	 *  7: 自发光纹理贴图。每三层纹理必须为空。
	 */
	WORD      m_wEffectType;

	//@{
	//! 第二层贴图矩阵用到的数据。
	//! River @ 2010-2-25:这个数据在在自发光纹理模式中，代表自发光纹理变亮变暗
	//                    一个周期的时间。
	float     m_fRotSpeed;

	/** \brief
	 *  旋转最好不使用除绕Z轴(0,0,1)以外的其它轴旋转的方式，
	 *  因为旋转时纹理会有一些特殊的情况出现,
	 */
	osVec3D   m_vec3RotAxis;

	//! 纹理X,Y坐标的移动速度。
	float     m_fUMoveSpeed;
	float     m_fVMoveSpeed;
	//@} 

	//! 当前第二层贴图的纹理名字。
	char      m_szSecTexName[64];
	//! 第三层贴图的纹理名.
	char	  m_szThdTexName[64];

	//! 第四层贴图的纹理名.
	char      m_szFourthTexName[64];

};
# endif 



/**  \brief
 *   用于初始化3dDevice的结构,可以加入更多的控制.
 *   这个结构也用于Reset3d Device.
 *
 *   5/20 2003
 *   因为这个结构是和引擎交互的结构,所以可以加入其它
 *   一些引擎初始化需要的数据.
 *
 *   5/31 2003
 *   引擎初始化需要的数据由另外一个ini文件einit.ini负责.
 *
 */
struct OSENGINE_API os_deviceinit
{
	//! 是不是全屏,在目前假全屏程序机制下，这个值必须设置为false才行。
	bool         m_bFullScr;

	//! 是不是使用d3d的硬件光标.
	bool         m_bHwCursor;
	//! 全屏时屏幕宽度.
	DWORD        m_dwScrWidth;
	//! 全屏时屏幕高度.
	DWORD        m_dwScrHeight;
	//! 全屏时屏幕颜色位数.
	DWORD        m_dwColorBits;
	//! 全屏时最低要求的Zbuffer位数
	DWORD        m_dwDepthBits;
	//! 最低要求的Stencil位数.
	DWORD        m_dwStencilBits;
	//! 使用Query系统，降低性能，防止抖动
	BOOL		 m_bStartQuery;
	
	//! 初始化完成设备创建后要显示的图片。
	char         m_szDisTexName[128];
	//! 初始化图片放置的位置，x方向上的浮点值，占整个屏幕的宽度。
	float        m_fStartX;
	//! y方向上的浮点值，占一个屏幕的高度。
	float        m_fStartY;

	//! 屏幕刷新率 add syq
	int          m_displayFrequency;

	//  River mod @ 2008-5-23:
	//! 不修改视口大小
	BOOL         m_bResetViewPortSize;

	//
	os_deviceinit() 
	{
		m_bResetViewPortSize = TRUE;
		m_bStartQuery = FALSE;
	}
};



/** \brief
 *  显示temp屏幕多边形时要填充的结构.  
 * 
 *  这种显示方式是最简单的显示方式,由d3dmanager
 *  真接支持,不需中间管道的支持.
 */
class OSENGINE_API os_screenPicDisp
{
public:
	//! 要显示的BillBoard的四个顶点.
	os_screenVertex  m_scrVer[4];

	//! 要显示的Texture的ID.
	int             tex_id;

	/** \brief
	 *  要显示图片的blend mode.1:直接渲染 2:Invsrccolor 3: SrcAlpha
	 * 
	 *  因为引擎中使用tga做为贴图，所以目前引擎只支持第三种混合方式。
	 *  就是blend_mode的值必须为3.
	 *  River @ 2008-5-27:加入srcalpha和背景相加的功能,
	 *          即D3DRS_SRCBLEND 为srcalpha, D3DRS_DESTBLEND为 one
	 *          使用blend_mode值为5
	 */
	int             blend_mode;

	//! 是否打开Z缓冲，用于渲染一些需要折挡的图片
	BOOL            m_bUseZBuf;

public:
	// 初始化blend_mode 为3,顶点uv为从0到1.
	os_screenPicDisp();
};

//! 用于更新人物第二层纹理的数据结构
struct os_chaSecEffect;

/** \brief 
 *  用于初始化一个Camera的数据结构.
 *
 *  使用这个结构来初始化一个相机.初始化一个相机后,
 *  在程序运行过程中可以使用这个相机的接口更改这个
 *  相机的内部数据.
 *
 */
struct OSENGINE_API os_cameraInit
{
	//! 当前相机的位置.
	osVec3D      curPos;

	//! 相机的焦点
	osVec3D      camFocus;

	//@{
	/**
	 *  用于设置ProjMatrix的数据.
	 */
	float        nearp;
	float        farp;
	float        fov;
	float        faspect;
	//@}

	//@{
	/**
	 *  用于设置ViewPort的数据.
	 */
	int          tlx;
	int          tly;
	int          wid;
	int          hei;
	float        min_z;
	float        max_z;
	//@}	

};






/** \brief
 *  使用DirectInput得到的mouse data.使用自己的格式.
 *
 */
struct  OSENGINE_API os_dInputdata
{
	//! 相当于dinput中的:DIMOUSESTATE2结构.
	struct os_DImousedata
	{
		long   lx;
		long   ly;
		long   lz;
		BYTE   rgbButtons[8];
	};
    
	//! 从Dinput中得到的Mouse数据.
	os_DImousedata   mousedata;

	//! 从Dinput中得到的键盘数据.
	char             keydata[256];
};



//! 定义字体颜色解晰用到的关键字。


/** \brief
 *  在屏幕上渲染加入了颜色解晰符的字串。
 *
 *  象HTML一样，可以根据解晰符的不同，显示出不同的字串颜色。
 */
class OSENGINE_API os_colorInterpretStr
{
private:
	//@{ River mod @ 2006-2-27:用于超长字串的处理
	//! 固定的缓冲区
	char           m_szStaticSzBuf[256];
	s_string       m_szDynamicBuf;
	//@}

	// syq *2 @ 2005.2.24
	const char*    m_szCIStr;

public:

	//@{
	//! 字串在屏幕上的起始位置和在三维中的Z值。
	int            m_iPosX,m_iPosY;
	float          m_fZ;
	//@}
	//! 字符的高度.英文字符宽度是高度的一半，中文字符高度和宽度一样。
	int             charHeight;

	/** \brief
	 *  每一行的所能达到的最大X相素值。
	 *
	 *  默认情况下，此数据的值为-1,表示不需要换行。
	 */
	int            m_iMaxPixelX;

	//! Y方向上的最大相素索引值。
	int            m_iMaxPixelY;

	//! 如果需要换行，行间矩的大小，以实际屏幕中的相素数目计。
	int            m_iRowDis;

	/** \brief
	 *  返回值，让上层得到信息，此字串在给定的框架限制中显示了多少行。
	 *
	 *  如果强制显示打开时，此值包含了那些在框架矩形内不显示的行。
	 */
	int            m_iResRowNum;

	/** \brief
	 *  是否强制显示出字串，如果是强制显示，如果要显示的行所覆盖的相素
	 *  索引超过了我们给出的区域，则只显示区内可以显示到的部分。
	 *
	 *  否则，不渲染当前的字串并返回。
	 */
	BOOL           m_bForceDis;


public:
	os_colorInterpretStr();
	os_colorInterpretStr& operator = ( const char* _dstr );
	//! 得到字串内容
	const char*           get_ciStr( void ) { return m_szCIStr;}
};


/** \brief
 *  在3d窗口内显示文字需要填充的结构.
 *
 *  和其它的3d元素渲染一样,显示文字也需要批处理,所以我们要填充一个
 *  显示文字的结构.可以使用Unicode.
 */
class OSENGINE_API os_stringDisp
{
private:
	s_string        m_szDynamic;
	char            m_szStatic[32];

	const char*     dstring;
	//@}

public:
	DWORD           color;
	int             x,y;


	//! 渲染当前的字体时,使用的z值,默认为0.0f，在渲染中最靠前面。
	float           z;

	//! 字符的高度.英文字符宽度是高度的一半，中文字符高度和宽度一样。
	int             charHeight;

	/** \brief
	 *  文字背景对应的颜色值。
	 *
	 *  格式为:0xffffffff,其中前两个ff代码当前背景颜色的透明度
	 *  ，比如想使用半透明的红色背景框，颜色值应为：0x88ff0000
	 *  
	 */
	DWORD           m_dwBackColor;


public:
	//! 加入其它显示字串需要的数据.
	os_stringDisp();

	//! 为当前的结构赋一个string的值.
	os_stringDisp& operator = ( const char* _dstr );

	//! 得到内部的字串　
	const char*    get_sdStr( void ) { return dstring; } 

};

typedef std::vector<os_stringDisp> VEC_stringDisp;



/** \brief
 *  得到键盘信息的帮助宏.
 *  
 *  使用这个宏得到某个按键是不是被按下.
 */
#define OS_KEYDOWN(name, key)            (name[key] & 0x80)



////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      开始骨骼系统和他的管理器相关类,调入*.BFS文件
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
*  骨骼的索引。
*/ 
struct OSENGINE_API os_boneIdx
{
	int   m_iParentIdx;
	int   m_iBoneIdx;
};

typedef std::vector<os_boneIdx>  VEC_boneIdx;

/** \brief
*  存放骨骼信息的临时结构。
*
*  *.bfs文件的调入信息存放结构，到内存后处理为对所有的Frame进行
*  连接。
*  
*/
struct OSENGINE_API os_boneSys
{
	//! 当前骨骼系统的名字。　
	s_string    m_strBoneSysName;

	//! River added @ 2010-12-21:加入hash值的比较.
	DWORD       m_dwHash;

	//! 当前骨骼系统中骨骼的数目。
	int         m_iBoneNum;

	//! 骨骼名字列表。主要用于调试。
	std::vector<const char*>   m_vecBoneNameStr;

	//! 骨骼的id列表。
	VEC_boneIdx  m_vecBoneIdx;
public:

	//! 从文件中调入一个骨骼系统。
	bool        load_boneSysFromFile( const char* _fname );

	//! 从一个骨骼ID得到骨骼的名字.
	const char* get_boneNameFromId( int _id );

	//! 从一个骨骼ID得到此骨骼父骨的名字.
	const char* get_parentBoneNameFromId( int _id );
};

//! Bonesys' stl vector.
typedef std::vector<os_boneSys>  VEC_boneSys;
/** \brief
*  当前系统中骨骼系统的管道器
*
*  在调入一个新的人物动画时，通过管理器来调入骨骼系统，
*  这样在内存中已经存大的骨骼系统就不需要在从硬盘上调入了。
*
*  ATTENTION: 因为场景中用到的骨骼系统是有限的，所以可能会
*             场景中所有的骨骼系统放到一起初始化，这样在游戏 
*             运行过程中，就不需要从硬盘上调入骨骼系统文件了。
*/
class OSENGINE_API osc_boneSysMgr
{
private:
	//! 骨骼系统的容器.
	VEC_boneSys       m_vecBoneSys;

	//! 当前系统中骨骼系统的数目。
	int              m_iBoneSysNum;

public:
	osc_boneSysMgr();
	~osc_boneSysMgr();

	/** \brief
	*  从骨骼系统管理器中调入一套骨骼。
	*/
	os_boneSys*   create_boneSysFromMgr( const char* _fname );

	/** \brief
	*  释放当前的骨骼管理器。
	*/
	void         release_boneSysMgr( void );

};

//! d3d_present 的参数
struct OSENGINE_API os_present
{
	//! 目标窗口
	HWND			m_destWnd;

	//! 源矩形
	const RECT*		m_pScrRect;

	//! 目标矩形
	const RECT*		m_pDestRect;

	os_present(void):m_destWnd(NULL),m_pScrRect(NULL),m_pDestRect(NULL){}
};

//===================================================================================
//
/** \brief
 *  3d Deivce管理器的接口.
 * 
 *  3d Deivce的管理器接口,外部库操作这些接口,可以初始化和释放3d Device,并对Device进行操作.
 *  主程序中必须要对这个接口进行操作才可以进入3D显示模式. 
 */
//
//===================================================================================
struct OSENGINE_API I_deviceManager
{
	/**
	 *  Device init&release.从文件中初始化或是从结构中初始化一个d3d.
	 *  
	 *  \param _hwnd            传入的窗口句柄.
	 *  \param _initdata        从这个结构中得到3dDevice的初始化参数.
	 *  \param _cfm_ptr         Confirm Device的回调函数,函数原型见
	 *                          这个类之产的函数指针声明:   CFMPTR.   
	 *  \return bool            如果创建成功,返回true.
	 */
	virtual bool         init_device( HWND _hwnd,const os_deviceinit* _initdata,
		                              void* _cfm_ptr ) = 0;

	//! 重新设置d3dDevice的宽度，高度,用于上层调整游戏的分辨率
	virtual bool         reset_device( int _width,int _height,bool _windowed = true ) = 0;

	/* \brie3f
	 * 创建额外的swapChain,用于多窗口渲染
	 *
	 * 当上层的聊天窗口大小改变的时候，调用这个接口，内部已经做好相应的处理，直接创建新的
	 * swapChain并释放原来的swapchain.
	 *
	 * 如果 _hwnd == NULL ，就是释放交换链资源
	 */
	virtual bool         create_addSwapChain( int _width,int _height,HWND _hwnd ) = 0;


	/**
	 * 释放3d设备的函数.
	 */
	virtual void         release_device( void ) = 0;
    

	/** \brief 
	 *  使用d3d的硬件光标.
	 *
	 *  这个函数在主程序的消息WM_SETCURSOR消息中调用,调用这个函数使用d3d的硬件光标.
	 *
	 */
	virtual void         set_hwCursor( void ) = 0;
	//! 响应WM_MOUSEMOVE消息，对硬件光标进行处理。
	virtual void         set_cursorPos() = 0;


	/** 
	 *  设置全屏幕的Gamma值.
	 *  \param _gama 要传入的Gamma参数,见os_gamma.
	 */
	virtual void         set_gamma( const os_gamma* _gama ) = 0;

	//@{
    //uper layer should not control these func??????如果需要再公开.
	/**
	 *  Render control functions.所有的渲染都在beginDraw后发生,
	 *  并且在Present后结束(Present中包含有enddraw );
	 */
	virtual void         d3d_begindraw( void ) = 0;
	virtual void         d3d_enddraw( void ) = 0;

	//! river added @ 2010-3-1:用于多窗口渲染
	virtual void         d3d_addSwapClear( DWORD Count,
                                     CONST D3DRECT* pRects,
                                     DWORD Flags,
                                     D3DCOLOR Color ) = 0;

	//!交换链是否已经准备好了
	virtual BOOL		d3d_isSwapPrepare(void) const = 0;

	virtual void         d3d_clear(  DWORD Count,
                                     CONST D3DRECT* pRects,
                                     DWORD Flags,
                                     D3DCOLOR Color,
                                     float Z,
                                     DWORD Stencil ) = 0;
	virtual DWORD        d3d_present( const os_present* = NULL,bool _reset = true ) = 0;
    //@}

# if __MOVIE_RECORD__
	//@{
	//! 开始和结束录制电影。
	virtual bool         begin_recordMovie( const char* _aviName ) = 0;
	virtual void         end_recordMovie( void ) = 0;
	//@}
# endif 

	/** \brief
	 *  得到当前设备所有的纹理显存的大致数量。
	 *
	 *  以m为单位。
	 */
	virtual int          get_availableTextureMem( void ) = 0;

	//syq
	virtual int		get_curUsedSceneScrTextureNum()=0;


	//@{
	/**
	 *  Maybe used for other way.such as play animation movie at the 
	 *  begin of Application.
	 */
	virtual LPDIRECT3DDEVICE9   get_d3ddevice( void ) = 0;
	virtual LPDIRECT3D9         get_d3d( void ) = 0;
    //@}




	/** \brief
	 *  全屏幕的淡入淡出特效.
	 *
	 *  \param DWORD _scolor,_ecolor  淡入淡出时的全屏幕起始颜色和结束颜色,
	 *                                需要使用颜色的alpha通道.
	 *  \param float _fadetime        淡出淡入时的动画时间.
	 */
	virtual void         start_fullscrFade( 
		                     DWORD _scolor,DWORD _ecolor,float _fadetime ) = 0;

	/** \brief
	 *  上层得到fade的百分比;
	 *  
	 *   用于客户端处理一些事件，如快要fadeout或是fadeIn的时候，播放特效，发送消息等。
	 */
	virtual float        get_fullscrFadePercent( void ) = 0;



    /**********************************************************************/
	//
	// Use following func to display string of Engine.
	//
	/**********************************************************************/


	//@{
	/** \brief
	 *  设置调入游戏或其它长时间任务的背景画面。 
	 * 
	 *  背景画面使用统一的图片规格，使用640*480的真实图片大小，把这张图片
	 *  切分成四张256*256的图加两张128*256的图。
	 *  并且使用一张256*64的调入框。
	 *
	 *  \param _bgName 传入的是界面目录的名字，在这个目录下存入了背景图片的名字。
	 *  \param _x,_y,_info 设置背景时，这些参数可以在背景的某个位置设置一个提示信息。
	 *  \param _infoColor   设置背景的提示信息时，提示信息的字串颜色
	 */
	virtual bool         set_backGround( const char* _bgName,int _x =0,
		                     int _y = 0,const char* _info = NULL,
							   DWORD _infoColor = 0xffffffff ) = 0;

	/** \brief
	 *  设置符任务进展。
	 *
	 *  最大值为1.0,这个进展将拖动我们任务进展条。
	 *  \param _addps 这个数据是往原来进度上加入的进度，刚开始时进度为0,如果此值
	 *                加上系统内部的进度大于1.0,则系统使用1.0的进度。
	 *  \param _x,_y  任务等待画面可以设置提示信息，此为提示信息的位置。
	 *  \param _info  任务提示信息,为空则不显示信息。
	 *  \param _infoColor 提示信息字串的颜色.
	 */
	virtual void         set_progress( float _addps,int _x,
		                  int _y,const char* _info = NULL,DWORD _infoColor = 0xffffffff ) = 0;
	//@}


	/** \brief
	 *  对当前的屏幕进行截图.
	 *  使用日期加当前的时间做为截图的名字.
	 */
	virtual char*         catch_screen( void ) = 0;

	//! 得到后端缓冲区的描述.
	virtual const D3DSURFACE_DESC*  get_backBufDesc( void ) = 0;

	//! 得到主表面.
	virtual bool get_mainRenderTarget( LPDIRECT3DSURFACE9& _target ) = 0;

	//! 得到显卡的信息
	virtual const char* get_displayCardInfo(void)const = 0;


#if __YY__

	virtual int					yy_msg_process(UINT _msg,WPARAM _wParam,LPARAM _lParam) = 0;
	virtual void				yy_show_openId(BOOL _show,int _x,int _y) = 0;
	virtual void				yy_set_callback(PFN_SETCALLBACK _callback) = 0;
	virtual void				yy_set_login_key(const char* _key) = 0;
	virtual void				yy_set_login_failed(const char* _why) = 0;
	virtual void				yy_set_login_succ(void) = 0;
#endif //__YY__


};




//===================================================================================
/** \brief
 *  对DirectInput的包装类.
 * 
 *  和渲染引擎完全分离的一个部分,由上层程序把他们组合到一起.
 *  Input的管理器包括 mouse&keyboard.
 */
 //===================================================================================
struct OSENGINE_API I_inputMgr
{
	/** \brief
	 *  InputManager的初始化函数.
	 *
	 *  \param _hInst  创建这个InputMgr程序的实例.
	 *  \return bool   如果创建成功,返回真.
	 */
	virtual bool               init_inputmgr( HINSTANCE _hInst,HWND _hWnd ) = 0;
	/** \brief 
	 *  释放这个Input Manager.
	 */
	virtual bool               release_inputmgr( void ) = 0;

	/** \brief
	 *  从当前的Input Manager得到数据的函数.
	 *
	 */
	virtual os_dInputdata*     get_inputdata( void ) = 0;

	//@{
	/**
	 *  在程序一开始和我们按下ALT+TAB键的时候需要调用这两个函数.
	 *  具体见DirectInput的文档.
	 */
	virtual bool               acquire( void ) = 0;
	virtual bool               unacquire( void ) = 0;
	//@}

};


//! 上层注册网络断开后需要调用的函数
typedef int (*sockClosed_callbackFunc)( void* _para );


//===================================================================================
/** \brief
 *  为上层提供网络接口。
 *
 *  这个接口提供了最基础的网络功能。
 */
//===================================================================================
struct OSENGINE_API I_socketMgr
{

	/** \brief
	 *  连接到服务器的接口。
	 *
	 *  \param   _hostAddr   目标服务器的地址，比如"202.196.168.0"
	 *  \param   _port       目标服务器的端口。
	 *  \param   _WSA        当网络事件发生时，主程序得到的消息。
	 *  \param   _hwnd       主程序的窗口句柄。
	 */
	virtual bool         connect_server( const char* _hostAddr,int _port,int _WSA,HWND _hwnd ) = 0;
	/** \brief
	 *  关闭当前的连接。
	 */
	virtual bool         close_socket( void ) = 0;

	//! 当前的Socket是否关闭
	virtual bool         is_socketClosed( void ) = 0;

	//@{
	/** \brief
	 *  读入数据，返回数据指针。
	 *
	 *  \param _msgSize    读入的消息数据的长度。
	 *  \param _errorCode  保留用于如果出错，返回的错误类型。
	 *  \return char*      返回的数据指针。
	 */
	virtual char *        read_message( WORD& _msgSize,int* _errorCode,int* _errorType ) = 0;
	//@}

	//读取消息缓冲区，返回NULL即为读取完毕
	virtual char* read_buffer( WORD& _msgSize ) = 0;

	//@{
	//  River @ 2005-10-6: 删除批量发送网络消息的接口，客户端没有用到此接口。
	//! 发送单条的指令，最常用的函数。
	virtual bool          send_oneMsg( const char* _msg,WORD _size ) = 0;
	//@}

	//! 设置为脱机模式，此模式下，send_oneMsg永远返回true.
	virtual bool          set_offLineState( bool _offLine ) = 0;

	//! 注册网络关闭后调用的客户端回调函数 
	virtual bool          register_socketCloseCallback( sockClosed_callbackFunc _func ) = 0;

	/*! 获得发送数据的总长度
	*	
	*	\param _dwDataLenHight			: DWORD长度的高位
	*	\param _llDataLen				: 总的长度
	*	\return DWORD					: DWORD长度的低位
	*/
	virtual DWORD		  get_sendDataLen(DWORD* _dwDataLenHight = NULL)const = 0;

	/*! 获得接受数据的总长度
	*	
	*	\param _dwDataLenHight			: DWORD长度的高位
	*	\param _llDataLen				: 总的长度
	*	\return DWORD					: DWORD长度的低位
	*/
	virtual DWORD		  get_recvDataLen(DWORD* _dwDataLenHight = NULL)const = 0;

	/*! \brief 加密算法的Key，由服务器最先发送过来。
	*
	*	\param _key						: key value
	*	\param _len						: key length;
	*/
	virtual void		  set_cryptKey(char* _key,int _len) = 0;
};



//! 对相机进行操作时，相机返回的操作结果。
enum ose_camOperateRS
{
	//! 相机正处于动画中，不能进行当前操作。
	OSE_CAMIN_ANIMATION = 0,

	//! 相机不能压的过低。
	OSE_CAMPITCH_TOO_LOW,
	//! 相机最多可以达到垂直看场景。
	OSE_CAMPITCH_TOO_HIGH,

	OSE_CAMOPE_SUCCESS,
};

//===================================================================================
/** \brief 
 *  为上层提供可操作的Camera的接口.
 *
 *  这个Camera提供了上层可以操作的接口,但在实现这个接口的类里面也有一些接口是用
 *  于引擎内部的.这些用于引擎内部的接口不必要公开到上层使用.
 *
 */
//===================================================================================
struct OSENGINE_API I_camera
{
	/**********************************************************************/
	//
	//  初始化Camera的函数.
	//
	/**********************************************************************/
	/** \brief 
	 *  从一个结构中初始化一个Camera.
	 */
	virtual void         init_camera( const os_cameraInit* _cdata )   = 0;

	//! 得到相机可看到的远剪切面的矩离，即场景可见的最远矩离
	virtual float        get_farClipPlaneDis( void ) = 0;
	virtual float		 get_nearClipPlaneDis( void ) = 0;

	
	//! 相机位置与相机焦点的最大距离
	virtual void          set_maxDistance( float _maxDis ) = 0;

	//! 相机位置与相机焦点的最小距离
	virtual void          set_minDistance( float _minDis ) = 0;


	/**********************************************************************/
	//
	//  操纵Camera需要的接口.
	//
	/**********************************************************************/
	/** \brief
	 *  设置一个Camera的Projection Matrix
	 *
	 *  \param _nearp,_farp   近远剪切面的距离。
	 *  \param _fovh          相机fov的值。field of view.
	 *  \param _faspect       屏幕的(宽/高)的值。
	 */
	virtual void         set_projmat( float _nearp,
		                    float _farp,float _fovh,float _faspect ) = 0;


	/** \brief
	 *  设置远剪切面.
	 *  \param _farp   数值最小为128.0f,最大为280.0f
	 */
	virtual void         set_farClipPlane( float _farp ) = 0;


	/** \brief
	 *  设置一个Camera的ViewPort.
	 *
	 *  不同的Camera可能需要不同的ViewPort,可能出现多个Camera多个
	 *   ViewPort的情景(画中画).
	 *
	 *   _tlx,_tly        Viewport在屏幕左上角的顶点.
	 *
	 *   _wid,_hei        Viewport的宽和高。
	 *
	 *   _min_z,_max_z    Viewport的最大和最小Z值.
	 */
	virtual void         set_viewport( int _tlx,int _tly,
		                    int _wid,int _hei,float _min_z,float _max_z ) = 0;

	/** \brief
	 *  重新设置camear的aspect.
	 *
	 *  \param _aspectAdd 当前相机的aspect加上这个数据。内部把_aspectAdd设置为
	 *                    改变后的aspect数据，并传出.
	 */
	virtual void         set_camAspect( float& _aspectAdd ) = 0;


	//! 设置相机的fov值。
	virtual void         set_cameraFov( float _fov ) = 0;
	//! 得到现有的相机视角宽度.
	virtual float        get_cameraFov( void ) = 0;


	/** \brief
	 *  得到相机当前的指向的方向,不管相机处于何种模型,加入了方向的
	 *  分量.
	 */
	virtual void         get_godLookVec( osVec3D& _vec ) = 0;

	/** \brief
	 *  相机的前后移动.输入正值，则相机的位置离焦点更远。
	 */
	virtual void         move_close( float _length ) = 0;

	/** \brief
	 *  获得相机的位置到焦点的距离
	 */
	virtual float        get_disFromCamposToFocus() = 0;


	/** \brief
	 *  得到当前相机的向右的方向分量.
	 */
	virtual void         get_rightvec( osVec3D& _vec ) = 0;

	/** \brief
	 *  设置相机当前的位置.
	 *
	 *   \param bool _force 引擎内部使用参数。
	 */
	virtual void         set_curpos( osVec3D* _vec,bool _force = false )  = 0;

	//! 2009-9-4:一次性设置位置和focus,用于解决一个很变态的屏幕晃动的bug.
	virtual void         set_curPosFocus( osVec3D& _pos,osVec3D& _focus,bool _force = false ) = 0;


	/** \brief
	 *  得到相机当前的位置.
	 */
	virtual void			 get_curpos( osVec3D* _vec )  = 0;
	virtual const osVec3D&	 get_curpos( void) = 0;

	/** \brief
	 *  相机的焦点操作
	 */
	virtual void         set_camFocus( const osVec3D& _focus ) = 0;
	virtual void         get_camFocus( osVec3D& _focus ) const = 0;
	virtual const osVec3D& get_camFocus( void) const = 0;


	/** \brief
	 *  使相机绕一个顶点进行水平旋转。
	 *　
	 *  \param bool _force 强制性的旋转,引擎内部使用。
	 */
	virtual ose_camOperateRS  rotate_horizon( const osVec3D& _ver,
		                   float _radians,bool _force = false )  = 0;

	/** \brief
	 *  使相机绕一个顶点进行上下旋转。
	 *
	 *  \param bool _force 强制性的旋转,引擎内部使用。
     *  \return 
	 */
	virtual ose_camOperateRS  rotate_vertical( const osVec3D& _ver,
		                   float _radians,bool _force = false ) = 0;




	/**********************************************************************/
	//
	//  使用相机中的数据进行一些常用量的计算.
	//
	/**********************************************************************/
	//@{
	/** \brief
	 *  得到屏幕上一个二维坐标在相机三维空间中的向量.
	 */
	virtual osVec3D      get_upprojvec( const osVec2D* _svec,osMatrix& _wmat ) = 0; 
	virtual osVec3D      get_upprojvec( int _x,int _y,osVec3D& _dir ) = 0;
	//@} 


	/** \brief
	 *  给出一个世界空间的顶点,给出当前世界空间顶点对应的屏幕空间的顶点.
	 *
	 *  \param _wpos 传入的世界空间的顶点坐标,此坐标为绝对坐标.
	 *  \param _spos 返回了世界空间顶点对应的屏幕坐标.
	 *  
	 */
	virtual bool          get_scrCoodFromVec3( const osVec3D& _wpos,osVec3D& _spos ) = 0;


	//! 从场景中调入camera,仅表示相机的位置和旋转方向等数据，跟视口无关。
	virtual bool          load_camera( const char* _cname,
		                       bool _ani = false,float _time = 0.0f ) = 0;

	/** \brief
	 *  对相机的旋转和Pitch做动画。
	 *  
	 *  普通用途：相机与地表碰撞时，相机可以柔和的到达新位置
	 *  旋转和pitch都是绕相机的焦点坐标进行。
	 */
	virtual bool          animate_camRotPicth( 
		                   float _rotRad,float _pitchRad,float _time ) = 0;


	virtual void  set_autoResetState( bool _autoReset ) = 0;

	/** \brief
	 *  机测一个顶点是否在相机的视域内
	 */
	virtual bool          is_ptInCamVF( osVec3D& _pos ) = 0;


# if __EFFECT_EDITOR__
	//! 存储场景中的camera位置数据。
	virtual bool          save_camera( const char* _cname ) = 0;
# endif 

	//syq 2005.4.3 播放相机运动的特效
	virtual void play_camEffect( char * _camEffectName, int _nCntPerSec ) = 0;


	/** \brief
	 *  限制俯仰角为一固定值
	 *
	 *  \param _rad 固定的俯仰角度值(弧度),如里此值小于等于零，则使用当前
	 *              相机的pitch值做为锁定的pitch值
	 */
	virtual void   limit_pitch( float _rad ) = 0;

	/** \brief
	 *  限制俯仰角为一固定区间值
	 *
	 *  \param _rad     最直的相机视角.
	 *  \param _radMin  最平的相机视角.
	 *  \param _minFDis 相机离焦点可以达到的最近矩离，在此位置上，相机使用最平的视角。
	 *  \param _maxFDis 相机离焦点可以达到的最远矩离，在此位置上，相机使用最直的视角。
	 */
	virtual void   limit_pitch( float _rad,float _radMin,float _minFDis,float _maxFDis ) = 0;


	/** \brief
	*  限制俯仰角为一固定值 ,之前的一个值
	*/
	virtual void   limit_pitch( void ) = 0;

	/** \brief
	*  取消限制俯仰角为一固定值
	*
	*  \param _rad 固定的俯仰角度值(弧度)
	*  
	*/
	virtual void unlimit_pitch() = 0;

	/** \brief
	 *  使相机振动的接口
	 *  
	 *  \param float _fDamp		相机振动阻尼.
	 *  \param float _swing      相机的振幅.
	 */
	virtual void shake_camera( float _fDamp,float _swing ) = 0;

	//! 得到相机viewFrustum的8个顶点,这个八个顶点分别是: xyz,Xyz,xYz,XYz,xyZ,XyZ,xYZ,XYZ
	virtual const osVec3D* get_vfPt( void ) = 0;

	//! 保存autoResetState.
	virtual void  save_autoResetState( void ) = 0;
	//! 恢复autoResetState.
	virtual void  restore_autoResetState( void ) = 0;


};




//===================================================================================
//! 骨骼动画最多可有拥有的mesh数目。
# define MAX_SKINPART      8

/** \brief
 *  七个身体部位的显示效果,由不同的文件名来调入这些显示效果。
 *   
 *  所有骨骼动画的物品最多可以使用七个身体部位。
 *  
 */ 
struct OSENGINE_API os_bodyLook
{
	//! mesh数目
	int      m_iPartNum;     
	
	//! 身体部位和顶点和皮肤的索引，默认从1开始,传入0表示此部位为空，
	//! 什么都不需要显示。
	WORD     m_mesh[MAX_SKINPART];
	WORD     m_skin[MAX_SKINPART];

};

/** \brief
*  变换装备需要的数据结构。
*/
struct OSENGINE_API os_equipment
{
	//! 需要多少个部分更换显示相关资源。
	int           m_iCPartNum;

	//! 需要替换的部位的id.部位id从0开始。
	int           m_arrId[MAX_SKINPART];
	//! 需要替换的meshId.meshId从1开始.
	int           m_arrMesh[MAX_SKINPART];
	//! 需要替换的skinId.SkinId从1开始。
	int           m_arrSkin[MAX_SKINPART];
};

/** \brief
 *  初始化一个skinMesh物品时使用的结构。
 */
class OSENGINE_API os_skinMeshInit
{
public:

	//! 要初始化的skinMesh调入数据的目录。
	char          m_strSMDir[32];


	//! 要初始化的物品的显示信息。
	os_bodyLook    m_smObjLook;

	//! 插入这个object的位置。
	osVec3D        m_vec3Pos;

	//! 当前skinMesh的总的动作套数。
	int            m_iTotalActTypeNum;

	//! 默认要插入的skin mesh Object的动作id.
	//! 这个id从零开始.
	int            m_iActIdx;

	//! 如果不是使用动作索引,则播放默认的动作名字.
	char           m_szDefaultAct[32];


	//! 当前人物的旋转角度。
	float          m_fAngle;

	//@{
	// 人物调整值。
	//! 人物的旋转方向调整。
	float          m_fAdjRotAgl;
	//! 人物位置微调。
	osVec3D        m_vec3AdjPos;

	//! 人物BoundingBox的大小。
	osVec3D        m_vec3Max;
	osVec3D        m_vec3Min;
	//@ 


	//! 当前动画中的透明部分是否需要写入ZBuffer.
	bool           m_bWriteZbuf;

	/** \brief
	 *  当前的动画是否支持实时阴影.
	 *
	 *  游戏中只有主角人物可以使用此项, 阴影使用的计算量太大。
	 *  此值默认为false,不支持阴影。
	 */
	bool           m_bShadowSupport;

	//! 平面阴影投射
# if GALAXY_VERSION
	bool           m_bSupportProjectShadow;
# endif 

	// River added @ 2005-6-5: 测试场景中透明物品没有明暗面的效果。
	/** \brief
	 *  是否使用场景中的方向光做为当前skinMesh的全局光。
	 *
	 *  用于那些全部是透明面的skinMesh物品，使物品不显示明暗面，整体上使用场景
	 *  中最亮的光做为照到物体上的光。
	 * 
	 *  默认这个变量为假。
	 */
	bool           m_bUseDirLAsAmbient;
	//@{
	// Windy mod @ 2005-8-30 19:06:00
	//Desc: ride 只有是坐骑此值才为真
	bool			m_bIsRide;
	//!用于坐骑上的人物的微调位置。
	osVec3D			m_vec3RideOffset;

	//@}

	//! 使用多层贴图的部位索引，从零开始，一个character只能有一个身体部位使用多层旗帜贴图
	int             m_iMlFlagIdx;


	//! 人物身上绑定特效名字和偏移，windy add 7.25
	//s_string		m_speFileName;
	char			m_szspeFileName[64];
	osVec3D			m_spePostion;
	//! 2007-1-29 winyd add 加入人物缩放值
	float			m_Scale;

	//! 初创建的人物，是否不可见
	bool            m_bInitFade;

	//  
	//  River added @ 2009-12-30:
	//! 是否此人物要渲染成为界面上使用的人物，人物附加的特效使用此值。
	bool            m_bUseInInterface;

	//! 初始化调入武器特效？
	bool            m_bPlayWeaponEffect;

	//! 默认参数设置
	os_skinMeshInit();

	//! 设置
	void load_equipment(const os_equipment* _equ);
};






/** \brief
 *  替换新动作的方式。
 */
enum  ose_ActionChangeType
{
	//! 替换旧的动作。
	OSE_DIS_OLDACT = 0,

	//! 按次数播放新的动作,播放完成后继续播放原来的动作。
	OSE_DIS_NEWACT_TIMES = 1,

	
	/** \brief
	 *  按次数播放新的动画，播放完成动作后使用新动作的
	 *  最后一帧显示skinMesh.显示一段时间后，消失。
	 *
	 */
	OSE_DIS_NEWACT_LASTFRAME = 2,


};

/** \brief
 *  播放新动作的结构。
 */
class OSENGINE_API os_newAction
{
public:

	//! 要播放的新动作的索引。新动作的索引从0 开始.
	int                   m_iNewAct;

	/** \brief
	 *  要播放的新动作的名字。
	 *
	 *  可以使用动作的名字去播放动作，但必须把要播放动作的索引设置为-1.
	 *
	 *  如果找不到可以播放的动作，则使用第零套动作做为默认动作.
	 *  
	 */
	char                  m_strActName[32];

	/** \brief
	 *  要随机播放的动作名字
	 *  
	 *  只有m_changeType为OSE_DIS_OLDACT时，此变量才起作用。
	 *  引擎会随机的播放m_strActName和这个变量内名字的动作。
	 *  最多可以在四个动作中随机播放。
	 *  如果需要随机播放"单手攻击1"和"单手攻击2"，则把m_strActName赋值为"单手攻击1",
	 *  把m_strRandActName[0]赋值为"单手攻击2"。动作的名字不能超过24个字符
	 *  
	 *  os_newAction的建构函数会把m_strRandActName[0][0],m_strRandActName[1][0],
	 *  m_strRandActName[2][0]赋值为NULL.
	 */
	char                  m_strRandActName[3][24];

	/** \brief
	 *  最后播放的动作,永久性的播放下去
	 *
	 *  如果此值不为空,则按次数播放完动作后,播放这个动作,一直播放下去.
	 *  
	 */
	char                  m_strNextActName[32];


	/** \brief
	 *  换动作的方式。
	 */
	ose_ActionChangeType   m_changeType;


	/** \brief
	 *  只有当m_changeType的值为OSE_DIS_NEWACT_TIMES或
	 *  OSE_DIS_NEWACT_LASTFRAME时，这个值才有效。
	 *  
	 *  表示播放新动作的次数.
	 */
	int                   m_iPlayNum;

	/** \brief
	 *  如果是OSE_DIS_NEWACT_TIMES播放方式，播放完成后,是播放上一个动作,
	 *  还是播放原来正常循环播放的动作.
	 */
	bool                  m_bPlayLoopAct;

	//! 新动作的播放速度,如果使用系统默认速度,则不理这个变量就好了.
	//! 此值为播放完新动作需要的时间.
	float                 m_fFrameTime;

	//! 是否在特定的停止帧处停止动作的继续播放,默认情况下不停止。
	bool                  m_bStopAtStopFrame;

public:
	os_newAction();

};


/** \brief
 *  武器对应的特效文件名。
 *  
 *  River @ 2009-12-9:一个武器特效可以对应三个种类的特效各一
 *  每一个种类，可以为空。相当于特效组合可以每一个都有一类。
 *  三个特效元素，可以任意组合，可以全是粒子，全是osa,全是公告板
 *  或者这三种特效元素的任意组合。
 */
struct OSENGINE_API I_weaponEffect
{
	char     m_szEffObjName[3][48];
public:
	I_weaponEffect()
	{
		m_szEffObjName[0][0] = NULL;
		m_szEffObjName[1][0] = NULL;
		m_szEffObjName[2][0] = NULL;
	} 

	//  
	//! 赋值处理
	I_weaponEffect& operator=( const I_weaponEffect& _b )
	{
		strcpy( m_szEffObjName[0],_b.m_szEffObjName[0] );
		strcpy( m_szEffObjName[1],_b.m_szEffObjName[1] );
		strcpy( m_szEffObjName[2],_b.m_szEffObjName[2] );

		return *this;
	}

	// River added @ 2010-8-11:当前weapon特效是否存在.
	bool is_weExist( void ) const
	{
		if( (m_szEffObjName[0][0]== NULL)&&
			(m_szEffObjName[1][0]== NULL)&&
			(m_szEffObjName[2][0]== NULL) )
			return false;
		else
			return true;
	}
};

/** \brief
 *  上层操作skinMesh物体的接口。
 *
 *  在创建的时候初始化.
 *  
 */
struct OSENGINE_API I_skinMeshObj
{

	//! 用于标识第几部分,WEAPON7是右手武器，WEAPON8是左手武器
	enum WEAPON_ID {
		WEAPON7 = 6,
		WEAPON8 = 7,
	};


	//! enable post trail state
	virtual void			enable_postTrail(bool _enabled = true) = 0;

	//! exclude light effect
	virtual void			exclude_light(bool _exclude = true) = 0;

	//! is exclude light effect 
	virtual bool			is_excludeLight(void) = 0;

	//! River added @ 2009-6-7:当前的skinMesh接口操作是否可操作.多线程调入skinMesh
	virtual bool             legal_operatePtr( void ) = 0;

	//! River added @ 2009-8-26: 隐藏人物的即时阴影.
	virtual bool             hide_realShadow( bool _hide = true ) = 0;


	/** \brief
	 *  改变当前skinMeshObject的动作.
	 *
	 *  \param  _act     要插入新动作需要填充的结构。  
	 *  \return float    返回要播放的新动作的播放过程需要的时间，以秒为单位。
	 */
	virtual float            change_skinMeshAction( os_newAction&  _act ) = 0;

	//! 使停止状态的动作继续播放。
	virtual void             start_moveAction( void ) = 0;


	/** \brief
	 *  人物动作是否到达当前动作的关键帧
	 *
	 *  到达关键帧之前返回值为假，到达之后返回值为真。
	 *  
	 */
	virtual bool             arrive_stopFrameOrNot( void ) = 0;
	//! 得到从动作开始到动作停止帧所用的时间。
	virtual float            get_timeFromStartToSF( void ) = 0;



	/** \brief
	 *  替换skinMesh的装备　.
	 */
	virtual bool            change_equipment(const  os_equipment& _equi ) = 0;

	/** \brief
	 *  释放当前的skinMesh所占用的资源。
	 *  
	 *  释放后当前的接口不能够再用.
	 * 
	 *  \param _sceneC 内部使用的参数，外部调用不需考虑
	 */
	virtual void            release_character( BOOL _sceneC = false ) = 0;


	/** \brief
	 *  得到当前skinMesh在世界空间中的位置和旋转角度.
	 * 
	 *  \param _pos    返回skinMesh在世界空间的位置.
	 *  \param _fangle 返回skinMesh在世界空间的旋转角度.
	 */
	virtual void            get_posAndAgl( osVec3D& _pos,float& _fangle ) = 0;

	/** \brief
	 *  得到手部的矩阵，主要用于游戏中的特效处理.
	 *
	 *  使用此处得到手部矩阵，来设置上层需要的特效的位置。
	 *  只有设置了刀光数据的人物模型才可以得到手部的矩阵。
	 *  
	 */
	virtual void            get_handPos( osVec3D& _pos ) = 0;

	//! 测试得到某一根骨骼的位置.
	virtual void            get_bonePos( const char* _boneName,osVec3D& _pos ) = 0;

	//! 得到当前skinMesh的旋转方向，
	virtual float           get_smAgl( void ) = 0;

	//! 上层更新特效时用到的旋转方向，这个是渲染用到的真正方向。
	virtual float           get_renderSmAgl( void ) = 0;


	/** \brief
	 *  更新当前的skinmeshObj的位置。
	 *
	 *  此函数仅用于修改skinMesh物体的位置和旋转角度.
	 *  引擎内部的skinObjFramemove函数对skinMesh做出处理。
	 *  
	 *  \return float 返回当前人物的当前动作点在“正在播放的当前动作进程”中的时间百分比。
	 *                比如当前正在播放走路动作，如果走路动作完了三分之一，则返回0.333333
	 */
	virtual float           frame_move( osVec3D& _npos,float _nangle ) = 0;

	//! 得到当前动作播放所完成的比例,例：攻击动作完成40%，则返回0.4
	virtual float           get_actionPlayPercent( void )  = 0;
	//! 得到当前正在播放的动作名字和索引
	virtual int             get_curPlayActIdx( char _name[32] ) const= 0;

	//! 得到某个动作事件点的百分比，比如受创特效需要在受创动作的播放多少时播放。
	virtual float           get_actionTriggerPoint( char _name[32],int _idx = -1 ) = 0;


	/** \brief
	 *  当前人物总共有多少套动作可以播放。
	 */
	virtual int             get_totalActNum( void ) = 0;

	//! 设置fade状态。
	virtual void            set_fadeState( bool _fadeS,float _alphaV = 1.0f ) = 0;

	/** \brief
	 *  从动作的名字得到播放当前动作需要的时间。
	 *  
	 *  \param _actName 传入要处理的动作名字。
	 *  \param _actIdx  返回当前动作名字对应的动作索引，主要用于刀光编辑器中
	 *               
	 */
	virtual float           get_actUsedTime( const char* _actName,int& _actIdx ) = 0;

	/** \brief
	*  从动作的名字得到播放当前动作需要的时间。
	*/
	virtual float           get_actUsedTime( const char* _actName ) = 0;

	/** \brief
	 *  从动作的id得到播放当前的动作需要的时间.
	 */
	virtual float           get_actUsedTime( int _id ) = 0;

	/** \brief
	 *  隐藏当前skinMesh的显示。
	 *
	 */
	virtual void            set_hideSkinMesh( bool _hide ) = 0;

	//! is hide state?
	virtual bool			is_hideState(void)const = 0;


	//! 隐藏当前人物身上绑定的特效
	virtual void            hide_followEffect( bool _hide ) = 0;


	/** \brief
	 *  从当前的character得到一个boundingBox.
	 */
	virtual os_bbox*        get_smBBox( void ) = 0;

	/** \brief
	*  从当前的character得到一个boundingBox.
	*/
	virtual const os_bbox*        get_smBBox( void )const = 0;

	/** \brief
	 *  角色可以使用刀光.
	 *
	 *  \param bool _glossyEnable 是否打开刀光显示。
	 *  \param bool _weaponEffEnable 是否打开武器特效的显示
	 */
	virtual void            playSwordGlossy( bool _glossyEnable,bool _weaponEffEnable = true ) = 0;

	//! Set VIP Load，多线程调入，当前skinObj放在队列的最前面，最后设置的char优先级别最高.
	virtual void            set_vipMTLoad( void ) = 0;


	/** \brief
	 *  切换人物的武器特效
	 *
	 *  /param int _idx           切换到第_idx个特效，超出索引的话，都使第零个特效。
	 *  /param WEAPON_ID _weapid  使用第七还是第八个武器部位。
	 */
	virtual void            change_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;

	//! 根据索引，得到当前weapon特效的文件名。
	virtual const I_weaponEffect*  get_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;


	//! 得到当前帧刀光的中心点。
	virtual void            get_swordGlossyCenter( osVec3D& _pos ) = 0;

	/** \brief
	 *  传入一个环境光值，系统和当前的场景环境光值做比较，使用更亮的光照亮当前角色。
	 *
	 *  因为是使用最亮的光照去照亮当前的skinMesh，所以要是想使用原来的场景
	 *  环境光照亮人物的话，只要把传入的osColor的r,g,b,a都设置为0.0f就可以了。
	 */
	virtual void            set_maxBright( osColor& _bcolor ) = 0;

	/** \brief
	 *  在屏幕一个二维区域上渲染当前的人物,这个渲染使用Orthogonal投影矩阵。
	 *
	 *  \param _rect  要在哪个屏幕区域内渲染人物。
	 *  \param _zoom  用于控制相机离人物远近的数据,内部把人物包围盒的中心点放在osVec3D( 0.0f,0.0f,0.0f)的
	 *                位置上，相机放在osVec3D( 0.0f,0.0f,-_zoom)的位置上。上层可以调整出一个合适的值，在
	 *                实时的渲染中一直使用这个值。
	 *  \param float _rotAgl 用于二维界面内人物的旋转方向。
	 *  \param _newChar 是否是新创建的人物，如果是新创建的人物接口，则这个人物的动作与三维场景内的人物没有
	 *                  任何的关系
	 *	\param _camera 如果这个参数有效，就不适用全局的相机指针惊醒渲染
	 */
	virtual void            render_charInScrRect( RECT& _rect,float _rotAgl,
		                       float _zoom,BOOL _newChar = FALSE ,I_camera* _camera = NULL) = 0;

	//@{ windy add 2006-04-13
	virtual void			get_Default_ActName(std::string& _ActName) = 0;
	virtual void			get_Act_List(std::vector<std::string>& _ActNameList) = 0;
	//@}

	/** \brief
	 *  暂停当前人物的动作。
	 *
	 *  \param bool _suspend 如果为true,则暂停动作，如果为false,则继续动作。
	 */
	virtual void            suspend_curAction( bool _suspend ) = 0;


# if __GLOSSY_EDITOR__

	/** brief
	 *  只显示某个身体部位的部分三角形,公用于调试目的
	 *
	 *  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
	 *  \param int _trinum 如果想显示某个部位全部的三角形索引(三角形索引也从零开始)，则传入此值为零即可。
	 *  \return int        传入_trinum为零的时候，返回了这个身体部位最多的三角形数目　
	 */
	virtual int             displaytri_bysel( int _paridx,int _triidx[24],int _trinum ) = 0;

	//! 设置人物刀光拖尾的长度。有效数据是6-12，如果大于或是小于此值，使用默认的拖尾长度
	virtual void            set_sgMoveLength( int _len ) = 0;

	// 得到刀光拖尾的长度数据
	virtual int             get_sgMoveLength( void ) = 0;


	//  以下函数只在编辑刀光的编辑器中使用。
	/** \brief
	 *  把当前sm中的武器文件存入到新的文件中。
	 */
	virtual bool            save_weaponFile( WEAPON_ID _weapid = WEAPON7) = 0;

	/** \brief
	 *  设置第N个武器特效文件，最多4个。
	 * 
	 *  \param int _idx 设置第几个武器特效，从零开始，最大值为3.超过3的话，内部会变为零
	 *  \param char* _effName 武器特效的文件名
	 *  \param WEAPON_ID _weapid 设置第七个还是第八个武器部位的特效。I_weaponEffect
	 */
	virtual bool            set_weaponEffect( int _idx,
		                      const I_weaponEffect& _effName,WEAPON_ID _weapid = WEAPON7 ) = 0; 

	//! 调整当前人物手中刀光的位置。
	virtual bool            adjust_swordGlossySize( bool _start,float _size , WEAPON_ID _weapid = WEAPON7) = 0;

	//! 在X方向上旋转刀光。
	virtual void            rot_swordGlossyX( float _radian , WEAPON_ID _weapid = WEAPON7) = 0;
	//! 在Y方向上旋转刀光。
	virtual void            rot_swordGlossyY( float _radian , WEAPON_ID _weapid = WEAPON7) = 0;

	//! 在X轴或是Y轴上移动刀光的位置。
	virtual void            move_swordGlossy( bool _x,float _dis , WEAPON_ID _weapid = WEAPON7) = 0;

	//! 设置此武器用于定位，不显示此武器。
	virtual void            set_displayWeapon( bool _dis , WEAPON_ID _weapid = WEAPON7) = 0;    

	//! 重设刀光的各种旋转角度，使完全回到调整前的刀光
	virtual void            reset_swordGlossyPosRot( WEAPON_ID _weapid = WEAPON7 ) = 0;

	/** \brief
	 *  设置刀光对应的动作索引, 并且设置每一个动作对应的颜色数据。
	 *
	 *  \param _actIdx    表示哪一个动作需要显示刀光。
	 *  \param const char* _glossyTex 传入了刀光使用纹理的名字，这个纹理必须存在于
	 *                                引擎运行目录下的efftexture目录下，比如
	 *                                "efftexture/glossy.hon" 传入这个参数应该是"glossy"
	 *  \param _color     每一个动作刀光对应的颜色,这个颜色乘以纹理的颜色是最后刀光显示
	 *                    出来的颜色.如果想要完全的显示出纹理的颜色,则设置这个颜色值为
	 *                     纯白色,即0xffffffff的数值.
	 */
	virtual void            set_swordGlossyActIdx( int _actNum,int* _actIdx,
		                          DWORD* _color,const char* _glossyTex , WEAPON_ID _weapid = WEAPON7) = 0;

	/** \brief
	 *  得到人物的当前武器对应刀光数据。
	 *  
	 *  刀光的位置信息还需要微调。
	 *  \param int& _actNum 返回有多少个动作使用了刀光.
	 *  \param int* _actIdx 传入一个整型数组，这个数组必须使用至少12元素，传出了使用刀光的
	 *                      的动作索引。
	 *  \param char* _glossyTex 上层传入的字串，必须是一个64字节的char型数组，传回了当前
	 *                          武器使用的刀光纹理名字。
	 *  
	 */
	virtual void            get_swordGlossyActIdx( int& _actNum,int* _actIdx,
		                            DWORD* _color,char* _glossyTex , WEAPON_ID _weapid = WEAPON7) = 0;

	/** \brief
	 *  设置skinMesh当前武器附加的刀光特效。
	 *  
	 *  \param _pos      从0.0f到1.0f的数值，表示从刀光开始和刀光结束位置之间的位置。
	 *                   particle,billboard以这个位置做为特效放置的位置.osa特效以这个
	 *                   位置做为osa在max中(0,0,0)点对应的位置.
	 *               
	 *  \param I_weaponEffect& _effFName 对应的特效数据结构，只能有一种类型
	 *
	 *  注意:目前支持的武器附加特效中,osa文件是特殊的种类,其它的两类文件跟随武器是
	 *       以"点"的形式跟随,即只能附在武器的某个"点"之上,跟方向是无关的.
	 *       osa文件与武器的跟随是以"线"的形式跟随的,跟随武器的osa文件在max中制做的时候,
	 *       以max中朝上的方向,即Z轴为标准方向,以(0,0,0)点为原点.在武器上附加的此特效会
	 *       把此osa的标准方向与武器的方向和位置保持一致.
	 *
	 *  River @ 2009-12-9: _effFname修改成结构的形式，以用于同一个武器对应多个特效元素。
	 *                     如果_effFname.m_szEffObjName[0][0] == NULL,则此武器不对应特效。
	 */
	virtual void            set_sgEffect( float _pos,
		                       const I_weaponEffect& _effFName, WEAPON_ID _weapid = WEAPON7) = 0;

	//! river @ 2010-3-5:设置effectPos.
	virtual void            set_sgEffectPos( float _pos,int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;

	/** \brief
	 *  得到武器附加特效相关的信息.
	 *
	 *  \param I_weaponEffect& _effFName  外部传入，引擎内部填充相应的数据,第一个武器特效数据
	 */
	virtual void            get_sgEffect( float& _pos,
		                            I_weaponEffect& _effFName, WEAPON_ID _weapid = WEAPON7) = 0;

	//! 得到附加的addPos.
	virtual void            get_sgEffectPos( float& _pos,int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;


	/** \brief
	 *  绕武器的中心轴旋转osa特效,使用osa文件与武器的对位更加精确.
	 *
	 *  如果当前武器对应的特效类型不是osa,则此函数没有效果.
	 *  目前主要用于调整osa文件,使之形状与武器的形状相匹配.
	 */
	virtual void            rotate_osaEffect( float _agl , WEAPON_ID _weapid = WEAPON7) = 0;


	/** \brief
	 *  缩放武器上的特效效果
	 *
	 *  \param int _idx 每一个武器特效可以有三个小特效，每一个有相应的缩放。
	 *  \param _scale 如果是1.0，则保持原特效大小不变，2.0则增大为原来的二倍.0.2则缩小为原来的5分之1
	 *                如果传入的缩放比例小于零，则内部的比例不变，返回内部当前特效的缩入比例。
	 *  \return float 返回了当前特效的缩放比例。
	 */
	virtual float           scale_weaponEffect( int _idx,float _scale,WEAPON_ID _weapin = WEAPON7 ) = 0;

	//@{
	// 人物的动作和装备编辑相关。
	

	/** \brief 
	 *  显示人物的下一个动作帧姿势。
	 * 
	 *  人物的动作必须处于suspend状态，这个函数才起作用。
	 *
	 *  \param bool _next 如果为true,则使用下帧的人物动作，如果为false,则使用上一帧的姿势。
	 */
	virtual void            next_actFramePose( bool _next ) = 0;

	/** \brief
	 *  得到某一个动作总共有多少帧。
	 *
	 *  \param _actName 默认情况下使用动作名字来得到帧的总Frame数目。
	 *  \param _actIdx  如果此值大于等零，则返回相应索引动作的总Frame数目
	 */
	virtual int             get_actFrameNum( const char* _actName,int _actIdx = -1 ) = 0;


	//! 得到当前人物正在播放动作的帧索引。
	virtual int             get_curActFrameIdx( void ) = 0;

	

	//! 的到这个skinMesh的骨骼系统
	virtual os_boneSys* 			get_boneSys(void) = 0;

	/** \brief
	 *  得到身体部位的面数，如果当前身体部位不存在，返回0
	 *
	 *  \param int _idx 身体部位索引，从零开始。
	 */
	virtual int             get_bodyTriNum( int _idx ) = 0;
	
	//@}

	
	


# endif 


# if __CHARACTER_EFFEDITOR__
	
	/** \brief
	 *  更新身体某一部位的纹理贴图效果。
	 *
	 *  如果身体部位原来没有第二层纹理，则更新为使用第二层纹理。
	 * 
	 *  \param _seceff 如果传入的值为空，如果原来此身体部位使用特效，则更新为不使用特效。
	 *  \param _idx    要更换的部位的id.身体部位的索引从0开始，目前分为7个部位。
	 */
	virtual void           update_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx ) = 0;

	/** \brief
	 *  得到当前身体部位的chaSecEffect数据。
	 *
	 *  \param _idx    要得到的身体部位id,部位id索引从0开始。
	 *  
	 */
	virtual bool           get_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx ) = 0;


	/** \brief
	 *  存储身体第_idx部位的第二层纹理贴图效果。
	 *
	 *   如果身体部位skin没有特效，而目录下又存在此部位skin的特效文件，则删除此特效文件。
	 *   \param _idx   要更换的部位的id.身体部位的索引从0开始，目前分为7个部位。
	 *   \return bool  如果打开文件错误，或是要存储部位没有第二层贴图特效，返回false。
	 */
	virtual bool           save_chaSecEffect( int _idx ) = 0;

# endif 



	/** \brief
	 *  对人物进行缩放。
	 *
	 *  \param float _scale 人物在三个方向上要缩放的值，场景中默认数据是1.0f
	 */
	virtual bool           scale_skinMesh( osVec3D& _svec ) = 0;

	//@{
	// Windy mod @ 2005-8-30 19:04:10
	//Desc: ride 得到第二部分，即定位模型盒的位置
	
	/** \brief
	* 设置坐骑，绑定后该人物将随坐骑一起移动，
	* 上层要以坐骑为主角人物控制。并设置相应的动画
	*/
	virtual void			SetRide(I_skinMeshObj * _rideobj) = 0;
	//!取消坐骑，但不释放任何资源
	virtual void			DelRide() = 0 ;

	//! 为了login场景更流畅加入的接口。
	virtual bool            is_rideObj( void ) = 0;
	virtual I_skinMeshObj*  get_humanObjPtr( void  ) = 0;

	//! 得到坐骑绑定点的世界位置坐标.如果坐骑还没有绑定人物，则函数返回false.
	virtual bool            GetRidePos( osVec3D& _vec3 ) = 0;

	
# if __RIDE_EDITOR__
	//!对坐骑模型编辑的接口
	//!得到最后一部分的位置
	virtual	void			GetLastPartPos( osVec3D & _v3d )	= 0;
	//!设置最后一部分的位置
	virtual void			SetLastPartPos( osVec3D & _v3d)		= 0;
	//!保存最后一部分的位置到文件中
	virtual void			SaveLastPart()						= 0;
# endif

	//@}

	//! 切换旗帜的贴图
	virtual bool            change_mlFlagTexture( const char* _tex1,
		                        const char* _tex2,const char* _tex3 ) = 0;

	//  
	// River @ 2009-6-23: 设置人物渲染的描边效果
	virtual void            set_fakeGlow( bool _glow,DWORD _color ) = 0;

};


//===================================================================================





//===================================================================================
/** \brief 
 *  引擎中的资源管理类,管理引擎运行中所有的资源.
 *
 *  资源可分为设备相关资源和设备无关资源,有了这个管理器,
 *  上层就可以透明的使用应该创建的资源了,而不需要考虑是
 *  设备相关资源还是设备无关资源.
 *
 */
//===================================================================================
struct OSENGINE_API I_resource
{
	/**  \brief
	 *   创建一个Camera.
	 *
	 *   得到I_camera指针后,我们可以操作这个camera.
	 *   不需要释放这个camera,如果再需要camera,可以把以前创建的
	 *   camera重新初始化后使用.
	 */
	virtual I_camera*    create_camera( void ) = 0;

	//! 删除一个相机
	virtual void         delete_camera( I_camera* _cam ) = 0;



	// Todo: 加入其它的接口.


};



/** \brief
 *  场景FrameMove用到的结构。
 *  
 */
struct OSENGINE_API os_sceneFM
{
	//! 场景用到的camera.
	I_camera*     m_ptrCam;

	//@{ 
	//! 场景中的参照中心点，目前写入人物所的碰撞格子索引
	WORD         m_iCx,m_iCy;
	//@} 

	/** \brief
	 *  主角人物当前的位置，
	 *  
	 *  跟相机一起，计算出当前场景中挡住主角人物的物品,色后半透明这些物品。
	 */
	osVec3D      m_vecChrPos;

};




/** \brief
 *  场景格子用到的索引结构。
 *  
 *  为在整个场景中索引，可能超过64.
 *  
 *  仅做为一个数据结构使用,可能不只用于tileIdx一种用途.
 */
struct OSENGINE_API os_tileIdx
{
	WORD    m_iX;
	WORD    m_iY;

	os_tileIdx(void){};
	os_tileIdx(WORD _x,WORD _y):m_iX(_x),m_iY(_y){}
};

/** \brief
 *  得到用户点击屏幕坐标得到的地图中的坐标.
 *
 *  先给出地图的索引,然后给出地图上格子的索引(因为碰撞相关,地图格子为128*128)
 */
struct OSENGINE_API os_pickTileIdx
{
	//! TileGround的索引.
	os_tileIdx    m_tgIdx;

	//! TileGround内碰撞格子的索引,127为最大值.
	os_tileIdx    m_colTileIdx;


	//! 返回当前格子的高度值。
	float        m_fTileHeight;

};


// 这个结构在每一个tg内是128*128个,占用空间很大,所以使用1个字节对齐
# pragma pack(push,r1,1) 
/** \brief
 *  格子的碰撞信息。
 *
 *  使用这个格子来组织整个场景的碰撞相关信息。
 *  
 */
struct OSENGINE_API os_TileCollisionInfo
{

	//! 代表格子能否通过， 0为通过，1为不通过
	bool     m_bHinder;

	//! 代表格子的高度值,这个高度值在存储的时候是物体高度加地图地表高度值.
	//float    m_fHeight;
};
# pragma pack(pop,r1)



/** \brief
 *  可以往场景中加入的动态光的类型。
 */
enum ose_dLightType
{
	//! 目前仅支持point类型的动态光。
	OSE_DLIGHT_POINT,
};


/** \brief
 *  创建一个动态光需要填充的结构。
 */
struct OSENGINE_API os_dlightStruct
{

	//! 动态光的颜色。
	osColor   m_lightColor;

	//! 动态光的位置。
	osVec3D   m_vec3Pos;

	//! 动态光的影响范围。
	float     m_fRadius;

	//! 光照的类型。
	ose_dLightType  m_ltype;

};



/** \brief
 *  动画mesh的播放类型。
 *
 *  关于动画的播放，按时间播放，按循环帧数播放，永远播放。
 */
enum ose_amPlayType
{
	//! 按时间播放动画.
	OSE_PLAYWITHTIME = 0,
	//! 按帧数播放动画.每一循环动画为一帧.
	OSE_PLAYWITHFRAME ,
	//! 动画永远播放下去.
	OSE_PLAYFOREVER,
};

//! 渲染引擎的中间管道,上层不需要处理此值.
class     osc_middlePipe;

/** \brief
 *  创建一个aniMeshInstance需要填充的数据结构。
 */
class OSENGINE_API os_aniMeshCreate
{
public:
	//! 要创建的mesh对应的文件名。
	/*
	s_string        m_szAMName;
	*/
	//@{ 减少new与delete的调用。
	char           m_szAMName[64];
	//@} 

	//! 当前aniMesh在三维空间中的位置。
	osVec3D         m_vec3Pos;

	//! 当前aniMesh的播放方式。
	ose_amPlayType   m_ePlayType;

	//! 如果是按时间播放，播放的时间。
	float           m_fPlayTime;

	//! 如果是按次播放，播放的次数。
	int             m_iPlayFrames;

	//! 全局光数据。
	DWORD           m_dwAmbientL;

	//! 是否受全局光的影响,场景内的osa文件受环境光影响,特效的osa文件不受.
	bool            m_bEffectByAmbiL;

	//! 按次数或是按时间播放完成后是否自动删除.
	bool            m_bAutoDelete;


	/** \brief
	 *  是否创建实例.
	 *  
	 *  如果不创建实例,则此AniMesh为引擎中最常用的
	 *  的AniMesh,一直存在于内存中,为下一次调入此mesh节省时间,
	 *  直到游戏结束此AniMesh才释放.
	 */
	bool            m_bCreateIns;

	//! 创建时设置当前osaMesh的缩放值
	float           m_fSizeScale;
	//! 当前aniMesh的旋转方向。
	float           m_fAngle;

	//! River mod @ 2008-12_10: 内部使用的变量，水类的物品.
	bool            m_bWaterAni;

	//! 内部使用的变量，雾类的物品.
	bool            m_bFogAni;

public:
	//! 设置一些值的初始化。
	os_aniMeshCreate()
	{
		m_bAutoDelete = true;
		m_dwAmbientL = 0xffffffff;
		m_bCreateIns = true;
		m_szAMName[0] = NULL;
		m_bEffectByAmbiL = false;
		m_fSizeScale = 1.0f;
		m_fAngle = 0.0f;
		m_bWaterAni = false;
		m_bFogAni   = false;
	}
};


/** \brief
 *  创建一个粒子系统需要填充的结构.
 */
class OSENGINE_API os_particleCreate
{
public:
	//! 粒子系统对应的文件名.
	/*
	s_string        m_szParticleName;
	*/
	//@{ //! 减少new与delete的调用。
	char           m_szParticleName[64];
	//@} 

	//! 粒子系统在世界空间中的位置.
	osVec3D         m_vec3Offset;

	//! 粒子系统是否是在生命周期完成时自动删除.
	bool            m_bAutoDelete;

	//! 是否创建粒子场景的实例,如果不创建,则仅仅用于下一次
	//! 创建这个粒子特效时,可以更快速的调入.默认为创建实例.
	bool            m_bCreateIns;

	//! 粒子创建后的缩放,必须大于等于零,默认大小为1.0,即大小不变
	float           m_fSizeScale;
public:
	os_particleCreate()
	{
		m_bAutoDelete = false;
		m_bCreateIns = true;
		m_szParticleName[0] = NULL;
		m_fSizeScale = 1.0f;
	}

	//! 加入其它的参数??????

};




//! 创建一个水面文件的数据。
struct OSENGINE_API os_waterCreate
{
	//! 水面文件对应的纹理名。
	char       m_szWTexName[128];

	//! 水面文件对应的纹理帧数。
	int        m_iFrameNum;

	//! 水面的移动速度。
	float      m_fMoveSpeed;

	//! 水面对应在x和z方向上的大小　。
	int        m_iXSize;
	int        m_iZSize;

	//! 水面对应的方向。1:x 2:-y 3: -x 4:y
	int        m_iWaterDir;

	//! 水面的alpha值.必须介于0.0f-1.0f之间。
	float      m_fWAlpah;

};
struct OSENGINE_API os_snow
{
	float	mSnowHeight;//!< 雪花开始的高度,一般为40.0f,建议不改动
	float	mSnowLower;//!< 雪花结束的高度,一般为-24.0f,建议不改动
	float	mSnowSize;//!< 雪花大小 0.15f
	int		mSnowNr;//!< 雪花个数  0 - 1000 只见的整数
	osVec3D mVelocity;//!< 雪花下落速度 (0.0f, -20.0f, 0.0f);
	osVec3D mVelocityVariation;//!< 雪花下落的扰动速度 (5.0f, -10.0f, -4.0f);
};



//! 场景中最多的雨滴数目。
# define  MAX_RAINNUM 500

/** \brief
 *  场景内雨效果的描述结构。
 */
class OSENGINE_API os_rainStruct
{
public:
	//! 雨效果的颜色
	DWORD      m_dwTopColor;
	DWORD      m_dwBottomColor;

	//! 雨的下落速度
	float      m_fRainMoveSpeed;

	//! 场景内雨点的整体数目,最少为50,最多为500
	int        m_iRainDotNum;

	//! 调整雨点横向的缩放
	float      m_fXScale;
	//! 雨点竖向的大小缩放。
	float      m_fYScale;

public:
	os_rainStruct()
	{
		m_dwTopColor = 0x22afbfbf;
		m_dwBottomColor = 0x55afbfbf;

		// 默认的值，外部可以根据这个值来调整雨点的下落速度
		m_fRainMoveSpeed = 12.0;

		// 最多可以使用500个雨点
		m_iRainDotNum = MAX_RAINNUM/2;

		m_fXScale = 1.0f;
		m_fYScale = 1.0f;
	}
};

/** \brief
 *  场景中中的天气数据结构。
 */
struct OSENGINE_API os_weather
{
	//! 是否下雨
	BOOL        m_bRain;

	//@{
	//! 雾相关的数据。
	osColor     m_sFogColor;
	float       m_fFogStart;
	float       m_fFogEnd;
	//@}

	//@{ 
	//! 场景中光相关的参数。
	osColor     m_sAmbientColor;
	osColor     m_sDirLightColor;
	osVec3D     m_vec3DirLight;
	//@}

	// 人物的动态光的颜色
	osColor m_charDLightColor;

	//@{
	//! 天空纹理相关.天空纹理必须放到miskEffTex目录下
	//! 填充这个数据结构时，只能填充纹理的名字：比如,"miskEffTex/sky.hon"纹理，
	//! 在此处填充为"sky"就可以了。
	//  River @ 2006-6-20: 此变量没有意义，放在此处仅为兼容性考虑
	char        m_szSkyTex[128];

	float       m_fSkyMoveSpeed;

	

	//! 背景天空的osa文件.必须使用osa文件的全路径,比如: "keyAni\\sky1.osa"
	char        m_szOsaBkSky[128];




	//! 远山和天空远景显示的颜色。
	osColor     m_sFarTerrColor;
	//@{
	// Windy mod @ 2005-9-22 11:05:33
	//Desc: 
	bool		m_bIsLensFlare;
	char		m_szLensPath[128];
	osVec3D		m_vec3LensPos;
	//@}

	//! 第二层和第三层天空的osa文件。
	char        m_szOsaBkSkyEffect[2][128];
	char		m_szOsaBolt[128];
	float		m_iBoltRate;	//!< 闪电播放概率(0~1)
	float		m_BoltAngle;	//!< 闪电角度，弧度值（0~2*PI）
	osColor		m_BoltColor;	//!< 闪电颜色
	int			m_BoltType;		//!< 闪电类型,0表示只影响方向光,1表示只影响环境光
	float		m_BoltHeight;	//!< 闪电高度

	//! 第二层和第三层天空旋转速度
	float		m_fSky1MoveSpeed;
	float		m_fSky2MoveSpeed;

	float		m_TerrFogDistance;
	float		m_SkyFogDistance;
	float		m_BoltTime;			//!< 闪电时间
	//{@ windy add 7.10 用于对闪电进行特殊处理
	int			m_BoltFrequency;	//!< 先不用,如果参数不够再加
	//@}
	//{@ windy add 7.11 加入对雪花的控制 
	BOOL		m_bSnow;
	os_snow		m_SnowSetting;
	//@}
	float		m_fBoltFanAngle;

	// 下雨相关的数据结构
	os_rainStruct m_sRainStruct;

	//! 远山OSA文件，可以为空
	char		m_szTerrain[128];

};





# define  MAX_MAPNAMELENGTH   8
/** \brief
 *  调入场景时需要填充的数据结构。
 *
 *  此处的调入场景包括游戏开始时初始化场景，在游戏运行的过程中传送等操作
 *  也需要填充这个数据结构，并调入场景。
 */
struct OSENGINE_API os_sceneLoadStruct
{
	//! 地图的名字，比如vill,city等。
	char       m_szMapName[MAX_MAPNAMELENGTH];
	
	//@{
	/** \brief
	 *  初始化相机在世界空间中的绝对坐标。
	 *
	 *  我们根据这个绝对坐标找出我们应该静态调入的4张地图。
	 */
	int        m_iX;
	int        m_iY;
	//@}

	//!map 的ID号，这个和m_szMapName可以相互转换
	DWORD	   m_dwMapID;

	//! 初始化场景时天空相关的数据。
	char       m_szSkyName[128];

};

//! 可以最多创建两个eleBolt而对应一个Id.
# define MAX_ELEPERBOLT   2

//! 初始化一个bolt需要的数据结构
class os_boltInit
{
public:
	//! bolt要细分的数目，必须是2的n次方，8，16，32等
	int       m_iNumPt;

	//! bolt的开始和结束位置。
	osVec3D    m_vec3Start;
	osVec3D    m_vec3End;

	//! bolt的运动速度。
	float      m_fWildness;

	//! 每秒多少次动画bolt.
	int        m_iBoltFreq;

	//! Bolt的宽度。
	float      m_fWidth;

	//! 电弧的颜色
	DWORD      m_dwColor;

public:
	os_boltInit()
	{
		m_iBoltFreq = 20;
	}
};
//@{
// Windy mod @ 2005-9-22 15:14:21
//Desc: 以下是镜头特效用到的结构体
typedef class _FlarePart {
public:
	float	m_fPos;		//相对位置0-1之间表示太阳和屏幕中点的位置
	int		m_nIndex;	//使用的纹理ID
	float	m_fSize;	//该部分大小
	D3DXCOLOR m_color;	//颜色
	float	m_fAlpha;	//图片ALPHA ，值在0-1之间
	_FlarePart():m_fPos(0.0f),m_nIndex(0),m_fSize(0),m_color(0.0f,0.0f,0.0f,0.0f),m_fAlpha(0.0f){}
	~_FlarePart(){}

}FlarePart,*PFlarePart;
typedef enum _LFMask {
	LF_POS = 0x1,
	LF_INDEX = 0x2,
	LF_SIZE = 0x4,
	LF_RED = 0x8,
	LF_COLOR = 0x10,
	LF_ALPHA = 0x20,
	LF_ALL =0x3F 
}LFMask;

//@}


//! 默认每一帧纹理动画的时间
# define DEFAULT_ANITIME 0.033f

/** \brief
 *  在场景内点击地表时，为产生多种动画效果的初始化结构
 *
 */
class OSENGINE_API os_sceneCursorInit
{
public:
	//! 光标所在的位置
	osVec3D     m_vec3CursorPos;

	/** \brief
	 *  如果使用平面的动画光标，光标图片的名字,如"passCursor",
	 *  如果不使用平面动画光标，则m_szCursorName[0] = NULL;
	 */
	char        m_szCursorName[64];
	//! 光标的颜色
	DWORD       m_dwCursorColor;

	//! 光标动画的帧数
	WORD        m_wXFrameNum;
	WORD        m_wYFrameNum;
	//! 每隔多长时间切换一次光标动画帧
	float       m_fFrameTime;

	//! 光标如果使用osa动画的话，osa文件的名字,使用完整的路径，如"keyani\\cursor.osa"
	char        m_szOsaCursorName[64];
	//! 播放osa动画的次数
	int         m_iPlayOsaNum;

public:
	os_sceneCursorInit()
	{
		m_szCursorName[0] = NULL;
		m_szOsaCursorName[0] = NULL;

		m_dwCursorColor = 0xffffffff;
		m_wXFrameNum = 1;
		m_wYFrameNum = 1;
		m_iPlayOsaNum = 1;
		// 默认每秒切换30次
		m_fFrameTime = DEFAULT_ANITIME;
	}
};


//! 创建一个Decal用到的结构
class os_decalInit
{
public:
	//! Decal的位置和大小,m_fSize为decal的半径
	osVec3D     m_vec3Pos;
	float       m_fSize;

	//! 用到的shader名字
	char        m_szDecalTex[32];

	//! 旋转角度，主要用于产生一些随机的效果和脚印等有方向的效果
	float       m_fRotAgl;

	//! decal消失的时间,如果为零，则永远不消失，除非上层删除
	float       m_fVanishTime;

	//! decal使用的顶点色
	DWORD       m_dwDecalColor;

	//! Decal使用的纹理是否有纹理动画
	WORD        m_wXFrameNum;
	WORD        m_wYFrameNum;
	//! 如果有动画，切换一个动画帧的时间
	float       m_fFrameTime;

	//! 非跟随地形的decal,只需要计算位置，不需要跟随地形进行切分，用于回合游戏战斗场景内的地基.
	BOOL        m_bFakeDecal;

	//! 跟地表之间的高度位移，默认为零
	float       m_fYOffset;

	//! decal跟背景的结合方式。默认是0: srcAlpha InvSrcAlph. 1: srcAlpha  one.
	int         m_iAlphaType;

public:
	os_decalInit()
	{
		m_fSize = 0.0f;
		m_szDecalTex[0] = NULL;
		m_fRotAgl = 0.0f;
		m_fVanishTime = 0.0f;
		m_dwDecalColor = 0xffffffff;
		// 动画相关
		m_wXFrameNum = 1;
		m_wYFrameNum = 1;
		m_fFrameTime = DEFAULT_ANITIME;
		m_bFakeDecal = FALSE;

		m_iAlphaType = 0;

		// 默认状态下，系统会加入一个小量的位移
		m_fYOffset = 0.0f;
	}
};


//  
//! 场景内地表纹理的属性
#pragma pack(push,1)
class os_terrainTexSnd 
{
public:

	// 材质ID
	int  m_iMaterialID;
	// 纹理
	char m_szTexture[32];
	// 音效
	char m_szSound[32];
	// 是否有特效
	char m_szEffect[32];
	union 
	{
		float mWaterHeight;
		// 是否有脚印
		BOOL m_bFootPrint;
	};
	

	os_terrainTexSnd()
	{
		m_iMaterialID = 0;
		m_szTexture[0] = NULL;
		m_szSound[0]  =  NULL;
		m_szEffect[0] =  NULL;
		m_bFootPrint  =  FALSE;
	}
	 
};
#pragma pack(pop)



//! 每一个碰撞格子要细分的格子数目,服务器传回的格子使用这个单位的格子索引
# define DETAIL_TILENUM    15


//! 是否加入修改地图碰撞信息和高度信息接口的宏.
# define __COLHEI_MODIFY__   1

//! 是否可以使用对整个场景中人物和npc的体形统一调大的接口。
# define __ALLCHR_SIZEADJ__  0


//! 立即处理渲染消息的回调函数原型
typedef int ( *UPDATE_LOADINGSTEPCALLBACKIMM)(LONG,LONG);




//===================================================================================
/** \brief
 *  用于组织边缘游戏场景数据的接口。
 *
 *  这个接口主要用于完成服务器显示方面的指令，逻辑方面的程序由游戏程序而不是由引擎
 *  来完成。
 *  Camera需要从服务器进行初始化。
 *
 *  在游戏程序中不需要实际操作游戏场景中的数据，只需要动服务器指令进行分析，并分配
 *  给下层的执行程序就可以了。
 *  
 *  I_fdScene内用到了两种坐标单位：
 *  第一：以米为单位。接口内的参数，如果传入或传出的是float类型的值，都是以米为单位。
 *  第二：以碰撞坐标为单位。接口中传入或是传出的类型是整型或是无符号型，则都是以场景内的
 *        碰撞坐标为单位。
 *  
 */
//===================================================================================
struct OSENGINE_API I_fdScene
{

	//! 回调函数注册
	virtual void register_updateLoadingStepCallbackFunc( UPDATE_LOADINGSTEPCALLBACKIMM _pFunc ) = 0;

	// 
	// HDR Light    River: 要删除以下接口？？？？
	virtual void init_hdrLight() = 0;
	virtual void set_hdrRenderTarget() = 0;
	virtual void set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure ) = 0;
	virtual void set_hdrFm( float _fm ) = 0;
	virtual void set_hdrDiffuse( float _diffuse ) = 0;
	virtual void set_hdrExposure( float _exposure ) = 0;


	/**********************************************************************/
	//
	//  场景的初始化.
	//
	/**********************************************************************/	
	/** \brief
	 *  使用我们的场景调入结构调入场景地图。
	 *
	 */
	virtual bool          load_scene( os_sceneLoadStruct* _sl ) = 0;

# if __QTANG_VERSION__
	//! 由上层设置地图的碰撞信息,其实这些工作完全可以由客户端完成，为兼容以前的接口。
	virtual bool          set_sceneCollInfo( char* _coll ) = 0;
# endif 

	virtual bool          isLoadMap() = 0;

	//! 此处返回的size是以米为单位的size。每一个显示格子占三米，一个
	//! 碰撞格子是1.5米
	virtual osVec2D       get_mapSize() = 0;

	/** \brief
	 *  释放一个场景.
	 *
	 *  释放这个场景中所有的数据,比如地图传送等操作需要整个场景都变化需要
	 *  使用这个函数。
	 *  
	 *  \param 是否释放我们屏幕图片，如果处于界面切换状态时，如果下一个屏幕用
	 *         到当前的屏幕图片的话，不释放屏幕图片，可以节省下一个屏幕的调入时间，
	 *  
	 */
	virtual bool          release_scene( bool _rlseScrPic = true ) = 0;



	/** \brief
	 *  为解决地图中的瞬移问题加入的接口。
	 *
	 *  \param _mapName 瞬移后地图的名字，可以使用和当前地图同样的名字。
	 *  \param _xCord,_yCord 瞬移后人物的坐标,是场景内的碰撞格子坐标。一个显示格子有2*2个
	 *                       碰撞坐标
	 */
	virtual bool          spacemove_process( const char* _mapName,int _xCord,int _yCord ) = 0;

	/** \brief
	 *  场景的FrameMove.
	 */
	virtual bool         frame_move( os_sceneFM* _fm ) = 0;

	/** \brief
	 *  渲染当前的地图。
	 *
	 *  \param _scrPic 如果当前变量为真，则只渲染屏幕内的二维图片，否则渲染场景
	 *                 加二维图片。
	 *  \param _LensPic 表示，渲染的二维图片用LensFlare的渲染状态。
	 *  \param _rtype   渲染场景的模式，为回合制游戏而加入
	 *                  此参数为OS_COMMONRENDER，普通的渲染
	 *                  参数为OS_RENDER_LAYER,分层渲染，先渲染背景，再渲染参战的人物和特效
	 *                  
	 */
	virtual bool          render_scene( bool _scrPic = false,
		                    bool _LensPic = false,os_FightSceneRType _rtype = OS_COMMONRENDER ) = 0; 

	/** \brief
	 *  得到当前帧渲染的三角形数目。
	 */
	virtual int           get_renderTriNum( void ) = 0;

	/** \brief
	 *  得到当前场景中的skinMesh用到的三角形的数目。
	 *
	 *  skinMesh主要用于人物的渲染，所以此处得到的值，为人物的面片数目。
	 */
	virtual int           get_skinMeshTriNum( void ) = 0;

	/** \brief
	 *  改变场景的天气。
	 *
	 *  \param _weather  新的场景天气数据结构。
	 *  \param _itime    改变天气需要的天气熔合时间,以秒为单位.如果此值
     *                   小于等于零，则直接修改天气。
	 *  
	 */
	virtual void          change_sceneWeather( os_weather& _weather,float _itime ) = 0;

	/** \brief
	 *  改变场景的远景显示osa模型.
	 *  
	 *  \param _terr 远山的模型文件名.
	 */
	virtual void          change_sceneFarTerr( const char* _terr ) = 0;

	/** \brief
	 *  得到当前鼠标点击所在格子的索引。
	 *
	 *  这个索引值是在整张大地图上的格子的索引值。必须由服务器确认才有效。
	 *  
	 *  ATTENTION : 
	 *
	 *  \param _dix   引擎内部处理完成后输出到客户程序的值.
	 *  \param _sidx  输入到引擎内部用于处理的屏幕坐标.
	 *  \param _osVec3D* _intersectPos 如果相交，输出的是交点的实际三维坐标
	 */
	virtual bool          get_pickTileIdx( os_pickTileIdx& _dix,
		                    os_tileIdx& _sidx,osVec3D* _intersectPos = NULL ) = 0;


	/** \brief
	 *  根据一个地图坐标，得到当前地图坐标中心点内对应的三维，如果当前坐标不在场景内，
	 *  则对应的三维坐标的y值为0.0f.
	 */
	virtual bool          get_tilePos( int _x,int _y,osVec3D& _pos,BOOL _precision = FALSE  ) = 0;
	//! 显示传入坐标所在位置的地表碰撞信息
	virtual void	get_collision_info(const osVec3D& _pos,float _radius = 2) = 0;

	/** \brief
	 *  给出场景内的一个xz值，得到此处的地表高度
	 *
	 *  \param osVec3D& _pos x,z的值由_pos内的x,z值给出.返回的高度值在_pos.y内
	 *  \param BOOL _precision 是否精确碰撞,即加入物品高度信息,只有主角人物移动使此变量为true
	 */
	virtual bool         get_detailPos( osVec3D& _pos,BOOL _precision = FALSE ) = 0;
     //! 得到场景内某个位置上跟地表物品相关的高度信息,如果没有物品,则高度信息为很小的负值.
	virtual float       get_objHeightInfo( osVec3D& _pos ) = 0;

	/** \brief 
	 *	根据一个世界坐标，返回一个DWORD的值，表明这个世界坐标的光照图的颜色。
	 *
	 *	\param osVec3D& _pos		x,z 有作用,为世界坐标
	 */
	virtual DWORD		get_lightMapColor(const osVec3D& _pos) = 0;

	//! 传入世界坐标的位置,得到该位置的材质索引
	/*! \note 传入的坐标只用到了X,Z分量*/
	virtual int			get_materialIdx(osVec2D& _pos,os_terrainTexSnd& _texsnd ) = 0;
	/** \brief
	 *  客户端的碰撞检测函数.
	 *  
	 *  如果客户端碰撞检测不能通过，则不需要再由服务器进行确认，
	 *  如果能通过，则由服务器确认后进行一系列的动作和操作。 
	 *  
	 */
	virtual bool          collision_detection( const os_tileIdx& _tile ) = 0;

	/** \brief
	 *  得到场景中某一个地图的碰撞信息数据块。
	 * 
	 *  \param _tgIdx     场景中地图的索引，从(0,0)开始。
	 *  \return os_TileCollisionInfo** 返回的是os_TileCollisionInfo的二维数组。
	 *                                 就是os_TileCollisionInfo［128][128] 
	 */
	virtual const os_TileCollisionInfo* get_sceneColData( os_tileIdx& _tgIdx )=0;

	/** \brief
	 *  由传入的世界空间点计算当前世界空间点在哪一个地图上，在地图的哪一个图块上.
	 *  
	 *  \param  _res     使用os_pickTileIdx结构返回我们得到的值.
	 *  \param  _pos   　传入世界空间的顶点位置。
	 *
	 *  \return bool     如果当前传入的值超出当前整张大地图的宽度，返回false.
	 */
	virtual bool                   get_sceneTGIdx( 
		                             os_pickTileIdx& _res,const osVec3D& _pos ) = 0;

	/** \biref
	 *  得到当前视域内的地图的数目和索引.
	 * 
	 *  客户端在寻路算法时会用到这些数据.
	 *  
	 *  \return int     返回在视域内的地图的数目.
	 *  \param  _tgidx  传入的返回地图索引的指针结构,最多返回4个._tgidx应该是os_tileIdx[4].
	 *  
	 */
	virtual int                   get_inViewTGIdx( os_tileIdx* _tgidx ) = 0;

	//! 得到当前场景中可以看到的格子的数目。
	virtual int                   get_inViewTileNum( void ) = 0;


	/** \brief
	 *  用于创建和操作上层需要的SkinMeshObject.
	 *
	 *  \param _sceneSm 是否创建场景中的sm物品，引擎外不需要使用这个参数。
	 *  \param _smi 用于初始化新创建的skinMeshObject.
	 *  \return     如果创建失败，返回NULL.
	 *
	 *  释放一个I_skinMeshObj接口时,使用I_skinMeshObj::release_character();
	 */
	virtual I_skinMeshObj*  create_character( os_skinMeshInit* _smi,
		                        bool _sceneSm = false,bool _waitForFinish = false ) = 0; 

	//! 把一个character指针推进到墙后渲染队列中
	virtual void            push_behindWallRenderChar( I_skinMeshObj* _ptr ) = 0;

	//! 把一个character指针从墙后渲染队列中删除
	virtual void            delete_behindWallRenderChar( I_skinMeshObj* _ptr ) = 0;


	//@{
	//  程序中光标相关的接口。
	/** \brief
	 *  使用文件名创建光标。
	 */
	virtual int            create_cursor( const char* _cursorName ) = 0;
	//! 设置引擎中显示的光标。
	virtual void           set_cursor( int _id, int _centerX = 0, int _centerY = 0 ) = 0;
	//! 获得当前显示的光标id
	virtual int            get_cursor() = 0;
	//! 是否显示光标
	virtual void           hide_cursor( bool _hide = false ) = 0;

	//@} 



	//@{
	//
	// 场景中公告板相关的函数。
	// 
	/** \brief
	 *  往场景中加入billBoard.
	 *
	 *  场景中的Billboard在初始化的时候固定位置，以后不能改变。
	 *  
	 *  ATTENTION: 如果是加入了循环播放的公告板，则必须手工调用删除函数才能删除。
	 *             运行过程中主要是加入运行一次的公告板动画，播放完成后，会自己删除自己。
	 *
	 *  \return int 播放的公告板返回可以操作的id,因为公告板可能会自动删除，所以在操作
	 *              公告板之前，一要调用函数validate_bbId,如果是不可操作的id,则上层需要
	 *              对这个id进行处现，使下一次不再发生对此id的操作。
	 * 
	 *  \param  bool _createIns 是否创建公告板的实例,如果不创建的话,则用于内存catch公告板
	 *                          资源,使下一次创建公告板的时候可以速度更快.
	 *  \param  float _sizeScale 对要创建的公告板进行大小缩放。默认是创建的大小，即1.0
	 *  \param  bool _topLayer   是否顶层渲染，用于回合制的战斗场景
	 */
	virtual int           create_billBoard( const char *_bfile,osVec3D& _pos,
		                    bool _createIns = true,float _sizeScale = 1.0f,bool _topLayer = false ) = 0;


	//! 在场景内创建不受远剪切面影响的公告板,其它的操作跟公告板的操作一致
	virtual int           create_hugeBillBoard( const char* _bfile,osVec3D& _pos,
		                    bool _createIns = true,float _sizeScale = 1.0f ) = 0;

	/** \brief
	 *  控制billBoard的位置。一次性运行的公告板，不能控制位置。
	 *
	 *  \param _id 　要设置的公告板的id。
	 *  \param _pos  要设置的公告板的位置。
	 */
	virtual void          set_billBoardPos( int _id,osVec3D& _pos ) = 0;

	/** \brief
	 *  设置公告板的播放时间。
	 *  
	 *   \return 返回了要设置公告板的正常播放时间。
	 *   \param _timeScale 要设置的公告板的播放时间与原来播放时间比例，如果为负值，接口只返回公告板
	 *                的正常播放时间，不设置播放时间比例。比如_timeScale为2.0f,则播放速度为正常
	 *                速度的2倍。
	 */
	virtual float         set_bbPlayTime( int _id,float _timeScale ) = 0;

	//! 验证billBoardId的有效性。
	virtual bool          validate_bbId( int _id ) = 0;


	/** \brief
	 *  删除billBoard.
	 *
	 *  \param _id 删除指定id的billBoard.
	 *  \param bool _finalRelease 是否删除此公靠板对应的设备资源
	 */
	virtual void          delete_billBoard( int _id,bool _finalRelease = false ) = 0;

	//! 隐藏当前的billBoard.
	virtual void          hide_billBoard( int _id,bool _hide ) = 0;

	//! 缩放当前的公告板.river added @ 2010-1-7:
	virtual void          scale_billBoard( int _id,float _rate ) = 0;

	// 
	//@}

	//@{
	// 
	// 场景中动画相关的函数.此处的动画非骨骼动画.
	// 动画文件的格式为*.osa格式的动画.
	/** \brief
	 *  场景中加入动画文件,如果加入的动画文件不在当前相机的视野内,则不需要渲染新加入的
	 *  动画.
	 *
	 *  \param _mc 创建动画数据的结构,填充这个结构,创建我们的动画数据.
	 */
	virtual int           create_osaAmimation( os_aniMeshCreate* _mc,bool _topLayer = false ) = 0;
	
	//! 开始动画
	virtual void          start_animationMesh( int _id ) = 0;

	//! fade或是fadeIn当前的osa动画.
	virtual void          start_fadeOsa( int _id,
		                    float _fadeTime,bool _fadeOut = true,float _minFade = 0.0f ) = 0;

	//! 隐藏当前的osa动画
	virtual void          hide_osa( int _id,bool _hide ) = 0;


	/** \brief
	 *  需要验证每一个动画物品id的合法性
	 *
	 *  \param int& _id 如果传入的id不合法，则把_id赋值为-1
	 */
	virtual bool          validate_aniMeshId( int _id ) = 0;

	//! 设置某一个动画的位置。
	virtual void          set_aniMeshPos( int _id,osVec3D& _pos ) = 0;
	//! 设置一个动作的旋转方向.
	virtual void          set_aniMeshRot( int _id,float _angle ) = 0;
	//! 缩放当前的osa动画
	virtual void          scale_aniMesh( int _id,float _rate,float _time ) = 0;

	/** \brief
	 *  设置osa动画的播放时间。
	 *  
	 *   \return 返回了要设置osa动画的正常播放时间。
	 *   \param _timeScale 要设置的osa的播放时间与原来播放时间比例，如果为负值，接口只返回公告板
	 *                的正常播放时间，不设置播放时间。
	 */
	virtual float         set_osaPlayTime( int _id,float _timeScale ) = 0;


	/** \brief
	 *  上层播放整体个特效的接口
	 *  
	 *  \param const char* _ename 要创建特效的名字
	 *  \param osVec3D& _pos      要创建特效在场景内位置。
	 *  \param float _angle       要创建特效的旋转角度，绕Y轴
	 *  \param BOOL _instance     是否创建场景内特效实例，如果此值为false,则引擎内预调入特效，不显示。
	 *  \param float _scale       创建特效的缩放比较例。
	 *  River added @ 2009-6-22: 用于上层得到特效播放的时长。如果时间为零或是一直播放的特效，则
	 *                           此返回值为负值
	 *  \param float* _effectPlaytime 要创建的特效，正常的播放时间。 
	 */
	virtual DWORD         create_effect( const char* _ename,
		                      osVec3D& _pos,float _angle = 0.0f,
							  BOOL _instance = TRUE,float _scale = 1.0f,
							  float* _effectPlaytime = NULL ) = 0;
	/** \brief
	 *  在上层删除一个特效
	 *
	 *  \param bool _finalRelease 是否全部释放特效设备相关资源，通常只编辑器内使用。
	 */
	virtual void          delete_effect( DWORD _id,bool _finalRelease = false ) = 0;

	//! River @ 2010-1-14:上层可以到一个effect的keyTime.
	virtual float         get_effectKeyTime( DWORD _id ) = 0;


	//! 得到effect对应的boundingBox,其中boxsize是经过scale后的size.此box无旋转
	virtual bool          get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize ) = 0;

	/*  \brief 
	 *   得到一个有旋转方向的包围盒。
	 *   
	 *   上层得到包围盒后需要在下一次调用这个接口之前使用，内部的bbox是一个static变量。
	 *   必须判别返回值，对于在特效编辑器内没有设定包围盒的特效，返回值为空.
	 */
	virtual const os_bbox*   get_effectBox( DWORD _id ) = 0;


	//! 对特效进行缩放,用于对场景内的道具进行缩放动画，但此缩放仅限于osa动画和公告板，粒子场景不能动态缩放
	virtual void          scale_effect( int _id,float _rate,float _time ) = 0;
	//! 对特效进行fadeOut
	virtual void          fade_effect( int _id,float _fadetime,bool _fadeOut = true ) = 0;

	/*  \brief
	 *  River @ 2010-8-24:对特效的播放速度进行缩放。
	 *
	 *  \param float _speedRate  原来播放速度的倍数，如此值为0.1，则此特效为正常播放速度的0.1倍。
	 */

	virtual void          scale_effectPlaySpeed( int _id,float _speedRate ) = 0;


	/** \brief
	 *  设置特效的alpha值,只有osa特效才能正确的设置alpha值。alpha值介于0.0和1.0f之间。
	 *
	 *  \param int _id   特效对应的id.
	 *  \param float _alpha 特效要使用alpha值
	 */
	virtual void          set_effectAlpha( int _id,float _alpha ) = 0;


	//! 隐藏或显示当前id的effect
	virtual void		  hide_effect(int _id,bool _bHide = TRUE ) = 0;

	/** \brief
	 *  上层设置特效的位置和旋转方向
	 *
	 *  \param float _speed 上层在设置特效位置时，可以传入的参数，对于有三维声音的特效，
	 *                      此参数可以使声音更加生动。 
	 */
	virtual void          set_effectPos( DWORD _id,osVec3D& _pos,float _angle,
		                     float _speed = 1.0f,bool _forceUpdate = false ) = 0;
	//! 验证一个effect是否已经在内部被删除
    virtual BOOL          validate_effectId( DWORD _id ) = 0;


	/** \brief
	 *  加入一个可以显示鼠标点击位置的标志。
	 */
	virtual void	      draw_cursorInScene( os_sceneCursorInit& _scInit ) = 0;


	//! 删除一个动画.
	virtual void          delete_animationMesh( int _id,bool _sceC = false ) = 0;

	//@} 

	//@{
	//  客户端用到的特定数据，引擎只负责调入并由上层调用，引擎内不做处理。
	/** \brief 
	 *  得到特效播放的位置类型 
	 *
	 *  \param int _id  传入创建的特效实例的id.
	 *  \return DWORD   0:包围盒比例播放。 1:攻击位置播放。2:被攻击位置播放。
	 */
	virtual DWORD         get_effectPlayPosType( int _id ) = 0;
	//! 得到特效播放的包围盒比例，返回的值在0.0到1.0之间。
	virtual float         get_effectPlayBoxScale( int _id ) = 0;
	//@}



	//@{
	//  粒子相关的函数接口.创建,设置和删除.
	/** \brief 
	 *  创建一个particle实例.
	 *
	 *  \param bool _topLayer 是否顶层渲染，用于回合制游戏相关
	 */
	virtual int           create_particle( os_particleCreate& _pcreate,
		                         BOOL _inScene,bool _topLayer = false ) = 0;

	//! 设置particle的位置.
	virtual void          set_particlePos( int _idx,osVec3D& _pos ) = 0;

	//! 隐藏当前的particle.
	virtual int           hide_particle( int _id,bool _hide ) = 0;

	/** \brief
	 *  设置粒子动画的播放时间。
	 *  
	 *   \return 返回了要设置粒子动画的正常播放时间。
	 *   \param _timeScale 要设置的粒子动画的播放时间与正常播放时间的比例，如果为负值，接口只返回公告板
	 *                的正常播放时间，不设置播放时间比。比如_timeScale为2.0f,则播放速度为正常
	 *                速度的2倍。
	 */
	virtual float         set_parPlayTime( int _id,float _timeScale ) = 0;


	//! 检测particleId的可用性，如果不可用，则上层需要处理当前的particle Id.
	virtual bool          validate_parId( int _id ) = 0;

	/* \brief
	 * 删除一个particle.
     *
	 * \param int _id  粒子id.
	 * \param bool _finalRelease 是否释放此粒子对应的设备相关资源
	 */
	virtual void          delete_particle( int _idx,bool _finalRelease = false ) = 0;

	//@} 

	//@{
	//  电弧相关的特效。
	//! 创建一个电弧。
	virtual int           create_eleBolt( os_boltInit* _binit,int _bnum ) = 0;
	//! 重新设置弧。
	virtual void          reset_eleBolt( os_boltInit* _bset,int _idx ) = 0;
	//! 删除一个电弧资源。
	virtual void          delete_eleBolt( int _id ) = 0;
	//@}


	//{@ windy 6-21 add
	//! 开始播放闪电,
	/*!\param _times ,闪电时间长度
	\param frequency 一秒钟闪几次.
	\param flashTime 每次闪电,所占时间 [0-1]之间数
	\param _color 闪电颜色*/
	virtual void	start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f))=0;
	virtual void	stop_sceneLight() = 0;
	//@}
	//@{
	// 动态光相关的接口。
	/** \brief
	 *  创建一个动态光。
	 *
	 *  \return int  如果返回-1,则创建失败。
	 */
	virtual int           create_dynamicLight( os_dlightStruct& _dl ) = 0;

	/** \brief
	 *  得到一个动态光的各项参数。
	 */
	virtual void          get_dynamicLight( int _id,os_dlightStruct& _dl ) = 0;

	/** \brief
	 *  FrameMove一个动态光。
	 */
	virtual void          set_dynamicLight( int _id,os_dlightStruct& _dl ) = 0;

	//! 设置动态光的位置。
	virtual void          set_dLightPos( int _id,osVec3D& _pos ) = 0;

	/** \brief
	 *  删除一个动态光。
	 */
	virtual void          delete_dynamicLight( int _id,float _fadeTime = 0.0f ) = 0;


	/** \brief
	 *  改变场景的环境光系数。
	 *
	 *  \param _r,_g,_b  场景中环境光的系数，使用这三个值缩放场景中的环境光。
	 */
	virtual void          change_amibentCoeff( float _r,float _g,float _b ) = 0;
	//@}


	//@{
	/** \brief
	 *  简单阴影相关的接口，人物,npc和怪物下面有一个黑色的小圈。
	 *
	 *  \param float _scale 此阴影的大小缩放值。对于大的怪物，黑圈可以大一些。
	 */
	virtual int           create_fakeShadow( osVec3D& _pos,float _scale = 1.0f ) = 0;
	virtual void          set_shadowPos( int _id,osVec3D& _pos ) = 0;
	virtual void          delete_shadowId( int _id ) = 0;
	virtual void          hide_shadow( int _id,bool _hide = true ) = 0;
	//@} 

	//  TEST CODE:
	//! 创建和删除场景地表上的Decal
	virtual int           create_decal( os_decalInit& _dinit ) = 0;
	virtual void          delete_decal( int _id ) = 0;
	virtual void          reset_decalPos( int _id,osVec3D& _pos ) = 0;


	//@{
	//! 场景中开始和结束下雨效果。
	virtual void          start_sceneRain( os_rainStruct* _rs = NULL ) = 0;
	virtual void          stop_sceneRain( void ) = 0;
	
	/** \brief
	 *  场景内开始和结束下雪的效果
	 *
	 *  \param BOOL _start 如果真则开始下雪，如果为false,结束下雪的效果
	 *  \param int _grade  分为0,1,2三个级别。级别越高，则雪花越多。
	 */
	virtual void          scene_snowStorm( BOOL _start = TRUE,int _grade = 0 ) = 0;
    //@} 


	//@{
	//  加入场景中的二维屏幕图片显示需要的接口。
	//  二维图片的显示需要由客户程序自己进行排序，把需要放到下边的图片优先推进行到场景
	//  中进行显示。
	
	/** \brief
	 *  创建一个二维显示需要的纹理
	 *　
	 *  场景切换的时候，所有的纹理会由场景程序自动的释放。
	 */
	virtual int            create_scrTexture( const char* _texname ) = 0;

	//! 通过id得到一个纹理 ATTENTION : River:上层需要直接操作LPDIRECT3DTEXTURE9接口？？
	virtual LPDIRECT3DTEXTURE9 get_sceneScrTex( int _id ) = 0;
	//! test注册一个纹理,返回id
	virtual int               register_texid( LPDIRECT3DTEXTURE9 _pTexture ) = 0;
	virtual void    unregister_texid( int _texid ) = 0;
	virtual int		register_targetAsTex( int _targetIdx ) = 0;


	//! 创建一个纹理
	virtual int			create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture ) = 0;

	/** \brief
	 *  释放一张贴图。
	 */
	virtual void           release_scrTexture( int _texid ) = 0;

	//syq_mask 创建渲染目标
	virtual int                create_renderTarget( int _width, int _height, D3DFORMAT _format = D3DFMT_A8R8G8B8 ) = 0;
	/** \brief
	 *  syq_mask 获得渲染目标
	 *
	 *  如果传入的id是-1，则返回了渲染设备的Backbuffer.
	 */
	virtual LPDIRECT3DSURFACE9      get_renderTarget( int _id ) = 0;
	virtual LPDIRECT3DTEXTURE9      get_renderTexture( int _id ) = 0;


	/** \brief
	 *  把需要显示的数据结构推进到场景中进行显示。
	 *
	 *  这个数据被推进行到场景中的数据结构中，在显示完场景中的三维元素后，
	 *  开始显示这些二维的元素。
	 */
	virtual void           push_scrDisEle( const os_screenPicDisp* _dis ) = 0;
	//@}

	//@{
	/** \brief
	 *  Draw text on the screen.  
	 *
	 *  这个函数是引擎显示文字信息的主要的函数,必须支持Unicode.这个函数必须
	 *  用在beginScene和endScene函数之间.
	 */
	virtual int          disp_string( os_stringDisp* _dstr,int _dnum,int _fontType = 0,
		                 bool _useZ = false,bool _bShadow = false, RECT* _clipRect = NULL ) = 0;

	/** \brief
	*  立即在屏幕上显示字串，参数跟disp_string参数一致。
	*
	*  \param _cursor 需要立即显示的光标数据结构
	*/
	virtual int          disp_stringImm( os_stringDisp* _dstr,int _fontType = 0,os_screenPicDisp* _cursor = NULL ) = 0;


	/** \brief
	 *  渲染有颜色解晰功能的文字字串,返回值为当前的字串在给定的限制下，显示了多少行。
	 *
	 *  \return int 如果返回值小零，则表示参数内给出的区域不能完整的绘出给出的字串。
	 *  
	 */
	virtual int          disp_string( os_colorInterpretStr* _str,int _fontType = 0,
		                 bool _useZ = false,bool  _bShadow = false, RECT* _clipRect = NULL ) = 0;
	//@}
	//@{
	// Windy mod @ 2005-9-20 15:02:51
	//Desc: 关于Lens Flare的接口
	//!以下两个控制Lens Flare是否显示，并不涉及到资源
	//virtual void	start_LensFlare() = 0;
	//virtual void	stop_LensFlare() = 0;


	///从文件中加载lens flare
	virtual void	Load_LensFlare(const char * _fname) = 0;
	virtual void	clear_LensFlare() = 0;
	//!设置3D位置
	virtual void	set_RealPos(osVec3D & _vec3Pos) = 0;

	///以下是编辑器的接口
	
	// 
	// ATTENTION River:编辑器的接口，就加入宏，最终运行时，去掉这些接口和代码
	///修改nPartID指定的Flare的值，具体项由_Mask决定
# if __LENSFLARE_EDITOR__
	virtual void	set_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart) = 0;
	virtual void	get_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart) = 0;
	virtual int		get_LensFlareNum() = 0;
	virtual void	save_LensFlare(const char * _fname) = 0 ;

	virtual void	add_LensFlarePart() = 0;
	virtual void	del_LensFlarePart(int iPartID) = 0;

	//@}
#endif

# if __EFFECT_EDITOR__

	//! 得到当前场景的天气。
	virtual void          get_sceneWeather( os_weather& _weather ) = 0;

	//! 强制结束一个动画.
	virtual void          stop_animationMesh( int _id ) = 0;

	//! 设置某一个动画相关的参数.
	virtual bool          set_animationMesh( int _id,os_aniMeshCreate* _mc ) = 0;

# endif 


	//@{
	//  修改地图的高度信息和碰撞信息的接口。
# if __COLHEI_MODIFY__
	/** \brief
	 *  修改主角人物所在位置的高度信息，os_sceneFM内的m_iCx,m_iCy变量表示的位置。
	 *
	 *  \param float _mod  要修改的高度，可正可负。
	 */
	virtual bool          modify_heightInHeroPos( float _mod ) = 0;

	
	//! 把鼠标点击的位置设置为跟原来相反的可通过性。
	virtual bool          modify_colAtMousePick( os_pickTileIdx& _ps, BOOL bUseParm=FALSE,bool bPass=false) = 0;
	
# endif
	//@} 



	//! 调试用函数，画一个bounding Box.
	virtual void          draw_bbox( os_bbox* _box,DWORD _color,bool _solid = false ) = 0;
# ifdef _DEBUG

	//! 用于对下层的变量进行输出.
	virtual void          debug_test( void ) = 0;
# endif 

	/** \brief
	 *  调试用函数，画一条线.此函数仅用于调试，接口不全,效率不高。
	 *  ATTENTION :这个函数必须在设备的benginScene和endScene之间调用。
	 *
	 *  \param const osVec3D* _verList 要画线的顶点列表，画的是lineStrip.即传入a,b,c三个顶点
	 *                                 则会画出a,b,c三个顶点之间的连线。
	 */
	virtual void          draw_line( const osVec3D* _verList,int _vnum,
		                   DWORD _color = 0xffffffff,float _width =1.0f ) = 0;


# if __ALLCHR_SIZEADJ__
	//! 使用这个接口，可以调整整个场景中所有skinMesh物品的大小，
	//! 此处的调整为统一调整。包括树和草相关的物品。
	virtual void          set_allCharSizeAdj( osVec3D& _sizeAdj ) = 0;
# endif 

	
//--------------------------------------------------------------------------------
//@{ tzz added for galaxy shader effect
//--------------------------------------------------------------------------------

	/*!	\fn virtual void gala_actGrowScene(BOOL _bActivate = TRUE) = 0;
	*	\brief : activate the grow scene after been initialized
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actGrowScene(BOOL _bActivate = TRUE) = 0;

	/*! \fn virtual void gala_initGrowScene(ini _iScreenWidth,int _iScreenHeight) =0;
	*	\brief : initialize the grow scene 
	*
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\return void
	*/
	virtual void gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) = 0;

	
	//! initialize the hdr data
	virtual void init_realHDRScene(int _iScreenWidth,int _iScreenHeight) = 0 ;

	virtual void act_waterEff(BOOL _bAct = TRUE) = 0;

	//! activate the scene effect
	virtual void act_realHDRScene(BOOL _bActivate = TRUE) = 0;

	//! enable light 
	virtual void enable_light(BOOL _bEnable) = 0;

	//! set the day time for light
	virtual void set_dayTimeForLight(size_t _dayTime) = 0;

	//! set the day time for light
	virtual void set_constLight(const osColor& _color,const osVec3D&,float _range) = 0;


# if  GALAXY_VERSION

	/*! \fn virtual void gala_genExplode(gala_exlode* _explode) = 0;
	*	\brief : generate a explode effect
	*
	*	\param _explode		:  struct of exploding effect
	*	\return void
	*/
	virtual void gala_genExplode(gala_explode* _explode) = 0;

	/*! \fn virtual void gala_refreshSpaceHole(gala_refreshSpaceHole* _hole) = 0;
	*	\brief : refresh the space hole position
	*
	*	\param _bullet		:  struct of space hole effect
	*	\return void
	*/
	virtual void gala_refreshSpaceHole(gala_spaceHole* _hole) = 0;

	/*! \fn virutal void gala_delSpaceHole(int	_index) = 0;
	*	\brief : delete the shader effect of bullet
	*	
	*	\param _index		: index of bullet effect
	*	\return void		
	*/
	virtual void gala_delSpaceHole(int _index) = 0;

	/*! \fn virtual void gala_refreshBullet(gala_spaceHole* _bullet)
	*	\brief : refresh the bullet position and direction
	*			because the bullet track is no longer a line
	*
	*	\param _bullet : bullet struct
	*	\return void
	*/
	//virtual void gala_refreshBullet(gala_spaceHole* _bullet) = 0;

	/*! \fn virtual void gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY)) = 0;
	*	\biref : begin the space hole effect
	*	
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\param _iWidthNum		: horizontal grid Number
	*	\param _iHeightNum		: vertical grid Number
	*	\param _iStartX		: rotating center x
	*	\param _iStartY		: rotating center y
	*/
	virtual void gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY) = 0;

	/*! \fn virtual BOOL gala_isRenderHole(void) = 0;
	*	\biref : is render hole if yes return TRUE;
	*
	*	\param void
	*	\return BOOL	: TRUE if rendering hole;
	*/
	virtual BOOL gala_isRenderHole(void) = 0;


	/*! \fn virtual void gala_initGrowScene(ini _iScreenWidth,int _iScreenHeight) =0;
	*	\brief : initialize the grow scene 
	*
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\return void
	*/
	virtual void gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) = 0;

	

	/*!	\fn virtual void gala_actHeroCorona(BOOL _bActivate = TRUE) = 0;
	*	\brief : draw the hero around corona
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actHeroCorona(BOOL _bActivate = TRUE) = 0;

	/*!	\fn void gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP)
	*	\brief show the blood process of craft in game
	*
	*	\param _craftPos	: the screen position of craft
	*	\param _fCraftScale	: the mesh of craft scale
	*	\param _fMaxHP		: the max HP of npc air craft
	*	\param _fCurrHP		: current HP of NPC air craft
	*/
	virtual void gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP) = 0;




# endif //GALAXY_VERSION

# if __EFFECT_EDITOR__
	//!设计osa模型动画播放速度
	virtual void 			set_OSAPlaySpeed(float _speed) = 0;

# endif 

//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------

};

class os_Ray;
//! 三角片拾取接口,通过传入不同的选取方式,得到三角片列表.
/** 该接口主要用于碰撞检测,引擎内部需要做碰撞检测的物体都要派生该接口.
在碰撞检测之前每一帧,物理引擎都要从getTriangles接口得到相应的三角片列表.
以进行碰撞检测.*/
class ITriangleSelector : public os_refObj
{
public:

	//! 析构
	virtual ~ITriangleSelector() {};
	//! 返回所有可用的三角片个数
	virtual int getTriangleCount() const = 0;
	//! 得到所有的三角片列表
	//! \param[out] triangles 返回的三角列表数组
	//! \param[in] arraySize 三角列表数组个数
	//! \param[out] outTriangleCount 实际写入的三角片个数
	//! \param[in] transform 顶点变换矩阵,如果为空表示不进行变换.
	//! 该矩阵通常用于把欧氏空间变换到黎曼空间,这样做检测更容易
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const osMatrix* transform=0) const =0;

	//! 得到位于一个包围盒的三角片列表
	//! \param[in] box 要取得三角片的包围盒
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_bbox& box, 
		const osMatrix* transform=0) const= 0;

	//! 得到和射线相交的三角片列表
	//! \param[in] box 要取得三角片的相交的射线
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_Ray& line, 
		const osMatrix* transform=0) const= 0;

};

//@{
//  包含杂项函数。
# include "miskFunc.h"
//@} 




//==============================================================================================
//
//  从以下的函数中得到我们可以操纵的引擎接口.
//
//==============================================================================================
/** \brief 
 *  得到全局性的device Manager指针的函数.
 *
 *  使用这个函数来得到I_deviceManager的接口集,使用这个接口集来操作
 *  3D DEVICE.
 */
OSENGINE_API I_deviceManager*   get_deviceManagerPtr( void );
/** \brief
 *  得到InputManager接口的函数.
 */
OSENGINE_API I_inputMgr*        get_inputManagerPtr( void );


/** \brief
 *  得到I_reource接口集的函数.
 */
OSENGINE_API I_resource*        get_resourceMgr( void );

/** \brief
 *  得到场景操作接口集的函数。
 */
OSENGINE_API I_fdScene*         get_sceneMgr( void );

/** \brief
 *  得到全局的网络相关接口。
 */
OSENGINE_API I_socketMgr*       get_socketMgr( void );

//! 程序退出时，释放全局的静态引擎变量。
OSENGINE_API void              release_renderRes( void );

//! 通知上层设备状态回调 syq 2006-9-20
typedef int (*CALLBACK_DEVICE )(int,int);
OSENGINE_API void register_lostDeviceFunc( CALLBACK_DEVICE lpFunc );
OSENGINE_API void register_resetDeviceFunc( CALLBACK_DEVICE lpFunc );
