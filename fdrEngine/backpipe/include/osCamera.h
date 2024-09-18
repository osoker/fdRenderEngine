//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osCamera.h
 *
 *   Desc:     实现Camera的接口,同时加入osok引擎内部需要的Camera接口.
 *          
 *   His:      River Created @ 4/18 2003.
 *
 *   "不要轻易改变花了很长时间做出的决定,那样就等于是背叛了自己的信念".
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"


/** \brief
 *  使程序的clipPlane远一些,增加场景真实性.处理物品的可见性时,
 *  照常使用旧的clipPlane.
 */
# define CLIPDIS_ADD     60.0f

//! 　在视矩的一半矩离以外，不可视物品,出现此种情况，需要淡出物品。
# define MASK_OUTHALF_NOTVIEW  0x01
//! 　在视矩一半矩离内的可视物品。
# define MASK_HALF_VIEW       0x02
//!   在视矩一半矩离外的可视物品。
# define MASK_OUTHALF_VIEW    0x04

//! 必须autoReset通过以下的时间后，才能真正开始autoReset
# define  AUTORESET_STARTTIME 0.25f



class osc_TGManager;
class osc_middlePipe;
class osc_camera;

/** \brief
 *  Osok引擎的View frustum类.
 *
 *  结构中有最简单的函数,用于判断bsphere or bbox or point是否在
 *  这个view frustum内部.
 *
 */
struct  os_ViewFrustum
{
	//
	// 前四个顶点为近剪切面，后四个顶点为远剪切面顶点。
	osVec3D     m_FrustumVer[8];

	/** \brief
	 *  这些面是Frustum的面,朝向Frustum内部.
	 */
	osPlane     m_Frustum[5];


	//! 当前vf的aabbox,用于精确的检测aabbox是否跟vf相交
	os_aabbox   m_sAabbox;

	//! 检测一条射线是否和左，右，上，下，远剪切面相交，是三角形的精确相交
	BOOL        ray_intersectLeftClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectRightClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectTopClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectBottomClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectFarClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );


	//! 一个射线起点，一个长度，一个方向，但到新的顶点
	void        get_pt( osVec3D& _s,osVec3D& _dir,float _len,osVec3D& _res );

	//! 给出一个开始点和结束点，找到跟clip面的交点
	BOOL        get_clipPt( osVec3D& _s,osVec3D& _e,osVec3D& _res );

public:
	os_ViewFrustum();


	/** \brief
	 *  查看一个点是否在一个view frustum内
	 */
	bool        objInVF( osVec3D& _vec );
	
	/** \brief
	 *  查看一个bounding sphere是否在一个view frustum内
	 */
	bool        objInVF( os_bsphere& _bs );

	/** \brief
	 *  查看一个bounding sphere是否在一个view frustum内
	 */
	bool        objInVF( osVec3D& _vec,float _rad );

	/** \brief
	 *  查看一个bounding box是否在一个view frustum内
	 */
	bool        objInVF( os_bbox& _bb );



	//! 查看一个aabbox是否在一个vf内.
	bool        objInVF( const os_aabbox& _box );

	//! 精确检测vf是否与aabbox相交
	bool        objInVFAccurate( const os_aabbox& _box );

	/** \brief
	 *  用于检测一个aabbox跟vf的关系
	 *
	 *  \return bool aabbox全部在vf内，返回true.相交返回false.
	 *                     全部在vf外情况由objInVF函数判断
	 */
	bool        objCompleteInVF( const os_aabbox& _box );


	/** \brief
	 *  over load the operator =
	 */
	os_ViewFrustum& operator=( const os_ViewFrustum& _v )
	{
		memcpy( m_FrustumVer,_v.m_FrustumVer,sizeof( osVec3D )*8 );
		memcpy( m_Frustum,_v.m_Frustum,sizeof( osPlane )*5 );
		m_sAabbox.m_vecMax = _v.m_sAabbox.m_vecMax;
		m_sAabbox.m_vecMin = _v.m_sAabbox.m_vecMin;

		return *this;
	}

	//! 此函数把一个线段clip到视域内,可能修改两个顶点
	bool        clip_lineSeg( osVec3D& _s,osVec3D& _e );

	//  TEST CODE:
	//! 画出Vf的远剪切面，要比实际的远剪切面近一点
	void        draw_farPlane( osc_middlePipe* _pipe );

	//! 检测一条射线是否跟相机的近剪切面相交。是精确的两个近剪切面三角形，而非近剪切面的平面。
	BOOL        ray_intersectNearClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );


	//! 输出调试信息
	void        output_debugInfo( void );

};


