//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: dlight.h
 *
 *  Desc:     处理引擎中的动态光，最后动态光需要和场景相结合。
 *
 *  His:      River created @ 2004-3-3
 *
 *  "育才造士,为国之本"  
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once


# include "../../interface/osInterface.h"


# include "../../mfpipe/include/osShader.h"                 // 使用ShaderMgr创建相关的设备相关资源。
# include "../../mfpipe/include/middlePipe.h"


//! 是否使用实时的灯光,如果要测试lightMap，或是用顶点色来模拟灯光，此处为0
# define REALTIME_LIGHT  1


//! 动态光影响的最大范围，在4个方向上都是6个格子,就是18米。
# define MAXTILE_DLEFFECT    8



/** \brief
 *  场景中动态光的数据结构。　
 *  
 */
struct osc_dlight
{
	//! 动态光位置。
	osVec3D    m_vec3Pos;

	//! 当前动态光的颜色。
	osColor    m_dlightColor;

	//! 当前动态光的影响范围。
	float      m_fRadius;

	//! 当前结构是否在使用中。
	BOOL       m_bInuse;


	//@{
	//  动态光插值相关。
	//! 源光线颜色与目标光线颜色。
	osColor            m_sSrcColor;
	osColor            m_sDstColor;
	//! 总共需要插值的时间，和当前所在的时间值。
	float              m_fTolLerpTime;
	float              m_fCurTime;
	//! 当前是否处于插值状态。
	BOOL               m_bInLerpState;
	//@} 

public:
	osc_dlight();

	//! 在两个动态光的数据之间进行插值。
	void                lerp_DLight( float _time );

	//! 开始动态光插值。
	void                start_dLightFadeOut( float _totalTime ); 

};
typedef std::vector< osc_dlight >  VEC_dlight;



//! 全局的动态光信息
struct os_dlInfo
{
	int                m_iDLNum;

	//! 每一个动态光对应的动态光指针.
	const osc_dlight*  m_vecDLPtr[MAX_LIGHTNUM];
	//! 每一个动态光在动态光管理器内的ID
	int                m_iDLId[MAX_LIGHTNUM];
};

/** \brief
 *  管理动态光的管理器。
 *  
 *  ATTENTION TO FIX: 构造多个动态光的时候，可能会内存操作问题，但在特定的条件下才能出现，
 *                     出现问题时调试。
 */
class osc_dlightMgr
{
	friend class     osc_TGManager;
private:
	//! 当前的动态光数据。
	osc_dlight       m_vecDlight[MAX_LIGHTNUM];

	//! 当前lightMgr中保存的中间管道的指针。
	osc_middlePipe*   m_ptrMpipe;

	//! 动态光用到的纹理使用的shaderid.
	int              m_iShaderId;

	//! dx的灯光结构，普通物品使用这些结构。
	D3DLIGHT9        m_vecDXLight[MAX_LIGHTNUM];
	//! 当前用到的dx灯光数目。
	int             m_iDxLightNum;

private:

	void fill_dxGDirLight( D3DLIGHT9& _dxL );

public:
	osc_dlightMgr();
	~osc_dlightMgr();


	//! 初始化动态光管理器。
	void          init_dlightMgr( osc_middlePipe* _mpipe );

	//! 释放动态光管理器
	void          reset_dlightMgr( void );

	//! 帧间move动态光管理器( 场景中删除动态光时，需要淡出)
	void          frame_moveDLMgr( void );

	/** \brief
	 *  创建一个动态光。
	 *
	 *  \return int  如果返回-1,则创建失败。
	 */
	int           create_dynamicLight( os_dlightStruct& _dl );

	/** \brief
	 *  得到一个动态光的各项参数。
	 */
	void          get_dynamicLight( int _id,os_dlightStruct& _dl );

	/** \brief
	 *  FrameMove一个动态光。
	 */
	void          set_dynamicLight( int _id,os_dlightStruct& _dl );

	/** \brief
	 *  设置动态光的位置。
	 */
	void          set_dLightPos( int _id,osVec3D& _pos );


	/** \brief
	 *  删除一个动态光。
	 */
	void          delete_dynamicLight( int _id,float _fadeTime = 0.6f );

	/** \brief
	 *  设置当前场景中的动态光为dx兼容格式。
	 *
	 *  \param _enable 此值为true时，打开灯光，否则关闭灯光。
	 *  \param bool _fullAmbi 是否开足环境光.
	 */
	void          dxlight_enable( bool _enable,bool _fullAmbi = false );

	//! 得到动态光管理器内的信息
	void          get_dlINfo( os_dlInfo& _info );

	//! 设置一个动态光
	void          set_pointLight( os_meshDLight& _dl,bool _enable = true );

	


};


extern osc_dlightMgr*   g_ptrDLightMgr;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  动态光位置的改变。
/** \brief
*  设置动态光的位置。
*/
inline void osc_dlightMgr::set_dLightPos( int _id,osVec3D& _pos )
{
	osc_dlight*  t_ptrL;

	osassert( (_id>=0)&&(_id<MAX_LIGHTNUM) );

	t_ptrL = &this->m_vecDlight[_id];
	osassert( t_ptrL->m_bInuse );

	t_ptrL->m_vec3Pos = _pos;

}

