//===================================================================================

/** \file  
*  Filename: fyMP3_Lib.cpp
*  Desc:     ��װ��DirectShow( Microsoft DirectX 9.0 SDK Update (Summer 2003) )
*			  ����MP3����,��������һ����̬�Ŀ�
*
*  ��Ŀ��������ҳ���������Ŀ:
*	���Ӱ���Ŀ¼:BaseClasses
*	Ԥ����������:_WIN32_WINNT=0x400
*/
//===================================================================================

#include <atlbase.h>
# include "fyMP3_Lib.h"
# include "../interface/fileOperate.h"
// !�����ַ���������Ҫ���ø�ͷ�ļ�




/** \brief
* ��Ĺ��캯��.
*
* �Գ����һЩ�������г�ʼ��.
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


	//�ʼû��MP3�ļ�����
	m_bFileOpened		= false;

	//�ʼ���Ǿ���
	m_bMute				= false;

	//��ѭ������
	m_bRepeat			= false;

	//��������Ϊ���ֵ
	m_lCurrentVolume	= 0;

	//���ò����ٶ�Ϊ�����ٶ�
	m_dRate				= 1;

	//�������������ľ���ֵΪ����һ��
	m_lBalance			= 0;

	//����Ϊ�������
	m_bFinished			=true;

	m_lCurrentVolume     = -2000;

	//! River @ 2010-9-12:ȷ���Ƿ���Գ�ʼ��MP3.
	m_bCanInitMp3 = true;

	unguard;
}


/** \brief
* �����������.
*
* ɾ�������е�ָ����� �ͷ��ڴ�
*/
fyc_MP3Lib::~fyc_MP3Lib()
{
	guard;

	//���ͷ���Դ
	ReleaseMP3();

	// ���� COM
	CoUninitialize();

	unguard;

}

//
void fyc_MP3Lib::InitPlayer( HWND _hwnd )
{
	HRESULT hr;

	m_hWnd = _hwnd;

	//��ʼ��COM
	hr=CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );

	if( FAILED( hr ) )
	{
		Outstr_tolog("MUSIC: mp3 ��ʼ��ʧ��");
	}
}

/** \brief
*  ���б�Ҫ�ĳ�ʼ���ĺ���
*
*  \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
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
		// �õ� IMediaControl �Ľӿ�
		hr = m_cGraphBuilder->QueryInterface(	IID_IMediaControl, 
			reinterpret_cast<void **>(&m_cMediaControl) );
	}
	else
	{
		osDebugOut( "��ʼ��mp3�ӿ�ʧ��...." );
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
		// �õ� IMediaEvent �Ľӿ�
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
		// �õ� IMediaControl �Ľӿ�
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
		// �õ� IBasicAudio �Ľӿ�
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
		// �õ� IMediaPosition �Ľӿ�
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
*  ���б�Ҫ������ĺ���
*
*  
*/
void	fyc_MP3Lib::ReleaseMP3()
{
	guard;

	HRESULT hr=E_FAIL;

	// ֹͣ����
	if( m_cMediaControl )
	{
		StopMP3();

		Outstr_tolog("MUSIC: ��������ֹͣ����");
	}

	// �ͷ�����ָ��
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
		//! ATTENTION TO FIX:River @ 2007-2-2:�˳���Ϸʱ��Щ�����ܳ���
		//SAFE_RELEASE( m_cGraphBuilder );
	}

	osDebugOut( "Mp3�������ͷ�....\n" );

	unguard;
}

/** \brief
*  ���ļ�����mp3��������
*
*  \param   _FileName     ���������ֵ��ļ�����
*               _KBPerSec    ���ֵĲ����ٶ�(KB/S), һ��Ϊ128,160,256
*				  _bRepeat		�Ƿ�Ϊѭ������
*
*  \return   bool               ����ֵ�����Ƿ񲥷ųɹ�
*/
bool fyc_MP3Lib::PlayFileMP3( const char *_FileName,BOOL _bRepeat/* = TRUE*/ )
{
	guard;

	// River @ 2010-9-12:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
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
		osDebugOut( "��ǰ�ı������ֲ��Ŵ��ھ���״̬...\n" );

		// �ж������Ƿ����,mp3�ı������ֲ�ʹ�ô���ļ�
		if ( !file_exist( (char*)t_szMp3.c_str(),true ) )
		{
			Outstr_tolog("MUSIC: Ҫ���ŵ����� <%s> ������",t_szMp3.c_str() );
			return false;
		}
		// �����ļ���
		_tcscpy( m_chFileName, t_szMp3.c_str() );
		// Ensure NULL termination
		m_chFileName[ MAX_PATH-1 ] = 0;     

	}
	else
	{
		//  
		//! River @ 2010-9-7: ȷ�ϲ������������ܻ���ֿ�ָ��Ĵ���
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

		Outstr_tolog("MUSIC: ��ǰ���ڲ���%s", t_szMp3.c_str() );
	}

	return true;

	unguard;
}



