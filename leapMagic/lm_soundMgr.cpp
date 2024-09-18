#include ".\lm_soundmgr.h"

lm_soundMgr::lm_soundMgr(void)
{
	m_ptrSoundMgr = NULL;
}

lm_soundMgr::~lm_soundMgr(void)
{
	if( m_ptrSoundMgr )
		release_soundManager();
}

//! 初始化全局的声音相关接口,需要从ini文件内读取相应的配置
BOOL lm_soundMgr::init_soundMgr( HWND _hwnd )
{
	guard;

	// 初始化声音相关的接口
	os_lisenerParam    t_lp;
	t_lp.m_fDisFactor = 1.0f;
	t_lp.m_vOrientFront = osVec3D( 0.0f,0.0f,1.0f );
	t_lp.m_vOrientTop = osVec3D( 0.0f,1.0f,0.0f );
	t_lp.m_vPosition = osVec3D( 0.0f,0.0f,0.0f );

	m_ptrSoundMgr = ::get_soundManagerPtr();
	if( !m_ptrSoundMgr->InitSoundManager( _hwnd,44100,16,t_lp ) )
	{
		g_bUseSound = FALSE;
		return FALSE;
	}

	int t_iMin,t_iMax;

	m_ptrSoundMgr->GetVolumeRange( &t_iMin,&t_iMax );
	int t_iVol = m_ptrSoundMgr->GetSoundVolume( SOUNDTYPE_3D );
	m_ptrSoundMgr->SetSoundVolume( SOUNDTYPE_3D,t_iMax-1 );


	return TRUE;

	unguard;
}

//! 每一帧对声音的听者位置处理,以使3d的声音能够正确的播放
BOOL lm_soundMgr::frame_moveListener( osVec3D& _pos,osVec3D& _ori )
{
	guard;

	os_lisenerParam    t_lp;

	t_lp.m_vPosition = _pos;
	t_lp.m_vOrientFront = _ori;
	t_lp.m_vOrientTop = osVec3D( 0.0f,1.0f,0.0f );
	t_lp.m_fDisFactor = 1.0f;

	// 
	if( m_ptrSoundMgr )
		m_ptrSoundMgr->SetListenerPara( t_lp );
 
	// 用于一些声音的处理和mp3播放的循环
	m_ptrSoundMgr->FrameMove();

	return true;
	unguard;
}

/** \brief
*  播放声音，需要一个位置和一个文件名。
*
*  \param const char* _fname 要播放的声音的文件名
*  \param BOOL _bloop        是否循环播放的声音。
*  \param osVec3D* _pos      如果在3d空间内播放声音，则此值为一个位置值，否则
*                            此声音做为界面的声音进行播放。
*/
BOOL lm_soundMgr::play_sound( const char* _fname,
							 BOOL _bloop/* = FALSE*/,osVec3D* _pos/* = NULL*/ )
{
	guard;

	osassert( _fname );
	osassert( _fname[0] );

	if( !m_ptrSoundMgr )
		return true;

	os_initSoundSrc    t_src;


	if( _pos )
	{
		t_src.m_flMaxDistance = 50.0f;
		t_src.m_flMinDistance = 15.0f;
		t_src.m_lVolume = 100;
		t_src.m_vPosition = *_pos;
	}


	if( _pos )
		m_ptrSoundMgr->PlayFromFile( (char*)_fname,&t_src,_bloop,FALSE );
	else
		m_ptrSoundMgr->PlayFromFile( (char*)_fname,NULL,_bloop,TRUE );


	return FALSE;

	unguard;
}
