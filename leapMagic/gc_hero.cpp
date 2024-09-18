///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_hero.cpp
 *
 *  Desc:     魔法demo客户端用到的主物人物显示类
 * 
 *  His:      River created @ 2006-5－8
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# include ".\gc_creature.h"
# include "gc_gameScene.h"
# include "lm_configInfoReader.h"
# include "gameApp.h"
# include "gc_camera.h"
# include "lm_soundMgr.h"
# include "gc_charSound.h"
//# include "gc_chaEffect.h"
# include "gc_creature.h"


# define  RUN_ACTNAME   "跑步"

# define  MONST_FREEACT1 "待机1"

//! 主角人物的路步速度，用于调试方便
float gc_hero::m_fRunSpeed = 1.0f;



gc_hero::gc_hero()
{
	strcpy(m_sFootWaterInfo.m_szFootWaterIdleSpe,"");
	//strcpy(m_sFootWaterInfo.m_szFootWaterIdleSpe,"speff/S_GHliliang_.spe");
	m_sFootWaterInfo.m_playTime = 0.1f;
	mOldPos.x =mOldPos.y =mOldPos.z = 0.0f;


}

gc_hero::~gc_hero()
{
	
}

//! 执行指令的接口
BOOL gc_hero::process_command( lm_command& _command )
{
	guard;

	osColor t_sColor( 1.0f,1.0f,1.0f,1.0f );

	switch( _command.m_eCommandId )
	{
	case LM_HEROINSERT:
		this->create_creature( _command );
		break;
	case LM_HEROSAY:
		start_talkString( _command );
		break;
	case LM_HEROWALK:
		start_heroWalk( _command );
		break;
	case LM_HEROATTACK:
		start_attack( _command );
		break;

		// 改变主角人物的动作
	case LM_HERONEWACTION:
		start_newAction( _command );
		break;

		//! 
	case LM_HEROEQUIPCHANGE:
		{
			change_euqipment( _command );	
			//m_ptrSkinMesh->displaytri_bysel( 2,t_k,3 );
			m_ptrSkinMesh->set_maxBright( t_sColor );
			break;
		}

	case LM_HEROHIDE:
		creatureHideProcess( _command );
		break;
	case LM_HERORIDE:
		start_ride( _command );
		break;
	case LM_HEROUNRIDE:
		end_ride();
		update_camera();
		break;
	default:
		break;
	}
	return true;

	unguard;
}
//! 更新相机 即时更新相机，不然会有顿的现象出现
void gc_hero::update_camera( void )
{
	guard;

	if( !lm_gvar::g_bCamFollowHero )
		return;

	lm_command   t_cmd;
	osVec3D      t_vec3CamFocus;

	t_vec3CamFocus = m_vec3EntPos;
	t_vec3CamFocus.y += gc_camera::m_fFocusOffset;

	t_cmd.m_eCommandId = LM_CAMMOVE;
	t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
	t_cmd.m_ptrCommandContent = &t_vec3CamFocus;
	lmc_commandMgr::Instance()->do_commandImm( t_cmd );

	return;

	unguard;
}

//! 对于怪物有坐骑时，使用全样的开始移动函数
void gc_hero::start_monsterRideWalk( lm_command& _command )
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
		if( this->m_fEarthOffset > 0.0f )
			m_ptrRides->get_actUsedTime( MONSTER_FLY,t_iActIdx );
		else
			m_ptrRides->get_actUsedTime( MONSTER_FREE,t_iActIdx );
	
		if( t_iActIdx != -1 )
			strcpy( t_newAct.m_strActName,MONSTER_FLY );
		else
		{
			// 尝试跑步的动作
			m_ptrRides->get_actUsedTime( MONSTER_RUN,t_iActIdx );
			if( t_iActIdx != -1 )
			{
				strcpy( t_newAct.m_strActName,MONSTER_RUN );
			}
			else
			{
				// 最后渲染待机动作
				m_ptrRides->get_actUsedTime( MONSTER_FREE,t_iActIdx );
				if( t_iActIdx == -1 )
				{
					osassertex( false,"找不到怪物的待机动作...\n" );
				}
				else
					strcpy( t_newAct.m_strActName,MONSTER_FREE );
			}
		}

		t_newAct.m_changeType = OSE_DIS_OLDACT;		

		m_ptrRides->change_skinMeshAction( t_newAct );
		strcpy( m_szCurActName,t_newAct.m_strActName );

	}
		
	m_vec3DestMovePos = *(osVec3D*)_command.m_ptrCommandContent;
	// 确认向前移动的方向
	m_fMoveDir = gc_creature::get_charNewDir( m_vec3EntPos,m_vec3DestMovePos );

	return;

	unguard;
}



