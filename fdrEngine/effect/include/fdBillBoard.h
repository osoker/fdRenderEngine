//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdBillboard.h
 *
 *  Desc:     处理引擎中的billBoard,用于显示场景中的特效.
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

# include "../../mfpipe/include/osShader.h"                 // 使用ShaderMgr创建相关的设备相关资源。
# include "../../mfpipe/include/middlePipe.h"
//


// 关于管理器的使用和渲染可以参考ObjectManager中的做法。
//
// 注意有两种bill Board,一种是真正的billBoard,一种是仅对一个轴起作用的billBoard.
// billBoard每一个阶段应该对应一张贴图.每一张贴图动画的时间和动画的方式.
// billBoard每一个阶段的大小.billBoard对应贴图的熔合方式等等.
// 
// billBoard的创建分为两种类型,一种是永久性的在场景中存在,一种是只播放一个或是多个动画
// 周期的billBoard.永久性存在的billBoard应该在调入场景的时候调入,而只播放几帧的billBoard
// 应该在场景中动态的加入.
// 
// 应该有一个billboard编辑器去编辑billBoard相关的信息。
// 公告板使用的纹理文件名从xxx1.tga开始，直到得到我们需要的帧数。如果我们需要三帧动画的话，
// 就是xxx1.tga,xxx2.tga,xxx3.tga
//
// 公告板渲染时的状态：顶点颜色＊纹理颜色 = 最终需要渲染的颜色。
//                     



/** \brief
 *  BillBoard对应的类型.
 */
enum ose_BillBoardType
{
	/** \brief
	 *   在xz平面上显示的billBoard,这种方式几乎不能算做是billBoard.
	 *
	 *   可以做出地面上显示的魔法扩张效果。
	 */
	BILLBOARD_XZ = 0,

	/** \brief
	 *  在Y轴上旋转,但永远对着相机的billboard,简单的billBoard,
	 *  这种billBoard可以做火光，闪电，树等效果。
	 */
	BILLBOARD_Y = 1,


	/** \brief
	 *  永远朝着相机的billBoard.是真正的公告板。
	 *  
	 *  可以做出魔法,光球等效果。
	 */
	BILLBOARD_ALL = 2,


	/** \brief
	 *   十字交叉面片的billBoard，实际上算不上是完全的BillBoard.
	 *   
	 *   保留类型。目前可以先不支持。
	 */ 
	BILLBOARD_CROSS = 3,


	/** \brief
	 *  绕某个轴线旋转的billBoard,主要用于刀光效果的制做。
	 *
	 *  这个轴线是完全自定义的轴线,在编辑器内调整时，默认使用Y轴
	 *  在程序内，可以随意的设置这个轴线
	 */
	 BILLBOARD_AXIS = 4,


};


/** \brief
 *  公告板文件的文件头结构。
 *  
 *  
 */
struct os_bbHeader
{
	char  m_chFlag[3];
	DWORD m_dwVersion;
};


//! 公告板最多支持的动画帧数.
# define MAX_BBTEXNUM    12


//! 巨型公告板的开始id
# define HUGEBB_IDSTART  4096


/** \brief
 *  由这个类生成和播放场景中所有的公告板相关的物品。
 * 
 *  需要公布由外层使用的接口。billBoard的默认位置是在(0,0,0)点。
 * 
 *  需要使用最近最少使用法来释放最近最少使用的billboard，主要是纹理占用的显存。
 *  
 */
struct osc_billBoard : public os_lruEle
{
	//! 当前billBoard对应的文件名。
	char      m_szBFilename[128];
	DWORD     m_dwHash;

	//! 循环播放的billBoard。另外一种方式是只播放一次的billBoard.
	BOOL      m_bLoopBB;

	//! 播放一帧用到的时间。
	float     m_fFrameTime;

	//! 总共的播放帧数。
	int       m_iTotalFrame;

	//! 播放当前的公告板总共需要的时间.
	float     m_fTotalPlayTime;

