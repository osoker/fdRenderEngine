#ifndef _R4SOUND_H_
#define _R4SOUND_H_


#include "../interface/osInterface.h"
#include "gc_r4Singleton.h"
#include <map>

// �ϲ�ɵ��ڵ�������Χ
#define MIN_VOLUME 0
#define MAX_VOLUME 100

/** \brief
 *  ����ID�Ľṹ						
 * 
 * \param  hash_value�������ļ����ֵ�hashֵ		
 * \param  index����������Ĳ�������������������hashValue��ͬ����ֵҲ����ͬ��
 *                ��ֵ���ظ��ϲ㣬���ڲ���������
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


//! ���������֡��,��֡�����ܲ�̫׼ȷ����Ѹ��
# define  MAX_DELAYCOUNT  120 

//! �����������resource���ݽṹ
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

	//! ֹͣ��ǰ���������š�
	void    stop_sound( void );

	//! ��ǰ�������Ƿ��Ѿ�ֹͣ����,���count������-1,���Ѿ���ʼֹͣ���Ŵ�������
	bool	isVanish()	const { return  ( m_iCount != -1 ); }
	//! �������Ž��������������ʼ.
	void    start_vanish( void )    { m_iCount = MAX_DELAYCOUNT; } 


	//! �Ƿ���UI���� 
	bool					m_bUI;
	
	//! ��ʱɾ���ļ�ʱ����m_iCound == -1,��ʾ�˽ṹ���ڲ�����,�����Ǵӻ������������ģ�����ɾ��
	int						m_iCount;

	//! Bufferָ��
	LPDIRECTSOUNDBUFFER8	m_lpBuffer;

	//! River @ 2009-5-11: ÿһ��soundBuffer��Ҫ�Ĳ���ʱ��
	float                   m_fPlayTime;
};


/** \brief
 *  ʵ�ֶ��������ӿڼ�����.
 *  
 */
class gc_r4SoundMng:public I_soundManager,public ost_Singleton<gc_r4SoundMng>
{
public:
	
	gc_r4SoundMng();
	virtual ~gc_r4SoundMng();
	//! ���_soundID��index = -1 ���ʾ��û����������Ļ��棬����һ���᷵��null��Ҫ������
	void	initSoundResourcePool(const char* _lpszRootpath);
	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static gc_r4SoundMng*    Instance( void );
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void             DInstance( void );



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
	virtual bool   InitSoundManager( HWND _hWnd,DWORD _dwPrimaryFreq,
		                DWORD _dwPrimaryBitRate,os_lisenerParam &_sListenerPara,
		                DWORD _dwCoopLevel=DSSCL_PRIORITY   );

	/** \brief
	 *  �ͷ����������������е�������Դ.
	 *  
	 *  �����������ڲ��������ļ����ڴ�Catch,���ô˺�����ʹ�����е�CatchʧЧ��
	 */
	virtual bool   destory_allsoundResource( void );


	/** \brief
	 *  �ָ�ϵͳ����
	 *  
	 */
	virtual void   ResetSoundVolume( void ) ;

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
	virtual DWORD   PlayFromFile( char *_szFileName, const os_initSoundSrc* _bufferPara,
		                             bool _bLoop = false, bool _bUI = false, float* _fSoundPlayTime = NULL ); 


	/** \brief
	 *  ֹͣ����ĳ����Ч
	 *
	 *  \param   _dwID          ������Ϊ���з��������һ��Ψһ��ʶ
	*/
	virtual void    StopSound( DWORD _dwID );

	/** \brief
	 *  ���÷������������ ÿһ֡��һ�δ���
	 *
	 *  \param   _dwID              ����������Secondory buffer��Ψһ��Ӧ��ʶ  
	 *  
	 *           _vecSoundPos       ���������λ�� 
	 *         
	 *  \return  bool               ���ڲ���״̬true;���򷵻�false
	*/
	virtual bool    FrameMoveSndPos(DWORD _dwID, osVec3D& _vecSoundPos,osVec3D* _velocity = NULL);

