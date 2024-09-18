//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osAutoResetCamPos.cpp
 *
 *   Desc:     ʵ��������Զ���λ����
 *          
 *   His:      River Created @ 2006-7-11
 *
 *   "��Ҫ���׸ı仨�˺ܳ�ʱ�������ľ���,�����͵����Ǳ������Լ�������".
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osCamera.h"
# include "../../interface/miskFunc.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"


//! ����Զ���λ���ٶ�.
# define  AUTORESET_SPEED  6.0f


//! ����Զ���λ�ĳ��ٺͼ��ٶ�
OSENGINE_API float                    g_fInitAutoResetSpeed = 2.0f;
OSENGINE_API float                    g_fAutoResetAcce = 5.0f;


osc_autoResetMgr::osc_autoResetMgr()
{
	m_bInARState = false;
	m_fLastFrameMove = 0.0f;

	m_bPauseAutoset = false;

}


//! ��ʼ�Զ���λ״̬
void osc_autoResetMgr::start_autoResetState( float _dis,float _pitch )
{
	guard;
	
	osassert( _dis > 0.0f );

	// ����Ѿ������Զ���λ״̬,�������ϵ��Զ���λ����Ϊ׼.
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
*  �����Զ���λ��λ��.
*  
*  \param _focus  ����Ľ���λ��.���е��Զ���λ�������µĽ���Ϊ����.
*  \param _curPos ����ĵ�ǰ�����λ��.
*
*  �㷨����:
*  1: ���ݴ����λ��,�������ǰ�����pitch
*  2: �ڵ�ǰ��pitch��������Ҫ��pitch֮���ֵ����.
*  3: ��������Ļ�λ�ٶ�,������õ��Զ������λӦ�û�λ�ľ���
*  4: �ϲ��ж��µ�λ���Ƿ�Ϸ�,����Ϸ������µ�λ��,����ȴ���һ֡���ٴε���.
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
		// ���ٶȼ��ϼ��ٶ�
		m_fResetSpeedPerSec += 
			g_fAutoResetAcce*sg_timer::Instance()->get_lastelatime();
	}

	float t_f = m_fResetSpeedPerSec*t_fLastElaTime;

	if( t_f < 0.0f ) t_f = 0.0f;
	if( t_f > 1.0f ) t_f = 1.0f;

	// �����������ԭʼ�Ӿ�
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




//! ���õ�ǰ֡ͨ���Զ���λ���Ե����.
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



//! ����Զ���λ�Ĵ���
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

