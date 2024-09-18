/////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename:  vformat_macro.h
 *
 *  His:       River created @ 2003-12-25 
 *
 *  Des:       用于后管道用到的顶点格式和宏。
 *  
 *             River added @ 2004-2-9日，使用这个文件，加入调试和特效控制用的全局变量。
 *  
 */
/////////////////////////////////////////////////////////////////////////////////////////
# pragma once
# include "osInterface.h"



/** \brief
 *  引擎是为边缘项目优化的版本还是普通版本,
 *  
 *  使用此宏,注释掉一些边缘用不到的引擎功能.减小引擎在编译后文件大小.
 */
# define  GENERAL_VERSION  0 


//! 引擎中格子的长度和高度，每张地图的格子数目。
#define MAX_GROUNDWIDTH		 64   //4 
#define MAX_GROUNDHEIGHT	 64   //4
#define CELL_COUNT			 4096 //16

//! 引擎中每张地图中用于碰撞检测的格子数目
#define GROUNDMASK_WIDTH	 128  //8 
#define GROUNDMASK_HEIGHT	 128  //8 


//! 定义场景中格子的在现实世界中的大小.
//! 使用变量定义,使在外部容易修改格子的大小.
extern OSENGINE_API float    TILE_WIDTH;

//! 半个地图格子的宽度。
extern OSENGINE_API float    HALF_TWIDTH;
//! 半个碰撞地图格子的宽度。
extern OSENGINE_API float    HALF_CTWIDTH;




//! 一张地图的宽度和高度.
# define TGMAP_WIDTH         (TILE_WIDTH*MAX_GROUNDWIDTH)
# define TGMAP_HEIGHT        (TILE_WIDTH*MAX_GROUNDHEIGHT)


//! 每一个格子可以最多使用的物体数目。
# define MAXOBJNUM_PERTILE      3 

//! 定义主要shader文件名。
# define TGSHADER_FNAME         "ini\\mapshader.ini"
# define  OBJECT_SHADERINI      "ini\\objectshader.ini"
# define  CHARACTER_SHADERINI    "ini\\charactershader.ini"
# define  BILLBOARD_SHADERINI    "ini\\bbshader.ini"
# define  SKY_SHADERINI          "ini\\skyshader.ini"






//! TGMgr中最多的tileGround的数目
extern OSENGINE_API int   MAXTG_INTGMGR;


# define   HEIGHT_SCALE  0.1f

//! 视野内最多可以有的物体数目。
# define MAXINVIEW_OBJNUM   256

//! ATTENTION: 可以使用小一些值.
# define   INIT_TILEVSSIZE 4096

# define   INIT_VATNUM     1024

# define DYNAMIC_LOADOBJ   1

//! 场景中可以使用的最多动态光数目,如果有更多的灯光，则上层选择最重要的灯光进行创建
# define  MAX_LIGHTNUM    4


// 测试。
# define RENDER_OBJ  1
# define MAX_OBJNUM  1024

//! 是否使用静态的顶点缓冲区存放地图的顶点数据。
//! 使用静态顶点冲区应该不会有太大的优势，因为
//! 顶点相隔太远，显卡可能还是需要多次调入才可以
//! 用到，失去了使用了静态catch的原意。
# define   USE_STATICVB   0


//!　场景中地图存放在目录。
# define   SCENEMAP_DIR   "data\\map\\"


//@{
//! 用于网络相关的宏定义。
#define WSA_READ              WM_USER+1 
#define WSA_READDB             WM_USER+2 
#define WSA_ACCEPT             WM_USER+3 
#define MAX_PENDING_CONNECTS  　8
//@}

 
//! 相相远剪切面的最远矩离是192
# define  MAX_VIEWCILPDIS       192


/** \brief
 *  如果雾的结束矩离小于地表的最远矩clip+此值，则设置雾的结束矩离
 *  为相机的最远clip+此值。 
 */
# define   FOG_OFFSET          50

/** \brief
 *  场景格子用到的顶点数据格式。
 */ 
struct OSENGINE_API os_sceVerTile
{
	osVec3D    m_vecPos;    
	osVec3D    m_vecNormal;
	DWORD      m_dwBaseColor;
	osVec2D    m_v2Uv;
	osVec2D    m_v2LmpUv;
};

//! 用于回合制游戏战斗场景的渲染
enum OSENGINE_API os_FightSceneRType
{
	//! 普通的渲染，全部渲染
	OS_COMMONRENDER = 0,
	
