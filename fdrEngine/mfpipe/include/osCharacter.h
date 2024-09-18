///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacter.h
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     用于os引擎中的人物动画调用。
 *  
 *  "女人如甘醇，少饮则提神爽气抑性，多饮则耗神消气，濫饮则灭志杀身"
 *  
 */
///////////////////////////////////////////////////////////////////////////////////////////
# ifndef __OSCHARACTER_INCLUDE__
# define __OSCHARACTER_INCLUDE__



# include "../../interface/osInterface.h"
# include "../../tools/include/fadeMgr.h"
# include "osShader.h"
# include "osPolygon.h"
# include "../include/osChrRide.h"

#include "../../Terrain/include/fdFieldMgr.h"
#include "../../fdrEngine/scenePostTrail.h"

//
// 在新的插件程序中，每一个顶点最多关联３根骨骼，如果只关联了一根或是两根骨骼，则程序
// 自动加到三根骨骼的数目。以使3d设备最少的使用状态切换。

//! 一个武器最多可以有的特效元素数
# define EFFECTNUM_PERWEAPON  3

//! 人物加入明暗面
# define CHAR_AMBISCALE 1.0f

//
// 使用库文件调入各种文件格式。

//! 每一个bpm最多可以使用的bone数目.
# define MAX_BONEPERBPM 26

//! 一个骨骼系统中最多的Frame数目。
# define MAX_FRAMENUM    128

# define SKINMESH_DEBUG   1

//! bodyPartMesh文件名的最大长度。
# define MAX_BPMFNLEN 64


//! 定义场景中最多可以使用skinMesh数目,第个场景最多64个skinMesh物品
extern int  MAX_SCENESMNUM; 

//@{
//! 身体部位的定义
# define PART_DECORATION     0
# define PART_HEAD          1
# define PART_COAT          2
# define PART_PANTS         3
# define PART_FOOT          4
# define PART_HAND          5
# define PART_WEAPON        6
//@}


//! 刀光文件的文件头和扩展名。
# define GLOSSY_FHEADER  "gls"


//! 每一个动作最多可以使用32个字节做为这个动作的名字。　
# define ACT_NAMELEN        32

//! 变换一个动作需要的时间。
# define ACT_CHANGESTATETIME 0.10f

//! 每一个骨骼顶点关联的骨骼数目。
# define VERTEX_INFLUENCE    3


//////////////////////////////////////////////////////////
//////! 刀光文件数据的版本号
// 
// 加入了刀光附加数据后的BodyPartMesh(*.skn)文件版本号。
# define GLOSSY_BPMFVERSION    102
//! 
# define GLOSSY_BPMFVERSION103    103
//! 104的刀光数据加入了对刀光拖尾长度的编辑
# define GLOSSY_BPMFVERSION104    104
//! 105的刀光数据加入了对武器特效缩放的支持
# define GLOSSY_BPMFVERSION105    105
//! 106的刀光数据加入了对4个武器特效文件的支持，用于多次强化武器
# define GLOSSY_BPMFVERSION106    106
//! 107的刀光数据加入了多个武器特效位置的支持
# define GLOSSY_BPMFVERSION107    107


//! 用于渲染IndexedBlend的SkinMesh数据，支持Geforce级显卡,最后一个宏可以去掉???
#define IDXBLEND_FVF ( D3DFVF_XYZB3|D3DFVF_LASTBETA_UBYTE4|D3DFVF_NORMAL|D3DFVF_TEX1 )


////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                              关于骨骼动画的简单说明

可以选择多种动画方式：
   1: 使用VS matrix palette动画。
   2: 使用d3d的索引动画方式。
   3: 可以测试使用hlsl的动画。
   4: 注意对应不同的身体部位数据，可能需要不同的
      effect文件。
装备相关：
　　装备和动作相关的结构由上层的游戏程序进行处理，此处只处理
  　最简单的动画信息。
    

关于骨骼动画数据需要的资源：
　　
　　1: 顶点缓冲区：
　　由中管道处理顶点的填充问题.
  　修改中间管道，使中间管道可以释放顶点缓冲区和重新填充顶点
    缓冲区，这在更换人物装备或外形的时候，用得到。

	2: Effect文件,中间包括了需要的vertex shader.
	由全局的shaderMgr来处理得到。

	3: 3d设备：
	在创建和渲染一个3d骨骼动画物品的时候，传入前端管道的指针或是中间
	管道的指针，使用可以设置3d设备的内部状态。
    

骨骼动画应该注意的事项：
　　在当前的动画结构中，只有osc_skinMesh和他中间的osc_bodyFrame和osc_bpmInstance
  　相对于屏幕上的显示角色是唯一的结构，其它的结构都由Manager管理和创建的共享类。


*/
////////////////////////////////////////////////////////////////////////////////////////////////////////



//! 当前场景中skinMesh用到的顶点结构。
struct os_characterVer
{
	osVec3D   m_vec3Pos;

	//! 骨骼权重,固定使用三根骨骼。
	float     m_vecWeight[2];
	BYTE      m_vecBoneIdx[4];

	//! skinMesh顶点的normal值。
	osVec3D   m_vec3Normal;

	//! 顶点的uv坐标，以后可能使用两层纹理，以做出特效。
	osVec2D   m_vec2TexUv;

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   　　　　　　　骨骼动画的动作相关类，所有的动作都从动作管理器中得到.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 定义每一套骨骼对应的最大的骨骼动画数目。
#define MAX_ANI_PERBONE	    128


/** \brief
 *  每一个动作的关键帧数据,包含quaternion和offset.
 *
 *  有些动作会出问题??????
 */
struct os_bactPose
{
	osQuat      m_actQuat;
	osVec3D     m_vec3Offset;


	//! River @ 2005-3-4: 加入了FlipDeterminant值的动作版本。
	//! 如果此值为真，则需要对旋转数据产生的矩阵进行-1的Scale.
	BOOL        m_bFlipDeterminant;


	//! 从当前的动作得到矩阵。
	void        get_posMatrix( osMatrix& _mat );

};
typedef std::vector< os_bactPose > VEC_bactPose;

/** \brief
 *  骨骼的动作。
 */
struct os_boneAction
{
	int         m_iBoneNum;
	int         m_iNumFrame;

	//! 存放当前动作信息的矩阵
	VEC_bactPose   m_vecMatrix;



	//! 播放当前动作需要的时间。
	float       m_fTimeToPlay;

	//! 当前动作的播放速度.帧/秒
	int         m_iPlaySpeed;

	//! 处理得到当前人物的停止动作帧索引。
	int         m_iStopFrameIdx;

	//! 当前动作对应的名字,上层可以使用名字来播放动作。
	char        m_szActName[ACT_NAMELEN];


public:
	os_boneAction();

	/** \brief
	 *  从文件中调入一个骨骼的bone的动作文件。
	 *  
	 */
	bool        load_boneActFromFile( const char* _fname );

	/** \brief
	 *  设置当前动作的播放速度.
	 *
	 *  \param _speed 每秒钟播放当前动画多少次.
	 */
	void        set_actPlaySpeed( float _speed );

	/** \brief
	 *  设置当前动作的名字。
	 */
	void        set_actName( const char *_name );
 
	

};
typedef std::vector< os_boneAction > VEC_boneAction;


/** \brief
 *  每一套人物的动作都对应一个名字列表，这样上层可以通过名字来调用动作。
 */
struct os_actNameStruct
{

	//! 当前动作名字对应的动作数目。
	int       m_iActNum;

	//! 当前动作的名字。
	char      m_szActName[ACT_NAMELEN];

	//! 当前动作名字对应的动作的索引。
	int       m_vecActIdx;

	//  River @ 2010-6-22:
	//! 做当前的动作时，是否隐藏武器部位.
	BOOL      m_bHideWeapon;

	os_actNameStruct()
	{
		m_iActNum = 1;
		m_szActName[0] = NULL;
		m_vecActIdx = 0;
		m_bHideWeapon = FALSE;
	}
};
typedef  std::vector<os_actNameStruct>  VEC_actName;

/** \brief
 *  处理骨骼动画数据的类。
 *  
 *  是当前骨骼的动作列表,最后可能把一个骨骼所有的动作都合并到一个文件中去。
 *  调入当前的骨骼动作时，调入所有的动作。
 */
struct os_boneAni
{
	char                m_szAniName[32];
	DWORD               m_dwHash;

	DWORD				m_iNumBones;
	DWORD				m_iNumParts;
	VEC_dword			m_ptrBone;
	DWORD				m_iNumAniTypeCount;

	//! 分配最大的动画数目。
	DWORD				m_iTotalAniFrame;

	//! 骨骼动画帧的数目。
	int               m_iBANum;
	//! 骨骼动画帧的列表。
	VEC_boneAction     m_vecBATable;

	//! 当前套动作每一个动作的名字信息。
	VEC_actName        m_vecActName;


	/** \brief
	 *  从动作文件中得到新的动作数据。
	 *  
	 *  \param _id 因为动作文件使用了命名准则，所以只需要
	 *             给出_id就可以了得到新的动作。
	 */
	bool              load_boneAction( const char* _actName,int _actNum );

	/** \brief
	 *  从文件中调入骨骼动作的信息文件，包括每一个动作的名字，每一个动作的播放速度等。
	 *
	 *  \param _accIfname  传入的动作信息文件名。
	 */
	bool              load_actionInfoFile( const char* _actIfname );

	//! 从一个动作名字得到一个动作的索引。
	int               get_actIdxFromName( const char* _actName,int& _nameIdx );

	//! River @ 2010-7-1: 当前的动作名字是否需要隐藏武器.
	bool              is_actNameHideWeapon( const char* _actName );


	//! 从一个actNameIdx和这个名字内动作的索引，得到一个bone Act Table.
	int               get_actBoneAction( int _nameIdx );


};
typedef std::vector<os_boneAni>  VEC_boneAni;

/** \brief
 *  骨骼动作管理器。
 *
 *  根据角色的名字，判定当前管理器是否有当前角色的动作，如果没有，把当前角色的所有的
 *  动作都调入到动作管理器中。
 *  
 */
class osc_boneAniMgr
{
private:
	 VEC_boneAni       m_vecBoneAni;
	 int              m_iBoneAniNum;

public:
	osc_boneAniMgr();
	~osc_boneAniMgr();

	//! 释放当前的boneAniMgr.
	void               release_boneAniMgr( void );

	/** \brief
	 *  从文件中创建出当前骨骼系统中全部的动作数据。
	 *  
	 *  如果当前的管理器中已经有了这个数据，返回数据指针。
	 *  
	 *  \return os_boneAin* 如果返回空，则创建失败。
	 */
	os_boneAni*        create_boneAni( const char* _aniName,int _aniNum );

};



//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      开始Mesh数据相关类和他的管理器，调入*.SKN文件
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////

/** \brief
 *  使用以下数据去初始化一个skinMesh的设备相关数据部分。
 *  
 */
struct  os_smDevdepDataInit
{
	int     m_iVerNum;
	int     m_iVerSize;
	void*   m_ptrDStart;

	int     m_iIdxNum;
	WORD*   m_ptrIStart;

	//! 对应的文件名。
	char*   m_strFname;

	os_smDevdepDataInit()
	{
		m_ptrDStart = NULL;
		m_ptrIStart = NULL;
		m_strFname = NULL;
	}

};

/** \brief
 *  Mesh的储备相关数据。
 *  
 *  加入os_refObj用于在使用过程中动态的释放不需要的内存资源。
 *  
 */
struct os_smMeshDevdep : public os_refObj
{
public:
	//! 当前的mesh对应的顶点缓冲区。
	LPDIRECT3DVERTEXBUFFER9   m_pBuf;

	//! 顶点数目。
	int                     m_iVerNum;
	int                     m_iVerSize;

	//! 当前的mesh对应的Idx缓冲区。
	LPDIRECT3DINDEXBUFFER9    m_pIdxBuf;
	int                       m_iIdxNum; 

	//! 当前的mesh对应的硬盘文件。
	char                    m_strMFname[64];
	DWORD                   m_dwHash;

	//! 当前的mesh是否处于被占用状态。
	bool                    m_bUsed;

protected:
	/** \brief
	 *  释放当前资源的纯虚函数，当当前的引用计数变为0时，自动调用这个
	 *  函数。
	 */
	virtual void release_obj( DWORD _ptr = NULL );


public:
	os_smMeshDevdep();
	~os_smMeshDevdep();

	//! 使用文件初始化当前的dev dep resource.
	bool                     init_smMesh( os_smDevdepDataInit& _vi,LPDIRECT3DDEVICE9 _dev );

	//! restore dev dep resource.
	bool                     restore_smMesh( LPDIRECT3DDEVICE9 _dev );

	//! on deivce lost.
	bool                     onlost_smMesh( void );


# if __GLOSSY_EDITOR__
	//! 选择后备份的索引缓冲区数据
	WORD*               m_wTriIdx;
	int                 m_iBackIdxNum;
	bool                m_bFirstSel;
	/** brief
	 *  只显示某个身体部位的部分三角形,公用于调试目的
	 *
	 *  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
	 *  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
	 */
	virtual int        displaytri_bysel( int _triidx[24],int _trinum );
# endif 	

};


//! Devdep 数据的stl vector
typedef std::vector<os_smMeshDevdep>    VEC_smMeshDevdep;

/** \brief
 *  管理skinMesh点储备相关资源的类。
 *
 *  上层需要从文件中创建一个mesh的时候，先查询中这个管理器，如果管理器
 *  中间存在这个mesh,则不需要再年重新创建mesh.
 *  
 *  由这个类和中间管道或是3d设备打交道，去创建设备相关的顶点和index资源。
 *  当一个顶点缓冲区没有使用他的客户的时候，释放这个顶点缓冲区或是Index buffer.
 *
 *  这个管理器中包含了顶点数据和shader数据的创建。
 *  
 */
class osc_smDepdevResMgr 
{
private:
	//! 储备相关数据的列表。
	VEC_smMeshDevdep       m_vecDevdepRes;

	//! 设备相关数据的指针。
	LPDIRECT3DDEVICE9       m_pd3dDevice;


private:
	//! 

public:
	osc_smDepdevResMgr();
	~osc_smDepdevResMgr();

	/** \brief
	 *  初始化当前的manager.
	 */
	void           init_smDevdepMgr( LPDIRECT3DDEVICE9 _dev );

