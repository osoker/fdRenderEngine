
# include "r4Sound.h"
# include "gc_SoundFilePool.h"
# include <vfw.h>
# include "fyMp3_lib.h"



#ifndef DSVOLUME
#	define DSVOLUME(volume)  ((volume==0)?-10000:(-60*(MAX_VOLUME-volume)))
#endif

# pragma comment( lib,"dxguid" )

//! �����п���������������������
# define MAX_SOUNDDIS    128.0f

//! ȫ�ֵ��������أ����Դ򿪺͹ر������Ĳ��ţ�Ĭ�ϴ򿪡�
SMMANAGER_API BOOL      g_bUseSound = TRUE;

//! ����ʹ�����������ȷ�������������Ƿ��ʼ�������û�г�ʼ�������ʱ������������ص�����
SMMANAGER_API BOOL      g_bUseSoundMgrHaveInit = FALSE;


void gcs_sound_buffer::stop_sound( void )
{
	guard;

	if( m_lpBuffer )
	{
		m_lpBuffer->Stop();
		m_lpBuffer->SetCurrentPosition( 0 );
	}

	//! ����ɾ����־
	start_vanish();

	unguard;
}



//template<> gc_r4SoundMng* gc_r4Singleton<gc_r4SoundMng>::ms_Singleton = 0;
//! ��ʼ����̬����
template<> gc_r4SoundMng* ost_Singleton<gc_r4SoundMng>::m_ptrSingleton = NULL;
gc_r4SoundMng::gc_r4SoundMng()
{
	guard;
	m_lCurMaxSoundIndex	=	0;
	m_lpDirectSound		=	NULL;
	m_p3DListener		=	NULL;
	m_bForcePlay		=	false;
	CREATESINGLETON(gc_SoundFilePool);

	m_sMp3Player = new fyc_MP3Lib();

	unguard;
}
gc_r4SoundMng::~gc_r4SoundMng()
{
	guard;

	SAFE_DELETE( m_sMp3Player );

	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	itor	=	m_mSoundBuffer.begin();
	for(;itor!=m_mSoundBuffer.end();itor++)
	{
		delete itor->second;
	}
	m_mSoundBuffer.clear();
	ReleaseSound();
	destory_allsoundResource();

	unguard;
}
void   gc_r4SoundMng::ResetSoundVolume()
{
	SetSoundVolume( SOUNDTYPE_3D );
	
	SetSoundVolume( SOUNDTYPE_UI );
}

void	gc_r4SoundMng::initSoundResourcePool(const char* _lpszRootpath)
{
	gc_SoundFilePool::getSingletonPtr()->set_rootpath(_lpszRootpath);
}



bool	gc_r4SoundMng::InitSoundManager(HWND _hWnd,DWORD _dwPrimaryFreq,
										DWORD _dwPrimaryBitRate,os_lisenerParam& _sListener,DWORD _dwCoopLevel)
{
	//! ���������ļ��أ���ʼ������·��
	SAFE_RELEASE(m_lpDirectSound);
	//	return false;
	// Create IDirectSound using the primary sound device
	//!default sound device,third should be null,use the default device
	if( FAILED( DirectSoundCreate8( NULL, &m_lpDirectSound, NULL ) ) )
	{
		//MessageBox( NULL, "��Ƶ��ʼ��ʧ��!", "����", MB_OK );
		osDebugOut( "��Ƶ��ʼ��ʧ��!\n" );
		return false;
	}
	char szTemp[64] = { 0 };
	m_deviceCap.dwSize = sizeof(DSCAPS); 
	HRESULT hr = m_lpDirectSound->GetCaps(&m_deviceCap); 

	//////////////////////////////////////////////////////////////////////////
	// Set DirectSound coop level 
	if(FAILED(m_lpDirectSound->SetCooperativeLevel(_hWnd, _dwCoopLevel)))
	{
		osDebugOut( "SOUND: error in SetCooperativeLevel\n" );
		return false; 
	}
	//! ����������
	if(!initPrimaryBufferFormat(_dwPrimaryFreq,_dwPrimaryBitRate))
		return false;
	// 
	//! ��ʼ����ά������λ��
	init3DListener(_sListener);

	SetSoundVolume( SOUNDTYPE_3D, MAX_VOLUME/2 );
	SetSoundVolume( SOUNDTYPE_UI, MAX_VOLUME/2 );

	// �����������Ѿ���ʼ��
	g_bUseSoundMgrHaveInit = TRUE;

    m_sMp3Player->InitPlayer( _hWnd );

	return true;
}
gcs_sound_buffer*	gc_r4SoundMng::getSoundBuffer(const gcs_sound_id& _soundID)
{
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	itor	=	m_mSoundBuffer.find(_soundID);
	if(itor!=m_mSoundBuffer.end())	return itor->second;
	return NULL;
}


/** \brief
 *  ��һ���Ѿ����Ź��������������������ã�ʹ�û������ڴ��ڵ�������Դ�����²���
 *
 *  \param _pbuffer  Ҫ�����������������
 *  \param _fPlayTime �ݲ�ʹ�ô˲��������ڿ����������ŵ�ʱ�䳤�̡�
 */
gcs_sound_buffer*	gc_r4SoundMng::resetSoundBuffer( gcs_sound_buffer* _pbuff )
{
	guard;
	osassert( _pbuff );

	
	_pbuff->m_lpBuffer->Stop();
	_pbuff->m_lpBuffer->SetCurrentPosition(0);
	_pbuff->m_iCount = -1;

	return _pbuff;

	unguard;
}

// 
/** \brief
 *  ���������ļ������ִ���һ������������Ϊ�����仺��
 * 
 *  \param const char* _lpszFilename Ҫ�������������ļ���
 *  \param float _fPlayTime          ����Ҫ���ŵ�ʱ�䣬Ŀǰ�ݲ�ʹ�ô˲�����
 *  \param bool  _bForce             �Ƿ񴴽���dSound��Ӳ��������������,�Ż�ʱʹ��
 */
