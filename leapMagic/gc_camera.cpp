///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_camera.cpp
 *
 *  Desc:     魔法demo客户端用到的相机类，相机跟其它的entity一样，处理场景内的消息
 * 
 *  His:      River created @ 2006-4-14
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "StdAfx.h"
# include ".\gc_camera.h"
# include "lm_configinforeader.h"


//! 引擎的相机接口
I_camera* gc_camera::m_ptrCamera = NULL;
//! 焦点偏移人物的高度
float gc_camera::m_fFocusOffset = 1.8f;

//! 鼠标旋转的速度调节
float gc_camera::m_fRotSpeed = 0.0025f;

//! 相机的角度限制
float gc_camera::m_fCamPitchLimit = 0.0f;

//! 场景内可编辑相机的最远可见矩离
# define MAX_EDITCAM_DIS   3.0f

//! 表示相机位置的盒子位置
# define CAM_POS_BOXSIZE   0.1f

gc_camera::gc_camera(void)
{

	I_resource*   t_ptrRes = get_resourceMgr();
	osassert( t_ptrRes );
	m_ptrCamera = t_ptrRes->create_camera();
	osassert( m_ptrCamera );


}

gc_camera::~gc_camera(void)
{
}

void gc_camera::process_pitchRot( const lm_command* _cmd )
{
	guard;

	osassert( _cmd );
	osassert( this->m_ptrCamera );

	osVec3D   t_vec3D;

	m_ptrCamera->get_camFocus( t_vec3D );

	if( _cmd->m_eCommandId == LM_CAMERAPITCH )
	{
		m_ptrCamera->rotate_vertical( t_vec3D, *((float*)_cmd->m_ptrCommandContent) );
	}
	else if( _cmd->m_eCommandId == LM_CAMERAROT )
		m_ptrCamera->rotate_horizon( t_vec3D, *((float*)_cmd->m_ptrCommandContent) );  
	else
		osassert( false );

	unguard;
}

void gc_camera::move_closef( const lm_command* _cmd )
{
	guard;

	osassert( _cmd );
	osassert( m_ptrCamera );

	float    t_f = *(float*)_cmd->m_ptrCommandContent;
	float    t_fLength;

	osVec3D   t_vec3Pos,t_vec3Focus;
	m_ptrCamera->get_curpos( &t_vec3Pos );
	m_ptrCamera->get_camFocus( t_vec3Focus );
	t_vec3Pos -= t_vec3Focus;
	t_fLength = osVec3Length( &t_vec3Pos );

	if( (t_fLength + t_f) < m_fMinFocusDis )
		t_f = m_fMinFocusDis - t_fLength;

	// 最远的限制
	if( (t_fLength + t_f) > m_fMaxFocusDis )
		t_f = m_fMaxFocusDis - t_fLength;


	m_ptrCamera->move_close( t_f );

	unguard;
}

//! 拉近放远
void gc_camera::move_close( const lm_command* _cmd )
{
	guard;

	osassert( _cmd );
	osassert( this->m_ptrCamera );

	int      t_iDelta = *(int*)(_cmd->m_ptrCommandContent);
	float    t_f = t_iDelta*m_fToFocusSpeed;
	float    t_fLength;

	osVec3D   t_vec3Pos,t_vec3Focus;
	m_ptrCamera->get_curpos( &t_vec3Pos );
	m_ptrCamera->get_camFocus( t_vec3Focus );
	t_vec3Pos -= t_vec3Focus;
	t_fLength = osVec3Length( &t_vec3Pos );

	if( (t_fLength + t_f) < m_fMinFocusDis )
		t_f = m_fMinFocusDis - t_fLength;

	// 最远的限制
	if( (t_fLength + t_f) > m_fMaxFocusDis )
		t_f = m_fMaxFocusDis - t_fLength;

	m_ptrCamera->move_close( t_f );


	unguard;
}