//! 开始主角人物走路状态
void gc_hero::start_heroWalk( lm_command& _command )
{
	guard;

	if( !m_bActive )
		return;

	// 骑乘状态需要特殊的函数处理　
	if( (!m_bInMoveStatus) && m_bInRideState )
	{
		start_monsterRideWalk( _command );
		return;
	}

	if( !m_bInMoveStatus )
	{

		m_bInMoveStatus = true;
		// 更改人物的动作???
		os_newAction    t_newAct;
		t_newAct.m_iNewAct = 2;
		strcpy( t_newAct.m_strActName,"跑步" );
		t_newAct.m_changeType = OSE_DIS_OLDACT;
		t_newAct.m_fFrameTime = 1.0f;
		

		// WORK START: 找到两个动作的不同之处?????
		m_ptrSkinMesh->change_skinMeshAction( t_newAct );

		m_bMark1 = false;
		m_bMark1 = false;
		m_fLastPosePercent = 0.0f;

		m_szCurActName[0] = NULL;

	}
		
	m_vec3DestMovePos = *(osVec3D*)_command.m_ptrCommandContent;

	// 确认向前移动的方向
	m_fMoveDir = gc_creature::get_charNewDir( m_vec3EntPos,m_vec3DestMovePos );


	return;

	unguard;
}




//　由Console设置一个动作，然后调用一个按键后播放。
//  命令行式：caction 0-2 动作名字
//! 开始主角人物新的动作
void gc_hero::start_newAction( lm_command& _command )
{
	guard;

	lm_creatureActChange*   t_ptrActC = (lm_creatureActChange*)_command.m_ptrCommandContent;
	os_newAction        t_newAct;

	// 
	if( t_ptrActC->m_bUseAgl )
	{
		m_ptrSkinMesh->frame_move( m_vec3EntPos,t_ptrActC->m_fRotAgl );
		m_fRotAgl = t_ptrActC->m_fRotAgl;
	}

	t_newAct.m_iNewAct = -1;
	strcpy( t_newAct.m_strActName,t_ptrActC->m_szNewActName );

	if( t_ptrActC->m_iPlayActType == 0 )
		t_newAct.m_changeType = OSE_DIS_OLDACT;
	else if( t_ptrActC->m_iPlayActType == 1 )
	{
		t_newAct.m_changeType = OSE_DIS_NEWACT_TIMES;
		t_newAct.m_iPlayNum = 1 ;
		// WORK START:
		//strcpy( t_newAct.m_strNextActName,"单手战斗待机" );
	}
	else 
	{
		t_newAct.m_changeType = OSE_DIS_NEWACT_LASTFRAME;
		t_newAct.m_iPlayNum = 1;
	}


	strcpy( m_szCurActName,t_ptrActC->m_szNewActName );
	m_bActSndHavePlayed = FALSE;


	// 如果人物动作列表内没有此动作，则使用默认的跑步动作
	// 如果人物此时下处于骑马的状态，则使用坐骑的动作播放

	int     t_iActIdx = 0;
	float   t_fActtime = 1.0f;

	if( m_bInRideState )
	{
		osassert( m_ptrRides );
		t_fActtime = m_ptrRides->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
		if( t_iActIdx < 0 )
		{
			strcpy( t_newAct.m_strActName,MONST_FREEACT1 );
			t_fActtime = m_ptrRides->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
			osassert( t_iActIdx >= 0 );
		}

		t_newAct.m_fFrameTime = t_ptrActC->m_fPlayActScale*t_fActtime;
		m_ptrRides->change_skinMeshAction( t_newAct );
	}
	else
	{
		t_fActtime = m_ptrSkinMesh->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
		if( t_iActIdx < 0 )
		{
			strcpy( t_newAct.m_strActName,RUN_ACTNAME );
			t_fActtime = m_ptrSkinMesh->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
			//osassert( t_iActIdx >= 0 );
			if( t_iActIdx < 0 ) t_iActIdx = 0;
		}

		t_newAct.m_fFrameTime = t_ptrActC->m_fPlayActScale*t_fActtime;
		m_ptrSkinMesh->change_skinMeshAction( t_newAct );
	}

	unguard;
}