gcs_sound_buffer* gc_r4SoundMng::createSoundBuffer( const char* _lpszFilename,
	          float* _fPlayTime,bool _bForce/* = false*/,bool _bUI/* = false*/ )
{
	guard;

	// ��������豸Ϊ��,�򷵻�.
	if( NULL == m_lpDirectSound )
		return NULL;

	long	lVolume = 0;
	gcs_sound_buffer* _pbuff = NULL;
	DSBUFFERDESC      t_sDesc;
	LPDIRECTSOUNDBUFFER t_soundBuffer;
	LPDIRECTSOUNDBUFFER8 t_ppDsb8 = NULL;
	HRESULT t_hr;
	HRESULT hrRet = S_OK;

	// �������ڴ�������еõ���ǰ�������ڴ�
	gc_SoundFile* _pSoundFile	=	gc_SoundFilePool::getSingletonPtr()->get_resource(_lpszFilename);
	if(NULL==_pSoundFile)	
	{
		osassertex( false,"���������ļ�ʧ��...\n" );
		return NULL;
	}

	_pSoundFile->getSoundDes( t_sDesc );
	if( _fPlayTime )
        *_fPlayTime = _pSoundFile->getSoundPlayTime();

	// �����ui����,��ʹ����ص���ά��־
	if( _bUI )
	{
		t_sDesc.dwFlags = DSBCAPS_CTRLVOLUME;
		t_sDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
		lVolume = m_lUISoundVolume;
	}
	else
		lVolume = m_lSoundVolume;

	// ͷһ�δ���
	if( NULL == _pSoundFile->m_sSoundBuffer )
	{
		t_hr = m_lpDirectSound->CreateSoundBuffer( &t_sDesc, &t_soundBuffer, NULL );

		// River mod @ 2007-8-26:�������Ĵ���������������������
		if( FAILED(t_hr) )
		{
			MessageBox( NULL,"�������������������򣬴�������������ʧ��!","ERROR",MB_OK );
			osDebugOut( "����ԭ��:<%s>...\n",DXGetErrorDescription9( t_hr ) );
			g_bUseSoundMgrHaveInit = FALSE;
			ReleaseSound();
			return NULL;
		}

		//
		// Be sure to return this error code if it occurs so the
		// callers knows this happened.
		if( t_hr == DS_NO_VIRTUALIZATION )	
			hrRet = DS_NO_VIRTUALIZATION;           

		if( FAILED(t_hr) )
		{       
			// It might also fail if hardware buffer mixing was requested
			// on a device that doesn't support it.
			_pSoundFile->getSoundDes( t_sDesc );
			osDebugOut( "SOUND: %s ����������ʱ����!\n", _lpszFilename );

			osDebugOut( "nAvgBytesPerSec(%d), nBlockAlign(%d),nChannels(%d)",
				t_sDesc.lpwfxFormat->nAvgBytesPerSec, 
				t_sDesc.lpwfxFormat->nBlockAlign,t_sDesc.lpwfxFormat->nChannels );
			osDebugOut( "nSamplesPerSec(%d), wBitsPerSample(%d),dwBufferBytes(%d)\n",
				t_sDesc.lpwfxFormat->nSamplesPerSec, 
				t_sDesc.lpwfxFormat->wBitsPerSample,
				t_sDesc.dwBufferBytes );

			return NULL;
		}

		// �õ��½ӿڣ��ͷ�soundBuffer�ӿ�	
		t_hr = t_soundBuffer->QueryInterface(IID_IDirectSoundBuffer8, 
			(LPVOID*) &_pSoundFile->m_sSoundBuffer );

		_pSoundFile->getSoundDes( t_sDesc );
		if( FAILED(fillBufferWithSound(
			(BYTE*)_pSoundFile->getDataBuff(),
			t_sDesc.dwBufferBytes,_pSoundFile->m_sSoundBuffer,true)) )
		{
			//osassertex( false,"���������������������ʧ��...\n" );
			return NULL;
		} 

		// 
		// ���û�����������,ÿ������һ����������������Ҫ���ô˻�����������.
		if( FAILED( t_hr = _pSoundFile->m_sSoundBuffer->SetVolume( DSVOLUME(lVolume)) ) )
		{
			//osassertex( false,(char*)DXGetErrorDescription9( t_hr ) );
			return NULL;
		}

		SAFE_RELEASE( t_soundBuffer );

		// ɾ����ÿһ������������ڴ棬ֻ����������������ok
		_pSoundFile->destory();
	}


	_pbuff = new gcs_sound_buffer;
	

	//�� DuplicateSoundBuffer���������£��ػش�ִ�У�
RESTORE_SOUNDBUFF_PROCESS:

	t_hr = m_lpDirectSound->DuplicateSoundBuffer( 
		_pSoundFile->m_sSoundBuffer,&t_soundBuffer );

	//! River @ 2010-10-8:�������Ĵ�����
	if( !FAILED( t_hr ) )
	{
		t_hr = t_soundBuffer->QueryInterface(IID_IDirectSoundBuffer8, 
			(LPVOID*) &t_ppDsb8 );
		osassertex( (!FAILED(t_hr)),(char*)DXGetErrorDescription9( t_hr ) );
	}
	else
	{
		// River @ 2011-2-14:�����Restore�����������Ĵ���
		if( t_hr == DSERR_BUFFERLOST )
		{
			_pSoundFile->getSoundDes( t_sDesc );
			_pSoundFile->load( NULL );
			if( FAILED(fillBufferWithSound(
				(BYTE*)_pSoundFile->getDataBuff(),
				t_sDesc.dwBufferBytes,_pSoundFile->m_sSoundBuffer,true)) )
			{
				//osassertex( false,"�ָ����������������������ʧ��...\n" );
				return NULL;
			} 
			_pSoundFile->destory();
			goto RESTORE_SOUNDBUFF_PROCESS;
		}
		else
		{
			//osassertex( false,(char*)DXGetErrorDescription9( t_hr ) );
			return NULL;
		}
	}

	SAFE_RELEASE( t_soundBuffer );

	_pbuff->m_lpBuffer	=	t_ppDsb8;
	_pbuff->m_iCount	=	-1;
	_pbuff->m_bUI		=	false;

	//! River added @ 2009-5-11:catch �������ŵ�ʱ��
	if( _fPlayTime )
		_pbuff->m_fPlayTime = *_fPlayTime;

	return _pbuff;

	unguard;
}



