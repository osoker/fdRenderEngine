//=======================================================================================================
/**  \file
 *   Filename: osInterface.h
 *   Desc:     Osok��ʾ�����ͷ�ļ�,���Ҫʹ��Osok����,�������ͷ�ļ�.���������ⲿ���������еĽṹ
 *             �ͽӿ�.
 *   His:      River created @ 4/14 2003.
 *
 *   "������С�γ�ֻ������һ����������,��������Ҫ�������."
 *  
 */
//=======================================================================================================
# pragma  once

#ifndef DIRECTINPUT_VERSION 
#define DIRECTINPUT_VERSION 0x0800
#endif

# pragma warning( disable : 195 )


//! ���嵼������
#ifdef FDRENGINE_EXPORTS
#define OSENGINE_API __declspec(dllexport)
#else
#define OSENGINE_API __declspec(dllimport)
#endif


/** 
 *  ��ʹ��common_mfc�汾��common�⣬��memoryManager.h�ĵ�һ�У���
 *   # define TRACKMEM_ALLOCK  0
 *   ���ֵ����Ϊ0, ����������mfc��ʹ�õ�common����Ⱦ�⣬��Ϊ1,��
 *   ȫ�ֵ�����new��delete. 
 */
# include "../../common/com_include.h"

//! RIVER added @ 2009-5-4:���QTang��Ŀ��һЩ�Ż��ʹ���
# define __QTANG_VERSION__   1


//! �Ƿ����Ϊ����༭���İ汾���ڵ���༭����װ�����ſ��Ա༭����Ķ�����װ��
# define __GLOSSY_EDITOR__   0


//! ���ʹ����Ч�༭������Ҫ����һЩ�����ò����ĺ�����
# define __EFFECT_EDITOR__   1

//@{
// Windy mod @ 2005-9-1 19:18:01
//Desc: ride �Ƿ�ʹ������༭������Ҫ����һЩ�����ò����ĺ�����
# define __RIDE_EDITOR__	1
//@}

//! �Ƿ�ʹ��movie Record�Ĺ��ܡ�
# define __MOVIE_RECORD__    0

//! ����һЩ�ļ��Ƿ������룬����spe.list,xml.list�ȵ�.
# define __PACKFILE_LOAD__   1

//! ���������Ƿ���256��С�����ơ�
# define __TEXTURE_SIZELIMIT__   0

//! �Ƿ����Ϊ���Ա༭����ڶ�����ͼ�İ汾.
# define __CHARACTER_EFFEDITOR__  1

# define  __LENSFLARE_EDITOR__		1

# define	__YY__ 0

// ����dx9.0��Error�����
# pragma comment(lib,"dxerr9" )


//@{
//! �������õĺ����ͺꡣ
# include "vformat_macro.h"
//@}



//! �����ͱ������ֲ�����ص�ͷ�ļ�
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
//	//! ������µ�
//	EFT_COUNT
//};


//==================================================================================================
//
//  �ӿڻ������ڽӿڵĽṹ����,ʹ�ô��麯��������ӿ�,��������������ʵ����Щ�ӿ�.
//  ���ⲿ�ĳ������ʹ�������ĳ���ֻ��ʹ�û��ǲ����ӿ�.
//
//==================================================================================================

// Class declear.
struct I_camera;



// ��miskFunc.hͷ�ļ��ж���
class OSENGINE_API os_aabbox;
class OSENGINE_API os_bsphere;
class OSENGINE_API os_bbox;

//! ����������Ŀ��С����
//
//1: ƽ����ӰͶ�书�ܣ������ڵر���ƽ�����Ŀ
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
	 * ��D3D��Gamma�ṹ������ͬ.
	 */
	WORD        m_arrRed[256];
	WORD        m_arrGreen[256];
	WORD        m_arrBlue[256];
	//@}
};







# if __CHARACTER_EFFEDITOR__
//! ���ʹ�õģ����ڸ�������ڶ���������Ч�����ݽṹ��
struct OSENGINE_API os_chaSecEffStruct
{
	/** \brief
	 *  �ڶ�����ͼ��shader��ʽ��
	 *
	 *  Ŀǰֻ֧������id: 
	 *  5: ���λ�����ͼ��
	 *  6: �ƶ�������ͼ��  
	 *  7: �Է���������ͼ��ÿ�����������Ϊ�ա�
	 */
	WORD      m_wEffectType;

	//@{
	//! �ڶ�����ͼ�����õ������ݡ�
	//! River @ 2010-2-25:������������Է�������ģʽ�У������Է�����������䰵
	//                    һ�����ڵ�ʱ�䡣
	float     m_fRotSpeed;

	/** \brief
	 *  ��ת��ò�ʹ�ó���Z��(0,0,1)�������������ת�ķ�ʽ��
	 *  ��Ϊ��תʱ�������һЩ������������,
	 */
	osVec3D   m_vec3RotAxis;

	//! ����X,Y������ƶ��ٶȡ�
	float     m_fUMoveSpeed;
	float     m_fVMoveSpeed;
	//@} 

	//! ��ǰ�ڶ�����ͼ���������֡�
	char      m_szSecTexName[64];
	//! ��������ͼ��������.
	char	  m_szThdTexName[64];

	//! ���Ĳ���ͼ��������.
	char      m_szFourthTexName[64];

};
# endif 



/**  \brief
 *   ���ڳ�ʼ��3dDevice�Ľṹ,���Լ������Ŀ���.
 *   ����ṹҲ����Reset3d Device.
 *
 *   5/20 2003
 *   ��Ϊ����ṹ�Ǻ����潻���Ľṹ,���Կ��Լ�������
 *   һЩ�����ʼ����Ҫ������.
 *
 *   5/31 2003
 *   �����ʼ����Ҫ������������һ��ini�ļ�einit.ini����.
 *
 */
struct OSENGINE_API os_deviceinit
{
	//! �ǲ���ȫ��,��Ŀǰ��ȫ����������£����ֵ��������Ϊfalse���С�
	bool         m_bFullScr;

	//! �ǲ���ʹ��d3d��Ӳ�����.
	bool         m_bHwCursor;
	//! ȫ��ʱ��Ļ���.
	DWORD        m_dwScrWidth;
	//! ȫ��ʱ��Ļ�߶�.
	DWORD        m_dwScrHeight;
	//! ȫ��ʱ��Ļ��ɫλ��.
	DWORD        m_dwColorBits;
	//! ȫ��ʱ���Ҫ���Zbufferλ��
	DWORD        m_dwDepthBits;
	//! ���Ҫ���Stencilλ��.
	DWORD        m_dwStencilBits;
	//! ʹ��Queryϵͳ���������ܣ���ֹ����
	BOOL		 m_bStartQuery;
	
	//! ��ʼ������豸������Ҫ��ʾ��ͼƬ��
	char         m_szDisTexName[128];
	//! ��ʼ��ͼƬ���õ�λ�ã�x�����ϵĸ���ֵ��ռ������Ļ�Ŀ�ȡ�
	float        m_fStartX;
	//! y�����ϵĸ���ֵ��ռһ����Ļ�ĸ߶ȡ�
	float        m_fStartY;

	//! ��Ļˢ���� add syq
	int          m_displayFrequency;

	//  River mod @ 2008-5-23:
	//! ���޸��ӿڴ�С
	BOOL         m_bResetViewPortSize;

	//
	os_deviceinit() 
	{
		m_bResetViewPortSize = TRUE;
		m_bStartQuery = FALSE;
	}
};



/** \brief
 *  ��ʾtemp��Ļ�����ʱҪ���Ľṹ.  
 * 
 *  ������ʾ��ʽ����򵥵���ʾ��ʽ,��d3dmanager
 *  ���֧��,�����м�ܵ���֧��.
 */
class OSENGINE_API os_screenPicDisp
{
public:
	//! Ҫ��ʾ��BillBoard���ĸ�����.
	os_screenVertex  m_scrVer[4];

	//! Ҫ��ʾ��Texture��ID.
	int             tex_id;

	/** \brief
	 *  Ҫ��ʾͼƬ��blend mode.1:ֱ����Ⱦ 2:Invsrccolor 3: SrcAlpha
	 * 
	 *  ��Ϊ������ʹ��tga��Ϊ��ͼ������Ŀǰ����ֻ֧�ֵ����ֻ�Ϸ�ʽ��
	 *  ����blend_mode��ֵ����Ϊ3.
	 *  River @ 2008-5-27:����srcalpha�ͱ�����ӵĹ���,
	 *          ��D3DRS_SRCBLEND Ϊsrcalpha, D3DRS_DESTBLENDΪ one
	 *          ʹ��blend_modeֵΪ5
	 */
	int             blend_mode;

	//! �Ƿ��Z���壬������ȾһЩ��Ҫ�۵���ͼƬ
	BOOL            m_bUseZBuf;

public:
	// ��ʼ��blend_mode Ϊ3,����uvΪ��0��1.
	os_screenPicDisp();
};

//! ���ڸ�������ڶ�����������ݽṹ
struct os_chaSecEffect;

/** \brief 
 *  ���ڳ�ʼ��һ��Camera�����ݽṹ.
 *
 *  ʹ������ṹ����ʼ��һ�����.��ʼ��һ�������,
 *  �ڳ������й����п���ʹ���������Ľӿڸ������
 *  ������ڲ�����.
 *
 */
struct OSENGINE_API os_cameraInit
{
	//! ��ǰ�����λ��.
	osVec3D      curPos;

	//! ����Ľ���
	osVec3D      camFocus;

	//@{
	/**
	 *  ��������ProjMatrix������.
	 */
	float        nearp;
	float        farp;
	float        fov;
	float        faspect;
	//@}

	//@{
	/**
	 *  ��������ViewPort������.
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
 *  ʹ��DirectInput�õ���mouse data.ʹ���Լ��ĸ�ʽ.
 *
 */
struct  OSENGINE_API os_dInputdata
{
	//! �൱��dinput�е�:DIMOUSESTATE2�ṹ.
	struct os_DImousedata
	{
		long   lx;
		long   ly;
		long   lz;
		BYTE   rgbButtons[8];
	};
    
	//! ��Dinput�еõ���Mouse����.
	os_DImousedata   mousedata;

	//! ��Dinput�еõ��ļ�������.
	char             keydata[256];
};



//! ����������ɫ�����õ��Ĺؼ��֡�


/** \brief
 *  ����Ļ����Ⱦ��������ɫ���������ִ���
 *
 *  ��HTMLһ�������Ը��ݽ������Ĳ�ͬ����ʾ����ͬ���ִ���ɫ��
 */
class OSENGINE_API os_colorInterpretStr
{
private:
	//@{ River mod @ 2006-2-27:���ڳ����ִ��Ĵ���
	//! �̶��Ļ�����
	char           m_szStaticSzBuf[256];
	s_string       m_szDynamicBuf;
	//@}

	// syq *2 @ 2005.2.24
	const char*    m_szCIStr;

public:

	//@{
	//! �ִ�����Ļ�ϵ���ʼλ�ú�����ά�е�Zֵ��
	int            m_iPosX,m_iPosY;
	float          m_fZ;
	//@}
	//! �ַ��ĸ߶�.Ӣ���ַ�����Ǹ߶ȵ�һ�룬�����ַ��߶ȺͿ��һ����
	int             charHeight;

	/** \brief
	 *  ÿһ�е����ܴﵽ�����X����ֵ��
	 *
	 *  Ĭ������£������ݵ�ֵΪ-1,��ʾ����Ҫ���С�
	 */
	int            m_iMaxPixelX;

	//! Y�����ϵ������������ֵ��
	int            m_iMaxPixelY;

	//! �����Ҫ���У��м�صĴ�С����ʵ����Ļ�е�������Ŀ�ơ�
	int            m_iRowDis;

	/** \brief
	 *  ����ֵ�����ϲ�õ���Ϣ�����ִ��ڸ����Ŀ����������ʾ�˶����С�
	 *
	 *  ���ǿ����ʾ��ʱ����ֵ��������Щ�ڿ�ܾ����ڲ���ʾ���С�
	 */
	int            m_iResRowNum;

	/** \brief
	 *  �Ƿ�ǿ����ʾ���ִ��������ǿ����ʾ�����Ҫ��ʾ���������ǵ�����
	 *  �������������Ǹ�����������ֻ��ʾ���ڿ�����ʾ���Ĳ��֡�
	 *
	 *  ���򣬲���Ⱦ��ǰ���ִ������ء�
	 */
	BOOL           m_bForceDis;


public:
	os_colorInterpretStr();
	os_colorInterpretStr& operator = ( const char* _dstr );
	//! �õ��ִ�����
	const char*           get_ciStr( void ) { return m_szCIStr;}
};


/** \brief
 *  ��3d��������ʾ������Ҫ���Ľṹ.
 *
 *  ��������3dԪ����Ⱦһ��,��ʾ����Ҳ��Ҫ������,��������Ҫ���һ��
 *  ��ʾ���ֵĽṹ.����ʹ��Unicode.
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


	//! ��Ⱦ��ǰ������ʱ,ʹ�õ�zֵ,Ĭ��Ϊ0.0f������Ⱦ���ǰ�档
	float           z;

	//! �ַ��ĸ߶�.Ӣ���ַ�����Ǹ߶ȵ�һ�룬�����ַ��߶ȺͿ��һ����
	int             charHeight;

	/** \brief
	 *  ���ֱ�����Ӧ����ɫֵ��
	 *
	 *  ��ʽΪ:0xffffffff,����ǰ����ff���뵱ǰ������ɫ��͸����
	 *  ��������ʹ�ð�͸���ĺ�ɫ��������ɫֵӦΪ��0x88ff0000
	 *  
	 */
	DWORD           m_dwBackColor;


public:
	//! ����������ʾ�ִ���Ҫ������.
	os_stringDisp();

	//! Ϊ��ǰ�Ľṹ��һ��string��ֵ.
	os_stringDisp& operator = ( const char* _dstr );

	//! �õ��ڲ����ִ���
	const char*    get_sdStr( void ) { return dstring; } 

};

typedef std::vector<os_stringDisp> VEC_stringDisp;



/** \brief
 *  �õ�������Ϣ�İ�����.
 *  
 *  ʹ�������õ�ĳ�������ǲ��Ǳ�����.
 */
#define OS_KEYDOWN(name, key)            (name[key] & 0x80)



////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      ��ʼ����ϵͳ�����Ĺ����������,����*.BFS�ļ�
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
*  ������������
*/ 
struct OSENGINE_API os_boneIdx
{
	int   m_iParentIdx;
	int   m_iBoneIdx;
};

typedef std::vector<os_boneIdx>  VEC_boneIdx;

/** \brief
*  ��Ź�����Ϣ����ʱ�ṹ��
*
*  *.bfs�ļ��ĵ�����Ϣ��Žṹ�����ڴ����Ϊ�����е�Frame����
*  ���ӡ�
*  
*/
struct OSENGINE_API os_boneSys
{
	//! ��ǰ����ϵͳ�����֡���
	s_string    m_strBoneSysName;

	//! River added @ 2010-12-21:����hashֵ�ıȽ�.
	DWORD       m_dwHash;

	//! ��ǰ����ϵͳ�й�������Ŀ��
	int         m_iBoneNum;

	//! ���������б���Ҫ���ڵ��ԡ�
	std::vector<const char*>   m_vecBoneNameStr;

	//! ������id�б�
	VEC_boneIdx  m_vecBoneIdx;
public:

	//! ���ļ��е���һ������ϵͳ��
	bool        load_boneSysFromFile( const char* _fname );

	//! ��һ������ID�õ�����������.
	const char* get_boneNameFromId( int _id );

	//! ��һ������ID�õ��˹������ǵ�����.
	const char* get_parentBoneNameFromId( int _id );
};

//! Bonesys' stl vector.
typedef std::vector<os_boneSys>  VEC_boneSys;
/** \brief
*  ��ǰϵͳ�й���ϵͳ�Ĺܵ���
*
*  �ڵ���һ���µ����ﶯ��ʱ��ͨ�����������������ϵͳ��
*  �������ڴ����Ѿ����Ĺ���ϵͳ�Ͳ���Ҫ�ڴ�Ӳ���ϵ����ˡ�
*
*  ATTENTION: ��Ϊ�������õ��Ĺ���ϵͳ�����޵ģ����Կ��ܻ�
*             ���������еĹ���ϵͳ�ŵ�һ���ʼ������������Ϸ 
*             ���й����У��Ͳ���Ҫ��Ӳ���ϵ������ϵͳ�ļ��ˡ�
*/
class OSENGINE_API osc_boneSysMgr
{
private:
	//! ����ϵͳ������.
	VEC_boneSys       m_vecBoneSys;