	//! 释放当前的skinMesh Devdep Manager.
	void           release_smDevdepMgr( bool _finalRelease = true );


	/** \brief
	 *  从Mesh文件创建设备相关的数据。
	 *  
	 *  \return int -1表示处理失败。
	 */
	int            create_devdepMesh( os_smDevdepDataInit& _vi );


	//! lost&restore devdepMeshMgr.
	void           onlost_devdepMesh( void );
	void           restore_devdepMesh( void );



	/*** \brief
	 *  释放一个body part mesh的设备相关数据。
	 *  
	 */
	void           release_devdepMesh( int _id );


	/** \brief
	 *  使用id得到我们需要的设备相关数据。
	 */
	os_smMeshDevdep* get_smMesh( int _id );


	/** \brief
	 *  得到当前的人物相关数据占用的顶点和索引缓冲区内存大小。
	 *
	 *  \return int 返回以M为单位的数目。
	 */
	int            get_occuMemSize( void );

# if __GLOSSY_EDITOR__
	/** brief
	 *  只显示某个身体部位的部分三角形,公用于调试目的
	 *
	 *  \param int _resId  要处理的resourceId
	 *  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
	 *  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
	 */
	virtual int        displaytri_bysel( int _resId,int _triidx[24],int _trinum );
# endif 

};


//! 每一个bodyPartmesh后,如果附加了刀光数据,此为后面刀光数据的版本号.
# define GLOSSSY_DATAVERSION     0x100

//! 记录刀光位置的数目　
# define  MAX_SWORDPOS  18
# define  MIN_SWORDPOS  6

//! 渲染刀光时,得关键帧数据细分的个数.
# define  MAX_RENDERSNUM 192

//! 每一个武器最多可以有8个动作对应刀光。
# define  MAX_ACTGLOSSY 8

//! 定义刀光两渲染帧之间的最短矩离.
# define  MINDIS_INGRLINE  0.18f

/** \brief
 *  处理刀光的数据结构。
 *
 *  记录上几帧刀光的位置信息，并连结这些位置信息。
 *  
 *  
 */
class osc_swordGlossy
{
public:
	//! 记录刀光的开始点和结束点.
	osVec3D      m_vec3Start;
	osVec3D      m_vec3End;



	//! 第一个动作对应刀光的颜色,默认是纯白色.
	DWORD        m_vecGlossyColor[MAX_ACTGLOSSY];


	//! 刀光对应的中心点。
	osVec3D      m_vec3Center;

	//! 刀光关键线开结点和结束点的的长度.
	float        m_fSgLength;

	//! 此武器对应的骨骼的索引。
	int         m_iMatrixIdx;



	/////////////////////////////////////////////////////////////////////
	// 以下的数据为可以在武器之间共享的数据,用于不同人物使用相同刀光时

	//! 特效位置,介于0.0f和1.0f之间，在vecStart和vecEnd间插值。
	float       m_fEffectPos;

	//! 如果是osa文件,osa文件可以绕Y轴旋转,以调整特效模型的方向.
	float       m_fOsaRotYAngle;


	//! 每一个特效可以缩放，此值为武器特效的缩放值
	float       m_fScaleVal[EFFECTNUM_PERWEAPON];


	//! 对应刀光的动作数目。即哪一个动作对应刀光。
	int          m_iActNum;
	//! 动作的数组。
	int          m_vecActIdx[MAX_ACTGLOSSY];

	//! 武器对应的特效文件数据，每一个武器最多对应4个特效
	I_weaponEffect  m_sEffectName[4];

	/** \brief
	 *  刀光对应的纹理文件名.
	 *
	 *  只有文件名，在effectTex目录找文件.比如: "glossy"
	 *  则对应文件: "effTexture/glossy.hon"文件。
	 */
	char        m_szGlossyTex[32];

	//! 刀光的拖尾长度。此值的大小从6-12
	int         m_iGlossyMoveLength;

	//! River @ 2010-3-5:特效位置2和3.
	float       m_fAddEffectPos[2];

public:
	osc_swordGlossy();

# if __GLOSSY_EDITOR__
	//! 把刀光沿刀光中心点的x轴旋转，用于调整刀光。
	void        rot_swordGlossyX( float _radian );
	//! 把刀光沿刀光中心点的Y轴旋转，调整刀光。
	void        rot_swordGlossyY( float _radian );

# endif 

};



/** \brief
 *  刀光的实例数据。
 *
 *  
 */
class osc_swordGlossyIns
{
	friend class osc_skinMesh;
private:
	osc_swordGlossy*   m_ptrSwordGlossy;

# if SKINMESH_DEBUG
    osc_skinMesh*      m_ptrmesh;
# endif 

	/** \brief
	 *  记录刀光关键帧用到的刀光数据。
	 *
     *  最后一个数据就是刀光关键帧当前所在的位置.
	 */
	osVec3D      m_arrVec3KeyS[MAX_SWORDPOS];
	osVec3D      m_arrVec3KeyE[MAX_SWORDPOS];

	//! 记录当前要记录的刀光的位置。
	int          m_iGlossyPos;

	/** \brief
	 *  当前刀光的颜色.
	 *
	 *  如果刀光DWORD的alpha部分为0,则不需要渲染刀光.
	 */
	DWORD              m_dwGlossyColor;

	//! 渲杂刀光时,用到的插值关键帧的数目.
	int                m_iGRNum;

	//! 刀光插值要开始的位置。
	int                m_iInterSPos;

	

	//! 当前记录刀光位置是否超过了最大的空间。
	bool                m_bPosOverFlow;

	//! 是否处理和渲染刀光,如果值大于0,此值为动作索引.
	int                 m_iPRSword;


	//! 转化后的刀光的开始位置和结束位置。
	osVec3D      m_vec3TransStart;
	osVec3D      m_vec3TransEnd;


	/** \brief
	 *  渲染刀光关键帧用到的数据.
	 */
	static osVec3D      m_arrVec3Start[MAX_RENDERSNUM];
	static osVec3D      m_arrVec3End[MAX_RENDERSNUM];

	//! 使用公告板的顶点数据，渲染方式是跟alphaAdd的方式。
	static os_billBoardVertex     m_verSG[MAX_RENDERSNUM*2];


	//@{
	//  设备相关数据.
	//! 用于渲染刀光用到的顶点和索引缓冲区.
	static int                 m_iIdxBufId;
	static int                 m_iVerBufId;
	static osc_middlePipe*      m_ptrMidPipe;
	static os_polygon          m_sPolygon;
	//@} 

	/* \brief
	 *  刀光对应的特效设备相关id.
	 *
	 *  River @ 2009-12-9:最多对应三个effect Shader ID.
	 */
	int               m_iEffectId[EFFECTNUM_PERWEAPON];


	// 
	//! 刀光使用的纹理的shaderId.
	int               m_iShaderId;

	//! effect的类型.
	int               m_iEffectType[EFFECTNUM_PERWEAPON];

	//! 要切换的武器特效索引
	int               m_iWeaponEffIdx;

	//! 武器附加特效的缩放值,默认为1.0f
	float             m_fScaleWeaponEffect;

	//  River @ 2011-2-25:
	//! 是否已经可以画当前的刀光.
	bool              m_bReadyToDraw;

private:

	//! 填充当前刀光用到的indexBuffer数据。
	static void   fill_indexBufferData( WORD* _idxBuf );

	//! 填充刀光的顶点数据，主要计算刀光的alpha数据。
	void         fill_vertexBufferData( os_billBoardVertex* _verBuf );

	//! 最终对刀光的渲染.
	void         final_renderSG( int _lineNum );

	//! 根据刀光的长度，开始点，结束点，重新刀光关键线结束点的位置。
	void         get_sgEndPt( osVec3D _spt,osVec3D& _ept,float _len );


	/** \brief
	 *  在InterPos与最终的刀光关键线之间插值。
	 *
	 *  \param _is,_ie 开始关键线和结束关键线.
	 */
	void        Interplate_keyLine( int _is,int _ie );


	//! 根据缓冲区中的数据，重新布置刀光相关的数据
	bool        recal_sgData( void );

	//! 对相关联特效的处理。
	void         process_sgEffect( bool _bindPos = true );

	//! 从文件的扩展名得到特效的类型
	int          get_effTypeFromName( const char* _name );

public:

	//! 渲染刀光用到的数据结构。　
	osMatrix     m_matBone;

public:
	osc_swordGlossyIns();
	~osc_swordGlossyIns();

	static void init_swordGlossyIns( osc_middlePipe* _mp );

	//! 设置刀光实例指向的刀光数据。
	void        set_swordGlossyPtr( osc_swordGlossy* _sg,osc_skinMesh* _mesh = NULL );

	
	/** \brief
	 *  开始渲染刀光,对刀光实例切换动作.
	 *
	 *  动作的切换在刀光编辑器的模式下会导致刀光颜色的改变.使用刀光的动作此时
	 *  显示出正常的刀光.不使用刀光的动作此时显示出红色的刀光.
	 *
	 *  在正常引擎模式下,不使用刀光的动作不显示刀光.
	 *  \param _actIdx 新动作的索引.
	 */
	DWORD        start_swordGlossy( int _idx );

	//! 重设当前的刀光数据。每一帧开始时，调用这个函数。
	void        reset_swordGlossy( void );
	
	//! 存储当前帧的刀光的位置。
	void        add_swordPos( void );

	/** \brief
	 *  对刀光进行渲染
	 *
	 *  \param _ds 如果此值为false,则不需要渲染刀光，定位物品要用此功能。
	 *  \param bool _glossyDraw  是否最后画刀光，可能会出现需要画武器附加特效，而不画刀光的情形
	 *  \param bool _bindEffPos 是否重设武器附加特效的位置，武器特效残存于场景内的处理。
	 */
	bool        draw_swordGlossy( osVec3D& _spos,bool _glossyDraw,
		                 bool _ds = true,bool _bindEffPos = false );

	//! 添加刀光的关键帧。
	void        add_swordGlossyKey( osMatrix& _rotMat );


	//! 得到刀光的中心点。
	void        get_glossyCenter( osVec3D& _center );

	/** \brief
	 *  删除刀光关联特效。
	 *
	 *  \param bool _glossy 如果此值为真，则删除刀光特效的设备相关数据
	 *  \param bool _effect 如果此值为真，则删除武器附加特效的相关数据
	 */
	void        delete_devdepRes( bool _glossy = true,bool _effect = true );

	//! 创建删除刀光关联特效的储备相关数据。
	bool        create_devdepRes( int _idx,bool _glossy = true,bool _effect = true,bool _topLayer = false );

	/** \brief
	 *  是否需要推进多个刀光位置.
	 *  
	 *  如果刀光附加的有特效,而此时又不显示刀光的话,则不需要推进多个刀光位置.
	 */
	bool        need_pushMultiSPos( void );


	//! 切换此武器特效到指定的索引
	bool        change_weaponEffect( int _idx );

	//! 缩放武器附加的特效.
	void        scale_weaponEffect( float _rate );




# if __GLOSSY_EDITOR__

	//! 得到使用刀光的动作索引。
	void        get_swordGlossyActIdx( int& _actNum,int* _actIdx,
											  DWORD* _color,char* _glossyTex );

	//! 设置当前glossy对应的纹理。
	void        set_glossyTex( const char* _tex );

	//! 设置osa特效自身的旋转.
	void        set_osaYRot( float _agl );

	//! 设置特效的缩放比例。
	float       set_effScale( int _idx,float _scale );

	//! 得到特效相关的信息.
	void        get_sgEffect( float& _pos,I_weaponEffect& _effFName );

	//! 得到附加的addPos.
	void        get_sgEffectPos( float& _pos,int _idx );

	//! 设置附加的特效位置
	void        set_sgEffectPos( float _pos,int _idx );


	//! 设置刀光拖尾的移动矩离
	void        set_sgMoveLength( int _len );
    

# endif 

	// 得到刀光拖尾的长度数据
	int         get_sgMoveLength( void ){ return m_ptrSwordGlossy->m_iGlossyMoveLength;} 

};





//! 使用这个关键字符来确认当前的bodyPartMesh是否是人物的武器部位。
# define  WEAPON_PARTCHAR   '7'
# define  WEAPON_PART8CHAR   '8'
# define  WEAPON_PARTID7  6
# define  WEAPON_PARTID8  7
# define  WEAPON_PARTOFFSET  8


/** \brief
 *  身体部位对应的顶点Mesh数据。
 *
 *  这个类放入osc_bodyFrame类中做为指针，渲染时需要从这个类中得到
 *  和skinAnimation相关的资源，比如matrix Palette.
 *  
 *  每一个bodyPartMesh应该对应固定的骨骼和frameID,所以这一部分也可以使用
 *  管理器，如果在内存中已经存在这一部分，只需要从内存取得指针，如果不存
 *  在，需要从硬盘上调入文件。
 *  
 *
 *  
 */
class osc_bodyPartMesh :public 	os_refObj
{

	//! bodyFrame类使用bpm来使用IndexedBlend的渲染方式。
	friend class    osc_bodyFrame;
	friend class    osc_skinMesh;

protected:

	/** \brief
	*  释放当前资源的纯虚函数，当当前的引用计数变为0时，自动调用这个
	*  函数。
	*/
	virtual void release_obj( DWORD _ptr = NULL );


public:
	
	//! 当前bpm对应的文件名。
	char                  m_strFname[MAX_BPMFNLEN];
	DWORD                 m_dwHash;

	//! 当前mesh对应的FrameId.
	DWORD					m_dwID;
	//! 当前mesh对应的父结点id.
	DWORD                 m_dwParentID;

	//! 有多少个palette和当前的mesh相联.
	DWORD					m_dwNumPalette;		// 5-17 

	//! 每一个顶点受几个骨骼的影响。
	DWORD                 m_dwNumInfluPV;


	//! 每一根骨骼对应的matrix.
	osMatrix 				m_arrBoneMatrix[MAX_BONEPERBPM];		// numPalette

	//! 对应的每一个骨骼id.
	DWORD                  m_arrBoneId[MAX_BONEPERBPM];

	DWORD					m_dwNumFaceInflunce;	// numFaceInflunce =   1,   2,   3,   4 		


	//! 当前mesh顶点对应的mesh fvf.
	DWORD                 m_dwFVF;
	//! 当前mesh顶点的vertex Size.
	DWORD                 m_dwVertexSize;

