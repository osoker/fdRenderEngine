////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_SoundFile.h
 *
 *  Desc:     声音文件包裹类。
 *
 *  His:      raffaello created @ 2005-6-20
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _GC_SOUNDFILE_H_
#define _GC_SOUNDFILE_H_
#include "stdafx.h"
#include "gc_r4Resource.h"
#include "../interface/osInterface.h"

# include "dsound.h"



/** \brief
*  wave文件操作类
*
*  这个类主要负责文件的读取等操作,其中包括从文件和从内存读取
*/
class fdWaveFile
{
private:
	//! 文件格式部分
	WAVEFORMATEX             m_wfx;
	WAVEFORMATEX*            m_pwfx;

	//! 文件中数据部分的大小
	DWORD                    m_dwSize;

public:
	/** \brief
	*  构造函数
	*/
	fdWaveFile();

	/** \brief
	*  析构函数
	*/
	~fdWaveFile();

	/** \brief
	*  返回wave的数据格式
	*
	*  \return  WAVEFORMATEX* 返回wave格式 
	*/
	WAVEFORMATEX& GetFormat() { return *m_pwfx; }

	/** \brief
	*  得到文件的大小
	* 
	*  \return   DWORD         返回媒体数据的大小 
	*/
	DWORD         GetSize();

	/** \brief
	* 获得wave的格式和数据部分
	*
	* \param  pRes        整个wave文件在内存中的位置
	* \param  pWaveHeader 返回wave文件的格式
	* \param  pbWaveData  wave的数据部分
	* \param  cbWaveSize  wave数据部分的长度
	* 
	* \return BOOL        是否读取wave数据成功
	*/
	BOOL          GetWaveData(void * pRes, WAVEFORMATEX * & pWaveHeader, void * & pbWaveData, DWORD & cbWaveSize);
};



class gc_SoundFile:public I_r4Resource
{
public:
	gc_SoundFile()
	{
		bFever	=	true;
		m_pbData	=	NULL;
		m_sSoundBuffer = NULL;
		m_fSoundPlayTime=0.0f;
	}
	//! 从文件中读入一个资源
	virtual bool load(const char* lpszFileName = NULL );
	//! 销毁这个资源
	virtual bool destory();
	void         getSoundDes( DSBUFFERDESC& _sdesc ) 
	{ 
		memset( &_sdesc,0,sizeof( DSBUFFERDESC ) );
		memcpy( &_sdesc,&m_DSBD,sizeof( DSBUFFERDESC ) ); 
	} 

	LPVOID		  getDataBuff()	{	return m_pbData;}

	// 返回音效的播放时间
	float         getSoundPlayTime( void ){ return m_fSoundPlayTime; }
protected:
	static PBYTE	m_pbStart;
	LPVOID			m_pbData ;
	DSBUFFERDESC	m_DSBD;

	// wav的播放时间
	float           m_fSoundPlayTime;
public:
	LPDIRECTSOUNDBUFFER8   m_sSoundBuffer;
};
#endif//_GC_SOUNDFILE_H_