	//! 公告板中总共用到的贴图的数目。
	int       m_iTotalTexNum;

	/** \brief
	 *  公告板用到的纹理的名字。
	 *  贴图的名字使用第一帧贴图的名字，只能使用*.tga做为贴图。
	 */
	char      m_strTextureName[128];

	//! 公告板每一帧对应的贴图的id.
	VEC_int       m_vecTexId;

	//! 公告板的大小,每一帧有一个大小。位于两帧之间时，使用两帧的线性插值。
	VEC_float     m_vecBBSize;


	/** \brief
	 *  公告板大小的调整值，第一值为宽度的调整，第二个值为高度的调整。
	 *  
	 */
	osVec2D          m_vecBBSizeAdj;


	//! 公告板的颜色，每一帧有一个颜色。
	//! 在播放的时候，两帧之间的颜色使用两个颜色的线性插值。
	VEC_dword       m_vecBBColor;


	//! 纹理重复的数目，用于一张公告板上显示纹理的重复效果,可以做出简单的水的效果。
	float          m_fTexScale;

	//
	/** \brief
	 *  渲染方式，有两种渲染方式：1：和背景混合。2：和背景相加。3:和背景混合相加
	 *
	 *  和背景相加的方式设置alpha状态大致为： 
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	 *   m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
	 *   m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );
	 * 
	 *  和背景混合的方式设置Alpha的状态大致为:
	 *
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1 );
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	 *   m_pd3dDevice->SetTextureStageState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
     *   m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA  );
	 *   
	 *  和背景混合相加的状态：
	 *   m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
	 *   m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
     *   m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );
	 *  
	 */
	WORD      m_wAlphaType;


	//! 公告板的类型。
	ose_BillBoardType  m_billBoardType;

	//! 当前的billboard数据是否在使用中。
	BOOL              m_bInuse;

	//@{
	//  使用billBoard可以按照一定的路线运动。
	bool              m_bUseRunTrack;


	/** \brief
	 *  如果billBoard按照路线文件进行移动的话，对应的文件名。
	 *
	 *  Track文件必须是以原点为基准。
	 *  
	 */
	char          m_strTrackFname[128];

	//! billBoard在路线上的移动速度。
	float         m_fTrackMoveSpeed;
	//@}


	/** \brief
	 *  设备相关的shaderId。此处存储的是最开始的shaderId,在一个billBoard
	 *  数据结构中，应该有m_strTextureName个shaderId.
	 */
	int           m_iShaderId[MAX_BBTEXNUM];

	//@{
	//! 公告板旋转相关的数据。
	BOOL          m_bRotOrNot;
	float         m_fRotSpeed;
	int           m_iRotDir;
	//@} 

	//! 是否可以置换出内存.重要的公告板,需要永久性的停留在内存中.
	bool          m_bCanSwap;

	//! 是否ZEnable.
	BOOL          m_bZEnable;

private:
	//! 初始化当前的公告板数据
	void          reset_bb( void );


public:
	osc_billBoard();

	/** \brief
	 *  从文件中调入一个billBoard.
	 */
	bool      load_billBoardFromFile( const char* _fname );


	//! 释放一个公告板文件.
	void      release_billBoard( void );

	//! 是否是巨形公告板
	bool             is_hugeBillBoard( void );


};

//! 使用std::vector来组织billBoard. 
typedef std::vector<osc_billBoard>  VEC_billBoard;



//! 声明中间管道类。
class      osc_middlePipe;



/** \brief
 *  外层创建的billBoard的实例。billBoard Instance.
 *
 *  上层操作是是billBoard实例。BillBoard数据是可以重用的数据，
 *　设计这个结构为可以重用的结构，当一个billBoard播放完成后，可以使用这个结构
 *  去播放下一个BillBoard.
 */
class osc_billBoardIns : public os_createVal
{
	friend class         osc_bbManager;
private:

	//! 从文件中调入的billBoard的数据指针。
	osc_billBoard*        m_ptrBBData;

