///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_magicEffect.cpp
 *
 *  Desc:     ħ��demo�ͻ����õ���ħ����Ч��ع�����
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
//! ����Demo��ʹ�ù̶�������ʱ��
# define  AMASSPOWER_TIME   0.8f


// ��װ�˺���������ȫ�ֵ�ʩ�������ٶȸı�
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

//! ����effect������
BOOL lm_magicEffectIns::init_magicEffectIns( lm_command& _cmd,const SKILLDATA* _skill )
{
	guard;

	lm_skillPlay*     t_ptrSkillPlay;

	osassert( _skill );
	this->m_ptrSkillData = _skill;

	m_fEleTime = 0.0f;
	t_ptrSkillPlay = (lm_skillPlay*)_cmd.m_ptrCommandContent;
	m_sSkillPlay = *t_ptrSkillPlay;
	
	// �߶ȵĵ�����
	m_fSkillAdjHeight = t_ptrSkillPlay->m_fHeiAdj;

	// ����������ָ��Ϊ�գ���ֵ��ָ�뵽������ڵ�λ��
	osassertex( m_sSkillPlay.m_ptrStartPos,"�����ʼ��λ�ò���Ϊ��....\n" );
	if( m_sSkillPlay.m_ptrAttackPos == NULL )
		m_sSkillPlay.m_ptrAttackPos = m_sSkillPlay.m_ptrStartPos;

	m_fFollowEffectEndTime = -1.0f;
	// ���ݼ���Ҫ���ŵĶ��������͸��������ﶯ���ı������
	if( m_ptrSkillData->actionName[0] )
	{
		lm_creatureActChange    t_sActChange;
		lm_command          t_sCmd;
		osVec3D             t_vec3StartPos,t_vec3AttackPos;

		t_vec3StartPos = *m_sSkillPlay.m_ptrStartPos;
		t_vec3AttackPos = *m_sSkillPlay.m_ptrAttackPos;

		// Ҫ���ŵĶ���
		t_sActChange.m_iPlayActType = 1;
		strcpy( t_sActChange.m_szNewActName,m_ptrSkillData->actionName );

		// ���ݵ�ǰ���������λ�ú�Ŀ���λ�ã������������������ת����
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

	// ������Ч�Ͱ�����Ч
	m_bArrivedEffStartPt = FALSE;
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
		m_dwEffInsId[t_i] = 0xffffffff;
	m_bArrivedFolEffStartPt = FALSE;
	m_dwFolEffId = 0xffffffff;
	if( m_ptrSkillData->followEffectFilename[0] )
		m_bFollowEffEnd = FALSE;
	else
		m_bFollowEffEnd = TRUE;

	// ����й�����Ч������Ҫ������Ч������ɺ��ٲ��Ź��������Ч
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
			// �ļ���Ϊ�գ�����Ҫ���ţ�����Ϊ�Ѿ����Ź�
			m_bSkillEffPlayed[t_i] = TRUE;

	}

	// ���湥������Ч�ĳ�ʼ��
	m_bPlayingFollowAttack = false;
	m_fFollowAttackTime = 0.0f;


	// �Դ˼����ڵ���Ч���д���
	frame_moveMagicEffect( 0.0f );

	return TRUE;

	unguard;
}

//! �ж�һ��������Ч�Ƿ�ȫ���������
BOOL lm_magicEffectIns::is_skillEffectPlaying( void )
{
	guard;

	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		// �����Чid����-1,����
		if( m_dwEffInsId[t_i] != 0xffffffff )
			return TRUE;

		// �����Ч��û�в��ţ��˼���Ҳû�в�����
		if( !m_bSkillEffPlayed[t_i] )
			return TRUE;
	}

	// �˼����ڲ�����Ч�Ѿ�ȫ����ɾ��,���������ЧҲ�����������Ч������
	if( m_bFollowEffEnd )
		return FALSE;
	else
		return TRUE;

	unguard;
}

//! ��������Ч���д���
void lm_magicEffectIns::framemove_amassPower( int _idx )
{
	guard;
	osassert( _idx>=0 );
	osassert( _idx < MAX_EFFECTPERSKILL );
	
	// ����ʱ��+��Ч��ʼʱ�� < ������Ч��ȥ��ʱ��
	//  ����������
	if( m_fEleTime >=
		(m_ptrSkillData->get_effStartTime()+get_amassPowerTime() ) )
	{
		if( m_dwEffInsId[_idx] != 0xffffffff )
			gc_gameScene::m_ptrScene->delete_effect( m_dwEffInsId[_idx] );
		
		m_bAmassPowerEnd = TRUE;
	}

	unguard;
}

