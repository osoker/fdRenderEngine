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
# pragma once

# include "../../interface/osInterface.h"

# include "osCamera.h"

# include <vector>


/**
 *
 */
typedef CSpaceMgr< osc_camera >  VEC_camera;


/** \brief 
 *  Resource�ӿڵ�ʵ����.
 *
 *  �������Դ���OSOK�������й�������Ҫ�����е���Դ.
 *  ��Դ�Ļ�����һ���Ƚ�ģ���ĸ���.ֻҪ��������Ϊ����Դ�Ķ���
 *  �����Լ��뵽������м�.
 * 
 */
class osc_resourceManager : public I_resource
{
public:
	osc_resourceManager();
	~osc_resourceManager();

	/** \brief 
	 *  ����camera.
	 *  
	 *  �����Ѿ�������camera�����ָ��,�������ʧ��,����null.
	 *
	 */
	virtual I_camera*       create_camera( void );

	//! ɾ��һ�����
	virtual void            delete_camera( I_camera* _cam );



private:
	//! ���ڱ���camera��vector.
	VEC_camera              m_vecCamera;

private:
	/**
	 *   ��ʼ��resource Manager,���������е���.
	 */
	void                    reinit_resMgr( void );

	/**
	 *   �ͷ�resource Manager��ռ�õ��ڴ�.
	 */
	void                    release_resMgr( void );

};