//! 备份当前相机的数据，可以随时间恢复到原来的相机
struct os_cameraDataBackup
{
	//! 相机在世界空间的位置信息
	osVec3D       m_vec3CamPos;
	osVec3D       m_vec3CamFocus;

	float         m_fCurPitch;
	float         m_fCurYaw;

	//@{
	/**
	 *  用于设置ProjMatrix的数据.
	 */
	float        m_fNearp;
	float        m_fFarp;
	float        m_fFov;
	float        m_fAspect;
	//@}

	//@{
	/**
	 *  用于设置ViewPort的数据.
	 */
	int          m_iTlx;
	int          m_iTly;
	int          m_iWid;
	int          m_iHei;
	float        m_fMin_z;
	float        m_fMax_z;
	//@}	

};


/** \brief
 *  自动回位状态管理器
 *
 *  相机的自动运动只会绕focus顶rot vertical,所以只需要记录相应的数据.
 */
class osc_autoResetMgr
{
private:
	bool      m_bInARState;

	//! 离相机的矩离
	float     m_fPosFocusDistance;

	//! 回位的目标pitch.
	float     m_fDestPitch;

	//! 自动回位的速度
	float     m_fResetSpeedPerSec;

	//! 规定相机必须在多长时间的自动回位检测通过后，才能在画面上产生真正的自动回位功能。
	float     m_fAutoResetPassTime;

	//! 上一帧移动的矩离
	float     m_fLastFrameMove;
	
	//！如果相机的rot改变，则开始reset
	bool      m_bPauseAutoset;

public:
	osc_autoResetMgr();

	//! 开始自动回位状态
	void      start_autoResetState( float _dis,float _pitch );

	/** \brief
	 *  计算自动回位的位置.
	 *  
	 *  \param _focus  相机的焦点位置.所有的自动回位都是以新的焦点为中心.
	 *  \param _curPos 传入的当前相机的位置.
	 *
	 *  算法描述:
	 *  1: 根据传入的位置,计算出当前相机的pitch
	 *  3: 根据相机的回位速度,来处理得到自动相机回位应该到达的位置.
	 *  4: 上层判断新的位置是否合法,如果合法设置新的位置,否则等待下一帧来再次调用.
	 */
	float     get_autoResetPos( osVec3D& _focus,osVec3D& _curPos );

	//! 相机自动回位的处理
	void      process_camAutoResetPos( osc_camera* _cam,float& _move );

	//! 当前是否处于自动回位状态.
	bool      is_autoResetState( void ) { return this->m_bInARState; }

	//! 结束自动回位的状态.
	void      end_autoResetState( void ) { m_fAutoResetPassTime = 0.0f;m_bInARState = false; }

	//! 设置当前帧通过自动回位测试的情况.
	void      set_autoResetPass( bool _pass );

	//  
	//! 暂停autoReset.
	void      pause_autoReset( bool _ar ) { m_bPauseAutoset = _ar; } 

	//! 得到上一帧移动的位置。
	float     get_lastMoveLength( void ) { return this->m_fLastFrameMove; } 

	//! 当前autoReset是否已经过了autoResetTime
	bool      pass_autoResetTime( void ) { return m_fAutoResetPassTime > AUTORESET_STARTTIME; }


	//@{
	//  River @ 2010-1-9:用于解决切换战斗场景时，相机回弹的位置不正确的bug.
	//! 把当前的AutoResetState数据保存到另一个实例内。
	//! 
	const osc_autoResetMgr& operator=( osc_autoResetMgr& _mgr );
	//@} 


};

