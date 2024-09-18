///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_magicEffect.cpp
 *
 *  Desc:     魔法demo客户端用到的魔法特效相关功能类
 * 
 *  His:      River created @ 2006-5-30
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include ".\lm_magiceffect.h"
# include "gc_gameScene.h"
# include "gc_creature.h"

# define  GSKILL_FILENAME   "speff\\MagicEffect.tec"

//  
//! 测试Demo内使用固定的蓄力时间
# define  AMASSPOWER_TIME   0.8f


// 包装此函数，加入全局的施法动作速度改变
float SKILLDATA::get_effStartTime( void ) const 
{
	return effectStartTime;
}
float SKILLDATA::get_followEffEndTime( void ) const
{
	return followEffectEndTime;
}


float lm_magicEffectIns::get_amassPowerTime( void )
{
	if( m_fFollowEffectEndTime > 0.0f )
		return  m_fFollowEffectEndTime;
	return AMASSPOWER_TIME;
}


lm_magicEffectIns::lm_magicEffectIns(void)
{
	m_ptrSkillData = NULL;
	m_fEleTime = 0.0f;
}

lm_magicEffectIns::~lm_magicEffectIns(void)
{

}

//! 创建effect并播放
BOOL lm_magicEffectIns::init_magicEffectIns( lm_command& _cmd,const SKILLDATA* _skill )
{
	guard;

	lm_skillPlay*     t_ptrSkillPlay;

	osassert( _skill );
	this->m_ptrSkillData = _skill;

	m_fEleTime = 0.0f;
	t_ptrSkillPlay = (lm_skillPlay*)_cmd.m_ptrCommandContent;
	m_sSkillPlay = *t_ptrSkillPlay;
	
	// 高度的调整。
	m_fSkillAdjHeight = t_ptrSkillPlay->m_fHeiAdj;

	// 如果攻击点的指针为空，则赋值此指针到玩家所在的位置
	osassertex( m_sSkillPlay.m_ptrStartPos,"玩家起始点位置不能为空....\n" );
	if( m_sSkillPlay.m_ptrAttackPos == NULL )
		m_sSkillPlay.m_ptrAttackPos = m_sSkillPlay.m_ptrStartPos;

	m_fFollowEffectEndTime = -1.0f;
	// 根据技能要播放的动作来发送给主角人物动作改变的命令
	if( m_ptrSkillData->actionName[0] )
	{
		lm_creatureActChange    t_sActChange;
		lm_command          t_sCmd;
		osVec3D             t_vec3StartPos,t_vec3AttackPos;

		t_vec3StartPos = *m_sSkillPlay.m_ptrStartPos;
		t_vec3AttackPos = *m_sSkillPlay.m_ptrAttackPos;

		// 要播放的动作
		t_sActChange.m_iPlayActType = 1;
		strcpy( t_sActChange.m_szNewActName,m_ptrSkillData->actionName );

		// 根据当前主角人物的位置和目标的位置，来设置主角人物的旋转方向
		t_sActChange.m_bUseAgl = TRUE;
		t_sActChange.m_fRotAgl = 
			gc_creature::get_charNewDir( t_vec3StartPos,t_vec3AttackPos );

		if( _cmd.m_iSHandle == -1 )
		{
			t_sCmd.m_eCommandId = LM_HERONEWACTION;
		}
		else
		{
			t_sCmd.m_iSHandle = _cmd.m_iSHandle;
			t_sCmd.m_eCommandId = LM_MONSTERACT;
		}

		t_sActChange.m_fPlayActScale = lm_gvar::g_fSkillActSpeed;

		t_sCmd.m_dwCmdContentSize = sizeof( lm_creatureActChange );
		t_sCmd.m_ptrCommandContent = &t_sActChange;

		lmc_commandMgr::Instance()->push_command( t_sCmd );

		m_fFollowEffectEndTime = gc_gameScene::m_ptrCreatureMgr->
			get_creatureActTime( t_sCmd.m_iSHandle,t_sActChange.m_szNewActName );
		m_fFollowEffectEndTime *= (lm_gvar::g_fSkillActSpeed);
	}

	// 技能特效和伴随特效
	m_bArrivedEffStartPt = FALSE;
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
		m_dwEffInsId[t_i] = 0xffffffff;
	m_bArrivedFolEffStartPt = FALSE;
	m_dwFolEffId = 0xffffffff;
	if( m_ptrSkillData->followEffectFilename[0] )
		m_bFollowEffEnd = FALSE;
	else
		m_bFollowEffEnd = TRUE;

	// 如果有过程特效，则需要过程特效播放完成后，再播放攻击点的特效
	m_bArrivedAttackTarget = TRUE;
	m_bAmassPowerEnd = TRUE;
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		m_bSkillEffPlayed[t_i] = FALSE;
		if( m_ptrSkillData->effectFilename[t_i][0] )
		{
			if( m_ptrSkillData->effectType[t_i] == SEP_NONE )
				m_bArrivedAttackTarget = FALSE;
			if( m_ptrSkillData->effectType[t_i] == SEP_FOLLOW_LAUNCHER_CENTER_FOREVER )
				m_bAmassPowerEnd = FALSE;
		}
		else
			// 文件名为空，不需要播放，设置为已经播放过
			m_bSkillEffPlayed[t_i] = TRUE;

	}

	// 跟随攻击点特效的初始化
	m_bPlayingFollowAttack = false;
	m_fFollowAttackTime = 0.0f;


	// 对此技能内的特效进行处理
	frame_moveMagicEffect( 0.0f );

	return TRUE;

	unguard;
}