	//! ��ǰϵͳ�й���ϵͳ����Ŀ��
	int              m_iBoneSysNum;

public:
	osc_boneSysMgr();
	~osc_boneSysMgr();

	/** \brief
	*  �ӹ���ϵͳ�������е���һ�׹�����
	*/
	os_boneSys*   create_boneSysFromMgr( const char* _fname );

	/** \brief
	*  �ͷŵ�ǰ�Ĺ�����������
	*/
	void         release_boneSysMgr( void );

};

//! d3d_present �Ĳ���
struct OSENGINE_API os_present
{
	//! Ŀ�괰��
	HWND			m_destWnd;

	//! Դ����
	const RECT*		m_pScrRect;

	//! Ŀ�����
	const RECT*		m_pDestRect;

	os_present(void):m_destWnd(NULL),m_pScrRect(NULL),m_pDestRect(NULL){}
};

//===================================================================================
//
/** \brief
 *  3d Deivce�������Ľӿ�.
 * 
 *  3d Deivce�Ĺ������ӿ�,�ⲿ�������Щ�ӿ�,���Գ�ʼ�����ͷ�3d Device,����Device���в���.
 *  �������б���Ҫ������ӿڽ��в����ſ��Խ���3D��ʾģʽ. 
 */
//
//===================================================================================
struct OSENGINE_API I_deviceManager
{
	/**
	 *  Device init&release.���ļ��г�ʼ�����Ǵӽṹ�г�ʼ��һ��d3d.
	 *  
	 *  \param _hwnd            ����Ĵ��ھ��.
	 *  \param _initdata        ������ṹ�еõ�3dDevice�ĳ�ʼ������.
	 *  \param _cfm_ptr         Confirm Device�Ļص�����,����ԭ�ͼ�
	 *                          �����֮���ĺ���ָ������:   CFMPTR.   
	 *  \return bool            ��������ɹ�,����true.
	 */
	virtual bool         init_device( HWND _hwnd,const os_deviceinit* _initdata,
		                              void* _cfm_ptr ) = 0;

	//! ��������d3dDevice�Ŀ�ȣ��߶�,�����ϲ������Ϸ�ķֱ���
	virtual bool         reset_device( int _width,int _height,bool _windowed = true ) = 0;

	/* \brie3f
	 * ���������swapChain,���ڶര����Ⱦ
	 *
	 * ���ϲ�����촰�ڴ�С�ı��ʱ�򣬵�������ӿڣ��ڲ��Ѿ�������Ӧ�Ĵ���ֱ�Ӵ����µ�
	 * swapChain���ͷ�ԭ����swapchain.
	 *
	 * ��� _hwnd == NULL �������ͷŽ�������Դ
	 */
	virtual bool         create_addSwapChain( int _width,int _height,HWND _hwnd ) = 0;


	/**
	 * �ͷ�3d�豸�ĺ���.
	 */
	virtual void         release_device( void ) = 0;
    

	/** \brief 
	 *  ʹ��d3d��Ӳ�����.
	 *
	 *  ������������������ϢWM_SETCURSOR��Ϣ�е���,�����������ʹ��d3d��Ӳ�����.
	 *
	 */
	virtual void         set_hwCursor( void ) = 0;
	//! ��ӦWM_MOUSEMOVE��Ϣ����Ӳ�������д���
	virtual void         set_cursorPos() = 0;


	/** 
	 *  ����ȫ��Ļ��Gammaֵ.
	 *  \param _gama Ҫ�����Gamma����,��os_gamma.
	 */
	virtual void         set_gamma( const os_gamma* _gama ) = 0;

	//@{
    //uper layer should not control these func??????�����Ҫ�ٹ���.
	/**
	 *  Render control functions.���е���Ⱦ����beginDraw����,
	 *  ������Present�����(Present�а�����enddraw );
	 */
	virtual void         d3d_begindraw( void ) = 0;
	virtual void         d3d_enddraw( void ) = 0;

	//! river added @ 2010-3-1:���ڶര����Ⱦ
	virtual void         d3d_addSwapClear( DWORD Count,
                                     CONST D3DRECT* pRects,
                                     DWORD Flags,
                                     D3DCOLOR Color ) = 0;

	//!�������Ƿ��Ѿ�׼������
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
	//! ��ʼ�ͽ���¼�Ƶ�Ӱ��
	virtual bool         begin_recordMovie( const char* _aviName ) = 0;
	virtual void         end_recordMovie( void ) = 0;
	//@}
# endif 

	/** \brief
	 *  �õ���ǰ�豸���е������Դ�Ĵ���������
	 *
	 *  ��mΪ��λ��
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
	 *  ȫ��Ļ�ĵ��뵭����Ч.
	 *
	 *  \param DWORD _scolor,_ecolor  ���뵭��ʱ��ȫ��Ļ��ʼ��ɫ�ͽ�����ɫ,
	 *                                ��Ҫʹ����ɫ��alphaͨ��.
	 *  \param float _fadetime        ��������ʱ�Ķ���ʱ��.
	 */
	virtual void         start_fullscrFade( 
		                     DWORD _scolor,DWORD _ecolor,float _fadetime ) = 0;

	/** \brief
	 *  �ϲ�õ�fade�İٷֱ�;
	 *  
	 *   ���ڿͻ��˴���һЩ�¼������Ҫfadeout����fadeIn��ʱ�򣬲�����Ч��������Ϣ�ȡ�
	 */
	virtual float        get_fullscrFadePercent( void ) = 0;



    /**********************************************************************/
	//
	// Use following func to display string of Engine.
	//
	/**********************************************************************/


	//@{
	/** \brief
	 *  ���õ�����Ϸ��������ʱ������ı������档 
	 * 
	 *  ��������ʹ��ͳһ��ͼƬ���ʹ��640*480����ʵͼƬ��С��������ͼƬ
	 *  �зֳ�����256*256��ͼ������128*256��ͼ��
	 *  ����ʹ��һ��256*64�ĵ����
	 *
	 *  \param _bgName ������ǽ���Ŀ¼�����֣������Ŀ¼�´����˱���ͼƬ�����֡�
	 *  \param _x,_y,_info ���ñ���ʱ����Щ���������ڱ�����ĳ��λ������һ����ʾ��Ϣ��
	 *  \param _infoColor   ���ñ�������ʾ��Ϣʱ����ʾ��Ϣ���ִ���ɫ
	 */
	virtual bool         set_backGround( const char* _bgName,int _x =0,
		                     int _y = 0,const char* _info = NULL,
							   DWORD _infoColor = 0xffffffff ) = 0;

	/** \brief
	 *  ���÷������չ��
	 *
	 *  ���ֵΪ1.0,�����չ���϶����������չ����
	 *  \param _addps �����������ԭ�������ϼ���Ľ��ȣ��տ�ʼʱ����Ϊ0,�����ֵ
	 *                ����ϵͳ�ڲ��Ľ��ȴ���1.0,��ϵͳʹ��1.0�Ľ��ȡ�
	 *  \param _x,_y  ����ȴ��������������ʾ��Ϣ����Ϊ��ʾ��Ϣ��λ�á�
	 *  \param _info  ������ʾ��Ϣ,Ϊ������ʾ��Ϣ��
	 *  \param _infoColor ��ʾ��Ϣ�ִ�����ɫ.
	 */
	virtual void         set_progress( float _addps,int _x,
		                  int _y,const char* _info = NULL,DWORD _infoColor = 0xffffffff ) = 0;
	//@}


	/** \brief
	 *  �Ե�ǰ����Ļ���н�ͼ.
	 *  ʹ�����ڼӵ�ǰ��ʱ����Ϊ��ͼ������.
	 */
	virtual char*         catch_screen( void ) = 0;

	//! �õ���˻�����������.
	virtual const D3DSURFACE_DESC*  get_backBufDesc( void ) = 0;

	//! �õ�������.
	virtual bool get_mainRenderTarget( LPDIRECT3DSURFACE9& _target ) = 0;

	//! �õ��Կ�����Ϣ
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
 *  ��DirectInput�İ�װ��.
 * 
 *  ����Ⱦ������ȫ�����һ������,���ϲ�����������ϵ�һ��.
 *  Input�Ĺ��������� mouse&keyboard.
 */
 //===================================================================================
struct OSENGINE_API I_inputMgr
{
	/** \brief
	 *  InputManager�ĳ�ʼ������.
	 *
	 *  \param _hInst  �������InputMgr�����ʵ��.
	 *  \return bool   ��������ɹ�,������.
	 */
	virtual bool               init_inputmgr( HINSTANCE _hInst,HWND _hWnd ) = 0;
	/** \brief 
	 *  �ͷ����Input Manager.
	 */
	virtual bool               release_inputmgr( void ) = 0;

	/** \brief
	 *  �ӵ�ǰ��Input Manager�õ����ݵĺ���.
	 *
	 */
	virtual os_dInputdata*     get_inputdata( void ) = 0;

	//@{
	/**
	 *  �ڳ���һ��ʼ�����ǰ���ALT+TAB����ʱ����Ҫ��������������.
	 *  �����DirectInput���ĵ�.
	 */
	virtual bool               acquire( void ) = 0;
	virtual bool               unacquire( void ) = 0;
	//@}

};


//! �ϲ�ע������Ͽ�����Ҫ���õĺ���
typedef int (*sockClosed_callbackFunc)( void* _para );


//===================================================================================
/** \brief
 *  Ϊ�ϲ��ṩ����ӿڡ�
 *
 *  ����ӿ��ṩ������������繦�ܡ�
 */
//===================================================================================
struct OSENGINE_API I_socketMgr
{

	/** \brief
	 *  ���ӵ��������Ľӿڡ�
	 *
	 *  \param   _hostAddr   Ŀ��������ĵ�ַ������"202.196.168.0"
	 *  \param   _port       Ŀ��������Ķ˿ڡ�
	 *  \param   _WSA        �������¼�����ʱ��������õ�����Ϣ��
	 *  \param   _hwnd       ������Ĵ��ھ����
	 */
	virtual bool         connect_server( const char* _hostAddr,int _port,int _WSA,HWND _hwnd ) = 0;
	/** \brief
	 *  �رյ�ǰ�����ӡ�
	 */
	virtual bool         close_socket( void ) = 0;

	//! ��ǰ��Socket�Ƿ�ر�
	virtual bool         is_socketClosed( void ) = 0;

	//@{
	/** \brief
	 *  �������ݣ���������ָ�롣
	 *
	 *  \param _msgSize    �������Ϣ���ݵĳ��ȡ�
	 *  \param _errorCode  ������������������صĴ������͡�
	 *  \return char*      ���ص�����ָ�롣
	 */
	virtual char *        read_message( WORD& _msgSize,int* _errorCode,int* _errorType ) = 0;
	//@}

	//��ȡ��Ϣ������������NULL��Ϊ��ȡ���
	virtual char* read_buffer( WORD& _msgSize ) = 0;

	//@{
	//  River @ 2005-10-6: ɾ����������������Ϣ�Ľӿڣ��ͻ���û���õ��˽ӿڡ�
	//! ���͵�����ָ���õĺ�����
	virtual bool          send_oneMsg( const char* _msg,WORD _size ) = 0;
	//@}

	//! ����Ϊ�ѻ�ģʽ����ģʽ�£�send_oneMsg��Զ����true.
	virtual bool          set_offLineState( bool _offLine ) = 0;

	//! ע������رպ���õĿͻ��˻ص����� 
	virtual bool          register_socketCloseCallback( sockClosed_callbackFunc _func ) = 0;

	/*! ��÷������ݵ��ܳ���
	*	
	*	\param _dwDataLenHight			: DWORD���ȵĸ�λ
	*	\param _llDataLen				: �ܵĳ���
	*	\return DWORD					: DWORD���ȵĵ�λ
	*/
	virtual DWORD		  get_sendDataLen(DWORD* _dwDataLenHight = NULL)const = 0;

	/*! ��ý������ݵ��ܳ���
	*	
	*	\param _dwDataLenHight			: DWORD���ȵĸ�λ
	*	\param _llDataLen				: �ܵĳ���
	*	\return DWORD					: DWORD���ȵĵ�λ
	*/
	virtual DWORD		  get_recvDataLen(DWORD* _dwDataLenHight = NULL)const = 0;

	/*! \brief �����㷨��Key���ɷ��������ȷ��͹�����
	*
	*	\param _key						: key value
	*	\param _len						: key length;
	*/
	virtual void		  set_cryptKey(char* _key,int _len) = 0;
};



//! ��������в���ʱ��������صĲ��������
enum ose_camOperateRS
{
	//! ��������ڶ����У����ܽ��е�ǰ������
	OSE_CAMIN_ANIMATION = 0,

	//! �������ѹ�Ĺ��͡�
	OSE_CAMPITCH_TOO_LOW,
	//! ��������Դﵽ��ֱ��������
	OSE_CAMPITCH_TOO_HIGH,

	OSE_CAMOPE_SUCCESS,
};

//===================================================================================
/** \brief 
 *  Ϊ�ϲ��ṩ�ɲ�����Camera�Ľӿ�.
 *
 *  ���Camera�ṩ���ϲ���Բ����Ľӿ�,����ʵ������ӿڵ�������Ҳ��һЩ�ӿ�����
 *  �������ڲ���.��Щ���������ڲ��Ľӿڲ���Ҫ�������ϲ�ʹ��.
 *
 */
//===================================================================================
struct OSENGINE_API I_camera
{
	/**********************************************************************/
	//
	//  ��ʼ��Camera�ĺ���.
	//
	/**********************************************************************/
	/** \brief 
	 *  ��һ���ṹ�г�ʼ��һ��Camera.
	 */
	virtual void         init_camera( const os_cameraInit* _cdata )   = 0;

	//! �õ�����ɿ�����Զ������ľ��룬�������ɼ�����Զ����
	virtual float        get_farClipPlaneDis( void ) = 0;
	virtual float		 get_nearClipPlaneDis( void ) = 0;

	
	//! ���λ������������������
	virtual void          set_maxDistance( float _maxDis ) = 0;

	//! ���λ��������������С����
	virtual void          set_minDistance( float _minDis ) = 0;


	/**********************************************************************/
	//
	//  ����Camera��Ҫ�Ľӿ�.
	//
	/**********************************************************************/
	/** \brief
	 *  ����һ��Camera��Projection Matrix
	 *
	 *  \param _nearp,_farp   ��Զ������ľ��롣
	 *  \param _fovh          ���fov��ֵ��field of view.
	 *  \param _faspect       ��Ļ��(��/��)��ֵ��
	 */
	virtual void         set_projmat( float _nearp,
		                    float _farp,float _fovh,float _faspect ) = 0;


	/** \brief
	 *  ����Զ������.
	 *  \param _farp   ��ֵ��СΪ128.0f,���Ϊ280.0f
	 */
	virtual void         set_farClipPlane( float _farp ) = 0;


	/** \brief
	 *  ����һ��Camera��ViewPort.
	 *
	 *  ��ͬ��Camera������Ҫ��ͬ��ViewPort,���ܳ��ֶ��Camera���
	 *   ViewPort���龰(���л�).
	 *
	 *   _tlx,_tly        Viewport����Ļ���ϽǵĶ���.
	 *
	 *   _wid,_hei        Viewport�Ŀ�͸ߡ�
	 *
	 *   _min_z,_max_z    Viewport��������СZֵ.
	 */
	virtual void         set_viewport( int _tlx,int _tly,
		                    int _wid,int _hei,float _min_z,float _max_z ) = 0;

	/** \brief
	 *  ��������camear��aspect.
	 *
	 *  \param _aspectAdd ��ǰ�����aspect����������ݡ��ڲ���_aspectAdd����Ϊ
	 *                    �ı���aspect���ݣ�������.
	 */
	virtual void         set_camAspect( float& _aspectAdd ) = 0;


	//! ���������fovֵ��
	virtual void         set_cameraFov( float _fov ) = 0;
	//! �õ����е�����ӽǿ��.
	virtual float        get_cameraFov( void ) = 0;