	//! 当前mesh中对应的indices的数目。
	DWORD                 m_dwNumIndices;
	//! mesh中顶点的数目。
	DWORD                 m_dwNumVertex;


	//! 对应在devdepResMgr中的mesh resource ID.
	int                     m_iDevdepResId;

	//! 当前bodyPartMesh是否处于使用状态中. 
	bool                    m_bInuse;

	//! 刀光用到的数据结构,如果不是武器的部分，此值为空。
	osc_swordGlossy*       m_ptrSwordGlossy;

	//! 是否需要渲染当前bpm,用于定位的盒子不需要渲染。
	BOOL                  m_bDisplay;
	//@{
	// Windy mod @ 2005-8-30 19:08:42
	//Desc: ride
	BOOL			 m_IsRideBox;
	//! 保存头一个顶点数据，用于给出坐骑所在的第二部分骨骼的索引。
	os_characterVer  m_RideVertData;
	osc_ChrRide *	 m_ptrRide;

	//@}

	//  River @ 2010-12-21:
	//! 释放时的计数，用于最近最少使用的算法.
	DWORD            m_dwReleaseCounter;


private:

	//@{
	//  用于IndexedBlend渲染方式的数据，支持Geforce级别的显卡。
	//! 网格的材质
	D3DMATERIAL9				  m_sMaterial;
	//@} 

	/** \brief
	 *  处理刀光数据的读入。
	 *
	 *  如果和当前的mesh名相同的*.gls文件存在，则数文件为刀光数据文件。
	 */
	void            read_weaponData( void );


public:
	osc_bodyPartMesh();
	~osc_bodyPartMesh();


	/** \brief
	 *  调入skinMesh相关的数据。
	 */
	//@{
	// Windy mod @ 2005-8-30 19:10:23
	//Desc: ride  _IsRide 为真表示该部位为坐骑定位盒子
	// River mod @ 2007-7-16:骑乘的怪物如果预先调入为非骑乘的怪物的话，必须重新调用
	bool        	 LoadMesh( char * _file ,bool _IsRide = false,int _ref = 0 );
	//!保存该部分的数据，就是它的第一个点的位置
	void			 SaveCenterPos(osVec3D _pos);
	//@}

	/** \brief
	 *  调入*.skn文件的顶点和索引数据。
	 *
	 *  此函数假设传入的两个缓冲区的指针已经分配数据，并且空间大小足够大。
	 *  这些调入的是阴影相关的数据。
	 *  
	 */
	bool           LoadMeshVNI( char* _file,BYTE* _vbuf,BYTE* _ibuf );


	/** \brief
	 *  得到设备相关数据的id.
	 */
	int             get_devdepResId( void )   { return m_iDevdepResId; }
	
	//! 得到当前mesh的fvf.不使用这个函数，我们使用自己的顶点声明。
	DWORD           get_meshFVF( void )       { return m_dwFVF; }

	//! 当前的BPM是否在使用中。
	bool            is_inUsed(  void );


	//! 当前的bpm是否是武器部分。
	bool             is_weaponPart( void );


	//! 得到武器特效的字符串.
	const I_weaponEffect*  get_weaponEffect( int _idx );


	//! river added @ 2010-12-21:真正释放
	void             release_bmpRes( void );

///////////////////////////////////////////////////////////////////////////////////////////////////////
	//  
	//  以下为编辑器相关的数据，最终的游戏渲染引擎不需要这些数据。
# if __GLOSSY_EDITOR__

	//! 用于创建刀光的临时数据。
	BYTE*           m_ptrVerData;

	//! 存储刀光相关的数据。
	void            save_glossyData( FILE* _file );


	//! 存储当前的bpm数据，并升级版本号，加入新的刀光数据。
	bool            save_weaponFile( void ) ;

	/** \brief
	*  设置第N个武器特效文件，最多4个。
	* 
	*  \param int _idx 设置第几个武器特效，从零开始，最大值为3.超过3的话，内部会变为零
	*  \param char* _effName 武器特效的文件名
	*/
	bool            set_weaponEffect( int _idx,const I_weaponEffect& _effName );

	//! 调整每一个刀光的长短。
	void            adjust_sowrdGlossySize( bool _start,float _size );

	//! 在X轴上旋转我们的刀光数据。
	void            rot_swordGlossyX( float _radian );
	void            rot_swordGlossyY( float _radian );

	//! 当前的刀光在X轴或Y轴上移动一定的向量。
	void            move_swordGlossy( bool _x,float _dis );

	//! 创建初始化的Sword Glossy.
	void            create_defaultGlossy( void );

	//! 设置此武器用于定位，不显示此武器。
	void            set_displayWeapon( bool _dis );

	//! 重设刀光的各种旋转角度，使完全回到调整前的刀光
	void            reset_swordGlossyPosRot( void );

	/** \brief
	 *  设置刀光对应的动作索引, 并且设置每一个动作对应的颜色数据。
	 *
	 *  \param _actIdx    表示哪一个动作需要显示刀光。
	 *  \param _arrColor  表示对应哪一个动作刀光的颜色。
	 */
	void            set_swordGlossyActIdx( int _actNum,int* _actIdx,DWORD* _color );
		                      

	// 设置刀光用到的特效。
	bool            set_sgEffect( float _pos,const I_weaponEffect& _effFName );

	/** brief
	 *  只显示某个身体部位的部分三角形,公用于调试目的
	 *
	 *  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
	 *  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
	 */
	virtual int        displaytri_bysel( int _triidx[24],int _trinum );


# else
	
	//! 保存头一个顶点数据，用于给出刀光所在骨骼的索引。
	os_characterVer  m_ptrVerData;

# endif 
///////////////////////////////////////////////////////////////////////////////////////////////////////


};

//! skin mesh数据的stl vector.
typedef std::vector<osc_bodyPartMesh>  VEC_bodyPartMesh;
//! bpm指针的stl vector.
typedef std::vector<osc_bodyPartMesh*>  VEC_bpmPtr;


/** \brief
 *  BodyPartMesh的管理器。
 *
 *  因为bodyPartMesh是从文件中创建，每一个bodyPartMesh又对应相同的骨骼和Frame,
 *  所以如果我们新调入的BodyPartMesh在内存中已经存在，则不需要调入，还省去了
 *  和骨骼相关联的过程。
 *  
 */
class osc_bpmMgr
{
private:
	VEC_bodyPartMesh        m_vecBPM;

	//! 设备相关数据的指针。
	LPDIRECT3DDEVICE9       m_pd3dDevice;

public:
	osc_bpmMgr();
	~osc_bpmMgr();

	//!  使用3d设备初始化当前的bpmMgr.
	void                  init_bmpMgr( LPDIRECT3DDEVICE9 _dev );

	//!  释放当前的bpmMgr
	void                  release_bpmMgr( void );

	//@{
	// Windy mod @ 2005-8-30 19:11:55
	//Desc:ride  从文件中调入bpm._IsRide 用于指定是否是坐骑部分，
	//如果是为真，这个数据由上层传入，
	int                   create_bpm( const char* _bpName,bool _IsRide = false );
	//@}

	//!  从一个id中得到Body part Mesh的指针。
	osc_bodyPartMesh*      get_bpmPtrFromId( int _id );


	//! 释放bpm的引用。
	void                  release_bpm( int _id );


};



//! 每一个三角形对应的邻接三角形索引。
struct  os_neighbour
{
	WORD   m_wN[3];
};



//! 用于全局的描边数据处理
class osc_fakeGlowData
{
private:
	//! 描边的长度
	float                      m_fLength;

public:
	//! 存储所有的bmp shadow内顶点的normal信息
	std::vector<osVec3D>       m_vec3Normal;
	int                        m_iNormalUseNum;

	//! 当前使用的描边宽度信息
	float                      m_fCurUseLength;

	//! 当前fakeGlow的颜色
	DWORD                      m_dwColor;

public:
	osc_fakeGlowData();

	osVec3D*       get_unUseNormlDataStart( void );
	void           set_useNormalNum( int _num );

	void           resetNormal( void );

	void           set_fakeGlowData( DWORD _color,float _length );
	void           frame_move( void );

};


//  
//! 声明中间管道
class osc_middlePipe;

//! 每一个mesh最多可以有的边数目
# define MAX_EDGE_PERMESH  4096

/** \brief
 *  阴影相关的数据结构。
 *
 */
class osc_bpmShadow
{
public:
	//! 当前bpm使用的顶点数据
	os_characterVer*      m_ptrBpmVer;
	int                  m_iVerNum;

	//! River @ 2009-6-23:后期优化此处，做一个好的内存池，来一次性初始
	//                    化所有的阴影系统用到的内存
	//! 每一帧顶点使用骨骼矩阵转化后得到的顶点,只需要位置就可以了
	osVec3D*             m_ptrBpmVerAT;


	//! 当前bpm使用的索引数据。
	WORD*                m_ptrBpmIdx;
	int                  m_iIdxNum;

	static osc_fakeGlowData*     m_ptrFakeGlowData;

private:
	//! 每一个模型的三角形对应一个bool值，表示此三角形是否面对着方向光。
	bool*                m_bFront;

	//! 每一个三角形应该对应一个neightbour列表.
	os_neighbour*         m_arrNeiList;

	//!  要渲染的边的列表。
	WORD*                m_arrEdgeList;
	int                  m_iEdgeNum;


	//! 当前的mesh对应的顶点缓冲区,此顶点缓冲区只接受osVec3D类型数据.
	LPDIRECT3DVERTEXBUFFER9    m_pShadowVBuf;
	int                        m_iShadowVerNum;
	//! 用于人物身上对着方向光的三角形索引，ZFAIL　STENCIL必须渲染，不然山挡住人物时，可以看到人物的阴影。
	LPDIRECT3DINDEXBUFFER9     m_pSourceIdxBuf;
	int                        m_iSourceTriNum;

	//! 静态的3dDevice指针
	static LPDIRECT3DDEVICE9        m_pd3dDevice;
	static LPDIRECT3DVERTEXBUFFER9  m_pVBufShadowSqu;


	//@{ 人物描边相关的数据
	//! 全局的，用于填充描边数据的dynamic vb idx.
	static int                      m_iDynamicVBIdx;
	static osc_middlePipe*          m_ptrMiddlePipe;
	static os_polygon               m_sRenderPolygon;
	//@} 


	//@{
	//! 渲染阴影时用到的渲染状态块索引
	static int                    m_iSHDrawState;
	static int                    m_iSHDrawRestoreState;
	static int                    m_iShadowSquareState;
	static int                    m_iShadowSSRestore;

	/** \brief
	 *  设置渲染状态，使渲染的时候，只渲染几何信息到到zbuffer,不显示。
	 * 
	 *  有阴影的武器需要先渲染武器的zbuffer,用于使用武器上没有阴影的效果。
	 */
	static int                    m_iOnlyZbufSB;
	static int                    m_iRestoreOnlyZbufSB;

	//@}

	//! 屏幕宽度，屏幕高度
	static float                    m_fScrWidth;
	static float                    m_fScrHeight;


	//! 当前的shadowVolume是否更新过。
	bool                         m_bUpdated;

	//! 灯光的方向，默认使用全局光的方向，
	float                m_fShadowProjLength;


	//! 当前的bpm是否需要描边效果
	bool                 m_bFakeGlow;
	osVec3D*             m_vec3Normal;


private:
	//! 根据传入的矩阵队列更新模型中每一个顶点的位置。
	void                update_verPos( osMatrix* _matArr,int _matNum );

	//! 得到模型中每一个三角形与方向光的关系，面对或是背对，
	void                cal_triToLight( osVec3D* _lightDir = NULL );

	//! 往边列表中加入一条边。
	void                push_edge( WORD _verIdxs,WORD _verIdxe );

	//! 处理得到产生阴影的模型边。
	void                get_shadowEdge( void );


	//! 把模型中的阴影边缘数据转化为顶点数据，并填充到顶点缓冲区中。
	void                get_shadowVertexData( void );

	//! fadeGlow相关，把人物的边数据填充到顶点缓冲区内。
	void                get_edgeVertexData( osVec3D& _vpVec );


	//! 每一帧开始时，重设当前的shadowVolume.
	void                reset_shadowVolume( void );

	//! 录制渲染阴影需要的渲染状态块。
	static void         record_ShadowVolumeStateBlock( void );
	static void         record_ShadowSquareStateBlock( void );

	//! 填充屏幕多边形的几何数据
	static bool         fill_screenSquareVB( void );

public:
	osc_bpmShadow();
	~osc_bpmShadow();

	//! 初始化阴影数据结构。　
	bool                init_shadowData( const char* _fname );


	//! 释放当前的阴影相关数据。
	void                release_shadowData( void );

	/** \brief
	 *  每一帧对阴影数据的更新
	 *
	 */
	void                update_shadow( osMatrix* _matArr,int _matNum );

	//! 开始画ShadowVolume.
	static void         start_shadowVolumeDraw( void );
	static void         end_shadowVolumeDraw( void );


	//! Shadow渲染的接口。
	bool                draw_shadowVolume( void );

	/* \brief
	 *  画fadeGlow的接口，描边效果
	 * \param DWORD _color        要画的fadeGlow的颜色数据
	 * \param DWORD _colorNoise   颜色数据的随机数据值,在颜色的基础，加入此随项随机值
	 * \param osVec3D& _vpVec     从某一个方向描边，即阴影数据时，灯光的方向。
	 * \param float _length       描边数据得到后，往外扩散的边缘数据的长度。
	 * \param float _lenthNoise   边缘数据长度的随机值。
	 * \param bool _projParallel  如果此值为真，则顺着投射方向往前画描边数据，否则垂直画。
	 */
	bool                draw_fakeGlow( DWORD _color,DWORD _colorNoise,
		                  osVec3D& _vpVec,float _length,
		                  float _lenthNoise,bool _projParallel );


	//! 在屏幕上画我们的StencilSquare.
	static void          draw_shadowSquare( void );


	//! 全局的初始化我们的阴影系统。
	static void          init_shadowSys( osc_middlePipe* _mp );	

	//! 重设全局的阴影透明度
	static void          reset_shadowColor( float _alpha );


	//! 全局的释放阴影系统.
	static void          release_shadowSys( bool _frel );

	//! 设置只渲染zBuffer的设备状态。
	static void         set_onlyZbufStateBlock( void );

	bool                is_updated( void ) { return m_bUpdated; } 
	void                frame_setShadow( void ) { m_bUpdated = false; } 

