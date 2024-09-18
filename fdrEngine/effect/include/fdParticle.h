//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdParticle.h
 *
 *  Desc:     场景中粒子特效。
 *
 *  His:      River created @ 2004-5-20.
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once 


# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"

//! 轨道相关的文件。
# include "fdTrack.h"


//! 用于处理每一个可以插值的元素.
# define  MAX_DIVISIONNUM   101

//! 每一个粒子场景最多有4个emissionControl.
# define  MAX_EMISSION      3

//! 定义每一个粒子控制中最多可以拥有的粒子的数目
# define  MAX_PARTICLENUM   100



//! 粒子场景的声明.
class         os_particleScene;
//! 象机类的声明.
class         osc_camera;


//
// 整理后的粒子文件和粒子编辑器中的文件有所不同，去掉了用处不大的部分，
// 使用结构更加清晰和可重用。
//
// 粒子的渲染使用中间管道的动态顶点缓冲区,每一帧中,处理顶点,填充中间管道动态顶点
// 缓冲区,并渲染这些顶点数据.

//
// River @ 2005-5-26:
// 关于粒子场景设定播放时间的方法，即让粒子加速播放的方法:
// 找到粒子场景的播放周期，然后使用传入的时间跟这个播放周期
// 比值，得到粒子播放的加速比。
//  
// 


/** \brief
 *  粒子运行控制类，对粒子产生器产生出粒子的生命过程进行控制的类。
 * 
 *  这个类也控制了粒子的显示方式。这个类是对粒子的整体控制，从粒子
 *  产生器产生出后，每一个粒子虽然是无序和随机运行的，但却有自己的
 *  运行规律，这个类表示了粒子运行的规律。
 */
struct os_particleRunCtl
{
	//! 粒子的重力加速度.
	float      m_fGravity;

	//! 粒子不受空气阻力影响的速度底限.
	float      m_fSpeedNoResis;

	//! 粒子面片面积大小受阻力影响的方式.0:没有 1:线性.
	int        m_iAreaSizeResisType;
	//! 粒子受面积大小影响的系数.
	float      m_fAreaSizeCons;


	//! 当前粒子对应的纹理文件名存储为相对路径.
	char       m_szTextureName[128];
	//! 当前粒子对应的纹理象素数目.16或是32是最常用的两种大小.
	float      m_fParticleYRot;

	//! 粒子对应动画的帧数,向右切分.
	int        m_iFrameNum;
	//! 粒子对应动画的套数,向下切分.
	int        m_iFrameSets;

	//! 当前粒子渲染时和背景的混合方式.0: 相加  1:混合.
	int        m_iBlendMode;

	//! 当前粒子的大小倍数.
	float      m_fSizeMulti;


	//@{
	//! 粒子生命周期大小的数据.
	float     m_vecParSize[MAX_DIVISIONNUM];

	//! 粒子生命周期旋转的数据.
	float     m_vecRotation[MAX_DIVISIONNUM];

	//! 粒子生命周期顶点的透明度.
	float     m_vecAlpha[MAX_DIVISIONNUM];

	//! 粒子生命周期的颜色.x代表r,y代表g,z代表b.
	osVec3D   m_vecColor[MAX_DIVISIONNUM];
	//@} 

	//! 当前的particle是否正在使用。
	bool      m_bInuse;



	//! 当前粒子的设备相关数据，shaderId号。
	int       m_iShaderId;

public:
	os_particleRunCtl();
	~os_particleRunCtl();


	//! 从内存中调入当前的particle数据.
	bool     load_particleRunCtl( BYTE*& _sdata,int _ver );

	//! 创建当前particle的设备相关数据，纹理id.
	bool     create_shader( void );


};
typedef std::vector< os_particleRunCtl > VEC_parRunCtl;


/** \brief
 *  粒子运行控制类的实例，每一个可见的粒子面片对应这样一个实例。
 *
 */
struct  os_parRunCtlIns
{
	//! 当前的particleInstance是否激活。
	bool         m_bAlive;

	//! 从粒子出生到当前过去的时间。
	float        m_fElapseTime;

	//! 当前粒子的生命周期长度.
	float        m_fLifeTime;

	//! 粒子在T轴上的值，用于得到粒子相关的其它数据。
	float        m_fTval;


	//! 当前粒子用的贴图帧。
	int          m_iTexFrame;
	//! 当前粒子用到的粒子贴图的套数索引。
	int          m_iTexSetsIdx;


