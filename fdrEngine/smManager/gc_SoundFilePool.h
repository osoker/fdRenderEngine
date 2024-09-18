////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_SoundFilePool.h
 *
 *  Desc:     �����ļ���Դ�ء�
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
 *  �������ļ����ڴ����Catch�Ĺ�����
 *  
 *  ��������ʱ�����е������ļ����ڴ��ж��ᱣ�浽����������С� 
 */
class gc_SoundFilePool:public I_r4ResourcePool<gc_SoundFile>
					  ,public gc_r4Singleton<gc_SoundFilePool>
{
};


//! ��sound�ļ���Դ�ػ��һ����Դ����ָ��
inline gc_SoundFile*	get_soundfile(const char* _lpszFileName)
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())
		static gc_SoundFilePool _static_;
	return gc_SoundFilePool::getSingletonPtr()->get_resource(_lpszFileName);
}
//! �黹һ��sound�ļ�����
inline bool	free_soundfile(const char* _lpszFileName)
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())
		static gc_SoundFilePool _static_;
	return gc_SoundFilePool::getSingletonPtr()->unload_resource(_lpszFileName);
}
//! �趨sound�ļ�������·��
inline void set_soundfile_rootpath(const char* _lpszRootPath)
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())
		static gc_SoundFilePool _static_;
	gc_SoundFilePool::getSingletonPtr()->set_rootpath(_lpszRootPath);
}
//! ��������sound��Դ
inline void destory_allsoundfiles()
{
	if(NULL==gc_SoundFilePool::getSingletonPtr())	return;
	gc_SoundFilePool::getSingletonPtr()->destory_allresources();
}
#endif//_GC_SOUNDFILEPOOL_