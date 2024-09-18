//===================================================================================

/** \file  
*  Filename: fyMP3_Lib.cpp
*  Desc:     包装了DirectShow( Microsoft DirectX 9.0 SDK Update (Summer 2003) )
*			  播放MP3的类,编译生成一个静态的库
*
*  项目属性设置页添加以下项目:
*	附加包含目录:BaseClasses
*	预处理器定义:_WIN32_WINNT=0x400
*/
//===================================================================================

#include <atlbase.h>
# include "fyMP3_Lib.h"
# include "../interface/fileOperate.h"
// !部分字符串操作需要引用该头文件




/** \brief
* 类的构造函数.
*
* 对程序的一些变量进行初始化.
*/
fyc_MP3Lib::fyc_MP3Lib()
{
	guard;
  
	m_hWnd = NULL;
	m_cVideoWindow = NULL;

	m_pbBuffer          = NULL; 

	m_dwKBPerSec		= 128;

	m_cGraphBuilder		= NULL;
	m_cMediaControl		= NULL;
	m_cMediaSeeking		= NULL;
	m_cBasicAudio		= NULL;
	m_cMediaPosition	= NULL;
	m_cMediaEvent		= NULL;

	m_ptrCurrentFilter  = NULL;


	//最开始没有MP3文件被打开
	m_bFileOpened		= false;

	//最开始不是静音
	m_bMute				= false;

	//不循环播放
	m_bRepeat			= false;

	//设置音量为最高值
	m_lCurrentVolume	= 0;

	//设置播放速度为正常速度
	m_dRate				= 1;

	//设置左右声道的均衡值为左右一样
	m_lBalance			= 0;

	//设置为播放完成
	m_bFinished			=true;

	m_lCurrentVolume     = -2000;

	//! River @ 2010-9-12:确认是否可以初始化MP3.
	m_bCanInitMp3 = true;

	unguard;
}


/** \brief
* 类的析构函数.
*
* 删除程序中的指针变量 释放内存
*/
fyc_MP3Lib::~fyc_MP3Lib()
{
	guard;

	//　释放资源
	ReleaseMP3();

	// 清理 COM
	CoUninitialize();

	unguard;

}

//
void fyc_MP3Lib::InitPlayer( HWND _hwnd )
{
	HRESULT hr;

	m_hWnd = _hwnd;

	//初始化COM
	hr=CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );

	if( FAILED( hr ) )
	{
		Outstr_tolog("MUSIC: mp3 初始化失败");
	}
}

/** \brief
*  进行必要的初始化的函数
*
*  \return  HRESULT   返回S_OK时表示成功
*/
bool fyc_MP3Lib::InitMP3( void )
{

	guard;

	HRESULT hr;


	// Create DirectShow Graph
	hr = CoCreateInstance(	CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,//CLSCTX_INPROC
		reinterpret_cast<void **>( &m_cGraphBuilder ) );

	if( SUCCEEDED( hr ) )
	{
		// 得到 IMediaControl 的接口
		hr = m_cGraphBuilder->QueryInterface(	IID_IMediaControl, 
			reinterpret_cast<void **>(&m_cMediaControl) );
	}
	else
	{
		osDebugOut( "初始化mp3接口失败...." );
		m_bCanInitMp3 = false;
		return false;
	}

	if( SUCCEEDED( hr ) ) //syq
	{
		osassert( m_cMediaControl );//syq
	}
	else
	{
		m_bCanInitMp3 = false;
		return false;
	}


	if( SUCCEEDED( hr ) )
	{
		// 得到 IMediaEvent 的接口
		hr = m_cGraphBuilder->QueryInterface(	IID_IMediaEvent, 
			reinterpret_cast<void **>(&m_cMediaEvent) );
	}
	else
	{
		m_bCanInitMp3 = false;
		return false;
	}


	if( SUCCEEDED( hr ) )
	{
		// 得到 IMediaControl 的接口
		hr = m_cGraphBuilder->QueryInterface(	IID_IMediaSeeking, 
			reinterpret_cast<void **>( &m_cMediaSeeking ) );
	}
	else
	{
		m_bCanInitMp3 = false;
		return false;
	}

	if( SUCCEEDED( hr ) )
	{
		// 得到 IBasicAudio 的接口
		hr = m_cGraphBuilder->QueryInterface(	IID_IBasicAudio,   
			reinterpret_cast<void **>( &m_cBasicAudio ) );
	}
	else
	{
		m_bCanInitMp3 = false;
		return false;
	}

	if( SUCCEEDED( hr ) )
	{
		// 得到 IMediaPosition 的接口
		hr = m_cGraphBuilder->QueryInterface(	IID_IMediaPosition,		
			reinterpret_cast<void **>( &m_cMediaPosition ) );
	}
	else
	{
		m_bCanInitMp3 = false;
		return false;
	}

	if( SUCCEEDED(hr) )
		return true;
	else 
	{
		m_bCanInitMp3 = false;
		return false;
	}

	unguard;
}