/** \brief
*  ������ϵͳ�Ļ����в��ŵ�ǰ�������ļ�
*
*  ���ϵͳ�Ỻ��洢һЩ������ÿ�β���������ʱ���ȴ�ϵͳ�����в��ң�
*  ���Ҫ���ŵ�������������ϵͳ�����У���ʹ�û����е�������
*
*/
bool gc_r4SoundMng::playSoundFromPool( gcs_sound_id* _pid,float* _playTime,
		const char* _lpszFileName,const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI )
{
	guard;

	unsigned long _lhashvalue	=	string_hash(_lpszFileName);
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	itor;

	for( itor = m_mSoundBuffer.begin();itor!=m_mSoundBuffer.end();itor++ )
	{
		gcs_sound_buffer* _pbuff	=	itor->second;

		//  ����ͬ��hashId���һ��滺�����ǲ���״̬
		if( (itor->first.hash_value == _lhashvalue)
			 && (_pbuff->isVanish()) )
		{
			_pbuff	=	resetSoundBuffer( _pbuff );

			osassertex( _pbuff->m_lpBuffer,"����Ļ���������Ϊ��...\n" );

			// ���������Ͳ�һ��.
			if( _pbuff->m_bUI != _bUI )
				continue;

			m_mSoundBuffer.erase(itor);
			gcs_sound_id t_id(_lhashvalue,++m_lCurMaxSoundIndex);
			m_mSoundBuffer.insert( std::pair<gcs_sound_id,gcs_sound_buffer*>(t_id,_pbuff) );
	
			// ������ֵ
			_pid->hash_value	=	_lhashvalue;
			_pid->value			=	m_lCurMaxSoundIndex;
			
			LPDIRECTSOUNDBUFFER8 t_pDSB = _pbuff->m_lpBuffer;
			HRESULT             hr;
			BOOL               t_bRestored;

			// ȷ��buf״̬
			if( FAILED( restoreBuffer(t_pDSB, &t_bRestored)) )	
				return false;

			if(_pbuff->m_bUI)
			{
				// ��3D��Ч�Ĳ��ŷ�ʽ
				// �ڶ�������������0
				t_pDSB->SetVolume( DSVOLUME(m_lUISoundVolume) );
				if( FAILED(t_pDSB->Play(0,0,_bLoop?DSBPLAY_LOOPING:0)) )
				{
					delete_sound( t_id );
					return false;
				}
				return true;
			}

			// QI for the 3D buffer 
			LPDIRECTSOUND3DBUFFER t_pDS3DBuffer; 

			hr = (t_pDSB)->QueryInterface( IID_IDirectSound3DBuffer, (VOID**) &t_pDS3DBuffer );
		
			if(SUCCEEDED(hr))
			{
				if( !set_bufferPara( _bufferPara,t_pDS3DBuffer ) )
				{
					// ĳЩ��������
					osassert(false);
					delete_sound( t_id );
					return false;
				}

				// ��ʼ������Ч
				t_pDSB->SetVolume( DSVOLUME(m_lSoundVolume) );
				if(FAILED((t_pDSB)->Play(0,0xFFFFFFFF,_bLoop?DSBPLAY_LOOPING:0)) )
				{
					delete_sound( t_id );
					return false;
				}

				t_pDS3DBuffer->Release();
			}

			//! River added @ 2009-5-11:�����������ŵ�ʱ�䡣
			if( _playTime )
				*_playTime = _pbuff->m_fPlayTime;

			return true;
		}
	}

	return false;

	unguard;
}


//! �ӵ�ǰ������������������ɾ��һ������
void gc_r4SoundMng::delete_sound( gcs_sound_id& _id )
{
	guard;

	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	itor;
	itor	=	m_mSoundBuffer.find( _id );

	if( itor!=m_mSoundBuffer.end() )
	{
		SAFE_DELETE( itor->second );
		m_mSoundBuffer.erase( itor );
		m_lCurMaxSoundIndex--;
	}

	return;

	unguard;
}


//! ����soundBuffer�Ĳ���
bool gc_r4SoundMng::set_bufferPara( const os_initSoundSrc* _bufferPara,LPDIRECTSOUND3DBUFFER _buf )
{
	guard;

	DS3DBUFFER t_srcBuf;
	HRESULT    t_hr;

	t_srcBuf.dwSize = sizeof(DS3DBUFFER);

	// ���3D Buffer��Ĭ�ϲ���
	if( FAILED( t_hr = _buf->GetAllParameters(&t_srcBuf) ) )
	{
		osassertex( false,(char*)DXGetErrorDescription9( t_hr )  );			
		return false;
	}

	// ��ʼ��3D Buffer��3D����
	t_srcBuf.flMinDistance			   = _bufferPara->m_flMinDistance;
	t_srcBuf.flMaxDistance			   = _bufferPara->m_flMaxDistance;
	t_srcBuf.vPosition                = _bufferPara->m_vPosition;		
	t_srcBuf.lConeOutsideVolume        = DSVOLUME(m_lSoundVolume);			

	// ����3D Buffer�Ĳ���
	if( FAILED( t_hr = _buf->SetAllParameters(&t_srcBuf, DS3D_IMMEDIATE)) )
	{
		osassertex( false,(char*)DXGetErrorDescription9( t_hr )  );			
		return false;
	}

	return true;

	unguard;
}


/** \brief
 *  �������������ĺ����������˵�ǰ���ŵ�hash ID.
 *
 *  �㷨������
 *  1:
 *  2: 
 */