//! 判断一个技能特效是否全部播放完成
BOOL lm_magicEffectIns::is_skillEffectPlaying( void )
{
	guard;

	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		// 如果特效id不是-1,返回
		if( m_dwEffInsId[t_i] != 0xffffffff )
			return TRUE;

		// 如果特效还没有播放，此技能也没有播放完
		if( !m_bSkillEffPlayed[t_i] )
			return TRUE;
	}

	// 此技能内部的特效已经全部被删除,如果伴随特效也结束，则此特效结束。
	if( m_bFollowEffEnd )
		return FALSE;
	else
		return TRUE;

	unguard;
}

//! 对蓄力特效进行处理。
void lm_magicEffectIns::framemove_amassPower( int _idx )
{
	guard;
	osassert( _idx>=0 );
	osassert( _idx < MAX_EFFECTPERSKILL );
	
	// 蓄力时间+特效开始时间 < 技能特效过去的时间
	//  则蓄力结束
	if( m_fEleTime >=
		(m_ptrSkillData->get_effStartTime()+get_amassPowerTime() ) )
	{
		if( m_dwEffInsId[_idx] != 0xffffffff )
			gc_gameScene::m_ptrScene->delete_effect( m_dwEffInsId[_idx] );
		
		m_bAmassPowerEnd = TRUE;
	}

	unguard;
}

//! 对跟随攻击点播放的特效进行FrameMove
void lm_magicEffectIns::framemove_followAttackEffect( int _idx )
{
	guard;

	osassert( _idx>=0 );
	osassert( _idx < MAX_EFFECTPERSKILL );

	float   t_fEleTime = sg_timer::Instance()->get_lastelatime();

	m_fFollowAttackTime += t_fEleTime;
	if( m_fFollowAttackTime > FOLLOWATTACK_EFFECTTIME )
	{
		m_bPlayingFollowAttack = false;

		if( m_dwEffInsId[_idx] != 0xffffffff )
			gc_gameScene::m_ptrScene->delete_effect( m_dwEffInsId[_idx] );
	}
	else
	{
		osVec3D  t_vec3 = *m_ptrAttackPos;

		// River added @ 2007-6-7:高度调整
		t_vec3.y += m_fSkillAdjHeight;
		if( vec3_equal( t_vec3,m_vec3LastFrameAttackPos ) )
			return ;

		m_vec3LastFrameAttackPos = t_vec3;
		gc_gameScene::m_ptrScene->set_effectPos( m_dwEffInsId[_idx],t_vec3,0.0f );
	}

	unguard;
}