/** \brief
*  进行必要的清理的函数
*
*  
*/
void	fyc_MP3Lib::ReleaseMP3()
{
	guard;

	HRESULT hr=E_FAIL;

	// 停止播放
	if( m_cMediaControl )
	{
		StopMP3();

		Outstr_tolog("MUSIC: 背景音乐停止播放");
	}

	// 释放所有指针
	//SAFE_RELEASE( _cVideoWindow );

	SAFE_RELEASE( m_cMediaSeeking );
	SAFE_RELEASE( m_cMediaControl );
	SAFE_RELEASE( m_cBasicAudio );
	SAFE_RELEASE( m_cMediaPosition );
	SAFE_RELEASE( m_cMediaEvent );

	if ( m_cGraphBuilder )
	{
		/*
		ULONG ref = m_cGraphBuilder->AddRef();
		ref = m_cGraphBuilder->Release();
		osDebugOut( "Ref is:<%d>..\n",ref );
		*/
		//! ATTENTION TO FIX:River @ 2007-2-2:退出游戏时，些处可能出错
		//SAFE_RELEASE( m_cGraphBuilder );
	}

	osDebugOut( "Mp3播放器释放....\n" );

	unguard;
}

/** \brief
*  从文件播放mp3背景音乐
*
*  \param   _FileName     待播放音乐的文件名称
*               _KBPerSec    音乐的播放速度(KB/S), 一般为128,160,256
*				  _bRepeat		是否为循环播放
*
*  \return   bool               返回值表明是否播放成功
*/
bool fyc_MP3Lib::PlayFileMP3( const char *_FileName,BOOL _bRepeat/* = TRUE*/ )
{
	guard;

	// River @ 2010-9-12:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	s_string   t_szMp3 = _FileName;
	int t_idx = t_szMp3.find( "sound\\" );
	if( t_idx == -1 )
	{
		t_szMp3 = "sound\\";
		t_szMp3 += _FileName;
	}

	if( m_bMute )
	{
		osDebugOut( "当前的背景音乐播放处于静音状态...\n" );

		// 判断音乐是否存在,mp3的背景音乐不使用打包文件
		if ( !file_exist( (char*)t_szMp3.c_str(),true ) )
		{
			Outstr_tolog("MUSIC: 要播放的音乐 <%s> 不存在",t_szMp3.c_str() );
			return false;
		}
		// 保存文件名
		_tcscpy( m_chFileName, t_szMp3.c_str() );
		// Ensure NULL termination
		m_chFileName[ MAX_PATH-1 ] = 0;     

	}
	else
	{
		//  
		//! River @ 2010-9-7: 确认不会出错，否则可能会出现空指针的错误。
		if( !m_cGraphBuilder )
		{
			if( !InitMP3() )
			{
				SAFE_RELEASE( m_cGraphBuilder );
				return false;
			}
		}

		if( FAILED( OpenMP3File( t_szMp3.c_str() ) ) )
			return false;

		m_bRepeat = _bRepeat;

		Outstr_tolog("MUSIC: 当前正在播放%s", t_szMp3.c_str() );
	}

	return true;

	unguard;
}