	//! 粒子当前的加速度值。
	osVec3D      m_vec3Velocity;
	//! 粒子当前帧受到的阴力值。
	osVec3D      m_vec3Resistance;

	//! 粒子当前位置向量。
	osVec3D      m_vec3ParticlePos;

	//! 粒子当前的大小。
	float        m_fSize;
	//! 粒子材质当前Alpha值。
	float        m_fAlpha;
	//! 粒子当前颜色值，和alpha值合成，做为粒子最终的颜色值。
	osVec3D      m_vec3Color;
	//! 粒子当前旋转角度。
	float        m_fRotation;

public:
	//! 渲染相关的函数。
	void         reset_particle( void );

};


/** \brief
 *  粒子产生器的控制类。
 *
 */
struct os_particleEmitter
{
	//! 当前emitter产生粒子的的相对位置,相对于(0,0,0).
	osVec3D       m_vec3RelativePosition;

	//! 当前emitter产生粒子时加入的随机加速度的范围.
	osVec3D       m_vec3EmissionSpeedScope;

	// 
	//! 当前emitter的形状.0: square 1:sphere, 2: custom shape.
	//! River added @ 2006-6-30: 加入在某个线段的范围内产生粒子的功能 3: LineSeg shape.
	//  River @ 2008-6-11:加入另外的形状，只在fallOff环形带上产生粒子 4: Ring Shape
	//                    ring shape与sphereShape的数据相同，不过sphereShape的hotPt不产生粒子，
	//                    所有的粒子都在FallOff内产生
	//! 线段的开始和结束点可以自定义，有一个默认的数据.
	int          m_iEmitterShape;


	//! 当前emitter发射粒子的Bounding Box.
	os_aabbox       m_sbox;
	

	//  
	//! River mod @ 2008-6-11:可以由fallOff环形带产生粒子，由
	//! HotPt处不产生粒子，用于做出粒子往中间吸引的效果。
	//! 当前emitter发射粒子的bounding Sphere.
	float        m_fHotPt;
	float        m_fFallOff;

	//! 如果当前的emitter用的是特殊的形状,对应文件名.
	char          m_szCSFname[64];

	//! 当前emitter每一个周期的长度.
	float         m_fCycleLength;

	//! 当前emitter是否是周期循环产生粒子.
	BOOL          m_bLoopCycle;

	//! 当前emitter产生粒子的速度的倍乘.
	float         m_fGenSpeedMulti;


	//@{
	//! emitter相关的曲线数据,这些数据在运行中需要插值.
	osVec3D       m_vec3Dir[MAX_DIVISIONNUM];

	//! 粒子产生器产生出粒子的随机方向值.
	osVec2D       m_vec2Rand[MAX_DIVISIONNUM];

	//! 粒子产生器产生出粒子的值.
	osVec2D       m_vec2EmissionRate[MAX_DIVISIONNUM];

	//! 粒子产生器产生出粒子的生命周期值.
	osVec2D       m_vec2EleLife[MAX_DIVISIONNUM];

	//@} 

	//! River @ 2006-5-18:如果是自定义产生粒子形状，保存trackId
	int           m_iTrackId;

public:
	os_particleEmitter();
	~os_particleEmitter();

	//! 从内存中调入particleEmitter的数据。
	void         load_emitterFromMem( BYTE*& _ps );


	//! 从一个tvalue得到当前emitter发射出的粒子的方向.
	void         get_emitDirection( osVec3D& _dir,float _tval );

	//! 从一个tvalue得到当前emitter发射出粒子的velocity.
	void         get_emitRandVelocity( osVec2D& _randVel,float _tval );

	//! 从一个tvalue得到当前emitter发射粒子的速度.
	void         get_emissionRate( osVec2D& _erate,float _tval );

	//! 从一个tvalue得到当前emitter发射的粒子在世界中的生命长度.
	void         get_emitParticleLifeTime( osVec2D& _ltime,float _tval );


};
typedef std::vector< os_particleEmitter > VEC_parEmitter;


/** \brief
 * 粒子产生器的实例类。
 *
 * 每一种类的粒子由一个产生器实例，产生出多个的粒子运行实例。
 *  
 */
struct os_particleEmtIns
{
	//! emitter当前是否激活.
	bool         m_bAlive;

	//! eimtter当前在T轴上的坐标值。
	float        m_fTVal;

	//! emitter产生粒子的初始化方向。
	osVec3D      m_vec3Dir;

