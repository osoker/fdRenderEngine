//=======================================================================================================
/**  \file
 *   Filename: smPlayerInterface.h
 *   Desc:     引擎中用到的声音播放相关的接口。
 *  
 *   His:      River created @ 2005-8-1
 *
 *   
 *  
 */
//=======================================================================================================
# pragma  once

/** 
 *  不使用common_mfc版本的common库，在memoryManager.h的第一行，把
 *   # define TRACKMEM_ALLOCK  0
 *   宏的值定义为0, 则编译可以在mfc下使用的common和渲染库，设为1,则
 *   全局的重载new和delete. 
 */
# include "../../common/com_include.h"

#pragma comment (lib,"winmm.lib")


#ifdef SMMANAGER_EXPORTS
#define SMMANAGER_API __declspec(dllexport)
#else
#define SMMANAGER_API __declspec(dllimport)
#endif

// 音效类型
enum OS_SOUNDTYPE{ SOUNDTYPE_UI = 0, SOUNDTYPE_3D };

//! 倾听者初始化参数
struct os_lisenerParam
{
	//!倾听者的初始位置,三维参数只是相对意义,因此采用与D3D中同一个坐标系即可
	osVec3D   m_vPosition;
	//!倾听者的上方向
	osVec3D   m_vOrientFront;
    //!倾听者的正面朝向
    osVec3D   m_vOrientTop;
	//! 衰减因子0.0f-----10.0f
	float           m_fRolloff;
	//! 距离因子
	float           m_fDisFactor;

	//! 缺省构造函数
	os_lisenerParam()
	{
		m_fRolloff   = 1.0f;            m_fDisFactor    = 1.0f;
	}
 
	//! 构造函数
	os_lisenerParam(osVec3D& _pos, osVec3D& _front, osVec3D& _top, 
		float _fRolloff=1.0f, float _fDisFactor=1.0f )
	{
		m_vPosition = _pos;			   m_vOrientFront = _front; m_vOrientTop = _top;
		m_fRolloff   = _fRolloff;     m_fDisFactor    = _fDisFactor;
	}

	//! 是否相同的参数，如果相同，内部不需处理
	bool operator==( os_lisenerParam& _s );

};

struct os_initSoundSrc
{
	//!发声者的初始位置
	osVec3D   m_vPosition;
	//!最近距离,在这个范围内最大,且不变;声音默认以米为单位,
	//起声音随距离衰减的作用;譬如,minDis为2,则与声源相距4时,听到的音量减半
	float     m_flMinDistance;
	//!最大距离,在这个距离以外,音量达到最小,且不再减小
	float     m_flMaxDistance; 
	//!声源的音量0到100;
	long      m_lVolume;
	//! 当前音效的播放时间,如果为0则按正常时间（速度）播放
	float     m_fPlayTime;

	//! 缺省构造函数
	os_initSoundSrc()
	{
		m_fPlayTime = 0.0f;
		m_lVolume = 100;
	}
	//! 构造函数
	os_initSoundSrc(osVec3D& _pos, float _minDis=9.0f, float _maxDis=100.0f, long _volume=100, float _fPlayTime=0 )
	{
		m_vPosition     = _pos;
		m_flMinDistance = _minDis;
		m_flMaxDistance = _maxDis;
		m_lVolume       = _volume;
		m_fPlayTime     = _fPlayTime; 
	}
};


/** \brief
 *  声音管理器
 *
 *  用于初始化DirectSound,对DircectSound进行各种3D参数设置
 *  同时也负责3D音效的播放停止等
 */ 
struct I_soundManager
{
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
	virtual bool   InitSoundManager(HWND _hWnd,DWORD _dwPrimaryFreq, 	DWORD _dwPrimaryBitRate,
		os_lisenerParam &_sListenerPara,DWORD _dwCoopLevel=DSSCL_PRIORITY)=0;

	/** \brief
	 *  释放声音管理器中所有的声音资源.
	 *  
	 */
	virtual bool   destory_allsoundResource( void ) = 0;

	/** \brief
	 *  恢复系统音量
	 *  
	 */
	virtual void   ResetSoundVolume( void ) = 0;

	/** \brief
	 *  从磁盘文件播放音效
	 *
	 *  \param:  _szFileName    要播放的生效文件名称
     *           _bufferPara    3D buffer参数 这个参数是某个声源的属性,如果
	 *                          是播放二维的界面声音,此参数可以为NULL.
	 *           _bLoop         是构循环播放  0为不循环,1为循环
	 *           _fSoundPlayTime[in,out] 返回音效的播放时间
	 *
	 *  \return  DWORD          返回一个声源的唯一ID      
	*/
	virtual DWORD   PlayFromFile(char *_szFileName, const os_initSoundSrc* _bufferPara,
		bool _bLoop = false, bool _bUI = false, float* _fSoundPlayTime = NULL )	=	0;  


