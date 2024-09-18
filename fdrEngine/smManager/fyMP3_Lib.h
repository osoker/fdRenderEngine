//===================================================================================
/** \file  
*  Filename: fyMP3_Lib.h
*  Desc:     包装了DirectShow( Microsoft DirectX 9.0 SDK Update (Summer 2003) )
*			  播放MP3的类,编译生成一个静态的库
*
*	
*	预处理器定义:	_WIN32_WINNT=0x400
* 
*/
//===================================================================================


#ifndef __MP3_H_
#define __MP3_H_

#undef MAX_VALUE

#if _MSC_VER > 1310
		
	// the VS 2005 move the direct show's function and interface to the 
	// platform SDK

	#include <strmif.h>     // Generated IDL header file for streams interfaces
	//#include <amvideo.h>    // ActiveMovie video interfaces and definitions
	//#include <amaudio.h>    // ActiveMovie audio interfaces and definitions
	#include <control.h>    // generated from control.odl
	//#include <evcode.h>     // event code definitions
	#include <uuids.h>      // declaration of type GUIDs and well-known clsids
	//#include <errors.h>     // HRESULT status and error definitions
	//#include <edevdefs.h>   // External device control interface defines
	//#include <audevcod.h>   // audio filter device error event codes
	//#include <dvdevcod.h>   // DVD error event codes

	// the platform SDK can't find follow macro
	#define EC_COMPLETE 0x01

#else
	// !所有使用了DirectShow的应用程序都必须引用该头文件
	#include <dshow.h>

#endif //_MSC_VER > 1310

// !部分字符串操作需要引用该头文件
#include <tchar.h>



# include "../interface/smPlayerInterface.h"


// Exports class identifiers (CLSIDs) and interface identifiers (IIDs). 
// All DirectShow applications require this library.
# pragma comment( lib,"strmiids.lib")



// 设置静音时时的音量
#define		VOLUME_SILENCE	-10000L
class      fyc_MP3Lib;


/** \brief
*  播放MP3的类.
*
*  包含一组常用的操作
*/
class fyc_MP3Lib
{
public:
	/** \brief
	* 类的构造函数.
	*
	* 对程序的一些变量进行初始化.
	*/
	fyc_MP3Lib();
	~fyc_MP3Lib();

    //
	void       InitPlayer( HWND _hwnd );

	/** \brief
	*   进行必要的初始化
	*
	*   \return    bool   mp3初始化是否成功  
	*/
	bool		InitMP3( void );

	/** \brief
	*  进行必要的清理
	*/
	void		ReleaseMP3();


	/** \brief
	*  从文件播放mp3背景音乐
	*
	*  \param   _FileName     待播放音乐的文件名
	*  \param   BOOL _bRepeat		是否为循环播放
	*
	*  \return   bool               返回值表明是否播放成功
	*/
	bool       PlayFileMP3( const char *_FileName,BOOL _bRepeat = TRUE );

	/** \brief
	*  设置当前播放的位置
	*
	*  \_position 新的位置值
	*/
	HRESULT		SetPosition( LONGLONG _position );


	/** \brief
	*  返回当前播放的位置
	*
	*  
	*/
	LONGLONG	GetPosition();


	/** \brief
	*   背景音乐循环播放的帧处理
	*
	*   此函数主要用于使mp3能够循环播放。
	*/
	void       FrameMove();

	/** \brief
	*  暂停播放
	*/
	bool		PauseMP3( bool _pause );

	/** \brief
	*  停止播放
	*/
	bool		StopMP3();

	/** \brief
	*  mp3音乐静音开关
	*
	*  \param _mute 如果为真，则关闭mp3的播放。
	*/
	void		ToggleMuteMP3( bool _mute );



	/** \brief
	*  设置音量
	*
	*  \_Volume 新的音量值
	*/
	bool		SetVolume( long _lVolume );
	/** \brief
	*  返回当前的音量
	*
	*     \return long 返回背景音乐的音量
	*/
	long		GetVolume();

private:

	/** \brief
	*  用来 打开一个指定的MP3文件.
	*
	*  \_FileName 文件名
	*/
	HRESULT		OpenMP3File( const char *_FileName );


	/** \brief
	*  左右声道的均衡设置
	*
	*  \_Balance 新的均衡值( -10000 ～ 10000 ),
	*						-10000时, 左声道音量为最大值	 右声道音量为零
	*						10000时,  左声道音量为零		右声道音量为最大值
	*						0时,	  左声道音量为最大值	 右声道音量为最大值
	*/
	bool		SetBalance( long _Balance );
	/** \brief
	*  返回当前的左右声道的均衡设置的值
	*
	*  
	*/
	long		GetBalance();


	/** \brief
	*  返回当前的MP3歌曲的长度（播放的毫秒数）
	*
	*  
	*/
	LONGLONG	GetDuration();


	/** \brief
	*  设置播放的速度
	*
	*  \_rate 新的速度的值
	*/
	HRESULT		SetRate( double _rate );

	/** \brief
	*  返回当前播放的速度
	*
	*  
	*/
	double		GetRate();


private:

	//! 是否可以正确的初始化MP3.
	bool            m_bCanInitMp3;

	//! 分配mp3内存缓冲区
	PBYTE    m_pbBuffer;
	//! 是否已经打开MP3文件(或赋于了内存块)
	bool			m_bFileOpened;

	//! 歌曲是否已经播放完毕(已经可以用来再次播放了)
	bool			m_bFinished;

	//! 是否循环播放
	bool			m_bRepeat;

	//! 是否为静音
	bool			m_bMute;

	//! MP3歌曲的文件名(字符串数组)
	char			m_chFileName[MAX_PATH];
	//! MP3歌曲的文件名(16位 Unicode字符串数组)
	WCHAR			m_wFileName[MAX_PATH];

	//! 当前的音量
	long int		m_lCurrentVolume;

	//! 当前左右声道的均衡值
	long int		m_lBalance;

	//! 当前播放的速度
	double			m_dRate;

	//! 背景音乐已经播放的时间(秒)
	float                     m_fPlayEleTime;

	//! 当前MP3歌曲播放的时间长度(毫秒)
	LONGLONG		m_llDuration;

	//! 当前的播放位置
	LONGLONG		m_llCurrentPosition;

	//! 歌曲原本结束时停止的位置
	LONGLONG		m_llStopPosition;

	//! 人为设置的停止的位置
	double			m_dStopPos;

	//! 每秒播放多少K字节,一般为128,160,256
	DWORD			m_dwKBPerSec;

	//! 定义一个IGraphBuilder指针
	IGraphBuilder	*m_cGraphBuilder;

	//! 定义一个IMediaControl指针
	IMediaControl	*m_cMediaControl;

	//! 定义一个IMediaSeeking指针
	IMediaSeeking	*m_cMediaSeeking;

	//! 定义一个IBasicAudio指针
	IBasicAudio		*m_cBasicAudio;

	//! 定义一个IMediaPosition指针
	IMediaPosition	*m_cMediaPosition;


	//! 定义一个IFilterGraph指针
	IFilterGraph	*m_cFilterGraph;


	//! 当前的Graph用到的filter指针
	IBaseFilter*    m_ptrCurrentFilter;

	//! 定义一个IMediaEvent指针
	IMediaEvent*	m_cMediaEvent;

	IVideoWindow*   m_cVideoWindow;

	HWND            m_hWnd;
};

#endif//#endif __MP3_H_