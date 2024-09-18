#ifndef _R4SOUND_H_
#define _R4SOUND_H_


#include "../interface/osInterface.h"
#include "gc_r4Singleton.h"
#include <map>

// 上层可调节的音量范围
#define MIN_VOLUME 0
#define MAX_VOLUME 100

/** \brief
 *  声音ID的结构						
 * 
 * \param  hash_value：声音文件名字的hash值		
 * \param  index：这个声音的播放索引，就算声音的hashValue相同，此值也不相同，
 *                此值返回给上层，用于操作声音。
 */
union gcs_sound_id
{
	gcs_sound_id()
	{
		value = 0;
	}
	gcs_sound_id(unsigned long _value)
	{
		value = _value;
	}
	gcs_sound_id(unsigned int _hashvalue,unsigned int _index)
	{
		hash_value = _hashvalue;
		index = _index;
	}
	gcs_sound_id(const gcs_sound_id& _data)
	{
		hash_value = _data.hash_value;
		index = _data.index;
	}

	bool operator == (const gcs_sound_id& _l)	{return	value	==	_l.value;}
	bool operator != (const gcs_sound_id& _l)	{return	value	!=	_l.value;}
	bool operator < (const gcs_sound_id& _l)	{return	value	<	_l.value;}
	gcs_sound_id&	operator = (const gcs_sound_id& _l)	
	{
		hash_value = _l.hash_value;
		index = _l.index;

		return *this;
	}

	struct
	{
		unsigned int	index;
		unsigned int	hash_value;
	};

	unsigned long value;
};
inline bool	operator < (const gcs_sound_id& _l,const gcs_sound_id& _r)	{return _l.value<_r.value;}


class fyc_MP3Lib;


//! 声音缓存的帧数,用帧数可能不太准确，但迅速
# define  MAX_DELAYCOUNT  120 

//! 声音的最基本resource数据结构
struct gcs_sound_buffer
{
	//! 
	gcs_sound_buffer()
	{
		m_iCount = -1;
		m_lpBuffer = NULL;
		m_fPlayTime = 0.0f;
	}

	~gcs_sound_buffer()
	{
		SAFE_RELEASE( m_lpBuffer );
	}

	//! 停止当前的声音播放。
	void    stop_sound( void );

	//! 当前的声音是否已经停止播放,如果count不等于-1,则已经开始停止播放此声音。
	bool	isVanish()	const { return  ( m_iCount != -1 ); }
	//! 声音播放结束，缓存计数开始.
	void    start_vanish( void )    { m_iCount = MAX_DELAYCOUNT; } 


	//! 是否是UI声音 
	bool					m_bUI;
	
	//! 延时删除的记时器。m_iCound == -1,表示此结构正在播放中,０则是从缓存数减下来的，可以删除
	int						m_iCount;

	//! Buffer指针
	LPDIRECTSOUNDBUFFER8	m_lpBuffer;

	//! River @ 2009-5-11: 每一个soundBuffer需要的播放时间
	float                   m_fPlayTime;
};


/** \brief
 *  实现对外声音接口集的类.
 *  
 */
class gc_r4SoundMng:public I_soundManager,public ost_Singleton<gc_r4SoundMng>
{
public:
	
	gc_r4SoundMng();
	virtual ~gc_r4SoundMng();
	//! 如果_soundID的index = -1 则表示还没有这个声音的缓存，函数一定会返回null，要求建立他
	void	initSoundResourcePool(const char* _lpszRootpath);
	/** 得到一个sg_timer的Instance指针.
	 */
	static gc_r4SoundMng*    Instance( void );
	/** 删除一个sg_timer的Instance指针.
	 */
	static void             DInstance( void );



	/** \brief
	 *  初始化声音管理器DircetSound
	 *
	 *  主缓冲区相当于一个音效混合器,游戏中所有的音效都在这里按着一定的方式进行混合
	 *  然后输出,整个游戏中只能有一个这样的主缓冲区
	 *
	 *  \param     _hWnd            窗口句柄
	 *  \param     _dwCoopLevel     硬件协作度 游戏中一般为DSSCL_PRIORITY
	 *  \param     _dwPrimaryFreq   播放音乐的采用频率,用于设置主缓冲区的格式,一般为22050或44100
	 *  \param     _dwPrimaryBitRate采样位的bit深度,用于设置主缓冲区的格式    一般为8或16
	 *  \param     _sListener       倾听者的参数
     *
	 *  \return    bool             成功返回true;否则返回false;
	*/
	virtual bool   InitSoundManager( HWND _hWnd,DWORD _dwPrimaryFreq,
		                DWORD _dwPrimaryBitRate,os_lisenerParam &_sListenerPara,
		                DWORD _dwCoopLevel=DSSCL_PRIORITY   );

