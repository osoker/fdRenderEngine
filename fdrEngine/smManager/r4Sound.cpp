
# include "r4Sound.h"
# include "gc_SoundFilePool.h"
# include <vfw.h>
# include "fyMp3_lib.h"



#ifndef DSVOLUME
#	define DSVOLUME(volume)  ((volume==0)?-10000:(-60*(MAX_VOLUME-volume)))
#endif

# pragma comment( lib,"dxguid" )

//! 引擎中可以听到的最完声音矩离
# define MAX_SOUNDDIS    128.0f

//! 全局的声音开关，可以打开和关闭声音的播放，默认打开。
SMMANAGER_API BOOL      g_bUseSound = TRUE;

//! 引擎使用这个变量来确认声音管理器是否初始化，如果没有初始化，则此时不处理声音相关的数据
SMMANAGER_API BOOL      g_bUseSoundMgrHaveInit = FALSE;


void gcs_sound_buffer::stop_sound( void )
{
	guard;

	if( m_lpBuffer )
	{
		m_lpBuffer->Stop();
		m_lpBuffer->SetCurrentPosition( 0 );
	}

	//! 设置删除标志
	start_vanish();

	unguard;
}



//template<> gc_r4SoundMng* gc_r4Singleton<gc_r4SoundMng>::ms_Singleton = 0;
//! 初始化静态变量
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
	//! 建立声音文件池，初始化声音路径
	SAFE_RELEASE(m_lpDirectSound);
	//	return false;
	// Create IDirectSound using the primary sound device
	//!default sound device,third should be null,use the default device
	if( FAILED( DirectSoundCreate8( NULL, &m_lpDirectSound, NULL ) ) )
	{
		//MessageBox( NULL, "音频初始化失败!", "错误", MB_OK );
		osDebugOut( "音频初始化失败!\n" );
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
	//! 创建主缓存
	if(!initPrimaryBufferFormat(_dwPrimaryFreq,_dwPrimaryBitRate))
		return false;
	// 
	//! 初始化三维的听觉位置
	init3DListener(_sListener);

	SetSoundVolume( SOUNDTYPE_3D, MAX_VOLUME/2 );
	SetSoundVolume( SOUNDTYPE_UI, MAX_VOLUME/2 );

	// 声音管理器已经初始化
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
 *  对一个已经播放过的声音缓冲区重新设置，使用缓冲区内存在的声音资源，重新播放
 *
 *  \param _pbuffer  要重设的声音缓冲区。
 *  \param _fPlayTime 暂不使用此参数，用于控制声音播放的时间长短。
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
 *  根据声音文件的名字创建一个声音，并且为他分配缓存
 * 
 *  \param const char* _lpszFilename 要创建的声音的文件名
 *  \param float _fPlayTime          声音要播放的时间，目前暂不使用此参数。
 *  \param bool  _bForce             是否创建到dSound的硬件混音缓冲区中,优化时使用
 */
gcs_sound_buffer* gc_r4SoundMng::createSoundBuffer( const char* _lpszFilename,
	          float* _fPlayTime,bool _bForce/* = false*/,bool _bUI/* = false*/ )
{
	guard;

	// 如果声音设备为空,则返回.
	if( NULL == m_lpDirectSound )
		return NULL;

	long	lVolume = 0;
	gcs_sound_buffer* _pbuff = NULL;
	DSBUFFERDESC      t_sDesc;
	LPDIRECTSOUNDBUFFER t_soundBuffer;
	LPDIRECTSOUNDBUFFER8 t_ppDsb8 = NULL;
	HRESULT t_hr;
	HRESULT hrRet = S_OK;

	// 从声音内存管理器中得到当前的声音内存
	gc_SoundFile* _pSoundFile	=	gc_SoundFilePool::getSingletonPtr()->get_resource(_lpszFilename);
	if(NULL==_pSoundFile)	
	{
		osassertex( false,"创建声音文件失败...\n" );
		return NULL;
	}

	_pSoundFile->getSoundDes( t_sDesc );
	if( _fPlayTime )
        *_fPlayTime = _pSoundFile->getSoundPlayTime();

	// 如果非ui播放,则不使用相关的三维标志
	if( _bUI )
	{
		t_sDesc.dwFlags = DSBCAPS_CTRLVOLUME;
		t_sDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
		lVolume = m_lUISoundVolume;
	}
	else
		lVolume = m_lSoundVolume;

	// 头一次创建
	if( NULL == _pSoundFile->m_sSoundBuffer )
	{
		t_hr = m_lpDirectSound->CreateSoundBuffer( &t_sDesc, &t_soundBuffer, NULL );

		// River mod @ 2007-8-26:把声音的错误屏掉，不开放声音。
		if( FAILED(t_hr) )
		{
			MessageBox( NULL,"请检查您的声卡驱动程序，创建声音缓冲区失败!","ERROR",MB_OK );
			osDebugOut( "出错原因:<%s>...\n",DXGetErrorDescription9( t_hr ) );
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
			osDebugOut( "SOUND: %s 创建缓冲区时出错!\n", _lpszFilename );

			osDebugOut( "nAvgBytesPerSec(%d), nBlockAlign(%d),nChannels(%d)",
				t_sDesc.lpwfxFormat->nAvgBytesPerSec, 
				t_sDesc.lpwfxFormat->nBlockAlign,t_sDesc.lpwfxFormat->nChannels );
			osDebugOut( "nSamplesPerSec(%d), wBitsPerSample(%d),dwBufferBytes(%d)\n",
				t_sDesc.lpwfxFormat->nSamplesPerSec, 
				t_sDesc.lpwfxFormat->wBitsPerSample,
				t_sDesc.dwBufferBytes );

			return NULL;
		}

		// 得到新接口，释放soundBuffer接口	
		t_hr = t_soundBuffer->QueryInterface(IID_IDirectSoundBuffer8, 
			(LPVOID*) &_pSoundFile->m_sSoundBuffer );

		_pSoundFile->getSoundDes( t_sDesc );
		if( FAILED(fillBufferWithSound(
			(BYTE*)_pSoundFile->getDataBuff(),
			t_sDesc.dwBufferBytes,_pSoundFile->m_sSoundBuffer,true)) )
		{
			//osassertex( false,"往声音缓冲区中填充数据失败...\n" );
			return NULL;
		} 

		// 
		// 设置缓冲区的音量,每创建完一个声音缓冲区，需要设置此缓冲区的音量.
		if( FAILED( t_hr = _pSoundFile->m_sSoundBuffer->SetVolume( DSVOLUME(lVolume)) ) )
		{
			//osassertex( false,(char*)DXGetErrorDescription9( t_hr ) );
			return NULL;
		}

		SAFE_RELEASE( t_soundBuffer );

		// 删除对每一个声音保存的内存，只保留声单缓冲区就ok
		_pSoundFile->destory();
	}


	_pbuff = new gcs_sound_buffer;
	

	//！ DuplicateSoundBuffer出错的情况下，重回此执行：
RESTORE_SOUNDBUFF_PROCESS:

	t_hr = m_lpDirectSound->DuplicateSoundBuffer( 
		_pSoundFile->m_sSoundBuffer,&t_soundBuffer );

	//! River @ 2010-10-8:加入更多的错误处理
	if( !FAILED( t_hr ) )
	{
		t_hr = t_soundBuffer->QueryInterface(IID_IDirectSoundBuffer8, 
			(LPVOID*) &t_ppDsb8 );
		osassertex( (!FAILED(t_hr)),(char*)DXGetErrorDescription9( t_hr ) );
	}
	else
	{
		// River @ 2011-2-14:加入对Restore声音缓冲区的处理
		if( t_hr == DSERR_BUFFERLOST )
		{
			_pSoundFile->getSoundDes( t_sDesc );
			_pSoundFile->load( NULL );
			if( FAILED(fillBufferWithSound(
				(BYTE*)_pSoundFile->getDataBuff(),
				t_sDesc.dwBufferBytes,_pSoundFile->m_sSoundBuffer,true)) )
			{
				//osassertex( false,"恢复往声音缓冲区中填充数据失败...\n" );
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

	//! River added @ 2009-5-11:catch 声音播放的时间
	if( _fPlayTime )
		_pbuff->m_fPlayTime = *_fPlayTime;

	return _pbuff;

	unguard;
}



/** \brief
*  从声音系统的缓存中播放当前的声音文件
*
*  这个系统会缓冲存储一些声音，每次播放声音的时候，先从系统缓存中查找，
*  如果要播放的声音，存在于系统缓存中，则使用缓存中的声音。
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

		//  有相同的hashId并且缓存缓存区非播放状态
		if( (itor->first.hash_value == _lhashvalue)
			 && (_pbuff->isVanish()) )
		{
			_pbuff	=	resetSoundBuffer( _pbuff );

			osassertex( _pbuff->m_lpBuffer,"重设的缓冲区不能为空...\n" );

			// 声音的类型不一致.
			if( _pbuff->m_bUI != _bUI )
				continue;

			m_mSoundBuffer.erase(itor);
			gcs_sound_id t_id(_lhashvalue,++m_lCurMaxSoundIndex);
			m_mSoundBuffer.insert( std::pair<gcs_sound_id,gcs_sound_buffer*>(t_id,_pbuff) );
	
			// 处理返回值
			_pid->hash_value	=	_lhashvalue;
			_pid->value			=	m_lCurMaxSoundIndex;
			
			LPDIRECTSOUNDBUFFER8 t_pDSB = _pbuff->m_lpBuffer;
			HRESULT             hr;
			BOOL               t_bRestored;

			// 确认buf状态
			if( FAILED( restoreBuffer(t_pDSB, &t_bRestored)) )	
				return false;

			if(_pbuff->m_bUI)
			{
				// 非3D音效的播放方式
				// 第二个参数必须是0
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
					// 某些参数出错
					osassert(false);
					delete_sound( t_id );
					return false;
				}

				// 开始播放音效
				t_pDSB->SetVolume( DSVOLUME(m_lSoundVolume) );
				if(FAILED((t_pDSB)->Play(0,0xFFFFFFFF,_bLoop?DSBPLAY_LOOPING:0)) )
				{
					delete_sound( t_id );
					return false;
				}

				t_pDS3DBuffer->Release();
			}

			//! River added @ 2009-5-11:返回声音播放的时间。
			if( _playTime )
				*_playTime = _pbuff->m_fPlayTime;

			return true;
		}
	}

	return false;

	unguard;
}


//! 从当前的声音缓冲区集合中删除一个声音
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


//! 设置soundBuffer的参数
bool gc_r4SoundMng::set_bufferPara( const os_initSoundSrc* _bufferPara,LPDIRECTSOUND3DBUFFER _buf )
{
	guard;

	DS3DBUFFER t_srcBuf;
	HRESULT    t_hr;

	t_srcBuf.dwSize = sizeof(DS3DBUFFER);

	// 获得3D Buffer的默认参数
	if( FAILED( t_hr = _buf->GetAllParameters(&t_srcBuf) ) )
	{
		osassertex( false,(char*)DXGetErrorDescription9( t_hr )  );			
		return false;
	}

	// 初始化3D Buffer的3D参数
	t_srcBuf.flMinDistance			   = _bufferPara->m_flMinDistance;
	t_srcBuf.flMaxDistance			   = _bufferPara->m_flMaxDistance;
	t_srcBuf.vPosition                = _bufferPara->m_vPosition;		
	t_srcBuf.lConeOutsideVolume        = DSVOLUME(m_lSoundVolume);			

	// 设置3D Buffer的参数
	if( FAILED( t_hr = _buf->SetAllParameters(&t_srcBuf, DS3D_IMMEDIATE)) )
	{
		osassertex( false,(char*)DXGetErrorDescription9( t_hr )  );			
		return false;
	}

	return true;

	unguard;
}


/** \brief
 *  真正播放声音的函数，返回了当前播放的hash ID.
 *
 *  算法描述：
 *  1:
 *  2: 
 */
gcs_sound_id gc_r4SoundMng::playSound( const char* _lpszFileName,
				const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI, float* _fSoundPlayTime/*=NULL*/ )
{
	guard;


	// _bUI为false时,_bufferPara不能为NULL.
	osassertex( ((_bufferPara)||(_bUI)),"buffer为空时,不能播放三维声音..\n" );
	if( _bufferPara )
		osassert( _bufferPara->m_flMinDistance >= 0.0 );

	if( !g_bUseSound )
		return gcs_sound_id(-1);

	//! 首先看一下当前缓存里有没有相同文件名的待删除的声音，如果有的话直接播放他就可以了
	gcs_sound_id   t_idx;
	if( playSoundFromPool( &t_idx,
		_fSoundPlayTime,_lpszFileName,_bufferPara,_bLoop,_bUI ) )
		return t_idx;


	// 
	// 从磁盘上播放,最近距离必须大于0
	gcs_sound_id   t_iResId( string_hash(_lpszFileName),++m_lCurMaxSoundIndex );
	gcs_sound_buffer*	_pbuff;
	
	_pbuff = createSoundBuffer( _lpszFileName,_fSoundPlayTime,m_bForcePlay,_bUI );
	if( !_pbuff)
	{
		osDebugOut("同时播放声音数大于<%d>,暂无缓冲区可用...\n",m_deviceCap.dwFreeHwMixingAllBuffers);
		return gcs_sound_id(-1);
	}


	_pbuff->m_bUI	=	_bUI;

	m_mSoundBuffer.insert(
		std::pair<gcs_sound_id,gcs_sound_buffer*>( t_iResId,_pbuff ) );

	LPDIRECTSOUNDBUFFER8 t_pDSB = _pbuff->m_lpBuffer;
	HRESULT  hr;
	BOOL    t_bRestored;

	// 确认声音缓冲区的状态
	if( FAILED(restoreBuffer(t_pDSB, &t_bRestored)) )	
		return gcs_sound_id(-1);

	// 
	// 根据是二维声音还是三维声音，使用不同的播放程序分支
	if( _pbuff->m_bUI )
	{
		// 非3D音效的播放方式,第二个参数必须是0, 如果播放失败，删除当前的音效
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

			// 开始播放音效
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
	//! stop的时候把这个buff删除
	//! 要不要保持一段时间在删除呢？
	// 音频初始化失败,传入无效的声源ID
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
*  设置发声对象的属性 每一帧做一次处理
*
*  \param   _dwID              发生对象与Secondory buffer的唯一对应标识  
*  
*           _vecSoundPos       发声对象的位置 
*         
*  \return  bool               处于播放状态true;否则返回false
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
		//! 设置这个声音的位置信息
		gcs_sound_buffer*	_pbuff	=	itor->second;
		if(_pbuff->m_bUI)		return true;
		// 判断当前音源是否正在播放
		if(_pbuff->isVanish()) return false;

		_pbuff->m_lpBuffer->GetStatus(&t_dwStatus);
		if((t_dwStatus&DSBSTATUS_PLAYING)==0)
		{
			_pbuff->m_lpBuffer->Stop();
			return false;
		}
		// 获得声道的格式
		WAVEFORMATEX bufFormat;
		_pbuff->m_lpBuffer->GetFormat( &bufFormat, sizeof(WAVEFORMATEX), NULL );
		if (bufFormat.nChannels>1)
			return true;

		// 获得3D控制接口
		_pbuff->m_lpBuffer->QueryInterface(IID_IDirectSound3DBuffer,(VOID**)&t_pDS3DBuffer );
		if (!t_pDS3DBuffer)
		{
			osassertex( false, "获得3D缓存区失败" );
			return true;
		}

		//! 距离倾听者太远的声音也要删除,以下公式相当于osVec3Length
		float _fDis	=	(float)sqrt( (m_sListenParams.vPosition.x-_vecSoundPos.x)*
			(m_sListenParams.vPosition.x-_vecSoundPos.x )+
			(m_sListenParams.vPosition.y-_vecSoundPos.y)*(m_sListenParams.vPosition.y-_vecSoundPos.y)+
			(m_sListenParams.vPosition.z-_vecSoundPos.z)*(m_sListenParams.vPosition.z-_vecSoundPos.z));

		
		// RIVER MOD 2008-3-21:TEST CODE
		// 如果声音的矩离太远，停止当前的声音。
		if( _fDis>MAX_SOUNDDIS )
		{
			_pbuff->stop_sound();
			return true;
		}
		/**/

		DS3DBUFFER              t_s3DBufPara;
		ZeroMemory(&t_s3DBufPara,sizeof(DS3DBUFFER));
		t_s3DBufPara.dwSize = sizeof(DS3DBUFFER);
		// 获得声音缓冲区的默认参数
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
*   背景音乐帧播放，对声音系统内的每一个声音处理，缓存或是删除相应的声音。
*/
void		 gc_r4SoundMng::FrameMove()
{
	guard;

	// 处理当前缓存内的每一个声音Buffer.
	std::map<gcs_sound_id,gcs_sound_buffer*>::iterator	
		                       itor	=	m_mSoundBuffer.begin();

	for(;itor!=m_mSoundBuffer.end();itor++)
	{
		// 处理正处于缓存期间的声音
		if( (itor->second->isVanish() )  )
		{
			if( (itor->second->m_iCount>0 ) )
			{
				itor->second->m_iCount--;

				// 
				// 把到期的缓存声音删除
				if( 0 == itor->second->m_iCount )
				{
					SAFE_DELETE( itor->second );
					m_mSoundBuffer.erase(itor);
					itor	=	m_mSoundBuffer.begin();
				
					// river mod remote:
					// 如果此时没有声音可处理，则直接退出
					if( 0 == m_mSoundBuffer.size() )
						break;
					else
						continue;				
				}

			}
		}
		else // 处理正在播放的声音。
		{
			DWORD dwStatus;
			itor->second->m_lpBuffer->GetStatus(&dwStatus);

			// 已经播放完，开始缓存此声音
			if( (dwStatus&DSBSTATUS_PLAYING) == 0 )
				itor->second->start_vanish();
		}
	}


	// 对mp3播放move,如果是循环播放，则需要在函数内处理。
	osassert( m_sMp3Player );
	m_sMp3Player->FrameMove();


	return;
	unguard;
}


//**********************************************************************************
//***************************MP3背景音乐的播放**************************************
//! Attention:  对于mp3的播放无需对管理器进行初始化,用完后也无需Release
//**********************************************************************************
//{@!以下为mp3背景音乐的播放,其实其他一些格式,譬如wav等也可以播放，主要是因为使
//    用了DShou的相应的接口,但效率肯定没有我们自己管理的Dsound效率高.
/** \brief
*  从文件播放mp3背景音乐
*
*  \return   bool              返回值表明是否播放成功
*/
bool gc_r4SoundMng::PlayFileMP3( const char *_FileName,BOOL _repeat/* = TRUE*/ )
{
	if( !g_bUseSoundMgrHaveInit )
		return false;

	m_CurMp3FileName = _FileName;
	return m_sMp3Player->PlayFileMP3( _FileName,_repeat );
}

/** \brief
*  停止播放mp3
*
*  这个函数几可以控制内存播放又可以控制文件播放
*
*  \return    bool   停止时是否成功
*/
bool gc_r4SoundMng::StopMP3()
{ 
	if( m_sMp3Player )
		return m_sMp3Player->StopMP3() ; 

	return true;
}

//@{ River @ 2010-8-20:加入得到mp3播放位置和设置mp3播放位置的接口。
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
*  暂停播放
*
*  这个函数几可以控制内存播放又可以控制文件播放
*
*  \return    bool   暂停播放
*/
bool gc_r4SoundMng::PauseMP3( bool _pause/* = true*/ )
{ 
	if( m_sMp3Player ) 
		return m_sMp3Player->PauseMP3( _pause ); 
	else
		return true;

}

/** \brief
*  静音开关,使用一次静音,再调用一个就可恢复正常
*/
void gc_r4SoundMng::ToggleMuteMP3( bool _mute/* = true*/ )
{ 
	if( m_sMp3Player )
		m_sMp3Player->ToggleMuteMP3( _mute ); 
}


/** \brief
*  设置音量
*
*  \_lVolume 新的音量值(0----100)
*/
bool gc_r4SoundMng::SetVolumeMP3( long _lVolume )
{ 
	if( m_sMp3Player )
		return m_sMp3Player->SetVolume( _lVolume);
	else
		return true;
}

/** \brief
*  获得背景音乐的音量
*
*  \param    _Volume 新的音量值
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
*	重新分配Buffer
*
*  如果缓冲区lost,需要重新分配
*
*  \param   _pDSB           次缓冲区
*  \param   BOOL _pbWasRestored  是否重新分配了buffer
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

			// River @ 2011-2-14:加入判断，不至于死循环
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
*  设置主缓冲区的格式
*
*  主缓冲区相当于一个音效混合器,游戏中所有的音效都在这里按着一定的方式进行混合
*  然后输出,整个游戏中只能有一个这样的主缓冲区
*
*  \param   _dwPrimaryFreq     播放音乐的采用频率
*           _dwPrimaryBitRate  采样位的bit深度
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
	//!the third parameter should be null because of 聚集 is not suported!
	if( FAILED(m_lpDirectSound->CreateSoundBuffer( &t_dsbd, &t_pDSBPrimary, NULL ) ) )
	{
		osDebugOut("%s","SOUND: 创建主缓冲区失败!");
		return false;
	}
	WAVEFORMATEX t_wfx;
	ZeroMemory( &t_wfx, sizeof(WAVEFORMATEX) ); 
	//!在我们的游戏中必须是WAVE_FORMAT_PCM
	t_wfx.wFormatTag        = (WORD) WAVE_FORMAT_PCM;
	// 
	// 3D 控制时声道数必须为1
	// River @ 2005-7-29:3d控制要求声音文件的声道数目为1，但对主缓冲的设置没有要求. 
	// 
	t_wfx.nChannels       = 2;                                                      
	t_wfx.nSamplesPerSec  = (DWORD)_dwPrimaryFreq; 
	t_wfx.wBitsPerSample  = (WORD) _dwPrimaryBitRate; 
	t_wfx.nBlockAlign     = (WORD) (t_wfx.wBitsPerSample / 8 * t_wfx.nChannels);
	t_wfx.nAvgBytesPerSec = (DWORD)(t_wfx.nSamplesPerSec * t_wfx.nBlockAlign);
	// 设置音效混合的主缓冲区
	if( FAILED( t_pDSBPrimary->SetFormat(&t_wfx) ) )
	{ 
		osDebugOut("%s","SOUND: 设置主缓冲区失败!");
		return false;
	}
	SAFE_RELEASE(t_pDSBPrimary );
	return true;
	unguard;
}

/** \brief
*  设置倾听者,主要用于声音管理器的初始化
*
*  整个游戏中只有一个倾听者,也就是玩家角色
*
*  \param   _fDopplerFactor  多普勒频移因子,用于模拟速度比较快的物体发声效果
*           _fRolloffFactor  声音在空气中的衰减因子,0到10,0为不衰减,这个参数可以用来控制音量
*                            相对于MinDis和MaxDis,它的效果是全局的,影响每一个buffer
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
		// 获得Listener接口
		if( FAILED( t_pDSBPrimary->QueryInterface( IID_IDirectSound3DListener, 
			(VOID**)&m_p3DListener ) ) )
		{
			osDebugOut("%s","SOUND: 获取DirectSound3DListener错误!");
			SAFE_RELEASE(t_pDSBPrimary);
			//osassert("SOUND: 获取DirectSound3DListener错误!"&&false);
			return;
		}
		// Release the primary buffer, since it is not need anymore
		SAFE_RELEASE( t_pDSBPrimary );
		// 获得当前的倾听者参数  
		m_sListenParams.dwSize=sizeof(DS3DLISTENER);
		m_p3DListener->GetAllParameters(&m_sListenParams);   

	}
	// 设置主角的位置
	if(FAILED(m_p3DListener->SetPosition(_sListener.m_vPosition.x,
		_sListener.m_vPosition.y,_sListener.m_vPosition.z,DS3D_IMMEDIATE)))
	{
		osDebugOut("%s","SOUND:  设置Listener参数时出错!");
		//osassert("SOUND:  设置Listener参数时出错!"&&false);
		return;
	}
	// 设置主角的Oriention
	if(FAILED(m_p3DListener->SetOrientation(
		_sListener.m_vOrientFront.x,_sListener.m_vOrientFront.y
		,_sListener.m_vOrientFront.z ,_sListener.m_vOrientTop.x ,
		_sListener.m_vOrientTop.y ,_sListener.m_vOrientTop.z ,DS3D_IMMEDIATE)))
	{
		osDebugOut("%s","SOUND:  设置Listener参数时出错!");
		//osassert("SOUND:  设置Listener参数时出错!"&&false);
		return;
	}
	// 设置距离因子
	m_p3DListener->SetDistanceFactor(1.0f, DS3D_IMMEDIATE);

	memcpy( &m_sCatchLp,&_sListener,sizeof( os_lisenerParam ) );

	unguard;
}


/** \brief
*  为指定的secondory buffer填充wave数据
*
*  \param   _szFileName     wave文件名
*           _pDSB           次缓冲区指针
*           _bRepeatIfLarger如果次缓冲区太大是否填充重复数据
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
	// 清空声源缓从区 
	ZeroMemory(t_pDSLockedBuffer,t_dwDSLockedBufferSize);
	CopyMemory(t_pDSLockedBuffer,_pbWave,_dataSize); 
	_pDSB->Unlock(t_pDSLockedBuffer,t_dwDSLockedBufferSize,NULL,0);//!same to lock  

	return S_OK;
}
/** \brief
*  释放声音管理器所用到的资源
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
*   判断当前音源是否正在播放
*
*    \param _dwID              声源ID
*
*    \return bool                   结束播放返回true
*/
bool gc_r4SoundMng::IsPlayOver(DWORD _dwID)
{
	guard;
	if (m_lpDirectSound == NULL)
	{
		osDebugOut("SOUND: DirectSound 初始化失败");  
		return true;
	}

	// 当前音源是否正在播放
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
*   设置音效的音量
*
*    \param		_lVolume     声音音效的音量(0---100)
*/
void  gc_r4SoundMng::SetSoundVolume(  OS_SOUNDTYPE _type, long _lVolume/* = 75*/  )
{
	guard;

	if( !m_lpDirectSound )
		return;

	// 把传入的音量转换到DSound 要求的范围
	if( _type == SOUNDTYPE_3D )
		m_lSoundVolume   = max( MIN_VOLUME, min(_lVolume,MAX_VOLUME) );
	else
		m_lUISoundVolume = max( MIN_VOLUME, min(_lVolume,MAX_VOLUME) );
	
	// 改变每个缓冲的音量
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
*  从磁盘文件播放音效
*
*  \param:  _szFileName    要播放的生效文件名称
*           _bufferPara    3D buffer参数 这个参数是某个声源的属性
*                          对于二维声音,此参数可以为ＮＵＬＬ
*           _bLoop         是构循环播放  0为不循环,1为循环
*
*  \return  DWORD          返回一个声源的唯一ID       
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

//! 是否相同的参数，如果相同，内部不需处理
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
*  设置倾听者的位置
*
*  \param   _sListenerParams   设置倾听者的参数
*
*  \return  bool               是否设置成功
*/
bool gc_r4SoundMng::SetListenerPara(os_lisenerParam &_sListenerParams )
{
	guard;

	// 音频初始化失败,返回无效的声源ID
	if ( m_lpDirectSound == NULL )
		return true;

# if 0
	//！如果相同，返回。否则继续设置
	if( _sListenerParams == m_sCatchLp )
		return true;
	else
		m_sCatchLp = _sListenerParams;
# endif 		

	LPDIRECTSOUNDBUFFER t_pDSBPrimary = NULL;
	if(&m_p3DListener==NULL)
	{
		osDebugOut("%s","SOUND: 无效的Listener指针地址!");
	}
	// 为Listern参数赋值
	m_sListenParams.vPosition			 = _sListenerParams.m_vPosition;
	m_sListenParams.vOrientFront		 = _sListenerParams.m_vOrientFront;
	m_sListenParams.vOrientTop			 = _sListenerParams.m_vOrientTop;
	m_sListenParams.flDistanceFactor	 = _sListenerParams.m_fDisFactor;
	m_sListenParams.flRolloffFactor		 = _sListenerParams.m_fRolloff;
	m_sListenParams.dwSize               = sizeof(DS3DLISTENER);

	// 规范参数到合适的位置
	if ( m_sListenParams.flRolloffFactor <= DS3D_MINROLLOFFFACTOR )
	{
		m_sListenParams.flRolloffFactor = DS3D_MINROLLOFFFACTOR;
	}
	else if ( m_sListenParams.flRolloffFactor >= DS3D_MAXROLLOFFFACTOR )
	{
		m_sListenParams.flRolloffFactor = DS3D_MAXROLLOFFFACTOR;
	}


	// 设置Listenrer的3D参数
	if(m_p3DListener)
	{
		if(FAILED(m_p3DListener->SetAllParameters(&m_sListenParams,DS3D_IMMEDIATE)))
		{
			osDebugOut("%s","SOUND:  设置Listener参数时出错!");
			// 输出Listener的3D信息
			osDebugOut("SOUND: Listener位置( %f, %f, %f)", 
				_sListenerParams.m_vPosition.x, _sListenerParams.m_vPosition.y, _sListenerParams.m_vPosition.z);
			osDebugOut("              距离因子( %f  )", 	m_sListenParams.flDistanceFactor);
			osDebugOut("              Listen大小( %d )",  m_sListenParams.dwSize);
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
*  释放声音管理器中所有的声音资源.
*  
*/
bool gc_r4SoundMng::destory_allsoundResource( void )
{
	guard;

	destory_allsoundfiles();
	return true;
	unguard;
}




//!获取全局的声音管理器指针
SMMANAGER_API I_soundManager*   get_soundManagerPtr( void )
{
	return (I_soundManager*)gc_r4SoundMng::Instance();
}
//! 释放全局的声音管理器指针,此函数必须可以多次释放不出现问题,在release_renderRes函数中会调用
SMMANAGER_API void             release_soundManager( void )
{
	return gc_r4SoundMng::DInstance();
}
