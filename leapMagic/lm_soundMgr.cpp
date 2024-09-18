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

//! ��ʼ��ȫ�ֵ�������ؽӿ�,��Ҫ��ini�ļ��ڶ�ȡ��Ӧ������
BOOL lm_soundMgr::init_soundMgr( HWND _hwnd )
{
	guard;

	// ��ʼ��������صĽӿ�
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

//! ÿһ֡������������λ�ô���,��ʹ3d�������ܹ���ȷ�Ĳ���
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
 
	// ����һЩ�����Ĵ����mp3���ŵ�ѭ��
	m_ptrSoundMgr->FrameMove();

	return true;
	unguard;
}

/** \brief
*  ������������Ҫһ��λ�ú�һ���ļ�����
*
*  \param const char* _fname Ҫ���ŵ��������ļ���
*  \param BOOL _bloop        �Ƿ�ѭ�����ŵ�������
*  \param osVec3D* _pos      �����3d�ռ��ڲ������������ֵΪһ��λ��ֵ������
*                            ��������Ϊ������������в��š�
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