gcs_sound_id gc_r4SoundMng::playSound( const char* _lpszFileName,
				const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI, float* _fSoundPlayTime/*=NULL*/ )
{
	guard;


	// _bUIΪfalseʱ,_bufferPara����ΪNULL.
	osassertex( ((_bufferPara)||(_bUI)),"bufferΪ��ʱ,���ܲ�����ά����..\n" );
	if( _bufferPara )
		osassert( _bufferPara->m_flMinDistance >= 0.0 );

	if( !g_bUseSound )
		return gcs_sound_id(-1);

	//! ���ȿ�һ�µ�ǰ��������û����ͬ�ļ����Ĵ�ɾ��������������еĻ�ֱ�Ӳ������Ϳ�����
	gcs_sound_id   t_idx;
	if( playSoundFromPool( &t_idx,
		_fSoundPlayTime,_lpszFileName,_bufferPara,_bLoop,_bUI ) )
		return t_idx;


	// 
	// �Ӵ����ϲ���,�������������0
	gcs_sound_id   t_iResId( string_hash(_lpszFileName),++m_lCurMaxSoundIndex );
	gcs_sound_buffer*	_pbuff;
	
	_pbuff = createSoundBuffer( _lpszFileName,_fSoundPlayTime,m_bForcePlay,_bUI );
	if( !_pbuff)
	{
		osDebugOut("ͬʱ��������������<%d>,���޻���������...\n",m_deviceCap.dwFreeHwMixingAllBuffers);
		return gcs_sound_id(-1);
	}


	_pbuff->m_bUI	=	_bUI;

	m_mSoundBuffer.insert(
		std::pair<gcs_sound_id,gcs_sound_buffer*>( t_iResId,_pbuff ) );

	LPDIRECTSOUNDBUFFER8 t_pDSB = _pbuff->m_lpBuffer;
	HRESULT  hr;
	BOOL    t_bRestored;

	// ȷ��������������״̬
	if( FAILED(restoreBuffer(t_pDSB, &t_bRestored)) )	
		return gcs_sound_id(-1);

	// 
	// �����Ƕ�ά����������ά������ʹ�ò�ͬ�Ĳ��ų����֧
	if( _pbuff->m_bUI )
	{
		// ��3D��Ч�Ĳ��ŷ�ʽ,�ڶ�������������0, �������ʧ�ܣ�ɾ����ǰ����Ч
		t_pDSB->SetVolume( DSVOLUME(m_lUISoundVolume) );
		if( FAILED(t_pDSB->Play( 0,0,_bLoop?DSBPLAY_LOOPING:0)) )
		{
			delete_sound( t_iResId );
			return gcs_sound_id(-1);
		}
	}
	else
	{
		// 
		// QI for the 3D buffer 
		LPDIRECTSOUND3DBUFFER       t_pDS3DBuffer; 
		hr = (t_pDSB)->QueryInterface( 
			IID_IDirectSound3DBuffer, (VOID**) &t_pDS3DBuffer );
		if( SUCCEEDED(hr) )
		{
			if( !set_bufferPara( _bufferPara,t_pDS3DBuffer ) )
			{
				delete_sound( t_iResId );
				return gcs_sound_id(-1);
			}

			// ��ʼ������Ч
			t_pDSB->SetVolume( DSVOLUME(m_lSoundVolume) );
			if( FAILED((t_pDSB)->Play(0,0xFFFFFFFF,_bLoop?DSBPLAY_LOOPING:0)) )
			{
				delete_sound( t_iResId );
				return gcs_sound_id(-1);
			}

			t_pDS3DBuffer->Release();
		}
	}

	return t_iResId;

	unguard;
}


void		 gc_r4SoundMng::StopSound( DWORD _dwID )
{
	//! stop��ʱ������buffɾ��
	//! Ҫ��Ҫ����һ��ʱ����ɾ���أ�
	// ��Ƶ��ʼ��ʧ��,������Ч����ԴID
	if (m_lpDirectSound == NULL)	return;
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator itor = m_mSoundBuffer.find(gcs_sound_id(_dwID));
	if(itor!=m_mSoundBuffer.end())
	{
		itor->second->stop_sound();
		return ;
	}
	return;
}


/** \brief
*  ���÷������������ ÿһ֡��һ�δ���
*
*  \param   _dwID              ����������Secondory buffer��Ψһ��Ӧ��ʶ  
*  
*           _vecSoundPos       ���������λ�� 
*         
*  \return  bool               ���ڲ���״̬true;���򷵻�false
*/
bool gc_r4SoundMng::FrameMoveSndPos(DWORD _dwID,osVec3D& _vecSoundPos,osVec3D* _velocity/* = NULL*/)
{
	guard;

	if(m_lpDirectSound == NULL)		return false;

	LPDIRECTSOUND3DBUFFER t_pDS3DBuffer;
	DWORD                 t_dwStatus;
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	itor;

    itor = m_mSoundBuffer.find(gcs_sound_id(_dwID));
	if( itor!=m_mSoundBuffer.end() )
	{
		//! �������������λ����Ϣ
		gcs_sound_buffer*	_pbuff	=	itor->second;
		if(_pbuff->m_bUI)		return true;
		// �жϵ�ǰ��Դ�Ƿ����ڲ���
		if(_pbuff->isVanish()) return false;

		_pbuff->m_lpBuffer->GetStatus(&t_dwStatus);
		if((t_dwStatus&DSBSTATUS_PLAYING)==0)
		{
			_pbuff->m_lpBuffer->Stop();
			return false;
		}
		// ��������ĸ�ʽ
		WAVEFORMATEX bufFormat;
		_pbuff->m_lpBuffer->GetFormat( &bufFormat, sizeof(WAVEFORMATEX), NULL );
		if (bufFormat.nChannels>1)
			return true;

		// ���3D���ƽӿ�
		_pbuff->m_lpBuffer->QueryInterface(IID_IDirectSound3DBuffer,(VOID**)&t_pDS3DBuffer );
		if (!t_pDS3DBuffer)
		{
			osassertex( false, "���3D������ʧ��" );
			return true;
		}

		//! ����������̫Զ������ҲҪɾ��,���¹�ʽ�൱��osVec3Length
		float _fDis	=	(float)sqrt( (m_sListenParams.vPosition.x-_vecSoundPos.x)*
			(m_sListenParams.vPosition.x-_vecSoundPos.x )+
			(m_sListenParams.vPosition.y-_vecSoundPos.y)*(m_sListenParams.vPosition.y-_vecSoundPos.y)+
			(m_sListenParams.vPosition.z-_vecSoundPos.z)*(m_sListenParams.vPosition.z-_vecSoundPos.z));

		
		// RIVER MOD 2008-3-21:TEST CODE
		// ��������ľ���̫Զ��ֹͣ��ǰ��������
		if( _fDis>MAX_SOUNDDIS )
		{
			_pbuff->stop_sound();
			return true;
		}
		/**/

		DS3DBUFFER              t_s3DBufPara;
		ZeroMemory(&t_s3DBufPara,sizeof(DS3DBUFFER));
		t_s3DBufPara.dwSize = sizeof(DS3DBUFFER);
		// ���������������Ĭ�ϲ���
		t_pDS3DBuffer->GetAllParameters(&t_s3DBufPara);
		t_s3DBufPara.vPosition = _vecSoundPos;
		/*
		if( _velocity )
			t_s3DBufPara.vVelocity = *_velocity;
		*/
		t_pDS3DBuffer->SetAllParameters(&t_s3DBufPara,DS3D_IMMEDIATE);

		return true;
	}
	return false;
	unguard;
}