	/** \brief
	 *  释放声音管理器中所有的声音资源.
	 *  
	 *  声音管理器内部有声音文件的内存Catch,调用此函数，使用所有的Catch失效。
	 */
	virtual bool   destory_allsoundResource( void );


	/** \brief
	 *  恢复系统音量
	 *  
	 */
	virtual void   ResetSoundVolume( void ) ;

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
	virtual DWORD   PlayFromFile( char *_szFileName, const os_initSoundSrc* _bufferPara,
		                             bool _bLoop = false, bool _bUI = false, float* _fSoundPlayTime = NULL ); 


	/** \brief
	 *  停止播放某个音效
	 *
	 *  \param   _dwID          可以作为所有发声对象的一个唯一标识
	*/
	virtual void    StopSound( DWORD _dwID );

	/** \brief
	 *  设置发声对象的属性 每一帧做一次处理
	 *
	 *  \param   _dwID              发生对象与Secondory buffer的唯一对应标识  
	 *  
	 *           _vecSoundPos       发声对象的位置 
	 *         
	 *  \return  bool               处于播放状态true;否则返回false
	*/
	virtual bool    FrameMoveSndPos(DWORD _dwID, osVec3D& _vecSoundPos,osVec3D* _velocity = NULL);

	/** \brief
	 *  设置倾听者的位置,需要时随时设置
	 *
	 *  \param   _sListenerParams   设置倾听者的参数
	 *
	 *  \return  bool               是否设置成功
	 */
	virtual bool    SetListenerPara(os_lisenerParam& _sListenerParams);

	/** \brief
	*   判断当前音源是否正在播放
	*
	*    \param _dwID              声源ID
	*
	*    \return bool                   结束播放返回true
	*/
	virtual bool    IsPlayOver( DWORD _dwID );

	/** \brief
	*   设置音效的音量
	*
	*    \param		_lVolume     声音音效的音量
	*/
	virtual void      SetSoundVolume(  OS_SOUNDTYPE _type, long _lVolume = 75  );

	/** \brief
	*   获得音效的音量
	*
	*    \return         long            返回声音的音量
	*/
	virtual long      GetSoundVolume( OS_SOUNDTYPE _type=SOUNDTYPE_3D ){ 
		return (_type==SOUNDTYPE_3D?m_lSoundVolume:m_lUISoundVolume); }

	// 返回音量的范围
	virtual void       GetVolumeRange(int* min, int* max ){ *min=MIN_VOLUME; *max=MAX_VOLUME; }


public: 
	/** \brief
	*   背景音乐帧播放，对声音系统内的每一个声音处理，缓存或是删除相应的声音。
	*/
	virtual void    FrameMove();


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
	virtual bool    PlayFileMP3( const char *_FileName,BOOL _repeat = TRUE );

	/** \brief
	 *  停止播放mp3
	 *
	 *  这个函数几可以控制内存播放又可以控制文件播放
     *
	 *  \return    bool   停止时是否成功
	 */
	virtual bool	StopMP3();

	//@{ River @ 2010-8-20:加入得到mp3播放位置和设置mp3播放位置的接口。
	virtual LONGLONG GetMp3PlayPosition( void );
	virtual void     SetMp3PlayPosition( LONGLONG  _pos );
	//@}


	/** \brief
	 *  暂停播放
	 *
	 *  这个函数几可以控制内存播放又可以控制文件播放
	 *
	 *  \return    bool   暂停播放
	 */
	virtual bool	PauseMP3( bool _pause = true );

	/** \brief
	 *  静音开关,使用一次静音,再调用一个就可恢复正常
	 */
	virtual void	ToggleMuteMP3( bool _mute = true );


	/** \brief
	*  设置音量
	*
	*  \_lVolume 新的音量值(0----100)
	*/
	virtual bool	SetVolumeMP3( long _lVolume );

	/** \brief
	*  获得背景音乐的音量
	*
	*  \param    _Volume 新的音量值
	*/
	virtual long       GetVolumeMP3();

	virtual const char* GetCurMp3FileName();
	//}@

protected:
	gcs_sound_buffer*	getSoundBuffer(const gcs_sound_id& _soundID);
	
	/** \brief
	* 根据声音文件的名字创建一个声音，并且为他分配缓存
	*
	* \param	_fPlayTime [in,out] 返回音效需要播放的时间
    */
	gcs_sound_buffer*	createSoundBuffer( const char* _lpszFilename,
		                float* _fPlayTime,bool _bForce = false,bool _bUI = false );