//! 从相机视矩的这个位置开始，地表开始和背景融合。
//! River @ 2006-4-5:此变量不在用于地表跟背景的融合，但远处物品的可见性数据跟这个相关
# define  TA_SCALE   0.90

/** \brief
 *  Camera接口的实现类.
 *
 *  这个类实现了Camera接口,并且加入Osok引擎内部需要的一些接口和操作.
 *
 *  ATTENTION TO OPP: 对相机记录当前相机的wvp矩阵，记录这个矩阵的反矩阵，
 *                 这样当每帧需要计算鼠标拾取信息的时候，可以减小计算量。
 *
 *  相机位置和旋转方向的插值动画：
 *  需要在相机中加入函数来支持插值动画。
 *  
 */
class osc_camera : public I_camera
{
public:

	/**********************************************************************/
	//
	//  初始化Camera的函数.
	//
	/**********************************************************************/
	/** \brief 
	 *  从一个结构中初始化一个Camera.
	 *
	 *  \param _cdata  从这个结构中初始化我们的Camera.
	 *
	 */
	void         init_camera( const os_cameraInit* _cdata );

	//! 得到相机可看到的远剪切面的矩离，即场景可见的最远矩离
	virtual float        get_farClipPlaneDis( void );
	virtual float		 get_nearClipPlaneDis( void );


	/**********************************************************************/
	//
	//  操纵Camera需要的接口.
	//
	/**********************************************************************/
	/** \brief
	 *  设置一个Camera的Projection Matrix
	 */
	virtual void         set_projmat( float _nearp,
		                    float _farp,float _fovh,float _faspect );

