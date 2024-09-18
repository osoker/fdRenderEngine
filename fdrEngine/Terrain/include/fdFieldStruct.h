//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldStruct.h
 *
 *  His:      River created @ 2004-5-21
 *
 *  Des:      地图数据类相关的结构。
 *   
 * “吾恐季氏之忧，不在颛臾，而在萧墙之内也。” 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

//! 包含引擎的接口头文件
# include "../../interface/osInterface.h"

# include "../../mfpipe/include/middlePipe.h"

# include "../../effect/include/fdBillboard.h"

# include "../../effect/include/fdTrack.h"
# include "../../effect/include/dlight.h"
# include "../../effect/include/fdSky.h"
# include "../../effect/include/fdWater.h"
# include "../../effect/include/fdParticle.h"


# include "../../tools/include/osConsole.h"

# include "../../mfpipe/include/osMesh.h"

# include "../../mfpipe/include/deviceInit.h"

//! 认定数据结构相关的头文件。
# include "../../backPipe/include/osCamera.h"

# include "fdTGQuadtree.h"


//! 屏幕显示二维图片的相关数据结构。
typedef std::vector<os_screenPicDisp>    VEC_screenPicDis;


//! 每张64*64的地图最多可以使用的动态物体数目,应该限制到64个.
# define MAX_SMOBJPERMAP         64

//! 是否使用WaterAlpha，水面与岸边的接缝处半透明
# define WATER_ALPHA             0


//! 调入新地图最长可用的时间。
# define  FIRST_TIMETICK         400
# define  MAX_TIMETICK           100

//! 过多少帧以后，再次调入物品。
# define  OBJLOAD_PERFRAME       800

//! 公告板物体和纹理放置的目录.
# define  BILLBOARD_DIR   "efftexture\\"
//! skinMesh动画物体放置的目录.
# define  SMAINOBJ_DIR    "data\\smani\\"
//! keyAniMesh动画物品所在的目录,他们所用的纹理在smAni目录下,以共用纹理。
# define  KEYANIOBJ_DIR   "keyani\\"
//! water文件所在的目录.
# define  WATERINI_DIR    "water\\"
//! 粒子文件所在的目录。
# define  PARTICLE_DIR    "particle\\"
//! 场景特效文件所在的目录
# define  SCEEFFECT_DIR   "speff\\"


///////////////////////////////////////////////////////////////////////////////////
//
// 要做到无缝的地图调入，可能会使用另外一个线程来调入快要看到的地图。
// 
///////////////////////////////////////////////////////////////////////////////////

//! TileIdx的stl Vector.
typedef std::vector<os_tileIdx>  VEC_tileIdx;

# define QUADTREE_TEST  1


//! 声明我们用到的tgManager类
class    osc_TGManager;
class    osc_tgLoadReleaseMgr;

/** \brief
 *   地图文件头s
 */
struct os_TGFileHeader
{
	//!文件标识
	char	   m_cID[2];
	//!文件类型
	WORD	   m_wFileType;
	//!文件版本
	DWORD	   m_dwVersion;
	//!地图名
	CHAR	   m_strMapName[128];

	//! 这个值做为逻辑上的索引，把64*64的地图块拼合为更大的地图。
	LONG       m_vMapPos[2];

	//!包含多少物体在地图上
	DWORD      m_dwObjectNum;

	DWORD      m_dwTexNum;
	//地图第二层一共使用了多少纹理
	DWORD      m_dwBackTexNum;

};




//! 渲染顶点数据的Stl vector.
typedef std::vector<os_sceVerTile> VEC_verTile;



//! 视野内最多的物品的数目. 
# define MAX_INVIEWOBJ      512   

//! 一帧中可以添加和删除的最多的object数目,
//! ATTENTION TO FIX:使用更少的队列，做更优化的处理。			
# define MAX_ADDDELOBJ      64



//! 当前场景视野内每一个物品的状态.
enum  ose_inviewObjState
{
	//! 当前元素不在视野内显示。
	OSE_IVO_NOTINVIEW = 0,

	//! 当前元素在上一帧中对应了物品索引,当前帧此物品还存在于视野中.
	OSE_IVO_NORMALDIS,

	//! 当前元素正处于边缘状态,正在淡入视野.
	OSE_IVO_FADEIN,

	//! 当前元素正处于边缘状态,正在淡出视野.
	OSE_IVO_FADEOUT,

	//! 当前元素由淡出视野变为淡入视野.即淡出没有完成,开始淡入.
	OSE_IVO_FOUT_TO_FIN,

	//! 当前元素在淡入视野的过程中，没有完成淡入状态时，开始淡出视野。
	OSE_IVO_FIN_TO_FOUT,

	//! 当前元素为半透明显示，为挡在相机和人物之间的物品显示。
	OSE_IVO_HALFALPHA,

};