//! 对过程特效进行处理，如火球术中，从玩家到怪物的火球
void lm_magicEffectIns::framemove_moveEffect( int _idx )
{
	guard;

	osassert( _idx>=0 );
	osassert( _idx < MAX_EFFECTPERSKILL );

	float    t_fEleTime = sg_timer::Instance()->get_lastelatime();

	m_fDisFromStart += (t_fEleTime*m_ptrSkillData->motionSpeed);

	// 根据速度算出每一帧的位置
	osVec3D        t_vec3Dir;
	float          t_fLength;

	t_vec3Dir = *m_sSkillPlay.m_ptrAttackPos - *m_sSkillPlay.m_ptrStartPos;
	t_fLength = osVec3Length( &t_vec3Dir );

	if( m_fDisFromStart > t_fLength )
	{
		if( m_dwEffInsId[_idx] != 0xffffffff )
			gc_gameScene::m_ptrScene->delete_effect( m_dwEffInsId[_idx] );

		m_bArrivedAttackTarget = TRUE;
	}
	else // 重设移动特效的位置
	{
		osVec3D   t_vec3EffPos;
		
		osVec3Normalize( &t_vec3Dir,&t_vec3Dir );
		t_vec3EffPos = *m_sSkillPlay.m_ptrStartPos + t_vec3Dir*m_fDisFromStart;

		// River added @ 2007-6-7:加入特效的高度调整
		t_vec3EffPos.y += m_fSkillAdjHeight;

		if( m_dwEffInsId[_idx] != 0xffffffff )
		{
			gc_gameScene::m_ptrScene->set_effectPos( m_dwEffInsId[_idx],
				t_vec3EffPos,  m_fMoveEffAgl,  m_ptrSkillData->motionSpeed );

			osDebugOut( "The effAnl is:<%f>..pos<%f,%f,%f>\n",
				m_fMoveEffAgl,t_vec3EffPos.x,t_vec3EffPos.y,t_vec3EffPos.z );
		}
	}

	return ;

	unguard;
}


/** \brief
*  当前的EffectIns是否在使用中,如果全部播放播放完，则返回false.
*
*  对于需要设置位置的特效，在此函数内设置位置。
*/
BOOL lm_magicEffectIns::frameMove_effectInsBePlaying( float _eletime )
{
	guard;

	// 对于非-1的effectId,validate并删除
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		if( m_dwEffInsId[t_i] == 0xffffffff )
			continue;

		// 如果是跟随主角人物的蓄力过程，则根据时间来删除特效
		if( (!m_bAmassPowerEnd ) &&
			(m_ptrSkillData->effectType[t_i] == SEP_FOLLOW_LAUNCHER_CENTER_FOREVER) )
			framemove_amassPower( t_i );
		// 如果是中间运动的特效，比如火球，根据速度设置位置，如果到达，则删除
		if( (!this->m_bArrivedAttackTarget ) &&
			( m_ptrSkillData->effectType[t_i] == SEP_NONE ) )
			framemove_moveEffect( t_i );
		
		// 跟随攻击点的特效
		if( m_bPlayingFollowAttack &&
			(m_ptrSkillData->effectType[t_i] == SEP_FOLLOW_TARGET) )
			framemove_followAttackEffect( t_i );
 	}

	// 播放某一个特效
	osVec3D    t_vec3StartPos,t_vec3AttackPos;
	osVec3D    t_vec3Tmp;
	float      t_fRotAgl;

	// 分解出对应的特效
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		// 没有特效对应或是此特效已经播放过
		if( m_bSkillEffPlayed[t_i] ||
		   (NULL == m_ptrSkillData->effectFilename[t_i][0]) )
			continue;

		t_vec3AttackPos = *m_sSkillPlay.m_ptrAttackPos;
		t_vec3StartPos =  *m_sSkillPlay.m_ptrStartPos;

		// River added @ 2007-6-7:特效的高度调整
		t_vec3StartPos.y += this->m_fSkillAdjHeight;
		t_vec3AttackPos.y += this->m_fSkillAdjHeight;

		switch( m_ptrSkillData->effectType[t_i] )
		{
			// 在主角人物身上播放的特效,在测试Demo内使用固定的时间.
		case SEP_FOLLOW_LAUNCHER_CENTER_FOREVER:
			{
				t_fRotAgl = gc_creature::get_charNewDir( t_vec3StartPos,t_vec3AttackPos );
				t_fRotAgl += m_ptrSkillData->motionAdjRadian;
				m_dwEffInsId[t_i] = gc_gameScene::m_ptrScene->create_effect( 
					m_ptrSkillData->effectFilename[t_i],t_vec3StartPos,t_fRotAgl );

				m_bSkillEffPlayed[t_i] = TRUE;

				break;
			}

			// 跟随攻击点播放的特效
		case SEP_FOLLOW_TARGET:
			{
				//　当过程特效到达目标点时，可能没有需要播放的中间特效
				if( m_bArrivedAttackTarget )
				{
					t_fRotAgl = gc_creature::get_charNewDir( t_vec3StartPos,t_vec3AttackPos );
					t_fRotAgl += m_ptrSkillData->motionAdjRadian;

					m_dwEffInsId[t_i] = gc_gameScene::m_ptrScene->create_effect( 
						m_ptrSkillData->effectFilename[t_i],t_vec3AttackPos,t_fRotAgl );

					m_bPlayingFollowAttack = true;
					m_fFollowAttackTime = 0.0f;
					m_vec3LastFrameAttackPos = t_vec3AttackPos;
					m_ptrAttackPos = m_sSkillPlay.m_ptrAttackPos;

					m_bSkillEffPlayed[t_i] = TRUE;
				}
				break;
			}		
			// 从主角人物运行到攻击点的特效,到达攻击点以后再播放跟随攻击点的特效
		case SEP_NONE:
			{
				// 蓄力结束，或是没有蓄力特效，才能播放技能的过程特效
				// 必须伴随特效结束，才能播放中间的过程特效
				if( m_bAmassPowerEnd && m_bFollowEffEnd )
				{
					t_fRotAgl = gc_creature::get_charNewDir( t_vec3StartPos,t_vec3AttackPos );
					t_fRotAgl += m_ptrSkillData->motionAdjRadian;

					m_dwEffInsId[t_i] = gc_gameScene::m_ptrScene->create_effect( 
						m_ptrSkillData->effectFilename[t_i],t_vec3StartPos,t_fRotAgl );

					m_fDisFromStart = 0.0f;
					m_fMoveEffAgl = t_fRotAgl;
					m_bSkillEffPlayed[t_i] = TRUE;
				}

				break;
			}
			// 在指定的位置上播放一次,碰上这种播放方式，直接播放就可以了
		case SEP_POSITION_ONETIMES:
			{
				t_fRotAgl = gc_creature::get_charNewDir( t_vec3StartPos,t_vec3AttackPos );
				t_fRotAgl += m_ptrSkillData->motionAdjRadian;
				m_dwEffInsId[t_i] = gc_gameScene::m_ptrScene->create_effect( 
					m_ptrSkillData->effectFilename[t_i],t_vec3AttackPos,t_fRotAgl );

				m_bSkillEffPlayed[t_i] = TRUE;

				break;
			}
		}
	}

	// 如果技能正在播放，返回真，播放完成，则返回FALSE
	return is_skillEffectPlaying();

	unguard;
}


