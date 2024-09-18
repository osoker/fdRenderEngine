///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_soundMgr.h
 *
 *  Desc:     ħ��demo�ͻ����õ���������ص���
 * 
 *  His:      River created @ 2006-6-2
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"



/** \brief
 *  ħ��Demo�ڵ��������Ź�����,���ڲ��ſͻ��˳���ͼ�ڹ����������������Ϣ
 *
 *  
 */
class lm_soundMgr
{
private:
	//! 
	I_soundManager*      m_ptrSoundMgr;


public:
	lm_soundMgr(void);
	~lm_soundMgr(void);

	//! ��ʼ��ȫ�ֵ�������ؽӿ�,��Ҫ��ini�ļ��ڶ�ȡ��Ӧ������
	BOOL      init_soundMgr( HWND _hwnd );

	/** \brief 
	 *  ÿһ֡������������λ�ô���,��ʹ3d�������ܹ���ȷ�Ĳ���
	 *
	 *  \param osVec3D& _pos �������ڵ�λ�á�
	 *  \param osVec3D& _ori ���߳���ķ���
	 */
	BOOL      frame_moveListener( osVec3D& _pos,osVec3D& _ori );

	/** \brief
	 *  ������������Ҫһ��λ�ú�һ���ļ�����
	 *
	 *  \param const char* _fname Ҫ���ŵ��������ļ���
	 *  \param BOOL _bloop        �Ƿ�ѭ�����ŵ�������
	 *  \param osVec3D* _pos      �����3d�ռ��ڲ������������ֵΪһ��λ��ֵ������
	 *                            ��������Ϊ������������в��š�
	 */
	BOOL      play_sound( const char* _fname,BOOL _bloop = FALSE,osVec3D* _pos = NULL );   

	void      play_mp3( const char* _name,BOOL _repeat )
	{
		if( m_ptrSoundMgr ) m_ptrSoundMgr->PlayFileMP3( _name,_repeat );
	}


};