/** \brief
*  用来打开一个指定的MP3文件的函数
*
*  \param  _FileName   文件名.
*  \return  HRESULT   如果成功返回S_OK.
*/
HRESULT		fyc_MP3Lib::OpenMP3File( const char *_FileName )
{
	guard;

	HRESULT  t_hr = S_OK;
	IBaseFilter*    t_ptrTmpFilter;
    IPin           *pPin = NULL;

	// River mod @ 2011-1-21:如果mp3播放出错，则不播放背景音乐。
	try { 

	osassert( m_cGraphBuilder ); 

	// 判断音乐是否存在,mp3的背景音乐不使用打包文件
	if ( !file_exist( (char*)_FileName,true ) )
	{
		Outstr_tolog("MUSIC: 要播放的音乐 <%s> 不存在",_FileName );
		
		return E_FAIL;
	}

	// 保存文件名
	_tcscpy( m_chFileName, _FileName );
	// Ensure NULL termination
	m_chFileName[ MAX_PATH-1 ] = 0;     


	// 转换成宽字符文件名
	USES_CONVERSION;
	wcsncpy( m_wFileName, T2W( m_chFileName ), MAX_PATH);

    // OPTIMIZATION OPPORTUNITY
    // This will open the file, which is expensive. To optimize, this
    // should be done earlier, ideally as soon as we knew this was the
    // next file to ensure that the file load doesn't add to the
    // filter swapping time & cause a hiccup.
    //
    // Add the new source filter to the graph. (Graph can still be running)
    t_hr = m_cGraphBuilder->AddSourceFilter( m_wFileName, m_wFileName, &t_ptrTmpFilter );

    if( SUCCEEDED(t_hr) )
	{
		t_hr = t_ptrTmpFilter->FindPin(L"Output", &pPin);  
		if( FAILED( t_hr ) )
		{
			t_hr = t_ptrTmpFilter->FindPin(L"Out", &pPin); 
			if( FAILED( t_hr ) )
			{
				t_hr = t_ptrTmpFilter->FindPin(L"1", &pPin); 
				if( FAILED( t_hr ) )
				{
					//! river mod @ 2009-12-21:播放mp3失败不出错。
					osDebugOut( "find pin<%s>失败!原因<%s>",m_chFileName,DXGetErrorDescription9( t_hr ) );
					return E_FAIL;
				}
			}
		}
	}
	else
	{
		/*
		osassertex( false,
			va( "add source filter <%s>失败!原因<%s>",m_chFileName,DXGetErrorDescription9( t_hr ) ) );
		*/
		//! river mod @ 2009-12-21:播放mp3失败不出错。
		osDebugOut( "add source filter <%s>失败!原因<%s>",m_chFileName,DXGetErrorDescription9( t_hr ) );
		return E_FAIL;
	}

	// Stop the graph
	if( SUCCEEDED( t_hr ) )
	{
		// River @ 2010-9-13:加入调试信息，此处可能为空...
		osassert( m_cMediaControl );

		t_hr = m_cMediaControl->Stop();
	}
	else
	{
		/*
		osassertex( false,
			va( "Find pin <%s>失败!原因<%s>..\n",m_chFileName,DXGetErrorDescription9( t_hr ) ) );
		*/

		//! river mod @ 2009-12-21:播放mp3失败不出错。
		osDebugOut( "Find pin <%s>失败!原因<%s>..\n",m_chFileName,DXGetErrorDescription9( t_hr ) );
		return E_FAIL;
	}
	/*
	osassertex( !FAILED( t_hr ),
		va( "Media Stop error,name<%s>,原因<%s>...\n",m_chFileName,DXGetErrorDescription9( t_hr ) ) );
	*/
	//! river mod @ 2009-12-21:播放mp3失败不出错。
	if( FAILED( t_hr ) )
	{
		osDebugOut( "Media Stop error,name<%s>,原因<%s>...\n",m_chFileName,DXGetErrorDescription9( t_hr ) );
		return E_FAIL;
	}



   // Break all connections on the filters. You can do this by adding 
    // and removing each filter in the graph
    if(SUCCEEDED( t_hr))
    {
        IEnumFilters *pFilterEnum = NULL;

        if(SUCCEEDED( t_hr = m_cGraphBuilder->EnumFilters(&pFilterEnum)))
        {
            int iFiltCount = 0;
            int iPos = 0;

            // Need to know how many filters. If we add/remove filters during the
            // enumeration we'll invalidate the enumerator
            while(S_OK == pFilterEnum->Skip(1))
            {
                iFiltCount++;
            }

            // Allocate space, then pull out all of the 
            IBaseFilter **ppFilters = reinterpret_cast<IBaseFilter **>
                                      (_alloca(sizeof(IBaseFilter *) * iFiltCount));
            pFilterEnum->Reset();

            while(S_OK == pFilterEnum->Next(1, &(ppFilters[iPos++]), NULL));

            SAFE_RELEASE(pFilterEnum);

            for(iPos = 0; iPos < iFiltCount; iPos++)
            {
                m_cGraphBuilder->RemoveFilter(ppFilters[iPos]);

                // Put the filter back, unless it is the old source
                if(ppFilters[iPos] != m_ptrCurrentFilter)
                {
                    m_cGraphBuilder->AddFilter(ppFilters[iPos], NULL);
                }
                SAFE_RELEASE(ppFilters[iPos]);
            }
        }
		else
		{
			/*
			osassertex( false,va( "enum filters <%s>失败!",m_chFileName ) );
			*/
			//! river mod @ 2009-12-21:播放mp3失败不出错。
			osDebugOut( "enum filters <%s>失败!",m_chFileName );
			return E_FAIL;
		}
    }

    // We have the new output pin. Render it
    if( SUCCEEDED(t_hr) )
    {
        // Release the old source filter, if it exists
        SAFE_RELEASE( m_ptrCurrentFilter );
		
		static BOOL t_bFirstRun = FALSE;
		CIniFile  t_file;
		// River @ 2011-10-13: DShow内部可能出错，找不到更加具体的原因，
		// 处理代码，使mp3出错的时候，再次进入游戏，可以不出错，但不再播放mp3.
		if( !t_bFirstRun )
		{
			if( file_exist( "data\\mp3_temp.ini" ) )
			{
				t_file.OpenFileRead( "data\\mp3_temp.ini" );
				t_file.ReadSection( "mp3" );
				int    t_iMp3Render = 0;
				t_file.ReadLine( "mp3Render",(int*)&t_iMp3Render );
				t_file.CloseFile();

				if( !t_iMp3Render )
				{
					m_bCanInitMp3 = false;
					return E_FAIL;
				}

			}
			else
			{
				bool   t_b;
				int    t_iMp3Render = 0;
				t_b = t_file.OpenFileWrite( "data\\mp3_temp.ini" );
				t_file.WriteSection( "mp3" );
				t_file.WriteLine( "mp3Render",t_iMp3Render );
				t_file.CloseFile();
			}
		}

		t_hr = m_cGraphBuilder->Render(pPin);

		if( !t_bFirstRun )
		{
			t_bFirstRun = TRUE;
			if( file_exist( "data\\mp3_temp.ini" ) )
				::DeleteFile( "data\\mp3_temp.ini" );
		}

        m_ptrCurrentFilter = t_ptrTmpFilter;
        t_ptrTmpFilter = NULL;
    }
	else
	{
		/*
		osassertex( false,va( "处理mp3文件<%s>失败!",m_chFileName ) );
		*/
		//! river mod @ 2009-12-21:播放mp3失败不出错。
		osDebugOut( "处理mp3文件<%s>失败!",m_chFileName );
		return E_FAIL;
	}


    SAFE_RELEASE( pPin );
    SAFE_RELEASE( t_ptrTmpFilter ); // In case of errors //还是有错 2007-9-8

    // Re-seek the graph to the beginning
    if(SUCCEEDED( t_hr) )
    {
        LONGLONG llPos = 0;
        t_hr = m_cMediaSeeking->SetPositions(&llPos, AM_SEEKING_AbsolutePositioning,
                                           &llPos, AM_SEEKING_NoPositioning);
    } 

    // Start the graph
	if( SUCCEEDED( t_hr ) ){
        t_hr = m_cMediaControl->Run();
	}else
	{
		/*
		osassertex( false,va( "Run mp3文件<%s>失败!,GetLastError() == %d",m_chFileName ,GetLastError()) );
		*/
		//! river mod @ 2009-12-21:播放mp3失败不出错。
		osDebugOut( "Run mp3文件<%s>失败!,GetLastError() == %d",m_chFileName ,GetLastError() );
		return E_FAIL;
	}
 
	// 其它的变量 ???
	m_bFinished = false;
	m_fPlayEleTime = 0.0f;
	SetRate( m_dRate );
	m_bRepeat = true;
	m_bFileOpened = true;

	// 设置mp3播放的音量.
	m_cBasicAudio->put_Volume( m_lCurrentVolume );

	return t_hr;


	//River @ 2011-1-21:如果MP3播放底层出错，则不播放mp3.
	} catch (CSE& e){
		osDebugOut( "MP3播放异常<%s>....\n",e.m_func );
		m_bMute = true;
		m_bCanInitMp3 = false;

		return E_FAIL;
	}

	unguard;
}