	//! 计算Shadow应该投射的阴影长度.
	void                set_shadowProjLength( float _length ) 
	{
		osassertex( _length >= -0.01f,va( "The length is:<%f>...\n",_length ) );
		// 加长2.0f,确认阴影不会出问题
		// River @ 2009-5-14:无需加长，程序自会加长。
		m_fShadowProjLength = _length;
	}

	//! 重设fake glow
	void                set_fakeGlow( bool _glow );

	//! 设备的reset相关函数.
	void                shadow_onLostDevice( void );
	void                shadow_onResetDevice( void );

};
//!人物阴影体调试FX文件 
#define HDRFX_FILENAME      "effect\\shadow.fx"




//! 人物第二层特效纹理使用的数据结构。
struct os_chaSecEffect					
{
	//! 第二层贴图的shader方式。
	/*  有以下的EffectType:
    5:  环境纹理.
	6:  流动纹理.
    7:  自发光纹理.
	8:  流动与环境相结合纹理
	9:  流动与自发光相结合纹理.
	10: 环境与自发光相结合纹理.
	*/
	WORD      m_wEffectType;

	//@{
	//! 第二层贴图矩阵用到的数据。
	float     m_fRotSpeed;
	osVec3D   m_vec3RotAxis;
	float     m_fUMoveSpeed;
	float     m_fVMoveSpeed;
	//@} 


	//! 第二层贴图的id.
	int       m_iSecTexId;

	//! 整体shader的id.
	int       m_iShaderId;

	//@{
	//! 在运行过程中需要的数据。
	osMatrix  m_matFinal;

	/** \brief
	 *  纹理从开始移动到现在过的时间。
	 */
	float     m_fTexmoveTime;
	//@} 


	//! 第二层纹理的贴图名字
	char      m_szSecTexName[64];

	int		m_iThdId;
	char    m_szThdTexName[64];

	// River @ 2010-3-11:
	int     m_iFourthId;
	char    m_szFourthTexName[64];
	


# if __CHARACTER_EFFEDITOR__

	//! 存储当前的effect数据,文件需要自动获得
	void      save_chaSecEffToFile( const char* _cname );
# endif 

public:
	os_chaSecEffect();
	~os_chaSecEffect();

	//! 每一帧计算当前贴图特效的内部数据。
	void      frame_moveSecEff( void );

	//! 删除设备相关的数据
	void      delete_devdepRes( void );

};


//! 人物第二层纹理贴图的预调入，不算是预调入，只能算是调入之后，内存缓存了一份。
struct os_secEffPreLoad
{
	char   m_szSecFile[48];
	DWORD  m_dwHash;

	os_chaSecEffect  m_sCharSecEffect;
};

typedef std::vector<os_secEffPreLoad>  VEC_secEffProLoad;


//! 人物贴图shader文件的缓存.如果shaderRef为零时，不删除，放入此结构。
struct os_chrShaderBuf
{
	// 因为数据结构的关系，不记录shader名字。
	//char   m_szShaderName[64];
	DWORD  m_dwHashId;

	int    m_iShaderId;
	DWORD  m_dwReleaseCount;

	os_chrShaderBuf() 
	{
		//m_szShaderName[0] = NULL;
		m_dwHashId = 0xffffffff;
		m_iShaderId = -1;
		m_dwReleaseCount = 0xffffffff;
	}

	//! 释放shader.
	void   reset( void );

};

typedef std::vector<os_chrShaderBuf>  VEC_chrShaderBuf;


/** \brief
 *  bodyPartMeshInstance的实例，每次调入一个bodypartMesh时，skinMesh结构得到这样一个实例。
 *
 *  这个类存储了每一个身体部位动画的动画数据，使用这个结构，使多个人物和共用一套数据成为可能，
 *  他们可能处于不同的动作阶段。所以每一个Instance使用一套独立的动画数据。
 *  
 *  shader和特效相关的数据，在这个类的内部处理。
 */
class osc_bpmInstance
{

	friend class             osc_skinMesh;
private:

	//! 当前实例指向的bodyPartMesh数据的指针。
	osc_bodyPartMesh*         m_ptrBodyPartMesh;

	//! River @ 2010-12-28:当前bpm ins用到纹理的hash ID.
	DWORD                     m_dwHashId;

	//! 当前bodyPartMesh在他的mgr中的id.
	int                      m_iBpmId;

	//! 当前的bodyPartMesh是否已经和骨骼关联起来了。
	bool                     m_bLinkBones;
	
	//! 存储每一个根骨骼的offsetMatrix.
	osMatrix*			    	m_ptrBoneOffset[MAX_BONEPERBPM];	

	osVec3D*					m_ptrBonePosOffset[MAX_BONEPERBPM];
	osQuat*						m_ptrBoneRotOffset[MAX_BONEPERBPM];

	//@{ 
	//  ATTENTION:
	//  在此存储和渲染阴影相关的数据,bodyFrame使用的这个类实例的指针，
	//  所以渲染bodyFrame时可以得到此处的阴影数据

	//! 如果此值不为空，则渲染时，需要渲染阴影相关的数据。
	osc_bpmShadow*            m_ptrShadow;

	// 
	//@}

	/** \brief
	 *  当前的Instance对应是否是纹理多层渲染.
	 *  
	 *  如果此值不为空，则指定了一个多层渲染的数据结构。
	 */
	os_chaSecEffect*          m_ptrSecEffect;

	//! bpm纹理相关的shaderId。
	int                      m_iShaderID;
	//! 当前的bodyPartMeshInstance是否是透明的shader.
	bool                     m_bAlpha;


	//! River @ 2010-7-1:是否处于隐藏状态.
	bool                     m_bHide;


	//@{
	//! shaderBuf数据相关。
	int                      get_freeBuf( void );
	void                     release_insShader( int _shaderId,bool _finalRelease = false );
	static int               get_bufShaderId( DWORD _hashId );
	//@} 


private:

	//! 得到一个实例化的指针.
	osc_swordGlossyIns*                   get_glossyInsPtr( void );

	int                                   m_iNodeIdx;

	//! 得到一个坐骑的实例化数据
	osc_ChrRideInst*                      get_chrRidePtr( void );
	int                                   m_iCRNodeIdx;


public:
	//! river added @ 2010-12-22: 加入一次性的内存分配
	static CSpaceMgr<osc_swordGlossyIns>  m_vecDGlossyIns;

	//! River @ 2010-12-24:预分配坐骑相关的数据指针。
	static CSpaceMgr<osc_ChrRideInst>     m_vecChrRideIns;

	//! River @ 2010-12-28:shader的缓冲数据 
	static VEC_chrShaderBuf               m_vecChrShaderBuf;


	//! 如果有刀光，创建刀光相关的实例数据。
	osc_swordGlossyIns*        m_ptrSGIns;
	//@{
	// Windy mod @ 2005-8-30 19:12:57
	//Desc: ride坐骑实例数据
	osc_ChrRideInst *				m_ptrRideIns;
	//@}


	//! 用于geforce级显卡的优化。每一个bpmInstance应有单独的shaderId和texture
	LPDIRECT3DTEXTURE9		   m_ptrBpmTexture;


# if __GLOSSY_EDITOR__
	//! 在调入人物数据时,初始化bpsInstance相关的数据。
	void                   init_swordGlossy( void );

	/** brief
	 *  只显示某个身体部位的部分三角形,公用于调试目的
	 *
	 *  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
	 *  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
	 */
	virtual int        displaytri_bysel( int _triidx[24],int _trinum );

# endif

	//! 设置第二层纹理的id.
	void                   set_secTexid( int _id );
	//! 设置人物第三层纹理，即第二层纹理通道
	void					set_thdTexid( int _id );

public:
	osc_bpmInstance();
	~osc_bpmInstance();


	/** \brief
	 *  使用这个函数调入bodyPartMesh,osc_skinMesh类使用这个接口来操作bodyPartMesh相关的操作。
	 *
	 *  使用bpm的Manager来得到bodypartMesh的指针。
	 *  \param _playSg 是否播放刀光
	 */
	bool        	        loadBpmIns( char *  _file,bool _playSg,bool _playWEff,
		                         bool _shadowSupport = false,bool _IsRide = false,
								 bool _topLayer = false,osc_skinMesh* _mesh = NULL );

	/** \brief
	 *  释放当前的bpmInstance.
	 */
	void                   release_bpmIns( void );


	osc_bodyPartMesh*       get_bpmPtr( void )       {  return m_ptrBodyPartMesh; }
	osMatrix**              get_bpmBoneOffset( void ) {  return m_ptrBoneOffset;  }
	osVec3D**				get_bpmBonePosOffset(){return m_ptrBonePosOffset;}
	osQuat**				get_bpmBoneRotOffset(){return m_ptrBoneRotOffset;}

	//! 得以bpmInstance的面数.
	int                    get_faceNum( void );

	//! 设置是否显示刀光。
	DWORD                  set_disSGlossy( int _idx );

	//! 当前动作索引是否是对应刀光的动作。
	bool                   is_glossyAction( int _id );

	//! 得到当前刀光的中心点。
	void                   get_glossyCenter( osVec3D& _center );

	//! 得到shadow相关的指针。
	osc_bpmShadow*          get_shadowPtr( void ) { return m_ptrShadow; } 


	//! 当前BodyPartMesh实例是否使用刀光。
	bool                   is_glossySM( void );

	//@{
	//! 得到和设置第二层贴图的id.
	int                    get_secTexid( void );
	int                    get_shaderId( void );
	bool                   has_secondEffect( void );
	bool                   is_rottexEffect( void );
	bool                   is_shineTexEffect( void );

	//! river added @ 2010-3-11:更多种类的贴图.
	bool                   is_rotEnvEffect( void );
	bool                   is_rotShineEffect( void );
	bool                   is_envShineEffect( void );


	osMatrix*               get_texTransMat( void ); 

	int						get_ThdTexid(void);

	//! River @ 2010-3-11:装备内第四张纹理的ID.
	int                     get_fourthTexid( void );
	//@} 

	//! 当前body part mesh是否有alpha Channel
	bool            has_alphaChannel( void )   { return m_bAlpha; }


	//!  Device reset相关.
	void            bpmins_onLostDevice( void );
	void            bpmins_onResetDevice( void );



	//! 缩放武器的特效数据
	void            scale_weaponEffect( float _rate );

	//! 是否隐藏.
	bool            get_hideState( void ) { return m_bHide; }


};

typedef std::vector< osc_bpmInstance*>    VEC_bpmInsPtr;




//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                     骨骼动画的层级关系相关类
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class osc_skinMesh;


/** \brief
 *  渲染bodyFrame用到的结构.
 */
struct os_rsBodyFrame
{
	LPDIRECT3DDEVICE9   m_pd3dDevice;
	BOOL               m_bAlpha;
	osc_bodyPartMesh*   m_ptrPM;
	osc_bpmInstance*    m_ptrPMIns;
	os_smMeshDevdep*    m_ptrRes;
	float              m_fAlphaVal;

	//! 用于IndexedBlend渲染。
	osMatrix*          m_ptrBoneMatrix;
};



/** \brief
 *  处理身体层级关系的类。
 *
 *  这个类需要从*.bfs文件中得到初始化信息，然后再和
 *  *.act调入的数据合并成最终的骨骼动画数据。
 *  
 */
class osc_bodyFrame
{
	friend class osc_skinMesh;
	friend class osc_bodyFrame;

private:

	//! 当前Frame对应的id.
	DWORD              m_dwID;             

	//! 当前骨骼自身的矩阵
	osMatrix           m_matRot;
	//! 当前frame的最终矩阵
	osMatrix           m_matCombined;	

	//!  子兄弟结点.
	osc_bodyFrame *     m_pSibling;
	//!  第一子结点
	osc_bodyFrame *     m_pFirstChild;

	//! skinMesh对应的顶点数据。
   VEC_bpmInsPtr       m_vecPtrBpmIns;

   //! 当前bodyFrame对应的几何结点(BODY PART MESH)的数目
   int                m_iBMNum;

	//! 当前bodyFrame对应的parent skin.
	osc_skinMesh*       m_pParentSkin;

	//! River added @ 2009-5-31:用于处理上层得到的骨骼位置数据。
	osMatrix           m_matBonePos;

private:
	//! 设置effect Shader的constant数据。
	void               set_shaderConstant( LPD3DXEFFECT _eff );


public:
	osc_bodyFrame(DWORD _id = 0);
	~osc_bodyFrame();

	
	void          set_id( DWORD _id )   { this->m_dwID = _id; }

	osc_bodyFrame *FindFrame (DWORD _dwID);
	HRESULT       InvalidateDeviceObjects();
	
	//
	// 对当前的Frame加入下一个层级。
	void          AddFrame(osc_bodyFrame* _pframe);

	/** \brief
	 *  骨骼动画的渲染函数。
	 *
	 *  由这个函数真正的调用dip,渲染骨骼动画。
	 *
	 *  \param _alpha 是否是画透明的alpha片.
	 *  \param _fade  是否处于淡入淡出状态的渲染，这种状态下，非alpha结点需要象
	 *                alpha结点一样得到渲染。
	 */
	bool	      Render( LPDIRECT3DDEVICE9 _dev,
		           bool _alpha,bool _fade = false,float _alphaVal = 1.0f );

	//! 渲染深度信息
	bool	      RenderDepth( LPDIRECT3DDEVICE9 _dev );
	

	/** \brief
	 *  对刀光的渲染，使用单独的一个函数。
	 *
	 *  \param bool _gdraw 是否渲染刀光，针对只渲染武器特效，不渲染刀光的情形
	 */
	bool         render_swordGlossy( LPDIRECT3DDEVICE9 _dev,bool _gdraw );

	/** \brief
	 *  加入刀光的关键帧.
	 *
	 *  \brief  返回刀光ins拖尾的长度
	 */
	int          add_swordGlossyKey( int& _addNum );


	/** \brief
	 *  使用d3dIndexedBlend的时候,使用这个函数对skinMesh进行渲染.
	 *
	 *  以后会增加其它的渲染方式,以更加合理的利用硬件功能.
	 */
	bool          RenderIndexBlend( os_rsBodyFrame& _rbf );


	/** \brief
	 *  使用设备相关数据的id加入新的设备相关数据。
	 */
	void          AddMesh( const osc_bpmInstance* _pm );