/** \brief
*  ������һ��ָ����MP3�ļ��ĺ���
*
*  \param  _FileName   �ļ���.
*  \return  HRESULT   ����ɹ�����S_OK.
*/
HRESULT		fyc_MP3Lib::OpenMP3File( const char *_FileName )
{
	guard;

	HRESULT  t_hr = S_OK;
	IBaseFilter*    t_ptrTmpFilter;
    IPin           *pPin = NULL;

	// River mod @ 2011-1-21:���mp3���ų����򲻲��ű������֡�
	try { 

	osassert( m_cGraphBuilder ); 

	// �ж������Ƿ����,mp3�ı������ֲ�ʹ�ô���ļ�
	if ( !file_exist( (char*)_FileName,true ) )
	{
		Outstr_tolog("MUSIC: Ҫ���ŵ����� <%s> ������",_FileName );
		
		return E_FAIL;
	}

	// �����ļ���
	_tcscpy( m_chFileName, _FileName );
	// Ensure NULL termination
	m_chFileName[ MAX_PATH-1 ] = 0;     


	// ת���ɿ��ַ��ļ���
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
					//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
					osDebugOut( "find pin<%s>ʧ��!ԭ��<%s>",m_chFileName,DXGetErrorDescription9( t_hr ) );
					return E_FAIL;
				}
			}
		}
	}
	else
	{
		/*
		osassertex( false,
			va( "add source filter <%s>ʧ��!ԭ��<%s>",m_chFileName,DXGetErrorDescription9( t_hr ) ) );
		*/
		//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
		osDebugOut( "add source filter <%s>ʧ��!ԭ��<%s>",m_chFileName,DXGetErrorDescription9( t_hr ) );
		return E_FAIL;
	}

	// Stop the graph
	if( SUCCEEDED( t_hr ) )
	{
		// River @ 2010-9-13:���������Ϣ���˴�����Ϊ��...
		osassert( m_cMediaControl );

		t_hr = m_cMediaControl->Stop();
	}
	else
	{
		/*
		osassertex( false,
			va( "Find pin <%s>ʧ��!ԭ��<%s>..\n",m_chFileName,DXGetErrorDescription9( t_hr ) ) );
		*/

		//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
		osDebugOut( "Find pin <%s>ʧ��!ԭ��<%s>..\n",m_chFileName,DXGetErrorDescription9( t_hr ) );
		return E_FAIL;
	}
	/*
	osassertex( !FAILED( t_hr ),
		va( "Media Stop error,name<%s>,ԭ��<%s>...\n",m_chFileName,DXGetErrorDescription9( t_hr ) ) );
	*/
	//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
	if( FAILED( t_hr ) )
	{
		osDebugOut( "Media Stop error,name<%s>,ԭ��<%s>...\n",m_chFileName,DXGetErrorDescription9( t_hr ) );
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
			osassertex( false,va( "enum filters <%s>ʧ��!",m_chFileName ) );
			*/
			//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
			osDebugOut( "enum filters <%s>ʧ��!",m_chFileName );
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
		// River @ 2011-10-13: DShow�ڲ����ܳ����Ҳ������Ӿ����ԭ��
		// ������룬ʹmp3�����ʱ���ٴν�����Ϸ�����Բ����������ٲ���mp3.
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
		osassertex( false,va( "����mp3�ļ�<%s>ʧ��!",m_chFileName ) );
		*/
		//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
		osDebugOut( "����mp3�ļ�<%s>ʧ��!",m_chFileName );
		return E_FAIL;
	}


    SAFE_RELEASE( pPin );
    SAFE_RELEASE( t_ptrTmpFilter ); // In case of errors //�����д� 2007-9-8

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
		osassertex( false,va( "Run mp3�ļ�<%s>ʧ��!,GetLastError() == %d",m_chFileName ,GetLastError()) );
		*/
		//! river mod @ 2009-12-21:����mp3ʧ�ܲ�����
		osDebugOut( "Run mp3�ļ�<%s>ʧ��!,GetLastError() == %d",m_chFileName ,GetLastError() );
		return E_FAIL;
	}
 
	// �����ı��� ???
	m_bFinished = false;
	m_fPlayEleTime = 0.0f;
	SetRate( m_dRate );
	m_bRepeat = true;
	m_bFileOpened = true;

	// ����mp3���ŵ�����.
	m_cBasicAudio->put_Volume( m_lCurrentVolume );

	return t_hr;


	//River @ 2011-1-21:���MP3���ŵײ�����򲻲���mp3.
	} catch (CSE& e){
		osDebugOut( "MP3�����쳣<%s>....\n",e.m_func );
		m_bMute = true;
		m_bCanInitMp3 = false;

		return E_FAIL;
	}

	unguard;
}