//! 相机的整体移动，焦点和位置
void gc_camera::move_camera( const lm_command* _cmd )
{
	guard;

	osVec3D    t_vec3NewFocus,t_vec3OldFocus,t_vec3CamPos;

	t_vec3NewFocus = *(osVec3D*)_cmd->m_ptrCommandContent;
	m_ptrCamera->get_camFocus( t_vec3OldFocus );
	m_ptrCamera->get_curpos( &t_vec3CamPos );
	t_vec3OldFocus = t_vec3NewFocus - t_vec3OldFocus;
	t_vec3CamPos += t_vec3OldFocus;

	m_ptrCamera->set_curpos( &t_vec3CamPos );
	m_ptrCamera->set_camFocus( t_vec3NewFocus );

	osVec3D  t_vec3Pos,t_vec3Focus;
	gc_camera::m_ptrCamera->get_curpos( &t_vec3Pos );


	return;

	unguard;
}



//! 初始化并插入场景内相机的消息
void gc_camera::init_camera( const os_cameraInit* _camInit )
{
	guard;

	osassert( m_ptrCamera );

	m_ptrCamera->init_camera( _camInit );

	// 备份相机的初始化信息
	memcpy( &m_sCamInitData,_camInit,sizeof( os_cameraInit ) );


	// 读入其相关的camera配置数据
	g_ptrConfigInfoReader->read_cameraData( this );

	
	if( !float_equal( m_fCamPitchLimit,0.0f ) )
		m_ptrCamera->limit_pitch( OS_PI/2.0f - m_fCamPitchLimit );

	m_ptrCamera->set_maxDistance( m_fMaxFocusDis );

	return;

	unguard;
}

//! 相机往前移动
void gc_camera::move_forword( const lm_command* _cmd )
{
	guard;

	osVec3D    t_vec3Look,t_vec3Focus,t_vec3Pos;
	float      t_fMoveLength = *((float*)_cmd->m_ptrCommandContent);

	if( m_ptrCamera )
	{
		m_ptrCamera->get_godLookVec( t_vec3Look );
		t_vec3Look.y = 0.0f;
		osVec3Normalize( &t_vec3Look,&t_vec3Look );
		t_vec3Look *= t_fMoveLength;

		m_ptrCamera->get_camFocus( t_vec3Focus );
		t_vec3Focus += t_vec3Look;

		m_ptrCamera->get_curpos( &t_vec3Pos );
		t_vec3Pos += t_vec3Look;
		m_ptrCamera->set_curpos( &t_vec3Pos );

		m_ptrCamera->set_camFocus( t_vec3Focus );
	}

	unguard;
}
//! 相机往右移动
void gc_camera::move_right( const lm_command* _cmd )
{
	osVec3D    t_vec3Right,t_vec3Focus,t_vec3Pos;
	float      t_fMoveLength = *((float*)_cmd->m_ptrCommandContent);

	if( m_ptrCamera )
	{
		m_ptrCamera->get_rightvec( t_vec3Right );
		t_vec3Right *= t_fMoveLength;
		m_ptrCamera->get_camFocus( t_vec3Focus );
		t_vec3Focus += t_vec3Right;

		m_ptrCamera->get_curpos( &t_vec3Pos );
		t_vec3Pos += t_vec3Right;
		m_ptrCamera->set_curpos( &t_vec3Pos );

		m_ptrCamera->set_camFocus( t_vec3Focus );

	}
}

//! 相机的焦点往上移动
void gc_camera::move_up( const lm_command* _cmd )
{
	osVec3D    t_vec3Look,t_vec3Focus,t_vec3Pos;
	float      t_fMoveLength = *((float*)_cmd->m_ptrCommandContent);

	if( m_ptrCamera )
	{
		t_vec3Look = osVec3D( 0.0f,1.0f,0.0f );
		t_vec3Look *= t_fMoveLength;
		m_ptrCamera->get_camFocus( t_vec3Focus );
		t_vec3Focus += t_vec3Look;

		m_ptrCamera->get_curpos( &t_vec3Pos );
		t_vec3Pos += t_vec3Look;
		m_ptrCamera->set_curpos( &t_vec3Pos );

		m_ptrCamera->set_camFocus( t_vec3Focus );
	}
}