/** \brief
*   ��������֡���ţ�������ϵͳ�ڵ�ÿһ�����������������ɾ����Ӧ��������
*/
void		 gc_r4SoundMng::FrameMove()
{
	guard;

	// ����ǰ�����ڵ�ÿһ������Buffer.
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	
		                       itor	=	m_mSoundBuffer.begin();

	for(;itor!=m_mSoundBuffer.end();itor++)
	{
		// ���������ڻ����ڼ������
		if( (itor->second->isVanish() )  )
		{
			if( (itor->second->m_iCount>0 ) )
			{
				itor->second->m_iCount--;

				// 
				// �ѵ��ڵĻ�������ɾ��
				if( 0 == itor->second->m_iCount )
				{
					SAFE_DELETE( itor->second );
					m_mSoundBuffer.erase(itor);
					itor	=	m_mSoundBuffer.begin();
				
					// river mod remote:
					// �����ʱû�������ɴ�����ֱ���˳�
					if( 0 == m_mSoundBuffer.size() )
						break;
					else
						continue;				
				}

			}
		}
		else // �������ڲ��ŵ�������
		{
			DWORD dwStatus;
			itor->second->m_lpBuffer->GetStatus(&dwStatus);

			// �Ѿ������꣬��ʼ���������
			if( (dwStatus&DSBSTATUS_PLAYING) == 0 )
				itor->second->start_vanish();
		}
	}


	// ��mp3����move,�����ѭ�����ţ�����Ҫ�ں����ڴ���
	osassert( m_sMp3Player );
	m_sMp3Player->FrameMove();


	return;
	unguard;
}


//**********************************************************************************
//***************************MP3�������ֵĲ���**************************************
//! Attention:  ����mp3�Ĳ�������Թ��������г�ʼ��,�����Ҳ����Release
//**********************************************************************************
//{@!����Ϊmp3�������ֵĲ���,��ʵ����һЩ��ʽ,Ʃ��wav��Ҳ���Բ��ţ���Ҫ����Ϊʹ
//    ����DShou����Ӧ�Ľӿ�,��Ч�ʿ϶�û�������Լ������DsoundЧ�ʸ�.
/** \brief
*  ���ļ�����mp3��������
*
*  \return   bool              ����ֵ�����Ƿ񲥷ųɹ�
*/
bool gc_r4SoundMng::PlayFileMP3( const char *_FileName,BOOL _repeat/* = TRUE*/ )
{
	if( !g_bUseSoundMgrHaveInit )
		return false;

	m_CurMp3FileName = _FileName;
	return m_sMp3Player->PlayFileMP3( _FileName,_repeat );
}

/** \brief
*  ֹͣ����mp3
*
*  ������������Կ����ڴ沥���ֿ��Կ����ļ�����
*
*  \return    bool   ֹͣʱ�Ƿ�ɹ�
*/
bool gc_r4SoundMng::StopMP3()
{ 
	if( m_sMp3Player )
		return m_sMp3Player->StopMP3() ; 

	return true;
}

//@{ River @ 2010-8-20:����õ�mp3����λ�ú�����mp3����λ�õĽӿڡ�
LONGLONG gc_r4SoundMng::GetMp3PlayPosition( void )
{ 
	if( m_sMp3Player )
		return m_sMp3Player->GetPosition(); 

	return 0;
}
void gc_r4SoundMng::SetMp3PlayPosition( LONGLONG  _pos )
{
	if( m_sMp3Player )
		m_sMp3Player->SetPosition( _pos );
}
//@}


/** \brief
*  ��ͣ����
*
*  ������������Կ����ڴ沥���ֿ��Կ����ļ�����
*
*  \return    bool   ��ͣ����
*/
bool gc_r4SoundMng::PauseMP3( bool _pause/* = true*/ )
{ 
	if( m_sMp3Player ) 
		return m_sMp3Player->PauseMP3( _pause ); 
	else
		return true;

}

/** \brief
*  ��������,ʹ��һ�ξ���,�ٵ���һ���Ϳɻָ�����
*/
void gc_r4SoundMng::ToggleMuteMP3( bool _mute/* = true*/ )
{ 
	if( m_sMp3Player )
		m_sMp3Player->ToggleMuteMP3( _mute ); 
}


/** \brief
*  ��������
*
*  \_lVolume �µ�����ֵ(0----100)
*/
bool gc_r4SoundMng::SetVolumeMP3( long _lVolume )
{ 
	if( m_sMp3Player )
		return m_sMp3Player->SetVolume( _lVolume);
	else
		return true;
}

