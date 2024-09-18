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
# pragma once

# include "../../interface/osInterface.h"

# include "osCamera.h"

# include <vector>


/**
 *
 */
typedef CSpaceMgr< osc_camera >  VEC_camera;


/** \brief 
 *  Resource接口的实现类.
 *
 *  这个类可以创建OSOK引擎运行过程中需要的所有的资源.
 *  资源的划分是一个比较模糊的概念.只要是我们认为是资源的东西
 *  都可以加入到这个类中间.
 * 
 */
class osc_resourceManager : public I_resource
{
public:
	osc_resourceManager();
	~osc_resourceManager();

	/** \brief 
	 *  创建camera.
	 *  
	 *  返回已经创建的camera数组的指针,如果创建失败,返回null.
	 *
	 */
	virtual I_camera*       create_camera( void );

	//! 删除一个相机
	virtual void            delete_camera( I_camera* _cam );



private:
	//! 用于保存camera的vector.
	VEC_camera              m_vecCamera;

private:
	/**
	 *   初始化resource Manager,构建函数中调用.
	 */
	void                    reinit_resMgr( void );

	/**
	 *   释放resource Manager所占用的内存.
	 */
	void                    release_resMgr( void );

};