	//! 渲染场景背景后，再渲染更高层的layer.
	OS_RENDER_LAYER,

	//! LAYER渲染时，第二次渲染的参数,引擎内部用，外部用不到这个参数
	OS_RENDER_TOPLAYER,

};

//@{
//! 画线相关的shader数据。
# define LINESHADER_INI  "ini\\lineShader.ini"
# define LINESHADER_NAME "line"
//@} 

//! 线相关顶点的fvf值.
# define FVF_LINEVERTEX  ( D3DFVF_XYZ | D3DFVF_DIFFUSE )  

/** \brief
*  场景中用到的雪的顶点数据。*/
struct OSENGINE_API os_SnowVertex
{
	osVec3D   m_vec3Pos;
	osVec2D   m_vec2Uv;
};
//! 线相关顶点的fvf值.
# define FVF_SNOWVERTEX  ( D3DFVF_XYZ |D3DFVF_TEX1)  

// 
//! 场景中地表贴图用到的纹理。
# define DEFAULT_MAPTEXDIR    "data\\texture\\" 


/** \brief
 *  用于显示顶点色的数据结构.
 */
struct OSENGINE_API os_sceVerDiffuse
{
	osVec3D   m_vecPos;
	DWORD     m_color;
};

/** \brief
 *  场景中用到的BillBoard的顶点数据。
 */
struct OSENGINE_API os_billBoardVertex
{
	osVec3D   m_vec3Pos;
	DWORD     m_dwColor;
	osVec2D   m_vec2Uv;
};

/** \brief
 *  屏幕上显示的屏幕金边形的顶点格式。
 *  
 *  只需要填充顶点的屏幕坐标就可以了。
 *  
 */
struct OSENGINE_API os_screenVertex
{
	osVec4D    m_vecPos;
	DWORD      m_dwDiffuse;
	osVec2D    m_vecUv;

	
public:
	os_screenVertex();
};

/** \brief
 *  场景中普通物品的顶点格式。
 * 
 *  可以使用最普通的dx灯光。
 */
struct OSENGINE_API os_objectVertex
{
	osVec3D   m_vecPos;
	osVec3D   m_vecNormal;
	osVec2D   m_vec2Tex1;
};
//! 渲染时用到的fvf值。
# define FVF_OBJECTVERTEX   (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)



/** \brief
 *  场景中点着色物品使用的顶点格式。
 *  
 */
struct OSENGINE_API os_objectColVertex
{
	osVec3D   m_vec3Pos;
	osVec3D   m_vec3Normal;

	//! 加入了顶点着用到的diffuse数据。
	DWORD     m_dwDiffuse;
	osVec2D   m_vec2Tex1;
};
//! 渲染点着色顶点时用到的fvf值。
# define FVF_OBJECTCOLVERTEX  (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)


/** \brief 
 *  Enum ose_VertexProcessingType
 * 
 *  Desc: Enumeration of all possible D3D vertex processing types.
 *
 */
enum ose_VertexProcessingType
{
    SOFTWARE_VP,
    MIXED_VP,
    HARDWARE_VP,
    PURE_HARDWARE_VP
};

/** \brief 
 * 声明用于确认3d device的回调函数的类型.
 */
typedef bool CFMPTR(  D3DCAPS9*,ose_VertexProcessingType,D3DFORMAT,D3DFORMAT );



/** \brief
 *  根据硬件设备的不同,使用不同的skinning Method.
 *
 */
enum ose_skinMethod
{
	//! Geforce级别的显示应该可以使用这一个方法,使用显卡的tnl功能做skin.
	OSE_D3DNONINDEXED,

	//  ATTENTION TO FIX:
	//! TnT级的显卡使用这种方法,比vertexShader的软件渲染要快????
	OSE_D3DINDEXED,

	//! Geforce3以上级别的显卡使用这种类型,硬件的vertexShader.
	OSE_D3DINDEXEDVS,

	//! 用于测试的类型,使用软件的VertexShader.
	OSE_SOFTWARE_D3DINDEXEDVS,

};



/** \brief
 *  场景中物品处理动态光时使用的结构。
 */
struct OSENGINE_API os_dLightData
{
	//! 全局光
	osColor    m_sAmbientLight;
	//! 动态光
	osColor    m_sDLight;

	//! 动态光的衰减矩离
	float      m_fAttenuationDis;

