///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_soundMgr.h
 *
 *  Desc:     魔法demo客户端用到的声音相关的类
 * 
 *  His:      River created @ 2006-6-2
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"



/** \brief
 *  魔法Demo内的声音播放管理器,用于播放客户端除地图内固有声音外的声音信息
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

	//! 初始化全局的声音相关接口,需要从ini文件内读取相应的配置
	BOOL      init_soundMgr( HWND _hwnd );

	/** \brief 
	 *  每一帧对声音的听者位置处理,以使3d的声音能够正确的播放
	 *
	 *  \param osVec3D& _pos 听者所在的位置。
	 *  \param osVec3D& _ori 听者朝向的方向。
	 */
	BOOL      frame_moveListener( osVec3D& _pos,osVec3D& _ori );

	/** \brief
	 *  播放声音，需要一个位置和一个文件名。
	 *
	 *  \param const char* _fname 要播放的声音的文件名
	 *  \param BOOL _bloop        是否循环播放的声音。
	 *  \param osVec3D* _pos      如果在3d空间内播放声音，则此值为一个位置值，否则
	 *                            此声音做为界面的声音进行播放。
	 */
	BOOL      play_sound( const char* _fname,BOOL _bloop = FALSE,osVec3D* _pos = NULL );   

	void      play_mp3( const char* _name,BOOL _repeat )
	{
		if( m_ptrSoundMgr ) m_ptrSoundMgr->PlayFileMP3( _name,_repeat );
	}


};

