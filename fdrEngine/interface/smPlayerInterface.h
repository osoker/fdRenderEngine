//=======================================================================================================
/**  \file
 *   Filename: smPlayerInterface.h
 *   Desc:     �������õ�������������صĽӿڡ�
 *  
 *   His:      River created @ 2005-8-1
 *
 *   
 *  
 */
//=======================================================================================================
# pragma  once

/** 
 *  ��ʹ��common_mfc�汾��common�⣬��memoryManager.h�ĵ�һ�У���
 *   # define TRACKMEM_ALLOCK  0
 *   ���ֵ����Ϊ0, ����������mfc��ʹ�õ�common����Ⱦ�⣬��Ϊ1,��
 *   ȫ�ֵ�����new��delete. 
 */
# include "../../common/com_include.h"

#pragma comment (lib,"winmm.lib")


#ifdef SMMANAGER_EXPORTS
#define SMMANAGER_API __declspec(dllexport)
#else
#define SMMANAGER_API __declspec(dllimport)
#endif

// ��Ч����
enum OS_SOUNDTYPE{ SOUNDTYPE_UI = 0, SOUNDTYPE_3D };

//! �����߳�ʼ������
struct os_lisenerParam
{
	//!�����ߵĳ�ʼλ��,��ά����ֻ���������,��˲�����D3D��ͬһ������ϵ����
	osVec3D   m_vPosition;
	//!�����ߵ��Ϸ���
	osVec3D   m_vOrientFront;
    //!�����ߵ����泯��
    osVec3D   m_vOrientTop;
	//! ˥������0.0f-----10.0f
	float           m_fRolloff;
	//! ��������
	float           m_fDisFactor;

	//! ȱʡ���캯��
	os_lisenerParam()
	{
		m_fRolloff   = 1.0f;            m_fDisFactor    = 1.0f;
	}
 
	//! ���캯��
	os_lisenerParam(osVec3D& _pos, osVec3D& _front, osVec3D& _top, 
		float _fRolloff=1.0f, float _fDisFactor=1.0f )
	{
		m_vPosition = _pos;			   m_vOrientFront = _front; m_vOrientTop = _top;
		m_fRolloff   = _fRolloff;     m_fDisFactor    = _fDisFactor;
	}

	//! �Ƿ���ͬ�Ĳ����������ͬ���ڲ����账��
	bool operator==( os_lisenerParam& _s );

};

struct os_initSoundSrc
{
	//!�����ߵĳ�ʼλ��
	osVec3D   m_vPosition;
	//!�������,�������Χ�����,�Ҳ���;����Ĭ������Ϊ��λ,
	//�����������˥��������;Ʃ��,minDisΪ2,������Դ���4ʱ,��������������
	float     m_flMinDistance;
	//!������,�������������,�����ﵽ��С,�Ҳ��ټ�С
	float     m_flMaxDistance; 
	//!��Դ������0��100;
	long      m_lVolume;
	//! ��ǰ��Ч�Ĳ���ʱ��,���Ϊ0������ʱ�䣨�ٶȣ�����
	float     m_fPlayTime;

	//! ȱʡ���캯��
	os_initSoundSrc()
	{
		m_fPlayTime = 0.0f;
		m_lVolume = 100;
	}
	//! ���캯��
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
 *  ����������
 *
 *  ���ڳ�ʼ��DirectSound,��DircectSound���и���3D��������
 *  ͬʱҲ����3D��Ч�Ĳ���ֹͣ��
 */ 
struct I_soundManager
{
	/** \brief
	 *  ��ʼ������������DircetSound
	 *
	 *  ���������൱��һ����Ч�����,��Ϸ�����е���Ч�������ﰴ��һ���ķ�ʽ���л��
	 *  Ȼ�����,������Ϸ��ֻ����һ����������������
	 *
	 *  \param     _hWnd            ���ھ��
	 *  \param     _dwCoopLevel     Ӳ��Э���� ��Ϸ��һ��ΪDSSCL_PRIORITY
	 *  \param     _dwPrimaryFreq   �������ֵĲ���Ƶ��,�����������������ĸ�ʽ,һ��Ϊ22050��44100
	 *  \param     _dwPrimaryBitRate����λ��bit���,�����������������ĸ�ʽ    һ��Ϊ8��16
	 *  \param     _sListener       �����ߵĲ���
     *
	 *  \return    bool             �ɹ�����true;���򷵻�false;
	*/
	virtual bool   InitSoundManager(HWND _hWnd,DWORD _dwPrimaryFreq, 	DWORD _dwPrimaryBitRate,
		os_lisenerParam &_sListenerPara,DWORD _dwCoopLevel=DSSCL_PRIORITY)=0;

	/** \brief
	 *  �ͷ����������������е�������Դ.
	 *  
	 */
	virtual bool   destory_allsoundResource( void ) = 0;

	/** \brief
	 *  �ָ�ϵͳ����
	 *  
	 */
	virtual void   ResetSoundVolume( void ) = 0;

	/** \brief
	 *  �Ӵ����ļ�������Ч
	 *
	 *  \param:  _szFileName    Ҫ���ŵ���Ч�ļ�����
     *           _bufferPara    3D buffer���� ���������ĳ����Դ������,���
	 *                          �ǲ��Ŷ�ά�Ľ�������,�˲�������ΪNULL.
	 *           _bLoop         �ǹ�ѭ������  0Ϊ��ѭ��,1Ϊѭ��
	 *           _fSoundPlayTime[in,out] ������Ч�Ĳ���ʱ��
	 *
	 *  \return  DWORD          ����һ����Դ��ΨһID      
	*/
	virtual DWORD   PlayFromFile(char *_szFileName, const os_initSoundSrc* _bufferPara,
		bool _bLoop = false, bool _bUI = false, float* _fSoundPlayTime = NULL )	=	0;  