/** \brief
*  ��ñ������ֵ�����
*
*  \param    _Volume �µ�����ֵ
*/
long gc_r4SoundMng::GetVolumeMP3()
{
	if( m_sMp3Player )
		return m_sMp3Player->GetVolume();

	return 0;
}

const char* gc_r4SoundMng::GetCurMp3FileName()
{
	return m_CurMp3FileName.c_str();
}
//}@

/** \brief
*	���·���Buffer
*
*  ���������lost,��Ҫ���·���
*
*  \param   _pDSB           �λ�����
*  \param   BOOL _pbWasRestored  �Ƿ����·�����buffer
*/
HRESULT gc_r4SoundMng::restoreBuffer(LPDIRECTSOUNDBUFFER8 _pDSB, BOOL* _pbWasRestored)
{
	guard;

	HRESULT t_hr;

	osassert( _pDSB );
	if( _pbWasRestored )
		*_pbWasRestored = FALSE;

	DWORD t_dwStatus;

	t_hr = _pDSB->GetStatus( &t_dwStatus );
	if( FAILED( t_hr ) )
	{
		//osassert(false);
		return t_hr;
	}

	if( t_dwStatus & DSBSTATUS_BUFFERLOST )
	{
		// Since the app could have just been activated, then
		// DirectSound may not be giving us control yet, so 
		// the restoring the buffer may fail.  
		// If it does, sleep until DirectSound gives us control.
		int  t_iTimes = 0;
		do 
		{
			t_iTimes ++;
			t_hr = _pDSB->Restore();
			if( t_hr == DSERR_BUFFERLOST )	Sleep( 10 );

			// River @ 2011-2-14:�����жϣ���������ѭ��
			if( t_iTimes >= 10 )
				return DSERR_INVALIDCALL;

		}while( (t_hr = _pDSB->Restore()) == DSERR_BUFFERLOST );

		if( _pbWasRestored != NULL )	
			*_pbWasRestored = TRUE;

	}

	return S_OK;

	unguard;
}
/** \brief
*  �������������ĸ�ʽ
*
*  ���������൱��һ����Ч�����,��Ϸ�����е���Ч�������ﰴ��һ���ķ�ʽ���л��
*  Ȼ�����,������Ϸ��ֻ����һ����������������
*
*  \param   _dwPrimaryFreq     �������ֵĲ���Ƶ��
*           _dwPrimaryBitRate  ����λ��bit���
*/
bool gc_r4SoundMng::initPrimaryBufferFormat(DWORD _dwPrimaryFreq, DWORD _dwPrimaryBitRate )
{
	guard;
	LPDIRECTSOUNDBUFFER t_pDSBPrimary = NULL;
	if( m_lpDirectSound == NULL )
		return false;
	// Get the primary buffer 
	DSBUFFERDESC t_dsbd;
	ZeroMemory( &t_dsbd, sizeof(DSBUFFERDESC) );
	t_dsbd.dwSize        = sizeof(DSBUFFERDESC);
	t_dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	t_dsbd.dwBufferBytes = 0;
	t_dsbd.lpwfxFormat   = NULL;
	//!the third parameter should be null because of �ۼ� is not suported!
	if( FAILED(m_lpDirectSound->CreateSoundBuffer( &t_dsbd, &t_pDSBPrimary, NULL ) ) )
	{
		osDebugOut("%s","SOUND: ������������ʧ��!");
		return false;
	}
	WAVEFORMATEX t_wfx;
	ZeroMemory( &t_wfx, sizeof(WAVEFORMATEX) ); 
	//!�����ǵ���Ϸ�б�����WAVE_FORMAT_PCM
	t_wfx.wFormatTag        = (WORD) WAVE_FORMAT_PCM;
	// 
	// 3D ����ʱ����������Ϊ1
	// River @ 2005-7-29:3d����Ҫ�������ļ���������ĿΪ1�����������������û��Ҫ��. 
	// 
	t_wfx.nChannels       = 2;                                                      
	t_wfx.nSamplesPerSec  = (DWORD)_dwPrimaryFreq; 
	t_wfx.wBitsPerSample  = (WORD) _dwPrimaryBitRate; 
	t_wfx.nBlockAlign     = (WORD) (t_wfx.wBitsPerSample / 8 * t_wfx.nChannels);
	t_wfx.nAvgBytesPerSec = (DWORD)(t_wfx.nSamplesPerSec * t_wfx.nBlockAlign);
	// ������Ч��ϵ���������
	if( FAILED( t_pDSBPrimary->SetFormat(&t_wfx) ) )
	{ 
		osDebugOut("%s","SOUND: ������������ʧ��!");
		return false;
	}
	SAFE_RELEASE(t_pDSBPrimary );
	return true;
	unguard;
}