	//! 设置远剪切面.
	virtual void         set_farClipPlane( float _farp );

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
		                    int _wid,int _hei,float _min_z,float _max_z );

	/** \brief
	 *  重新设置camear的aspect.
	 *
	 *  \param _aspectAdd 当前相机的aspect加上这个数据。内部把_aspectAdd设置为
	 *                    改变后的aspect数据，并传出.
	 */
	virtual void         set_camAspect( float& _aspectAdd );



	//! 设置相机的fov值。
	virtual void         set_cameraFov( float _fov );
	//! 得到现有的相机视角宽度.
	virtual float        get_cameraFov( void );




	/** \brief
	 *  得到相机当前的指向的方向,不管相机处于何种模型,加入了方向的
	 *  分量.
	 */
	virtual void         get_godLookVec( osVec3D& _vec );



	/** \brief
	 *  得到当前相机的向右的方向分量.
	 */
	virtual void         get_rightvec( osVec3D& _vec ) ;



	/** \brief
	 *  设置相机当前的位置.
	 */
	virtual void         set_curpos( osVec3D* _vec,bool _force = false  );

	//! 2009-9-4:一次性设置位置和focus,用于解决一个很变态的屏幕晃动的bug.
	virtual void         set_curPosFocus( osVec3D& _pos,osVec3D& _focus,bool _force = false );


	/** \brief
	 *  得到相机当前的位置.
	 */
	virtual void			get_curpos( osVec3D* _vec );
	virtual const osVec3D&	get_curpos( void) ;
	
	/** \brief
	 *  使相机Pitch一个角度.
	 */
	virtual void         pitch( float _radian );

	/** \brief
	 *  使相机Yaw一个角度.
	 */
	virtual void         yaw( float _radian );

	/** \brief
	 *  使相机以God模式往前移动一段距离(距离可正可负).
	 *
	 *   执行步骤:
	 *
	 *   如果是GodMode
	 *   1: 如果需要更新Look vector,先构建新的Look vector.
	 *
	 *   2: 在Look vector的方向上移动要移动的距离.
     *  
	 *   如果是GenMode.
	 *	 1: 先构建Camera Look vector在水平方向上的Vector.
	 *	 
	 *   2: 在Look vector的水平方向上移动要移动的距离.
	 */	 
	//virtual void         move_forward( float _dis );


	/**********************************************************************/
	//
	//  使用相机中的数据进行一些常用量的计算.
	//
	/**********************************************************************/
	//@{
	/** \brief
	 *  得到屏幕上一个二维坐标在相机三维空间中的向量.
	 */
	virtual osVec3D      get_upprojvec( const osVec2D* _svec,osMatrix& _wmat );      
	virtual osVec3D      get_upprojvec( int _x,int _y,osVec3D& _dir );
	//@} 

	/** \brief
	 *  给出一个世界空间的顶点,给出当前世界空间顶点对应的屏幕空间的顶点.
	 *
	 *  \param _wpos 传入的世界空间的顶点坐标,此坐标为绝对坐标.
	 *  \param _spos 返回了世界空间顶点对应的屏幕坐标.
	 *  
	 */
	virtual bool          get_scrCoodFromVec3( const osVec3D& _wpos,osVec3D& _spos );


	//! 从场景中调入camera,仅表示相机的位置和旋转方向等数据，跟视口无关。
	virtual bool          load_camera( const char* _cname,
		                     bool _ani = false,float _time = 0.0f );

	/** \brief
	 *  对相机的旋转和Pitch做动画。
	 *  
	 *  普通用途：相机与地表碰撞时，相机可以柔和的到达新位置
	 *  旋转和pitch都是绕相机的焦点坐标进行。
	 */
	virtual bool          animate_camRotPicth( 
		                   float _rotRad,float _pitchRad,float _time );

	virtual void  set_autoResetState( bool _autoReset );

	/** \brief
	 *  机测一个顶点是否在相机的视域内
	 */
	virtual bool          is_ptInCamVF( osVec3D& _pos );


	/** \brief
	 *  限制俯仰角为一固定值
	 *
	 *  \param _rad 固定的俯仰角度值(弧度),如里此值小于等于零，则使用当前
	 *              相机的pitch值做为锁定的pitch值,
	 *              ATTENTION!!! 此_rad值为相对于Y轴，即UP轴的旋转角度，即
	 *              如果我们的想让相机抬的更直，相当于顶视图，则此值越小。
	 *              此值越大，表示相机越平。_rad的传入参数可以使用 
	 *              旋转角度/180* OS_PI
	 *              这个公式在客户端获得并传入引擎。
	 *  
	 */
	virtual void   limit_pitch( float _rad );

	/** \brief
	 *  限制俯仰角为一固定区间值
	 *
	 *  \param _rad     最直的相机视角.
	 *  \param _radMin  最平的相机视角.
	 *  \param _minFDis 相机离焦点可以达到的最近矩离，在此位置上，相机使用最平的视角。
	 *  \param _maxFDis 相机离焦点可以达到的最远矩离，在此位置上，相机使用最直的视角。
	 */
	virtual void   limit_pitch( float _rad,float _radMin,float _minFDis,float _maxFDis );

	/** \brief
	*  限制俯仰角为一固定值，之前的某个limit_pitch的值
	*/
	virtual void   limit_pitch( void );

	/** \brief
	*  取消限制俯仰角为一固定值
	*
	*  \param _rad 固定的俯仰角度值(弧度)
	*  
	*/
	virtual void   unlimit_pitch();
	
	/** \brief
	*  使相机振动的接口
	*  
	*  \param float _fDamp 相机振动阻尼.
	*  \param float _swing      相机的振幅.
	*/
	virtual void shake_camera( float _fDamp,float _swing );


	//! 得到相机viewFrustum的8个顶点
	virtual const osVec3D* get_vfPt( void );

	//! 保存autoResetState.
	virtual void  save_autoResetState( void );

	//! 恢复autoResetState.
	virtual void  restore_autoResetState( void );



# if __EFFECT_EDITOR__
	//! 存储场景中的camera位置数据。
	virtual bool          save_camera( const char* _cname );
# endif 



	//! 得到Tile开始透明的相机视矩。
	float                get_taDis( void ) ;


	//! 移动相机到一个位置上,避开障碍物,
	//! 如果传入的参数值小于零，则仅仅使相机在未来一段时间不自动回位。
	bool                 move_cameraToAvoidShelter( float _disToFocus );

	//! 备份相机的动画状态
	void                 back_upCamCaState( void );
	void                 reset_CamCaState( void );