	//! 要处理的动态光的位置。
	osVec3D    m_vec3DLightPos;

};



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  以下为控制引擎显示和其它功能的全局变量。
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//! 是否更新地形
extern OSENGINE_API BOOL   g_bUpdateTile;
//! 是否显示tile为线框模式.
extern OSENGINE_API BOOL   g_bTileWireFrame;
//! 是否显示人物为线线框模式.
extern OSENGINE_API BOOL   g_bCharWireFrame;
//! 是否显示物品为线模框模式.
extern OSENGINE_API BOOL   g_bObjWireFrame;

//! 主角人物是否显示阴影。
extern OSENGINE_API BOOL   g_bHeroShadow;

//! 是否强制显示skinMesh上对应的第二层贴图。
extern OSENGINE_API BOOL   g_bForceSecEff;

//! 是否使用shaderDebug功能。
extern OSENGINE_API BOOL   g_bShaderDebug;

//! 是否使用场景中的动画物体。
extern OSENGINE_API BOOL   g_bUseAnimationObj;

//! 是否使遮挡住了人物的物品变得半透明。
extern OSENGINE_API BOOL   g_bAlphaShelter;

//  场景中雾相关的信息。
//! 场景中是否使用雾。
extern OSENGINE_API BOOL   g_bUseFog;
//! 场景中雾的开始点。
extern OSENGINE_API float  g_fFogStart;
//! 场景中雾的结束点。
extern OSENGINE_API float  g_fFogEnd;
//! 场景中雾的颜色信息。
extern OSENGINE_API DWORD  g_dwFogColor;


//! 用于控制场景中是否使用动态光。
extern OSENGINE_API BOOL    g_bEnableDLight;
//@{
//! 合局光和动态光相关的全局变量。
extern OSENGINE_API float   g_fAmbiR;
extern OSENGINE_API float   g_fAmbiG;
extern OSENGINE_API float   g_fAmbiB;

//! 此处为跟随人物的动态光的值。
extern OSENGINE_API float   g_fDlR ;
extern OSENGINE_API float   g_fDlG ;
extern OSENGINE_API float   g_fDlB ;
extern OSENGINE_API float   g_fDlRadius;
//@} 

extern OSENGINE_API BOOL	g_bRendSky;


//! 场景中用到的字体的宽度，默认值为12,如果想改变大小，
//! 则在初始化的时候，修改这个全局变量。
//extern OSENGINE_API int     DEVICE_CHAR_WIDTH;


//! 远山和天空远景显示的颜色.
extern OSENGINE_API DWORD   g_dwFarTerrColor;
//! 远山与雾的距离 0时雾不影响远山,越大影响越深
extern OSENGINE_API int   g_dwTerrFogDistance ;
extern OSENGINE_API int   g_dwSkyFogDistance ;


//! 用于控制纹理的精细度.纹理的大小会除以这个值.
extern OSENGINE_API DWORD  g_dwTexDiv; 

extern OSENGINE_API BOOL g_disableYY; 


//! 当前引擎中运行的场景的名字。
extern OSENGINE_API char   g_szMapName[16];

//! 调入地图时,引擎用到的背景图片集的名字.比如"back"表示ui\\back\\目录下按格式的调入图片.
extern OSENGINE_API char   g_szMapLoadBack[16];


//@{
/** \brief
 *  是否使用全局的方向光,如果不使用全局的方向光的话，则场景中渲染物品的时候 
 *  不打开灯光，此时场景中需要所有的物品都使用点着色才能显示出灯光效果。
 */
extern OSENGINE_API BOOL     g_bUseGDirLight;
/** \brief
 *  全局光的位置。
 *  
 *  就算场景中使用点着色的物品，也需要设置这个位置，这个位置人物渲染时灯光的默认位置。
 */
extern OSENGINE_API osVec3D  g_vec3LPos;
//! 场景中全局的方向光的颜色。
extern OSENGINE_API float    g_fDirectLR;
extern OSENGINE_API float    g_fDirectLG;
extern OSENGINE_API float    g_fDirectLB;
//@} 
extern OSENGINE_API float    g_fBoltFanAngle;
extern OSENGINE_API char	 g_CurFarTerrain[255];
extern OSENGINE_API BOOL     g_bHasBolt;
//! 是否使用HDR Light
extern OSENGINE_API BOOL   g_bUseHDRLight; 

extern OSENGINE_API BOOL   g_SceneSoundBox;

