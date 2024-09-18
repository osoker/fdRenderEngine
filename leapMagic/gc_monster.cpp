///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_monster.cpp
 *
 *  Desc:     魔法demo客户端用到的怪物类
 * 
 *  His:      River created @ 2006-5－8
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# include ".\gc_creature.h"
# include "gc_gameScene.h"
# include "lm_configInfoReader.h"
# include "gameApp.h"



//! 怪物每发一个魔法的时间间隔
# define   SKILL_TIME       2.0f
# define   SKILL_DISTANCE   30.0f


gc_monster::gc_monster()
{
	m_fLastSkillTime = 0.0f;

	m_fRunActTime = -1.0f;

	m_szWalkActName = "";

# if __DEPLOY_MONSTERMOD__
	//! 默认情况下，怪物快速移动
	m_fMoveSpeed = 25.0f;
# else
	m_fMoveSpeed = 1.5f;
# endif 
}

gc_monster::~gc_monster()
{

}

//! 在场景内创建一个monster
bool gc_monster::create_monster( lm_command& _command )
{
	guard;

	if( !create_creature( _command ) )
		return false;

	return true;

	unguard;
}


//  
//! 执行指令的接口
BOOL gc_monster::process_command( lm_command& _command )
{
	guard;

	switch( _command.m_eCommandId )
	{
	case LM_MONSTERINSERT:
		return create_monster( _command );
		break;
	case LM_MONSTERACT:
		start_newAction( _command );
		break;
	case LM_MONSTERWALK:
		start_monsterWalk( _command );
		break;
	case LM_MONSTEREQUIP:
		change_euqipment( _command );
		break;
	case LM_MONSTERRUNACTTIME:
		set_monsterWalkActTime( _command );
		break;
	case LM_MONSTERRUNSPEED:
		set_monsterMoveSpeed( _command );
		break;
	case LM_MONSTERSAY:
		start_talkString( _command );
		break;
	case LM_MONSTERHIDE:
		creatureHideProcess( _command );
		break;
	case LM_MONSTERRIDE:
		start_ride( _command );
		break;
	case LM_MONSTERUNRIDE:
		end_ride();
		break;
	case LM_MONSTERDEL:
		reset_creature();
		break;
	default:
		break;
	}

	return TRUE;

	unguard;
}

//! 设置怪物的完整步时间
void gc_monster::set_monsterWalkActTime( lm_command& _command )
{
	guard;

	if( !m_bActive )
		return;

	m_fRunActTime = *(float*)_command.m_ptrCommandContent;
	if( m_fRunActTime < 0.0f )
		m_fRunActTime = -1.0f;


	unguard;
}

//! 设置怪物的步幅，即移动速度
void gc_monster::set_monsterMoveSpeed( lm_command& _command )
{
	guard;

	if( !m_bActive )
		return;
	float   t_fSpeed = *(float*)_command.m_ptrCommandContent;
	float   t_fActTime = m_fRunActTime;
	int     t_iActIdx = -1;

	if( t_fActTime < 0.0f )
	{

		t_fActTime = m_ptrSkinMesh->get_actUsedTime( HERO_RUN,t_iActIdx );
		if( -1 == t_iActIdx )
		{
			t_fActTime = m_ptrSkinMesh->get_actUsedTime( MONSTER_WALK,t_iActIdx );
			if( -1 == t_iActIdx )
				t_fActTime = m_ptrSkinMesh->get_actUsedTime( MONSTER_WALK1,t_iActIdx );
		}
	}

	if( -1 == t_iActIdx )
	{
		t_fActTime = m_ptrSkinMesh->get_actUsedTime( MONSTER_FREE,t_iActIdx );
		osassertex( t_iActIdx != -1,va( "怪物<%s>找不到动作<%s>或是<%s>或<%s>..\n",
						m_szCreDir,HERO_RUN,MONSTER_WALK,MONSTER_WALK1 ) );
	}


	// 怪物的移动速度,以秒计
	m_fMoveSpeed = t_fSpeed / t_fActTime;

	unguard;
}