typedef std::vector<ose_inviewObjState>  VEC_inviewObjState;

//! 屏幕中最多显示64个挡在人物和相机之间的半透明物品。
# define MAX_HALFALPHAOBJ   64


/** \brief
 *  地形管理器中的可见物品管理器.
 *
 *  处理场景中的可见物品,使用这个数组,渲染物品,并使退出视野的物品
 *  有Alpha淡出过程,使进入视野的物品有淡入过程.
 *  
 *  River @ 2004-8-10:
 *  场景中每一个TG使用一个ObjMgr,每一帧设置时，只需要对比设置当前管理器
 *  内部每一个物品的状态就可以了。
 *  每一个viewObjMgr内存储了上一帧物品的可视状态，在每一个帧的处理中，根据
 *  当前帧内物品的状态和上一帧物品的可视状态，得到管理器内每一个物品的可视状态。
 *  
 */
class osc_inViewObjMgr
{
	friend class             osc_newTileGround;

	// TEST CODE:
	friend class             osc_TGManager;
private:

	//! 目前内存分配中，最大可用的结点数目。
	int               m_iMaxNodeNum;

	/** \brief
	 *  当前的状态数组代表了每一个objIndex的状态.
	 * 
	 */
	VEC_inviewObjState  m_vecIvOs;
	
	//! 正常显示的物品的列表。
	VEC_int           m_vecNormalDis;
	//! 当前数组存放了TG内每一个物品上一帧的可视状态。
	VEC_bool           m_vecLfVs;

	//! 正常显示的物品的数目。
	int                m_iNDObjNum;



	//! 可见物品管理器对应的淡入管理器.
	VEC_fadeMgr        m_vecFadeInMgr;
	//! 要加入到视野内的物品的队列.-1表示没有使用.
	VEC_int            m_vecFadeInObj;


	//! 从视野中消失时,对应的物品淡出管理器.
	VEC_fadeMgr        m_vecFadeOutMgr;
	//! 要从视野内删除的物品的队列.-1表示没有使用.
	VEC_int            m_vecFadeOutObj;


	//@{
	//! 半透明显示的物品列表,用于显示挡住相机的物品。
	int                m_vecHalfAlphaObj[MAX_HALFALPHAOBJ];
	//! 场景中已经存在的挡住相机的半透明物品的列表.
	int                m_vecAccuHAObj[MAX_HALFALPHAOBJ];
	/** \brief
	 *  从不透明的物品到半透明物品.
	 *
	 *  对于从半透明到不透明的物品,我们把这些物品入到
	 *  fadeIn物品的管理队列中处理.
	 */
	VEC_fadeMgr        m_vecHalfFadeMgr;
	//@} 



private:

	//! 添加一个fadeInObject.
	bool               add_fadeInObj( int _idx );

	//! 添加一个FadeOutObject
	bool               add_fadeOutObj( int _idx );

	//! 每一帧去挡住相机射线的透明物品的处理.
	void               process_halfFadeObj( void );

	//! 从正常显示的物品队列中删除一个物品.
	void               delete_normalDisObj( int _idx );

public:
	osc_inViewObjMgr();
	~osc_inViewObjMgr();

	//! 初始化当前的InviewObjMgr.
	void               init_ivoMgr( int _objNum );

	/** \brief
	 *  开始新一帧的处理.
	 *
	 *  必须在得到地表物品的可见性之前调用这个函数.
	 *  
	 */
	void              frame_setObjMgr( void );


	/** \brief
	 *  每一帧中得到当前TG内物品列表后，对inviewObjMgr内物品的处理。
	 */
	void              frame_processIvo( VEC_BYTE& _ivoList );

	//! 把一个正常显示的物品变为半透明的物品。
	void              make_halfFade( int _objIdx );


};


//@{

//! 文件版本信息。OSS（OS SOUND）
#define OSS_VERSION 100
static const unsigned long oss_version = (('O') | ('S'<<8) | ('S'<<16) | (OSS_VERSION<<24));

/** \brief
* OSSFile_hdr 文件头信息
*
* 保存声音条目的个数。*/
struct OSFile_hdr
{
	int		SoundCnt;	//!声音文件的个数.
};
// snd 文件头
struct SndHeader
{
	char	mFileFlag[4];	
	DWORD	mVersion;
	


	DWORD	mdwOfsSoundItem;	//!< OSSoundItem 类型
	DWORD	mdwNrSoundItem;		//!< 
};