//! 对伴随特效的处理
BOOL lm_magicEffectIns::frame_moveFollowEff( void )
{
	guard;

	if( m_bFollowEffEnd )
		return TRUE;

	// 如果已经到达，则查看是否到达删除特效的时间。
	if( m_bArrivedFolEffStartPt )
	{
		if( m_fEleTime >= (m_ptrSkillData->get_followEffEndTime()*lm_gvar::g_fSkillActSpeed) )
		{
			gc_gameScene::m_ptrScene->delete_effect( m_dwFolEffId );
			m_dwFolEffId = 0xffffffff;
			m_bFollowEffEnd = TRUE;
		}
		return TRUE;
	}

	if( m_fEleTime >= m_ptrSkillData->followEffectStartTime )
	{
		m_bArrivedFolEffStartPt = TRUE;

		osVec3D   t_vecStartPt = *m_sSkillPlay.m_ptrStartPos;

		// River mod @ 2007-6-7:加入特效的高度调整
		t_vecStartPt.y += m_fSkillAdjHeight;

		m_dwFolEffId = gc_gameScene::m_ptrScene->create_effect( 
			  m_ptrSkillData->followEffectFilename,t_vecStartPt );
	}

	return TRUE;

	unguard;
}


//! FrameMove
BOOL lm_magicEffectIns::frame_moveMagicEffect( float _eleTime )
{
	guard;

	m_fEleTime += _eleTime;

	// 处理伴随特效
	frame_moveFollowEff();

	// 如果已经播放过，则返回
	if( m_bArrivedEffStartPt )
		return frameMove_effectInsBePlaying( _eleTime );

	// 只有头一次到达特效开始时间点，才播放技能特效,如果没有到过播放点，直接返回
	if( m_fEleTime >= m_ptrSkillData->get_effStartTime() )
	{
		m_bArrivedEffStartPt = TRUE;
		return frameMove_effectInsBePlaying( _eleTime );
	}
	else
		return TRUE;

	return TRUE;

	unguard;
}




lm_meManager::lm_meManager()
{
	m_dwTestEffId = 0xffffffff;

	m_vecStateEffect.resize( 128 );
}

lm_meManager::~lm_meManager()
{

}


//! 从技能名得至此技能的id
int lm_meManager::get_skillIdFromName( const char* _name )
{
	guard;

	for( int t_i=0;t_i <(int)m_vecSkillData.size() ;t_i ++ )
	{
		if( strcmp( _name,m_vecSkillData[t_i].name ) == 0 )
			return (t_i + 1);
	}

	return -1;

	unguard;
}