	/** \brief
	 *  把Frame中对应的mesh使用新的bpMesh替换。
	 *
	 *  \param  _npm 要替换成的新的part mesh Instance.
	 */
	void          change_Mesh( osc_bpmInstance* _pm,
		                      const osc_bpmInstance* _npm );


	void          LinkBones(osc_bodyFrame * _root);

	/** \brief
	 *  使用矩阵的层级关系去更新当前的Frame.
	 */
	void          UpdateFrames(osMatrix & _matCur,bool _catchBoneMat = false );

	/** \brief
	 *  重置当前的frame为初始状态，释放占用的资源。
	 */
	void          Reset_Frame( void );
	//@{
	// Windy mod @ 2005-8-30 19:13:35
	//Desc: ride 用于计算第二部分邦定盒的位置，该位置用于放置人物。以实现坐骑
	void		  UpdateRideBoxPos();
	//@}

	//! TEST CODE:用于遍历层级.
	void        through( int& _hier,os_boneSys* _boneSys,bool _sibling = true );



};

typedef osc_bodyFrame*    osc_bodyFramefPtr;



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                          上层可操作的接口和skinMesh的管理器类
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class    osc_skinMeshMgr;

//! 根据每一帧人物的旋转来计算人物的详细包围盒位置,用于处理非中心点包围盒的问题.
struct osc_smOffsetAdjBBox
{
	//! 人物的包围盒相当于原点的位移
	osVec2D    m_vec2Offset;

	//! 初始化的时候,设置一些smBBox,然后每一帧根据人物的旋转,来得到新的bboxMin和max.
	//! 再得到移动到原点的MinX和maxX
	void       set_smOffsetAdjBBox( osVec3D& _min,osVec3D& _max,osVec3D& _minOut,osVec3D& _maxOut );


	void       get_smOffsetAdjBBox( osVec3D& _vecOut,float _rotAngl );

};


//! 武器对应的身体部位。
# define WEAPON_BPMID       6

/** \brief
 *  最上层的控制类，由这个类控制整个的骨骼动画显示。
 *
 *  由这个类向上层提供所有使用骨骼物体的动画。
 *  包括当前动画的帧数位置信息等。
 *  这个类从下层的manager中得到可重用的信息。
 *  
 */
class osc_skinMesh : public I_skinMeshObj
{

	friend   class     osc_skinMeshMgr;
	friend   class     osc_bodyFrame;

# if __ALLCHR_SIZEADJ__
public:
	static osVec3D     m_vec3ScaleAll;
# endif 

private:

	//! Mgr内管理skinMesh的内存管理器指针，为了使调用release_character
	//! 接口的时候，可以对管理器做出处理
	static CSpaceMgr<osc_skinMesh>*  m_ptrVecDadded;
	static CSpaceMgr<osc_skinMesh>*  m_ptrSceneSM;

	char               m_strSMDir[MAX_BPMFNLEN];

	//! ATTENTION:优化此处为静态分配的内存。
	osc_bodyFrame*	    m_pRoot;

	//! 记录整个树结构的frame指针，在处理frame层级关系时记录。
	osc_bodyFramefPtr	m_pframeToAnimate[MAX_FRAMENUM];


	//! 当前skinMesh的外观数据。
	os_bodyLook			m_Look;

	//! 当前skinMesh用到的bone系统。
	os_boneSys*        m_ptrBoneSys;

	//! 当前skinMesh用到的bone动画数据。
	os_boneAni*         m_ptrBoneAni;


	//! 当前骨骼动画对应bodyPartMesh的实例数据,定义使用最大数量.
	osc_bpmInstance     m_vecBpmIns[MAX_SKINPART];	

	//! 当前骨骼动画的mesh数目。
	int                m_iNumParts;

	//! 当前的skinMesh是否在使用中.
	bool               m_bInUse;


	//! 当前的skinMesh是否可见。
	bool               m_bInView;

	/** \brief
	 *  当前skinMesh渲染了刀光
	 *  
	 *  因为skinMesh渲染刀光需要多次更新人物的动作，使用这个变量来记录是否
	 *  渲染刀光，在人物的阴影渲染中，重新计算人物的动作。
	 * 
	 *  这个变量保证了在一帧中,只渲染一次武器对应的刀光.
	 *  
	 */
	bool               m_bRenderGlossy;



	//! 当前的skinMesh中是否有AlphaPart,如果有的话，需要第二次进行渲染。
	bool              m_bHasAlphaPart;


	//! 画当前的skinMesh的透明部分的时候,是否需要写Zbuffer
	bool              m_bWriteZbuf;



	//=================================================================================
	// 以下人物动作播放时需要和保持的数据。
	// ATTENTION TO FIX: 把这些数据分成一个单独的类，在此处加入动作切换的数据
	//                   如果把动作分开了，可以把skinMesh也做成是可以共享资源的类？？？
	//                   
	//@{
	//! 当前动作的id.
	int               m_iAniId;

	//! 当前动作名字的索引,一个动作名字可能对应多个动作。　
	int               m_iAniNameIdx;


	//! 变换动作时，备份的上一次动作的id.
	int               m_iLastActId;


	/** \brief
	 *  上一帧播放完成时余下的时间。
	 *  
	 *  在到达下一帧之前，这个值一直递增，到达下一帧的时候，
	 *　此值置零，然后再重新计算。
	 */
	float				m_fLastUsedTime;
    //! 当前动作所在的帧数,用于改变动作的时候做插值
	WORD				m_wAniIndex;
	//! 如果动作更换，则使用当前动作所在的帧跟新动作的第零帧进行插值，此数据保存当前的动作。
	WORD              m_wLastAniIndex;


	//@{
	//! 做"负动作"时，用到动作和插值数据。
	float             m_fNegLastUsedTime;
	int               m_iNegAniIndex;
	//@} 



	/** \brief
	 *   当前是否处理动作转换的状态，
	 *
	 *   处于这个状态时，人物需要从当前动作到要转化的动作的第一帧做出插值。
	 *   目前使用矩阵的线性插值，高效但效果普通。这个状态持续的时间为
	 *   ACT_CHANGESTATETIME,在对动作进行FrameMove处理时，如果动作处于当前
	 *   的状态，则需要使用另外的函数，进行特殊处理。
	 */
	bool              m_bChangeActState;




	//!  最后一次动作切换的结构，每一套动作播放完成时需要根据
	//!  这个结构做出相应的动作。
	os_newAction       m_actChange;


	//! 是否定格当前动作。
	bool              m_bFreezeAct;

	//! river added @ 2009-8-26: 是否隐藏实时的人物阴影.
	bool              m_bHideRealShadow;

	//@{
	//
	//  调入skinMesh
	//! 当前播放动画使用的帧数。
	DWORD				m_dwFPS;
	//! 每一帧用到的时间。
	float              m_fTPF;
	//! 上层变化动作时,使用的动作速度信息.
	float              m_fNewActSpeed;
	//
	//@}

	/** \brief
	 *  当前处于两个动作间的插值向量。
	 *
	 *  当动作改变的时候，使用这个值，可以使动作改变的更加柔和.
	 */
	float             m_fActLerpVal;

	/** \brief
	 *  动作切换时，是否使用最后一帧和第一帧进行插值。
	 *
	 *  默认情况下，都需要使用插值。
	 *
	 *  不插值是因为一种特殊的情况，即最后一帧和第一帧的位置差别很大，比如主角从树上跳下
	 *  来的情况,跳下来以后，马上开始作另外的一个动作，这种情况下，我们不能让主角树上的
	 *  第一帧动作和树下的最后一帧动作插件后再和另一个动作的第一帧进行插值。
	 *  
	 */
	bool              m_bLerpLastFAndFirstF;


	//@}
	//=================================================================================


	//! 动作是否处于停止状态,变换动作时，可以让新的动作运行到停止帧索引时，停止动作播放。
	bool              m_bActStopStatus;
	//! 动作是否到达当前动作的关键停止帧。
	bool              m_bArriveStopFrame;

	/** \brief
	 *  手部部位的位置或是武器上刀光关键点的位置，用于放置手部或是武器部位的特效。
	 *
	 *  只要武器有刀光，此顶点位置就起作用。
	 */
	osVec3D           m_vec3HandPos;

	//! 使用多层旗帜贴图的身体部位索引
	int               m_iMlFlagIdx;

	//! 2009-6-7: 当前是否还没有调入
	bool              m_bLoadLock;

	//! post trail effect
	static CPostTrailEff	  sm_postTrailEff;

	//! former render target(non-postTrail surface)
	static LPDIRECT3DSURFACE9 sm_formerSurBeforePostTrail;

	//! is former render target?
	static BOOL				  sm_bIsFormerSurBeforePostTrail;

	//! is post trail?
	bool						m_bPostTrailState;

	//! exclude the light effect...
	bool						m_bExcludeLightState;

	//! 当前人物是否用在界面上?
	bool                        m_bUseInInterface;

	//! River @ 2010-1-7:用于武器附加特效的缩放
	float                       m_fBaseSkinScale;

	//! River @ 2010-7-1:人物动作结束时，是否显示武器.
	bool                        m_bDisWeaponActionOver;

	//! River @ 2011-2-25:当前人物是否处于删除状态，用于多线程不出错.
	bool                        m_bToBeRelease;
	//! River @ 2011-2-25:当前人物是否在调入中处于标志优先状态。
	int                         m_iVipMTLoadLevel;

	//! 是否下一帧必须更新.多线程创建刀光特效，有可能刀光创建完的时候，
	//! 人物也frameMove完了，永远没有机会再进入视野了。
	bool                         m_bMustUpdateNextFrame;
public:

	//! enable post trail state
	virtual void			enable_postTrail(bool _enabled = true);

	//! exclude light effect
	virtual void			exclude_light(bool _exclude = true);

	//! is exclude light effect 
	virtual bool			is_excludeLight(void);


	//!渲染人物的Post trail 
	static void					start_renderPostTrail(osc_skinMesh* _ptrSkinMesh);
	static void					end_renderPostTrail(bool _alpha);
	static void					clear_postTrail(void);
	static void					init_postTrail(void);
	static void					restore_postTrail(void);
	static void					release_postTrail(void);

	//! is post trail enabled?
	bool					    is_postTrailEnabled(void)const{return m_bPostTrailState;}

	//! 得到当前的char是否要release.
	bool                        get_toBeRelease( void ) { return m_bToBeRelease; } 

	//! 得到当前char的vip load level.
	int                         get_mtLoadLevel( void ) { return m_iVipMTLoadLevel; } 

public:
	//! 用于flipDeterminant的Scale矩阵.
	static osMatrix   m_sFDMatrix;

	//!  IdentityMatrix.
	static osMatrix   m_smatIdentity;

	//! 替代当前场景环境光的光,仅用于当前的skinMesh.
	osColor           m_sCurSkinMeshAmbi;
	//! River @ 2007-7-7:
	//! 当前的人物，是否被设置过更亮的光，用于天气切换时，人物的高亮可以保存。
	bool              m_bSetHighLight;

	//! 人物的第二层贴图指针修改为内存池模式
	static CSpaceMgr<os_chaSecEffect> m_vecSecEffect;

	os_chaSecEffect*  get_sceEffectPtr( void );
	static void       release_secEffectPtr( os_chaSecEffect* _ptr );

	//! 人物的第二层贴图数据也使用预调入的形式
	static VEC_secEffProLoad  m_vecSecEffPreLoad;
	static int                m_iSecEffPLNum;

private:

	/** 
	 *  人物当前的位置
	 */
	osVec3D            m_vec3SMPos;

	/** \brief
	 *  当前人物的scaleVector.
	 */
	osVec3D 			m_vScale;



	//@{
	//  River added @ 2004-4-22: 人物旋转时，加入插值。
	float             m_fLastRotAgl;
	float             m_fInterpolationTime;
	bool              m_bRotInterpolateStatus;
	//@}
	//! 人物当前的旋转方向,单位是弧度。
	float              m_fRotAgl;

	//! 渲染时人物的旋转方向，为最终人物的旋转方向。
	float              m_fFinalRotAgl;


	//! 渲染时是否渲染当前的人物。
	bool               m_bHideSm;


	//! 当前人物的基准Bounding Box.
	os_bbox              m_sBBox;
	//! 当包围盒不在原点的时候,需要使用这个调整包围盒来处理一些包围盒的位移
	osc_smOffsetAdjBBox  m_sAdjBBox;

	//! 当前人物的Bounding Sphere半径。
	float              m_fBSRadius;

	//! 当前人物的微调位置。
	osVec3D            m_vec3AdjPos;

	//! 当前人物的微调旋转。
	float              m_fAdjRgl;

	//! 角色当前是否可以播放刀光,默不播放刀光.
	bool               m_bPlaySGlossy;
	//! 要播放刀光的颜色
	DWORD              m_dwFinalGlossyColor;
	//! 当前角色是否播放武器附加的特效
	bool               m_bPlayWeaponEffect;

	//! 是否正在多线程创建刀光.
	int                m_iMT_PlayWeaponEffect;

	//@{
	//! 在隐藏人物的时候,备份人物的刀光和武器特效显示效果
	bool               m_bPlaySGlossyBack;
	bool               m_bPlayWeaponEffectBack;
	//@}


	//! 当前人物的动作是否正处于Suspend状态。
	bool                    m_bSuspendState;


# if __GLOSSY_EDITOR__

	//! 人物身上武器的bodyPartMesh.
	osc_bodyPartMesh*       m_ptrWeaponBpm;

	//! 刀光的实例数据。
	osc_swordGlossyIns*     m_ptrSGIns; 


	//@{
	// Windy mod @ 2005-10-10 12:04:21
	//Desc: 加入身体第8部分支持，包括编辑器的支持
	//! 人物身上武器的第8部分bodyPartMesh.
	osc_bodyPartMesh*       m_ptrWeaponBpm8;
	//! 第8部分刀光的实例数据。
	osc_swordGlossyIns*     m_ptrSGIns8; 


# endif 

	//@}


	/** \brief
	 *  对应人物身体BodyPartMesh的实例.
	 *
	 *  这个实例中有刀光的实例数据。　如果为空，则没有刀光。
	 *
	 *  River @ 2005-8-16: 
	 *  如果第八个身体部位可以使用刀光，则需要修改多处刀光相关的变量.
	 *  第八个身体部位如果加入刀光，需要额外的工作量，所以开始可以只
	 *  在身体内加入第八个身体部位，接下来才编辑第八个身体部位的刀光。
	 *  比引擎支持多个阴影的复杂度要高。
	 */
	osc_bpmInstance*        m_ptrSGBpmIns;
	//第8部分刀光的实例数据
	osc_bpmInstance*        m_ptrSGBpmIns8;
	


