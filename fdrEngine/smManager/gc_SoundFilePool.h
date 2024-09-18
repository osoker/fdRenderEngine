////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_SoundFilePool.h
 *
 *  Desc:     声音文件资源池。
 *
 *  His:      raffaello created @ 2005-6-20
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _GC_SOUNDFILEPOOL_
#define _GC_SOUNDFILEPOOL_
#include "gc_r4Singleton.h"
#include "gc_r4ResourcePool.h"
#include "gc_SoundFile.h"


/** \brief
 *  对声音文件的内存进行Catch的管理器
 *  
 *  播放声音时，所有的声音文件在内存中都会保存到这个管理器中。 
 */
class gc_SoundFilePool:public I_r4ResourcePool<gc_SoundFile>
					  ,public gc_r4Singleton<gc_SoundFilePool>
{
};


//! 从sound文件资源池获得一个资源对象指针
inline gc_SoundFile*	get_soundfile(const char* _lpszFileName)
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())
		static gc_SoundFilePool _static_;
	return gc_SoundFilePool::getSingletonPtr()->get_resource(_lpszFileName);
}
//! 归还一个sound文件引用
inline bool	free_soundfile(const char* _lpszFileName)
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())
		static gc_SoundFilePool _static_;
	return gc_SoundFilePool::getSingletonPtr()->unload_resource(_lpszFileName);
}
//! 设定sound文件根修饰路径
inline void set_soundfile_rootpath(const char* _lpszRootPath)
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())
		static gc_SoundFilePool _static_;
	gc_SoundFilePool::getSingletonPtr()->set_rootpath(_lpszRootPath);
}
//! 销毁所有sound资源
inline void destory_allsoundfiles()
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())	return;
	gc_SoundFilePool::getSingletonPtr()->destory_allresources();
}
#endif//_GC_SOUNDFILEPOOL_