public:
	//=====================================================================================
	//
	//  The following public func is the func which will used in inner 
	//  engine part.
	//  接下来的这些公有函数只可以用在引擎内部,不提供上层使用这些函数的接口.
	//
	//=====================================================================================
	osc_camera();

	/** \brief 
	 *  使用这个函数来得到ViewPort
	 * 
	 *  Set_viewport是初始化以后必须要调用的的函数，使用这个函数来得到ViewPort.
	 */
	void               get_viewport( D3DVIEWPORT9& _v );                                  

	//@{
	//!  View port releated func.
	osVec2D            get_screencenter( void );
	int                get_viewporttlx( void );
	int                get_viewporttly( void );
	int                get_viewportwidth( void );
	int                get_viewportheight( void );
	int                get_cameramode( void );
	void               get_nearplane( osPlane& _plane );
	void               get_farplane( osPlane& _plane );
    //@}

	// FIXME: USE THIS FUNCTION IN PORTAL ENGINE?????
	//!  得到这个camera的frustum.返回的plane指针指向6个面.
	os_ViewFrustum*    get_frustum( void );

	
	//!  Get the view matrix..
	void               get_viewmat( osMatrix& _mat,bool _reflect = false );	

	//!  Get the proj Matrix.
	//! 是否使用offset,使用场景看起来更加真实.
	void               get_projmat( osMatrix& _mat,bool _offset = true );

	//! 得到右手坐标的view mat.
	void               get_rhViewMat( osMatrix& _mat );

	//! 得到右手坐标的proj Matrix.
	void               get_rhProjMat( osMatrix& _mat );


	//!  得到水平方向上的look vec.
	void               get_horlookvec( osVec3D& _look );                       

	//!  得到屏幕空间的准星变化到世界空间的vec.
	osVec3D            get_upprojvec( void );                                 


	//!  得到远剪切面的矩离。
	float              get_farPlaneDis( void );

	//!  设置远剪切面的矩离.
	void               set_farPlaneDis( float _fdis );

	//!  得到当前相机的yaw
	float              get_curYaw( void );

	//!  得到当前相机的pitch值。
	virtual float       get_curPitch( void );


	//!  是否被相机看见，传入一个半径和位置。
	BYTE               is_viewByCamera( osVec3D& _pos,float _radius );

	//!  相机的frameMove函数，用于处理相机的动画。
	void                frame_moveCamera( float _etime );


	//!  设置和得到相机的焦点.
	virtual void         set_camFocus( const osVec3D& _focus );
	virtual void         get_camFocus( osVec3D& _focus )const ;
	virtual const osVec3D& get_camFocus( void)const ;

	//!　　前后移动相机
	void                move_close( float _length );

	/** \brief
	 *  获得相机的位置到焦点的距离
	 */
	float               get_disFromCamposToFocus();

	//!  在过去的一帧中，相机有无旋转。
	bool                is_rotIncurFrame( void ) { return m_bRotInCurFrame; } 

	//! 备份当前相机的位置和方向等所有的相机数据
	void                backup_curCam( void );
	//! 重用相机的备份数据
	void                use_backupCam( void );

	//! 得到此相机的位置和近剪切面形成的bsphere,用于相机相关的一些检测处理,比如跟主角人物相交时,不显示人物.
	bool                get_camNearClipBSphere( os_bsphere& _bs );

	//! TEST CODE:用于确认相机不跟地表有碰撞发生.
	void                 assert_camera( void );