/** \brief
*   背景音乐循环播放的帧处理
*/
void  fyc_MP3Lib::FrameMove()
{
	guard;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return;

	// Caculate the the passed time
	m_fPlayEleTime += sg_timer::Instance()->get_lastelatime();

	long     t_lEventCode;
	long     t_lPara1,t_lPara2;
	if( m_cMediaEvent )
	{
		if( S_OK == m_cMediaEvent->GetEvent( 
			&t_lEventCode,&t_lPara1,&t_lPara2,0 ) )
		{
			// 如果是播放完了当前的音乐，则进行循环播放
			if( EC_COMPLETE == t_lEventCode )
			{
				SetPosition( 0 );
				m_cMediaControl->Run();
			}
		}
	}

	unguard;
}


/** \brief
*  暂停播放的函数
*
*  \return  HRESULT   返回S_OK时表示成功
*/
bool fyc_MP3Lib::PauseMP3( bool _pause )
{
	guard;
	HRESULT hr=E_FAIL;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	//! River @ 2010-9-7: 确认不会出错.
	if( !m_cMediaControl )
		return true;

	// 只有文件已经被打开了的时候才停止
	if ( m_bFileOpened )
	{
		if( _pause )
			hr=m_cMediaControl->Pause();
		else
			hr = m_cMediaControl->Run();
	}

	if( FAILED( hr ) )
	{
		return hr;
	}
	else
	{
		return S_OK;
	}

	unguard;

}

