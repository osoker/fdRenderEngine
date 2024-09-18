//=======================================================================================================
/**  \file 
 *   Filename: osResource.h
 *
 *   Des:      Resource 接口的实现文件.   
 *
 *   His:      River created @ 4/24 2003
 *
 *   "成功的人和普通人最大的不同是他们心中有一个理想，为了早日实现它，
 *    心中有一股内在的动力使他们拼命工作，而不是因为上级或是公司的
 *    规定而必须日夜加班。"
 *
 */
//=======================================================================================================
# include "stdafx.h"
# include "../include/osResource.h"

# define INIT_CAMERANUM   8

osc_resourceManager::osc_resourceManager() 
{
	reinit_resMgr();

}

osc_resourceManager::~osc_resourceManager() 
{

}


//! 删除一个相机
void osc_resourceManager::delete_camera( I_camera* _cam )
{
	guard;

	CSpaceMgr<osc_camera>::NODE  t_ptrNode;

	for( t_ptrNode = m_vecCamera.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecCamera.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p == _cam )
		{
			m_vecCamera.release_node( t_ptrNode.idx );
			break;
		}
	}

	return;

	unguard;
}


/** \brief 
*  创建camera.
*  
*  返回已经创建的camera数组的指针,如果创建失败,返回null.
*
*/
I_camera* osc_resourceManager::create_camera( void )
{
	guard;

	osc_camera*  t_ptrCam;

	m_vecCamera.get_freeNodeAUse( &t_ptrCam );

	return (I_camera*)t_ptrCam;

	unguard;
}






/**
*   初始化resource Manager,构建函数中调用.
*/
void osc_resourceManager::reinit_resMgr( void )
{
	m_vecCamera.resize( INIT_CAMERANUM );

}


/**
*   释放resource Manager所占用的内存.
*/
void osc_resourceManager::release_resMgr( void )
{

}