	/** \brief
	 *  �õ������ǰ��ָ��ķ���,����������ں���ģ��,�����˷����
	 *  ����.
	 */
	virtual void         get_godLookVec( osVec3D& _vec ) = 0;

	/** \brief
	 *  �����ǰ���ƶ�.������ֵ���������λ���뽹���Զ��
	 */
	virtual void         move_close( float _length ) = 0;

	/** \brief
	 *  ��������λ�õ�����ľ���
	 */
	virtual float        get_disFromCamposToFocus() = 0;


	/** \brief
	 *  �õ���ǰ��������ҵķ������.
	 */
	virtual void         get_rightvec( osVec3D& _vec ) = 0;

	/** \brief
	 *  ���������ǰ��λ��.
	 *
	 *   \param bool _force �����ڲ�ʹ�ò�����
	 */
	virtual void         set_curpos( osVec3D* _vec,bool _force = false )  = 0;

	//! 2009-9-4:һ��������λ�ú�focus,���ڽ��һ���ܱ�̬����Ļ�ζ���bug.
	virtual void         set_curPosFocus( osVec3D& _pos,osVec3D& _focus,bool _force = false ) = 0;


	/** \brief
	 *  �õ������ǰ��λ��.
	 */
	virtual void			 get_curpos( osVec3D* _vec )  = 0;
	virtual const osVec3D&	 get_curpos( void) = 0;

	/** \brief
	 *  ����Ľ������
	 */
	virtual void         set_camFocus( const osVec3D& _focus ) = 0;
	virtual void         get_camFocus( osVec3D& _focus ) const = 0;
	virtual const osVec3D& get_camFocus( void) const = 0;


	/** \brief
	 *  ʹ�����һ���������ˮƽ��ת��
	 *��
	 *  \param bool _force ǿ���Ե���ת,�����ڲ�ʹ�á�
	 */
	virtual ose_camOperateRS  rotate_horizon( const osVec3D& _ver,
		                   float _radians,bool _force = false )  = 0;

	/** \brief
	 *  ʹ�����һ���������������ת��
	 *
	 *  \param bool _force ǿ���Ե���ת,�����ڲ�ʹ�á�
     *  \return 
	 */
	virtual ose_camOperateRS  rotate_vertical( const osVec3D& _ver,
		                   float _radians,bool _force = false ) = 0;




	/**********************************************************************/
	//
	//  ʹ������е����ݽ���һЩ�������ļ���.
	//
	/**********************************************************************/
	//@{
	/** \brief
	 *  �õ���Ļ��һ����ά�����������ά�ռ��е�����.
	 */
	virtual osVec3D      get_upprojvec( const osVec2D* _svec,osMatrix& _wmat ) = 0; 
	virtual osVec3D      get_upprojvec( int _x,int _y,osVec3D& _dir ) = 0;
	//@} 


	/** \brief
	 *  ����һ������ռ�Ķ���,������ǰ����ռ䶥���Ӧ����Ļ�ռ�Ķ���.
	 *
	 *  \param _wpos ���������ռ�Ķ�������,������Ϊ��������.
	 *  \param _spos ����������ռ䶥���Ӧ����Ļ����.
	 *  
	 */
	virtual bool          get_scrCoodFromVec3( const osVec3D& _wpos,osVec3D& _spos ) = 0;


	//! �ӳ����е���camera,����ʾ�����λ�ú���ת��������ݣ����ӿ��޹ء�
	virtual bool          load_camera( const char* _cname,
		                       bool _ani = false,float _time = 0.0f ) = 0;

	/** \brief
	 *  ���������ת��Pitch��������
	 *  
	 *  ��ͨ��;�������ر���ײʱ�����������͵ĵ�����λ��
	 *  ��ת��pitch����������Ľ���������С�
	 */
	virtual bool          animate_camRotPicth( 
		                   float _rotRad,float _pitchRad,float _time ) = 0;


	virtual void  set_autoResetState( bool _autoReset ) = 0;

	/** \brief
	 *  ����һ�������Ƿ��������������
	 */
	virtual bool          is_ptInCamVF( osVec3D& _pos ) = 0;


# if __EFFECT_EDITOR__
	//! �洢�����е�cameraλ�����ݡ�
	virtual bool          save_camera( const char* _cname ) = 0;
# endif 

	//syq 2005.4.3 ��������˶�����Ч
	virtual void play_camEffect( char * _camEffectName, int _nCntPerSec ) = 0;


	/** \brief
	 *  ���Ƹ�����Ϊһ�̶�ֵ
	 *
	 *  \param _rad �̶��ĸ����Ƕ�ֵ(����),�����ֵС�ڵ����㣬��ʹ�õ�ǰ
	 *              �����pitchֵ��Ϊ������pitchֵ
	 */
	virtual void   limit_pitch( float _rad ) = 0;

	/** \brief
	 *  ���Ƹ�����Ϊһ�̶�����ֵ
	 *
	 *  \param _rad     ��ֱ������ӽ�.
	 *  \param _radMin  ��ƽ������ӽ�.
	 *  \param _minFDis ����뽹����Դﵽ��������룬�ڴ�λ���ϣ����ʹ����ƽ���ӽǡ�
	 *  \param _maxFDis ����뽹����Դﵽ����Զ���룬�ڴ�λ���ϣ����ʹ����ֱ���ӽǡ�
	 */
	virtual void   limit_pitch( float _rad,float _radMin,float _minFDis,float _maxFDis ) = 0;


	/** \brief
	*  ���Ƹ�����Ϊһ�̶�ֵ ,֮ǰ��һ��ֵ
	*/
	virtual void   limit_pitch( void ) = 0;

	/** \brief
	*  ȡ�����Ƹ�����Ϊһ�̶�ֵ
	*
	*  \param _rad �̶��ĸ����Ƕ�ֵ(����)
	*  
	*/
	virtual void unlimit_pitch() = 0;

	/** \brief
	 *  ʹ����񶯵Ľӿ�
	 *  
	 *  \param float _fDamp		���������.
	 *  \param float _swing      ��������.
	 */
	virtual void shake_camera( float _fDamp,float _swing ) = 0;

	//! �õ����viewFrustum��8������,����˸�����ֱ���: xyz,Xyz,xYz,XYz,xyZ,XyZ,xYZ,XYZ
	virtual const osVec3D* get_vfPt( void ) = 0;

	//! ����autoResetState.
	virtual void  save_autoResetState( void ) = 0;
	//! �ָ�autoResetState.
	virtual void  restore_autoResetState( void ) = 0;


};




//===================================================================================
//! ��������������ӵ�е�mesh��Ŀ��
# define MAX_SKINPART      8

/** \brief
 *  �߸����岿λ����ʾЧ��,�ɲ�ͬ���ļ�����������Щ��ʾЧ����
 *   
 *  ���й�����������Ʒ������ʹ���߸����岿λ��
 *  
 */ 
struct OSENGINE_API os_bodyLook
{
	//! mesh��Ŀ
	int      m_iPartNum;     
	
	//! ���岿λ�Ͷ����Ƥ����������Ĭ�ϴ�1��ʼ,����0��ʾ�˲�λΪ�գ�
	//! ʲô������Ҫ��ʾ��
	WORD     m_mesh[MAX_SKINPART];
	WORD     m_skin[MAX_SKINPART];

};

/** \brief
*  �任װ����Ҫ�����ݽṹ��
*/
struct OSENGINE_API os_equipment
{
	//! ��Ҫ���ٸ����ָ�����ʾ�����Դ��
	int           m_iCPartNum;

	//! ��Ҫ�滻�Ĳ�λ��id.��λid��0��ʼ��
	int           m_arrId[MAX_SKINPART];
	//! ��Ҫ�滻��meshId.meshId��1��ʼ.
	int           m_arrMesh[MAX_SKINPART];
	//! ��Ҫ�滻��skinId.SkinId��1��ʼ��
	int           m_arrSkin[MAX_SKINPART];
};

/** \brief
 *  ��ʼ��һ��skinMesh��Ʒʱʹ�õĽṹ��
 */
class OSENGINE_API os_skinMeshInit
{
public:

	//! Ҫ��ʼ����skinMesh�������ݵ�Ŀ¼��
	char          m_strSMDir[32];


	//! Ҫ��ʼ������Ʒ����ʾ��Ϣ��
	os_bodyLook    m_smObjLook;

	//! �������object��λ�á�
	osVec3D        m_vec3Pos;

	//! ��ǰskinMesh���ܵĶ���������
	int            m_iTotalActTypeNum;

	//! Ĭ��Ҫ�����skin mesh Object�Ķ���id.
	//! ���id���㿪ʼ.
	int            m_iActIdx;

	//! �������ʹ�ö�������,�򲥷�Ĭ�ϵĶ�������.
	char           m_szDefaultAct[32];


	//! ��ǰ�������ת�Ƕȡ�
	float          m_fAngle;

	//@{
	// �������ֵ��
	//! �������ת���������
	float          m_fAdjRotAgl;
	//! ����λ��΢����
	osVec3D        m_vec3AdjPos;

	//! ����BoundingBox�Ĵ�С��
	osVec3D        m_vec3Max;
	osVec3D        m_vec3Min;
	//@ 


	//! ��ǰ�����е�͸�������Ƿ���Ҫд��ZBuffer.
	bool           m_bWriteZbuf;

	/** \brief
	 *  ��ǰ�Ķ����Ƿ�֧��ʵʱ��Ӱ.
	 *
	 *  ��Ϸ��ֻ�������������ʹ�ô���, ��Ӱʹ�õļ�����̫��
	 *  ��ֵĬ��Ϊfalse,��֧����Ӱ��
	 */
	bool           m_bShadowSupport;

	//! ƽ����ӰͶ��
# if GALAXY_VERSION
	bool           m_bSupportProjectShadow;
# endif 

	// River added @ 2005-6-5: ���Գ�����͸����Ʒû���������Ч����
	/** \brief
	 *  �Ƿ�ʹ�ó����еķ������Ϊ��ǰskinMesh��ȫ�ֹ⡣
	 *
	 *  ������Щȫ����͸�����skinMesh��Ʒ��ʹ��Ʒ����ʾ�����棬������ʹ�ó���
	 *  �������Ĺ���Ϊ�յ������ϵĹ⡣
	 * 
	 *  Ĭ���������Ϊ�١�
	 */
	bool           m_bUseDirLAsAmbient;
	//@{
	// Windy mod @ 2005-8-30 19:06:00
	//Desc: ride ֻ���������ֵ��Ϊ��
	bool			m_bIsRide;
	//!���������ϵ������΢��λ�á�
	osVec3D			m_vec3RideOffset;

	//@}

	//! ʹ�ö����ͼ�Ĳ�λ���������㿪ʼ��һ��characterֻ����һ�����岿λʹ�ö��������ͼ
	int             m_iMlFlagIdx;


	//! �������ϰ���Ч���ֺ�ƫ�ƣ�windy add 7.25
	//s_string		m_speFileName;
	char			m_szspeFileName[64];
	osVec3D			m_spePostion;
	//! 2007-1-29 winyd add ������������ֵ
	float			m_Scale;

	//! ������������Ƿ񲻿ɼ�
	bool            m_bInitFade;

	//  
	//  River added @ 2009-12-30:
	//! �Ƿ������Ҫ��Ⱦ��Ϊ������ʹ�õ�������︽�ӵ���Чʹ�ô�ֵ��
	bool            m_bUseInInterface;

	//! ��ʼ������������Ч��
	bool            m_bPlayWeaponEffect;

	//! Ĭ�ϲ�������
	os_skinMeshInit();

	//! ����
	void load_equipment(const os_equipment* _equ);
};






/** \brief
 *  �滻�¶����ķ�ʽ��
 */
enum  ose_ActionChangeType
{
	//! �滻�ɵĶ�����
	OSE_DIS_OLDACT = 0,

	//! �����������µĶ���,������ɺ��������ԭ���Ķ�����
	OSE_DIS_NEWACT_TIMES = 1,

	
	/** \brief
	 *  �����������µĶ�����������ɶ�����ʹ���¶�����
	 *  ���һ֡��ʾskinMesh.��ʾһ��ʱ�����ʧ��
	 *
	 */
	OSE_DIS_NEWACT_LASTFRAME = 2,


};

/** \brief
 *  �����¶����Ľṹ��
 */
class OSENGINE_API os_newAction
{
public:

	//! Ҫ���ŵ��¶������������¶�����������0 ��ʼ.
	int                   m_iNewAct;

	/** \brief
	 *  Ҫ���ŵ��¶��������֡�
	 *
	 *  ����ʹ�ö���������ȥ���Ŷ������������Ҫ���Ŷ�������������Ϊ-1.
	 *
	 *  ����Ҳ������Բ��ŵĶ�������ʹ�õ����׶�����ΪĬ�϶���.
	 *  
	 */
	char                  m_strActName[32];

	/** \brief
	 *  Ҫ������ŵĶ�������
	 *  
	 *  ֻ��m_changeTypeΪOSE_DIS_OLDACTʱ���˱����������á�
	 *  ���������Ĳ���m_strActName��������������ֵĶ�����
	 *  ���������ĸ�������������š�
	 *  �����Ҫ�������"���ֹ���1"��"���ֹ���2"�����m_strActName��ֵΪ"���ֹ���1",
	 *  ��m_strRandActName[0]��ֵΪ"���ֹ���2"�����������ֲ��ܳ���24���ַ�
	 *  
	 *  os_newAction�Ľ����������m_strRandActName[0][0],m_strRandActName[1][0],
	 *  m_strRandActName[2][0]��ֵΪNULL.
	 */
	char                  m_strRandActName[3][24];

	/** \brief
	 *  ��󲥷ŵĶ���,�����ԵĲ�����ȥ
	 *
	 *  �����ֵ��Ϊ��,�򰴴��������궯����,�����������,һֱ������ȥ.
	 *  
	 */
	char                  m_strNextActName[32];


	/** \brief
	 *  �������ķ�ʽ��
	 */
	ose_ActionChangeType   m_changeType;


	/** \brief
	 *  ֻ�е�m_changeType��ֵΪOSE_DIS_NEWACT_TIMES��
	 *  OSE_DIS_NEWACT_LASTFRAMEʱ�����ֵ����Ч��
	 *  
	 *  ��ʾ�����¶����Ĵ���.
	 */
	int                   m_iPlayNum;

	/** \brief
	 *  �����OSE_DIS_NEWACT_TIMES���ŷ�ʽ��������ɺ�,�ǲ�����һ������,
	 *  ���ǲ���ԭ������ѭ�����ŵĶ���.
	 */
	bool                  m_bPlayLoopAct;

	//! �¶����Ĳ����ٶ�,���ʹ��ϵͳĬ���ٶ�,������������ͺ���.
	//! ��ֵΪ�������¶�����Ҫ��ʱ��.
	float                 m_fFrameTime;

	//! �Ƿ����ض���ֹͣ֡��ֹͣ�����ļ�������,Ĭ������²�ֹͣ��
	bool                  m_bStopAtStopFrame;

public:
	os_newAction();

};


/** \brief
 *  ������Ӧ����Ч�ļ�����
 *  
 *  River @ 2009-12-9:һ��������Ч���Զ�Ӧ�����������Ч��һ
 *  ÿһ�����࣬����Ϊ�ա��൱����Ч��Ͽ���ÿһ������һ�ࡣ
 *  ������ЧԪ�أ�����������ϣ�����ȫ�����ӣ�ȫ��osa,ȫ�ǹ����
 *  ������������ЧԪ�ص�������ϡ�
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
	//! ��ֵ����
	I_weaponEffect& operator=( const I_weaponEffect& _b )
	{
		strcpy( m_szEffObjName[0],_b.m_szEffObjName[0] );
		strcpy( m_szEffObjName[1],_b.m_szEffObjName[1] );
		strcpy( m_szEffObjName[2],_b.m_szEffObjName[2] );

		return *this;
	}

	// River added @ 2010-8-11:��ǰweapon��Ч�Ƿ����.
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
 *  �ϲ����skinMesh����Ľӿڡ�
 *
 *  �ڴ�����ʱ���ʼ��.
 *  
 */
struct OSENGINE_API I_skinMeshObj
{