	//! 如果当前的skinMesh处于Fade状态，此值表示此sm的alpha信息。
	float                  m_fAlphaVal;
	//! 当前是否处于fade状态,目前的人物半透明以后，会挡住花草的渲染
	bool                   m_bFadeState;

	/** \brief
	 *  当前的skinMesh物品是否需要渲染阴影
	 *
	 *  目前的阴影渲染大量的使用cpu运算和显卡填充率，所以只有主角人物
	 *  可以使用阴影。使用shadowSupport的skinMesh数据需要更多的内存来
	 *  存储相关的数据。
	 */
	bool                   m_bShadowSupport;


	/** \brief
	 *  当前的skinMesh是否把方向光做为环境光
	 *  
	 *  主要用于场景中的草等透明物品，不出现明暗面。
	 */
	bool                   m_bUseDirLAsAmbient;

	//! 此skinMesh的上一帧是否在视域内,主要用于刀光和刀光特效更新的处理
	bool                   m_bInViewLastFrame;

	//! 设备相关数据的指针。
	static LPDIRECT3DDEVICE9       m_pd3dDevice;

	//char	mszspeFileName[MAX_PATH];
	s_string	mszSpeFileName;

	//! River mod @ 2008-11-21:处理skinMesh包围盒子的位置移动和旋转,非中心位置的包围盒,不使用此代码
	//  处理,会有明显的Bug发生
	osVec2D                        m_vec2BBoxOffset;
	void                           process_bboxUpdata( void );

	//! River @ 2009-5-7:用于画charScrRect功能。
	DWORD                   m_dwLastRenderTime;


	/*
	 * River @ 2009-6-23:人物的描边数据和拖阴数据
	 * 后期扩充每一身体部位都可以DrawFadeGlow.fadeGlow使用相机的方向。
	 * 
	 * 先功能，后优化
	 */
	bool                    m_bFakeGlowDraw;
	DWORD                   m_dwFadeGlowColor;
	/*
	bool                    m_bDrawTail;
	float                   m_fTailLength;
	DWORD                   m_dwTailColor;
	//! 根据move的方向和速度，来画Tail.
	osVec3D                 m_vec3MoveDir;
	*/

private:

	// 当前skinMesh用到的bodyframe的数据，m_pRoot和他之下的指针都从这里面分配数据。
	osc_bodyFrame       m_arrBodyFrame[MAX_FRAMENUM];
	//! 记录当前使用到的bodyFrame的数目。
	int                m_iBFNum;
	//! 得到一个可以使用的bodyFrame空间。
	osc_bodyFrame*      get_freeBF( void );

	//! 创建人物时,得到初始化动作的id.
	void               get_defaultAct( os_skinMeshInit* _smi );

	//! 释放身体某一部分对应的skinShader.
	void               release_skinShader( int _idx );

	//! 用于检测两个动作是否可以插值
	bool               can_lerpInDiffAction( int _actIdx1,int _actIdx2 );

	//! 得到调入时用到的bis文件名。
	void               get_bisFileName( s_string& _res,s_string& _sdir );


	//@{
	// TEST CODE: 
	//! 用于确保Frame内身体部数目不为零的函数.在创建人物和切换装备前后调用.
	void              assert_frameBMNum( void );
	//@}


	//!  算出此时动作点在当前整个动作的播放百分比
	float             cal_posePercent( void );

	//! 从动作名字中，得到我们要播放哪一个动作，可能传入多个动作名随机播放
	int               get_aniNameIdx( void );

# if GALAXY_VERSION
	BOOL              m_bProjectShadow;
	//! 构建当前投影阴影用到的矩阵
	void              construct_shadowMat( osMatrix& _shaMat );
	
# endif

	//! 记录当前skinMeshFrame的mat数据


private:

	/** \brief
	 *  处理当前skinMesh的Frame Hierarchy数据
	 */
	void               process_frameHierarchy( void );


	/** \brief
	 *  替换某一身体部位的mesh.
	 *
	 *  \param _bidx    身体部位的id.
	 *  \param _midx    要替换的Mesh的id.
	 */
	bool               change_bodyMesh( int _bidx,int _midx );

	/** \brief
	 *  替换某一身体部位的skin.
	 *
	 *  \param _bidx    身体部位的id.
	 *  \param _midx    要替换的Mesh的id.
     *  \param _sidx    要替换的skin的id.
	 */
	bool               change_bodySkin( int _bidx,int _midx,int _sidx );


	/** \brief
	 *  设置动作播放的时间和速度相关信息。
	 */
	void               set_actSpeedInfo( float _actTime );

	/** \brief
	 *  如果当前处于动作变换状态，需要调用这个动作变得换函数。
	 *
	 */
	void               frame_moveActChange( float _ftime );

	//! 最终计算当前资势的矩阵。这个函数不能用于动作变化状态。
	void               cal_finalPose( void );

	/** \brief
	 *  对当前的骨骼动作frame move
	 */
	void               frame_moveAction( float _ftime );

	/** \brief
	 *  对于"负时间"的skinMesh动作计算。
	 *
	 *  用于刀光的倒推，残影的计算等等效果。
	 *  这个函数不考虑动作的改变，在第零帧的时候，不和结束帧做插值。
	 *  如果帧到达了开始帧，则下一帧直接从结束帧的位置开始。
	 *  
	 *  在开始这个函数之前，
	 *  \param float _ftime 此值必须小于等于零，这样才有此函数的意义。
	 */
	void               negative_frameMoveAction( float _ftime );

	//! 得到当前m_iNegAniIndex关键帧对应的的动作。
	void               get_keyFramePose( void );


	/** \brief
	 *  每播放完一套动作时，根据当前的actChange结构对动作进行处理。
	 */
	os_boneAction*               process_ActionOver( void );

	/** \brief
	 *  处理当前skinmesh内每一个frame的更新。
	 */
	void               update_frame( bool _catchBoneMat = false );

	/** \brief
	 *  初始化当前的skin Mesh.
	 */
	void               reset_skinMesh( void );

	/** \brief
	 *  对两个动作进行插件，填充插值后产生的矩阵。
	 */
	void               lerp_actionPose( osMatrix& _mat,
		                  os_bactPose* _pa,os_bactPose* _pb,float _lf );

	//  River Added @ 2007-4-20:加入新的动作插值方式。
	//! 对两个动作插值，产生一个新的pose,使用这个pose去跟新的动作再插值。
	void               lerp_actionPose( os_bactPose* _res,
		                  os_bactPose* _pa,os_bactPose* _pb,float _lf );


	//! 读入第二层特效纹理的数据。
	bool                read_secondTexEff( os_chaSecEffect* _seceff,char* _sname );

	//! 创建普通的单层纹理人物渲染效果。
	bool                create_singleTexShader( int _idx,char* _sname );

	//! 创建加入特效的双层纹理人物渲染效果。
	bool                create_doubleTexShader( int _idx,char* _sname );

	//! 创建特殊的三层贴图旗帜Shader
	bool                create_mlFlagShader( int _idx,char* _sname );

	//! 创建当前skinMeshSystem的shader.
	bool                create_smShader( os_skinMeshInit* _smi );

	/** \brief
	 *  渲染当前动作的刀光
	 *
	 *  把时间往前推，依次算出前几帧刀光的位置。这个技术应该也可以实现出其它的特效，比如
	 *  人的残影等等，以后扩展。
	 */
	bool                   render_smSwordGlossy( LPDIRECT3DDEVICE9 _dev );


# if __GLOSSY_EDITOR__
	//! 对刀光数据进行处理。刀光编辑器专用函数。
	void                weapon_process( int _idx,osc_bodyPartMesh* _bpm );
# endif 

public:
	/** \brief
	 *  得到当前skinMesh的位置顶点。
	 */
	void                   get_smPos( osVec3D& _vec )         { _vec = m_vec3SMPos; }
	const osVec3D&		   get_smPos(void)const               {return m_vec3SMPos;}
	osVec3D				   get_smPos(void)	                  {return m_vec3SMPos;}
	//! 得到此skinMesh在max制做后的偏移顶点
	void                   get_adjPos( osVec3D& _vec )        { _vec = m_vec3AdjPos; }

	/** \brief
	 *  得到身体部位使用的shaderId.
	 */
	int                    get_partShaderID( int _idx )        { return m_vecBpmIns[_idx].m_iShaderID; }


	/** \brief
	 *  得到当前的skinMesh是否有alphaPart.
	 */
	bool                   whether_hasAlphaPart( void )        { return m_bHasAlphaPart ; }


	//! 设置手部位的矩阵。
	void                   set_handPos( osVec3D& _pos )       { this->m_vec3HandPos = _pos; } 

	//! 得到当前skinMesh对应的目录。
	void                   get_smDir( s_string& _str )         { _str = m_strSMDir; } 
	//@{
	// Windy mod @ 2005-8-30 19:14:28
	//Desc: ride 设置第二部分坐骑的位置
	bool			  m_bIsRide;
	osVec3D           m_vec3RidePos;
	osVec3D			  m_vec3RideOffset;
	//!记录坐骑身上人物的指针.
	osc_skinMesh * 	  m_HumObj;
	//!记录人物身上坐骑的指针.
	osc_skinMesh *	  m_RideObj;

	DWORD	mEffectId;
	osVec3D mEffectOffset;

	//! 用于更新人物跟随特效的位置和方向
	void                    update_followEffect( void );

	//!用于设置定位盒的位置
	void					set_ridePos(osVec3D _pos)			{ this->m_vec3RidePos = _pos; }
	//!用于
	void					LinkHumObj(osc_skinMesh * _humobj) { osassert(_humobj); m_HumObj = _humobj; }
	//!
	void					DelLinkHumObj(){ m_HumObj = NULL; }
	//!
	void					HideLastPart(bool _IsHide);
	//!时实得到坐骑定位盒的位置，
	void					get_RideBoxPos( osVec3D& _pos );

	//! 由另外的线程来调用changeEquipment的数据
	bool                   change_equipmentThread( const os_equipment& _equi );
	//! 由另外的线程来释放人物.
	bool                   release_characterThread( BOOL _sceneSm );

	//! 如果当前是坐骑，得到坐骑上人物的指针，有可能返回空。
	virtual I_skinMeshObj* get_humanObjPtr( void  ); 
	virtual bool           is_rideObj( void ) { return m_bIsRide; }
 
public:
	//!对外使用的二个接口


	/** \brief
	* 设置坐骑，绑定后该人物将随坐骑一起移动，
	* 上层要以坐骑为主角人物控制。并设置相应的动画
	*/
	void					SetRide(I_skinMeshObj * _rideobj) ;
	//!取消坐骑，但不释放任何资源
	void					DelRide()  ;

	//! 得到坐骑绑定点的世界坐标
	bool osc_skinMesh::GetRidePos( osVec3D& _vec3 );

# if __RIDE_EDITOR__
	//!得到最后一部分的位置
	void			GetLastPartPos( osVec3D & _v3d );
	//!设置最后一部分的位置
	void			SetLastPartPos( osVec3D & _v3d);
	//!保存最后一部分的位置到文件中
	void			SaveLastPart();
# endif
	//@}

	//@{ windy add 2006-04-13
	void	get_Default_ActName(std::string& _ActName);
	void	get_Act_List(std::vector<std::string>& _ActNameList);
	//@}



public:
	osc_skinMesh();
	~osc_skinMesh();

	/** \brief
	 *  对当前的skinMesh进行渲染。
	 *
	 *  \param 当前渲染需要的渲染skinMesh中的alpha部分还是非alpha部分。
	 *  
	 */
	bool                   render_skinMesh( LPDIRECT3DDEVICE9 _dev,bool _alpha = false );

	//!渲染人物深度信息
	bool				   render_skinDepth(LPDIRECT3DDEVICE9 _dev);

	/** \brief
	 *  从目录文件调入一个骨骼系统。
	 */
	bool                   load_skinMeshSystem( os_skinMeshInit* _smi );

	//! River added @ 2009-6-7:当前的skinMesh接口操作是否可操作.多线程调入skinMesh
	virtual bool           legal_operatePtr( void ){ return !m_bLoadLock; }

	//! River added @ 2009-8-26: 隐藏人物的即时阴影.
	virtual bool             hide_realShadow( bool _hide = true );


	/** \brief
	 *  改变当前skinMeshObject的动作.
	 */
	virtual float           change_skinMeshAction( os_newAction&  _act );
   

	//! 使停止状态的动作继续播放。
	virtual void             start_moveAction( void );


	/** \brief
	 *  人物动作是否到达当前动作的关键帧
	 *
	 *  到达关键帧之前返回值为假，到达之后返回值为真。
	 *  
	 */
	virtual bool             arrive_stopFrameOrNot( void );
	//! 得到从动作开始到动作停止帧所用的时间。
	virtual float            get_timeFromStartToSF( void );




	/** \brief
	 *  替换skinMesh的装备.
	 */
	virtual bool            change_equipment(const  os_equipment& _equi );


	/** \brief
	 *  从当前的character得到一个boundingBox.
	 */
	virtual os_bbox*         get_smBBox( void );
	virtual const os_bbox*        get_smBBox( void )const ;

	virtual float            get_smBSRadius( void );

	//! Set VIP Load，多线程调入，当前skinObj放在队列的最前面，最后设置的char优先级别最高.
	virtual void            set_vipMTLoad( void );


	//! 角色可以使用刀光.
	virtual void            playSwordGlossy( bool _glossyEnable,bool _weaponEffEnable = true );

	//! 多线程创建刀光相关的数据.
	void                    playSwordGlossyThread( void );

	//! 内部用函数: 角色可以使用刀光,并且不记录刀光状态．
	void                    playSwordGlossyInner( bool _glossyEnable,bool _weaponEffEnable );


	/** \brief
	 *  切换人物的武器特效
	 *
	 *  /param int _idx           切换到第_idx个特效，超出索引的话，都使第零个特效。
	 *  /param WEAPON_ID _weapid  使用第七还是第八个武器部位。
	 */
	virtual void            change_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 );