//! 开始人物的攻击动作
void gc_hero::start_attack( lm_command& _command )
{
	guard;

	if( !m_bActive )
		return;

	os_newAction    t_newAct;

	t_newAct.m_iNewAct = 7;
	t_newAct.m_strActName[0] = NULL;
	t_newAct.m_changeType = OSE_DIS_OLDACT;
	m_ptrSkinMesh->change_skinMeshAction( t_newAct );
	m_szCurActName[0] = NULL;
	

	unguard;
}



//! 创建主角人物
BOOL gc_hero::create_creature( lm_command& _command )
{
	guard;
	os_skinMeshInit     t_sSmInit;
	lm_heroInsert*      t_ptrHeroInsert;
	
	osassert( gc_gameScene::m_ptrScene );

	// 读入主角人物的配置信息
	m_fMoveSpeed = g_ptrConfigInfoReader->read_heroWalkSpeed();
	m_fRunSpeed = m_fMoveSpeed;

	g_ptrConfigInfoReader->read_heroFootMarkInfo( &m_sFootMarkInfo );
	m_sFMInit.m_fSize = m_sFootMarkInfo.m_fMarkSize;
	strcpy( m_sFMInit.m_szDecalTex,m_sFootMarkInfo.m_szFootMarkTex );
	m_sFMInit.m_fVanishTime = m_sFootMarkInfo.m_fMarkVanishTime;


	t_ptrHeroInsert = (lm_heroInsert*)_command.m_ptrCommandContent;

	// 先读入bfs文件
	strcpy( t_sSmInit.m_strSMDir,t_ptrHeroInsert->m_szChrDir );
	if( !read_bfsInfo( t_sSmInit ) )
		return false;
	t_sSmInit.m_bShadowSupport = true;
	strcpy( t_sSmInit.m_strSMDir,t_ptrHeroInsert->m_szChrDir );


	// 如果已经创建了人物，则在已存在的人物位置创建新的人物
	if( m_ptrSkinMesh )
	{
		float     t_f;

		m_ptrSkinMesh->get_posAndAgl( t_sSmInit.m_vec3Pos,t_f );
		m_ptrSkinMesh->release_character();
		g_pCharSoundMgr->unregister_skinMesh(m_ptrSkinMesh);
//		g_pCharEffectMgr->unregister_skinMesh(m_ptrSkinMesh);
		m_ptrSkinMesh = NULL ;
	} 
	else
	{
		// 人物的三维位置
		gc_gameScene::m_ptrScene->get_tilePos( t_ptrHeroInsert->m_iXPos,
			t_ptrHeroInsert->m_iZPos,t_sSmInit.m_vec3Pos );
	}

	m_fRotAgl = t_sSmInit.m_fAngle;
	m_ptrSkinMesh = gc_gameScene::m_ptrScene->create_character( &t_sSmInit,false,true );

	//  
	//! TEST CODE：测试屏幕渲染
	osVec3D    t_vec3BAck = t_sSmInit.m_vec3Pos;
	t_sSmInit.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
	t_sSmInit.m_bShadowSupport = false;
	t_sSmInit.m_bUseInInterface = true;
	t_sSmInit.m_bPlayWeaponEffect = true;
	gc_gameScene::m_ptrHeroSkinObj = gc_gameScene::m_ptrScene->create_character( &t_sSmInit,false,true );
	gc_gameScene::m_ptrHeroSkinObj->set_fadeState( false );
	gc_gameScene::m_ptrHeroSkinObj->playSwordGlossy( false,true );

	// 
	// 创建第二个测试
	gc_gameScene::m_ptrHeroSkinObj2 = gc_gameScene::m_ptrScene->create_character( &t_sSmInit,false,true );
	gc_gameScene::m_ptrHeroSkinObj2->set_fadeState( false );
	gc_gameScene::m_ptrHeroSkinObj2->playSwordGlossy( false,true );




	osassert( m_ptrSkinMesh );

	t_sSmInit.m_vec3Pos = t_vec3BAck;

	g_pCharSoundMgr->register_skinMesh(m_ptrSkinMesh,t_sSmInit.m_strSMDir);

	//! 主角人物使用刀光特效
	//m_ptrSkinMesh->playSwordGlossy( false,true );

	//m_ptrSkinMesh->playSwordGlossy( true,false );

	// 读入声音相关的信息,从声音信息中，得到更准备的人物跑步声音的信息
	get_fmInfoFromSndInfo( t_sSmInit.m_strSMDir );

	m_vec3EntPos = t_sSmInit.m_vec3Pos;

	m_bActive = TRUE;

	// 发送相机焦点变化的消息,在相机的内部实现中，位置随着焦点的变化而变化
	update_camera();

	/*os_dlightStruct dlight;
	dlight.m_fRadius = 10.0f;
	dlight.m_lightColor.r = 1.0f;
	dlight.m_lightColor.g = 0.0f;
	dlight.m_lightColor.b = 0.0f;
	dlight.m_ltype = OSE_DLIGHT_POINT;
	dlight.m_vec3Pos = m_vec3EntPos;
	m_dynamiclight = gc_gameScene::m_ptrScene->create_dynamicLight(dlight);*/

	while( !m_ptrSkinMesh->legal_operatePtr() )
		::Sleep( 10 );
	m_ptrSkinMesh->set_fadeState( false );
	//m_ptrSkinMesh->set_fakeGlow( true,0xffffffff );

	//float t_f = m_ptrSkinMesh->get_actionTriggerPoint( "法术攻击" );


	gc_gameScene::m_ptrScene->push_behindWallRenderChar( m_ptrSkinMesh );

	return true;

	unguard;
}