/** \brief
*  停止播放的函数
*
* \return  HRESULT   返回S_OK时表示成功
*/
bool	fyc_MP3Lib::StopMP3()
{

	guard;


	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	HRESULT hr=E_FAIL;

	// 只有文件已经被打开了的时候才播放
	if ( m_bFileOpened )
	{
		hr = m_cMediaControl->Stop();

		//m_cVideoWindow->put_Visible( OAFALSE );
		//m_cVideoWindow->put_Owner( NULL );

		//把播放位置重置为0
		SetPosition( 0 );

		m_bRepeat = true;
	}


	if( FAILED( hr ) )
		return false;
	else
		return true;

	unguard;
}


/** \brief
*  开关静音的函数
*/
void		fyc_MP3Lib::ToggleMuteMP3( bool _mute )
{
	guard;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return;

	if( m_bMute == _mute )
		return;

	//切换开关
	m_bMute = _mute;

	//! River @ 2011-1-24:如果此时音量为零，则确保处于静音状态。-10000是dshow的值。
	if( m_lCurrentVolume <= (-10000) )
		m_bMute = true;

	HRESULT t_hr = E_FAIL;
	if( m_bMute )
	{
		// 只有文件已经被打开了的时候才设置
		if ( m_bFileOpened && m_bCanInitMp3 && (m_cBasicAudio!=	NULL) )
		{
			//先保存当前音量值
			t_hr = m_cBasicAudio->get_Volume( &m_lCurrentVolume );
			osassert( !FAILED( t_hr ) );

			//然后设成静音
			t_hr = m_cBasicAudio->put_Volume( VOLUME_SILENCE );

			// River @ 2010-9-13：简单返回，不做出错处理。
			if( t_hr == DSERR_BUFFERLOST )
				return;

			osassertex( !FAILED( t_hr ),
				va( "对mp3播放的静音失败<%s>..\n",DXGetErrorString9( t_hr )) );
		}

		Outstr_tolog("MUSIC: 静音背景音乐");
	}
	else
	{
		if( m_bFileOpened )
		{
			//恢复成静音前保存好的音量
			t_hr = m_cBasicAudio->put_Volume( m_lCurrentVolume );

			// River @ 2010-9-13：简单返回，不做出错处理。
			if( t_hr == DSERR_BUFFERLOST )
				return;

			osassertex( !FAILED( t_hr ),
				va( "对mp3播放的静音失败<%s>..\n",DXGetErrorString9( t_hr )) );

		}

		Outstr_tolog("MUSIC: 背景音乐静音结束");
	}

	//! 如果是从静音到播放，则重头播放
	if( m_bMute == false )
	{
		if( m_chFileName[0] )
			PlayFileMP3( m_chFileName );	
	}

	// 返回　.	
	return;

	unguard;
}



