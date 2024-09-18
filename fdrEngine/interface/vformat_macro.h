/////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename:  vformat_macro.h
 *
 *  His:       River created @ 2003-12-25 
 *
 *  Des:       ���ں�ܵ��õ��Ķ����ʽ�ͺꡣ
 *  
 *             River added @ 2004-2-9�գ�ʹ������ļ���������Ժ���Ч�����õ�ȫ�ֱ�����
 *  
 */
/////////////////////////////////////////////////////////////////////////////////////////
# pragma once
# include "osInterface.h"



/** \brief
 *  ������Ϊ��Ե��Ŀ�Ż��İ汾������ͨ�汾,
 *  
 *  ʹ�ô˺�,ע�͵�һЩ��Ե�ò��������湦��.��С�����ڱ�����ļ���С.
 */
# define  GENERAL_VERSION  0 


//! �����и��ӵĳ��Ⱥ͸߶ȣ�ÿ�ŵ�ͼ�ĸ�����Ŀ��
#define MAX_GROUNDWIDTH		 64   //4 
#define MAX_GROUNDHEIGHT	 64   //4
#define CELL_COUNT			 4096 //16

//! ������ÿ�ŵ�ͼ��������ײ���ĸ�����Ŀ
#define GROUNDMASK_WIDTH	 128  //8 
#define GROUNDMASK_HEIGHT	 128  //8 


//! ���峡���и��ӵ�����ʵ�����еĴ�С.
//! ʹ�ñ�������,ʹ���ⲿ�����޸ĸ��ӵĴ�С.
extern OSENGINE_API float    TILE_WIDTH;

//! �����ͼ���ӵĿ�ȡ�
extern OSENGINE_API float    HALF_TWIDTH;
//! �����ײ��ͼ���ӵĿ�ȡ�
extern OSENGINE_API float    HALF_CTWIDTH;




//! һ�ŵ�ͼ�Ŀ�Ⱥ͸߶�.
# define TGMAP_WIDTH         (TILE_WIDTH*MAX_GROUNDWIDTH)
# define TGMAP_HEIGHT        (TILE_WIDTH*MAX_GROUNDHEIGHT)


//! ÿһ�����ӿ������ʹ�õ�������Ŀ��
# define MAXOBJNUM_PERTILE      3 

//! ������Ҫshader�ļ�����
# define TGSHADER_FNAME         "ini\\mapshader.ini"
# define  OBJECT_SHADERINI      "ini\\objectshader.ini"
# define  CHARACTER_SHADERINI    "ini\\charactershader.ini"
# define  BILLBOARD_SHADERINI    "ini\\bbshader.ini"
# define  SKY_SHADERINI          "ini\\skyshader.ini"






//! TGMgr������tileGround����Ŀ
extern OSENGINE_API int   MAXTG_INTGMGR;


# define   HEIGHT_SCALE  0.1f

//! ��Ұ���������е�������Ŀ��
# define MAXINVIEW_OBJNUM   256

//! ATTENTION: ����ʹ��СһЩֵ.
# define   INIT_TILEVSSIZE 4096

# define   INIT_VATNUM     1024

# define DYNAMIC_LOADOBJ   1

//! �����п���ʹ�õ���ද̬����Ŀ,����и���ĵƹ⣬���ϲ�ѡ������Ҫ�ĵƹ���д���
# define  MAX_LIGHTNUM    4


// ���ԡ�
# define RENDER_OBJ  1
# define MAX_OBJNUM  1024

//! �Ƿ�ʹ�þ�̬�Ķ��㻺������ŵ�ͼ�Ķ������ݡ�
//! ʹ�þ�̬�������Ӧ�ò�����̫������ƣ���Ϊ
//! �������̫Զ���Կ����ܻ�����Ҫ��ε���ſ���
//! �õ���ʧȥ��ʹ���˾�̬catch��ԭ�⡣
# define   USE_STATICVB   0


//!�������е�ͼ�����Ŀ¼��
# define   SCENEMAP_DIR   "data\\map\\"