	//@{
	//  所有的二维最大值和最小值:x为最小值,y为最大值.
	//! emitter当前随机加速度的最大和最小值。
	osVec2D      m_vec2RandVelocity;

	//! emitter产生粒子数目的最大值和最小值。　
	osVec2D       m_vec2EmissionRate;

	//! emitter产生出的粒子的生命周期的最大值和最小值。
	osVec2D       m_vec2ParLifeTime;
	//@} 
    
};


/** \brief
 *  粒子产生系统的类。
 *
 *  粒子产生系统对应了粒子产生器类和粒子运行控制类。
 *  使用粒子产生器类，可以产生出粒子，在粒子产生出来后，可以使用
 *  粒子运行控制类来控制粒子在生命周期中的行为。
 */
struct os_particleEmissionCtl
{
	//! 对应的粒子运行控制结构的索引.
	int             m_iParticleRunIdx;

	//! 对应的粒子产生器控制的索引.
	int             m_iParEmitterIdx;

	//! 如果产生的粒子没有空间,如何替代现有粒子空间.
	int             m_iReplaceStyle;

	//! emission的循环时间.
	float           m_fCycleLength;


public:
	os_particleEmissionCtl();
	~os_particleEmissionCtl();

	//! 从内存调入emissionControl的数据。
	void           load_emiCtlFromMem( BYTE*& _ps );

};

typedef std::vector< os_particleEmissionCtl > VEC_parEmissionCtl;






/** \brief
 *  emissonControl的运行实例。
 *
 *  场景中可见粒子的简单组织，由一个粒子产生器实例和由这个粒子产生器
 *  产生的一大堆的粒子实例构成。
 *
 *  粒子系统中最主要的处理单位,emissionControlInstance使emitter产生粒子,
 *  然后对产生的粒子进行更新,然后提取每一个粒子的渲染数据,并使用中间管理
 *  对这些数据进行渲染.
 *  
 *  使用emission Instance做为一个渲染单位?????
 */
class os_emssionCtlIns
{
private:
	//! 粒子场景的指针.
	os_particleScene*        m_ptrParScene;
	//! 指向的emissionControl的指针.
	os_particleEmissionCtl*   m_ptrEmissionCtl;



	//! 产生出新的粒子的替换方式。
	int                    m_iNpwmr;

	//! 粒子控制emitter的实例。
	os_particleEmtIns       m_particleEmtIns;

	//! 当前emissionCtlIns对应的粒子的列表.
	os_parRunCtlIns         m_vecParticle[MAX_PARTICLENUM];

	os_billBoardVertex      m_vecRenderVer[MAX_PARTICLENUM*4];

	//! 指向的emitter的指针.
	os_particleEmitter*     m_ptrEmitter;

	//! 指向的粒子控制的指针.
	os_particleRunCtl*      m_ptrParticleRunCtl;



	//! 上一帧到现在过去的时间.
	float              m_fTimeLastFrame;

	//! 从emission的循环开始到现在过去的时间.from loop start.
	float              m_fTimeFromLS;

	//! 上一帧没有算进去的粒子数目。
	float              m_fAccumulate;




	//! 当前用到的粒子顶点数目。
	int                m_iParVertexNum;

	//! 渲染当前的粒子控制用到的polygon指针。
	os_polygon          m_sPolygon;

	//@{
	//! River added @ 2006-6-30:
	//! 加入线段范围内产生粒子的开始点和结束点
	osVec3D             m_vec3LSStart;
	osVec3D             m_vec3LSEnd;
	//@}

	//! 指向的粒子产生器控制的指针。

	//! River 2006-4-25:用于粒子的两个位置之间插值，使产生在两点间均匀发散的粒子
	osVec3D             m_vec3LastEmitPos;
	//! 粒子产生器的位置相对于上帧是否改变了
	BOOL                m_bEmitPosChanged;

	//! River @ 2006-6-10:粒子的缩放功能加入。
	float               m_fSizeScale;
	//! 第一次产生粒子，用于处理产生周期特别短(短于一个渲染帧时间)的粒子特效。
	BOOL                m_bFirstFrameGenPar;

	//! 计算新产生的每一个粒子的位置
	void                cal_newParticlePos( osVec3D* _pos,osVec3D& _offset,int _num );


private:

	/** \brief
	 *   初始化一个粒子.
	 *   
	 */
	void           init_particle( int _idx,osVec3D& _offset );
	/** \brief
	 *   从当前的粒子对列点,替代一定数目的粒子.
	 *   
	 */
	void           replace_particle( int _rnum,osVec3D* _offset );
	/** \brief
	 *  对粒子进行更新.
	 */
	void           update_particle( int _idx,osVec3D& _offset );