	//! ���ڱ�ʶ�ڼ�����,WEAPON7������������WEAPON8����������
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

	//! River added @ 2009-6-7:��ǰ��skinMesh�ӿڲ����Ƿ�ɲ���.���̵߳���skinMesh
	virtual bool             legal_operatePtr( void ) = 0;

	//! River added @ 2009-8-26: ��������ļ�ʱ��Ӱ.
	virtual bool             hide_realShadow( bool _hide = true ) = 0;


	/** \brief
	 *  �ı䵱ǰskinMeshObject�Ķ���.
	 *
	 *  \param  _act     Ҫ�����¶�����Ҫ���Ľṹ��  
	 *  \return float    ����Ҫ���ŵ��¶����Ĳ��Ź�����Ҫ��ʱ�䣬����Ϊ��λ��
	 */
	virtual float            change_skinMeshAction( os_newAction&  _act ) = 0;

	//! ʹֹͣ״̬�Ķ����������š�
	virtual void             start_moveAction( void ) = 0;


	/** \brief
	 *  ���ﶯ���Ƿ񵽴ﵱǰ�����Ĺؼ�֡
	 *
	 *  ����ؼ�֮֡ǰ����ֵΪ�٣�����֮�󷵻�ֵΪ�档
	 *  
	 */
	virtual bool             arrive_stopFrameOrNot( void ) = 0;
	//! �õ��Ӷ�����ʼ������ֹͣ֡���õ�ʱ�䡣
	virtual float            get_timeFromStartToSF( void ) = 0;



	/** \brief
	 *  �滻skinMesh��װ����.
	 */
	virtual bool            change_equipment(const  os_equipment& _equi ) = 0;

	/** \brief
	 *  �ͷŵ�ǰ��skinMesh��ռ�õ���Դ��
	 *  
	 *  �ͷź�ǰ�Ľӿڲ��ܹ�����.
	 * 
	 *  \param _sceneC �ڲ�ʹ�õĲ������ⲿ���ò��迼��
	 */
	virtual void            release_character( BOOL _sceneC = false ) = 0;


	/** \brief
	 *  �õ���ǰskinMesh������ռ��е�λ�ú���ת�Ƕ�.
	 * 
	 *  \param _pos    ����skinMesh������ռ��λ��.
	 *  \param _fangle ����skinMesh������ռ����ת�Ƕ�.
	 */
	virtual void            get_posAndAgl( osVec3D& _pos,float& _fangle ) = 0;

	/** \brief
	 *  �õ��ֲ��ľ�����Ҫ������Ϸ�е���Ч����.
	 *
	 *  ʹ�ô˴��õ��ֲ������������ϲ���Ҫ����Ч��λ�á�
	 *  ֻ�������˵������ݵ�����ģ�Ͳſ��Եõ��ֲ��ľ���
	 *  
	 */
	virtual void            get_handPos( osVec3D& _pos ) = 0;

	//! ���Եõ�ĳһ��������λ��.
	virtual void            get_bonePos( const char* _boneName,osVec3D& _pos ) = 0;

	//! �õ���ǰskinMesh����ת����
	virtual float           get_smAgl( void ) = 0;

	//! �ϲ������Чʱ�õ�����ת�����������Ⱦ�õ�����������
	virtual float           get_renderSmAgl( void ) = 0;


	/** \brief
	 *  ���µ�ǰ��skinmeshObj��λ�á�
	 *
	 *  �˺����������޸�skinMesh�����λ�ú���ת�Ƕ�.
	 *  �����ڲ���skinObjFramemove������skinMesh��������
	 *  
	 *  \return float ���ص�ǰ����ĵ�ǰ�������ڡ����ڲ��ŵĵ�ǰ�������̡��е�ʱ��ٷֱȡ�
	 *                ���統ǰ���ڲ�����·�����������·������������֮һ���򷵻�0.333333
	 */
	virtual float           frame_move( osVec3D& _npos,float _nangle ) = 0;

	//! �õ���ǰ������������ɵı���,���������������40%���򷵻�0.4
	virtual float           get_actionPlayPercent( void )  = 0;
	//! �õ���ǰ���ڲ��ŵĶ������ֺ�����
	virtual int             get_curPlayActIdx( char _name[32] ) const= 0;

	//! �õ�ĳ�������¼���İٷֱȣ������ܴ���Ч��Ҫ���ܴ������Ĳ��Ŷ���ʱ���š�
	virtual float           get_actionTriggerPoint( char _name[32],int _idx = -1 ) = 0;


	/** \brief
	 *  ��ǰ�����ܹ��ж����׶������Բ��š�
	 */
	virtual int             get_totalActNum( void ) = 0;

	//! ����fade״̬��
	virtual void            set_fadeState( bool _fadeS,float _alphaV = 1.0f ) = 0;

	/** \brief
	 *  �Ӷ��������ֵõ����ŵ�ǰ������Ҫ��ʱ�䡣
	 *  
	 *  \param _actName ����Ҫ����Ķ������֡�
	 *  \param _actIdx  ���ص�ǰ�������ֶ�Ӧ�Ķ�����������Ҫ���ڵ���༭����
	 *               
	 */
	virtual float           get_actUsedTime( const char* _actName,int& _actIdx ) = 0;

	/** \brief
	*  �Ӷ��������ֵõ����ŵ�ǰ������Ҫ��ʱ�䡣
	*/
	virtual float           get_actUsedTime( const char* _actName ) = 0;

	/** \brief
	 *  �Ӷ�����id�õ����ŵ�ǰ�Ķ�����Ҫ��ʱ��.
	 */
	virtual float           get_actUsedTime( int _id ) = 0;

	/** \brief
	 *  ���ص�ǰskinMesh����ʾ��
	 *
	 */
	virtual void            set_hideSkinMesh( bool _hide ) = 0;

	//! is hide state?
	virtual bool			is_hideState(void)const = 0;


	//! ���ص�ǰ�������ϰ󶨵���Ч
	virtual void            hide_followEffect( bool _hide ) = 0;


	/** \brief
	 *  �ӵ�ǰ��character�õ�һ��boundingBox.
	 */
	virtual os_bbox*        get_smBBox( void ) = 0;

	/** \brief
	*  �ӵ�ǰ��character�õ�һ��boundingBox.
	*/
	virtual const os_bbox*        get_smBBox( void )const = 0;

	/** \brief
	 *  ��ɫ����ʹ�õ���.
	 *
	 *  \param bool _glossyEnable �Ƿ�򿪵�����ʾ��
	 *  \param bool _weaponEffEnable �Ƿ��������Ч����ʾ
	 */
	virtual void            playSwordGlossy( bool _glossyEnable,bool _weaponEffEnable = true ) = 0;

	//! Set VIP Load�����̵߳��룬��ǰskinObj���ڶ��е���ǰ�棬������õ�char���ȼ������.
	virtual void            set_vipMTLoad( void ) = 0;


	/** \brief
	 *  �л������������Ч
	 *
	 *  /param int _idx           �л�����_idx����Ч�����������Ļ�����ʹ�������Ч��
	 *  /param WEAPON_ID _weapid  ʹ�õ��߻��ǵڰ˸�������λ��
	 */
	virtual void            change_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;

	//! �����������õ���ǰweapon��Ч���ļ�����
	virtual const I_weaponEffect*  get_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;


	//! �õ���ǰ֡��������ĵ㡣
	virtual void            get_swordGlossyCenter( osVec3D& _pos ) = 0;

	/** \brief
	 *  ����һ��������ֵ��ϵͳ�͵�ǰ�ĳ���������ֵ���Ƚϣ�ʹ�ø����Ĺ�������ǰ��ɫ��
	 *
	 *  ��Ϊ��ʹ�������Ĺ���ȥ������ǰ��skinMesh������Ҫ����ʹ��ԭ���ĳ���
	 *  ��������������Ļ���ֻҪ�Ѵ����osColor��r,g,b,a������Ϊ0.0f�Ϳ����ˡ�
	 */
	virtual void            set_maxBright( osColor& _bcolor ) = 0;

	/** \brief
	 *  ����Ļһ����ά��������Ⱦ��ǰ������,�����Ⱦʹ��OrthogonalͶӰ����
	 *
	 *  \param _rect  Ҫ���ĸ���Ļ��������Ⱦ���
	 *  \param _zoom  ���ڿ������������Զ��������,�ڲ��������Χ�е����ĵ����osVec3D( 0.0f,0.0f,0.0f)��
	 *                λ���ϣ��������osVec3D( 0.0f,0.0f,-_zoom)��λ���ϡ��ϲ���Ե�����һ�����ʵ�ֵ����
	 *                ʵʱ����Ⱦ��һֱʹ�����ֵ��
	 *  \param float _rotAgl ���ڶ�ά�������������ת����
	 *  \param _newChar �Ƿ����´��������������´���������ӿڣ����������Ķ�������ά�����ڵ�����û��
	 *                  �κεĹ�ϵ
	 *	\param _camera ������������Ч���Ͳ�����ȫ�ֵ����ָ�뾪����Ⱦ
	 */
	virtual void            render_charInScrRect( RECT& _rect,float _rotAgl,
		                       float _zoom,BOOL _newChar = FALSE ,I_camera* _camera = NULL) = 0;

	//@{ windy add 2006-04-13
	virtual void			get_Default_ActName(std::string& _ActName) = 0;
	virtual void			get_Act_List(std::vector<std::string>& _ActNameList) = 0;
	//@}

	/** \brief
	 *  ��ͣ��ǰ����Ķ�����
	 *
	 *  \param bool _suspend ���Ϊtrue,����ͣ���������Ϊfalse,�����������
	 */
	virtual void            suspend_curAction( bool _suspend ) = 0;


# if __GLOSSY_EDITOR__

	/** brief
	 *  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
	 *
	 *  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
	 *  \param int _trinum �������ʾĳ����λȫ��������������(����������Ҳ���㿪ʼ)�������ֵΪ�㼴�ɡ�
	 *  \return int        ����_trinumΪ���ʱ�򣬷�����������岿λ������������Ŀ��
	 */
	virtual int             displaytri_bysel( int _paridx,int _triidx[24],int _trinum ) = 0;

	//! �������ﵶ����β�ĳ��ȡ���Ч������6-12��������ڻ���С�ڴ�ֵ��ʹ��Ĭ�ϵ���β����
	virtual void            set_sgMoveLength( int _len ) = 0;

	// �õ�������β�ĳ�������
	virtual int             get_sgMoveLength( void ) = 0;


	//  ���º���ֻ�ڱ༭����ı༭����ʹ�á�
	/** \brief
	 *  �ѵ�ǰsm�е������ļ����뵽�µ��ļ��С�
	 */
	virtual bool            save_weaponFile( WEAPON_ID _weapid = WEAPON7) = 0;

	/** \brief
	 *  ���õ�N��������Ч�ļ������4����
	 * 
	 *  \param int _idx ���õڼ���������Ч�����㿪ʼ�����ֵΪ3.����3�Ļ����ڲ����Ϊ��
	 *  \param char* _effName ������Ч���ļ���
	 *  \param WEAPON_ID _weapid ���õ��߸����ǵڰ˸�������λ����Ч��I_weaponEffect
	 */
	virtual bool            set_weaponEffect( int _idx,
		                      const I_weaponEffect& _effName,WEAPON_ID _weapid = WEAPON7 ) = 0; 

	//! ������ǰ�������е����λ�á�
	virtual bool            adjust_swordGlossySize( bool _start,float _size , WEAPON_ID _weapid = WEAPON7) = 0;

	//! ��X��������ת���⡣
	virtual void            rot_swordGlossyX( float _radian , WEAPON_ID _weapid = WEAPON7) = 0;
	//! ��Y��������ת���⡣
	virtual void            rot_swordGlossyY( float _radian , WEAPON_ID _weapid = WEAPON7) = 0;

	//! ��X�����Y�����ƶ������λ�á�
	virtual void            move_swordGlossy( bool _x,float _dis , WEAPON_ID _weapid = WEAPON7) = 0;

	//! ���ô��������ڶ�λ������ʾ��������
	virtual void            set_displayWeapon( bool _dis , WEAPON_ID _weapid = WEAPON7) = 0;    

	//! ���赶��ĸ�����ת�Ƕȣ�ʹ��ȫ�ص�����ǰ�ĵ���
	virtual void            reset_swordGlossyPosRot( WEAPON_ID _weapid = WEAPON7 ) = 0;

	/** \brief
	 *  ���õ����Ӧ�Ķ�������, ��������ÿһ��������Ӧ����ɫ���ݡ�
	 *
	 *  \param _actIdx    ��ʾ��һ��������Ҫ��ʾ���⡣
	 *  \param const char* _glossyTex �����˵���ʹ����������֣����������������
	 *                                ��������Ŀ¼�µ�efftextureĿ¼�£�����
	 *                                "efftexture/glossy.hon" �����������Ӧ����"glossy"
	 *  \param _color     ÿһ�����������Ӧ����ɫ,�����ɫ�����������ɫ����󵶹���ʾ
	 *                    ��������ɫ.�����Ҫ��ȫ����ʾ���������ɫ,�����������ɫֵΪ
	 *                     ����ɫ,��0xffffffff����ֵ.
	 */
	virtual void            set_swordGlossyActIdx( int _actNum,int* _actIdx,
		                          DWORD* _color,const char* _glossyTex , WEAPON_ID _weapid = WEAPON7) = 0;

	/** \brief
	 *  �õ�����ĵ�ǰ������Ӧ�������ݡ�
	 *  
	 *  �����λ����Ϣ����Ҫ΢����
	 *  \param int& _actNum �����ж��ٸ�����ʹ���˵���.
	 *  \param int* _actIdx ����һ���������飬����������ʹ������12Ԫ�أ�������ʹ�õ����
	 *                      �Ķ���������
	 *  \param char* _glossyTex �ϲ㴫����ִ���������һ��64�ֽڵ�char�����飬�����˵�ǰ
	 *                          ����ʹ�õĵ����������֡�
	 *  
	 */
	virtual void            get_swordGlossyActIdx( int& _actNum,int* _actIdx,
		                            DWORD* _color,char* _glossyTex , WEAPON_ID _weapid = WEAPON7) = 0;

	/** \brief
	 *  ����skinMesh��ǰ�������ӵĵ�����Ч��
	 *  
	 *  \param _pos      ��0.0f��1.0f����ֵ����ʾ�ӵ��⿪ʼ�͵������λ��֮���λ�á�
	 *                   particle,billboard�����λ����Ϊ��Ч���õ�λ��.osa��Ч�����
	 *                   λ����Ϊosa��max��(0,0,0)���Ӧ��λ��.
	 *               
	 *  \param I_weaponEffect& _effFName ��Ӧ����Ч���ݽṹ��ֻ����һ������
	 *
	 *  ע��:Ŀǰ֧�ֵ�����������Ч��,osa�ļ������������,�����������ļ�����������
	 *       ��"��"����ʽ����,��ֻ�ܸ���������ĳ��"��"֮��,���������޹ص�.
	 *       osa�ļ��������ĸ�������"��"����ʽ�����,����������osa�ļ���max��������ʱ��,
	 *       ��max�г��ϵķ���,��Z��Ϊ��׼����,��(0,0,0)��Ϊԭ��.�������ϸ��ӵĴ���Ч��
	 *       �Ѵ�osa�ı�׼�����������ķ����λ�ñ���һ��.
	 *
	 *  River @ 2009-12-9: _effFname�޸ĳɽṹ����ʽ��������ͬһ��������Ӧ�����ЧԪ�ء�
	 *                     ���_effFname.m_szEffObjName[0][0] == NULL,�����������Ӧ��Ч��
	 */
	virtual void            set_sgEffect( float _pos,
		                       const I_weaponEffect& _effFName, WEAPON_ID _weapid = WEAPON7) = 0;