#define AFGVOLUME_TO_DB(volume)		((int)(-100*(32-(volume))))

/** \brief
*
*  设置音量的函数
*
*  \param _Volume	新的音量值(0---100)
*
*  \return  HRESULT   返回S_OK时表示成功
*/
bool fyc_MP3Lib::SetVolume( long _lVolume )
{
	guard;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	// 把音量规范到0---100
	if (_lVolume<=0)
	{
		//! 解决某些客户端出现
		m_bMute = true;
		_lVolume = 0;
	}
	else if (_lVolume>=100)
	{
		_lVolume = 100;
	}

	//@{
	// River @ 2010-1-8: 对数的-10000-0，有点复杂
	// 接下来把1-100之间的值，对应到50-100之间。
	int   t_mTmpVal = 0;
	if( _lVolume >= 10 )
	{
		float  t_f = _lVolume/100.0f;
		t_mTmpVal = 65 + int( t_f*(100-65) );
	}
	else
	{
		if( _lVolume > 1 )
		{
			float  t_f = _lVolume/100.0f;
			t_mTmpVal = 50 + int( t_f*(100-50) );
		}
	}
	//@}

	osDebugOut( "The src vol<%d>.dst vol<%d>..\n",_lVolume,t_mTmpVal );


	m_lCurrentVolume = -(10000-100*t_mTmpVal);

	// 只有文件已经被打开了的时候才播放
	if ( m_bFileOpened )
	{
		HRESULT hr = E_FAIL;
		hr=m_cBasicAudio->put_Volume( m_lCurrentVolume );
		if( FAILED( hr ) )
		{
			Outstr_tolog( "设置音量失败" );
			return false;
		}
	}
	
	return true;

	unguard;
}

/** \brief
*
*  返回当前的音量的函数
*
*  \return  long   返回当前的音量,失败则返回-1
*/
long		fyc_MP3Lib::GetVolume()
{
	guard;

	HRESULT t_hr = E_FAIL;

	// 只有文件已经被打开了的时候才播放
	if ( m_bFileOpened )
		t_hr = m_cBasicAudio->get_Volume( &m_lCurrentVolume );

	if( FAILED( t_hr ) )
	{
		return -1;
	}
	else
	{
		long  t_lVolume =  100 + m_lCurrentVolume/100;
		
		return t_lVolume;
	}

	unguard;
}