/** \brief
*  ����������,��Ҫ���������������ĳ�ʼ��
*
*  ������Ϸ��ֻ��һ��������,Ҳ������ҽ�ɫ
*
*  \param   _fDopplerFactor  ������Ƶ������,����ģ���ٶȱȽϿ�����巢��Ч��
*           _fRolloffFactor  �����ڿ����е�˥������,0��10,0Ϊ��˥��,�����������������������
*                            �����MinDis��MaxDis,����Ч����ȫ�ֵ�,Ӱ��ÿһ��buffer
*/
void gc_r4SoundMng::init3DListener(os_lisenerParam &_sListener)
{
	guard;
	DSBUFFERDESC        t_dsbdesc;
	LPDIRECTSOUNDBUFFER t_pDSBPrimary = NULL;
	if( m_lpDirectSound == NULL )
	{
		osassert(false);
		return;
	}
	if( m_p3DListener == NULL )
	{
		// Obtain primary buffer, asking it for 3D control
		ZeroMemory( &t_dsbdesc, sizeof(DSBUFFERDESC) );
		t_dsbdesc.dwSize = sizeof(DSBUFFERDESC);
		t_dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
		if( FAILED( m_lpDirectSound->CreateSoundBuffer( &t_dsbdesc, &t_pDSBPrimary, NULL ) ) )
		{
			osassert(false);
			return;
		}
		// ���Listener�ӿ�
		if( FAILED( t_pDSBPrimary->QueryInterface( IID_IDirectSound3DListener, 
			(VOID**)&m_p3DListener ) ) )
		{
			osDebugOut("%s","SOUND: ��ȡDirectSound3DListener����!");
			SAFE_RELEASE(t_pDSBPrimary);
			//osassert("SOUND: ��ȡDirectSound3DListener����!"&&false);
			return;
		}
		// Release the primary buffer, since it is not need anymore
		SAFE_RELEASE( t_pDSBPrimary );
		// ��õ�ǰ�������߲���  
		m_sListenParams.dwSize=sizeof(DS3DLISTENER);
		m_p3DListener->GetAllParameters(&m_sListenParams);   

	}
	// �������ǵ�λ��
	if(FAILED(m_p3DListener->SetPosition(_sListener.m_vPosition.x,
		_sListener.m_vPosition.y,_sListener.m_vPosition.z,DS3D_IMMEDIATE)))
	{
		osDebugOut("%s","SOUND:  ����Listener����ʱ����!");
		//osassert("SOUND:  ����Listener����ʱ����!"&&false);
		return;
	}
	// �������ǵ�Oriention
	if(FAILED(m_p3DListener->SetOrientation(
		_sListener.m_vOrientFront.x,_sListener.m_vOrientFront.y
		,_sListener.m_vOrientFront.z ,_sListener.m_vOrientTop.x ,
		_sListener.m_vOrientTop.y ,_sListener.m_vOrientTop.z ,DS3D_IMMEDIATE)))
	{
		osDebugOut("%s","SOUND:  ����Listener����ʱ����!");
		//osassert("SOUND:  ����Listener����ʱ����!"&&false);
		return;
	}
	// ���þ�������
	m_p3DListener->SetDistanceFactor(1.0f, DS3D_IMMEDIATE);

	memcpy( &m_sCatchLp,&_sListener,sizeof( os_lisenerParam ) );

	unguard;
}


/** \brief
*  Ϊָ����secondory buffer���wave����
*
*  \param   _szFileName     wave�ļ���
*           _pDSB           �λ�����ָ��
*           _bRepeatIfLarger����λ�����̫���Ƿ�����ظ�����
*/
HRESULT gc_r4SoundMng::fillBufferWithSound( BYTE* _pbWave, int _dataSize, 
					LPDIRECTSOUNDBUFFER8 _pDSB, BOOL _bRepeatWavIfBufferLarger )
{
	HRESULT hr; 
	VOID*   t_pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
	DWORD   t_dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
	DWORD   t_dwWavDataRead        = 0;    // Amount of data read from the wav file 
	if( _pDSB == NULL )	return CO_E_NOTINITIALIZED;
	// Make sure we have focus, and we didn't just switch in from
	// an app which had a DirectSound device
	if( FAILED( hr = restoreBuffer(_pDSB,NULL)))
	{
		//osassert(false);
		return S_FALSE;
	}
	// Lock the buffer down
	if( FAILED( hr = _pDSB->Lock( 0, 0, //!offset and size to lock
		&t_pDSLockedBuffer, &t_dwDSLockedBufferSize,//!return lock address&size 
		NULL,NULL,DSBLOCK_ENTIREBUFFER )))//!the second locked part and lock flags
	{
		osassert(false);
		return S_FALSE;
	} 
	// �����Դ������ 
	ZeroMemory(t_pDSLockedBuffer,t_dwDSLockedBufferSize);
	CopyMemory(t_pDSLockedBuffer,_pbWave,_dataSize); 
	_pDSB->Unlock(t_pDSLockedBuffer,t_dwDSLockedBufferSize,NULL,0);//!same to lock  

	return S_OK;
}
/** \brief
*  �ͷ��������������õ�����Դ
*/
void gc_r4SoundMng::ReleaseSound()
{
	guard;
	
	SAFE_RELEASE(m_p3DListener);
	SAFE_RELEASE(m_lpDirectSound );
	osDebugOut("Sound:  Sound Manager Release");

	unguard;
}
/** \brief
*   �жϵ�ǰ��Դ�Ƿ����ڲ���
*
*    \param _dwID              ��ԴID
*
*    \return bool                   �������ŷ���true
*/
bool gc_r4SoundMng::IsPlayOver(DWORD _dwID)
{
	guard;
	if (m_lpDirectSound == NULL)
	{
		osDebugOut("SOUND: DirectSound ��ʼ��ʧ��");  
		return true;
	}

	// ��ǰ��Դ�Ƿ����ڲ���
	DWORD    t_dwStatus;
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	itor	=	m_mSoundBuffer.find(gcs_sound_id(_dwID));
	if( itor!=m_mSoundBuffer.end() )
	{
		if( itor->second->isVanish() ) 
			return true;
		itor->second->m_lpBuffer->GetStatus( &t_dwStatus );
	}
	else 
		return true;

	return (t_dwStatus&DSBSTATUS_PLAYING) == 0;

	unguard;
}
/** \brief
*   ������Ч������
*
*    \param		_lVolume     ������Ч������(0---100)
*/
void  gc_r4SoundMng::SetSoundVolume(  OS_SOUNDTYPE _type, long _lVolume/* = 75*/  )
{
	guard;

	if( !m_lpDirectSound )
		return;

	// �Ѵ��������ת����DSound Ҫ��ķ�Χ
	if( _type == SOUNDTYPE_3D )
		m_lSoundVolume   = max( MIN_VOLUME, min(_lVolume,MAX_VOLUME) );
	else
		m_lUISoundVolume = max( MIN_VOLUME, min(_lVolume,MAX_VOLUME) );
	
	// �ı�ÿ�����������
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator itor = m_mSoundBuffer.begin();
	for ( ; itor!=m_mSoundBuffer.end(); itor++ )
	{
		if( itor->second->m_bUI )
            itor->second->m_lpBuffer->SetVolume( DSVOLUME(m_lUISoundVolume) );
		else
            itor->second->m_lpBuffer->SetVolume( DSVOLUME(m_lSoundVolume) );
	}

	unguard;	
}

