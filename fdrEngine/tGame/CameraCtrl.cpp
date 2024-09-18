//===================================================================
/** \file  
*  Filename: CameraCtrl.cpp
*  Desc:   相机控制的实现文件
*  His:      王凯 @ 1/12 2004 16:18
*/
//================================================================

#include "StdAfx.h"
#include "cameractrl.h"
# include "tengine.h"
#include "dxutil.h"
/**brief*****************************************************************/
/*!相机的构造函数
/************************************************************************/
CCameraCtrl::CCameraCtrl(void)
{
	m_pCamera = NULL;
	m_pChar	 = NULL;
	ZeroMemory(&m_sCameraData,sizeof(m_sCameraData));
}

CCameraCtrl::~CCameraCtrl(void)
{
}




/**brief*****************************************************************/
/*!创建一个相机
* \param  _hwnd
*		  窗口句柄
/************************************************************************/
bool CCameraCtrl::Create(HWND _hwnd )
{
	RECT             t_rect;
	osVec3D          t_vec( 1,0,1 );
	osVec3D          t_vec1( 0,0,0 );

	if( !g_bFullScr )
		::GetClientRect( _hwnd,&t_rect );
	else
	{
		t_rect.top = 0;
		t_rect.left = 0;
		t_rect.right = g_iScrWidth;
		t_rect.bottom = g_iScrHeight;
	}

	m_sCameraData.farp = g_fTerrFar;
	m_sCameraData.nearp = 0.1f;
	m_sCameraData.fov = 0.785f;
	m_sCameraData.faspect = float(t_rect.right-t_rect.left)
		/float(t_rect.bottom-t_rect.top);


	m_sCameraData.tlx = 0;
	m_sCameraData.tly = 0;
	m_sCameraData.wid = t_rect.right - t_rect.left;
	m_sCameraData.hei = t_rect.bottom - t_rect.top;
	m_sCameraData.max_z = 1.0;
	m_sCameraData.min_z = 0.0;

	m_VerLookat = m_pChar->m_verPos;

	float t_f;

	m_pChar->m_pObj->get_posAndAgl( m_sCameraData.camFocus,t_f );
	m_sCameraData.camFocus.y += 0.5f;
	m_sCameraData.curPos = osVec3D( -1,1,-1 );
	osVec3Normalize( &m_sCameraData.curPos,&m_sCameraData.curPos );
	m_sCameraData.curPos *= 15;
	m_sCameraData.curPos += m_sCameraData.camFocus;

	
	g_resourceMgr = ::get_resourceMgr();
	osassert( g_resourceMgr );
	m_pCamera = g_resourceMgr->create_camera();
	osassert( m_pCamera );
	m_pCamera->init_camera( &m_sCameraData );


	return true;
}




/**brief*****************************************************************/
/* !设置视点跟随的对象
*	\param  _pChar
*			相机跟随的对象
*	\parma	_pVerDist
*			相机和跟随对象的距离，这个距离不变，但可以用ModifyDist来修改它
/************************************************************************/
void  CCameraCtrl::SetFollowObject(I_Character   *_pChar, osVec3D* _pVerDist)
{
	m_pChar = _pChar;
	m_VerDist = *_pVerDist;
}

/**brief*****************************************************************/
/* !更新相机的位置，在每一帧中使用
/************************************************************************/
void CCameraCtrl::Update()
{
	static osVec3D   t_lastPos;
	static bool      t_bFirst = true;
	osVec3D          t_curPos,t_camPos;

	if( t_bFirst )
	{
		t_lastPos = m_pChar->m_verPos;
		t_bFirst = false;

		return;
	}

	t_curPos = m_pChar->m_verPos;
	t_curPos.y += 1.5f;

	if( vec3_equal( t_curPos,t_lastPos ) )
		return;


	t_camPos = t_curPos;
	t_curPos = t_curPos - t_lastPos;
	t_lastPos = t_camPos;



	m_pCamera->get_curpos( &t_camPos );
	t_camPos += t_curPos;
	m_pCamera->set_curpos( &t_camPos );

	m_pCamera->get_camFocus( t_camPos );
	t_camPos += t_curPos;
	m_pCamera->set_camFocus( t_camPos );
	

	osVec3D   t_vec;
	t_vec = g_cHero.m_verPos;
	t_vec.y += 0;


	while( !camera_posLegal() )
	{
		if( OSE_CAMIN_ANIMATION == 
			m_pCamera->rotate_vertical( t_vec, 0.05f ) )
			break;
	}
}

/**brief*****************************************************************/
/* !修改相机和跟随对象的距离
*	\parma	_pVerDist
*			修改相机和跟随对象的距离
/************************************************************************/
void CCameraCtrl::ModifyDist(osVec3D *_pVerDist)
{
	m_VerDist += *_pVerDist;
}
