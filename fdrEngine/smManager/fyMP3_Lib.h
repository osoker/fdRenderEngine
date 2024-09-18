//===================================================================================
/** \file  
*  Filename: fyMP3_Lib.h
*  Desc:     ��װ��DirectShow( Microsoft DirectX 9.0 SDK Update (Summer 2003) )
*			  ����MP3����,��������һ����̬�Ŀ�
*
*	
*	Ԥ����������:	_WIN32_WINNT=0x400
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
	// !����ʹ����DirectShow��Ӧ�ó��򶼱������ø�ͷ�ļ�
	#include <dshow.h>

#endif //_MSC_VER > 1310

// !�����ַ���������Ҫ���ø�ͷ�ļ�
#include <tchar.h>



# include "../interface/smPlayerInterface.h"


// Exports class identifiers (CLSIDs) and interface identifiers (IIDs). 
// All DirectShow applications require this library.
# pragma comment( lib,"strmiids.lib")



// ���þ���ʱʱ������
#define		VOLUME_SILENCE	-10000L
class      fyc_MP3Lib;


/** \brief
*  ����MP3����.
*
*  ����һ�鳣�õĲ���
*/
class fyc_MP3Lib
{
public:
	/** \brief
	* ��Ĺ��캯��.
	*
	* �Գ����һЩ�������г�ʼ��.
	*/
	fyc_MP3Lib();
	~fyc_MP3Lib();

    //
	void       InitPlayer( HWND _hwnd );

	/** \brief
	*   ���б�Ҫ�ĳ�ʼ��
	*
	*   \return    bool   mp3��ʼ���Ƿ�ɹ�  
	*/
	bool		InitMP3( void );

	/** \brief
	*  ���б�Ҫ������
	*/
	void		ReleaseMP3();


	/** \brief
	*  ���ļ�����mp3��������
	*
	*  \param   _FileName     ���������ֵ��ļ���
	*  \param   BOOL _bRepeat		�Ƿ�Ϊѭ������
	*
	*  \return   bool               ����ֵ�����Ƿ񲥷ųɹ�
	*/
	bool       PlayFileMP3( const char *_FileName,BOOL _bRepeat = TRUE );

	/** \brief
	*  ���õ�ǰ���ŵ�λ��
	*
	*  \_position �µ�λ��ֵ
	*/
	HRESULT		SetPosition( LONGLONG _position );


	/** \brief
	*  ���ص�ǰ���ŵ�λ��
	*
	*  
	*/
	LONGLONG	GetPosition();


	/** \brief
	*   ��������ѭ�����ŵ�֡����
	*
	*   �˺�����Ҫ����ʹmp3�ܹ�ѭ�����š�
	*/
	void       FrameMove();

	/** \brief
	*  ��ͣ����
	*/
	bool		PauseMP3( bool _pause );

	/** \brief
	*  ֹͣ����
	*/
	bool		StopMP3();

	/** \brief
	*  mp3���־�������
	*
	*  \param _mute ���Ϊ�棬��ر�mp3�Ĳ��š�
	*/
	void		ToggleMuteMP3( bool _mute );



	/** \brief
	*  ��������
	*
	*  \_Volume �µ�����ֵ
	*/
	bool		SetVolume( long _lVolume );
	/** \brief
	*  ���ص�ǰ������
	*
	*     \return long ���ر������ֵ�����
	*/
	long		GetVolume();

private:

	/** \brief
	*  ���� ��һ��ָ����MP3�ļ�.
	*
	*  \_FileName �ļ���
	*/
	HRESULT		OpenMP3File( const char *_FileName );


	/** \brief
	*  ���������ľ�������
	*
	*  \_Balance �µľ���ֵ( -10000 �� 10000 ),
	*						-10000ʱ, ����������Ϊ���ֵ	 ����������Ϊ��
	*						10000ʱ,  ����������Ϊ��		����������Ϊ���ֵ
	*						0ʱ,	  ����������Ϊ���ֵ	 ����������Ϊ���ֵ
	*/
	bool		SetBalance( long _Balance );
	/** \brief
	*  ���ص�ǰ�����������ľ������õ�ֵ
	*
	*  
	*/
	long		GetBalance();


	/** \brief
	*  ���ص�ǰ��MP3�����ĳ��ȣ����ŵĺ�������
	*
	*  
	*/
	LONGLONG	GetDuration();


	/** \brief
	*  ���ò��ŵ��ٶ�
	*
	*  \_rate �µ��ٶȵ�ֵ
	*/
	HRESULT		SetRate( double _rate );

	/** \brief
	*  ���ص�ǰ���ŵ��ٶ�
	*
	*  
	*/
	double		GetRate();


private:

	//! �Ƿ������ȷ�ĳ�ʼ��MP3.
	bool            m_bCanInitMp3;

	//! ����mp3�ڴ滺����
	PBYTE    m_pbBuffer;
	//! �Ƿ��Ѿ���MP3�ļ�(�������ڴ��)
	bool			m_bFileOpened;

	//! �����Ƿ��Ѿ��������(�Ѿ����������ٴβ�����)
	bool			m_bFinished;

	//! �Ƿ�ѭ������
	bool			m_bRepeat;

	//! �Ƿ�Ϊ����
	bool			m_bMute;

	//! MP3�������ļ���(�ַ�������)
	char			m_chFileName[MAX_PATH];
	//! MP3�������ļ���(16λ Unicode�ַ�������)
	WCHAR			m_wFileName[MAX_PATH];

	//! ��ǰ������
	long int		m_lCurrentVolume;

	//! ��ǰ���������ľ���ֵ
	long int		m_lBalance;

	//! ��ǰ���ŵ��ٶ�
	double			m_dRate;

	//! ���������Ѿ����ŵ�ʱ��(��)
	float                     m_fPlayEleTime;

	//! ��ǰMP3�������ŵ�ʱ�䳤��(����)
	LONGLONG		m_llDuration;

	//! ��ǰ�Ĳ���λ��
	LONGLONG		m_llCurrentPosition;

	//! ����ԭ������ʱֹͣ��λ��
	LONGLONG		m_llStopPosition;

	//! ��Ϊ���õ�ֹͣ��λ��
	double			m_dStopPos;

	//! ÿ�벥�Ŷ���K�ֽ�,һ��Ϊ128,160,256
	DWORD			m_dwKBPerSec;

	//! ����һ��IGraphBuilderָ��
	IGraphBuilder	*m_cGraphBuilder;

	//! ����һ��IMediaControlָ��
	IMediaControl	*m_cMediaControl;

	//! ����һ��IMediaSeekingָ��
	IMediaSeeking	*m_cMediaSeeking;

	//! ����һ��IBasicAudioָ��
	IBasicAudio		*m_cBasicAudio;

	//! ����һ��IMediaPositionָ��
	IMediaPosition	*m_cMediaPosition;


	//! ����һ��IFilterGraphָ��
	IFilterGraph	*m_cFilterGraph;


	//! ��ǰ��Graph�õ���filterָ��
	IBaseFilter*    m_ptrCurrentFilter;

	//! ����һ��IMediaEventָ��
	IMediaEvent*	m_cMediaEvent;

	IVideoWindow*   m_cVideoWindow;

	HWND            m_hWnd;
};

#endif//#endif __MP3_H_