	/** \brief
	 *  ���������ߵ�λ��,��Ҫʱ��ʱ����
	 *
	 *  \param   _sListenerParams   ���������ߵĲ���
	 *
	 *  \return  bool               �Ƿ����óɹ�
	 */
	virtual bool    SetListenerPara(os_lisenerParam& _sListenerParams);

	/** \brief
	*   �жϵ�ǰ��Դ�Ƿ����ڲ���
	*
	*    \param _dwID              ��ԴID
	*
	*    \return bool                   �������ŷ���true
	*/
	virtual bool    IsPlayOver( DWORD _dwID );

	/** \brief
	*   ������Ч������
	*
	*    \param		_lVolume     ������Ч������
	*/
	virtual void      SetSoundVolume(  OS_SOUNDTYPE _type, long _lVolume = 75  );

	/** \brief
	*   �����Ч������
	*
	*    \return         long            ��������������
	*/
	virtual long      GetSoundVolume( OS_SOUNDTYPE _type=SOUNDTYPE_3D ){ 
		return (_type==SOUNDTYPE_3D?m_lSoundVolume:m_lUISoundVolume); }

	// ���������ķ�Χ
	virtual void       GetVolumeRange(int* min, int* max ){ *min=MIN_VOLUME; *max=MAX_VOLUME; }


public: 
	/** \brief
	*   ��������֡���ţ�������ϵͳ�ڵ�ÿһ�����������������ɾ����Ӧ��������
	*/
	virtual void    FrameMove();


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
	virtual bool    PlayFileMP3( const char *_FileName,BOOL _repeat = TRUE );

	/** \brief
	 *  ֹͣ����mp3
	 *
	 *  ������������Կ����ڴ沥���ֿ��Կ����ļ�����
     *
	 *  \return    bool   ֹͣʱ�Ƿ�ɹ�
	 */
	virtual bool	StopMP3();

	//@{ River @ 2010-8-20:����õ�mp3����λ�ú�����mp3����λ�õĽӿڡ�
	virtual LONGLONG GetMp3PlayPosition( void );
	virtual void     SetMp3PlayPosition( LONGLONG  _pos );
	//@}


	/** \brief
	 *  ��ͣ����
	 *
	 *  ������������Կ����ڴ沥���ֿ��Կ����ļ�����
	 *
	 *  \return    bool   ��ͣ����
	 */
	virtual bool	PauseMP3( bool _pause = true );

	/** \brief
	 *  ��������,ʹ��һ�ξ���,�ٵ���һ���Ϳɻָ�����
	 */
	virtual void	ToggleMuteMP3( bool _mute = true );


	/** \brief
	*  ��������
	*
	*  \_lVolume �µ�����ֵ(0----100)
	*/
	virtual bool	SetVolumeMP3( long _lVolume );

	/** \brief
	*  ��ñ������ֵ�����
	*
	*  \param    _Volume �µ�����ֵ
	*/
	virtual long       GetVolumeMP3();

	virtual const char* GetCurMp3FileName();
	//}@

protected:
	gcs_sound_buffer*	getSoundBuffer(const gcs_sound_id& _soundID);
	
	/** \brief
	* ���������ļ������ִ���һ������������Ϊ�����仺��
	*
	* \param	_fPlayTime [in,out] ������Ч��Ҫ���ŵ�ʱ��
    */
	gcs_sound_buffer*	createSoundBuffer( const char* _lpszFilename,
		                float* _fPlayTime,bool _bForce = false,bool _bUI = false );

	/** \brief
	*  ��һ���Ѿ����Ź��������������������ã�ʹ�û������ڴ��ڵ�������Դ�����²���
	*
	*  \param _pbuffer  Ҫ�����������������
	*  \param _fPlayTime �ݲ�ʹ�ô˲��������ڿ����������ŵ�ʱ�䳤�̡�
	*/
	gcs_sound_buffer*	resetSoundBuffer( gcs_sound_buffer* _pbuff );
	

	/** \brief
	*	���·���Buffer
	*
	*  ���������lost,��Ҫ���·���
	*
	*  \param   _pDSB           �λ�����
	*  \param   BOOL _pbWasRestored  �Ƿ����·�����buffer
	*/
	HRESULT restoreBuffer(LPDIRECTSOUNDBUFFER8 _pDSB, BOOL* _pbWasRestored);