//! 对于怪物有坐骑时，使用全样的开始移动函数
void gc_monster::start_monsterRideWalk( lm_command& _command )
{
	guard;

	osassert( m_bInRideState );
	osassert( this->m_ptrRides );

	if( !m_bInMoveStatus )
	{
		int    t_iActIdx = 0;

		m_bInMoveStatus = true;


		// 更改人物的动作???
		os_newAction    t_newAct;
		t_newAct.m_iNewAct = -1;

		// 先尝试使用主角人物的跑步动作.
		// 先尝试使用主角人物的跑步动作.
		if( this->m_fEarthOffset > 0.0f )
			m_ptrRides->get_actUsedTime( MONSTER_FLY,t_iActIdx );
		else
			m_ptrRides->get_actUsedTime( MONSTER_FREE,t_iActIdx );
	
		if( t_iActIdx != -1 )
			strcpy( t_newAct.m_strActName,MONSTER_FLY );
		else
		{
			m_ptrRides->get_actUsedTime( MONSTER_FREE,t_iActIdx );
			if( t_iActIdx == -1 )
			{
				osassertex( false,"找不到怪物的待机动作...\n" );
			}
			else
				strcpy( t_newAct.m_strActName,MONSTER_FREE );
		}


		t_newAct.m_changeType = OSE_DIS_OLDACT;		

		// 如果设置过此怪物的移动动作时间，则此处使用
		if( m_fRunActTime >= 0.0f )
			t_newAct.m_fFrameTime = m_fRunActTime;


		m_ptrRides->change_skinMeshAction( t_newAct );
		strcpy( m_szCurActName,t_newAct.m_strActName );

	}
		
	m_vec3DestMovePos = *(osVec3D*)_command.m_ptrCommandContent;
	// 确认向前移动的方向
	m_fMoveDir = gc_creature::get_charNewDir( m_vec3EntPos,m_vec3DestMovePos );

	return;

	unguard;
}


//! 尝试找到一个可用的怪物走路动作.
bool gc_monster::try_monsterWalkAction( os_newAction& _newAct )
{
	guard;

	int    t_iActIdx = 0;

	// 先尝试使用主角人物的跑步动作.
	m_ptrSkinMesh->get_actUsedTime( HERO_RUN,t_iActIdx );
	if( t_iActIdx != -1 )
		strcpy( _newAct.m_strActName,HERO_RUN );
	else
	{
		m_ptrSkinMesh->get_actUsedTime( MONSTER_WALK,t_iActIdx );
		if( -1 != t_iActIdx )
			strcpy( _newAct.m_strActName,MONSTER_WALK );
		else
		{
			m_ptrSkinMesh->get_actUsedTime( MONSTER_WALK1,t_iActIdx );
			if( -1 != t_iActIdx )
				strcpy( _newAct.m_strActName,MONSTER_WALK1 );
		}
	}	

	// 
	// 最后使用休闲状态的动作
	if( t_iActIdx == -1 )
	{
		m_ptrSkinMesh->get_actUsedTime( MONSTER_FREE,t_iActIdx );
		osassertex( t_iActIdx != -1,va( "怪物<%s>找不到动作<%s>或是<%s>或<%s>...\n",
			m_szCreDir,HERO_RUN,MONSTER_WALK,MONSTER_WALK1 ) );
		strcpy( _newAct.m_strActName,MONSTER_FREE );
	}

	return true;

	unguard;
}