void gc_hero::get_fmInfoFromSndInfo( const char* _chrDir )
{
	guard;

	gcs_ACTSoundData*  t_ptrSnd = g_pCharSoundMgr->get_CharRunSoundList( _chrDir );

	if( !t_ptrSnd )
		return;

	// 路步必须有两个声音
	osassert( t_ptrSnd->mSoundItemList.size() == 2 );
	m_sFootMarkInfo.m_fMark1Percent = t_ptrSnd->mSoundItemList[0].m_fStartTime;
	m_sFootMarkInfo.m_fMark2Percent = t_ptrSnd->mSoundItemList[1].m_fStartTime;

	// 保证第一个声音百分比小于第二个
	if( m_sFootMarkInfo.m_fMark1Percent > m_sFootMarkInfo.m_fMark2Percent )
	{
		float t_f = m_sFootMarkInfo.m_fMark2Percent;
		m_sFootMarkInfo.m_fMark2Percent = m_sFootMarkInfo.m_fMark1Percent;
		m_sFootMarkInfo.m_fMark1Percent = t_f;
	}

	unguard;
}
void gc_hero::footWater_frameMove( float TimeMs )
{
	guard;
	static float EscapeTime = 0.0f;
	EscapeTime+=TimeMs;
	if (strlen(m_sFootWaterInfo.m_szFootWaterIdleSpe)==0)
	{
		return;
	}
	if (EscapeTime>m_sFootWaterInfo.m_playTime)
	{
		EscapeTime = 0;
		os_terrainTexSnd   t_sSnd;
		osVec3D pos;
		float angle;
		m_ptrSkinMesh->get_posAndAgl(pos,angle);
		osVec2D            t_vec2Pos( pos.x,pos.z );
		//!windy mod
		if (gc_gameScene::m_ptrScene->get_materialIdx( t_vec2Pos,t_sSnd )==-1)
		{
			
			if (pos.y <= t_sSnd.mWaterHeight)
			{
				pos.y = t_sSnd.mWaterHeight;
				
				gc_gameScene::m_ptrScene->create_effect(m_sFootWaterInfo.m_szFootWaterIdleSpe,pos,angle);
			}
		}
	}
	unguard;
}
//! frame move footMark.
void gc_hero::footmark_frameMove( float _percent )
{
	guard;

/*
	osVec3D   t_sVec3( 287.105f, 0.0f, 381.216f );
	gc_gameScene::m_ptrScene->get_detailPos( t_sVec3,true );
*/

	// 重新开始一轮路步动作
	if( _percent < m_fLastPosePercent )
	{
		m_bMark1 = false;
		m_bMark2 = false;
	}

	// 
	// 动作中的第一个脚印
	if( (_percent > m_sFootMarkInfo.m_fMark1Percent )&&(!this->m_bMark1) )
	{
		m_bMark1 = true;

		osVec3D    t_vec3Dir,t_vec3Top,t_vec3Offset;

		// 
		// 脚印在人物移动方向的侧方移动一定的矩离，更能跟脚步配合好。
		t_vec3Dir = m_vec3DestMovePos - m_vec3EntPos;
		t_vec3Dir.y = 0.0f;
		t_vec3Top = osVec3D( 0.0f,1.0f,0.0f );
		osVec3Cross( &t_vec3Offset,&t_vec3Top,&t_vec3Dir );
		osVec3Normalize( &t_vec3Offset,&t_vec3Offset );
		t_vec3Offset *= m_sFootMarkInfo.m_fMarkOffset;


		m_sFMInit.m_vec3Pos = m_vec3EntPos + t_vec3Offset;
		m_sFMInit.m_fRotAgl = m_fMoveDir + m_sFootMarkInfo.m_fFootMarkRotAdj;

		// 如果此位置的纹理显示特效脚印，则显创建脚印的Decal.
		os_terrainTexSnd   t_sSnd;
		osVec2D            t_vec2Pos( m_sFMInit.m_vec3Pos.x,m_sFMInit.m_vec3Pos.z );
		//!windy mod
		if (gc_gameScene::m_ptrScene->get_materialIdx( t_vec2Pos,t_sSnd )>=0)
		{
			if( t_sSnd.m_bFootPrint )
				gc_gameScene::m_ptrScene->create_decal( m_sFMInit );

			// 脚部产生的特效
			gc_gameScene::m_ptrScene->get_detailPos( m_sFMInit.m_vec3Pos );
			if( t_sSnd.m_szEffect[0] != NULL )
				gc_gameScene::m_ptrScene->create_effect( t_sSnd.m_szEffect,m_sFMInit.m_vec3Pos );

			// 脚步声音的播放
			if( t_sSnd.m_szSound[0] != NULL )
				gc_gameScene::m_ptrSoundMgr->play_sound( t_sSnd.m_szSound,FALSE,&m_sFMInit.m_vec3Pos );
		}	
	}

	//
	// 动作中的第二个脚印
	if( (_percent > m_sFootMarkInfo.m_fMark2Percent )&&(!this->m_bMark2) )
	{
		m_bMark2 = true;

		osVec3D    t_vec3Dir,t_vec3Top,t_vec3Offset;

		// 脚印在人物移动方向的侧方移动一定的矩离，更能跟脚步配合好。
		t_vec3Dir = m_vec3DestMovePos - m_vec3EntPos;
		t_vec3Dir.y = 0.0f;
		t_vec3Top = osVec3D( 0.0f,1.0f,0.0f );
		osVec3Cross( &t_vec3Offset,&t_vec3Dir,&t_vec3Top );
		osVec3Normalize( &t_vec3Offset,&t_vec3Offset );
		t_vec3Offset *= m_sFootMarkInfo.m_fMarkOffset;

		m_sFMInit.m_vec3Pos = m_vec3EntPos + t_vec3Offset;
		m_sFMInit.m_fRotAgl = m_fMoveDir + m_sFootMarkInfo.m_fFootMarkRotAdj;

		// 
		os_terrainTexSnd   t_sSnd;
		osVec2D            t_vec2Pos( m_sFMInit.m_vec3Pos.x,m_sFMInit.m_vec3Pos.z );
		//!windy mod
		if (gc_gameScene::m_ptrScene->get_materialIdx( t_vec2Pos,t_sSnd )>=0)
		{
			if( t_sSnd.m_bFootPrint )
				gc_gameScene::m_ptrScene->create_decal( m_sFMInit );

			// 脚部产生的特效
			gc_gameScene::m_ptrScene->get_detailPos( m_sFMInit.m_vec3Pos );
			if( t_sSnd.m_szEffect[0] != NULL )
				gc_gameScene::m_ptrScene->create_effect( t_sSnd.m_szEffect,m_sFMInit.m_vec3Pos );

			// 脚步声音的播放
			if( t_sSnd.m_szSound[0] != NULL )
				gc_gameScene::m_ptrSoundMgr->play_sound( t_sSnd.m_szSound,FALSE,&m_sFMInit.m_vec3Pos );

		}
		
		
	}

	m_fLastPosePercent = _percent;


	unguard;
}