//@{
//! ����������صĺ궨�塣
#define WSA_READ              WM_USER+1 
#define WSA_READDB             WM_USER+2 
#define WSA_ACCEPT             WM_USER+3 
#define MAX_PENDING_CONNECTS  ��8
//@}

 
//! ����Զ���������Զ������192
# define  MAX_VIEWCILPDIS       192


/** \brief
 *  �����Ľ�������С�ڵر����Զ��clip+��ֵ����������Ľ�������
 *  Ϊ�������Զclip+��ֵ�� 
 */
# define   FOG_OFFSET          50

/** \brief
 *  ���������õ��Ķ������ݸ�ʽ��
 */ 
struct OSENGINE_API os_sceVerTile
{
	osVec3D    m_vecPos;    
	osVec3D    m_vecNormal;
	DWORD      m_dwBaseColor;
	osVec2D    m_v2Uv;
	osVec2D    m_v2LmpUv;
};

//! ���ڻغ�����Ϸս����������Ⱦ
enum OSENGINE_API os_FightSceneRType
{
	//! ��ͨ����Ⱦ��ȫ����Ⱦ
	OS_COMMONRENDER = 0,
	
	//! ��Ⱦ��������������Ⱦ���߲��layer.
	OS_RENDER_LAYER,

	//! LAYER��Ⱦʱ���ڶ�����Ⱦ�Ĳ���,�����ڲ��ã��ⲿ�ò����������
	OS_RENDER_TOPLAYER,

};

//@{
//! ������ص�shader���ݡ�
# define LINESHADER_INI  "ini\\lineShader.ini"
# define LINESHADER_NAME "line"
//@} 

//! ����ض����fvfֵ.
# define FVF_LINEVERTEX  ( D3DFVF_XYZ | D3DFVF_DIFFUSE )  

/** \brief
*  �������õ���ѩ�Ķ������ݡ�*/
struct OSENGINE_API os_SnowVertex
{
	osVec3D   m_vec3Pos;
	osVec2D   m_vec2Uv;
};
//! ����ض����fvfֵ.
# define FVF_SNOWVERTEX  ( D3DFVF_XYZ |D3DFVF_TEX1)  

// 
//! �����еر���ͼ�õ�������
# define DEFAULT_MAPTEXDIR    "data\\texture\\" 


/** \brief
 *  ������ʾ����ɫ�����ݽṹ.
 */
struct OSENGINE_API os_sceVerDiffuse
{
	osVec3D   m_vecPos;
	DWORD     m_color;
};

/** \brief
 *  �������õ���BillBoard�Ķ������ݡ�
 */
struct OSENGINE_API os_billBoardVertex
{
	osVec3D   m_vec3Pos;
	DWORD     m_dwColor;
	osVec2D   m_vec2Uv;
};

/** \brief
 *  ��Ļ����ʾ����Ļ����εĶ����ʽ��
 *  
 *  ֻ��Ҫ��䶥�����Ļ����Ϳ����ˡ�
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
 *  ��������ͨ��Ʒ�Ķ����ʽ��
 * 
 *  ����ʹ������ͨ��dx�ƹ⡣
 */
struct OSENGINE_API os_objectVertex
{
	osVec3D   m_vecPos;
	osVec3D   m_vecNormal;
	osVec2D   m_vec2Tex1;
};
//! ��Ⱦʱ�õ���fvfֵ��
# define FVF_OBJECTVERTEX   (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)



/** \brief
 *  �����е���ɫ��Ʒʹ�õĶ����ʽ��
 *  
 */
struct OSENGINE_API os_objectColVertex
{
	osVec3D   m_vec3Pos;
	osVec3D   m_vec3Normal;

	//! �����˶������õ���diffuse���ݡ�
	DWORD     m_dwDiffuse;
	osVec2D   m_vec2Tex1;
};
//! ��Ⱦ����ɫ����ʱ�õ���fvfֵ��
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
 * ��������ȷ��3d device�Ļص�����������.
 */
typedef bool CFMPTR(  D3DCAPS9*,ose_VertexProcessingType,D3DFORMAT,D3DFORMAT );



/** \brief
 *  ����Ӳ���豸�Ĳ�ͬ,ʹ�ò�ͬ��skinning Method.
 *
 */