/** \brief
*   ��������ѭ�����ŵ�֡����
*/
void  fyc_MP3Lib::FrameMove()
{
	guard;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
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
			// ����ǲ������˵�ǰ�����֣������ѭ������
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
*  ��ͣ���ŵĺ���
*
*  \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
*/
bool fyc_MP3Lib::PauseMP3( bool _pause )
{
	guard;
	HRESULT hr=E_FAIL;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	//! River @ 2010-9-7: ȷ�ϲ������.
	if( !m_cMediaControl )
		return true;

	// ֻ���ļ��Ѿ������˵�ʱ���ֹͣ
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
*  ֹͣ���ŵĺ���
*
* \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
*/
bool	fyc_MP3Lib::StopMP3()
{

	guard;


	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	HRESULT hr=E_FAIL;

	// ֻ���ļ��Ѿ������˵�ʱ��Ų���
	if ( m_bFileOpened )
	{
		hr = m_cMediaControl->Stop();

		//m_cVideoWindow->put_Visible( OAFALSE );
		//m_cVideoWindow->put_Owner( NULL );

		//�Ѳ���λ������Ϊ0
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
*  ���ؾ����ĺ���
*/
void		fyc_MP3Lib::ToggleMuteMP3( bool _mute )
{
	guard;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return;

	if( m_bMute == _mute )
		return;

	//�л�����
	m_bMute = _mute;

	//! River @ 2011-1-24:�����ʱ����Ϊ�㣬��ȷ�����ھ���״̬��-10000��dshow��ֵ��
	if( m_lCurrentVolume <= (-10000) )
		m_bMute = true;

	HRESULT t_hr = E_FAIL;
	if( m_bMute )
	{
		// ֻ���ļ��Ѿ������˵�ʱ�������
		if ( m_bFileOpened && m_bCanInitMp3 && (m_cBasicAudio!=	NULL) )
		{
			//�ȱ��浱ǰ����ֵ
			t_hr = m_cBasicAudio->get_Volume( &m_lCurrentVolume );
			osassert( !FAILED( t_hr ) );

			//Ȼ����ɾ���
			t_hr = m_cBasicAudio->put_Volume( VOLUME_SILENCE );

			// River @ 2010-9-13���򵥷��أ�����������
			if( t_hr == DSERR_BUFFERLOST )
				return;

			osassertex( !FAILED( t_hr ),
				va( "��mp3���ŵľ���ʧ��<%s>..\n",DXGetErrorString9( t_hr )) );
		}

		Outstr_tolog("MUSIC: ������������");
	}
	else
	{
		if( m_bFileOpened )
		{
			//�ָ��ɾ���ǰ����õ�����
			t_hr = m_cBasicAudio->put_Volume( m_lCurrentVolume );

			// River @ 2010-9-13���򵥷��أ�����������
			if( t_hr == DSERR_BUFFERLOST )
				return;

			osassertex( !FAILED( t_hr ),
				va( "��mp3���ŵľ���ʧ��<%s>..\n",DXGetErrorString9( t_hr )) );

		}

		Outstr_tolog("MUSIC: �������־�������");
	}

	//! ����ǴӾ��������ţ�����ͷ����
	if( m_bMute == false )
	{
		if( m_chFileName[0] )
			PlayFileMP3( m_chFileName );	
	}

	// ���ء�.	
	return;

	unguard;
}



#define AFGVOLUME_TO_DB(volume)		((int)(-100*(32-(volume))))

/** \brief
*
*  ���������ĺ���
*
*  \param _Volume	�µ�����ֵ(0---100)
*
*  \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
*/
bool fyc_MP3Lib::SetVolume( long _lVolume )
{
	guard;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	// �������淶��0---100
	if (_lVolume<=0)
	{
		//! ���ĳЩ�ͻ��˳���
		m_bMute = true;
		_lVolume = 0;
	}
	else if (_lVolume>=100)
	{
		_lVolume = 100;
	}

	//@{
	// River @ 2010-1-8: ������-10000-0���е㸴��
	// ��������1-100֮���ֵ����Ӧ��50-100֮�䡣
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

	// ֻ���ļ��Ѿ������˵�ʱ��Ų���
	if ( m_bFileOpened )
	{
		HRESULT hr = E_FAIL;
		hr=m_cBasicAudio->put_Volume( m_lCurrentVolume );
		if( FAILED( hr ) )
		{
			Outstr_tolog( "��������ʧ��" );
			return false;
		}
	}
	
	return true;

	unguard;
}

/** \brief
*
*  ���ص�ǰ�������ĺ���
*
*  \return  long   ���ص�ǰ������,ʧ���򷵻�-1
*/
long		fyc_MP3Lib::GetVolume()
{
	guard;

	HRESULT t_hr = E_FAIL;

	// ֻ���ļ��Ѿ������˵�ʱ��Ų���
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
*  ���������ľ������õĺ���
*
*  \param	_Balance �µľ���ֵ( -10000 �� 10000 ),
*								-10000ʱ, ����������Ϊ���ֵ	 ����������Ϊ��
*								10000ʱ,  ����������Ϊ��		����������Ϊ���ֵ
*								0ʱ,	  ����������Ϊ���ֵ	 ����������Ϊ���ֵ
*
*  \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
*/
bool fyc_MP3Lib::SetBalance( long _Balance )
{
	guard;
	HRESULT hr = E_FAIL;

	osassert( _Balance<=10000 && _Balance>=-10000 );


	// ֻ���ļ��Ѿ������˵�ʱ��Ų���
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
*  �õ���ǰ�����������ľ������õ�ֵ�ĺ���
*
*   \return  long   ���ص�ǰ�����������ľ������õ�ֵ,ʧ���򷵻�-1
*/
long		fyc_MP3Lib::GetBalance()
{
	guard;

	HRESULT hr=E_FAIL;

	// ֻ���ļ��Ѿ������˵�ʱ�������
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
*  �õ���ǰ��MP3�����ĳ��ȵĺ���
*
*   \return  LONGLONG   ���ص�ǰ��MP3�����ĳ��ȣ����ŵĺ�������,���ʧ�ܷ���-1
*/
LONGLONG	fyc_MP3Lib::GetDuration()
{
	guard;

	HRESULT hr = S_OK;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	// ֻ���ļ��Ѿ������˵�ʱ��ż���
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
*  ���õ�ǰ���ŵ�λ�õĺ���
*
*   \param	_position �µ�λ��ֵ
*   \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
*/
HRESULT		fyc_MP3Lib::SetPosition( LONGLONG _position )
{
	guard;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	//! River @ 2011-1-21:mp3�����Ȳ�����
	try{

	HRESULT hr = S_OK;

	// ֻ���ļ��Ѿ������˵�ʱ�������
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

	// River mod @ 2010-8-23:��ʱ������.
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

	//River @ 2011-1-21:���MP3���ŵײ�����򲻲���mp3.
	} catch (CSE& e){
		osDebugOut( "MP3�����쳣<%s>....\n",e.m_func );
		m_bMute = true;
		m_bCanInitMp3 = false;

		return E_FAIL;
	}

	unguard;
}


/** \brief
*  �õ���ǰ���ŵ�λ�õĺ���
*
*  \return  LONGLONG   ���ص�ǰ���ŵ�λ��,���ʧ�ܷ���0
*/
LONGLONG	fyc_MP3Lib::GetPosition()
{
	guard;

	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	HRESULT hr = E_FAIL;

	// ֻ���ļ��Ѿ������˵�ʱ��Ż�ȡ
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
*  ���ò��ŵ��ٶȵĺ���
*
*  \param _rate �µ��ٶȵ�ֵ
*  \return  HRESULT   ����S_OKʱ��ʾ�ɹ�
*/
HRESULT		fyc_MP3Lib::SetRate( double _rate )
{
	guard;


	// River @ 2011-1-21:���������ĳ�ʼ�����֣�ֱ�ӷ��ء�
	if( !m_bCanInitMp3 )
		return false;

	HRESULT hr = E_FAIL;

	// ֻ���ļ��Ѿ������˵�ʱ�������
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
*  �õ����ŵ��ٶȵĺ���
*
*  \return  double   ���ص�ǰ���ŵ��ٶ�,ʧ���򷵻�-1
*/
double		fyc_MP3Lib::GetRate()
{
	guard;


	HRESULT hr=E_FAIL;

	// ֻ���ļ��Ѿ������˵�ʱ��ż���
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




