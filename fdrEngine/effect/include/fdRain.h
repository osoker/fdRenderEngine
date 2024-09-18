//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdRain.h
 *
 *  Desc:     场景中下雨特效的处理。
 *
 *  His:      River created @ 2004-7-3
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"


/** \brief
 *  场景中的下雨效果。
 *
 *  下雨效果使用在场景中画线段来实现。
 *  
 */
class osc_rain
{
private:

	float		m_fWidth;
	float		m_fHeight;
	
	float		m_fRainLength;

	osVec3D 	m_vec3Velocity;

	//! 设备相关的数据。
	osc_middlePipe*  m_ptrMPipe;

	//! 当前雨滴数据对应的dynamicVertex bufferID.
	int             m_iVBid;
	//! 当前雨滴数据用到的Index buffer数据。
	int             m_iIBid;

	//! 当前雨滴对应的effectID
	int             m_iEffectId;

	//! 渲染当前的Effect使用的polygon.
	os_polygon       m_sPolygon;

	//! 雨滴对应的顶点数据。
	os_sceVerDiffuse* m_vecRainVer;

	//! 雨滴的位置数据。
	osVec3D*         m_vecRainPos;
	//! 每一个雨滴对应的下落速度。
	float*           m_vecSpeed;



	//! 每一个雨滴都是由如果下的四边形数据构成。
	os_sceVerDiffuse  m_vecRainVerOffset[4];

	//! 开始停止下雨。
	bool            m_bStopRaining;

	//! 当前是否处于下雨
	bool            m_bRainning;


	//! 对下雨效果进行描述的数据结构
	os_rainStruct    m_sRainStart;
	//! 外部修改后的雨点初始化大小
	os_sceVerDiffuse m_vecRainVerStart[4];

private:
	//! 对下雨效果的FrameMove处理。
	void       frame_moveRain( osVec3D& _campos );

	//! 设置渲染雨滴效果使用的相机。
	void       set_rainRenderCam( float _pitch );

	/** \brief
	 *  初始化下雨效果的设备相关数据。
	 *
	 *  这个函数在init_rainData中自动调用，以后每次释放了场景中的设备相关
	 *  数据后，需要再调用这个函数来恢复下雨效果的设备相关数据。
	 */
	void       init_rainDevdepData( void );

public:
	osc_rain();
	virtual ~osc_rain();

	//! 初始化雨滴效果的数据。
	void       init_rainData( osc_middlePipe* _mpipe );


	/** \brief
	 *  渲染当前的雨效果。
	 *
	 *  \param _pitch  当前场景中主相机的pitch值。
	 *  \param _pos    当前场景中相机的位置。
	 */
	void       render_rain( osVec3D& _pos,float _pitch );


	//! 开始下雨效果。
	void       start_rain( os_rainStruct* _rs = NULL );


	//! 结束下雨的效果。
	void       stop_rain( void );


	//! 得到是否正在下雨.
	bool       get_rainning( void );



};


//! 场景内最多的雪花



//! 雪花的数据结构
struct os_snowStruct
{
	//! 雪花的大小
	float   m_fSize;
	
	//! 雪花洒落的方向和加速度。
	osVec3D m_vec3Velocity;

	//! 雪花当前的位置.
	osVec3D m_ve3SnowPos;

};

/** \brief
 *  下雪效果的管理器
 *
 *  下雪需要实现的算法：
 *  1: 先初化整个管理器,创建顶点缓冲区和索引缓冲区还有effectId.
 *  2: 开始下雪后，初始化雪花的数据结构。
 *  3: 每一帧对雪花FrameMove.
 *  4: 设置相机，正确的渲染雪花。
 *  5: 停止下雪后的处理
 */
class osc_snowStorm
{
private:
	//! 雪花的数据结构
	os_snowStruct*    m_vecSnowData;


	//! 设备相关的数据。
	osc_middlePipe*  m_ptrMPipe;
	//! 当前雪花数据对应的dynamicVertex bufferID.
	int             m_iVBid;
	//! 当前雪花数据用到的Index buffer数据。
	int             m_iIBid;
	//! 当前雪花对应的effectID
	int             m_iEffectId;
	//! 渲染当前的Effect使用的polygon.
	os_polygon       m_sPolygon;
	//! 每一个雪花都是由如果下的四边形数据构成。
	//os_sceVerDiffuse  m_vecSnowVerOffset[4];
	os_SnowVertex  m_vecSnowVerOffset[4];

	//! 雪花对应的顶点数据
	//os_sceVerDiffuse* m_vecSnowRenderVer2;
	os_SnowVertex*	m_vecSnowRenderVer;
	osVec3D*			m_vecSnowPos;
	osVec3D*				m_vecSpeed;

	//! 开始停止下雪。
	bool            m_bStopSnowing;

	//! 当前是否处于下雪
	bool            m_bSnowing;

	os_snow			mSnowSetting;
private:
	//! 对下雪效果的FrameMove处理。
	void       frame_moveSnow( osVec3D& _campos );

	//! 设置渲染雪效果使用的相机。
	void       set_snowRenderCam( float _pitch );

public:
	osc_snowStorm();
	~osc_snowStorm();

	//! 初始化整个下雪的管理器
	void    init_snowStormMgr( osc_middlePipe* _mpipe );

	//! 开始下雪
	void    start_snowStorm( os_snow& snowSetting );

	//! 结束下雪
	void    stop_snowStorm( void );

	/** \brief
	 *  渲染当前的雪效果。
	 *
	 *  \param _pitch  当前场景中主相机的pitch值。
	 *  \param _pos    当前场景中相机的位置。
	 */
	void       render_snowStorm( osVec3D& _pos,float _pitch );


	//! 得到是否下雪
	bool       get_weatherSnowing( os_snow* snowSetting = NULL);

	//! 
private:
	//! 对下雪效果的FrameMove处理。
	void       frame_moveRain( osVec3D& _campos );

};




///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  
//! 结束下雨的效果。
inline void osc_rain::stop_rain( void )
{
	m_bStopRaining = true;
}
//! 得到是否正在下雨.
inline bool osc_rain::get_rainning( void )
{
	return !m_bStopRaining;
}