	/** \brief
	 *  停止播放某个音效
	 *
	 *  \param   _dwID          可以作为所有发声对象的一个唯一标识
	*/
	virtual void    StopSound(DWORD _dwID)=0;

	/** \brief
	 *  设置发声对象的属性 每一帧做一次处理
	 *
	 *  \param   _dwID              发生对象与Secondory buffer的唯一对应标识  
	 *  
	 *           _vecSoundPos       发声对象的位置 
	 *           _velocity          可以为空，表示了声音的速度
	 *  \return  bool               处于播放状态true;否则返回false
	*/
	virtual bool    FrameMoveSndPos(DWORD _dwID, osVec3D& _vecSoundPos,osVec3D* _velocity = NULL )=0;

	/** \brief
	 *  设置倾听者的位置,需要时随时设置
	 *
	 *  \param   _sListenerParams   设置倾听者的参数
	 *
	 *  \return  bool               是否设置成功
	 */
	virtual bool    SetListenerPara(os_lisenerParam& _sListenerParams)=0;

	/** \brief
	*   判断当前音源是否正在播放
	*
	*    \param _dwID              声源ID
	*
	*    \return bool                   结束播放返回true
	*/
	virtual bool    IsPlayOver(DWORD _dwID) = 0;

	/** \brief
	*   设置音效的音量
	*
	*    \param		_lVolume     声音音效的音量
	*/
	virtual void      SetSoundVolume( OS_SOUNDTYPE _type, long _lVolume = 75 ) = 0;

	/** \brief
	*   获得音效的音量
	*
	*    \return         long            返回声音的音量
	*/
	virtual long          GetSoundVolume( OS_SOUNDTYPE _type ) = 0;

	// 返回音量的范围
	virtual void          GetVolumeRange( int* min, int* max ) = 0;	


    //**********************************************************************************
	//***************************MP3背景音乐的播放**************************************
	//!Attention: 对于mp3的播放无需对管理器进行初始化,用完后也无需Release
	//**********************************************************************************
	//{@!以下为mp3背景音乐的播放,其实其他一些格式,譬如wav等
	/** \brief
	 *  从文件播放mp3背景音乐
	 *
	 *  \return   bool              返回值表明是否播放成功
	 *  \param const char*          要播放的mp3文件名
	 *  \param BOOL _repeat         是否循正播放输入的mp3文件。
	 */
	virtual bool    PlayFileMP3(const char *_FileName,BOOL _repeat = TRUE )=0;

	virtual const char* GetCurMp3FileName() = 0;

	/** \brief
	*   背景音乐帧播放
	*/
	virtual void    FrameMove() = 0;

	/** \brief
	 *  停止播放mp3
	 *
	 *  这个函数几可以控制内存播放又可以控制文件播放
     *
	 *  \return    bool   停止时是否成功
	 */
	virtual bool	StopMP3()=0;

	//@{ River @ 2010-8-20:加入得到mp3播放位置和设置mp3播放位置的接口。
	virtual LONGLONG GetMp3PlayPosition( void ) = 0;
	virtual void     SetMp3PlayPosition( LONGLONG  _pos ) = 0;
	//@}

	/** \brief
	 *  暂停播放
	 *
	 *  这个函数几可以控制内存播放又可以控制文件播放
	 *
	 *  \return    bool   暂停播放
	 *  \param bool _pause 如果为true,则暂停播放mp3,如果为false,则重新播放mp3
	 *  
	 */
	virtual bool	PauseMP3( bool _pause = true )=0;

	/** \brief
	 *  mp3背景音乐播放的静音开关
	 *
	 *  \param bool _mute 为真则使当前的mp3播放静音，为假则打开mp3播放。
	 *                    如果mute为真，调用PlayFileMP3新播放mp3时，内部不做任何的处理。
	 */
	virtual void	ToggleMuteMP3( bool _mute = true )=0;


	/** \brief
	*  设置音量
	*
	*  \_lVolume 新的音量值(0----100)
	*/
	virtual bool	SetVolumeMP3( long _lVolume )=0;

	/** \brief
	*  获得背景音乐的音量
	*
	*  \param    _Volume 新的音量值
	*/
	virtual long       GetVolumeMP3( ) = 0;
	//}@

};

//! 全局的声音开关，可以打开和关闭声音的播放，默认打开。
extern SMMANAGER_API BOOL      g_bUseSound;
//! 引擎使用这个变量来确认声音管理器是否初始化，如果没有初始化，则此时不处理声音相关的数据
extern SMMANAGER_API BOOL      g_bUseSoundMgrHaveInit;

//!获取全局的声音管理器指针
SMMANAGER_API I_soundManager*   get_soundManagerPtr( void );
//! 释放全局的声音管理器指针,此函数必须可以多次释放不出现问题,在release_renderRes函数中会调用
SMMANAGER_API void             release_soundManager( void );