/*

特效相关的一些内容：
1：跟随玩家的攻击点播放：指击中怪物后，跟随怪物播放。
2：其它：指运动的火球术的播放。
3：在指定的位置上播放：面攻击的魔法播放方式。

*/

//! 必须初始化整个场景用到的技能相关的信息
BOOL lm_meManager::init_skillList( const char* _fname )
{
	guard;

	BYTE*    t_fstart = NULL;
	int      t_iSize = 0;
	char     t_szDir[256];
	::getcwd( t_szDir,256 );
	if( !file_exist( GSKILL_FILENAME ) )
		osassertex( false,va("找不到文件<%s>..\n",GSKILL_FILENAME ) );

	char    t_szMagic[64];

	int  t_idx;
	t_fstart = START_USEGBUF( t_idx ); ;
	t_iSize = read_fileToBuf( GSKILL_FILENAME,t_fstart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
		return FALSE;
	osassertex( t_iSize>0,va( "打开文件<%s>失败....\n",GSKILL_FILENAME) );
	READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	osassertex( strcmp( t_szMagic,"tec" ) == 0 ,"文件格式错误...\n" );

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
	osassertex( t_iSize == 5,va( "<%s>文件的版本号必须是5<%d>",_fname,t_iSize ) );

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	osassert( t_iSize > 0 );
	m_vecSkillData.resize( t_iSize );

	READ_MEM_OFF( &m_vecSkillData[0],t_fstart,sizeof( SKILLDATA )*t_iSize );

	::g_bLogToFile = true;

	for( int t_i = 0;t_i<t_iSize;t_i ++ )
	{
		osDebugOut( "The <%d> skill name is:<%s>...\n",t_i,m_vecSkillData[t_i].name );
	}

	END_USEGBUF( t_idx );


# if 0
	// 
	// TEST CODE: 测试用到的effect名字
	FILE*   t_file = fopen( "qtangEff.tec","wb" );
	int     t_vecSkillIdx[] = { 15,18,19,21,24,25,26,27,28,29,30,33,34,35};
	int     t_iVersion = 5;

	fwrite( t_szMagic,sizeof( char),4,t_file );
	fwrite( &t_iVersion,sizeof( int ),1,t_file );
	t_iVersion = sizeof( t_vecSkillIdx )/sizeof(int);
	fwrite( &t_iVersion,sizeof( int ),1,t_file );
	

	for( int t_i=0;t_i<sizeof( t_vecSkillIdx )/sizeof( int );t_i ++ )
	{
		SKILLDATA*   t_ptrSkill = &m_vecSkillData[t_vecSkillIdx[t_i]];
		fwrite( t_ptrSkill,sizeof( SKILLDATA ),1,t_file );
	}
	fclose( t_file );
# endif 

	//! 
	m_vecMeIns.resize( 64 );

	return TRUE;                  

	unguard;
}

//! 在场景内播放一个状态特效
BOOL lm_meManager::play_stateMagic( lm_command& _cmd )
{
	guard;

	// 先根据handle,找到当前怪物所存储的位置
	int      t_iHandle = _cmd.m_iSHandle;
	int      t_idx = -1;


	CSpaceMgr<lm_monsterStateIns>::NODE   t_ptrNode;
	for( t_ptrNode = m_vecStateEffect.begin_usedNode();
		t_ptrNode.p != NULL;t_ptrNode = m_vecStateEffect.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p->m_iHandleId == t_iHandle )
		{
			t_idx = t_ptrNode.idx;
			break;
		}
	}

	// 先删除当前的状态，然后才创建新的状态
	if( t_idx >= 0 )
	{
		if( m_vecStateEffect[t_idx].m_iSpeffId != -1 )
		{
			gc_gameScene::m_ptrScene->delete_effect( m_vecStateEffect[t_idx].m_iSpeffId );
			m_vecStateEffect[t_idx].reset();
		}
	}
	else
		t_idx = m_vecStateEffect.get_freeNode();


	//! 
	lm_cmdFileName*    t_ptrStateName = (lm_cmdFileName*)_cmd.m_ptrCommandContent;
	if( NULL != t_ptrStateName->m_szFileName[0] )
	{
		gc_creature*   t_ptrCreature;

		if( _cmd.m_iSHandle < 0 )
		{
			t_ptrCreature = (gc_creature*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity(); 
		}
		else
		{
			t_ptrCreature = gc_gameScene::m_ptrCreatureMgr->
				get_creaturePtrByHandle( _cmd.m_iSHandle );
		}

		if( t_ptrCreature )
		{
			m_vecStateEffect[t_idx].m_vec3PtrPos = t_ptrCreature->get_entityPosPtr();
			m_vecStateEffect[t_idx].m_vec3LastPos = *m_vecStateEffect[t_idx].m_vec3PtrPos;
			m_vecStateEffect[t_idx].m_iHandleId = _cmd.m_iSHandle;

			m_vecStateEffect[t_idx].m_iSpeffId = 
				gc_gameScene::m_ptrScene->create_effect( t_ptrStateName->m_szFileName,
				m_vecStateEffect[t_idx].m_vec3LastPos,0.0f );
		}
	}


	return TRUE;

	unguard;
}


//! 执行指令的接口
BOOL lm_meManager::process_command( lm_command& _command )
{
	guard;

	if( LM_MESKILLPLAY == _command.m_eCommandId )
	{
		play_magicEffect( _command );
	}
	else if( LM_MEEFFECTPLAY == _command.m_eCommandId )
	{
		play_effectInSce( _command );
	}
	else if( LM_MESTATEEFFECT == _command.m_eCommandId )
	{
		play_stateMagic( _command );
	}
	else
		osassertex( false,"找不到的命令的对应处理...\n" );

	return TRUE;

	unguard;
}

//! 根据特效的命令，来播放一个特效
BOOL lm_meManager::play_magicEffect( lm_command& _cmd )
{
	guard;

	int   t_idx = m_vecMeIns.get_freeNode();

	lm_skillPlay*     t_ptrSkillPlay = (lm_skillPlay*)_cmd.m_ptrCommandContent;
	osassert( t_ptrSkillPlay->m_iSkillIndex < int(m_vecSkillData.size()+1) );

	// TEST CODE: 记录上次播放的skillId
	lm_gvar::g_szLastSkill = m_vecSkillData[t_ptrSkillPlay->m_iSkillIndex-1].name;
	
	osassert( t_idx != -1 );
	if( !m_vecMeIns[t_idx].init_magicEffectIns( 
		_cmd,&m_vecSkillData[t_ptrSkillPlay->m_iSkillIndex-1] ) )
		return FALSE;

	return TRUE;

	unguard;
}

//! 在场景内一个位置上播放一个特效
BOOL lm_meManager::play_effectInSce( lm_command& _cmd )
{
	guard;

	lm_effectPlay*    t_ptrEp;

	t_ptrEp = (lm_effectPlay*)_cmd.m_ptrCommandContent;

	// River @ 2006-8-12:不删除特效，用于测试最在场景内加入更多的特效:
	m_dwTestEffId = 
		gc_gameScene::m_ptrScene->create_effect( 
		t_ptrEp->m_szEffName,t_ptrEp->m_vec3Pos );
	
	return true;

	unguard;
}

void lm_monsterStateIns::frame_move( void )
{
	if( m_iSpeffId >= 0 )
	{
		if( vec3_equal( m_vec3LastPos,(osVec3D&)(*m_vec3PtrPos) ) )
			return;
		else
		{
			m_vec3LastPos = *m_vec3PtrPos;
			gc_gameScene::m_ptrScene->set_effectPos( m_iSpeffId,m_vec3LastPos,0.0f );
		}
	}
}



//! 整理特效管理器的帧间处理
void lm_meManager::frame_moveMeMgr( void )
{
	guard;

	float                                 t_fEleTime;
	CSpaceMgr<lm_magicEffectIns>::NODE    t_ptrNode;

	t_fEleTime = sg_timer::Instance()->get_lastelatime();
	for( t_ptrNode = m_vecMeIns.begin_usedNode();
		t_ptrNode.p != NULL;t_ptrNode = m_vecMeIns.next_validNode( &t_ptrNode ) )
	{
		// 
		if( !t_ptrNode.p->frame_moveMagicEffect( t_fEleTime ) )
			m_vecMeIns.release_node( t_ptrNode.idx );
	}

	// 怪物身上的状态
	CSpaceMgr<lm_monsterStateIns>::NODE   t_ptrMsNode;
	for( t_ptrMsNode = m_vecStateEffect.begin_usedNode();
		t_ptrMsNode.p != NULL;
		t_ptrMsNode = m_vecStateEffect.next_validNode( &t_ptrMsNode ) )
		t_ptrMsNode.p->frame_move();

	unguard;
}