enum ose_skinMethod
{
	//! Geforce�������ʾӦ�ÿ���ʹ����һ������,ʹ���Կ���tnl������skin.
	OSE_D3DNONINDEXED,

	//  ATTENTION TO FIX:
	//! TnT�����Կ�ʹ�����ַ���,��vertexShader�������ȾҪ��????
	OSE_D3DINDEXED,

	//! Geforce3���ϼ�����Կ�ʹ����������,Ӳ����vertexShader.
	OSE_D3DINDEXEDVS,

	//! ���ڲ��Ե�����,ʹ�������VertexShader.
	OSE_SOFTWARE_D3DINDEXEDVS,

};



/** \brief
 *  ��������Ʒ����̬��ʱʹ�õĽṹ��
 */
struct OSENGINE_API os_dLightData
{
	//! ȫ�ֹ�
	osColor    m_sAmbientLight;
	//! ��̬��
	osColor    m_sDLight;

	//! ��̬���˥������
	float      m_fAttenuationDis;

	//! Ҫ����Ķ�̬���λ�á�
	osVec3D    m_vec3DLightPos;

};



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ����Ϊ����������ʾ���������ܵ�ȫ�ֱ�����
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//! �Ƿ���µ���
extern OSENGINE_API BOOL   g_bUpdateTile;
//! �Ƿ���ʾtileΪ�߿�ģʽ.
extern OSENGINE_API BOOL   g_bTileWireFrame;
//! �Ƿ���ʾ����Ϊ���߿�ģʽ.
extern OSENGINE_API BOOL   g_bCharWireFrame;
//! �Ƿ���ʾ��ƷΪ��ģ��ģʽ.
extern OSENGINE_API BOOL   g_bObjWireFrame;

//! ���������Ƿ���ʾ��Ӱ��
extern OSENGINE_API BOOL   g_bHeroShadow;

//! �Ƿ�ǿ����ʾskinMesh�϶�Ӧ�ĵڶ�����ͼ��
extern OSENGINE_API BOOL   g_bForceSecEff;

//! �Ƿ�ʹ��shaderDebug���ܡ�
extern OSENGINE_API BOOL   g_bShaderDebug;

//! �Ƿ�ʹ�ó����еĶ������塣
extern OSENGINE_API BOOL   g_bUseAnimationObj;

//! �Ƿ�ʹ�ڵ�ס���������Ʒ��ð�͸����
extern OSENGINE_API BOOL   g_bAlphaShelter;

//  ����������ص���Ϣ��
//! �������Ƿ�ʹ����
extern OSENGINE_API BOOL   g_bUseFog;
//! ��������Ŀ�ʼ�㡣
extern OSENGINE_API float  g_fFogStart;
//! ��������Ľ����㡣
extern OSENGINE_API float  g_fFogEnd;
//! �����������ɫ��Ϣ��
extern OSENGINE_API DWORD  g_dwFogColor;


//! ���ڿ��Ƴ������Ƿ�ʹ�ö�̬�⡣
extern OSENGINE_API BOOL    g_bEnableDLight;
//@{
//! �Ͼֹ�Ͷ�̬����ص�ȫ�ֱ�����
extern OSENGINE_API float   g_fAmbiR;
extern OSENGINE_API float   g_fAmbiG;
extern OSENGINE_API float   g_fAmbiB;

//! �˴�Ϊ��������Ķ�̬���ֵ��
extern OSENGINE_API float   g_fDlR ;
extern OSENGINE_API float   g_fDlG ;
extern OSENGINE_API float   g_fDlB ;
extern OSENGINE_API float   g_fDlRadius;
//@} 

extern OSENGINE_API BOOL	g_bRendSky;


//! �������õ�������Ŀ�ȣ�Ĭ��ֵΪ12,�����ı��С��
//! ���ڳ�ʼ����ʱ���޸����ȫ�ֱ�����
//extern OSENGINE_API int     DEVICE_CHAR_WIDTH;


