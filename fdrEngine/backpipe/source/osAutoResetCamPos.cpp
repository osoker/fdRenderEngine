//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osAutoResetCamPos.cpp
 *
 *   Desc:     实现相机的自动回位功能
 *          
 *   His:      River Created @ 2006-7-11
 *
 *   "不要轻易改变花了很长时间做出的决定,那样就等于是背叛了自己的信念".
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osCamera.h"
# include "../../interface/miskFunc.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"


//! 相机自动回位的速度.
# define  AUTORESET_SPEED  6.0f


//! 相机自动回位的初速和加速度
OSENGINE_API float                    g_fInitAutoResetSpeed = 2.0f;
OSENGINE_API float                    g_fAutoResetAcce = 5.0f;


osc_autoResetMgr::osc_autoResetMgr()
{
	m_bInARState = false;
	m_fLastFrameMove = 0.0f;

	m_bPauseAutoset = false;

}


//! 开始自动回位状态
void osc_autoResetMgr::start_autoResetState( float _dis,float _pitch )
{
	guard;
	
	osassert( _dis > 0.0f );

	// 如果已经处于自动回位状态,则以最老的自动回位数据为准.
	if( !m_bInARState )
	{
		m_bInARState = true;
		m_fDestPitch = _pitch;

		m_fPosFocusDistance = _dis;


		m_fAutoResetPassTime = 0.0f;
	}

	unguard;
}


/** \brief
*  计算自动回位的位置.
*  
*  \param _focus  相机的焦点位置.所有的自动回位都是以新的焦点为中心.
*  \param _curPos 传入的当前相机的位置.
*
*  算法描述:
*  1: 根据传入的位置,计算出当前相机的pitch
*  2: 在当前的pitch和最终想要的pitch之间插值处理.
*  3: 根据相机的回位速度,来处理得到自动相机回位应该回位的矩离
*  4: 上层判断新的位置是否合法,如果合法设置新的位置,否则等待下一帧来再次调用.
*/
float osc_autoResetMgr::get_autoResetPos( osVec3D& _focus,osVec3D& _curPos )
{
	guard;

	osVec3D   t_vec3Dir;
	float     t_fDis;
	float     t_fLastElaTime = sg_timer::Instance()->get_lastelatime();

	t_vec3Dir = _curPos - _focus;
	t_fDis = osVec3Length( &t_vec3Dir );

	if( m_fAutoResetPassTime < AUTORESET_STARTTIME )
	{
		m_fResetSpeedPerSec = g_fInitAutoResetSpeed;
	}
	else
	{
		// 初速度加上加速度
		m_fResetSpeedPerSec += 
			g_fAutoResetAcce*sg_timer::Instance()->get_lastelatime();
	}

	float t_f = m_fResetSpeedPerSec*t_fLastElaTime;

	if( t_f < 0.0f ) t_f = 0.0f;
	if( t_f > 1.0f ) t_f = 1.0f;

	// 不超出相机的原始视矩
	if( (t_f + t_fDis) >= m_fPosFocusDistance )
	{
		m_bInARState = false;
		t_f = m_fPosFocusDistance - t_fDis;
	}

	return t_f;

	unguard;
}

const osc_autoResetMgr& osc_autoResetMgr::operator=( osc_autoResetMgr& _mgr )
{
	guard;

	m_bInARState = _mgr.m_bInARState;
	m_bPauseAutoset = _mgr.m_bPauseAutoset;
	m_fAutoResetPassTime = _mgr.m_fAutoResetPassTime;
	m_fDestPitch = _mgr.m_fDestPitch;
	m_fLastFrameMove = _mgr.m_fLastFrameMove;
	m_fPosFocusDistance = _mgr.m_fPosFocusDistance;
	m_fResetSpeedPerSec = _mgr.m_fResetSpeedPerSec;

	return *this;

	unguard;
}




//! 设置当前帧通过自动回位测试的情况.
void osc_autoResetMgr::set_autoResetPass( bool _pass )
{
	guard;

	if( _pass  )
		m_fAutoResetPassTime += sg_timer::Instance()->get_lastelatime();
	else
		m_fAutoResetPassTime = 0.0f;

	return;

	unguard;
}



//! 相机自动回位的处理
void osc_autoResetMgr::process_camAutoResetPos( osc_camera* _cam,float& _move )
{
	guard;

	m_fLastFrameMove = 0.0f;

	if( !this->m_bInARState )
		return;

	osVec3D   t_vec3CurPos,t_vec3Focus;

	_cam->get_curpos( &t_vec3CurPos );
	_cam->get_camFocus( t_vec3Focus );

	if( m_bPauseAutoset )
		_move = 0.0f;
	else
		_move = get_autoResetPos( t_vec3Focus,t_vec3CurPos );

	m_fLastFrameMove = _move;

	return;

	unguard;
}