//! 开始执行怪物的走路命令
void gc_monster::start_monsterWalk( lm_command& _command )
{
	guard;

	if( !m_bActive )
		return;

	// TEST CODE:
	static bool t_bShadowHide = true;
	gc_gameScene::m_ptrScene->hide_shadow( m_iFakeShadowId,t_bShadowHide );
	t_bShadowHide = !t_bShadowHide;

	// 骑乘状态需要特殊的函数处理　
	if( (!m_bInMoveStatus) && m_bInRideState )
	{
		start_monsterRideWalk( _command );
		return;
	}

	if( !m_bInMoveStatus )
	{
		int    t_iActIdx = 0;

		m_bInMoveStatus = true;


		// 更改人物的动作???
		os_newAction    t_newAct;
		t_newAct.m_iNewAct = -1;

		//! 如果是上层指定的走路动作,则使用上层的动作.
		if( m_szWalkActName == "" )
		{
			try_monsterWalkAction( t_newAct );
		}
		else
		{
			m_ptrSkinMesh->get_actUsedTime( m_szWalkActName.c_str(),t_iActIdx );
			if( t_iActIdx == -1 )
			{
				char   t_sz[128];
				sprintf( t_sz,"找不到怪物<%s>的<%s>动作,使用默认的动作",
					m_szCreDir,m_szWalkActName.c_str() );
				MessageBox( NULL,t_sz,"ERROR",MB_OK );
				try_monsterWalkAction( t_newAct );
			}
			else
				strcpy( t_newAct.m_strActName,m_szWalkActName.c_str() );
		}

		t_newAct.m_changeType = OSE_DIS_OLDACT;		

		// 如果设置过此怪物的移动动作时间，则此处使用
		if( m_fRunActTime >= 0.0f )
			t_newAct.m_fFrameTime = m_fRunActTime;

		m_ptrSkinMesh->change_skinMeshAction( t_newAct );
		strcpy( m_szCurActName,t_newAct.m_strActName );

	}
		
	m_vec3DestMovePos = *(osVec3D*)_command.m_ptrCommandContent;
	// 确认向前移动的方向
	m_fMoveDir = gc_creature::get_charNewDir( m_vec3EntPos,m_vec3DestMovePos );



	return;

	unguard;
}

//! 怪物随机特效的测试
void gc_monster::frame_moveRandEffect( float _etime )
{
	guard;

	// 用于怪物的动画播放或是其它智能相关的功能
	m_fLastSkillTime += _etime;

	if( m_fLastSkillTime > SKILL_TIME )
	{
		// 检测主角人物离怪物的矩离是否在三十米以内，如果在，插入播放火球术
		const osVec3D*   t_ptrHeroPos = NULL;
		osVec3D          t_vec3Vector;

		t_ptrHeroPos = 
			gc_gameScene::m_ptrCreatureMgr->get_heroEntity()->get_entityPosPtr();

		t_vec3Vector = *t_ptrHeroPos - this->m_vec3EntPos;

		if( SKILL_DISTANCE >= osVec3Length( &t_vec3Vector ) )
		{

			lm_skillPlay    t_sSp;
			lm_command      t_cmd;
	
			// 
			// 开始随机魔法的播放
			t_sSp.m_iSkillIndex = osRand()%15;
			if( t_sSp.m_iSkillIndex == 0 )
				t_sSp.m_iSkillIndex = 1;		

			t_sSp.m_ptrStartPos = get_entityPosPtr();
			t_sSp.m_ptrAttackPos = t_ptrHeroPos;
			t_sSp.m_fHeiAdj = 1.0f;

			t_cmd.m_dwCmdContentSize = sizeof( lm_skillPlay );
			t_cmd.m_eCommandId = LM_MESKILLPLAY;
			t_cmd.m_ptrCommandContent = &t_sSp;

			lmc_commandMgr::Instance()->push_command( t_cmd );

			// 调整怪物的面对方向，使用面对主角人物:
			osVec3D   t_vec3OldPos,t_vec3NewPos;
			float     t_fAgl;

			t_vec3OldPos = *t_sSp.m_ptrStartPos;
			t_vec3NewPos = *t_sSp.m_ptrAttackPos;
			t_fAgl = get_charNewDir( t_vec3OldPos,t_vec3NewPos );
			m_ptrSkinMesh->frame_move( t_vec3OldPos,t_fAgl );
		}

		m_fLastSkillTime = 0.0f;
	}

	unguard;
}