	HRESULT fillBufferWithSound( BYTE* _pbWave, int _dataSize, 
		LPDIRECTSOUNDBUFFER8 _pDSB, BOOL _bRepeatWavIfBufferLarger);
	 

	//! ����soundBuffer�Ĳ���
	bool           set_bufferPara( const os_initSoundSrc* _bufferPara,LPDIRECTSOUND3DBUFFER _buf );

	/** \brief
	 *  �������������ĺ����������˵�ǰ���ŵ�hash ID.
	 *
	 */
	gcs_sound_id    playSound(const char* _lpszFileName,
		             const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI, float* _fSoundPlayTime=NULL );


	/** \brief
	 *  ������ϵͳ�Ļ����в��ŵ�ǰ�������ļ�
	 *
	 *  ���ϵͳ�Ỻ��洢һЩ������ÿ�β���������ʱ���ȴ�ϵͳ�����в��ң�
	 *  ���Ҫ���ŵ�������������ϵͳ�����У���ʹ�û����е�������
	 */
	bool           playSoundFromPool( gcs_sound_id* _pid,float* _playTime,
		const char* _lpszFileName,const os_initSoundSrc* _bufferPara,bool _bLoop,bool _bUI );


	//! �ӵ�ǰ������������������ɾ��һ������
	void           delete_sound( gcs_sound_id& _id );

	
private:
	//@{
	//  ��ʼ����صĺ�����
	/** \brief
	 *  �������������ĸ�ʽ
	 *
	 *  ���������൱��һ����Ч�����,��Ϸ�����е���Ч�������ﰴ��һ���ķ�ʽ���л��
	 *  Ȼ�����,������Ϸ��ֻ����һ����������������
	 *
	 *  \param   _dwPrimaryFreq     �������ֵĲ���Ƶ��
	 *           _dwPrimaryBitRate  ����λ��bit���
	 *
	 */
	bool    initPrimaryBufferFormat(DWORD _dwPrimaryFreq, DWORD _dwPrimaryBitRate );


	/** \brief�ͷ��������������õ�����Դ
	*/
	void    ReleaseSound();


	/** \brief
	 *  ����������
	 *
	 *  ������Ϸ��ֻ��һ��������,Ҳ������ҽ�ɫ
	 *
	 *  \param   _sListener     ����3D�����ߵĲ���
	 *          
	*/
	void    init3DListener(os_lisenerParam &_sListener);
	//@} 
	//! ÿ�β��Ŷ���m_vDSoundBuffersѡ��һ�����л��棬
	//! ���û�о��ڷǿ�����ѡ��һ�����ܱ��ͷŵģ�Ȼ���ͷŵ�?

	//! ��ǰ���ڲ��ŵ���������
	std::map<gcs_sound_id,gcs_sound_buffer*>	m_mSoundBuffer;

	//! 
	unsigned long			m_lCurMaxSoundIndex;

	//! ��Ч����
	long					m_lSoundVolume;
	// ������Ч����
	long                    m_lUISoundVolume;

	//! �ڲ�catch os_lisenerParam
	os_lisenerParam         m_sCatchLp;

	/** \brief
	 *  DirectSound����
	*/
	LPDIRECTSOUND8           m_lpDirectSound;
	/** \brief
	 * 
	 */
	DSCAPS                   m_deviceCap;
	/*������,����ҽ�ɫ,��Ϸ��Ψһ��һ������,���ж����Դ
	*/
	LPDIRECTSOUND3DLISTENER  m_p3DListener;
	//!�����ߵĲ���
	DS3DLISTENER             m_sListenParams;

	//! mp3��صķ�װ��
	fyc_MP3Lib*              m_sMp3Player;
	//
	bool					 m_bForcePlay;
	
	DWORD                    m_dwUISoundOriVolume;
	std::string				 m_CurMp3FileName;
};
//! ʹ�����޸�dsound�����������������������ͷŻ�������ʱ��δʹ�õ�buffer�����µ�������
#endif//_R4SOUND_H_