/** \brief
*  �Ӵ����ļ�������Ч
*
*  \param:  _szFileName    Ҫ���ŵ���Ч�ļ�����
*           _bufferPara    3D buffer���� ���������ĳ����Դ������
*                          ���ڶ�ά����,�˲�������Ϊ�Σգ̣�
*           _bLoop         �ǹ�ѭ������  0Ϊ��ѭ��,1Ϊѭ��
*
*  \return  DWORD          ����һ����Դ��ΨһID       
*/
DWORD   gc_r4SoundMng::PlayFromFile(char *_szFileName, const os_initSoundSrc* _bufferPara,bool _bLoop/* = false*/, 
									bool _bUI/* = false */, float* _fSoundPlayTime /*= NULL*/ )
{
	if( NULL == m_lpDirectSound  )
		return -1;

	gcs_sound_id   t_sId = playSound(
		const_cast<char*>(_szFileName),_bufferPara,_bLoop,_bUI,_fSoundPlayTime); 

	return (unsigned long)t_sId.value;

}

//! �Ƿ���ͬ�Ĳ����������ͬ���ڲ����账��
bool os_lisenerParam::operator==( os_lisenerParam& _s )
{
	guard;

	if( !float_equal( _s.m_fDisFactor,this->m_fDisFactor ) )
		return false;
	if( !float_equal( _s.m_fRolloff,this->m_fRolloff ) )
		return false;
	if( !vec3_equal( _s.m_vOrientFront,this->m_vOrientFront ) )
		return false;
	if( !vec3_equal( _s.m_vOrientTop,this->m_vOrientTop ) )
		return false;
	if( !vec3_equal( _s.m_vPosition,this->m_vPosition ) )
		return false;

	return true;

	unguard;
}


/** \brief
*  ���������ߵ�λ��
*
*  \param   _sListenerParams   ���������ߵĲ���
*
*  \return  bool               �Ƿ����óɹ�
*/
bool gc_r4SoundMng::SetListenerPara(os_lisenerParam &_sListenerParams )
{
	guard;

	// ��Ƶ��ʼ��ʧ��,������Ч����ԴID
	if ( m_lpDirectSound == NULL )
		return true;

# if 0
	//�������ͬ�����ء������������
	if( _sListenerParams == m_sCatchLp )
		return true;
	else
		m_sCatchLp = _sListenerParams;
# endif 		

	LPDIRECTSOUNDBUFFER t_pDSBPrimary = NULL;
	if(&m_p3DListener==NULL)
	{
		osDebugOut("%s","SOUND: ��Ч��Listenerָ���ַ!");
	}
	// ΪListern������ֵ
	m_sListenParams.vPosition			 = _sListenerParams.m_vPosition;
	m_sListenParams.vOrientFront		 = _sListenerParams.m_vOrientFront;
	m_sListenParams.vOrientTop			 = _sListenerParams.m_vOrientTop;
	m_sListenParams.flDistanceFactor	 = _sListenerParams.m_fDisFactor;
	m_sListenParams.flRolloffFactor		 = _sListenerParams.m_fRolloff;
	m_sListenParams.dwSize               = sizeof(DS3DLISTENER);

	// �淶���������ʵ�λ��
	if ( m_sListenParams.flRolloffFactor <= DS3D_MINROLLOFFFACTOR )
	{
		m_sListenParams.flRolloffFactor = DS3D_MINROLLOFFFACTOR;
	}
	else if ( m_sListenParams.flRolloffFactor >= DS3D_MAXROLLOFFFACTOR )
	{
		m_sListenParams.flRolloffFactor = DS3D_MAXROLLOFFFACTOR;
	}


	// ����Listenrer��3D����
	if(m_p3DListener)
	{
		if(FAILED(m_p3DListener->SetAllParameters(&m_sListenParams,DS3D_IMMEDIATE)))
		{
			osDebugOut("%s","SOUND:  ����Listener����ʱ����!");
			// ���Listener��3D��Ϣ
			osDebugOut("SOUND: Listenerλ��( %f, %f, %f)", 
				_sListenerParams.m_vPosition.x, _sListenerParams.m_vPosition.y, _sListenerParams.m_vPosition.z);
			osDebugOut("              ��������( %f  )", 	m_sListenParams.flDistanceFactor);
			osDebugOut("              Listen��С( %d )",  m_sListenParams.dwSize);
		}
	}
	return true;

	unguard;
}


gc_r4SoundMng* gc_r4SoundMng::Instance()
{
	guard;

	if( !gc_r4SoundMng::m_ptrSingleton )
	{

		gc_r4SoundMng::m_ptrSingleton = new gc_r4SoundMng;

		osassert( gc_r4SoundMng::m_ptrSingleton );
	}

	return gc_r4SoundMng::m_ptrSingleton;

	unguard;
}

void gc_r4SoundMng::DInstance()
{
	guard;// sg_timer::DInstance() );

	SAFE_DELETE( gc_r4SoundMng::m_ptrSingleton );

	unguard;
}

/** \brief
*  �ͷ����������������е�������Դ.
*  
*/
bool gc_r4SoundMng::destory_allsoundResource( void )
{
	guard;

	destory_allsoundfiles();
	return true;
	unguard;
}




//!��ȡȫ�ֵ�����������ָ��
SMMANAGER_API I_soundManager*   get_soundManagerPtr( void )
{
	return (I_soundManager*)gc_r4SoundMng::Instance();
}
//! �ͷ�ȫ�ֵ�����������ָ��,�˺���������Զ���ͷŲ���������,��release_renderRes�����л����
SMMANAGER_API void             release_soundManager( void )
{
	return gc_r4SoundMng::DInstance();
}