	/** \brief
	 *  ֹͣ����ĳ����Ч
	 *
	 *  \param   _dwID          ������Ϊ���з��������һ��Ψһ��ʶ
	*/
	virtual void    StopSound(DWORD _dwID)=0;

	/** \brief
	 *  ���÷������������ ÿһ֡��һ�δ���
	 *
	 *  \param   _dwID              ����������Secondory buffer��Ψһ��Ӧ��ʶ  
	 *  
	 *           _vecSoundPos       ���������λ�� 
	 *           _velocity          ����Ϊ�գ���ʾ���������ٶ�
	 *  \return  bool               ���ڲ���״̬true;���򷵻�false
	*/
	virtual bool    FrameMoveSndPos(DWORD _dwID, osVec3D& _vecSoundPos,osVec3D* _velocity = NULL )=0;

	/** \brief
	 *  ���������ߵ�λ��,��Ҫʱ��ʱ����
	 *
	 *  \param   _sListenerParams   ���������ߵĲ���
	 *
	 *  \return  bool               �Ƿ����óɹ�
	 */
	virtual bool    SetListenerPara(os_lisenerParam& _sListenerParams)=0;

	/** \brief
	*   �жϵ�ǰ��Դ�Ƿ����ڲ���
	*
	*    \param _dwID              ��ԴID
	*
	*    \return bool                   �������ŷ���true
	*/
	virtual bool    IsPlayOver(DWORD _dwID) = 0;

	/** \brief
	*   ������Ч������
	*
	*    \param		_lVolume     ������Ч������
	*/
	virtual void      SetSoundVolume( OS_SOUNDTYPE _type, long _lVolume = 75 ) = 0;

	/** \brief
	*   �����Ч������
	*
	*    \return         long            ��������������
	*/
	virtual long          GetSoundVolume( OS_SOUNDTYPE _type ) = 0;

	// ���������ķ�Χ
	virtual void          GetVolumeRange( int* min, int* max ) = 0;	


    //**********************************************************************************
	//***************************MP3�������ֵĲ���**************************************
	//!Attention: ����mp3�Ĳ�������Թ��������г�ʼ��,�����Ҳ����Release
	//**********************************************************************************
	//{@!����Ϊmp3�������ֵĲ���,��ʵ����һЩ��ʽ,Ʃ��wav��
	/** \brief
	 *  ���ļ�����mp3��������
	 *
	 *  \return   bool              ����ֵ�����Ƿ񲥷ųɹ�
	 *  \param const char*          Ҫ���ŵ�mp3�ļ���
	 *  \param BOOL _repeat         �Ƿ�ѭ�����������mp3�ļ���
	 */
	virtual bool    PlayFileMP3(const char *_FileName,BOOL _repeat = TRUE )=0;

	virtual const char* GetCurMp3FileName() = 0;

	/** \brief
	*   ��������֡����
	*/
	virtual void    FrameMove() = 0;

	/** \brief
	 *  ֹͣ����mp3
	 *
	 *  ������������Կ����ڴ沥���ֿ��Կ����ļ�����
     *
	 *  \return    bool   ֹͣʱ�Ƿ�ɹ�
	 */
	virtual bool	StopMP3()=0;

	//@{ River @ 2010-8-20:����õ�mp3����λ�ú�����mp3����λ�õĽӿڡ�
	virtual LONGLONG GetMp3PlayPosition( void ) = 0;
	virtual void     SetMp3PlayPosition( LONGLONG  _pos ) = 0;
	//@}

	/** \brief
	 *  ��ͣ����
	 *
	 *  ������������Կ����ڴ沥���ֿ��Կ����ļ�����
	 *
	 *  \return    bool   ��ͣ����
	 *  \param bool _pause ���Ϊtrue,����ͣ����mp3,���Ϊfalse,�����²���mp3
	 *  
	 */
	virtual bool	PauseMP3( bool _pause = true )=0;

	/** \brief
	 *  mp3�������ֲ��ŵľ�������
	 *
	 *  \param bool _mute Ϊ����ʹ��ǰ��mp3���ž�����Ϊ�����mp3���š�
	 *                    ���muteΪ�棬����PlayFileMP3�²���mp3ʱ���ڲ������κεĴ���
	 */
	virtual void	ToggleMuteMP3( bool _mute = true )=0;


	/** \brief
	*  ��������
	*
	*  \_lVolume �µ�����ֵ(0----100)
	*/
	virtual bool	SetVolumeMP3( long _lVolume )=0;

	/** \brief
	*  ��ñ������ֵ�����
	*
	*  \param    _Volume �µ�����ֵ
	*/
	virtual long       GetVolumeMP3( ) = 0;
	//}@

};

//! ȫ�ֵ��������أ����Դ򿪺͹ر������Ĳ��ţ�Ĭ�ϴ򿪡�
extern SMMANAGER_API BOOL      g_bUseSound;
//! ����ʹ�����������ȷ�������������Ƿ��ʼ�������û�г�ʼ�������ʱ������������ص�����
extern SMMANAGER_API BOOL      g_bUseSoundMgrHaveInit;

//!��ȡȫ�ֵ�����������ָ��
SMMANAGER_API I_soundManager*   get_soundManagerPtr( void );
//! �ͷ�ȫ�ֵ�����������ָ��,�˺���������Զ���ͷŲ���������,��release_renderRes�����л����
SMMANAGER_API void             release_soundManager( void );