//! 设置人物的马上人物位移.
void gc_hero::add_ridePosOffset( osVec3D& _vecOffset )
{
	guard;

	osVec3D   t_vec3RidePos;
	
	if( m_ptrRides )
	{
		m_ptrRides->GetLastPartPos( t_vec3RidePos );
		t_vec3RidePos += _vecOffset;
		m_ptrRides->SetLastPartPos( t_vec3RidePos );
	}


	unguard;
}

//! 存储人物的坐骑信息调整.
void gc_hero::save_rideOffset( void )
{
	guard;

	// 先得到最后一部分的位移.
	osVec3D   t_vec3;
	if( m_ptrRides )
	{
		//
		m_ptrRides->GetLastPartPos( t_vec3 );
		s_string   t_szFile = m_szCreatureDir + ".cha";
		gc_creature::m_sChaFileLoader.save_rideChaFile( t_szFile.c_str(),t_vec3 );
		MessageBox( NULL,va( "%s<%s>","存储文件成功",t_szFile.c_str() ),"SUCCESS",MB_OK );
	}

	unguard;
}



//! 重设主角人物的位置
void gc_hero::reset_heroPos( osVec3D& _pos )
{
	guard;

	osVec3D     t_pos;
	float       t_fAngle;

	start_freeStatus();
	m_ptrSkinMesh->get_posAndAgl( t_pos,t_fAngle );

	t_pos = _pos;
	m_ptrSkinMesh->frame_move( t_pos,t_fAngle );

	// 更新entity Pos
	m_vec3EntPos = t_pos;
	m_fRotAgl = t_fAngle;

	// 更新相机位置
	update_camera();

	unguard;
}