	//@{
	//  对粒子相关数据进行操作的杂性函数.

	//! 粒子产生器是否生命终结,处理.
	void           emission_cycleEnd( void );
	//! 重新开始一个emitter的cycle.
	void           emitter_cycleStart( void );
	//! 得到emitterInstantce在当前时刻的数据.
	void           get_emitterInsData( void );

	//! 得到当前时刻产生emitter产生粒子的数目.
	int            get_emitterGenParNum( void );


	//! 得到粒子初始化的velocity.
	void           get_ptlInitVelocity( int _idx );

	//! 得到粒子的resistantance.
	void           get_particleResistance( int _idx );

	//! 得到粒子的初始化位置.
	void           get_ptlInitPosition( int _idx );

	//! 得到粒子的大小.
	void           get_parSize( int _idx );

	//! 得到粒子的alpha
	void           get_parAlpha( int _idx );

	//! 得到粒子的color
	void           get_parColor( int _idx );

	//! 得到粒子的旋转.
	void           get_parRotation( int _idx );

	//! 得到当前粒子的tvalue
	void           get_tvalue( int _idx );
	//! 得到当前粒子用到的纹理帧数.
	void           get_textureFrame( int _idx );

	//@} 


	/** \brief
	 *  每一帧对当前的emissionControlInstance进行更新。
	 *
	 *  更新粒子控制内的emitterIns和particleInsArray.
	 *
	 *  \param _offset 如果粒子发散到世界空间,需要这个_offset对粒子进行处理.
	 */
	void          framemove_emission( float _etime,osVec3D& _offset ); 

	/** \brief
	 *  填充当前emissionControl Instance内的粒子Array到指定的数据缓冲区中。
	 *
	 */
	void          fill_renderVerBuf( os_billBoardVertex* _verStart,osc_camera* _cam );

public:
	os_emssionCtlIns();

	//! 初始化当前的粒子控制。
	void          init_emissionCtlIns( os_particleScene* _sce,
		                             os_particleEmissionCtl* _emi,float _sizeScale );

	//! 对当前的emiControlIns进行FrameMove.
	void          frameMove_eci( osVec3D& _offset,float _etime,osc_camera* _cam );


	/** \brief
	 *  渲染当前的emissionControlInstance.
	 *
	 *  \param _offset 有关粒子的位置移动使用世界矩阵来处理.
	 *  
	 */
	bool          render_emissionControlIns( osc_middlePipe* _pipe,osVec3D& _offset );


	//! 重设当前的emssionInstance.
	void          reset_emssionIns( void );

	//! 得到emitter的指针列表
	os_particleEmitter*  get_emitterList ( void ) { return m_ptrEmitter; } 

	//! 设置粒子产生所在的线段范围
	void          set_parGetLineSeg( osVec3D& _start,osVec3D& _end );

	//! 当前的粒子场景是否是刀光线段的粒子
	bool          is_lineSegPar( void ) ;

	//! 当前的emicontrolIns是否激活。
	bool          is_alive( void );

	//! 停止发射当前的粒子,用于平和的结束粒子的运行
	void stop_emissionIns( void ) { m_particleEmtIns.m_bAlive = false; }

	//! 是否是循正播放的emitter.
	bool          is_loopPlayEmitter( void ) 
	{ 
		if( m_ptrEmitter ) return m_ptrEmitter->m_bLoopCycle;
		else return false;
	}

};





/** \brief
 *  粒子场景类，包含了一个或者多个的粒子产生系统.
 *
 *  
 */
class os_particleScene : public os_lruEle
{
	//! 使用os_particleScene类最多的类。
	friend class    os_particleSceneIns;
	friend class    os_emssionCtlIns;
private:

	//! 当前particle scene的名字.
	s_string        m_szParSceneName;
	//! hash val.
	DWORD           m_dwHash;

	//! 场景中emission Control的数目.
	int          m_iEmissionCtlNum;
	VEC_parEmissionCtl   m_vecEmssionCtl;


	//! 场景中粒子运行控制器的数目.
	int          m_iParRunCtlNum;
	VEC_parRunCtl m_vecParRunCtl;


	//! 场景中粒子产生器的数目.
	int          m_iEmitterNum;
	VEC_parEmitter m_vecParEmitter;

	//! 粒子场景限制的最多粒子数目.
	int           m_iMaxParticleNum;