	//! 当前billBoard在世界空间中的位置。
	osVec3D              m_vec3Pos;

	//! 当前的billBoard是否在使用中。
	bool                 m_bInuse;

	//! 当前BillBoard渲染时用到的数据。
	os_billBoardVertex    m_arrRVertex[4];

	/** \brief
	 *  播放公告板的时间加速比.
	 *
	 *  根据公告板正常播放完的时间和外部要求的公告板播放时间来算出这个速度比
	 *  使用这个速度比来处理每一帧frameMove时，传入的时间，从来可以在外部要求
	 *  的时间内播放完此公告板。
	 *  如果这个数据值为1.0，则是按公告板编辑时的时间来播放完动画。
	 */
	float               m_fTimeScale;

	//! 创建时公告板的基准缩放值，以后在缩放值都在这个基准之上缩放
	float               m_fBaseSizeScale;

	//! 公告板的大小缩放值
	float               m_fSizeScale;


	//@{
	// 以下为插值使用到的信息。

	//! 两帧之间线性插值用到的系数。
	float                m_fInteValue;

	//! 如果上一帧用到的时间不够切换到下一帧的话，流逝的时间存储到这个值中。
	float                m_fAccumulateTime;

	//! 当前播放到的帧索引。
	int                  m_iCurFrame;


	//! 当前instance用到的track的id.
	int                  m_iTrackId;
	//! 经过track计算后的最终billBoard应该到达到的位置数据。
	osVec3D              m_vec3Fpos;

	//@}


	//! 当前billBoard的bounding sphere.
	os_bsphere           m_bsphere;

	//! 当前公告板实例的旋转角度。
	float                m_fRotAgl;

	//! 使用定制的颜色.
	bool                 m_bUseCustomColor;
	DWORD                m_dwCustomColor;

	//! 当前bb是否可见。在Framemove中判断可见性，渲染时用。
	bool                m_bInview;

	//! 当前的bb是否是隐藏状态
	bool                m_bHide;

	//! 当前bb是否顶层渲染
	bool                m_bTopLayer;

	//! River added @ 2006-6-29: 
	//! 对于绕某个特定的轴线旋转的公告板，此值为所要绕的轴,默认使用Y轴.
	osVec3D            m_vec3BBRotAxis;

	//! River added @ 2011-1-30:任意线程可指定此bb是否释放。
	bool               m_bInReleaseState;
private:

	//! 初始化billBoard　instance的动画数据。
	void             init_bbAniData( void );

	/** \brief
	 *  当前的billBoard动画进行处理。
	 *
	 *  \return bool 如果返回false,则表示当前的bb已经释放
	 */
	bool             frame_moveBBIns( float _etime,bool _canReleaseIns = false );

	
	//! 得到公告板实例数据的UP&Right Vector.
	void             get_bbUpRightVec( 
		               I_camera* _camptr,osVec3D& _up,osVec3D& _right );

	//! 根据billBoard和instance数据来填充我们渲染用的顶点数据。
	bool             fill_renderVerData( I_camera* _camptr );

	//! 释放当前的billboard,使当前的billBoardIns变为可用的Instance.
	void             release_billBoardInsInner( void );


public:
	osc_billBoardIns();

	/** \brief
	 *  初始化当前的billboard Instance.
	 *
	 *  \param bool _createIns 如果此值为false,则仅仅把billBoard相关的资源调入硬盘,
	 *                         实例并不创建,使用下一次创建实例时加快速度.
	 *  \param DWORD _cusColor 是否使用定制的颜色.如果此值大于0,则不使用公告板文件中
	 *                         对应的公告板颜色,使用此颜色值,但使用公告板文件中对应的
	 *                         公告板alpha值.
	 *  \param float _sizeScale  公告板是否缩放
	 *  \param bool _topLayer   是否顶层渲染
	 */
	WORD             init_billBoardIns( osc_billBoard* _bbdata,osVec3D& _pos,
		                  bool _createIns = true, DWORD _cusColor = 0,
						  float _sizeScale = 1.0f,bool _topLayer = false );