	//! river @ 2010-3-5:����effectPos.
	virtual void            set_sgEffectPos( float _pos,int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;

	/** \brief
	 *  �õ�����������Ч��ص���Ϣ.
	 *
	 *  \param I_weaponEffect& _effFName  �ⲿ���룬�����ڲ������Ӧ������,��һ��������Ч����
	 */
	virtual void            get_sgEffect( float& _pos,
		                            I_weaponEffect& _effFName, WEAPON_ID _weapid = WEAPON7) = 0;

	//! �õ����ӵ�addPos.
	virtual void            get_sgEffectPos( float& _pos,int _idx,WEAPON_ID _weapid = WEAPON7 ) = 0;


	/** \brief
	 *  ����������������תosa��Ч,ʹ��osa�ļ��������Ķ�λ���Ӿ�ȷ.
	 *
	 *  �����ǰ������Ӧ����Ч���Ͳ���osa,��˺���û��Ч��.
	 *  Ŀǰ��Ҫ���ڵ���osa�ļ�,ʹ֮��״����������״��ƥ��.
	 */
	virtual void            rotate_osaEffect( float _agl , WEAPON_ID _weapid = WEAPON7) = 0;


	/** \brief
	 *  ���������ϵ���ЧЧ��
	 *
	 *  \param int _idx ÿһ��������Ч����������С��Ч��ÿһ������Ӧ�����š�
	 *  \param _scale �����1.0���򱣳�ԭ��Ч��С���䣬2.0������Ϊԭ���Ķ���.0.2����СΪԭ����5��֮1
	 *                �����������ű���С���㣬���ڲ��ı������䣬�����ڲ���ǰ��Ч�����������
	 *  \return float �����˵�ǰ��Ч�����ű�����
	 */
	virtual float           scale_weaponEffect( int _idx,float _scale,WEAPON_ID _weapin = WEAPON7 ) = 0;

	//@{
	// ����Ķ�����װ���༭��ء�
	

	/** \brief 
	 *  ��ʾ�������һ������֡���ơ�
	 * 
	 *  ����Ķ������봦��suspend״̬����������������á�
	 *
	 *  \param bool _next ���Ϊtrue,��ʹ����֡�����ﶯ�������Ϊfalse,��ʹ����һ֡�����ơ�
	 */
	virtual void            next_actFramePose( bool _next ) = 0;

	/** \brief
	 *  �õ�ĳһ�������ܹ��ж���֡��
	 *
	 *  \param _actName Ĭ�������ʹ�ö����������õ�֡����Frame��Ŀ��
	 *  \param _actIdx  �����ֵ���ڵ��㣬�򷵻���Ӧ������������Frame��Ŀ
	 */
	virtual int             get_actFrameNum( const char* _actName,int _actIdx = -1 ) = 0;


	//! �õ���ǰ�������ڲ��Ŷ�����֡������
	virtual int             get_curActFrameIdx( void ) = 0;

	

	//! �ĵ����skinMesh�Ĺ���ϵͳ
	virtual os_boneSys* 			get_boneSys(void) = 0;

	/** \brief
	 *  �õ����岿λ�������������ǰ���岿λ�����ڣ�����0
	 *
	 *  \param int _idx ���岿λ���������㿪ʼ��
	 */
	virtual int             get_bodyTriNum( int _idx ) = 0;
	
	//@}

	
	


# endif 


# if __CHARACTER_EFFEDITOR__
	
	/** \brief
	 *  ��������ĳһ��λ��������ͼЧ����
	 *
	 *  ������岿λԭ��û�еڶ������������Ϊʹ�õڶ�������
	 * 
	 *  \param _seceff ��������ֵΪ�գ����ԭ�������岿λʹ����Ч�������Ϊ��ʹ����Ч��
	 *  \param _idx    Ҫ�����Ĳ�λ��id.���岿λ��������0��ʼ��Ŀǰ��Ϊ7����λ��
	 */
	virtual void           update_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx ) = 0;

	/** \brief
	 *  �õ���ǰ���岿λ��chaSecEffect���ݡ�
	 *
	 *  \param _idx    Ҫ�õ������岿λid,��λid������0��ʼ��
	 *  
	 */
	virtual bool           get_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx ) = 0;


	/** \brief
	 *  �洢�����_idx��λ�ĵڶ���������ͼЧ����
	 *
	 *   ������岿λskinû����Ч����Ŀ¼���ִ��ڴ˲�λskin����Ч�ļ�����ɾ������Ч�ļ���
	 *   \param _idx   Ҫ�����Ĳ�λ��id.���岿λ��������0��ʼ��Ŀǰ��Ϊ7����λ��
	 *   \return bool  ������ļ����󣬻���Ҫ�洢��λû�еڶ�����ͼ��Ч������false��
	 */
	virtual bool           save_chaSecEffect( int _idx ) = 0;

# endif 



	/** \brief
	 *  ������������š�
	 *
	 *  \param float _scale ����������������Ҫ���ŵ�ֵ��������Ĭ��������1.0f
	 */
	virtual bool           scale_skinMesh( osVec3D& _svec ) = 0;

	//@{
	// Windy mod @ 2005-8-30 19:04:10
	//Desc: ride �õ��ڶ����֣�����λģ�ͺе�λ��
	
	/** \brief
	* ��������󶨺�����ｫ������һ���ƶ���
	* �ϲ�Ҫ������Ϊ����������ơ���������Ӧ�Ķ���
	*/
	virtual void			SetRide(I_skinMeshObj * _rideobj) = 0;
	//!ȡ����������ͷ��κ���Դ
	virtual void			DelRide() = 0 ;

	//! Ϊ��login��������������Ľӿڡ�
	virtual bool            is_rideObj( void ) = 0;
	virtual I_skinMeshObj*  get_humanObjPtr( void  ) = 0;

	//! �õ�����󶨵������λ������.������ﻹû�а������������false.
	virtual bool            GetRidePos( osVec3D& _vec3 ) = 0;

	
# if __RIDE_EDITOR__
	//!������ģ�ͱ༭�Ľӿ�
	//!�õ����һ���ֵ�λ��
	virtual	void			GetLastPartPos( osVec3D & _v3d )	= 0;
	//!�������һ���ֵ�λ��
	virtual void			SetLastPartPos( osVec3D & _v3d)		= 0;
	//!�������һ���ֵ�λ�õ��ļ���
	virtual void			SaveLastPart()						= 0;
# endif

	//@}

	//! �л����ĵ���ͼ
	virtual bool            change_mlFlagTexture( const char* _tex1,
		                        const char* _tex2,const char* _tex3 ) = 0;

	//  
	// River @ 2009-6-23: ����������Ⱦ�����Ч��
	virtual void            set_fakeGlow( bool _glow,DWORD _color ) = 0;

};


//===================================================================================





//===================================================================================
/** \brief 
 *  �����е���Դ������,�����������������е���Դ.
 *
 *  ��Դ�ɷ�Ϊ�豸�����Դ���豸�޹���Դ,�������������,
 *  �ϲ�Ϳ���͸����ʹ��Ӧ�ô�������Դ��,������Ҫ������
 *  �豸�����Դ�����豸�޹���Դ.
 *
 */
//===================================================================================
struct OSENGINE_API I_resource
{
	/**  \brief
	 *   ����һ��Camera.
	 *
	 *   �õ�I_cameraָ���,���ǿ��Բ������camera.
	 *   ����Ҫ�ͷ����camera,�������Ҫcamera,���԰���ǰ������
	 *   camera���³�ʼ����ʹ��.
	 */
	virtual I_camera*    create_camera( void ) = 0;

	//! ɾ��һ�����
	virtual void         delete_camera( I_camera* _cam ) = 0;



	// Todo: ���������Ľӿ�.


};



/** \brief
 *  ����FrameMove�õ��Ľṹ��
 *  
 */
struct OSENGINE_API os_sceneFM
{
	//! �����õ���camera.
	I_camera*     m_ptrCam;

	//@{ 
	//! �����еĲ������ĵ㣬Ŀǰд������������ײ��������
	WORD         m_iCx,m_iCy;
	//@} 

	/** \brief
	 *  �������ﵱǰ��λ�ã�
	 *  
	 *  �����һ�𣬼������ǰ�����е�ס�����������Ʒ,ɫ���͸����Щ��Ʒ��
	 */
	osVec3D      m_vecChrPos;

};




/** \brief
 *  ���������õ��������ṹ��
 *  
 *  Ϊ���������������������ܳ���64.
 *  
 *  ����Ϊһ�����ݽṹʹ��,���ܲ�ֻ����tileIdxһ����;.
 */
struct OSENGINE_API os_tileIdx
{
	WORD    m_iX;
	WORD    m_iY;

	os_tileIdx(void){};
	os_tileIdx(WORD _x,WORD _y):m_iX(_x),m_iY(_y){}
};

/** \brief
 *  �õ��û������Ļ����õ��ĵ�ͼ�е�����.
 *
 *  �ȸ�����ͼ������,Ȼ�������ͼ�ϸ��ӵ�����(��Ϊ��ײ���,��ͼ����Ϊ128*128)
 */
struct OSENGINE_API os_pickTileIdx
{
	//! TileGround������.
	os_tileIdx    m_tgIdx;

	//! TileGround����ײ���ӵ�����,127Ϊ���ֵ.
	os_tileIdx    m_colTileIdx;


	//! ���ص�ǰ���ӵĸ߶�ֵ��
	float        m_fTileHeight;

};


// ����ṹ��ÿһ��tg����128*128��,ռ�ÿռ�ܴ�,����ʹ��1���ֽڶ���
# pragma pack(push,r1,1) 
/** \brief
 *  ���ӵ���ײ��Ϣ��
 *
 *  ʹ�������������֯������������ײ�����Ϣ��
 *  
 */
struct OSENGINE_API os_TileCollisionInfo
{

	//! ��������ܷ�ͨ���� 0Ϊͨ����1Ϊ��ͨ��
	bool     m_bHinder;

	//! ������ӵĸ߶�ֵ,����߶�ֵ�ڴ洢��ʱ��������߶ȼӵ�ͼ�ر�߶�ֵ.
	//float    m_fHeight;
};
# pragma pack(pop,r1)



/** \brief
 *  �����������м���Ķ�̬������͡�
 */
enum ose_dLightType
{
	//! Ŀǰ��֧��point���͵Ķ�̬�⡣
	OSE_DLIGHT_POINT,
};


/** \brief
 *  ����һ����̬����Ҫ���Ľṹ��
 */
struct OSENGINE_API os_dlightStruct
{

	//! ��̬�����ɫ��
	osColor   m_lightColor;

	//! ��̬���λ�á�
	osVec3D   m_vec3Pos;

	//! ��̬���Ӱ�췶Χ��
	float     m_fRadius;

	//! ���յ����͡�
	ose_dLightType  m_ltype;

};



/** \brief
 *  ����mesh�Ĳ������͡�
 *
 *  ���ڶ����Ĳ��ţ���ʱ�䲥�ţ���ѭ��֡�����ţ���Զ���š�
 */
enum ose_amPlayType
{
	//! ��ʱ�䲥�Ŷ���.
	OSE_PLAYWITHTIME = 0,
	//! ��֡�����Ŷ���.ÿһѭ������Ϊһ֡.
	OSE_PLAYWITHFRAME ,
	//! ������Զ������ȥ.
	OSE_PLAYFOREVER,
};

//! ��Ⱦ������м�ܵ�,�ϲ㲻��Ҫ�����ֵ.
class     osc_middlePipe;

/** \brief
 *  ����һ��aniMeshInstance��Ҫ�������ݽṹ��
 */
class OSENGINE_API os_aniMeshCreate
{
public:
	//! Ҫ������mesh��Ӧ���ļ�����
	/*
	s_string        m_szAMName;
	*/
	//@{ ����new��delete�ĵ��á�
	char           m_szAMName[64];
	//@} 

	//! ��ǰaniMesh����ά�ռ��е�λ�á�
	osVec3D         m_vec3Pos;

	//! ��ǰaniMesh�Ĳ��ŷ�ʽ��
	ose_amPlayType   m_ePlayType;

	//! ����ǰ�ʱ�䲥�ţ����ŵ�ʱ�䡣
	float           m_fPlayTime;

	//! ����ǰ��β��ţ����ŵĴ�����
	int             m_iPlayFrames;

	//! ȫ�ֹ����ݡ�
	DWORD           m_dwAmbientL;

	//! �Ƿ���ȫ�ֹ��Ӱ��,�����ڵ�osa�ļ��ܻ�����Ӱ��,��Ч��osa�ļ�����.
	bool            m_bEffectByAmbiL;

	//! ���������ǰ�ʱ�䲥����ɺ��Ƿ��Զ�ɾ��.
	bool            m_bAutoDelete;


	/** \brief
	 *  �Ƿ񴴽�ʵ��.
	 *  
	 *  ���������ʵ��,���AniMeshΪ��������õ�
	 *  ��AniMesh,һֱ�������ڴ���,Ϊ��һ�ε����mesh��ʡʱ��,
	 *  ֱ����Ϸ������AniMesh���ͷ�.
	 */
	bool            m_bCreateIns;

	//! ����ʱ���õ�ǰosaMesh������ֵ
	float           m_fSizeScale;
	//! ��ǰaniMesh����ת����
	float           m_fAngle;

	//! River mod @ 2008-12_10: �ڲ�ʹ�õı�����ˮ�����Ʒ.
	bool            m_bWaterAni;

	//! �ڲ�ʹ�õı������������Ʒ.
	bool            m_bFogAni;

public:
	//! ����һЩֵ�ĳ�ʼ����
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
 *  ����һ������ϵͳ��Ҫ���Ľṹ.
 */
class OSENGINE_API os_particleCreate
{
public:
	//! ����ϵͳ��Ӧ���ļ���.
	/*
	s_string        m_szParticleName;
	*/
	//@{ //! ����new��delete�ĵ��á�
	char           m_szParticleName[64];
	//@} 

	//! ����ϵͳ������ռ��е�λ��.
	osVec3D         m_vec3Offset;

	//! ����ϵͳ�Ƿ����������������ʱ�Զ�ɾ��.
	bool            m_bAutoDelete;

	//! �Ƿ񴴽����ӳ�����ʵ��,���������,�����������һ��
	//! �������������Чʱ,���Ը����ٵĵ���.Ĭ��Ϊ����ʵ��.
	bool            m_bCreateIns;

	//! ���Ӵ����������,������ڵ�����,Ĭ�ϴ�СΪ1.0,����С����
	float           m_fSizeScale;
public:
	os_particleCreate()
	{
		m_bAutoDelete = false;
		m_bCreateIns = true;
		m_szParticleName[0] = NULL;
		m_fSizeScale = 1.0f;
	}

	//! ���������Ĳ���??????

};




//! ����һ��ˮ���ļ������ݡ�
struct OSENGINE_API os_waterCreate
{
	//! ˮ���ļ���Ӧ����������
	char       m_szWTexName[128];

	//! ˮ���ļ���Ӧ������֡����
	int        m_iFrameNum;

	//! ˮ����ƶ��ٶȡ�
	float      m_fMoveSpeed;

	//! ˮ���Ӧ��x��z�����ϵĴ�С����
	int        m_iXSize;
	int        m_iZSize;

	//! ˮ���Ӧ�ķ���1:x 2:-y 3: -x 4:y
	int        m_iWaterDir;

	//! ˮ���alphaֵ.�������0.0f-1.0f֮�䡣
	float      m_fWAlpah;

};
struct OSENGINE_API os_snow
{
	float	mSnowHeight;//!< ѩ����ʼ�ĸ߶�,һ��Ϊ40.0f,���鲻�Ķ�
	float	mSnowLower;//!< ѩ�������ĸ߶�,һ��Ϊ-24.0f,���鲻�Ķ�
	float	mSnowSize;//!< ѩ����С 0.15f
	int		mSnowNr;//!< ѩ������  0 - 1000 ֻ��������
	osVec3D mVelocity;//!< ѩ�������ٶ� (0.0f, -20.0f, 0.0f);
	osVec3D mVelocityVariation;//!< ѩ��������Ŷ��ٶ� (5.0f, -10.0f, -4.0f);
};



//! ���������������Ŀ��
# define  MAX_RAINNUM 500

/** \brief
 *  ��������Ч���������ṹ��
 */
class OSENGINE_API os_rainStruct
{
public:
	//! ��Ч������ɫ
	DWORD      m_dwTopColor;
	DWORD      m_dwBottomColor;

	//! ��������ٶ�
	float      m_fRainMoveSpeed;

	//! ����������������Ŀ,����Ϊ50,���Ϊ500
	int        m_iRainDotNum;