/** \brief
* 每个声音条目的信息
*
* 保存音源id,声音文件名,最近距离等信息*/
struct OSSoundItem
{
	//!音源id
    int		sound_id;	
	//!音源名
	//char    m_idString[40];
	//! 位置
	osVec3D m_pos;
    //!声音文件名
    char	m_SoundName[128];
	//!最近距离,默认初始2
	float   m_flMinDistance;	
	//!最大距离,默认初始100
	float	m_flMaxDistance;	
	//!声源的音量0到-10000;默认10000
	//long	m_lVolume;		
	//! 播放概率类型,不播放,1/10~9/10,循环播放,  对应0-10
	DWORD     m_playType;
	// 音效的播放时间
	float   m_fSoundPlayTime;

	OSSoundItem( void )
	{
		sound_id = -1;
		m_pos = osVec3D( 0.0f, 0.0f, 0.0f );
		m_SoundName[0] = 0;
		m_flMinDistance= 2.0f;
		m_flMaxDistance= 100.0f;
		m_playType  = 0;
	}
	OSSoundItem& operator=( const OSSoundItem& _sounItem )
	{
		sound_id = _sounItem.sound_id;
		m_pos = _sounItem.m_pos;
		strcpy( m_SoundName, _sounItem.m_SoundName );
		m_flMinDistance = _sounItem.m_flMinDistance;
		m_flMaxDistance = _sounItem.m_flMaxDistance;
		m_playType   = _sounItem.m_playType;

		return *this;
	}
};

//! 正在播放的声音信息
struct OSSoundPlayingItem
{
	// River mod 2008-3-21:减少内存的分配和释放
	BOOL   m_bPlayingSound;

	//! 一个声音信息
	OSSoundItem * pItem;
	//! 动态获得的循环播放声音的id,为了关闭他，对于播放一次的声音，此值为-1
	int idCloseLoopSound;
	//! 
	float mPlayTimeMs;
	//! 
	float mTimeLength;

	OSSoundPlayingItem()
	{
		m_bPlayingSound = FALSE;
	}

	// 重设此结构
	void reset( void )
	{
		m_bPlayingSound = FALSE;
		pItem = NULL;
		idCloseLoopSound = -1;
		mPlayTimeMs = 0.0f;
		mTimeLength = 0.0f;
	}

	bool operator == (const OSSoundItem* rhs)
	{
		return (pItem==rhs);
	}
	bool operator == (const OSSoundPlayingItem& rhs)
	{
		return (pItem == rhs.pItem);
	}
};

//@}

//@{
// Windy mod @ 2005-8-12 9:22:25
//Desc: 用于保存每个顶点的额外高度的数据信息
typedef struct _AddHeight{
	WORD x;
	WORD y;
	float Modify;
public:
	_AddHeight(WORD _x = 0,WORD _y = 0,float _Mod = 0):x(_x),y(_y),Modify(_Mod){
	}

	bool operator == (const _AddHeight & _add){
		return (x == _add.x) && (y == _add.y);
	}
}AddHeight,*PAddHeight;
typedef std::vector<AddHeight> VEC_AddHeight;

//@}


//! 定义每一个格子的属性值。
extern const int MASK_TILEDISPLAY;
extern const int MASK_WATERTILE;


//! 使用osa文件做为场景中的水的文件.
# define  USE_OSAWATER      1


//! 如果把一个TG分成四块的话，每一块的名字。
enum osn_inTgPos 
{
	OSN_TGPOS_ERROR = 0,
	OSN_TGPOS_TOPLEFT,
	OSN_TGPOS_TOPRIGHT,
	OSN_TGPOS_BOTLEFT,
	OSN_TGPOS_BOTRIGHT
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  场景中用到的颜色字符解释。
// 
//! 每一个颜色解晰字串最多可以解晰出的字串数目。
// syq mod @ 05.2.26       24改为48   (旧的24在字串长时内存空间不足，会导致错误)
// River mod @ 06-7-22:    48改为72, 字串越来越大。
# define  MAX_STRDIS_PERCI    512
// WORK START: 优化此数据

//! 把颜色解晰字串解释为正常显示字串的功能类
class osc_szCI
{
	static char          m_cSelChar;
	static char          m_cSelCharS;
	static char          m_cSelCharE;

private:
	static DWORD     get_strColor( const char* _sz,int _idx,int& _cnum );

	//!
	static int       get_disStr( os_stringDisp* _resSD,int& _strIdx,
		                int& _xOffset,int& _yOffset,os_colorInterpretStr* _srcCI,int _cidx ,int _TextSize );

public:

	//! 如果在给定的屏幕相素限制区域内，可以显示字串，返回真否则返回false.
	static int       interpret_strDisp( os_stringDisp* _resSD,
		                     os_colorInterpretStr* _srcCI,int& _iresNum ,int _fontType = 0);
};

/** \brief
 *  在透明物体的排序算法中用到的camera Ptr
 *  
 *  场景中使用到的全局唯一的相机.
 */
extern osc_camera*           g_ptrCamera;