	//! 粒子发射的空间:０:世界空间.1:粒子空间.
	int           m_iEmitterTo;

	//! 新产生的粒子如何替代旧有的粒子. 1:替换最老的粒子. 2:随机的替换粒子.
	int           m_iNpwmr;

	//! 粒子的放大倍数.
	float         m_fMultiSize;



	//@{
	//  路径相关的数据.
	//! 场景使用使用了路径文件.
	bool          m_bUseTrack;
	//! 路径文件名字.
	char          m_szTrackFileName[128];
	//! 粒子运行的速度.
	float         m_fRunSpeed;

	//@} 

	//! 当前场景是否使用中.
	bool          m_bInuse;

	//! 是否可以置换出内存.
	bool          m_bCanSwap;

public:
	os_particleScene();
	~os_particleScene();

	//! 从文件中读入我们的Particle数据。
	bool          load_parSceneFromFile( char * _fname );

	//! 从内存中释放一个粒子.
	void          release_parScene( void );

	//! 是否是我们要创建的场景.
	bool          is_needScene( int _hash ) { return (_hash==m_dwHash); }

	//! 当前场景是否在使用中
	bool          is_sceneInuse( void )             { return m_bInuse; } 

	const char*   get_sceneName( void )   { return m_szParSceneName.c_str(); } 

	//
	bool          get_canSwap( void ) { return m_bCanSwap; }
	void          set_swap( bool _s )  {  m_bCanSwap = _s; } 


};
typedef std::vector< os_particleScene >  VEC_particleScene;

/** \brief
 *  粒子场景的实例类,场景中播放相同文件名粒子场景的时候,
 *  可以共用粒子场景类,而使用不同的粒子场景实例.
 *
 *  
 */ 
class os_particleSceneIns : public os_createVal
{
	friend class osc_particleMgr;
private:
	//! 当前粒子场景的指针。
	os_particleScene*   m_ptrParScene;


	//! 粒子场景内的emissonControl实例控制。
	os_emssionCtlIns    m_arrEmissonCtlIns[MAX_EMISSION];

	//! 当前粒子场景在世界空间中的位置,由上层函数进行控制.
	osVec3D            m_vec3Offset;

	//! 当前粒子的最终偏移位置,发散到粒子空间的粒子使用
	osVec3D            m_vec3FinalOffset;

	//! 当前实例是否被使用。
	bool               m_bInuse;
	//! 当前实例是否顶层渲染
	bool               m_bTopLayer;

	//! 播放当前的粒子时，用的时间加速比。
	float              m_fTimeScale;

	//! 当前粒子场景的路径id.
	int                m_iTrackId;

	//! 当前的粒子是否显示
	bool               m_bHide;
public:
	os_particleSceneIns();
	~os_particleSceneIns();


	/** \brief
	 *  使用场景指针来初始化一个particleInstance.
	 *
	 *  \param _scale 整个粒子的缩放值,必须大于零
	 */
	WORD               init_particleIns( os_particleScene* _psce,osVec3D& _offset,
		                    float _scale,BOOL _inScene = FALSE,bool _topLayer = false );

	//! 对partileIns进行framemove.
	void               frameMove_curIns( float _etime,osc_camera* _cam,bool _canDeleteCurIns = false );

	/** \brief
	 *  渲染当前的particleSceneInstance.
	 *
	 *  \param _etime 上一帧过去的时间。
	 *  
	 */
	bool               render_currentIns( osc_camera* _cam,osc_middlePipe* _mpipe );
		                 
	//! 设置当前particleIns的位置.
	void               set_curInsPos( osVec3D& _vec3 )    { m_vec3Offset = _vec3; } 
	

	//! 当前粒子场景实例是否在使用中,
	bool               is_inUse( void )  { return m_bInuse; } 

	//! 设置当前粒子场景实例的播放时间。
	void               set_parPlayTime( float _timeScale );

	//! 得到当前粒子场景实例的播放时间。
	float              get_parPlayTime( void );

	//! 当前的粒子场景是否是循环播放的粒子场景
	bool               is_loopPlayParticle( void );

	//! 使用当前的instance变为不可用.
	void               delete_parSceIns( bool _finalRelease = false ) ;

	//! 平和的停止当前的粒子场景
	void               stop_parSceIns( void );

	//! 是否topLayer Render.
	bool               get_topLayer( void ) { return m_bTopLayer; } 

	//! 隐藏或是显示当前的particle.
	void               hide_particle( bool _hide ) { m_bHide = _hide; } 