/** \brief
*  左右声道的均衡设置的函数
*
*  \param	_Balance 新的均衡值( -10000 ～ 10000 ),
*								-10000时, 左声道音量为最大值	 右声道音量为零
*								10000时,  左声道音量为零		右声道音量为最大值
*								0时,	  左声道音量为最大值	 右声道音量为最大值
*
*  \return  HRESULT   返回S_OK时表示成功
*/
bool fyc_MP3Lib::SetBalance( long _Balance )
{
	guard;
	HRESULT hr = E_FAIL;

	osassert( _Balance<=10000 && _Balance>=-10000 );


	// 只有文件已经被打开了的时候才播放
	if ( m_bFileOpened )
	{
		m_lBalance=_Balance;
		hr=m_cBasicAudio->put_Balance( m_lBalance );
	}

	if( FAILED( hr ) )
	{
		return false;
	}
	else
	{
		return true;
	}

	unguard;
}



/** \brief
*  得到当前的左右声道的均衡设置的值的函数
*
*   \return  long   返回当前的左右声道的均衡设置的值,失败则返回-1
*/
long		fyc_MP3Lib::GetBalance()
{
	guard;

	HRESULT hr=E_FAIL;

	// 只有文件已经被打开了的时候才设置
	if ( m_bFileOpened )
	{
		hr=m_cBasicAudio->get_Balance( &m_lBalance );
	}

	if( FAILED( hr ) )
	{
		return -1;
	}
	else
	{
		return m_lBalance;
	}

	unguard;
}


/** \brief
*  得到当前的MP3歌曲的长度的函数
*
*   \return  LONGLONG   返回当前的MP3歌曲的长度（播放的毫秒数）,如果失败返回-1
*/
LONGLONG	fyc_MP3Lib::GetDuration()
{
	guard;

	HRESULT hr = S_OK;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	// 只有文件已经被打开了的时候才继续
	if ( !m_bFileOpened )
	{
		return -1;
	}


	// Is media time supported for this file?
	if ( S_OK != m_cMediaSeeking->IsFormatSupported( &TIME_FORMAT_MEDIA_TIME ) )
		return E_NOINTERFACE;

	// Read the time format to restore later
	GUID guidOriginalFormat;
	hr = m_cMediaSeeking->GetTimeFormat( &guidOriginalFormat );
	if ( FAILED(hr) )
	{
		return -1;
	}

	// Ensure media time format for easy display
	hr = m_cMediaSeeking->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );
	if ( FAILED(hr) )
	{
		return -1;
	}



	hr=m_cMediaSeeking->GetDuration( &m_llDuration );
	if ( FAILED(hr) )
	{
		return -1;
	}


	// Return to the original format
	if ( guidOriginalFormat != TIME_FORMAT_MEDIA_TIME )
	{
		hr = m_cMediaSeeking->SetTimeFormat( &guidOriginalFormat );
		if ( FAILED(hr) )
		{
			return -1;
		}
	}

	if( SUCCEEDED(hr) )
	{
		m_llDuration/=10000;
		return m_llDuration;
	}
	else
	{
		return -1;
	}

	unguard;

}


