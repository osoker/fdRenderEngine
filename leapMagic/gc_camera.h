///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_camera.h
 *
 *  Desc:     魔法demo客户端用到的相机类，相机跟其它的entity一样，处理场景内的消息
 * 
 *  His:      River created @ 2006-4-14
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "stdafx.h"
# include "gc_command.h"

class lm_configInfoReader;
/** \brief
 *  全局的相机类。整个场景内只使用一个相机。
 *
 *  这个相机包装引擎内的I_camera相机接口。在场景调入之前，先处理相机，把相机插入到场景内。
 *  相机可以接受和处理消息。消息可以来自输入，也可以来自网络等等。
 *
 *  初始化并插入场景的处理由函数调用
 *  
 *  相机接受和处理以下的消息：
 *  1: 
 *  2: 动画消息。 
 *  3: Pitch,Rotate,Move等消息。
 *  4: 其它...
 */
class gc_camera : public lm_gameEntity
{
	//! 为方便读入相机的配置数据
	friend class lm_configInfoReader;
private:

	//! 相机初始化用到的一些信息
	os_cameraInit        m_sCamInitData;

	//! 拉近放远的速度
	float                m_fToFocusSpeed;
	//! 相机位置离焦点最近的矩离
	float                m_fMinFocusDis;  
	//! 相机位置离焦点的最远矩离
	float                m_fMaxFocusDis;

public:
	//! 引擎的相机接口
	static I_camera*     m_ptrCamera;

	//! 焦点偏移人物的高度
	static float         m_fFocusOffset;

	//! 鼠标旋转的速度调节
	static float         m_fRotSpeed;

	//! 相机的角度限制
	static float         m_fCamPitchLimit;
	
	//! 对消息的处理。
private:
	//! 相机动画相关的消息

	//! pitch,rotate和move
	void          process_pitchRot( const lm_command* _cmd );

	//! 拉近放远
	void          move_close( const lm_command* _cmd );
	void          move_closef( const lm_command* _cmd );

	//! 相机的整体移动，焦点和位置
	void          move_camera( const lm_command* _cmd );

	//! 根据三个向量，画bbox
	static void   draw_bbox( I_fdScene* _scene,osVec3D& _pos,osVec3D& _lookvec,osVec3D& _rvec );

	//! 相机往前移动
	void          move_forword( const lm_command* _cmd );
	//! 相机往右移动
	void          move_right( const lm_command* _cmd );
	//! 相机的焦点往上移动
	void          move_up( const lm_command* _cmd );
	//! 存储相机
	void          save_camera( const lm_command* _cmd );
	//! 调入相机.
	void          load_camera( const lm_command* _cmd );



public:
	gc_camera(void);
	~gc_camera(void);

	//! 初始化并插入场景内相机的消息
	void          init_camera( const os_cameraInit* _camInit );  

	//! 快照当前相机的信息,返回一个跟当前的相机一样的相机指针。
	I_camera*     snapshot_curCamera( void );

	//! 画出相机的位置和vf
	static void   draw_camera( I_camera* _cam,I_fdScene* _scene );

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );

};



//! 关键顶点.
struct gcs_keyPoint
{
	osVec3D      m_vec3Pt;
	float        m_fTime;
};

typedef std::vector< gcs_keyPoint >  VEC_keyPoint;

//! 最大可以有的KeyPt数目.
# define   MAX_KEYPOINT      256

/** \brief
 *  这个类可以使相机的动画更加的柔和.
 *  
 *  算法描述:
 *  推入一定量的关键顶点,然后在这些关键顶点间进行插值,得到更加柔和的关键顶点.
 *  因为像机随时运动中,所以推入的关键顶点只能是象机的焦点数据.
 */
class gc_smoothMotion
{
private:
	//! time Axis and Key point.
	VEC_keyPoint  m_vecKeyPt;

	//! 关键点队列是否满过
	BOOL          m_bFirstFull;
	//! 计算smoothPt的开始顶点.
	int           m_iStartPt;
	//! 总关键点所在的位置.
	int           m_iKeyPtEnd;

	//! 柔和动画推迟时间的值.
	float         m_fDeferTime;
	//! 队列内总共顶点的数目
	int           m_iMaxPtNum;

	//! 上一帧得到的平滑顶点的位置
	osVec3D       m_vec3LastFramePt;

	//! 重设smoothMotion结构
	void          reset_smoothMotion( void );

	//! 得到下一个Pt的索引.
	int           get_nextPtIdx( int _idx );
	//! 得到上一个Pt的索引.
	int           get_previousPtIdx( int _idx );

public:
	gc_smoothMotion();

	/** \brief
	 *  动画后延的时间,按秒计.
	 *
	 *  \param _time 延后的时间.
	 *  \param _maxPtNum 最多缓存的关键顶点数目.
	 */
	void   set_deferTime( float _time,int _maxPtNum );

	//! 推入关键点和关键点所在的时间
	void   push_keyPt( osVec3D& _focus,float _curTime )
	{
		m_vecKeyPt[m_iKeyPtEnd].m_vec3Pt = _focus;
		m_vecKeyPt[m_iKeyPtEnd].m_fTime = _curTime;
		m_iKeyPtEnd ++;
	}

	//! 得到当前帧可以使用的平滑动画点.
	BOOL   get_smoothPt( osVec3D& _resPt,float _curTime );


};