private:
	
	//!  Contain the view matrix of the scene...
	osMatrix          m_viewmat ;  
	
	//!  右手坐标第的matrix.
	osMatrix          m_rhViewMat;


	//!  Range from -2pi--2pi  we can do more accurate if we use wide range.
	float             m_fCuryaw;                     

	//!  Range from   -pi/2   --    +pi/2
	float             m_fCurpitch;  

	//! River mod @ 2008-10-11:lock后的pitch数据
	float             m_fLockPitch;

	//! River mod @ 2009-4-9:加入pitch值在一个区间根据离相机位置离焦点远近而变化
	float             m_fLockPitchMin;
	float             m_fMinFDis;
	float             m_fMaxFDis;


	//!  相机的焦点,相机旋转时,此点不变.需要每一帧进行设置.
	osVec3D           m_vec3CamFocus;

	//@{
	/** 
	 *   以下坐标是象机的参照坐标.
	 */
	osVec3D           m_lookvec;
	osVec3D           m_rightvec;
	osVec3D           m_upvec;
	//@}


	//@{
	/** \brief
	 *  以下的坐标是相机在真实世界中的坐标.
	 */
	osVec3D           m_curpos; 
	osVec3D           m_curLookVec;
	osVec3D           m_curRightVec;
	//@}
	
	//! 当前相机的Common look vector,用于common mode,用于相机移动.
	//  使用函数get_horlookvec来得到这个值.
	// osVec3D           m_curComLookVec;

	bool              m_bShouldUpdate;


	//! 是否与地面进行碰撞检测
	bool m_bCammerLimitY;
	
	//@{
	//! View port releated var.
	int               m_iTopleftx,m_iToplefty;
	int               m_iWidth,m_iHeight;
	float             m_fMinz,m_fMaxz;
	//! Screen center.
	osVec2D           m_scenter;             
	//@}
	
	
	//@{
	//!  Projected matrix releated var.
	float             m_fFov;
	float             m_fNearPlane,m_fFarPlane;
	float             m_fHalfFar;

	//! 在这个矩离之外的物品肯定不可见。
	float             m_fObjFullView;
	float             m_fAspect;
	//@}
    

	//! 当前Camera的位置和focus位置之间的矩离.
	float                   m_fCamDisFocus;

    
	//!  当前Camera的frustum.
	os_ViewFrustum    m_Frustum;
	

	//@{
	// River added @ 2004-9-3:相机位置和方向间的插值动画。
	// ATTENTION : 以后加入队列式的相机插值动画。
	//! 当前是否相机插值动画状态。
	bool              m_bInCAState;

	//! 目标相机的参数。
	osVec3D           m_vec3NewPos;
	osVec3D           m_vec3NewLook;
	osVec3D           m_vec3NewRight;
	float             m_fNewPitch;
	float             m_fNewYaw;

	//! 旧的相机参数。
	osVec3D           m_vec3OldPos;
	osVec3D           m_vec3OldLook;
	osVec3D           m_vec3OldRight;

	float             m_fOldPitch;
	float             m_fOldYaw;
	//@} 

	//! 从插值动画到现在过去的时间。
	float             m_fEleTime;
	//! 当前插值动画总的时间。
	float             m_fCATime;

	//@{
	//! 用于相机位置之间的插值。
	bool              m_bInRotPitchLerpState;
	float             m_fTotalPitch;
	float             m_fTotalRot;

	float             m_fPitchPerSec;
	float             m_fRotPerSec;
	//@} 

	//syq 新的相机运动参数
	DWORD   m_dwVersion;
	osVec3D m_vec3OldCamPos, m_vec3NewCamPos;
	osVec3D m_vec3OldCamFocus, m_vec3NewCamFocus;

	//相机的最近最远距离
	float m_fMaxDis, m_fMinDis; 


	//! 地表格子开始透明的相机视矩。
	float             m_fTADis;


	//! 相机在当前帧中，是否有旋转的动作
	bool              m_bRotInCurFrame;


	//! 场景的接口
	osc_TGManager*     m_pScene;

	//! 用于备份当前相机位置和渲染相关结构的数据
	os_cameraDataBackup  m_sCamBackup;
	
	//! 如果自动回位的位置不合法,则尽量使用上一帧的相机位置,这样可避免相机的抖动现象发生.
	osVec3D              m_vec3LastFrameCamPos;
	//! 相机的自动回位功能管理器
	osc_autoResetMgr     m_sAutoResetMgr;

	//! 备份用自动回位数据
	osc_autoResetMgr     m_sBackUpARM;


	//! 相机动画状态的备份
	bool                 m_bBackCaState;