//! 设置此怪物走路动作的名字. 
void gc_monster::set_walkActionName( const char* _actName )
{
	guard;

	osassert( _actName );
	osassert( _actName[0] );

	// 
	m_szWalkActName = _actName;

	return;
	unguard;
}
//! 设置此怪物的步幅和步时间
void gc_monster::set_monstWalkInfo( float _stepLength,float _stepTime )
{
	guard;

	// 
	m_fRunActTime = _stepTime;
	m_fMoveSpeed = _stepLength / _stepTime;

	return;

	unguard;
}


//! 释放并重设当前的monster结构
void gc_monster::reset_monster( void )
{
	guard;

	m_fLastSkillTime = 0.0f;

	m_fRunActTime = -1.0f;

	m_fMoveSpeed = 1.0f;

	reset_creature();

# if __DEPLOY_MONSTERMOD__
	//! 刷怪编辑器中,默认情况下，怪物快速移动
	m_fMoveSpeed = 25.0f;
# else
	m_fMoveSpeed = 1.5f;
# endif 

	m_szWalkActName = "";


	unguard;
}


//! 对当前的monster进行FrameMove.
void gc_monster::frame_move( float _etime )
{
	guard;

	if( lm_gvar::g_bRandSkillPlay )
		frame_moveRandEffect( _etime );

	if( !m_bActive )
		return ;

	// 处于骑乘状态的话，需要每一帧得到位置和旋转角度
	if( m_bInRideState )
		m_ptrSkinMesh->get_posAndAgl( m_vec3EntPos,m_fRotAgl );

	// 怪物说话相关
	framemove_talkString( _etime );

	//! 如果怪物创建在还没有调入内存的地图上,此处需要重设一下怪物位置
	if( !m_bRightCrePos )
	{
		m_bRightCrePos = true;
		if( gc_gameScene::m_ptrScene->get_detailPos( m_vec3EntPos,true ) )
			m_ptrSkinMesh->frame_move( m_vec3EntPos,m_fRotAgl );
		else
			m_bRightCrePos = false;
	}


	//! 如果处于移动状态
	if( m_bInMoveStatus && m_bActive )
	{
		osassert( m_ptrSkinMesh );

		osVec3D   t_vec3Dir,t_vec3NewPos;
		float     t_fLength = 0;
		float     t_fMoveLength = 0;

		t_vec3Dir = m_vec3DestMovePos - this->m_vec3EntPos;
		t_vec3Dir.y = 0.0f;
		t_fLength = osVec3Length( &t_vec3Dir );
		osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

		// 暂时使用固定的移动速度
		t_fMoveLength = _etime * m_fMoveSpeed;
		if( (t_fMoveLength > t_fLength) )
		{
			t_vec3NewPos = this->m_vec3DestMovePos;
			m_bInMoveStatus = FALSE;
			start_freeStatus();
		}
		else
		{
			t_vec3Dir *= t_fMoveLength;
			t_vec3NewPos = m_vec3EntPos + t_vec3Dir;
		}
		

		gc_gameScene::m_ptrScene->get_detailPos( t_vec3NewPos,true );
		if( m_bInRideState )
		{
			t_vec3NewPos.y += m_fEarthOffset;
			m_ptrRides->frame_move( t_vec3NewPos,m_fMoveDir );
		}
		else
			m_ptrSkinMesh->frame_move( t_vec3NewPos,m_fMoveDir );
		m_vec3EntPos = t_vec3NewPos;
		m_fRotAgl = m_fMoveDir;

		// 
		// 如果有似阴影:
		if( m_iFakeShadowId >= 0 )
			gc_gameScene::m_ptrScene->set_shadowPos( m_iFakeShadowId,m_vec3EntPos );

	}


	unguard;
}