	//! 得到当前帧刀光的中心点。
	virtual void            get_swordGlossyCenter( osVec3D& _pos );

	/** \brief
	 *  得到当前skinMesh在世界空间中的位置和旋转角度.
	 * 
	 *  \param _pos    返回skinMesh在世界空间的位置.
	 *  \param _fangle 返回skinMesh在世界空间的旋转角度.
	 */
	virtual void            get_posAndAgl( osVec3D& _pos,float& _fangle );
	
	//! 得到手部的矩阵。
	virtual void            get_handPos( osVec3D& _pos )    { _pos = m_vec3HandPos; }

	//! 得到当前skinMesh的旋转方向，
	virtual float           get_smAgl( void );

	//! 上层更新特效时用到的旋转方向，这个是渲染用到的真正方向。
	virtual float           get_renderSmAgl( void );


	/** \brief
	 *  更新当前的skinmeshObj的位置。
	 *
	 *  此函数仅用于修改skinMesh物体的位置和旋转角度.
	 *  引擎内部的skinObjFramemove函数对skinMesh做出处理。
	 *  
	 */
	virtual float           frame_move( osVec3D& _npos,float _nangle );

	//! 用于坐骑上人物位置的更改
	virtual float           frame_move( osVec3D& _npos,
		                         float _nangle,bool _rideSta );


	//! 得到当前动作播放所完成的比例,例：攻击动作完成40%，则返回0.4
	virtual float           get_actionPlayPercent( void ){ return cal_posePercent(); } 

	//! 得到当前正在播放的动作名字和索引
	virtual int             get_curPlayActIdx( char _name[32] )const;

	//! 得到某个动作事件点的百分比，比如受创特效需要在受创动作的播放多少时播放。
	virtual float           get_actionTriggerPoint( char _name[32],int _idx = -1 );

	/** \brief
	 *  释放当前Character所占用的资源。
	 *
	 *  网络游戏过程中可能需要调入和释放大量的资源，所以这个函数必须实现。
	 *  
	 */
	virtual void            release_character( BOOL _sceneC = false );


	/** \brief
	 *  当前人物总共有多少套动作可以播放。
	 */
	virtual int             get_totalActNum( void );

	/** \brief
	 *  从动作的名字得到播放当前动作需要的时间。
	 */
	virtual float           get_actUsedTime( const char* _actName,int& _actIdx );

	/** \brief
	*  从动作的名字得到播放当前动作需要的时间。
	*/
	virtual float           get_actUsedTime( const char* _actName );

	/** \brief
	 *  隐藏当前skinMesh的显示。
	 *
	 */
	virtual void            set_hideSkinMesh( bool _hide );

	//! is hide state?
	virtual bool			is_hideState(void) const;

	//! 隐藏当前人物身上绑定的特效
	virtual void            hide_followEffect( bool _hide );


	/** \brief
	 *  从动作的id得到播放当前的动作需要的时间.
	 */
	virtual float           get_actUsedTime( int _id );

	/** \brief
	 *  得到当前skinMesh的面数
	 */
	int                    get_smFaceNum( void );


	//! 设置fade状态。
	virtual void            set_fadeState( bool _fadeS,float _alphaV = 1.0f );


	//! 传入一个环境光值，系统和当前的场景环境光值做比较，使用更亮的光照亮当前角色。
	virtual void            set_maxBright( osColor& _bcolor );

	//! Device reset相关.
	void                   sm_onLostDevice( void );
	void                   sm_onResetDevice( void );

	/** \brief
	 *  暂停当前人物的动作。
	 *
	 *  \param bool _suspend 如果为true,则暂停动作，如果为false,则继续动作。
	 */
	virtual void            suspend_curAction( bool _suspend );


	//! 根据索引，得到当前weapon特效的文件名。
	virtual const I_weaponEffect* get_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 );

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// 以下为编辑器相关的内容，跟最终的游戏渲染引擎无关。
# if __GLOSSY_EDITOR__
	/** brief
	 *  只显示某个身体部位的部分三角形,公用于调试目的
	 *
	 *  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
	 *  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
	 */
	virtual int        displaytri_bysel( int _paridx,int _triidx[24],int _trinum );


	//! 设置人物刀光拖尾的长度。有效数据是6-12，如果大于或是小于此值，使用默认的拖尾长度
	virtual void        set_sgMoveLength( int _len );

	// 得到刀光拖尾的长度数据
	virtual int         get_sgMoveLength( void );



	//  以下函数只在编辑刀光的编辑器中使用。
	/** \brief
	 *  把当前sm中的武器文件存入到新的文件中。
	 */
	virtual bool            save_weaponFile( WEAPON_ID _weapid = WEAPON7);

	/** \brief
	 *  设置第N个武器特效文件，最多4个。
	 * 
	 *  \param int _idx 设置第几个武器特效，从零开始，最大值为3.超过3的话，内部会变为零
	 *  \param char* _effName 武器特效的文件名
	 *  \param WEAPON_ID _weapid 设置第七个还是第八个武器部位的特效。
	 */
	virtual bool            set_weaponEffect( int _idx,
		                     const I_weaponEffect& _effName,WEAPON_ID _weapid = WEAPON7 ); 



	//! 调整当前人物手中刀光的大小。
	virtual bool            adjust_swordGlossySize( bool _start,float _size, WEAPON_ID _weapid = WEAPON7 );

	//! 在X方向上旋转刀光。
	virtual void            rot_swordGlossyX( float _radian , WEAPON_ID _weapid = WEAPON7) ;
	//! 在Y方向上旋转刀光。
	virtual void            rot_swordGlossyY( float _radian , WEAPON_ID _weapid = WEAPON7);

	//! 在X轴或是Y轴上移动刀光。
	virtual void            move_swordGlossy( bool _x,float _dis, WEAPON_ID _weapid = WEAPON7 );

	//! 设置此武器用于定位，不显示此武器。
	virtual void            set_displayWeapon( bool _dis, WEAPON_ID _weapid = WEAPON7 );    

	//! 重设刀光的各种旋转角度，使完全回到调整前的刀光
	virtual void            reset_swordGlossyPosRot( WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  设置刀光对应的动作索引, 并且设置每一个动作对应的颜色数据。
	 *
	 *  \param _actIdx    表示哪一个动作需要显示刀光。
	 *  \param _arrColor  表示对应哪一个动作刀光的颜色。
	 */
	virtual void            set_swordGlossyActIdx( int _actNum,int* _actIdx,
		                      DWORD* _color,const char* _glossyTex, WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  得到人物的当前武器对应刀光数据。
	 *  
	 *  刀光的位置信息还需要微调。
	 *  \param int& _actNum 返回有多少个动作使用了刀光.
	 *  \param int* _actIdx 传入一个整型数组，这个数组必须使用至少12元素，传出了使用刀光的
	 *                      的动作索引。
	 *  \param char* _glossyTex 上层传入的字串，必须是一个64字节的char型数组，传回了当前
	 *                          武器使用的刀光纹理名字。
	 */
	virtual void            get_swordGlossyActIdx( int& _actNum,int* _actIdx,
		                            DWORD* _color,char* _glossyTex, WEAPON_ID _weapid = WEAPON7 );


	//! 设置skinMesh当前武器附加的刀光特效。
	virtual void            set_sgEffect( float _pos,
		                       const I_weaponEffect& _effFName,WEAPON_ID _weapid = WEAPON7);

	//! river @ 2010-3-5:设置effectPos.
	virtual void            set_sgEffectPos( float _pos,int _idx,WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  得到武器附加特效相关的信息.
	 *
	 *  \param _effFName 外部传入的特效名字字串,至少应该有128个字节长.
	 */
	virtual void            get_sgEffect( float& _pos,
		                            I_weaponEffect& _effFName, WEAPON_ID _weapid = WEAPON7 );
	//! 得到附加的addPos.
	virtual void            get_sgEffectPos( float& _pos,int _idx,WEAPON_ID _weapid = WEAPON7 );


	/** \brief
	 *  绕武器的中心轴旋转osa特效,使用osa文件与武器的对位更加精确.
	 *
	 *  如果当前武器对应的特效类型不是osa,则此函数没有效果.
	 */
	virtual void            rotate_osaEffect( float _agl, WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  缩放武器上的特效效果
	 *
	 *  \param _scale 如果是1.0，则保持原特效大小不变，2.0则增大为原来的二倍.0.2则缩小为原来的5分之1
	 *                如果传入的缩放比例小于零，则内部的比例不变，返回内部当前特效的缩入比例。
	 *  \return float 返回了当前特效的缩放比例。
	 */
	virtual float           scale_weaponEffect( int _idx,float _scale,WEAPON_ID _weapin = WEAPON7 );

	//@{
	// 人物的动作和装备编辑相关。
	

	/** \brief 
	 *  显示人物的下一个动作帧姿势。
	 * 
	 *  人物的动作必须处于suspend状态，这个函数才起作用。
	 *
	 *  \param bool _next 如果为true,则使用下帧的人物动作，如果为false,则使用上一帧的姿势。
	 */
	virtual void            next_actFramePose( bool _next );

	/** \brief
	 *  得到某一个动作总共有多少帧。
	 *
	 *  \param _actName 默认情况下使用动作名字来得到帧的总Frame数目。
	 *  \param _actIdx  如果此值大于等零，则返回相应索引动作的总Frame数目
	 */
	virtual int             get_actFrameNum( const char* _actName,int _actIdx = -1  );


	//! 得到当前人物正在播放动作的帧索引。
	virtual int             get_curActFrameIdx( void );

	//! 的到这个skinMesh的骨骼系统
	//! 的到这个skinMesh的骨骼系统
	virtual os_boneSys* 			get_boneSys(void) {return m_ptrBoneSys;}
	
	/** \brief
	 *  得到身体部位的面数，如果当前身体部位不存在，返回0
	 *
	 *  \param int _idx 身体部位索引，从零开始。
	 */
	virtual int             get_bodyTriNum( int _idx );

	//@}


# endif
	///////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// 人物身体部位第二层贴图的特效编辑相关函数
# if __CHARACTER_EFFEDITOR__
	/** \brief
	 *  更新身体某一部位的纹理贴图效果。
	 *
	 *  如果身体部位原来没有第二层纹理，则更新为使用第二层纹理。
	 * 
	 *  \param _seceff 如果传入的值为空，如果原来此身体部位使用特效，则更新为不使用特效。
	 *  
	 */
	virtual void           update_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx );

	//! 得到当前身体部位的chaSecEffect数据。
	virtual bool           get_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx );

	//! 存储身体第_idx部位的第二层纹理贴图效果。
	virtual bool           save_chaSecEffect( int _idx );
# endif 


	/** \brief
	 *  对人物进行缩放。
	 *
	 *  \param osVec3D _svec 表示了在三维中的三个方向人物要缩放的比例。
	 */
	virtual bool           scale_skinMesh( osVec3D& _svec );

	// Rriver @ 2010-1-15: TEST CODE:内部使用接口。
	bool                   scale_skinMesh( float _x,float _y,float _z );


# if __ALLCHR_SIZEADJ__
	//! 使用全局的人物缩放信息来得处理人物.用于创建人物时,使用全局的scale.
	void                  createScale_withAllChrScale( void );
# endif 

	//! 是否拿方向光做为环境光渲染。
	bool                  is_useDlAsAmbi( void ) { return m_bUseDirLAsAmbient; }


# ifdef _DEBUG
	//! 导出当前skinMesh内的内容。
	void                  dump_debugStr( void );
# endif 

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
							float _zoom,BOOL _newChar = FALSE ,I_camera* _camera = NULL);


	//! 用于当skin不在视域内的时候，播放死亡类显示最后一帧的动作
	void                    play_lastFrameStopAction( void );

	//! 切换旗帜的贴图
	virtual bool            change_mlFlagTexture( const char* _tex1,
		                        const char* _tex2,const char* _tex3 );

	//! 测试得到某一根骨骼的位置.
	virtual void            get_bonePos( const char* _boneName,osVec3D& _pos );

	//! 设置调入人物成功
	void                    set_loadLock( bool _lock ) { m_bLoadLock = _lock; } 

	//! 设置人物渲染fade glow.
	virtual void            set_fakeGlow( bool _glow,DWORD _color );

	//! River @ 2010-7-1: 人物是否隐藏武器，如果隐藏，则不渲染武器部分
	void                    hide_weapon( bool _hide = true );



# if GALAXY_VERSION
	//! 画当前skinMesh对应的projectShadow.
	void                    draw_projectShadow( void );
# endif 
};

//! skinMesh类的stl vector. 
typedef std::vector< osc_skinMesh >   VEC_skinMesh;
typedef std::vector< osc_skinMesh*>   VEC_skinMeshPtr;

# if 1


struct os_charLoadData
{
	//! 要调入的数据结构
	os_skinMeshInit   m_sMeshInit;

	//! 要调的skinMesh主体
	osc_skinMesh*     m_ptrSkinPtr;

	//  
	//! 是否切换装备
	os_equipment      m_sEquipent;
	bool              m_bChangeEquipent;
	
	//! 是否释放character.
	bool              m_bReleaseChar;
	//! 是否场景的sm.
	bool              m_bSceneSm;

	//! 是否playSwordGlossy.
	bool              m_bPlaySwordGlossy;

public:
	os_charLoadData() 
	{
		m_ptrSkinPtr = NULL;
		m_sMeshInit.m_szspeFileName[0] = NULL;
		m_sMeshInit.m_strSMDir[0] = NULL;

		//! 如果此值为真，则不需要调入skinMesh,直接切换人物的装备。
		m_bChangeEquipent = false;
		m_bReleaseChar = false;
		m_bSceneSm = false;

		//！River @ 2011-2-25: 如果此值为真，处理人物的刀光播放相关数据
		m_bPlaySwordGlossy = false;

	}
};

typedef std::list<os_charLoadData>   LIST_charLoadData;

/** \brief
 *  人物的多线程调入类
 *
 *  根据地图的多线程调入类修改而成
 */
class osc_charLoadThread : public com_thread
{
private:
	//! River @ 2011-2-12:使用list来操作.
	LIST_charLoadData   m_listCharLoadData;
	

	//! 当前正在处理的数据指针。
	int    m_iProcessPtr;
	//! 正在推进的数据指针
	int    m_iDataPushPtr;

	//! 当前正在调入的char.
	os_charLoadData     m_sCurLoadChar;