	//! ���������������
	float      m_fXScale;
	//! �������Ĵ�С���š�
	float      m_fYScale;

public:
	os_rainStruct()
	{
		m_dwTopColor = 0x22afbfbf;
		m_dwBottomColor = 0x55afbfbf;

		// Ĭ�ϵ�ֵ���ⲿ���Ը������ֵ���������������ٶ�
		m_fRainMoveSpeed = 12.0;

		// ������ʹ��500�����
		m_iRainDotNum = MAX_RAINNUM/2;

		m_fXScale = 1.0f;
		m_fYScale = 1.0f;
	}
};

/** \brief
 *  �������е��������ݽṹ��
 */
struct OSENGINE_API os_weather
{
	//! �Ƿ�����
	BOOL        m_bRain;

	//@{
	//! ����ص����ݡ�
	osColor     m_sFogColor;
	float       m_fFogStart;
	float       m_fFogEnd;
	//@}

	//@{ 
	//! �����й���صĲ�����
	osColor     m_sAmbientColor;
	osColor     m_sDirLightColor;
	osVec3D     m_vec3DirLight;
	//@}

	// ����Ķ�̬�����ɫ
	osColor m_charDLightColor;

	//@{
	//! ����������.����������ŵ�miskEffTexĿ¼��
	//! ���������ݽṹʱ��ֻ�������������֣�����,"miskEffTex/sky.hon"����
	//! �ڴ˴����Ϊ"sky"�Ϳ����ˡ�
	//  River @ 2006-6-20: �˱���û�����壬���ڴ˴���Ϊ�����Կ���
	char        m_szSkyTex[128];

	float       m_fSkyMoveSpeed;

	

	//! ������յ�osa�ļ�.����ʹ��osa�ļ���ȫ·��,����: "keyAni\\sky1.osa"
	char        m_szOsaBkSky[128];




	//! Զɽ�����Զ����ʾ����ɫ��
	osColor     m_sFarTerrColor;
	//@{
	// Windy mod @ 2005-9-22 11:05:33
	//Desc: 
	bool		m_bIsLensFlare;
	char		m_szLensPath[128];
	osVec3D		m_vec3LensPos;
	//@}

	//! �ڶ���͵�������յ�osa�ļ���
	char        m_szOsaBkSkyEffect[2][128];
	char		m_szOsaBolt[128];
	float		m_iBoltRate;	//!< ���粥�Ÿ���(0~1)
	float		m_BoltAngle;	//!< ����Ƕȣ�����ֵ��0~2*PI��
	osColor		m_BoltColor;	//!< ������ɫ
	int			m_BoltType;		//!< ��������,0��ʾֻӰ�췽���,1��ʾֻӰ�컷����
	float		m_BoltHeight;	//!< ����߶�

	//! �ڶ���͵����������ת�ٶ�
	float		m_fSky1MoveSpeed;
	float		m_fSky2MoveSpeed;

	float		m_TerrFogDistance;
	float		m_SkyFogDistance;
	float		m_BoltTime;			//!< ����ʱ��
	//{@ windy add 7.10 ���ڶ�����������⴦��
	int			m_BoltFrequency;	//!< �Ȳ���,������������ټ�
	//@}
	//{@ windy add 7.11 �����ѩ���Ŀ��� 
	BOOL		m_bSnow;
	os_snow		m_SnowSetting;
	//@}
	float		m_fBoltFanAngle;

	// ������ص����ݽṹ
	os_rainStruct m_sRainStruct;

	//! ԶɽOSA�ļ�������Ϊ��
	char		m_szTerrain[128];

};





# define  MAX_MAPNAMELENGTH   8
/** \brief
 *  ���볡��ʱ��Ҫ�������ݽṹ��
 *
 *  �˴��ĵ��볡��������Ϸ��ʼʱ��ʼ������������Ϸ���еĹ����д��͵Ȳ���
 *  Ҳ��Ҫ���������ݽṹ�������볡����
 */
struct OSENGINE_API os_sceneLoadStruct
{
	//! ��ͼ�����֣�����vill,city�ȡ�
	char       m_szMapName[MAX_MAPNAMELENGTH];
	
	//@{
	/** \brief
	 *  ��ʼ�����������ռ��еľ������ꡣ
	 *
	 *  ���Ǹ���������������ҳ�����Ӧ�þ�̬�����4�ŵ�ͼ��
	 */
	int        m_iX;
	int        m_iY;
	//@}

	//!map ��ID�ţ������m_szMapName�����໥ת��
	DWORD	   m_dwMapID;

	//! ��ʼ������ʱ�����ص����ݡ�
	char       m_szSkyName[128];

};

//! ������ഴ������eleBolt����Ӧһ��Id.
# define MAX_ELEPERBOLT   2

//! ��ʼ��һ��bolt��Ҫ�����ݽṹ
class os_boltInit
{
public:
	//! boltҪϸ�ֵ���Ŀ��������2��n�η���8��16��32��
	int       m_iNumPt;

	//! bolt�Ŀ�ʼ�ͽ���λ�á�
	osVec3D    m_vec3Start;
	osVec3D    m_vec3End;

	//! bolt���˶��ٶȡ�
	float      m_fWildness;

	//! ÿ����ٴζ���bolt.
	int        m_iBoltFreq;

	//! Bolt�Ŀ�ȡ�
	float      m_fWidth;

	//! �绡����ɫ
	DWORD      m_dwColor;

public:
	os_boltInit()
	{
		m_iBoltFreq = 20;
	}
};
//@{
// Windy mod @ 2005-9-22 15:14:21
//Desc: �����Ǿ�ͷ��Ч�õ��Ľṹ��
typedef class _FlarePart {
public:
	float	m_fPos;		//���λ��0-1֮���ʾ̫������Ļ�е��λ��
	int		m_nIndex;	//ʹ�õ�����ID
	float	m_fSize;	//�ò��ִ�С
	D3DXCOLOR m_color;	//��ɫ
	float	m_fAlpha;	//ͼƬALPHA ��ֵ��0-1֮��
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


//! Ĭ��ÿһ֡��������ʱ��
# define DEFAULT_ANITIME 0.033f

/** \brief
 *  �ڳ����ڵ���ر�ʱ��Ϊ�������ֶ���Ч���ĳ�ʼ���ṹ
 *
 */
class OSENGINE_API os_sceneCursorInit
{
public:
	//! ������ڵ�λ��
	osVec3D     m_vec3CursorPos;

	/** \brief
	 *  ���ʹ��ƽ��Ķ�����꣬���ͼƬ������,��"passCursor",
	 *  �����ʹ��ƽ�涯����꣬��m_szCursorName[0] = NULL;
	 */
	char        m_szCursorName[64];
	//! ������ɫ
	DWORD       m_dwCursorColor;

	//! ��궯����֡��
	WORD        m_wXFrameNum;
	WORD        m_wYFrameNum;
	//! ÿ���೤ʱ���л�һ�ι�궯��֡
	float       m_fFrameTime;

	//! ������ʹ��osa�����Ļ���osa�ļ�������,ʹ��������·������"keyani\\cursor.osa"
	char        m_szOsaCursorName[64];
	//! ����osa�����Ĵ���
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
		// Ĭ��ÿ���л�30��
		m_fFrameTime = DEFAULT_ANITIME;
	}
};


//! ����һ��Decal�õ��Ľṹ
class os_decalInit
{
public:
	//! Decal��λ�úʹ�С,m_fSizeΪdecal�İ뾶
	osVec3D     m_vec3Pos;
	float       m_fSize;

	//! �õ���shader����
	char        m_szDecalTex[32];

	//! ��ת�Ƕȣ���Ҫ���ڲ���һЩ�����Ч���ͽ�ӡ���з����Ч��
	float       m_fRotAgl;

	//! decal��ʧ��ʱ��,���Ϊ�㣬����Զ����ʧ�������ϲ�ɾ��
	float       m_fVanishTime;

	//! decalʹ�õĶ���ɫ
	DWORD       m_dwDecalColor;

	//! Decalʹ�õ������Ƿ���������
	WORD        m_wXFrameNum;
	WORD        m_wYFrameNum;
	//! ����ж������л�һ������֡��ʱ��
	float       m_fFrameTime;

	//! �Ǹ�����ε�decal,ֻ��Ҫ����λ�ã�����Ҫ������ν����з֣����ڻغ���Ϸս�������ڵĵػ�.
	BOOL        m_bFakeDecal;

	//! ���ر�֮��ĸ߶�λ�ƣ�Ĭ��Ϊ��
	float       m_fYOffset;

	//! decal�������Ľ�Ϸ�ʽ��Ĭ����0: srcAlpha InvSrcAlph. 1: srcAlpha  one.
	int         m_iAlphaType;

public:
	os_decalInit()
	{
		m_fSize = 0.0f;
		m_szDecalTex[0] = NULL;
		m_fRotAgl = 0.0f;
		m_fVanishTime = 0.0f;
		m_dwDecalColor = 0xffffffff;
		// �������
		m_wXFrameNum = 1;
		m_wYFrameNum = 1;
		m_fFrameTime = DEFAULT_ANITIME;
		m_bFakeDecal = FALSE;

		m_iAlphaType = 0;

		// Ĭ��״̬�£�ϵͳ�����һ��С����λ��
		m_fYOffset = 0.0f;
	}
};


//  
//! �����ڵر����������
#pragma pack(push,1)
class os_terrainTexSnd 
{
public:

	// ����ID
	int  m_iMaterialID;
	// ����
	char m_szTexture[32];
	// ��Ч
	char m_szSound[32];
	// �Ƿ�����Ч
	char m_szEffect[32];
	union 
	{
		float mWaterHeight;
		// �Ƿ��н�ӡ
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



//! ÿһ����ײ����Ҫϸ�ֵĸ�����Ŀ,���������صĸ���ʹ�������λ�ĸ�������
# define DETAIL_TILENUM    15


//! �Ƿ�����޸ĵ�ͼ��ײ��Ϣ�͸߶���Ϣ�ӿڵĺ�.
# define __COLHEI_MODIFY__   1

//! �Ƿ����ʹ�ö����������������npc������ͳһ����Ľӿڡ�
# define __ALLCHR_SIZEADJ__  0


//! ����������Ⱦ��Ϣ�Ļص�����ԭ��
typedef int ( *UPDATE_LOADINGSTEPCALLBACKIMM)(LONG,LONG);




//===================================================================================
/** \brief
 *  ������֯��Ե��Ϸ�������ݵĽӿڡ�
 *
 *  ����ӿ���Ҫ������ɷ�������ʾ�����ָ��߼�����ĳ�������Ϸ���������������
 *  ����ɡ�
 *  Camera��Ҫ�ӷ��������г�ʼ����
 *
 *  ����Ϸ�����в���Ҫʵ�ʲ�����Ϸ�����е����ݣ�ֻ��Ҫ��������ָ����з�����������
 *  ���²��ִ�г���Ϳ����ˡ�
 *  
 *  I_fdScene���õ����������굥λ��
 *  ��һ������Ϊ��λ���ӿ��ڵĲ������������򴫳�����float���͵�ֵ����������Ϊ��λ��
 *  �ڶ�������ײ����Ϊ��λ���ӿ��д�����Ǵ��������������ͻ����޷����ͣ������Գ����ڵ�
 *        ��ײ����Ϊ��λ��
 *  
 */
//===================================================================================
struct OSENGINE_API I_fdScene
{

	//! �ص�����ע��
	virtual void register_updateLoadingStepCallbackFunc( UPDATE_LOADINGSTEPCALLBACKIMM _pFunc ) = 0;

	// 
	// HDR Light    River: Ҫɾ�����½ӿڣ�������
	virtual void init_hdrLight() = 0;
	virtual void set_hdrRenderTarget() = 0;
	virtual void set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure ) = 0;
	virtual void set_hdrFm( float _fm ) = 0;
	virtual void set_hdrDiffuse( float _diffuse ) = 0;
	virtual void set_hdrExposure( float _exposure ) = 0;


	/**********************************************************************/
	//
	//  �����ĳ�ʼ��.
	//
	/**********************************************************************/	
	/** \brief
	 *  ʹ�����ǵĳ�������ṹ���볡����ͼ��
	 *
	 */
	virtual bool          load_scene( os_sceneLoadStruct* _sl ) = 0;

# if __QTANG_VERSION__
	//! ���ϲ����õ�ͼ����ײ��Ϣ,��ʵ��Щ������ȫ�����ɿͻ�����ɣ�Ϊ������ǰ�Ľӿڡ�
	virtual bool          set_sceneCollInfo( char* _coll ) = 0;
# endif 

	virtual bool          isLoadMap() = 0;

	//! �˴����ص�size������Ϊ��λ��size��ÿһ����ʾ����ռ���ף�һ��
	//! ��ײ������1.5��
	virtual osVec2D       get_mapSize() = 0;

	/** \brief
	 *  �ͷ�һ������.
	 *
	 *  �ͷ�������������е�����,�����ͼ���͵Ȳ�����Ҫ�����������仯��Ҫ
	 *  ʹ�����������
	 *  
	 *  \param �Ƿ��ͷ�������ĻͼƬ��������ڽ����л�״̬ʱ�������һ����Ļ��
	 *         ����ǰ����ĻͼƬ�Ļ������ͷ���ĻͼƬ�����Խ�ʡ��һ����Ļ�ĵ���ʱ�䣬
	 *  
	 */
	virtual bool          release_scene( bool _rlseScrPic = true ) = 0;



	/** \brief
	 *  Ϊ�����ͼ�е�˲���������Ľӿڡ�
	 *
	 *  \param _mapName ˲�ƺ��ͼ�����֣�����ʹ�ú͵�ǰ��ͼͬ�������֡�
	 *  \param _xCord,_yCord ˲�ƺ����������,�ǳ����ڵ���ײ�������ꡣһ����ʾ������2*2��
	 *                       ��ײ����
	 */
	virtual bool          spacemove_process( const char* _mapName,int _xCord,int _yCord ) = 0;

	/** \brief
	 *  ������FrameMove.
	 */
	virtual bool         frame_move( os_sceneFM* _fm ) = 0;

	/** \brief
	 *  ��Ⱦ��ǰ�ĵ�ͼ��
	 *
	 *  \param _scrPic �����ǰ����Ϊ�棬��ֻ��Ⱦ��Ļ�ڵĶ�άͼƬ��������Ⱦ����
	 *                 �Ӷ�άͼƬ��
	 *  \param _LensPic ��ʾ����Ⱦ�Ķ�άͼƬ��LensFlare����Ⱦ״̬��
	 *  \param _rtype   ��Ⱦ������ģʽ��Ϊ�غ�����Ϸ������
	 *                  �˲���ΪOS_COMMONRENDER����ͨ����Ⱦ
	 *                  ����ΪOS_RENDER_LAYER,�ֲ���Ⱦ������Ⱦ����������Ⱦ��ս���������Ч
	 *                  
	 */
	virtual bool          render_scene( bool _scrPic = false,
		                    bool _LensPic = false,os_FightSceneRType _rtype = OS_COMMONRENDER ) = 0; 

	/** \brief
	 *  �õ���ǰ֡��Ⱦ����������Ŀ��
	 */
	virtual int           get_renderTriNum( void ) = 0;

	/** \brief
	 *  �õ���ǰ�����е�skinMesh�õ��������ε���Ŀ��
	 *
	 *  skinMesh��Ҫ�����������Ⱦ�����Դ˴��õ���ֵ��Ϊ�������Ƭ��Ŀ��
	 */
	virtual int           get_skinMeshTriNum( void ) = 0;

	/** \brief
	 *  �ı䳡����������
	 *
	 *  \param _weather  �µĳ����������ݽṹ��
	 *  \param _itime    �ı�������Ҫ�������ۺ�ʱ��,����Ϊ��λ.�����ֵ
     *                   С�ڵ����㣬��ֱ���޸�������
	 *  
	 */
	virtual void          change_sceneWeather( os_weather& _weather,float _itime ) = 0;

	/** \brief
	 *  �ı䳡����Զ����ʾosaģ��.
	 *  
	 *  \param _terr Զɽ��ģ���ļ���.
	 */
	virtual void          change_sceneFarTerr( const char* _terr ) = 0;