//! �Ը��湥���㲥�ŵ���Ч����FrameMove
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

		// River added @ 2007-6-7:�߶ȵ���
		t_vec3.y += m_fSkillAdjHeight;
		if( vec3_equal( t_vec3,m_vec3LastFrameAttackPos ) )
			return ;

		m_vec3LastFrameAttackPos = t_vec3;
		gc_gameScene::m_ptrScene->set_effectPos( m_dwEffInsId[_idx],t_vec3,0.0f );
	}

	unguard;
}


//! �Թ�����Ч���д�����������У�����ҵ�����Ļ���
void lm_magicEffectIns::framemove_moveEffect( int _idx )
{
	guard;

	osassert( _idx>=0 );
	osassert( _idx < MAX_EFFECTPERSKILL );

	float    t_fEleTime = sg_timer::Instance()->get_lastelatime();

	m_fDisFromStart += (t_fEleTime*m_ptrSkillData->motionSpeed);

	// �����ٶ����ÿһ֡��λ��
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
	else // �����ƶ���Ч��λ��
	{
		osVec3D   t_vec3EffPos;
		
		osVec3Normalize( &t_vec3Dir,&t_vec3Dir );
		t_vec3EffPos = *m_sSkillPlay.m_ptrStartPos + t_vec3Dir*m_fDisFromStart;

		// River added @ 2007-6-7:������Ч�ĸ߶ȵ���
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
*  ��ǰ��EffectIns�Ƿ���ʹ����,���ȫ�����Ų����꣬�򷵻�false.
*
*  ������Ҫ����λ�õ���Ч���ڴ˺���������λ�á�
*/
BOOL lm_magicEffectIns::frameMove_effectInsBePlaying( float _eletime )
{
	guard;

	// ���ڷ�-1��effectId,validate��ɾ��
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		if( m_dwEffInsId[t_i] == 0xffffffff )
			continue;

		// ����Ǹ�������������������̣������ʱ����ɾ����Ч
		if( (!m_bAmassPowerEnd ) &&
			(m_ptrSkillData->effectType[t_i] == SEP_FOLLOW_LAUNCHER_CENTER_FOREVER) )
			framemove_amassPower( t_i );
		// ������м��˶�����Ч��������򣬸����ٶ�����λ�ã���������ɾ��
		if( (!this->m_bArrivedAttackTarget ) &&
			( m_ptrSkillData->effectType[t_i] == SEP_NONE ) )
			framemove_moveEffect( t_i );
		
		// ���湥�������Ч
		if( m_bPlayingFollowAttack &&
			(m_ptrSkillData->effectType[t_i] == SEP_FOLLOW_TARGET) )
			framemove_followAttackEffect( t_i );
 	}

	// ����ĳһ����Ч
	osVec3D    t_vec3StartPos,t_vec3AttackPos;
	osVec3D    t_vec3Tmp;
	float      t_fRotAgl;

	// �ֽ����Ӧ����Ч
	for( int t_i=0;t_i<MAX_EFFECTPERSKILL;t_i ++ )
	{
		// û����Ч��Ӧ���Ǵ���Ч�Ѿ����Ź�
		if( m_bSkillEffPlayed[t_i] ||
		   (NULL == m_ptrSkillData->effectFilename[t_i][0]) )
			continue;

		t_vec3AttackPos = *m_sSkillPlay.m_ptrAttackPos;
		t_vec3StartPos =  *m_sSkillPlay.m_ptrStartPos;

		// River added @ 2007-6-7:��Ч�ĸ߶ȵ���
		t_vec3StartPos.y += this->m_fSkillAdjHeight;
		t_vec3AttackPos.y += this->m_fSkillAdjHeight;

		switch( m_ptrSkillData->effectType[t_i] )
		{
			// �������������ϲ��ŵ���Ч,�ڲ���Demo��ʹ�ù̶���ʱ��.
		case SEP_FOLLOW_LAUNCHER_CENTER_FOREVER:
			{
				t_fRotAgl = gc_creature::get_charNewDir( t_vec3StartPos,t_vec3AttackPos );
				t_fRotAgl += m_ptrSkillData->motionAdjRadian;
				m_dwEffInsId[t_i] = gc_gameScene::m_ptrScene->create_effect( 
					m_ptrSkillData->effectFilename[t_i],t_vec3StartPos,t_fRotAgl );

				m_bSkillEffPlayed[t_i] = TRUE;

				break;
			}

			// ���湥���㲥�ŵ���Ч
		case SEP_FOLLOW_TARGET:
			{
				//����������Ч����Ŀ���ʱ������û����Ҫ���ŵ��м���Ч
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
			// �������������е����������Ч,���﹥�����Ժ��ٲ��Ÿ��湥�������Ч
		case SEP_NONE:
			{
				// ��������������û��������Ч�����ܲ��ż��ܵĹ�����Ч
				// ���������Ч���������ܲ����м�Ĺ�����Ч
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
			// ��ָ����λ���ϲ���һ��,�������ֲ��ŷ�ʽ��ֱ�Ӳ��žͿ�����
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

	// ����������ڲ��ţ������棬������ɣ��򷵻�FALSE
	return is_skillEffectPlaying();

	unguard;
}


//! �԰�����Ч�Ĵ���
BOOL lm_magicEffectIns::frame_moveFollowEff( void )
{
	guard;

	if( m_bFollowEffEnd )
		return TRUE;

	// ����Ѿ������鿴�Ƿ񵽴�ɾ����Ч��ʱ�䡣
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

		// River mod @ 2007-6-7:������Ч�ĸ߶ȵ���
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

	// ���������Ч
	frame_moveFollowEff();

	// ����Ѿ����Ź����򷵻�
	if( m_bArrivedEffStartPt )
		return frameMove_effectInsBePlaying( _eleTime );

	// ֻ��ͷһ�ε�����Ч��ʼʱ��㣬�Ų��ż�����Ч,���û�е������ŵ㣬ֱ�ӷ���
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


//! �Ӽ����������˼��ܵ�id
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

��Ч��ص�һЩ���ݣ�
1��������ҵĹ����㲥�ţ�ָ���й���󣬸�����ﲥ�š�
2��������ָ�˶��Ļ������Ĳ��š�
3����ָ����λ���ϲ��ţ��湥����ħ�����ŷ�ʽ��

*/

//! �����ʼ�����������õ��ļ�����ص���Ϣ
BOOL lm_meManager::init_skillList( const char* _fname )
{
	guard;

	BYTE*    t_fstart = NULL;
	int      t_iSize = 0;
	char     t_szDir[256];
	::getcwd( t_szDir,256 );
	if( !file_exist( GSKILL_FILENAME ) )
		osassertex( false,va("�Ҳ����ļ�<%s>..\n",GSKILL_FILENAME ) );

	char    t_szMagic[64];

	int  t_idx;
	t_fstart = START_USEGBUF( t_idx ); ;
	t_iSize = read_fileToBuf( GSKILL_FILENAME,t_fstart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
		return FALSE;
	osassertex( t_iSize>0,va( "���ļ�<%s>ʧ��....\n",GSKILL_FILENAME) );
	READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	osassertex( strcmp( t_szMagic,"tec" ) == 0 ,"�ļ���ʽ����...\n" );

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
	osassertex( t_iSize == 5,va( "<%s>�ļ��İ汾�ű�����5<%d>",_fname,t_iSize ) );

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
	// TEST CODE: �����õ���effect����
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

//! �ڳ����ڲ���һ��״̬��Ч
BOOL lm_meManager::play_stateMagic( lm_command& _cmd )
{
	guard;

	// �ȸ���handle,�ҵ���ǰ�������洢��λ��
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

	// ��ɾ����ǰ��״̬��Ȼ��Ŵ����µ�״̬
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


//! ִ��ָ��Ľӿ�
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
		osassertex( false,"�Ҳ���������Ķ�Ӧ����...\n" );

	return TRUE;

	unguard;
}

//! ������Ч�����������һ����Ч
BOOL lm_meManager::play_magicEffect( lm_command& _cmd )
{
	guard;

	int   t_idx = m_vecMeIns.get_freeNode();

	lm_skillPlay*     t_ptrSkillPlay = (lm_skillPlay*)_cmd.m_ptrCommandContent;
	osassert( t_ptrSkillPlay->m_iSkillIndex < int(m_vecSkillData.size()+1) );

	// TEST CODE: ��¼�ϴβ��ŵ�skillId
	lm_gvar::g_szLastSkill = m_vecSkillData[t_ptrSkillPlay->m_iSkillIndex-1].name;
	
	osassert( t_idx != -1 );
	if( !m_vecMeIns[t_idx].init_magicEffectIns( 
		_cmd,&m_vecSkillData[t_ptrSkillPlay->m_iSkillIndex-1] ) )
		return FALSE;

	return TRUE;

	unguard;
}

//! �ڳ�����һ��λ���ϲ���һ����Ч
BOOL lm_meManager::play_effectInSce( lm_command& _cmd )
{
	guard;

	lm_effectPlay*    t_ptrEp;

	t_ptrEp = (lm_effectPlay*)_cmd.m_ptrCommandContent;

	// River @ 2006-8-12:��ɾ����Ч�����ڲ������ڳ����ڼ���������Ч:
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



//! ������Ч��������֡�䴦��
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

	// �������ϵ�״̬
	CSpaceMgr<lm_monsterStateIns>::NODE   t_ptrMsNode;
	for( t_ptrMsNode = m_vecStateEffect.begin_usedNode();
		t_ptrMsNode.p != NULL;
		t_ptrMsNode = m_vecStateEffect.next_validNode( &t_ptrMsNode ) )
		t_ptrMsNode.p->frame_move();

	unguard;
}