//! Զɽ�����Զ����ʾ����ɫ.
extern OSENGINE_API DWORD   g_dwFarTerrColor;
//! Զɽ����ľ��� 0ʱ��Ӱ��Զɽ,Խ��Ӱ��Խ��
extern OSENGINE_API int   g_dwTerrFogDistance ;
extern OSENGINE_API int   g_dwSkyFogDistance ;


//! ���ڿ�������ľ�ϸ��.����Ĵ�С��������ֵ.
extern OSENGINE_API DWORD  g_dwTexDiv; 

extern OSENGINE_API BOOL g_disableYY; 


//! ��ǰ���������еĳ��������֡�
extern OSENGINE_API char   g_szMapName[16];

//! �����ͼʱ,�����õ��ı���ͼƬ��������.����"back"��ʾui\\back\\Ŀ¼�°���ʽ�ĵ���ͼƬ.
extern OSENGINE_API char   g_szMapLoadBack[16];


//@{
/** \brief
 *  �Ƿ�ʹ��ȫ�ֵķ����,�����ʹ��ȫ�ֵķ����Ļ����򳡾�����Ⱦ��Ʒ��ʱ�� 
 *  ���򿪵ƹ⣬��ʱ��������Ҫ���е���Ʒ��ʹ�õ���ɫ������ʾ���ƹ�Ч����
 */
extern OSENGINE_API BOOL     g_bUseGDirLight;
/** \brief
 *  ȫ�ֹ��λ�á�
 *  
 *  ���㳡����ʹ�õ���ɫ����Ʒ��Ҳ��Ҫ�������λ�ã����λ��������Ⱦʱ�ƹ��Ĭ��λ�á�
 */
extern OSENGINE_API osVec3D  g_vec3LPos;
//! ������ȫ�ֵķ�������ɫ��
extern OSENGINE_API float    g_fDirectLR;
extern OSENGINE_API float    g_fDirectLG;
extern OSENGINE_API float    g_fDirectLB;
//@} 
extern OSENGINE_API float    g_fBoltFanAngle;
extern OSENGINE_API char	 g_CurFarTerrain[255];
extern OSENGINE_API BOOL     g_bHasBolt;
//! �Ƿ�ʹ��HDR Light
extern OSENGINE_API BOOL   g_bUseHDRLight; 

extern OSENGINE_API BOOL   g_SceneSoundBox;

//! ȫ�ֱ��������Ƶ�ǰ��Ʒ������Ұʱ�ĵ��뵭��״̬��
extern OSENGINE_API bool     g_bFadeEnable;
//! ȫ�ֵĿ��Ƶ��뵭����Ʒʱ��ı�����
extern OSENGINE_API float    ALPHA_FADETIME;
//! ��ס�������Ʒ�İ�͸��ʱ��.
extern OSENGINE_API float    HALF_FADETIME;


//! �Ƿ��ڳ�������ʾ����ĵڶ�����ͼ��
extern OSENGINE_API BOOL     g_bUseSecondTexOnCha;

//! �Ƿ���Ⱦ�ر��ϸ������.
extern OSENGINE_API BOOL     g_bUseTileDetailTex;

//! ��պ�Զ������ʱ���ۺ���Ҫ��ʱ�䡣
extern OSENGINE_API float    g_fSkyTerrAnitime;

//! TEST CODE:��ǰ�Ƿ��ڶ��̵߳�ͼ����ʱ��
extern OSENGINE_API BOOL     g_bMapLoadding;


//! ���峡�����õ���alpha Ref��ֵ,ʹ��ͳһ��ֵ.
#  define GLOBAL_MAXALPHAREF     0x0000005f
#  define GLOBAL_MIDALPHAREF     0x0000002f
#  define GLOBAL_MINMIDALPHAREF  0x00000006
#  define GLOBAL_MINALPHAREF     0x00000001


//@{
// ������Ϊ�����ڲ������ı�����ֻ�ܵõ��� ����������Щ������
//! �õ���ȫ�ֵ�view&proj Matrix
extern OSENGINE_API osMatrix        g_matView;
extern OSENGINE_API osMatrix        g_matProj;
extern OSENGINE_API osVec3D			g_vec3ViewPos;