	/** \brief
	 *  �õ���ǰ��������ڸ��ӵ�������
	 *
	 *  �������ֵ�������Ŵ��ͼ�ϵĸ��ӵ�����ֵ�������ɷ�����ȷ�ϲ���Ч��
	 *  
	 *  ATTENTION : 
	 *
	 *  \param _dix   �����ڲ�������ɺ�������ͻ������ֵ.
	 *  \param _sidx  ���뵽�����ڲ����ڴ������Ļ����.
	 *  \param _osVec3D* _intersectPos ����ཻ��������ǽ����ʵ����ά����
	 */
	virtual bool          get_pickTileIdx( os_pickTileIdx& _dix,
		                    os_tileIdx& _sidx,osVec3D* _intersectPos = NULL ) = 0;


	/** \brief
	 *  ����һ����ͼ���꣬�õ���ǰ��ͼ�������ĵ��ڶ�Ӧ����ά�������ǰ���겻�ڳ����ڣ�
	 *  ���Ӧ����ά�����yֵΪ0.0f.
	 */
	virtual bool          get_tilePos( int _x,int _y,osVec3D& _pos,BOOL _precision = FALSE  ) = 0;
	//! ��ʾ������������λ�õĵر���ײ��Ϣ
	virtual void	get_collision_info(const osVec3D& _pos,float _radius = 2) = 0;

	/** \brief
	 *  ���������ڵ�һ��xzֵ���õ��˴��ĵر�߶�
	 *
	 *  \param osVec3D& _pos x,z��ֵ��_pos�ڵ�x,zֵ����.���صĸ߶�ֵ��_pos.y��
	 *  \param BOOL _precision �Ƿ�ȷ��ײ,��������Ʒ�߶���Ϣ,ֻ�����������ƶ�ʹ�˱���Ϊtrue
	 */
	virtual bool         get_detailPos( osVec3D& _pos,BOOL _precision = FALSE ) = 0;
     //! �õ�������ĳ��λ���ϸ��ر���Ʒ��صĸ߶���Ϣ,���û����Ʒ,��߶���ϢΪ��С�ĸ�ֵ.
	virtual float       get_objHeightInfo( osVec3D& _pos ) = 0;

	/** \brief 
	 *	����һ���������꣬����һ��DWORD��ֵ�����������������Ĺ���ͼ����ɫ��
	 *
	 *	\param osVec3D& _pos		x,z ������,Ϊ��������
	 */
	virtual DWORD		get_lightMapColor(const osVec3D& _pos) = 0;

	//! �������������λ��,�õ���λ�õĲ�������
	/*! \note ���������ֻ�õ���X,Z����*/
	virtual int			get_materialIdx(osVec2D& _pos,os_terrainTexSnd& _texsnd ) = 0;
	/** \brief
	 *  �ͻ��˵���ײ��⺯��.
	 *  
	 *  ����ͻ�����ײ��ⲻ��ͨ��������Ҫ���ɷ���������ȷ�ϣ�
	 *  �����ͨ�������ɷ�����ȷ�Ϻ����һϵ�еĶ����Ͳ����� 
	 *  
	 */
	virtual bool          collision_detection( const os_tileIdx& _tile ) = 0;

	/** \brief
	 *  �õ�������ĳһ����ͼ����ײ��Ϣ���ݿ顣
	 * 
	 *  \param _tgIdx     �����е�ͼ����������(0,0)��ʼ��
	 *  \return os_TileCollisionInfo** ���ص���os_TileCollisionInfo�Ķ�ά���顣
	 *                                 ����os_TileCollisionInfo��128][128] 
	 */
	virtual const os_TileCollisionInfo* get_sceneColData( os_tileIdx& _tgIdx )=0;

	/** \brief
	 *  �ɴ��������ռ����㵱ǰ����ռ������һ����ͼ�ϣ��ڵ�ͼ����һ��ͼ����.
	 *  
	 *  \param  _res     ʹ��os_pickTileIdx�ṹ�������ǵõ���ֵ.
	 *  \param  _pos   ����������ռ�Ķ���λ�á�
	 *
	 *  \return bool     �����ǰ�����ֵ������ǰ���Ŵ��ͼ�Ŀ�ȣ�����false.
	 */
	virtual bool                   get_sceneTGIdx( 
		                             os_pickTileIdx& _res,const osVec3D& _pos ) = 0;

	/** \biref
	 *  �õ���ǰ�����ڵĵ�ͼ����Ŀ������.
	 * 
	 *  �ͻ�����Ѱ·�㷨ʱ���õ���Щ����.
	 *  
	 *  \return int     �����������ڵĵ�ͼ����Ŀ.
	 *  \param  _tgidx  ����ķ��ص�ͼ������ָ��ṹ,��෵��4��._tgidxӦ����os_tileIdx[4].
	 *  
	 */
	virtual int                   get_inViewTGIdx( os_tileIdx* _tgidx ) = 0;

	//! �õ���ǰ�����п��Կ����ĸ��ӵ���Ŀ��
	virtual int                   get_inViewTileNum( void ) = 0;


	/** \brief
	 *  ���ڴ����Ͳ����ϲ���Ҫ��SkinMeshObject.
	 *
	 *  \param _sceneSm �Ƿ񴴽������е�sm��Ʒ�������ⲻ��Ҫʹ�����������
	 *  \param _smi ���ڳ�ʼ���´�����skinMeshObject.
	 *  \return     �������ʧ�ܣ�����NULL.
	 *
	 *  �ͷ�һ��I_skinMeshObj�ӿ�ʱ,ʹ��I_skinMeshObj::release_character();
	 */
	virtual I_skinMeshObj*  create_character( os_skinMeshInit* _smi,
		                        bool _sceneSm = false,bool _waitForFinish = false ) = 0; 

	//! ��һ��characterָ���ƽ���ǽ����Ⱦ������
	virtual void            push_behindWallRenderChar( I_skinMeshObj* _ptr ) = 0;

	//! ��һ��characterָ���ǽ����Ⱦ������ɾ��
	virtual void            delete_behindWallRenderChar( I_skinMeshObj* _ptr ) = 0;


	//@{
	//  �����й����صĽӿڡ�
	/** \brief
	 *  ʹ���ļ���������ꡣ
	 */
	virtual int            create_cursor( const char* _cursorName ) = 0;
	//! ������������ʾ�Ĺ�ꡣ
	virtual void           set_cursor( int _id, int _centerX = 0, int _centerY = 0 ) = 0;
	//! ��õ�ǰ��ʾ�Ĺ��id
	virtual int            get_cursor() = 0;
	//! �Ƿ���ʾ���
	virtual void           hide_cursor( bool _hide = false ) = 0;

	//@} 



	//@{
	//
	// �����й������صĺ�����
	// 
	/** \brief
	 *  �������м���billBoard.
	 *
	 *  �����е�Billboard�ڳ�ʼ����ʱ��̶�λ�ã��Ժ��ܸı䡣
	 *  
	 *  ATTENTION: ����Ǽ�����ѭ�����ŵĹ���壬������ֹ�����ɾ����������ɾ����
	 *             ���й�������Ҫ�Ǽ�������һ�εĹ���嶯����������ɺ󣬻��Լ�ɾ���Լ���
	 *
	 *  \return int ���ŵĹ���巵�ؿ��Բ�����id,��Ϊ�������ܻ��Զ�ɾ���������ڲ���
	 *              �����֮ǰ��һҪ���ú���validate_bbId,����ǲ��ɲ�����id,���ϲ���Ҫ
	 *              �����id���д��֣�ʹ��һ�β��ٷ����Դ�id�Ĳ�����
	 * 
	 *  \param  bool _createIns �Ƿ񴴽�������ʵ��,����������Ļ�,�������ڴ�catch�����
	 *                          ��Դ,ʹ��һ�δ���������ʱ������ٶȸ���.
	 *  \param  float _sizeScale ��Ҫ�����Ĺ������д�С���š�Ĭ���Ǵ����Ĵ�С����1.0
	 *  \param  bool _topLayer   �Ƿ񶥲���Ⱦ�����ڻغ��Ƶ�ս������
	 */
	virtual int           create_billBoard( const char *_bfile,osVec3D& _pos,
		                    bool _createIns = true,float _sizeScale = 1.0f,bool _topLayer = false ) = 0;


	//! �ڳ����ڴ�������Զ������Ӱ��Ĺ����,�����Ĳ����������Ĳ���һ��
	virtual int           create_hugeBillBoard( const char* _bfile,osVec3D& _pos,
		                    bool _createIns = true,float _sizeScale = 1.0f ) = 0;

	/** \brief
	 *  ����billBoard��λ�á�һ�������еĹ���壬���ܿ���λ�á�
	 *
	 *  \param _id ��Ҫ���õĹ�����id��
	 *  \param _pos  Ҫ���õĹ�����λ�á�
	 */
	virtual void          set_billBoardPos( int _id,osVec3D& _pos ) = 0;

	/** \brief
	 *  ���ù����Ĳ���ʱ�䡣
	 *  
	 *   \return ������Ҫ���ù�������������ʱ�䡣
	 *   \param _timeScale Ҫ���õĹ����Ĳ���ʱ����ԭ������ʱ����������Ϊ��ֵ���ӿ�ֻ���ع����
	 *                ����������ʱ�䣬�����ò���ʱ�����������_timeScaleΪ2.0f,�򲥷��ٶ�Ϊ����
	 *                �ٶȵ�2����
	 */
	virtual float         set_bbPlayTime( int _id,float _timeScale ) = 0;

	//! ��֤billBoardId����Ч�ԡ�
	virtual bool          validate_bbId( int _id ) = 0;


	/** \brief
	 *  ɾ��billBoard.
	 *
	 *  \param _id ɾ��ָ��id��billBoard.
	 *  \param bool _finalRelease �Ƿ�ɾ���˹������Ӧ���豸��Դ
	 */
	virtual void          delete_billBoard( int _id,bool _finalRelease = false ) = 0;

	//! ���ص�ǰ��billBoard.
	virtual void          hide_billBoard( int _id,bool _hide ) = 0;

	//! ���ŵ�ǰ�Ĺ����.river added @ 2010-1-7:
	virtual void          scale_billBoard( int _id,float _rate ) = 0;

	// 
	//@}

	//@{
	// 
	// �����ж�����صĺ���.�˴��Ķ����ǹ�������.
	// �����ļ��ĸ�ʽΪ*.osa��ʽ�Ķ���.
	/** \brief
	 *  �����м��붯���ļ�,�������Ķ����ļ����ڵ�ǰ�������Ұ��,����Ҫ��Ⱦ�¼����
	 *  ����.
	 *
	 *  \param _mc �����������ݵĽṹ,�������ṹ,�������ǵĶ�������.
	 */
	virtual int           create_osaAmimation( os_aniMeshCreate* _mc,bool _topLayer = false ) = 0;
	
	//! ��ʼ����
	virtual void          start_animationMesh( int _id ) = 0;

	//! fade����fadeIn��ǰ��osa����.
	virtual void          start_fadeOsa( int _id,
		                    float _fadeTime,bool _fadeOut = true,float _minFade = 0.0f ) = 0;

	//! ���ص�ǰ��osa����
	virtual void          hide_osa( int _id,bool _hide ) = 0;


	/** \brief
	 *  ��Ҫ��֤ÿһ��������Ʒid�ĺϷ���
	 *
	 *  \param int& _id ��������id���Ϸ������_id��ֵΪ-1
	 */
	virtual bool          validate_aniMeshId( int _id ) = 0;

	//! ����ĳһ��������λ�á�
	virtual void          set_aniMeshPos( int _id,osVec3D& _pos ) = 0;
	//! ����һ����������ת����.
	virtual void          set_aniMeshRot( int _id,float _angle ) = 0;
	//! ���ŵ�ǰ��osa����
	virtual void          scale_aniMesh( int _id,float _rate,float _time ) = 0;

	/** \brief
	 *  ����osa�����Ĳ���ʱ�䡣
	 *  
	 *   \return ������Ҫ����osa��������������ʱ�䡣
	 *   \param _timeScale Ҫ���õ�osa�Ĳ���ʱ����ԭ������ʱ����������Ϊ��ֵ���ӿ�ֻ���ع����
	 *                ����������ʱ�䣬�����ò���ʱ�䡣
	 */
	virtual float         set_osaPlayTime( int _id,float _timeScale ) = 0;


	/** \brief
	 *  �ϲ㲥���������Ч�Ľӿ�
	 *  
	 *  \param const char* _ename Ҫ������Ч������
	 *  \param osVec3D& _pos      Ҫ������Ч�ڳ�����λ�á�
	 *  \param float _angle       Ҫ������Ч����ת�Ƕȣ���Y��
	 *  \param BOOL _instance     �Ƿ񴴽���������Чʵ���������ֵΪfalse,��������Ԥ������Ч������ʾ��
	 *  \param float _scale       ������Ч�����űȽ�����
	 *  River added @ 2009-6-22: �����ϲ�õ���Ч���ŵ�ʱ�������ʱ��Ϊ�����һֱ���ŵ���Ч����
	 *                           �˷���ֵΪ��ֵ
	 *  \param float* _effectPlaytime Ҫ��������Ч�������Ĳ���ʱ�䡣 
	 */
	virtual DWORD         create_effect( const char* _ename,
		                      osVec3D& _pos,float _angle = 0.0f,
							  BOOL _instance = TRUE,float _scale = 1.0f,
							  float* _effectPlaytime = NULL ) = 0;
	/** \brief
	 *  ���ϲ�ɾ��һ����Ч
	 *
	 *  \param bool _finalRelease �Ƿ�ȫ���ͷ���Ч�豸�����Դ��ͨ��ֻ�༭����ʹ�á�
	 */
	virtual void          delete_effect( DWORD _id,bool _finalRelease = false ) = 0;

	//! River @ 2010-1-14:�ϲ���Ե�һ��effect��keyTime.
	virtual float         get_effectKeyTime( DWORD _id ) = 0;


	//! �õ�effect��Ӧ��boundingBox,����boxsize�Ǿ���scale���size.��box����ת
	virtual bool          get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize ) = 0;

	/*  \brief 
	 *   �õ�һ������ת����İ�Χ�С�
	 *   
	 *   �ϲ�õ���Χ�к���Ҫ����һ�ε�������ӿ�֮ǰʹ�ã��ڲ���bbox��һ��static������
	 *   �����б𷵻�ֵ����������Ч�༭����û���趨��Χ�е���Ч������ֵΪ��.
	 */
	virtual const os_bbox*   get_effectBox( DWORD _id ) = 0;


	//! ����Ч��������,���ڶԳ����ڵĵ��߽������Ŷ������������Ž�����osa�����͹���壬���ӳ������ܶ�̬����
	virtual void          scale_effect( int _id,float _rate,float _time ) = 0;
	//! ����Ч����fadeOut
	virtual void          fade_effect( int _id,float _fadetime,bool _fadeOut = true ) = 0;

	/*  \brief
	 *  River @ 2010-8-24:����Ч�Ĳ����ٶȽ������š�
	 *
	 *  \param float _speedRate  ԭ�������ٶȵı��������ֵΪ0.1�������ЧΪ���������ٶȵ�0.1����
	 */

	virtual void          scale_effectPlaySpeed( int _id,float _speedRate ) = 0;


	/** \brief
	 *  ������Ч��alphaֵ,ֻ��osa��Ч������ȷ������alphaֵ��alphaֵ����0.0��1.0f֮�䡣
	 *
	 *  \param int _id   ��Ч��Ӧ��id.
	 *  \param float _alpha ��ЧҪʹ��alphaֵ
	 */
	virtual void          set_effectAlpha( int _id,float _alpha ) = 0;


	//! ���ػ���ʾ��ǰid��effect
	virtual void		  hide_effect(int _id,bool _bHide = TRUE ) = 0;

	/** \brief
	 *  �ϲ�������Ч��λ�ú���ת����
	 *
	 *  \param float _speed �ϲ���������Чλ��ʱ�����Դ���Ĳ�������������ά��������Ч��
	 *                      �˲�������ʹ�������������� 
	 */
	virtual void          set_effectPos( DWORD _id,osVec3D& _pos,float _angle,
		                     float _speed = 1.0f,bool _forceUpdate = false ) = 0;
	//! ��֤һ��effect�Ƿ��Ѿ����ڲ���ɾ��
    virtual BOOL          validate_effectId( DWORD _id ) = 0;


	/** \brief
	 *  ����һ��������ʾ�����λ�õı�־��
	 */
	virtual void	      draw_cursorInScene( os_sceneCursorInit& _scInit ) = 0;