	/** \brief
	 *  渲染当前的billBoard Instance.
	 *
	 *  根据billBoard的不同类型，得出不同的billBoard渲染数。
	 * 
	 *  \param _pipe    渲染当前的billBoard时使用的中间管道。
	 */
	bool             render_billBoardIns( osc_middlePipe* _pipe );

	//! 从bbIns中得到要渲染的shaderId,用于优化的BB渲染模式。
	int              get_bbInsRShader( void );


	//! 释放当前的billboard,使当前的billBoardIns变为可用的Instance.
	void             release_billBoardIns( bool _finalRelease = false );

	/** \brief
	 *  根据公告板的类型来设置当前billBoard的位置。
	 *  
	 */
	void             set_billBoardPos( osVec3D& _pos );

	//! 设置公告板的播放时间.
	void             set_bbPlayTime( float _timeScale );

	//! 得到公告板的播放时间.
	float            get_bbPlayTime( void );

	//! 是否是循环播放的公告板
	bool             is_loopPlayBB( void ) { return m_ptrBBData->m_bLoopBB; }

	/** \brief
	 *  当前的billBoard是否在使用中。
	 */
	bool             is_billBoardInUsed( void )  { return this->m_bInuse; }

	//! 设置公告板的旋转轴
	void             set_bbRotAxis( osVec3D& _axis ) { m_vec3BBRotAxis = _axis; }

};

//! 使用std::vector来组织billBoard的实例数据。
typedef std::vector<osc_billBoardIns>  VEC_billBoardIns;

//! 如果有
/** \brief
 *  billBoard的管理器。
 *  
 *  这个管理器管理了当前场景中用到的billBoard和Billboard实例。
 *
 *
 *  阴影和公告板分开是因为阴影的渲染需要在场景的地表 
 *  渲染之后，其它物品渲染之前，需要关闭ZBuffer.
 *  
 */
class osc_bbManager : public ost_Singleton<osc_bbManager>
{
	//! 这个需要常常使用管理器中的billBoard vector.
	friend  class    osc_billBoardIns;

private:
	//! 管理器中的billBoard数据。
	CSpaceMgr<osc_billBoard> m_vecBillboard;

	//! 管理器中的billboardInstance
	CSpaceMgr<osc_billBoardIns> m_vecBBIns;

	//! 管理器内的不受远剪切面影响的巨型公告板实例
	CSpaceMgr<osc_billBoardIns> m_vecHugeBBIns;

	//! 下雨下到地上效果的billBoard.
	CSpaceMgr<osc_billBoardIns> m_vecRainEffect;


	//! 当前管理器保存的中间管道指针。
	osc_middlePipe*   m_ptrMPipe;


	//! 当前帧渲染的面数。
	int              m_iRenderTriNum;


	//! 渲染当前的bbMgr时，可以用到的静态索引缓冲区。
	int              m_iIdxBuf;
	//! 渲染当前的bbMgr时，可以用到的动态顶点缓冲区。
	int              m_iVerBuf;


	//! 第一次运行雨滴效果
	bool             m_bFirstRainEff;
	//! 第一次运行人物简单阴影.
	bool             m_bFristShadow;

	//! 每隔一段时间，释放一些不用的billboard.
	float            m_fDiscardBBTime;

	//! 用于创建雨滴特效时的公告板位置
	static I_fdScene* m_ptrScePtr;

	//! 全局的camera指针
	I_camera*         m_ptrGlobalCam;

private:

	//! 从一个bb文件名,得到billBoard对应内存结构的指针.
	osc_billBoard*    get_billBoardPtr( const char* _fname );

	//! 设置billBoard的lru.
	void              set_bbMgrLru( void );
	//! 释放最近一段时间没有使用的billboard.
	void              discard_lruBB( void );

	//! 渲染RainEff用到的polygon.
	os_polygon         m_sRainEffPoly;
	//! 第一次初始化RainEff相关的数据。
	void              first_initRainEff( void );