//! ȫ���õ�����Ⱦ��ʽ:ֻ�ܵõ����������á�
extern OSENGINE_API ose_VertexProcessingType   g_vertexProType;
//! ȫ���õ���skinMethod.
extern OSENGINE_API ose_skinMethod            g_eSkinMethod;
//@} 

//! �Ƿ���ʾ��̬��obj��Ʒ��
extern OSENGINE_API BOOL                     g_bDisplayStaticMesh;

//! �Ƿ�ʹ�������
extern OSENGINE_API BOOL					 g_bUseVolumeFog ;
//! ������Ƿ��ʼ����
extern OSENGINE_API BOOL					 g_bVolumeFogInited ;

//! ȫ���Ƿ�ʹ������ѹ��.
extern OSENGINE_API BOOL                     g_bUseDXTC;


//! ���������ת���ٶ�
extern OSENGINE_API float                    g_fBkSkyRotSpeed;
//{@ windy add 7.5
//! �������1��ת���ٶ�
extern OSENGINE_API float					g_fBkSky1RotSpeed;
//! �������2��ת���ٶ�
extern OSENGINE_API float					g_fBkSky2RotSpeed;
//@}
//! ������������Ƿ���Ⱦ�����еĵ��Ρ�
extern OSENGINE_API BOOL                     g_bRenderTerr;
//! �Ƿ���ʾ��ײ��Ϣ
extern OSENGINE_API BOOL                     g_bRenderCollision;


//! ������������Ƿ����Զ���ĵ��Σ������Ϊfalse,��Զ���ĵ���һ�����������������ļ���
extern OSENGINE_API BOOL                     g_bClipFarTerr;

//! ���������������ڿ��Ƶ�������ڵر�����ĵر���,���Ĵ�С���͹����ʧ��ʱ��
extern OSENGINE_API float                    g_fTerrCursorSize;
extern OSENGINE_API float                    g_fTerrCursorVanishTime;

//! ����궯�����ٶ�
extern OSENGINE_API float                    g_fMouseCursorFrameAniTime;

//! �������Ƿ��Զ���������ر�����ײ���,Ĭ�������������⡣
extern OSENGINE_API BOOL                     g_bCamTerrCol;


//! ����Զ���λ�ĳ��ٺͼ��ٶ�
extern OSENGINE_API float                    g_fInitAutoResetSpeed;
extern OSENGINE_API float                    g_fAutoResetAcce;

//@{ ����Ϊ�����ڲ�ʹ�õı�����û�е������ⲿ����ʹ�á���
//! �Ƿ�ʹ��16λɫ
extern BOOL                     g_bUse16Color;

//! ��ͼ��x,z�����ϵ�������ֵ.�ǵر������*3
extern float                    g_fMapMaxX;
extern float                    g_fMapMaxZ;

//! ������ʹ�õļ���Ӱ�õ�������ͼ���ӰĬ�ϴ�С
extern char                     g_szFakeShadowTexture[128];
extern float                    g_fFakeShadowSize;

//@} 


//! ��������ʾ����������Ŀ,��������ڵ����ﳬ������Ŀ����������������Ĵ���Ŀ���ﲢ��ʾ��
//! Ĭ��������ʾ96������
extern OSENGINE_API int                    MAX_INVIEWSM_NUM;

/**  
 * �ر�chunk��Ⱦʱ��ϸ�ڿ��Ʊ���,���������chunk������Ⱦ��㣬ԶһЩchunk����ϸ�ڱ����
 * �˱���ֵΪ1.0��ʱ��ϸ����ߣ���ֵΪ0.0��ʱ��ϸ�����,Ĭ��Ϊ1.0,��ϸ��ȫ����
 */
extern OSENGINE_API float                  g_fTerrDetailCoef;


//! �Ƿ�ʹ��ȫ�������.
extern OSENGINE_API BOOL                   g_bUseFSAA;

//! �Ƿ�ʹ�ö������.
extern OSENGINE_API BOOL                   g_bUseAniCursor;

//��River added @ 2008-4-28:�Ƿ����������ڳ�����ͼ��
extern OSENGINE_API BOOL                   g_bClipCamInMap;