//! 存储相机
void gc_camera::save_camera( const lm_command* _cmd )
{
	guard;

	lm_cameraSaveLoad*   t_ptrCam = (lm_cameraSaveLoad*)_cmd->m_ptrCommandContent;

# if __EFFECT_EDITOR__
	
	osassert( t_ptrCam->m_szCameraFile[0] );
	m_ptrCamera->save_camera( t_ptrCam->m_szCameraFile );
	MessageBox( NULL,"存储相机文件成功","success",MB_OK );

# else
	MessageBox( NULL,"不能存储相机","INFO",MB_OK );
# endif

	unguard;
}

//! 调入相机.
void gc_camera::load_camera( const lm_command* _cmd )
{
	guard;

	lm_cameraSaveLoad*   t_ptrCam = (lm_cameraSaveLoad*)_cmd->m_ptrCommandContent;;

# if __EFFECT_EDITOR__

	osassert( t_ptrCam->m_szCameraFile[0] );
	m_ptrCamera->load_camera( t_ptrCam->m_szCameraFile,true,t_ptrCam->m_fCamAniTime );

# else
	MessageBox( NULL,"不能存储相机","INFO",MB_OK );
# endif

	unguard;
}



//! 执行指令的接口
BOOL gc_camera::process_command( lm_command& _command )
{
	guard;
	
	switch( _command.m_eCommandId )
	{
	case LM_CAMERAPITCH:
	case LM_CAMERAROT:
		process_pitchRot( &_command );
		break;
	case LM_MOVECLOSE:
		move_close( &_command );
		break;
	case LM_MOVECLOSEF:
		move_closef( &_command );
		break;
	case LM_CAMMOVE:
		move_camera( &_command );
		break;
	case LM_MOVEFORWORD:
		move_forword( &_command );
		break;
	case LM_MOVERIGHT:
		move_right( &_command );
		break;
	case LM_MOVEUP:
		move_up( &_command );
		break;
	case LM_SAVECAMERA:
		save_camera( &_command );
		break;
	case LM_LOADCAMERA:
		load_camera( &_command );
		break;
	default:
		break;
	}

	return true;
	unguard;
}