	osc_bbManager();
	~osc_bbManager();

public:

	/** 得到一个sg_timer的Instance指针.
	 */
	static osc_bbManager*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void         DInstance( void );


	//! 初始化billBoardManager.
    void             init_bbManager( osc_middlePipe* _pipe );


	//
	//! 创建一个billBoardInstance.
	int              create_billBoard( const char *_bfile,osVec3D& _pos,
		                  bool _createIns = true,float _sizeScale = 1.0f,bool _topLayer = false );

	/** \brief
	 *  创建一个不受剪切面的影响的billBoard
	 *
	 *  这种类型的公告板主要用于全局的场景特效，用于做出宏大的效果出来。
	 */
	int              create_hugeBillBoard( const char *_bfile,osVec3D& _pos,
		                  bool _createIns = true,float _sizeScale = 1.0f );

	/** \brief
	 *  控制billBoard的位置。一次性运行的公告板，不能控制位置。
	 *
	 *  \param _id 　要设置的公告板的id。
	 *  \param _pos  要设置的公告板的位置。
	 */
	void              set_billBoardPos( int _id,osVec3D& _pos,bool _updateImm = false );

	/** \brief
	 *  即时更新当前id公告板实例的位置
	 *
	 *  用于更新在场景整体的FrameMove后，又想准确得到公告板位置的情况，
	 *  比如武器上用到的公告板。
	 */
	void              update_bbPos( int _id,I_camera* _camptr );


	//! 设置公告板的播放时间。
	void              set_bbPlayTime( int _id,float _timeScale );

	//! 设置公告板的旋转轴
	void              set_bbRotAxis( int _id,osVec3D& _axis );


	//! 得到公告板的正常播放时间。
	float             get_bbPlayTime( int _id );


	/** \brief
	 *  删除billBoard.
	 *
	 *  \param _id 删除指定id的billBoard.
	 */
	void              delete_billBoard( int _id,bool _finalRelease = false );

	// River @ 2010-12-24:为了上层少进行删除创建的操作。
	const char*       get_bbNameFromId( int _id );

	/** \brief
	 *  对整个billboard中需要处理的billBoard处理，为渲染准备数据。
	 *  
	 *  这个函数内包含对shadow相关bb的数据填充处理。
	 */
	void             frameMove_bbMgr( I_camera* _camptr );

	//
	/** \brief
	 *  渲染整个管理器中可见的billBoard.
	 *
	 *  \param _camptr    场景中激活的相机。
	 */
	bool             render_bbManager( I_camera*  _camptr,os_FightSceneRType _rtype = OS_COMMONRENDER );

	//! 渲染巨型公告板，在天空渲染的阶段渲染
	bool             render_mgrHugeBB( I_camera* _camptr );



	/** \brief
	 *  渲染阴影公告板。
	 *
	 *  所有的阴影bb都是xz方向上的bb,所以不需要相机。
	 */
	bool             render_raindropBB( I_camera* _camptr );
	//@} 

	



	//@{
	//  雨滴效果落到地面的效果。
	//! 创建雨滴效果。
	void            create_rainEff( osVec3D& _vec );
	//@} 

	//! 验证当前的billBoardId是否有效。
	bool             validate_bbId( int _id );

	//! 当前的公告板是否是循环播放的公告板,对于武器特效,必须使用循环播放的公告板.
	bool             is_loopPlayBB( int _id );

	//! 隐藏或显示当前的bb.
	void             hide_bb( int _id,bool _hide );

	//! 缩放当前的公告板.river added @ 2010-1-7:
	void             scale_bb( int _id,float _rate );


	//! 得到当前帧渲染的bbMgr面数。
	int            get_bbMgrRenderTriNum( void )    { return m_iRenderTriNum; }

	//! IdentityMat
	static osMatrix   m_smatMatrix;


};



//! 包含当前billBoardMgr对应的inline函数。
# include "fdBillBoard.inl"







