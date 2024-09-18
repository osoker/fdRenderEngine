//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTerrainDecal.h
 *
 *  His:      River created @ 2006-3-6
 *
 *  Des:      用于在当前的地表上创建Decal.
 *   
 * “具有最高的智商的标志是，具有两个完全相反的想法在头脑中，而行为丝毫不受影响的能力”
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once


//! 包含引擎的接口头文件
# include "../../interface/osInterface.h"
# include "../../mfpipe/include/osPolygon.h"

class     osc_TGManager;


/*
Decal是紧贴地表渲染的多边形，需要根据地表多边形的高低起伏做出相应的调整，
Decal可以做出的效果：
1: 人物的假阴影。
2: 人物在地表的脚印。
3: 魔法特效对地表的破坏，黑色痕迹

Decal分为有方向和无方向两种情况，有方向的更灵活，无方向的decal则速度更快。
无方向的比如假人物阴影，反正是圆的。有方向的比如人物的脚印和魔法痕迹。

Decal还需要再细分为需要淡出的Decal和一直显示直至删除的Decal.比如脚印和点击地表出
出现的光标需要淡出，而人物的似阴影则需要一直显示。

Decal有位置和大小，创建时，给出位置和大小。在渲染Decal的过程中，可以不断的调整
Decal的位置和大小(用于渲染人物的假阴影,但调整大小的接口不需要对上层开放)。


Decal应该有一个管理器，跟地形相关联，FrameMove和draw

1: 先算出decal在x,z方向上的boundSquare大小。
2: 算出影响decal的格子。
3: 根据decal影响的格子，剪切出此decal顶点和由这些顶点组成的三角形。
4: 计算每一个decal顶点对应的uv坐标(Decal的uv坐标都是从0到1)。


使用同一个Shader的Decal使用同一个DIP来处理。

地表的Cursor也使用Decal来统一处理，把点击地表出现光标的代码去掉。

所有的decal使用动态填充的顶点缓冲区??索引缓冲区也使用动态的索引.


*/


//! 定义使用
typedef std::vector<os_billBoardVertex>   VEC_decalVer;

class   osc_decalMgr;

//! 一个decal最多影响的tile数目。
# define MAX_TILE_PERDECAL  256

/** \brief
 *  地表上可以显示的decal数据结构.
 *
 *  所有Decal内的顶点都是世界空间的顶点。
 */
class osc_decal : public os_createVal
{
	friend class   osc_decalMgr;
private:
	//! 场景管理和中间管道的指针.
	static osc_TGManager*   m_ptrTGMgr;
	static osc_middlePipe*  m_ptrMPipe;

	//! 创建decal时，存放decal剪切tile格子的四个面。
	static osPlane         m_arrDecalPlane[4];


	//! Decal用到的顶点和渲染数据
	VEC_decalVer       m_vec3DecalVer;
	VEC_word           m_vecDecalIdx;
	int               m_iDecalIdxnum;

	//! 当前decal的textureId.
	DWORD             m_dwDecalColor;

	bool              m_bAniDecal;
	//! 从开始显示decal到目前过去的时间。
	float             m_fEtime;
	//! decal消失的时间
	float             m_fVanishTime;

	//! 是否是旋转的Decal
	float             m_fRotAgl;

	//! Decal的位置和大小.位置位于decal的中心点。大小是半径。即decal宽度的一半。
	osVec3D           m_vec3DecalPos;
	float             m_fDecalSize;

	//! 旋转后的u和v source Plane,每一个顶点到此面的矩离，除以decalSize后为u.
	static osPlane     m_sUSourcePlane;      
	static osPlane     m_sVSourcePlane;

	//! 渲染当前的Decal使用到的数据
	os_polygon         m_sRenderData;

	//! 当前Decal的offset值
	float              m_fDecalOffset;

	//! 当前Decal的纹理动画相关的数据
	WORD               m_wXFrameNum;
	WORD               m_wYFrameNum;
	float              m_fFrameTime;

	//! 当前动画时间刻，此Decal纹理所在的区间
	WORD               m_iCurXFrame;
	WORD               m_iCurYFrame;
	float              m_fAcctFrameTime;

	//! 当前的decal是否隐藏
	bool               m_bHide;

	//! 当前decal跟背景的熔合方式.
	int                m_iAlphaType;

	//! 是否fakeDecal.
	bool               m_bFake;

	//! 当前的decal是否正处于渲染动画中。
	bool               m_bInRender;

private:
	//! 处理得到decal用到的四个剪切面
	void             get_decalClipPlane( osPlane* _planeArray = NULL );