//! 重设主角人物的位置
void gc_hero::reset_heroPos( int _x,int _z )
{
	guard;

	osVec3D     t_pos;
	float       t_fAngle;

	start_freeStatus();
	m_ptrSkinMesh->get_posAndAgl( t_pos,t_fAngle );

	gc_gameScene::m_ptrScene->get_tilePos( _x,_z,t_pos );
	m_ptrSkinMesh->frame_move( t_pos,t_fAngle );

	// 更新entity Pos
	m_vec3EntPos = t_pos;
	this->m_fRotAgl = t_fAngle;

	// 更新相机位置
	update_camera();

	unguard;
}


//! 对动作声音的FrameMove.
void gc_hero::frame_moveActSnd( int _sndIdx )
{
	guard;

	osassert( _sndIdx >= 0 );
	osassert( _sndIdx < (int)m_vecActSound.size() );

	float  t_fActPercent = 0.0f;

	// 确认当前播放的动作是我们要播放声音的动作
	char   t_szActName[32];

	// 
	// 处理引擎内部把动作名换为其它动作的情形
	m_ptrSkinMesh->get_curPlayActIdx( t_szActName );
	if( strcmp( t_szActName,m_szCurActName ) != 0 )
	{
		m_szCurActName[0] = NULL;
		m_bActSndHavePlayed = FALSE;
		return;
	}


	// 
	// 得到当前动作播放的比例
	t_fActPercent = m_ptrSkinMesh->get_actionPlayPercent();

	if( m_bActSndHavePlayed )
	{
		if( m_fLastPosePercent > t_fActPercent )
			m_bActSndHavePlayed = false;
	}
	m_fLastPosePercent = t_fActPercent;
	//! 播放声音
	if( !m_bActSndHavePlayed )
	{
		osassert( m_vecActSound[_sndIdx].m_dwNumSound >= 1 );
		gcs_sound*   t_ptrSnd = &m_vecActSound[_sndIdx].m_vecSounds[0];
		osassert( t_ptrSnd->m_szSound[0] != NULL );

		if( t_fActPercent >= t_ptrSnd->m_fStartTime )
		{
			// 是否播放声音
			if( getrand_fromfloat( t_ptrSnd->m_fPlayChance ) )
			{
				osVec3D   t_vec3Pos;
				get_entityPos( t_vec3Pos );
				t_vec3Pos.y += gc_camera::m_fFocusOffset;

				gc_gameScene::m_ptrSoundMgr->play_sound( 
					t_ptrSnd->m_szSound,FALSE,&t_vec3Pos );
			}

			m_bActSndHavePlayed = TRUE;
		}
	}

	return ;

	unguard;
}