	//! 需要等待完成调入或装备切换的数据结构
	os_charLoadData     m_sVipChar;
	//! 是否正在处理vip的调入，只在调入线程内使用的变量
	bool                m_bVipProcess;

private:
	//! 在推进和读出要调入的人物数据时,使用的临界区.
	static CRITICAL_SECTION   m_sCharNameCS;

	//! 是否唤醒人物调入线程的event.
	static HANDLE             m_sEventLCStartMt;

	//! 人物调入线程是否处于空闲状态，在等待线程被唤醒的消息
	static HANDLE             m_sEventLCThreadFree;

	//！退出线程的event
	static HANDLE             m_sEventLCExitThread;

	//! 当前的人物是否调入完成.
	static HANDLE             m_sEventCLLoadFinish;

private:
	//! 得到可以调入的character,如果队列内没有任务，返回false.
	bool         get_frontChar( os_charLoadData& _cl );

	
public:
	osc_charLoadThread();
	~osc_charLoadThread();

	//  
	//! 传入相应的指针.
	void                    init_mtLoadCharMgr( void );


	//! 每一帧对多线程地图调入管理器进行处理。
	void                    frame_moveCLMgr( void );

	/** \brief
	 *  给管理器指定一个要调入的地图。
	 *
	 *  \param _mapName 要调地图的名字.
	 *  \param _mapIdx 调入地图时,使用的地图索引.ATTENTION TO FIX:
	 *  \param _waitForFinish 是否等待另外的线程完成地图的调入,
	 *                  此机制用于完成阻塞式的地图调入.
	 */
	void                    push_loadChar( os_charLoadData* _ldata,
		                                   BOOL _waitForFinish = FALSE );

	//! 清空管理器要调入的人物,在释放整个地图时，可能有另外的线程正在调入人物，处理。
	void                    reset_loadCharMgr( void );

	//! 当前是否处于闲置状态,即等待主线程推进要调入的人物
	bool                    is_freeStatus( void );

	/**
	 *  每一个派生类都必须重载的函数.
	 *
	 *  函数处理流程:
	 *  
	 */
	virtual DWORD            ThreadProc();
	virtual void			 ThreadExceptionProc(); //异常处理


	//! 结束线程的执行
	void                     end_thread( void );


	//! River @ 2011-2-25:同步要删除的人物指针临界区.
	static CRITICAL_SECTION  m_sCharReleaseCS;

};
# endif 

//！ River @ 2009-5-10:高配版本，所有的人物都可以加入阴影
# if 1 

# define MAX_RTSHADOWNUM  128 

# else

//! 引擎中支持最多的即时阴影数目,目前需要最多支持两个,即选人场景需要两个.
# define MAX_RTSHADOWNUM  2 

# endif 

/*
   关于Character的可见性判断

   每一次对character 设置新的位置时，设置当前character所在的地图和格子。
   并把原来的地图和格子的character索引值设为-1.并且设置地图管理器中新的character为
   当前character的id索引。

   渲染地图的时候，对当前地图内所有可视格子对应的character设置可见性。

   //
   // 
   // 人物的FADE相关操作:
   把每一个I_skinMeshObj*和一个osc_fadeMgr结合成一个数据结构，然后在每一帧内进行FrameMove,
   如果完成了fade,则删除此结构，否则，我们需要对此skinMesh数据处理，渲染成半透明的人物。
   
*/


//! 宠物，坐骑，加主角自己，以后可以再扩充
# define MAX_BEHINDWALLCHAR   3

//! 坐骑的队列可以更大一些，以防后期不够用。
# define MAX_RIDENUM   168
//! River mod @ 20109-28:坐骑系统，可以多线程调入.
struct os_ridePtr
{
	osc_skinMesh*   m_ptrHuman;
	osc_skinMesh*   m_ptrRide;

	os_ridePtr() { m_ptrHuman = NULL;m_ptrRide = NULL; } 

};


/** \brief
 *  管理场景中所有的skinMesh的管理器。
 *  
 *  包括了调入场景时创建的加入了骨骼动画的物体和
 *  程序运行过程中创建的character骨骼动画数据。
 *  
 *  如果每张地图使用最多64个动画物品的话，系统大约需要多出10M左右的内存来渲染这些物品。
 *  大约5.5M用于存储skinMesh数组。如果再加上其它相关的计算因素，比如cpu和gpu等等，这将是很大的资源需求。
 *  
 *  ATTENTION TO OPP:测试Gefoce级别显卡上IndexedBlend的SkinMesh动画方式。
 *  
 */
class osc_skinMeshMgr : public ost_Singleton<osc_skinMeshMgr>
{
	friend class osc_skinMesh;
private:

	//! 上层存储所有的坐骑与人物关联指针.
	os_ridePtr           m_vecRidePtrArray[MAX_RIDENUM];

	/** \brief
	 *  人物的顶点和索引设备相关资源的管理器。
	 *
	 *  使用这个管理器，对于同一个mesh来说，我们在内存中只需要保
	 *  存一份就可以了。
	 */
	osc_smDepdevResMgr   m_devdepResMgr;

	/** \brief
	 *  骨骼系统的管理器。
	 */
	osc_boneSysMgr       m_boneSysMgr;

	/** \brief
	 *  骨骼动画数据系统的管理器。
	 */
	osc_boneAniMgr       m_boneAniMgr;


	/** \brief
	 *  身体部位数据的manager,管理调入数据。如果身体部分的引用为0,释放内存。
	 */
	osc_bpmMgr           m_bpmMgr;


	/** \brief
	 *   由这些资源提供上层的可操作物体指针。
	 *
	 *  
	 */
	CSpaceMgr<osc_skinMesh> m_vecDadded;

	//! 场景中动画物品使用的skinMesh列表。
	CSpaceMgr<osc_skinMesh> m_vecSceneSM;



	//! 设备相关数据的指针。
	static LPDIRECT3DDEVICE9       m_pd3dDevice;

	//! 当前视域内的skinMesh 索引的列表。
	VEC_skinMeshPtr          m_vecInViewSM;
	//! 当前帧内在场景中的skinMesh的数目。
	int                    m_iInviewSMNum;

	//! 当前mgr内，在墙后要渲染的人物列表
	VEC_skinMeshPtr          m_vecRCBehindWall;


	//! 当前的skinMeshMgr渲染到的面的数量.
	int                    m_iRFaceNum;

	//! 当前帧要渲染的即时阴影人物数目.
	int                    m_iRTShadowNum;

private:
	//! 使用阴影的skinMesh指针。//syq
	static osc_skinMesh*           m_ptrShadowSkinPtr[MAX_RTSHADOWNUM];



private:

	//! d3dIndexedBlend渲染时用到的渲染状态块。
	static int              m_iIndexedBlendSB;

	//! 渲染smMgr前的渲染状态改变。
	static int              m_iSetsmSB;
	//! 结束smMgr渲染时的状态改变。
	static int              m_iEndSetsmSB;


	//! 多线程调入人物.
	osc_charLoadThread      m_sCharLoadThread;

private:
	
	/** \brief
  	 *  得到在视域内的Character的列表。
	 *  
	 */
	void                   get_inViewChrList( void );

	/** \brief
	 *  在初始化skinMeshMgr的时候,初始化我们的skinMethod.
	 */
	void                   init_skinMethod( void );

	/** \brief
	 *  在渲染skinMeshMgr之前根据设备不同,设置渲染状态.
	 */
	void                   set_smMgrRState( void );

	//! 创建smMgrRState的渲染块。
	void                   create_smMgrrsSB( void );
	//! 创建结束smMgrRState的渲染块。
	void                   create_smMgrEndrsSB( void );


	/** \brief
	 *  创建indexed Blend渲染时使用的渲染状态块。
	 */
	static void            create_IndexedBlendSB( void );

	/** \brief
	 *  在结束skinMeshMgr渲染的时候,设置的渲染状态.
	 */
	void                   endset_smMgrRState( void );

	//! 渲染skinMesh中有阴影那一个
	void                   render_shadowSkin( void );

	//! 推进要渲染的有即时阴影的skinMesh指针.
	void                   push_rtShadowSmPtr( osc_skinMesh* _smPtr );

	//! 重设管理器中所有的即时阴影数据,使下次渲染的时候,更新阴影.
	void                   reset_rtShadow( void );

	/** \brief
	 *  根据相机的位置和方向光的向量，来确认是否需要画人物的阴影
	 *
	 *  如果相机的位置在人物的阴影体内，则不需要画阴影，否则会出现错误的阴影。
	 */
	BOOL                   is_cameraInShadowVolume( void );
	//! 相机的近剪切面是否剪切到人物的投射阴影体，大致检测
	BOOL                   is_nearCamPlaneClipSV( float _length,int _shaIdx );

	//! 相机位置和焦点的射线是否被地形挡住
	BOOL                   is_camFocusWardByTerr( void );

	//! 使用ZPass阴影时用到的测试函数
	void                   zpass_process( void );

	//! 找到离传入位置最远的character索引,用于场景内人物过多时,只显示近处的人物.
	int                    get_farthestChrIdx( osVec3D& _pos,float& _resDis );



	//! 人物实时阴影的渲染
	void                   shadow_draw( BOOL _meshWithoutShadow );


private:
	osc_skinMeshMgr();
	~osc_skinMeshMgr();

	//! River @ 2009-6-23:渲染fakeGlow效果
	void                  render_fakeGlow( void );



# if GALAXY_VERSION
	//! 投射阴影的状态块
	static int             m_iProjectShadowSB;

	//! 创建投射阴影的状态块
	void                   create_smMgrProjectShadowSB( void );

public:
	//! 投影阴影用到的平面
	static osMatrix                m_matShadow;

# endif 

public:

	//! 每一时刻mgr只能创建一个mesh.
	static CRITICAL_SECTION  m_sChrCreateCS;

	/** 得到一个skinMeshMgr的Instance指针.
	 */
	static osc_skinMeshMgr* Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void         DInstance( void );


	//! 初始化当前的skinMeshManager.
	virtual bool                init_smManager( osc_middlePipe* _mp );


	//! 释放当前skinMeshManager.
	void                        release_smManager( bool _finalRelease = false );


	/** \brief
	 *  用于创建和操作上层需要的SkinMeshObject.
	 *
	 *  由场景调用这个函数给上层提供可以使用的skinMesh指针。
	 *
	 *  \param _smi      用于初始化新创建的skinMeshObject.
	 *  \param _sceneSm  是否创建场景中的动画物品。场景中的动画和人物怪物动画分开存储。
	 *  \return     如果创建失败，返回NULL.
	 */
	virtual I_skinMeshObj*       create_character( os_skinMeshInit* _smi,
		                            bool _sceneSm = false,bool _waitForFinish = false );   


	/** \brief
	 *  设置Mgr内某一个skin Mesh的可见性。
	 */
	void                set_inViewChr( int _id );

	//! 当环境光发生变化时，对skinMeshMgr进行处理。
	void                reset_ambient( void );

	
	//! Restore 当前的skinMeshManager.
	void               smMgr_onLostDevice( void );
	void               smMgr_onResetDevice( void );


	virtual bool       render_smDepthMgr();
	//
	/** \brief
	 *   渲染当前的skinMeshManager.
	 *
	 *   \param _ralpha 当前渲染是否渲染有alpha通道的ani部分。
	 *	 \param _meshWithoutShadow 如果是 TRUE 的时候渲染 模型 , FALSE 的时候渲染模型 + 阴影
	 *   \param _rtype             普通渲染还是分层渲染,分层渲染需要把个别的sm后期再渲染
	 */
	virtual bool                render_smMgr( os_FightSceneRType _rtype = OS_COMMONRENDER,
		                            BOOL _ralpha = false ,BOOL _meshWithoutShadow = FALSE );

	//! frameMove当前的skin mesh Mgr.
	virtual bool                framemove_smMgr( void );


	//@{
	//！  River @ 2010-1-29: 渲染墙折挡物后的人物。
	virtual bool                renderBehindWallChar( void );
	virtual void                push_behindWallChar( osc_skinMesh* _ptrMesh );
	virtual void                delete_behindWallChar( osc_skinMesh* _ptrMesh );
	//@} 

# if GALAXY_VERSION
	//! 画场景内所有skinMesh的projectShadow
	void                        draw_projectShadow( void );
# endif 

	//! 对某一个Character进行单独渲染，主要用于把三维人物渲染到二维的屏幕界面内
	bool                        render_character( osc_skinMesh* _ptrMesh );

	//! 每一帧动skinmeshMGR设置。
	virtual void                frame_setSmMgr( void );

	/** \brief
	 *  得到某一个skin Mesh的世界坐标位置。
	 */
	void                       get_smPos( int _id ,osVec3D& _pos );

	/** \brief
	 *  得到当前的smMgr的面的数目.
	 */
	virtual int                 get_smMgrFaceNum( void )   { return m_iRFaceNum; } 


	//! 往管理器中加入可见的skinMesh物品。
	virtual void                add_sceneInviewSm( osc_skinMesh* _ptr );



# if __ALLCHR_SIZEADJ__
	//! 重设当前smMgr中每一个可用sm的scale值。
	void                        reset_smMgrScale( void );
# endif 


};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      以下全局变量只在骨骼动画的相关文件中使用。
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 管理场景中所有人物的设备相关资源。
extern osc_smDepdevResMgr*       g_smDevdepMgr;

//! 管理场景中所有的骨骼系统的管理器指针。
extern osc_boneSysMgr*           g_boneSysMgr;

//! 管理场景中所有的骨骼动画数据的管理器指针。
extern osc_boneAniMgr*           g_boneAniMgr;

//! 管理场景中身体部分的顶点数据的管理器指针。
extern osc_bpmMgr*               g_bpmMgr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//　                   文件格式相关的定义
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 不同文件的扩展名.
# define BIS_EXTENT  ".bis"
# define ACT_EXTENT  ".act"
# define SKN_EXTENT  ".skn"
# define ACS_EXTENT  ".acs"

//! act文件的开头字节.
# define ACT_FILEMAGIC "act"
//! 动作信息文件的开头字节."acf"
# define ACTINFO_FILEMAGIC  "acs"

//! skn文件的开头字节.
# define SKN_FILEMAGIC "skn"

//! 第二层贴图数据的扩展名.
# define  CHASEC_EXTENT       "cse"




# include "osCharacter.inl"




# endif  // # define __OSCHARACTER_INCLUDE__