	//! 设置自定义粒子的发散线段范围
	void               set_parGetLineSeg( osVec3D& _start,osVec3D& _end );

	//! 是否是由上层设置线段发生器的粒子
	bool               is_lineSegParticle( void ) { return m_arrEmissonCtlIns[0].is_lineSegPar(); };


	//! River added @ 2010-6-9:
	//! 当前的ins是否已经渲染,用于人物身上的武器特效，
	//! 不会多个人物在界面上渲染的时候出错
	bool               m_bRenderedInCurFrame;

};


//===================================================================================================
// particle Mgr相关的函数。　
//! 定义管理器中可以使用最多的粒子场景数目。
# define      MAX_PARTICLEFILE     64
# define      MAX_PARTICLEINS      48 


/** \brief
 *  粒子管理器类.
 *
 *  粒子管理器管理器粒子场景的创建,粒子场景的渲染.
 *  上层使用的所有接口都由这个类得到.
 *
 *  River @ 2006-6-2:
 *  粒子的删除，创建，FrameMove和渲染，都需要进行入临界区。
 */
class osc_particleMgr
{
	friend class os_particleSceneIns;
private:
	//! 粒子场景的数据组.
	CSpaceMgr<os_particleScene> m_vecParticleScene;

	//! 粒子运行实例的数据组,os_particleSceneIns数据量比较大，所以使用48个的数组。
	CSpaceMgr<os_particleSceneIns> m_vecParSceIns;


	//! 创建设备相关数据的中间管道。
	osc_middlePipe*      m_ptrMPipe;

	//! 释放最长时间不使用particle.
	void      discard_lruParticleScene( void );


public:
	//@{
	//  场景中所有的粒子使用同一个顶点缓冲区，填充顶点到缓冲区内不同的位置上。
	//  初始化particleMgr的时候，要初始化这些数据。
	//  
	//! 当前粒子场景用到的顶点缓冲区的id.
	static int                 m_iVbId;

	/** \brief
	 *  当前粒子场景用到的index buffer缓冲区的索引。
	 *
	 *  所有的粒子场景都使用这一个idx buffer,不需要重新填充idxBuffer,
	 *  只需要一次性初始化可以了。
	 */
	static int                 m_iIbId;

	//@} 

public:
	osc_particleMgr();
	~osc_particleMgr();

	//! 初始化当前的particle管理器,初始化的全部是设备相关的数据。
	void              init_particleMgr( osc_middlePipe* _pipe );

	//! 对ParticleMgr中的particleInstance进行FrameMove,渲染和处理分开.
	void              framemove_particle( osc_camera* _cam );

	//! 测试渲染particle.
	bool              render_particle( osc_camera* _cam,os_FightSceneRType _rtype = OS_COMMONRENDER );


	/** \bief
	 *  创建一个粒子场景实例.
	 * 
	 *  \return int 返回了当前particle的id.
	 */
	int               create_parSceIns( os_particleCreate& _pcreate,BOOL _inScene = FALSE,bool _topLayer = false );

	/** \brief
	 *  设置一个particle的益位置数据.
	 *
	 *  \param _forceUpdate 强制更新当前的粒子位置到渲染数据，用于跟随人物武器的粒子能正确的渲染
	 */
	void              set_particlePos( osVec3D& _pos,int _idx,bool _forceUpdate = false );

	//! 删除一个particle.
	void              delete_particle( int _idx,bool _finalRelease = false );

	const char*       get_parNameFromId( int _id );


	//! 设置粒子场景的播放时间。
	void              set_parPlayTime( int _id,float _timeScale );
	//! 得以粒子场景的播放时间。
	float             get_parPlayTime( int _id );

	//! 是否是循正播放的公告板
	bool              is_loopPlayParticle( int _id );

	//! 隐藏传入的particle id效果
	bool              hide_particle( int _id,bool _hide );

	//! River @ 2010-6-9:为了人物身上的粒子渲染在界面中正确.
	void              reset_particleInsRender( int _id );


	/** \brief
	 *  检测粒子id对应粒子的有效性.
	 *
	 *  如果对应的粒子处于死亡状态，则处理当前
	 *  的粒子id.
	 *  \return bool 如果有效，返回true.
	 */
	bool              validate_parId( int _id );

	//! 设置粒子的发射线段的开始和结束位置
	void              set_parGenLineSeg( int _id,osVec3D& _start,osVec3D& _end );

};



//! 包含particle的inline数据.
# include "fdParticle.inl"