//! 重新计算下一步人物应该到达的直线位置
bool gc_hero::recal_nextPos( void )
{
	guard;

	// TEST CODE:
	int  t_iSrcPtx,t_iSrcPtz;
	int  t_iDstPtx,t_iDstPtz;
	int  t_iResPtx,t_iResPtz;

	osVec3D   t_vec3Src;

	t_iSrcPtx = int(m_vec3DestMovePos.x * 1000);
	t_iSrcPtz = int(m_vec3DestMovePos.z * 1000);


	t_iDstPtx = int(m_vec3RealTarget.x * 1000);
	t_iDstPtz = int(m_vec3RealTarget.z * 1000);

	osVec3D  t_vec3BackPos = m_vec3DestMovePos;

	bool t_bRes = true;
	if( !gc_gameScene::m_ptrPathFinder->CheckLine( t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz ) )
	{

		t_bRes = gc_gameScene::m_ptrPathFinder->FindPathMap(
			t_iSrcPtx,t_iSrcPtz,t_iDstPtx,t_iDstPtz,t_iResPtx,t_iResPtz );

		osDebugOut( "Recal src pt:<%d,%d>..DstPt<%d,%d>..Res pt<%d,%d>.\n",
			t_iSrcPtx,t_iSrcPtz,t_iDstPtx,t_iDstPtz,t_iResPtx,t_iResPtz );


		if( t_bRes )
		{
			m_vec3DestMovePos.x = t_iResPtx/1000.0f;
			m_vec3DestMovePos.z = t_iResPtz/1000.0f;
		}
	}
	else
	{
		osDebugOut( "Check line success<%d,%d,%d,%d>..\n",
			t_iSrcPtx, t_iSrcPtz, t_iDstPtx, t_iDstPtz );
		m_vec3DestMovePos = m_vec3RealTarget; 
	}

	// 确认向前移动的方向
	m_vec3EntPos = t_vec3BackPos;
	m_fMoveDir = gc_creature::get_charNewDir( t_vec3BackPos,m_vec3DestMovePos );


	return t_bRes;

	unguard;
}


# define FADE_TOZEROTIME  1.0f