//! 根据三个向量，画bbox
void gc_camera::draw_bbox( I_fdScene* _scene,osVec3D& _pos,osVec3D& _lookvec,osVec3D& _rvec )
{
	guard;

	//CAM_POS_BOXSIZE
	osVec3D   t_vec3Up;
	osVec3D   t_vec3Pt[8];
	osVec3D   t_vec3Draw[2];

	osVec3Cross( &t_vec3Up,&_lookvec,&_rvec );

	for( int t_i=0;t_i<8;t_i ++ )
		t_vec3Pt[t_i] = _pos;

	// 
	for( int t_i=0;t_i<4;t_i ++ )
		t_vec3Pt[t_i] -= (_lookvec*CAM_POS_BOXSIZE);
	for( int t_i=4;t_i<8;t_i ++ )
		t_vec3Pt[t_i] += (_lookvec*CAM_POS_BOXSIZE);

	t_vec3Pt[0] -= (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[0] -= (_rvec*CAM_POS_BOXSIZE);

	t_vec3Pt[1] -= (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[1] += (_rvec*CAM_POS_BOXSIZE);

	t_vec3Pt[2] += (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[2] -= (_rvec*CAM_POS_BOXSIZE);

	t_vec3Pt[3] += (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[3] +=  (_rvec*CAM_POS_BOXSIZE);

	t_vec3Pt[4] -= (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[4] -= (_rvec*CAM_POS_BOXSIZE);
	
	t_vec3Pt[5] -= (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[5] += (_rvec*CAM_POS_BOXSIZE);
	
	t_vec3Pt[6] += (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[6] -= (_rvec*CAM_POS_BOXSIZE);
	
	t_vec3Pt[7] += (t_vec3Up*CAM_POS_BOXSIZE);
	t_vec3Pt[7] +=  (_rvec*CAM_POS_BOXSIZE);

	// 开始画
	t_vec3Draw[0] = t_vec3Pt[0];
	t_vec3Draw[1] = t_vec3Pt[1];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[0];
	t_vec3Draw[1] = t_vec3Pt[2];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[2];
	t_vec3Draw[1] = t_vec3Pt[3];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[3];
	t_vec3Draw[1] = t_vec3Pt[1];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );


	t_vec3Draw[0] = t_vec3Pt[0+4];
	t_vec3Draw[1] = t_vec3Pt[1+4];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[0+4];
	t_vec3Draw[1] = t_vec3Pt[2+4];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[2+4];
	t_vec3Draw[1] = t_vec3Pt[3+4];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[3+4];
	t_vec3Draw[1] = t_vec3Pt[1+4];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[0];
	t_vec3Draw[1] = t_vec3Pt[4];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[1];
	t_vec3Draw[1] = t_vec3Pt[5];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[2];
	t_vec3Draw[1] = t_vec3Pt[6];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );

	t_vec3Draw[0] = t_vec3Pt[3];
	t_vec3Draw[1] = t_vec3Pt[7];
	_scene->draw_line( t_vec3Draw,2,0xffffff00,1 );


	return;

	unguard;
}


//! 画出相机的位置和vf
void gc_camera::draw_camera( I_camera* _cam,I_fdScene* _scene )
{
	guard;

	osassert( _cam );
	osassert( _scene );

	const osVec3D*   t_vec3FrusPt;

	t_vec3FrusPt = _cam->get_vfPt();

	osVec3D   t_vec3Pt[2];

	t_vec3Pt[0] = t_vec3FrusPt[0];
	t_vec3Pt[1] = t_vec3FrusPt[1];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[0];
	t_vec3Pt[1] = t_vec3FrusPt[2];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[2];
	t_vec3Pt[1] = t_vec3FrusPt[3];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[3];
	t_vec3Pt[1] = t_vec3FrusPt[1];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	// 画远方的四条线
	t_vec3Pt[0] = t_vec3FrusPt[4];
	t_vec3Pt[1] = t_vec3FrusPt[5];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[4];
	t_vec3Pt[1] = t_vec3FrusPt[6];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[6];
	t_vec3Pt[1] = t_vec3FrusPt[7];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[7];
	t_vec3Pt[1] = t_vec3FrusPt[5];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[4];
	t_vec3Pt[1] = t_vec3FrusPt[7];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	//
	t_vec3Pt[0] = t_vec3FrusPt[0];
	t_vec3Pt[1] = t_vec3FrusPt[4];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[1];
	t_vec3Pt[1] = t_vec3FrusPt[5];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[2];
	t_vec3Pt[1] = t_vec3FrusPt[6];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );

	t_vec3Pt[0] = t_vec3FrusPt[3];
	t_vec3Pt[1] = t_vec3FrusPt[7];
	_scene->draw_line( t_vec3Pt,2,0xffffffff,2 );


# if 0
	osVec3D   t_vec3Pos,t_vec3LookVec;
	osVec3D   t_vec3R;

	_cam->get_curpos( &t_vec3Pos );
	_cam->get_godLookVec( t_vec3LookVec );	
	_cam->get_rightvec( t_vec3R );

	osDebugOut( "The pos is:<%f,%f,%f>..\n",
		t_vec3Pos.x,t_vec3Pos.y,t_vec3Pos.z );

	draw_bbox( _scene,t_vec3Pos,t_vec3LookVec,t_vec3R );
	t_vec3Pos += ( t_vec3LookVec*MAX_EDITCAM_DIS );
	draw_bbox( _scene,t_vec3Pos,t_vec3LookVec,t_vec3R );
# endif 
	unguard;
}


//! 快照当前相机的信息,返回一个跟当前的相机一样的相机指针。
I_camera* gc_camera::snapshot_curCamera( void )
{
	guard;

	I_camera* t_ptrCam = ::get_resourceMgr()->create_camera();

	osVec3D  t_vec3Focus,t_vec3Pos;

	m_ptrCamera->get_camFocus( t_vec3Focus );
	m_ptrCamera->get_curpos( &t_vec3Pos );

	m_sCamInitData.farp = MAX_EDITCAM_DIS;
	t_ptrCam->init_camera( &m_sCamInitData );

	t_ptrCam->set_camFocus( t_vec3Focus );
	t_ptrCam->set_curpos( &t_vec3Pos );


	return t_ptrCam;

	unguard;
}

gc_smoothMotion::gc_smoothMotion()
{
	reset_smoothMotion();
}

//! 重设smoothMotion结构
void gc_smoothMotion::reset_smoothMotion( void )
{
	m_bFirstFull = false;
	m_iStartPt = 0;
	m_iKeyPtEnd = 0;
	m_fDeferTime = 0.0f;

	m_iMaxPtNum = 0;
	m_vec3LastFramePt = osVec3D( 0.0f,0.0f,0.0f );
}


/** \brief
*  动画后延的时间,按秒计.
*
*  \param _time 延后的时间.
*  \param _maxPtNum 最多缓存的关键顶点数目.
*/
void gc_smoothMotion::set_deferTime( float _time,int _maxPtNum )
{
	guard;

	m_fDeferTime = _time;
	m_iMaxPtNum = _maxPtNum;
	m_vecKeyPt.resize( m_iMaxPtNum );

	unguard;
}


//! 得到下一个Pt的索引.
int gc_smoothMotion::get_nextPtIdx( int _idx )
{
	if( _idx == (m_iMaxPtNum-1) )
		return 0;
	else
		return _idx + 1;
}

//! 得到上一个Pt的索引.
int gc_smoothMotion::get_previousPtIdx( int _idx )
{
	if( (_idx == 0)&&(!m_bFirstFull) )
		return -1;
	else
		return _idx -1;
}


//! 得到当前帧可以使用的平滑动画点.
BOOL gc_smoothMotion::get_smoothPt( osVec3D& _resPt,float _curTime )
{
	guard;

	int   t_i,t_iPreIdx,t_iPtNum;
	float t_fLerp;

	t_iPtNum = m_iMaxPtNum;
	for( t_i=m_iStartPt;t_i<=m_iMaxPtNum;t_i ++ )
	{
		if( m_iKeyPtEnd > m_iStartPt )
		{
			if( t_i > m_iKeyPtEnd )
				return FALSE;
		}
		else
		{
			if( t_i == m_iMaxPtNum )
			{
				t_i = 0;
				t_iPtNum = m_iKeyPtEnd;
			}
		}


		// 
		if( m_vecKeyPt[t_i].m_fTime < _curTime  )
			continue;

		// 等待,无法移动
		t_iPreIdx = get_previousPtIdx( t_i );
		if( t_iPreIdx == -1 )
			return FALSE;

		// 
		if( m_vecKeyPt[t_iPreIdx].m_fTime > _curTime )
			return FALSE;

		//! 在两个顶点之间进行插值
		t_fLerp = m_vecKeyPt[t_i].m_fTime - m_vecKeyPt[t_iPreIdx].m_fTime;
		t_fLerp = (_curTime - m_vecKeyPt[t_iPreIdx].m_fTime)/t_fLerp;

		osVec3Lerp( &_resPt,&m_vecKeyPt[t_i].m_vec3Pt,
			&m_vecKeyPt[t_iPreIdx].m_vec3Pt,t_fLerp );

		break;
	}

	return TRUE;

	unguard;
}