/** \brief
*  设置当前播放的位置的函数
*
*   \param	_position 新的位置值
*   \return  HRESULT   返回S_OK时表示成功
*/
HRESULT		fyc_MP3Lib::SetPosition( LONGLONG _position )
{
	guard;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	//! River @ 2011-1-21:mp3播放先不出错。
	try{

	HRESULT hr = S_OK;

	// 只有文件已经被打开了的时候才设置
	if ( !m_bFileOpened )
	{
		return -1;
	}

	// Is media time supported for this file?
	if ( S_OK != m_cMediaSeeking->IsFormatSupported( &TIME_FORMAT_MEDIA_TIME ) )
		return E_NOINTERFACE;

	// Read the time format to restore later
	GUID guidOriginalFormat;
	hr = m_cMediaSeeking->GetTimeFormat( &guidOriginalFormat );
	if ( FAILED(hr) )
	{
		return -1;
	}

	// Ensure media time format for easy display
	hr = m_cMediaSeeking->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );
	if ( FAILED(hr) )
	{
		return -1;
	}

	GetDuration();

	// River mod @ 2010-8-23:临时不出错.
	//osassert( _position>=0 && _position<=m_llDuration );
	if( (_position>=0) && (_position<=m_llDuration) )
	{
		m_llCurrentPosition = _position;
		m_llCurrentPosition*=10000;
	}
	else
		m_llCurrentPosition = 0;

	hr = m_cMediaSeeking->SetPositions( &m_llCurrentPosition, AM_SEEKING_AbsolutePositioning,
		&m_llCurrentPosition, AM_SEEKING_NoPositioning );


	// Return to the original format
	if ( guidOriginalFormat != TIME_FORMAT_MEDIA_TIME )
	{
		hr = m_cMediaSeeking->SetTimeFormat( &guidOriginalFormat );
		if ( FAILED(hr) )
		{
			return -1;
		}
	}


	if( SUCCEEDED(hr) )
	{
		return S_OK;
	}
	else
	{
		return hr;
	}

	//River @ 2011-1-21:如果MP3播放底层出错，则不播放mp3.
	} catch (CSE& e){
		osDebugOut( "MP3播放异常<%s>....\n",e.m_func );
		m_bMute = true;
		m_bCanInitMp3 = false;

		return E_FAIL;
	}

	unguard;
}


/** \brief
*  得到当前播放的位置的函数
*
*  \return  LONGLONG   返回当前播放的位置,如果失败返回0
*/
LONGLONG	fyc_MP3Lib::GetPosition()
{
	guard;

	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	HRESULT hr = E_FAIL;

	// 只有文件已经被打开了的时候才获取
	if ( !m_bFileOpened )
	{
		return 0;
	}


	// Is media time supported for this file?
	if ( S_OK != m_cMediaSeeking->IsFormatSupported( &TIME_FORMAT_MEDIA_TIME ) )
		return 0;

	// Read the time format to restore later
	GUID guidOriginalFormat;
	hr = m_cMediaSeeking->GetTimeFormat( &guidOriginalFormat );
	if ( FAILED(hr) )
	{
		return 0;
	}

	// Ensure media time format for easy display
	hr = m_cMediaSeeking->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );
	if ( FAILED(hr) )
	{
		return 0;
	}

	hr = m_cMediaSeeking->GetPositions( &m_llCurrentPosition, &m_llStopPosition );


	// Return to the original format
	if ( guidOriginalFormat != TIME_FORMAT_MEDIA_TIME )
	{
		hr = m_cMediaSeeking->SetTimeFormat( &guidOriginalFormat );
		if ( FAILED(hr) )
		{
			return 0;
		}
	}

	if( SUCCEEDED(hr) )
	{
		m_llCurrentPosition/=10000;
		return m_llCurrentPosition;
	}
	else
	{
		return 0;
	}

	unguard;
}



/** \brief
*  设置播放的速度的函数
*
*  \param _rate 新的速度的值
*  \return  HRESULT   返回S_OK时表示成功
*/
HRESULT		fyc_MP3Lib::SetRate( double _rate )
{
	guard;


	// River @ 2011-1-21:不能正常的初始化音乐，直接返回。
	if( !m_bCanInitMp3 )
		return false;

	HRESULT hr = E_FAIL;

	// 只有文件已经被打开了的时候才设置
	if ( m_bFileOpened )
	{
		m_dRate=_rate;
		hr=m_cMediaPosition->put_Rate( m_dRate );
	}

	if( SUCCEEDED(hr) )
	{
		return S_OK;
	}
	else
	{
		return hr;
	}

	unguard;
}


/** \brief
*  得到播放的速度的函数
*
*  \return  double   返回当前播放的速度,失败则返回-1
*/
double		fyc_MP3Lib::GetRate()
{
	guard;


	HRESULT hr=E_FAIL;

	// 只有文件已经被打开了的时候才继续
	if ( m_bFileOpened )
	{
		hr= m_cMediaPosition->get_Rate( &m_dRate );
	}

	osassert( m_dRate!=0 );

	if( SUCCEEDED(hr) )
	{
		return m_dRate;
	}
	else
	{
		return -1;
	}

	unguard;
}