//! 更新主角人物的状态
BOOL gc_hero::frame_move( float _time,bool _nextUpdate/* = false*/ )
{
	guard;

	if( !m_bActive )
		return true;
/*
	osDebugOut( "The hero ent pos is:<%f,%f,%f>.DestMove pos<%f,%f,%f>..\n",this->m_vec3EntPos.x,
		this->m_vec3EntPos.y,this->m_vec3EntPos.z,
		m_vec3DestMovePos.x,m_vec3DestMovePos.y,m_vec3DestMovePos.z );
*/

	if( !_nextUpdate )
	{
		// 说话相关
		framemove_talkString( _time );

		// 坐骑状态，非移动时，需要更新相机
		if( (!m_bInMoveStatus) && m_bInRideState )
			update_camera();
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

		t_fMoveLength = _time * m_fRunSpeed;

		if( (t_fMoveLength > t_fLength) )
		{
			if( ::float_equal( m_vec3DestMovePos.x,m_vec3RealTarget.x ) &&
				::float_equal( m_vec3DestMovePos.z,m_vec3RealTarget.z )    )
				
			{
				t_vec3NewPos = this->m_vec3DestMovePos;
				m_bInMoveStatus = FALSE;
				start_freeStatus();

			}
			else
			{
				// 
				// River mod @ 2008-10-9: 完成人物寻路算法的修改
				// 重新寻路算法，计算人物应该到达的位置，然后返回
				if( !recal_nextPos() )
				{
					t_vec3NewPos = this->m_vec3DestMovePos;

					bool t_b = gc_gameScene::m_ptrPathFinder->CanMove( m_vec3EntPos );

					m_bInMoveStatus = FALSE;
					start_freeStatus();
				}

				// cal time use new move dir.
				float t_fNewDirTime = (t_fMoveLength - t_fLength)/m_fRunSpeed;
				frame_move( t_fNewDirTime,true );
				return true;
			}

		}
		else
		{
			t_vec3Dir *= t_fMoveLength;
			t_vec3NewPos = m_vec3EntPos + t_vec3Dir;

			bool t_b = gc_gameScene::m_ptrPathFinder->CanMove( m_vec3EntPos );

		}
		
		float  t_fPercent;
		bool   t_bDetailPosRight = true;

		t_bDetailPosRight = gc_gameScene::m_ptrScene->get_detailPos( t_vec3NewPos,true );
		// 点击到了错误的区域
		if( !t_bDetailPosRight )
		{
			m_bInMoveStatus = false;
			return false;
		}

		if( fabs( t_vec3NewPos.y - m_vec3EntPos.y ) > 10.0f )
		{
			m_bInMoveStatus = false;
			start_freeStatus();
			return false;
		}

#if __COLL_MOD__
		//{@ test 加入走路碰撞	
		os_pickTileIdx OldTileIdx,NewTileIdx;
		gc_gameScene::m_ptrScene->get_sceneTGIdx( NewTileIdx,t_vec3NewPos);
		gc_gameScene::m_ptrScene->get_sceneTGIdx( OldTileIdx,mOldPos);
		if(OldTileIdx.m_tgIdx.m_iX!=NewTileIdx.m_tgIdx.m_iX||
			OldTileIdx.m_tgIdx.m_iY!=NewTileIdx.m_tgIdx.m_iY)
		{
			osDebugOut("自动保存碰撞信息,目前什么都没有做\n");
		}                     
		mOldPos = t_vec3NewPos;
		//@}
#endif
		// 骑乘状态以坐骑为中心点
		if( m_bInRideState )
		{
			osassert( m_ptrRides );
			t_vec3NewPos.y += m_fEarthOffset;
			m_ptrRides->frame_move( t_vec3NewPos,m_fMoveDir );
		}
		else
		{
			t_fPercent = m_ptrSkinMesh->frame_move( t_vec3NewPos,m_fMoveDir );

# if 0
			// TEST CODE:
			osVec3D  t_vec3CamPos = gc_gameScene::m_ptrCamera->m_ptrCamera->get_curpos();
			osVec3D  t_vec3CamFocus = gc_gameScene::m_ptrCamera->m_ptrCamera->get_camFocus();
			float t_fdir = gc_creature::get_charNewDir( t_vec3CamPos,t_vec3CamFocus );	
			if( !float_equal( m_fMoveDir-t_fdir,0.0f ) )
			{
				float t_f = _time*1.0f;
				if( m_fMoveDir-t_fdir < 0 )
					t_f = -t_f;

				if( fabs( m_fMoveDir-t_fdir) < t_f )
					t_f = fabs( m_fMoveDir-t_fdir);

				gc_gameScene::m_ptrCamera->m_ptrCamera->
					rotate_horizon( t_vec3CamFocus,t_f );
			}
# endif 

			gc_gameScene::m_ptrScene->get_collision_info(t_vec3NewPos,5);
			// 如果此时的条件需要显示脚印，则更新脚印,特效和声音
			footmark_frameMove( t_fPercent );
			
		}

		m_vec3EntPos = t_vec3NewPos;
		m_fRotAgl = m_fMoveDir;

		// 发出更新相机的命令
		update_camera();

	}
	else
	{
		int t_idx = find_actSndInfo( m_szCurActName );

		// 当前的动作是否有音效需要播放
		if( t_idx != -1 )
			frame_moveActSnd( t_idx );

	}
	//!windy mod 用于更新脚下的水花
	footWater_frameMove(_time);

	// 输出人物的调试信息
	if( m_bActive )
	{
		lm_gvar::g_iHeroPosX = int(m_vec3EntPos.x/HALF_TWIDTH);
		lm_gvar::g_iHeroPosZ = int(m_vec3EntPos.z/HALF_TWIDTH);

		// River mod @ 2007-10-9:加入人物的详细位置信息
		lm_gvar::g_fHeroPosX = m_vec3EntPos.x;
		lm_gvar::g_fHeroPosZ = m_vec3EntPos.z;


		// 更新人物的位置
		gc_gameApp::push_debugInfo( 
			va( "pos:<%d,%d> map<%d,%d>..",lm_gvar::g_iHeroPosX,
			lm_gvar::g_iHeroPosZ,lm_gvar::g_iHeroPosX/128,lm_gvar::g_iHeroPosZ/128 ),
			10,10,0xffffffff );
		gc_gameApp::push_debugInfo(
			va( "位置:<%f,%f,%f> 旋转:<%f>",m_vec3EntPos.x,m_vec3EntPos.y,m_vec3EntPos.z,m_fRotAgl ),
			10,30,0xffffffff );

		// 相机的焦点位置.TEST CODE:
		
		osVec3D  t_vec3Pos,t_vec3Focus;
		gc_camera::m_ptrCamera->get_curpos( &t_vec3Pos );
		gc_camera::m_ptrCamera->get_camFocus( t_vec3Focus );

		float t_f = osVec3Length( &(t_vec3Pos-t_vec3Focus) );
		gc_gameApp::push_debugInfo( va( "焦点长度:<%f>",t_f ),10,50,0xffffffff );
		/**/


		//gc_gameApp::m_ptrActiveScene->get_det
		if( gc_gameScene::m_ptrScene->get_detailPos( m_vec3EntPos,true ) )
		{
			m_ptrSkinMesh->frame_move( m_vec3EntPos,m_ptrSkinMesh->get_smAgl() );
		}
	}

	return TRUE;

	unguard;
}

