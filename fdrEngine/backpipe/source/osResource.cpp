//=======================================================================================================
/**  \file 
 *   Filename: osResource.h
 *
 *   Des:      Resource �ӿڵ�ʵ���ļ�.   
 *
 *   His:      River created @ 4/24 2003
 *
 *   "�ɹ����˺���ͨ�����Ĳ�ͬ������������һ�����룬Ϊ������ʵ������
 *    ������һ�����ڵĶ���ʹ����ƴ����������������Ϊ�ϼ����ǹ�˾��
 *    �涨��������ҹ�Ӱࡣ"
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


//! ɾ��һ�����
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
*  ����camera.
*  
*  �����Ѿ�������camera�����ָ��,�������ʧ��,����null.
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
*   ��ʼ��resource Manager,���������е���.
*/
void osc_resourceManager::reinit_resMgr( void )
{
	m_vecCamera.resize( INIT_CAMERANUM );

}


/**
*   �ͷ�resource Manager��ռ�õ��ڴ�.
*/
void osc_resourceManager::release_resMgr( void )
{

}

