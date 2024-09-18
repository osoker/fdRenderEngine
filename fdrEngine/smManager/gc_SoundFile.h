////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_SoundFile.h
 *
 *  Desc:     �����ļ������ࡣ
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
*  wave�ļ�������
*
*  �������Ҫ�����ļ��Ķ�ȡ�Ȳ���,���а������ļ��ʹ��ڴ��ȡ
*/
class fdWaveFile
{
private:
	//! �ļ���ʽ����
	WAVEFORMATEX             m_wfx;
	WAVEFORMATEX*            m_pwfx;

	//! �ļ������ݲ��ֵĴ�С
	DWORD                    m_dwSize;

public:
	/** \brief
	*  ���캯��
	*/
	fdWaveFile();

	/** \brief
	*  ��������
	*/
	~fdWaveFile();

	/** \brief
	*  ����wave�����ݸ�ʽ
	*
	*  \return  WAVEFORMATEX* ����wave��ʽ 
	*/
	WAVEFORMATEX& GetFormat() { return *m_pwfx; }

	/** \brief
	*  �õ��ļ��Ĵ�С
	* 
	*  \return   DWORD         ����ý�����ݵĴ�С 
	*/
	DWORD         GetSize();

	/** \brief
	* ���wave�ĸ�ʽ�����ݲ���
	*
	* \param  pRes        ����wave�ļ����ڴ��е�λ��
	* \param  pWaveHeader ����wave�ļ��ĸ�ʽ
	* \param  pbWaveData  wave�����ݲ���
	* \param  cbWaveSize  wave���ݲ��ֵĳ���
	* 
	* \return BOOL        �Ƿ��ȡwave���ݳɹ�
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
	//! ���ļ��ж���һ����Դ
	virtual bool load(const char* lpszFileName = NULL );
	//! ���������Դ
	virtual bool destory();
	void         getSoundDes( DSBUFFERDESC& _sdesc ) 
	{ 
		memset( &_sdesc,0,sizeof( DSBUFFERDESC ) );
		memcpy( &_sdesc,&m_DSBD,sizeof( DSBUFFERDESC ) ); 
	} 

	LPVOID		  getDataBuff()	{	return m_pbData;}

	// ������Ч�Ĳ���ʱ��
	float         getSoundPlayTime( void ){ return m_fSoundPlayTime; }
protected:
	static PBYTE	m_pbStart;
	LPVOID			m_pbData ;
	DSBUFFERDESC	m_DSBD;

	// wav�Ĳ���ʱ��
	float           m_fSoundPlayTime;
public:
	LPDIRECTSOUNDBUFFER8   m_sSoundBuffer;
};
#endif//_GC_SOUNDFILE_H_