	/** \brief
	 *  Clip一个Tile格子三角形，生成新的Decal使用的三角形列表。
	 *
	 *  \param osVec3D* _pos 传入的三角形三个顶点，顺时针排列。
	 */
	void             clip_tileTriangle( const osVec3D* _pos );

	//! TriangleFan的索引变成TriangleStrip的索引后，索引数目的变化。
	int              triFanNum2TriStripNum( int _tfIdxNum );

	//! 计算切分顶点后，每一个decal顶点的uv坐标
	void             cal_decalVerUv( void );
	//! 计算有纹理动画的顶点uv坐标
	void             cal_decalVerUvTexAnim( void );


	//! 用于存储decal影响格子的临时缓冲区,存储了tile在世界空间的坐标，会超过64
	static os_tileIdx    m_arrTileIdx[MAX_TILE_PERDECAL];
	static int          m_iAffectNum;


	//! 根据一个中心点和一个Size,得到受影响的Tg格子索引
	int              get_affectTile( osVec3D& _pos,float _size );
	int              get_affectTileWithRot( osVec3D& _pos,float _size,float _rotAgl );

	//! 更新此decal的纹理动画信息
	void             frame_texAnim( float _etime );

	//! 创建非跟随地形的普通decal.
	void             move_fakeDecal( osVec3D& _pos,float _rot,float _offset = 0.0f );

public:
	osc_decal();
	~osc_decal();

	void       reset_decal( void );


	//! 初始化一个decal,返回Decal的创建值，删除Decal时，必须确保删除的值和创建值相同。
	WORD       init_decal( os_decalInit& _dinit,int _shaderId = -1 );

	//! 重设decal的旋转和位置信息
	void       move_decal( osVec3D& _pos,float _rotAgl );
	//! 重设Decal的位置信息
	void       move_decal( osVec3D& _pos ) { move_decal( _pos,m_fRotAgl ); } 

	//! 对Decal进行FrameMove.计算decal的animation信息
	bool       frame_move( float _etime );

	//! 设置当前的decal是否可见
	void       set_hide( bool _hide ){ m_bHide = _hide; }

	//! 当前的decal是否处理渲染中.
	bool       is_inRender( void ) { return m_bInRender; } 

};


//! 场景内最多的scene cursorNum.
# define MAX_SCENECURSORNUM   30
/** \brief
 *  decal的创建管理，所有decal相关的功能，都在这个管理器内创建。
 *
 */
class osc_decalMgr
{
private:

	//! 填充顶点用到的顶点缓冲区id.

	//! 填充索引用到的索引缓冲区id,画的时候使用TriangleStrip,
	//! 使用Degenerate Triangle,把分散的多边形连接起来

	CSpaceMgr<osc_decal>      m_arrDecalIns;

	//! 点击场景的光标用到的Decal
	osc_decal                 m_sSceneCursor[MAX_SCENECURSORNUM];

	// 渲染Decal的meshSet.
	static os_polygonSet      m_sPolySet;

	//! fake shadow的shaderID.
	static int                m_iFakeShadowShaId;

private:
	//! 对整个的decalMgr进行FrameMove

	//! 对要渲染的Decal结构指针进行排序.


public:
	osc_decalMgr();
	~osc_decalMgr();

	//! 把当前的decalMgr跟tgMgr相关联系起来.
	static void  OneTimeInit( osc_TGManager* _tgMgr,osc_middlePipe* _pipe );

	//! 为sceneCursor创建一个特殊的Decal,为的是可以在场景内正确的显示光标
	void       create_sceneCursor( os_decalInit& _dinit );

	/** \brief
	 *  创建一个decal
	 *
	 */
	int         create_decal( os_decalInit& _dinit );

	//! River @ 2011-2-11:加入fakeShadow创建代码，走一个捷径流程，使人物出现的更快.
	int         create_fakeShadow( os_decalInit& _dinit );

	//! 删除一个decal.
	void        delete_decal( int _id );

	//! 隐藏一个decal.
	void        hide_decal( int _id,bool _hide );


	//! 重设一个decal的位置
	void        reset_decalPos( int _id,osVec3D& _pos );


	//! 渲染当前的decalMgr.
	void        render_decalmgr( os_FightSceneRType _rtype );

	//! 渲染场景内的光标.
	void        render_sceneCursor( void );

	//! Release All the Decal.
	void        release_allDecal( void );


};