//! 全局变量，控制当前物品进入视野时的淡入淡出状态。
extern OSENGINE_API bool     g_bFadeEnable;
//! 全局的控制淡入淡出物品时间的变量。
extern OSENGINE_API float    ALPHA_FADETIME;
//! 挡住相机的物品的半透明时间.
extern OSENGINE_API float    HALF_FADETIME;


//! 是否在场景中显示人物的第二层贴图。
extern OSENGINE_API BOOL     g_bUseSecondTexOnCha;

//! 是否渲染地表的细节纹理.
extern OSENGINE_API BOOL     g_bUseTileDetailTex;

//! 天空和远景渐变时，熔合需要的时间。
extern OSENGINE_API float    g_fSkyTerrAnitime;

//! TEST CODE:当前是否处于多线程地图调入时刻
extern OSENGINE_API BOOL     g_bMapLoadding;


//! 定义场景中用到的alpha Ref的值,使用统一的值.
#  define GLOBAL_MAXALPHAREF     0x0000005f
#  define GLOBAL_MIDALPHAREF     0x0000002f
#  define GLOBAL_MINMIDALPHAREF  0x00000006
#  define GLOBAL_MINALPHAREF     0x00000001


//@{
// 　以下为引擎内部传出的变量，只能得到， 不能设置这些变量。
//! 用到的全局的view&proj Matrix
extern OSENGINE_API osMatrix        g_matView;
extern OSENGINE_API osMatrix        g_matProj;
extern OSENGINE_API osVec3D			g_vec3ViewPos;

//! 全局用到的渲染方式:只能得到，不能设置。
extern OSENGINE_API ose_VertexProcessingType   g_vertexProType;
//! 全局用到的skinMethod.
extern OSENGINE_API ose_skinMethod            g_eSkinMethod;
//@} 

//! 是否显示静态的obj物品。
extern OSENGINE_API BOOL                     g_bDisplayStaticMesh;

//! 是否使用体积雾。
extern OSENGINE_API BOOL					 g_bUseVolumeFog ;
//! 体积雾是否初始化。
extern OSENGINE_API BOOL					 g_bVolumeFogInited ;

//! 全局是否使用纹理压缩.
extern OSENGINE_API BOOL                     g_bUseDXTC;


//! 背景天空旋转的速度
extern OSENGINE_API float                    g_fBkSkyRotSpeed;
//{@ windy add 7.5
//! 背景天空1旋转的速度
extern OSENGINE_API float					g_fBkSky1RotSpeed;
//! 背景天空2旋转的速度
extern OSENGINE_API float					g_fBkSky2RotSpeed;
//@}
//! 这个变量控制是否渲染场景中的地形。
extern OSENGINE_API BOOL                     g_bRenderTerr;
//! 是否显示碰撞信息
extern OSENGINE_API BOOL                     g_bRenderCollision;


//! 这个变量控制是否剪切远处的地形，如果设为false,则远处的地形一块块跳出。否则，整齐的剪切
extern OSENGINE_API BOOL                     g_bClipFarTerr;

//! 以下两个变量用于控制点击场景内地表产生的地表光标,光标的大小，和光标消失的时间
extern OSENGINE_API float                    g_fTerrCursorSize;
extern OSENGINE_API float                    g_fTerrCursorVanishTime;

//! 鼠标光标动画的速度
extern OSENGINE_API float                    g_fMouseCursorFrameAniTime;

//! 引擎内是否自动把相机跟地表做碰撞检测,默认情况下做出检测。
extern OSENGINE_API BOOL                     g_bCamTerrCol;


//! 相机自动回位的初速和加速度
extern OSENGINE_API float                    g_fInitAutoResetSpeed;
extern OSENGINE_API float                    g_fAutoResetAcce;

//@{ 以下为引擎内部使用的变量，没有导出，外部不能使用。　
//! 是否使用16位色
extern BOOL                     g_bUse16Color;

//! 地图在x,z方向上的最大矩离值.是地表格子数*3
extern float                    g_fMapMaxX;
extern float                    g_fMapMaxZ;

//! 引擎内使用的假阴影用到的纹理和假阴影默认大小
extern char                     g_szFakeShadowTexture[128];
extern float                    g_fFakeShadowSize;

//@} 


//! 引擎内显示最多的人物数目,如果场景内的人物超出此数目，则查找离相机最近的此数目人物并显示。
//! 默认最多可显示96个人物
extern OSENGINE_API int                    MAX_INVIEWSM_NUM;