private:
	//=====================================================================================
    //
	//  osc_camera内部用到的帮助函数.
	//
	//=====================================================================================
	
	//! 修正角度值
	void               modify_pitch_yaw();

	/**
	 *  Update the camera.更新Camera中的各个Matrix.
	 *
	 *  \param bool _colwithTerr 是否跟地表做碰撞处理
	 */
	void               update_camera( bool _buildFrus = true,bool _colwithTerr = true,bool _reflect = false );

	/** \brief
	 *   使相机以General模式往前移动一段距离.
	 *
	 *   执行步骤:
	 *
	 *   1: 先构建Camera Look vector在水平方向上的Vector.
	 *
	 *   2: 在Look vector的水平方向上移动要移动的距离.
     */
	void               move_forward_godmod( float _dis );


	/** \brief
	 *   使相机以God模式往前移动一段距离(距离可正可负).
	 *
	 *   执行步骤:
	 *
	 *   1: 如果需要更新Look vector,先构建新的Look vector.
	 *
	 *   2: 在Look vector上移动要移动的距离.
	 * 
	 */
	void               move_forward_genmod( float _dis );


	/** \brief
	 *  使相机绕一个顶点进行水平旋转。
	 *　
	 */
	virtual ose_camOperateRS rotate_horizon( const osVec3D& _ver,
		                   float _radians,bool _force = false  );

	/** \brief
	 *  使相机绕一个顶点进行上下旋转。
	 */
	virtual ose_camOperateRS rotate_vertical( const osVec3D& _ver,
		                   float _radians,bool _force = false  );



	//! 对两种相机动画进行插值。
	void                lerp_cameraFile( void );
	void                lerp_cameraRotPitch( float _etime );

	/** \brief
	 *  构建当前Camera的frustum.
	 */
	void               build_camFrustum( bool _guaranteeFrums = false );

	//! 对自动回位后的相机位置进行处理,如果不对,则重设为旧的相机位置.
	void               process_camAutoReset( void );


	// 设置相机的位置与焦点的最大距离
	void set_maxDistance( float _maxDis );

	// 设置相机的位置与焦点的最小距离
	void set_minDistance( float _minDis );

	// 修改相机位置与焦点的距离在有效距离区间内
	void modify_disdance();

	//! 用转动相机的两个角度及焦点，求得相机的位置
	void cam_rot( float _pitch, float _yaw );


	//! 相机跟地表间的碰撞检测,在这个函数内,会根据地形对相机的位置进行自动位置处理.
	void modify_CollsionTerrain();

	//! 把相机焦点之上拉,即位于人物的头顶
	void pitch_camAboveFocus( void );

	//! 把相机的位置拉到许可的范围内
	void pull_camPosToMaxDis( void );

	//! pitchLock后，如果pitch有变动，一定时间后自动恢复为lock时的Pitch.
	void update_camera_toLockPitch( float _time );

	//! River @ 2009-4-9:根据最远最近矩离算出当前应该的pitch值
	float update_lockPitch( void );





	void play_camEffect( char * _camEffectName, int _nCntPerSec );
	void update_camEffect();
	std::vector< osVec3D > m_camEffectPoint;
	BOOL m_bPlayingCamEffect;
	int m_nCntPerSec;
	float m_fPlayEffectTime;
	osVec3D m_oldCamPos;
	osVec3D m_oldCamFocus;
	osVec3D m_oldLookVec;
	osVec3D m_offset,m_v3;

	BOOL m_bLockPitch;

	osVec3D mLastFramePos;
	BOOL m_NeedCollsionTerrain;

//---------------------------------------------------------------------------------
//@{ \brief : tzz add in tail of class for galaxy camera shaking @ 2008-6-11
//---------------------------------------------------------------------------------
private: //! member data of shaking
	//! swing of camera shaking
	float			m_fShakeSwing;

	//! damp of camera shaking
	float			m_fDamp;

	//! timer to calculate the
	float			m_fCurrTime;

	//! if shake?
	bool			m_bShaking;

private: //! private function to process shaking

	/*! \fn void ShakeCamera(void);
	*	\brief : shake camera when update camera
	*
	*	\param void
	*	\return void
	*/
	void ShakeCamera(void);

//---------------------------------------------------------------------------------
//@}
//---------------------------------------------------------------------------------

};



# include "osCamera.inl"