	/** \brief
	*  对一个已经播放过的声音缓冲区重新设置，使用缓冲区内存在的声音资源，重新播放
	*
	*  \param _pbuffer  要重设的声音缓冲区。
	*  \param _fPlayTime 暂不使用此参数，用于控制声音播放的时间长短。
	*/
	gcs_sound_buffer*	resetSoundBuffer( gcs_sound_buffer* _pbuff );
	

	/** \brief
	*	重新分配Buffer
	*
	*  如果缓冲区lost,需要重新分配
	*
	*  \param   _pDSB           次缓冲区
	*  \param   BOOL _pbWasRestored  是否重新分配了buffer
	*/
	HRESULT restoreBuffer(LPDIRECTSOUNDBUFFER8 _pDSB, BOOL* _pbWasRestored);

	HRESULT fillBufferWithSound( BYTE* _pbWave, int _dataSize, 
		LPDIRECTSOUNDBUFFER8 _pDSB, BOOL _bRepeatWavIfBufferLarger);
	 

	//! 设置soundBuffer的参数
	bool           set_bufferPara( const os_initSoundSrc* _bufferPara,LPDIRECTSOUND3DBUFFER _buf );

	/** \brief
	 *  真正播放声音的函数，返回了当前播放的hash ID.
	 *
	 */
	gcs_sound_id    playSound(const char* _lpszFileName,
		             const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI, float* _fSoundPlayTime=NULL );


	/** \brief
	 *  从声音系统的缓存中播放当前的声音文件
	 *
	 *  这个系统会缓冲存储一些声音，每次播放声音的时候，先从系统缓存中查找，
	 *  如果要播放的声音，存在于系统缓存中，则使用缓存中的声音。
	 */
	bool           playSoundFromPool( gcs_sound_id* _pid,float* _playTime,
		const char* _lpszFileName,const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI );


	//! 从当前的声音缓冲区集合中删除一个声音
	void           delete_sound( gcs_sound_id& _id );

	
private:
	//@{
	//  初始化相关的函数。
	/** \brief
	 *  设置主缓冲区的格式
	 *
	 *  主缓冲区相当于一个音效混合器,游戏中所有的音效都在这里按着一定的方式进行混合
	 *  然后输出,整个游戏中只能有一个这样的主缓冲区
	 *
	 *  \param   _dwPrimaryFreq     播放音乐的采用频率
	 *           _dwPrimaryBitRate  采样位的bit深度
	 *
	 */
	bool    initPrimaryBufferFormat(DWORD _dwPrimaryFreq, DWORD _dwPrimaryBitRate );


	/** \brief释放声音管理器所用到的资源
	*/
	void    ReleaseSound();


	/** \brief
	 *  设置倾听者
	 *
	 *  整个游戏中只有一个倾听者,也就是玩家角色
	 *
	 *  \param   _sListener     设置3D倾听者的参数
	 *          
	*/
	void    init3DListener(os_lisenerParam &_sListener);
	//@} 
	//! 每次播放都从m_vDSoundBuffers选择一个空闲缓存，
	//! 如果没有就在非空闲中选择一个最能被释放的，然后释放掉?

	//! 当前正在播放的所有声音
	std::map<gcs_sound_id,gcs_sound_buffer*>	m_mSoundBuffer;

	//! 
	unsigned long			m_lCurMaxSoundIndex;

	//! 音效音量
	long					m_lSoundVolume;
	// 界面音效音量
	long                    m_lUISoundVolume;

	//! 内部catch os_lisenerParam
	os_lisenerParam         m_sCatchLp;

	/** \brief
	 *  DirectSound对象
	*/
	LPDIRECTSOUND8           m_lpDirectSound;
	/** \brief
	 * 
	 */
	DSCAPS                   m_deviceCap;
	/*倾听者,即玩家角色,游戏中唯一的一个听者,但有多个声源
	*/
	LPDIRECTSOUND3DLISTENER  m_p3DListener;
	//!倾听者的参数
	DS3DLISTENER             m_sListenParams;

	//! mp3相关的封装类
	fyc_MP3Lib*              m_sMp3Player;
	//
	bool					 m_bForcePlay;
	
	DWORD                    m_dwUISoundOriVolume;
	std::string				 m_CurMp3FileName;
};
//! 使用有限个dsound缓存个数，超过这个个数就释放缓存池中最长时间未使用的buffer，把新的填充进来
#endif//_R4SOUND_H_