/**  
 * 地表chunk渲染时的细节控制变量,离相机近的chunk可以渲染多层，远一些chunk可以细节变代。
 * 此变量值为1.0的时候，细节最高，此值为0.0的时候，细节最低,默认为1.0,即细节全开。
 */
extern OSENGINE_API float                  g_fTerrDetailCoef;


//! 是否使用全屏抗锯齿.
extern OSENGINE_API BOOL                   g_bUseFSAA;

//! 是否使用动画光标.
extern OSENGINE_API BOOL                   g_bUseAniCursor;

//！River added @ 2008-4-28:是否把相机限制在场景地图内
extern OSENGINE_API BOOL                   g_bClipCamInMap;


//! River added @ 2008-5-23: 是否在场景内画人物的平面投射阴影
extern OSENGINE_API BOOL                   g_bDrawProjectShadow;

//! River added @ 2008-11-19:用于控制人物阴影的透明度，0完全透明.1完全不透明
extern OSENGINE_API float                  g_fShadowChroma;

//! River added @ 2008-12-15:是否使用水面倒影,如果此值为false,则使用最低效果的水面
extern OSENGINE_API BOOL                   g_bWaterReflect;

//! River added @ 2009-8-30: 是否渲染地形桥类物品倒影,如果此值为false,则不渲染场景内物品和地形有倒影。
//!                          此值默认为真。
extern OSENGINE_API BOOL                   g_bTerrWaterReflect;

//! River added @ 2009-5-3:用于处理相机与人物之间的物品半透明的偏移值。
extern OSENGINE_API float                  g_fCamFocusOffset;

//! River added @ 2009-5-4:用于上层得到当前引擎用到的纹理内存信息。
extern OSENGINE_API int                    g_iTextureSize;


/**  River @ 2009-5-10:
   人物的阴影是否淡入淡出，因为全局只画一次阴影的square,所以一个人物
   的阴影淡入淡出会影响所有的人物阴影淡入淡出，所以此功能仅仅用于强调
   单个人物的时候,使用完成后，尽快关掉此值。
*/
extern OSENGINE_API bool                   g_bShadowFade;


//! 水面倒影级别.0最高.2:最低.
extern OSENGINE_API int                    g_iReflectionGrade;

//! 记录主设备的shader版本号.
extern OSENGINE_API DWORD                  g_iMajorVSVersion;
extern OSENGINE_API DWORD                  g_iMinorVSVersion; 

extern OSENGINE_API DWORD                  g_iMajorPSVersion;
extern OSENGINE_API DWORD                  g_iMinorPSVersion; 

//! 是否dx管理纹理.默认为false,节约内存，但设备找回慢，
//! 此值必须在程序初始化时设置，如果程序运行中设置，设备找回会出现问题
extern OSENGINE_API BOOL                   g_bManagedTexture;                  


//! River @ 2010-6-25:是否使用屏幕光波效果.
extern OSENGINE_API BOOL                   g_bScreenBlastWave;                  

//! River @ 2010-12-28:游戏内人物纹理的缓冲区大小，此值大于零，小于2，
//  可以是浮点数，此值越大，占用内存越多，但调入人物时，与硬盘的交互越小。
extern OSENGINE_API float                  g_fCharacterTexBufScale;                  

// River @ 2011-2-10:加入全局的变量，用于创建人物使人物的出现稍好看一些。
extern OSENGINE_API BOOL                   g_bCharRotInterpolateStatus;

//! 定义视域内最多的skinMesh的数目。// ATTENTION TO FIX:
//! 如果可行，最好跟这个MAX_RTSHADOWNUM宏使用相同的数据
//  River @ 2011-2-25:修改为全局的变量.
extern OSENGINE_API int                    MAX_SMINVIWE;



////////////!通用文件合并相关的数据结构，用于把游戏内的一些小文件，预处理成一个大的文件。
struct SFileHead
{
	char  szTag[4];
	DWORD dwVersion;
	
	//!文件的数目
	DWORD dwFileNum;

	//!文件的偏移
	DWORD dwFileOfs;

	SFileHead()
	{
		dwFileNum = 0;
		dwFileOfs = 0;
		szTag[0] = NULL;
		dwVersion = 0;
	}
};

struct SFileBuf
{
	//!文件的名字
	char szFileName[32];

	//!当前缓存长度
	DWORD  dwBufLenth;

	// 每文件相关的数据结构。
	// byte* Buffer;

};