	//! ɾ��һ������.
	virtual void          delete_animationMesh( int _id,bool _sceC = false ) = 0;

	//@} 

	//@{
	//  �ͻ����õ����ض����ݣ�����ֻ������벢���ϲ���ã������ڲ�������
	/** \brief 
	 *  �õ���Ч���ŵ�λ������ 
	 *
	 *  \param int _id  ���봴������Чʵ����id.
	 *  \return DWORD   0:��Χ�б������š� 1:����λ�ò��š�2:������λ�ò��š�
	 */
	virtual DWORD         get_effectPlayPosType( int _id ) = 0;
	//! �õ���Ч���ŵİ�Χ�б��������ص�ֵ��0.0��1.0֮�䡣
	virtual float         get_effectPlayBoxScale( int _id ) = 0;
	//@}



	//@{
	//  ������صĺ����ӿ�.����,���ú�ɾ��.
	/** \brief 
	 *  ����һ��particleʵ��.
	 *
	 *  \param bool _topLayer �Ƿ񶥲���Ⱦ�����ڻغ�����Ϸ���
	 */
	virtual int           create_particle( os_particleCreate& _pcreate,
		                         BOOL _inScene,bool _topLayer = false ) = 0;

	//! ����particle��λ��.
	virtual void          set_particlePos( int _idx,osVec3D& _pos ) = 0;

	//! ���ص�ǰ��particle.
	virtual int           hide_particle( int _id,bool _hide ) = 0;

	/** \brief
	 *  �������Ӷ����Ĳ���ʱ�䡣
	 *  
	 *   \return ������Ҫ�������Ӷ�������������ʱ�䡣
	 *   \param _timeScale Ҫ���õ����Ӷ����Ĳ���ʱ������������ʱ��ı��������Ϊ��ֵ���ӿ�ֻ���ع����
	 *                ����������ʱ�䣬�����ò���ʱ��ȡ�����_timeScaleΪ2.0f,�򲥷��ٶ�Ϊ����
	 *                �ٶȵ�2����
	 */
	virtual float         set_parPlayTime( int _id,float _timeScale ) = 0;


	//! ���particleId�Ŀ����ԣ���������ã����ϲ���Ҫ����ǰ��particle Id.
	virtual bool          validate_parId( int _id ) = 0;

	/* \brief
	 * ɾ��һ��particle.
     *
	 * \param int _id  ����id.
	 * \param bool _finalRelease �Ƿ��ͷŴ����Ӷ�Ӧ���豸�����Դ
	 */
	virtual void          delete_particle( int _idx,bool _finalRelease = false ) = 0;

	//@} 

	//@{
	//  �绡��ص���Ч��
	//! ����һ���绡��
	virtual int           create_eleBolt( os_boltInit* _binit,int _bnum ) = 0;
	//! �������û���
	virtual void          reset_eleBolt( os_boltInit* _bset,int _idx ) = 0;
	//! ɾ��һ���绡��Դ��
	virtual void          delete_eleBolt( int _id ) = 0;
	//@}


	//{@ windy 6-21 add
	//! ��ʼ��������,
	/*!\param _times ,����ʱ�䳤��
	\param frequency һ����������.
	\param flashTime ÿ������,��ռʱ�� [0-1]֮����
	\param _color ������ɫ*/
	virtual void	start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f))=0;
	virtual void	stop_sceneLight() = 0;
	//@}
	//@{
	// ��̬����صĽӿڡ�
	/** \brief
	 *  ����һ����̬�⡣
	 *
	 *  \return int  �������-1,�򴴽�ʧ�ܡ�
	 */
	virtual int           create_dynamicLight( os_dlightStruct& _dl ) = 0;

	/** \brief
	 *  �õ�һ����̬��ĸ��������
	 */
	virtual void          get_dynamicLight( int _id,os_dlightStruct& _dl ) = 0;

	/** \brief
	 *  FrameMoveһ����̬�⡣
	 */
	virtual void          set_dynamicLight( int _id,os_dlightStruct& _dl ) = 0;

	//! ���ö�̬���λ�á�
	virtual void          set_dLightPos( int _id,osVec3D& _pos ) = 0;

	/** \brief
	 *  ɾ��һ����̬�⡣
	 */
	virtual void          delete_dynamicLight( int _id,float _fadeTime = 0.0f ) = 0;


	/** \brief
	 *  �ı䳡���Ļ�����ϵ����
	 *
	 *  \param _r,_g,_b  �����л������ϵ����ʹ��������ֵ���ų����еĻ����⡣
	 */
	virtual void          change_amibentCoeff( float _r,float _g,float _b ) = 0;
	//@}


	//@{
	/** \brief
	 *  ����Ӱ��صĽӿڣ�����,npc�͹���������һ����ɫ��СȦ��
	 *
	 *  \param float _scale ����Ӱ�Ĵ�С����ֵ�����ڴ�Ĺ����Ȧ���Դ�һЩ��
	 */
	virtual int           create_fakeShadow( osVec3D& _pos,float _scale = 1.0f ) = 0;
	virtual void          set_shadowPos( int _id,osVec3D& _pos ) = 0;
	virtual void          delete_shadowId( int _id ) = 0;
	virtual void          hide_shadow( int _id,bool _hide = true ) = 0;
	//@} 

	//  TEST CODE:
	//! ������ɾ�������ر��ϵ�Decal
	virtual int           create_decal( os_decalInit& _dinit ) = 0;
	virtual void          delete_decal( int _id ) = 0;
	virtual void          reset_decalPos( int _id,osVec3D& _pos ) = 0;


	//@{
	//! �����п�ʼ�ͽ�������Ч����
	virtual void          start_sceneRain( os_rainStruct* _rs = NULL ) = 0;
	virtual void          stop_sceneRain( void ) = 0;
	
	/** \brief
	 *  �����ڿ�ʼ�ͽ�����ѩ��Ч��
	 *
	 *  \param BOOL _start �������ʼ��ѩ�����Ϊfalse,������ѩ��Ч��
	 *  \param int _grade  ��Ϊ0,1,2�������𡣼���Խ�ߣ���ѩ��Խ�ࡣ
	 */
	virtual void          scene_snowStorm( BOOL _start = TRUE,int _grade = 0 ) = 0;
    //@} 


	//@{
	//  ���볡���еĶ�ά��ĻͼƬ��ʾ��Ҫ�Ľӿڡ�
	//  ��άͼƬ����ʾ��Ҫ�ɿͻ������Լ��������򣬰���Ҫ�ŵ��±ߵ�ͼƬ�����ƽ��е�����
	//  �н�����ʾ��
	
	/** \brief
	 *  ����һ����ά��ʾ��Ҫ������
	 *��
	 *  �����л���ʱ�����е�������ɳ��������Զ����ͷš�
	 */
	virtual int            create_scrTexture( const char* _texname ) = 0;

	//! ͨ��id�õ�һ������ ATTENTION : River:�ϲ���Ҫֱ�Ӳ���LPDIRECT3DTEXTURE9�ӿڣ���
	virtual LPDIRECT3DTEXTURE9 get_sceneScrTex( int _id ) = 0;
	//! testע��һ������,����id
	virtual int               register_texid( LPDIRECT3DTEXTURE9 _pTexture ) = 0;
	virtual void    unregister_texid( int _texid ) = 0;
	virtual int		register_targetAsTex( int _targetIdx ) = 0;


	//! ����һ������
	virtual int			create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture ) = 0;

	/** \brief
	 *  �ͷ�һ����ͼ��
	 */
	virtual void           release_scrTexture( int _texid ) = 0;

	//syq_mask ������ȾĿ��
	virtual int                create_renderTarget( int _width, int _height, D3DFORMAT _format = D3DFMT_A8R8G8B8 ) = 0;
	/** \brief
	 *  syq_mask �����ȾĿ��
	 *
	 *  ��������id��-1���򷵻�����Ⱦ�豸��Backbuffer.
	 */
	virtual LPDIRECT3DSURFACE9      get_renderTarget( int _id ) = 0;
	virtual LPDIRECT3DTEXTURE9      get_renderTexture( int _id ) = 0;


	/** \brief
	 *  ����Ҫ��ʾ�����ݽṹ�ƽ��������н�����ʾ��
	 *
	 *  ������ݱ��ƽ��е������е����ݽṹ�У�����ʾ�곡���е���άԪ�غ�
	 *  ��ʼ��ʾ��Щ��ά��Ԫ�ء�
	 */
	virtual void           push_scrDisEle( const os_screenPicDisp* _dis ) = 0;
	//@}

	//@{
	/** \brief
	 *  Draw text on the screen.  
	 *
	 *  ���������������ʾ������Ϣ����Ҫ�ĺ���,����֧��Unicode.�����������
	 *  ����beginScene��endScene����֮��.
	 */
	virtual int          disp_string( os_stringDisp* _dstr,int _dnum,int _fontType = 0,
		                 bool _useZ = false,bool _bShadow = false, RECT* _clipRect = NULL ) = 0;

	/** \brief
	*  ��������Ļ����ʾ�ִ���������disp_string����һ�¡�
	*
	*  \param _cursor ��Ҫ������ʾ�Ĺ�����ݽṹ
	*/
	virtual int          disp_stringImm( os_stringDisp* _dstr,int _fontType = 0,os_screenPicDisp* _cursor = NULL ) = 0;


	/** \brief
	 *  ��Ⱦ����ɫ�������ܵ������ִ�,����ֵΪ��ǰ���ִ��ڸ����������£���ʾ�˶����С�
	 *
	 *  \return int �������ֵС�㣬���ʾ�����ڸ����������������Ļ���������ִ���
	 *  
	 */
	virtual int          disp_string( os_colorInterpretStr* _str,int _fontType = 0,
		                 bool _useZ = false,bool  _bShadow = false, RECT* _clipRect = NULL ) = 0;
	//@}
	//@{
	// Windy mod @ 2005-9-20 15:02:51
	//Desc: ����Lens Flare�Ľӿ�
	//!������������Lens Flare�Ƿ���ʾ�������漰����Դ
	//virtual void	start_LensFlare() = 0;
	//virtual void	stop_LensFlare() = 0;


	///���ļ��м���lens flare
	virtual void	Load_LensFlare(const char * _fname) = 0;
	virtual void	clear_LensFlare() = 0;
	//!����3Dλ��
	virtual void	set_RealPos(osVec3D & _vec3Pos) = 0;

	///�����Ǳ༭���Ľӿ�
	
	// 
	// ATTENTION River:�༭���Ľӿڣ��ͼ���꣬��������ʱ��ȥ����Щ�ӿںʹ���
	///�޸�nPartIDָ����Flare��ֵ����������_Mask����
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

	//! �õ���ǰ������������
	virtual void          get_sceneWeather( os_weather& _weather ) = 0;

	//! ǿ�ƽ���һ������.
	virtual void          stop_animationMesh( int _id ) = 0;

	//! ����ĳһ��������صĲ���.
	virtual bool          set_animationMesh( int _id,os_aniMeshCreate* _mc ) = 0;

# endif 


	//@{
	//  �޸ĵ�ͼ�ĸ߶���Ϣ����ײ��Ϣ�Ľӿڡ�
# if __COLHEI_MODIFY__
	/** \brief
	 *  �޸�������������λ�õĸ߶���Ϣ��os_sceneFM�ڵ�m_iCx,m_iCy������ʾ��λ�á�
	 *
	 *  \param float _mod  Ҫ�޸ĵĸ߶ȣ������ɸ���
	 */
	virtual bool          modify_heightInHeroPos( float _mod ) = 0;

	
	//! ���������λ������Ϊ��ԭ���෴�Ŀ�ͨ���ԡ�
	virtual bool          modify_colAtMousePick( os_pickTileIdx& _ps, BOOL bUseParm=FALSE,bool bPass=false) = 0;
	
# endif
	//@} 



	//! �����ú�������һ��bounding Box.
	virtual void          draw_bbox( os_bbox* _box,DWORD _color,bool _solid = false ) = 0;
# ifdef _DEBUG

	//! ���ڶ��²�ı����������.
	virtual void          debug_test( void ) = 0;
# endif 

	/** \brief
	 *  �����ú�������һ����.�˺��������ڵ��ԣ��ӿڲ�ȫ,Ч�ʲ��ߡ�
	 *  ATTENTION :��������������豸��benginScene��endScene֮����á�
	 *
	 *  \param const osVec3D* _verList Ҫ���ߵĶ����б�������lineStrip.������a,b,c��������
	 *                                 ��ử��a,b,c��������֮������ߡ�
	 */
	virtual void          draw_line( const osVec3D* _verList,int _vnum,
		                   DWORD _color = 0xffffffff,float _width =1.0f ) = 0;


# if __ALLCHR_SIZEADJ__
	//! ʹ������ӿڣ����Ե�����������������skinMesh��Ʒ�Ĵ�С��
	//! �˴��ĵ���Ϊͳһ�������������Ͳ���ص���Ʒ��
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
	//!���osaģ�Ͷ��������ٶ�
	virtual void 			set_OSAPlaySpeed(float _speed) = 0;

# endif 

//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------

};

class os_Ray;
//! ����Ƭʰȡ�ӿ�,ͨ�����벻ͬ��ѡȡ��ʽ,�õ�����Ƭ�б�.
/** �ýӿ���Ҫ������ײ���,�����ڲ���Ҫ����ײ�������嶼Ҫ�����ýӿ�.
����ײ���֮ǰÿһ֡,�������涼Ҫ��getTriangles�ӿڵõ���Ӧ������Ƭ�б�.
�Խ�����ײ���.*/
class ITriangleSelector : public os_refObj
{
public:

	//! ����
	virtual ~ITriangleSelector() {};
	//! �������п��õ�����Ƭ����
	virtual int getTriangleCount() const = 0;
	//! �õ����е�����Ƭ�б�
	//! \param[out] triangles ���ص������б�����
	//! \param[in] arraySize �����б��������
	//! \param[out] outTriangleCount ʵ��д�������Ƭ����
	//! \param[in] transform ����任����,���Ϊ�ձ�ʾ�����б任.
	//! �þ���ͨ�����ڰ�ŷ�Ͽռ�任�������ռ�,��������������
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const osMatrix* transform=0) const =0;

	//! �õ�λ��һ����Χ�е�����Ƭ�б�
	//! \param[in] box Ҫȡ������Ƭ�İ�Χ��
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_bbox& box, 
		const osMatrix* transform=0) const= 0;

	//! �õ��������ཻ������Ƭ�б�
	//! \param[in] box Ҫȡ������Ƭ���ཻ������
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_Ray& line, 
		const osMatrix* transform=0) const= 0;

};

//@{
//  �����������
# include "miskFunc.h"
//@} 




//==============================================================================================
//
//  �����µĺ����еõ����ǿ��Բ��ݵ�����ӿ�.
//
//==============================================================================================
/** \brief 
 *  �õ�ȫ���Ե�device Managerָ��ĺ���.
 *
 *  ʹ������������õ�I_deviceManager�Ľӿڼ�,ʹ������ӿڼ�������
 *  3D DEVICE.
 */
OSENGINE_API I_deviceManager*   get_deviceManagerPtr( void );
/** \brief
 *  �õ�InputManager�ӿڵĺ���.
 */
OSENGINE_API I_inputMgr*        get_inputManagerPtr( void );


/** \brief
 *  �õ�I_reource�ӿڼ��ĺ���.
 */
OSENGINE_API I_resource*        get_resourceMgr( void );

/** \brief
 *  �õ����������ӿڼ��ĺ�����
 */
OSENGINE_API I_fdScene*         get_sceneMgr( void );

/** \brief
 *  �õ�ȫ�ֵ�������ؽӿڡ�
 */
OSENGINE_API I_socketMgr*       get_socketMgr( void );

//! �����˳�ʱ���ͷ�ȫ�ֵľ�̬���������
OSENGINE_API void              release_renderRes( void );

//! ֪ͨ�ϲ��豸״̬�ص� syq 2006-9-20
typedef int (*CALLBACK_DEVICE )(int,int);
OSENGINE_API void register_lostDeviceFunc( CALLBACK_DEVICE lpFunc );
OSENGINE_API void register_resetDeviceFunc( CALLBACK_DEVICE lpFunc );