//! River added @ 2008-5-23: �Ƿ��ڳ����ڻ������ƽ��Ͷ����Ӱ
extern OSENGINE_API BOOL                   g_bDrawProjectShadow;

//! River added @ 2008-11-19:���ڿ���������Ӱ��͸���ȣ�0��ȫ͸��.1��ȫ��͸��
extern OSENGINE_API float                  g_fShadowChroma;

//! River added @ 2008-12-15:�Ƿ�ʹ��ˮ�浹Ӱ,�����ֵΪfalse,��ʹ�����Ч����ˮ��
extern OSENGINE_API BOOL                   g_bWaterReflect;

//! River added @ 2009-8-30: �Ƿ���Ⱦ����������Ʒ��Ӱ,�����ֵΪfalse,����Ⱦ��������Ʒ�͵����е�Ӱ��
//!                          ��ֵĬ��Ϊ�档
extern OSENGINE_API BOOL                   g_bTerrWaterReflect;

//! River added @ 2009-5-3:���ڴ������������֮�����Ʒ��͸����ƫ��ֵ��
extern OSENGINE_API float                  g_fCamFocusOffset;

//! River added @ 2009-5-4:�����ϲ�õ���ǰ�����õ��������ڴ���Ϣ��
extern OSENGINE_API int                    g_iTextureSize;


/**  River @ 2009-5-10:
   �������Ӱ�Ƿ��뵭������Ϊȫ��ֻ��һ����Ӱ��square,����һ������
   ����Ӱ���뵭����Ӱ�����е�������Ӱ���뵭�������Դ˹��ܽ�������ǿ��
   ���������ʱ��,ʹ����ɺ󣬾���ص���ֵ��
*/
extern OSENGINE_API bool                   g_bShadowFade;


//! ˮ�浹Ӱ����.0���.2:���.
extern OSENGINE_API int                    g_iReflectionGrade;

//! ��¼���豸��shader�汾��.
extern OSENGINE_API DWORD                  g_iMajorVSVersion;
extern OSENGINE_API DWORD                  g_iMinorVSVersion; 

extern OSENGINE_API DWORD                  g_iMajorPSVersion;
extern OSENGINE_API DWORD                  g_iMinorPSVersion; 

//! �Ƿ�dx��������.Ĭ��Ϊfalse,��Լ�ڴ棬���豸�һ�����
//! ��ֵ�����ڳ����ʼ��ʱ���ã�����������������ã��豸�һػ��������
extern OSENGINE_API BOOL                   g_bManagedTexture;                  


//! River @ 2010-6-25:�Ƿ�ʹ����Ļ�ⲨЧ��.
extern OSENGINE_API BOOL                   g_bScreenBlastWave;                  

//! River @ 2010-12-28:��Ϸ����������Ļ�������С����ֵ�����㣬С��2��
//  �����Ǹ���������ֵԽ��ռ���ڴ�Խ�࣬����������ʱ����Ӳ�̵Ľ���ԽС��
extern OSENGINE_API float                  g_fCharacterTexBufScale;                  

// River @ 2011-2-10:����ȫ�ֵı��������ڴ�������ʹ����ĳ����Ժÿ�һЩ��
extern OSENGINE_API BOOL                   g_bCharRotInterpolateStatus;

//! ��������������skinMesh����Ŀ��// ATTENTION TO FIX:
//! ������У���ø����MAX_RTSHADOWNUM��ʹ����ͬ������
//  River @ 2011-2-25:�޸�Ϊȫ�ֵı���.
extern OSENGINE_API int                    MAX_SMINVIWE;



////////////!ͨ���ļ��ϲ���ص����ݽṹ�����ڰ���Ϸ�ڵ�һЩС�ļ���Ԥ�����һ������ļ���
struct SFileHead
{
	char  szTag[4];
	DWORD dwVersion;
	
	//!�ļ�����Ŀ
	DWORD dwFileNum;

	//!�ļ���ƫ��
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
	//!�ļ�������
	char szFileName[32];

	//!��ǰ���泤��
	DWORD  dwBufLenth;

	// ÿ�ļ���ص����ݽṹ��
	// byte* Buffer;

};

