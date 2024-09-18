///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_creature.cpp
 *
 *  Desc:     魔法demo客户端用到的人物显示基础类，魔法demo内的怪物，主角人物都从这个类继承
 * 
 *  His:      River created @ 2006-4-14
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# include ".\gc_creature.h"
# include "gc_gameScene.h"
# include "lm_configInfoReader.h"
# include "gameApp.h"
# include "gc_camera.h"
# include "gc_charSound.h"
//# include "gc_chaEffect.h"
//! 初始化monster队列的大小
# define INIT_MONSTERSIZE    256

//! 人物的sound相关文件后缀
# define CHAR_SNDFILE  ".snd"

//! 旗帜所在的skinMesh身体部位
# define FLAG_PART     2


//! 由4个8位字符生成无符号32位类型数值宏
#define OS_MAKEFOURCC(ch0, ch1, ch2, ch3)			\
	((DWORD)(unsigned char)(ch0) | ((DWORD)(unsigned char)(ch1) << 8) |		\
	((DWORD)(unsigned char)(ch2) << 16) | ((DWORD)(unsigned char)(ch3) << 24 ))

#define ChaFileID OS_MAKEFOURCC('C','H','A','\0')


//! 
BOOL       g_bDrawCreatureBBox = FALSE;


CMyChaFile::CMyChaFile()
{
}

CMyChaFile::~CMyChaFile()
{
}


//! 存储cha文件，主要编辑坐骑信息
bool CMyChaFile::save_rideChaFile( const char* _filename,osVec3D& _offset )
{
	guard;

	m_header.mbIsRide = TRUE;
	m_header.mRidePos = _offset;

	FILE*   t_file = fopen( _filename,"wb" );
	osassert( t_file );

	fwrite( &m_header,sizeof( ChaHeader ),1,t_file );

	fclose( t_file );

	return true;

	unguard;
}


bool CMyChaFile::load_chaFile( const char* _filename )
{
	LPBYTE   t_fstart;

	int t_iGBufIdx;
	t_fstart = START_USEGBUF( t_iGBufIdx );

	if( 0 >= read_fileToBuf( (char*)_filename,t_fstart,TMP_BUFSIZE ) )
	{
		osassert( false );
		return false;
	}

	memcpy(&m_header,t_fstart,sizeof(ChaHeader));

	if (m_header.mID==ChaFileID)
	{
		mHeadPicFileNamStr = (char*)(t_fstart + m_header.mHeadPicFileNameOfs);
	}
	END_USEGBUF( t_iGBufIdx );

	// River added @ 2007-6-5:加入最新cha版本的验证:
	osassertex( m_header.mVersion >= 0x109,
		va( "<%s>文件不是最新的109版本...\n",_filename ) );


	m_osSkinMeshInit.m_iTotalActTypeNum = m_header.mTotalActTypeNum;
	m_osSkinMeshInit.m_smObjLook.m_iPartNum = m_header.mPartNum;
	osassert( m_osSkinMeshInit.m_smObjLook.m_iPartNum > 0 );
	osassert( m_osSkinMeshInit.m_smObjLook.m_iPartNum <= MAX_SKINPART );

	m_osSkinMeshInit.m_vec3Max = m_header.mBoxMax;
	m_osSkinMeshInit.m_vec3Min = m_header.mBoxMin;
	m_osSkinMeshInit.m_vec3AdjPos = m_header.mPos;
	m_osSkinMeshInit.m_fAdjRotAgl = m_header.mRot;
	m_osSkinMeshInit.m_bUseDirLAsAmbient = m_header.mbUseDirLAsAmbient;
	m_osSkinMeshInit.m_bIsRide = m_header.mbIsRide;
	m_osSkinMeshInit.m_vec3RideOffset = m_header.mRidePos;

	if(m_header.mVersion>=0x108)
	{
		m_osSkinMeshInit.m_Scale = m_header.mScale;
	
		// River @ 2008-11-20:更新人物的BoundBox.
		m_osSkinMeshInit.m_vec3Max /= m_header.mScale;
		m_osSkinMeshInit.m_vec3Min /= m_header.mScale; 
	}

	// 人物是否有伴随的特效
	if (m_header.mEffectNr >= 1)
	{
		SChaEffectItem* pEffectItem;
		pEffectItem = (SChaEffectItem*)(t_fstart + m_header.mEffectOfs);
		ZeroMemory(m_osSkinMeshInit.m_szspeFileName,64);
		strncpy(m_osSkinMeshInit.m_szspeFileName,(char*)
			(t_fstart + pEffectItem->mNameOfs),pEffectItem->mNameNr);
		m_osSkinMeshInit.m_spePostion = pEffectItem->mEffectOffset;
	}
	else
		m_osSkinMeshInit.m_szspeFileName[0] = NULL;
		


	WORD mesh[MAX_SKINPART] = { 1,1,0,0,0,0,0,0 };
	WORD skin[MAX_SKINPART] = { 1,1,0,0,0,0,0,0 };
	memcpy( m_osSkinMeshInit.m_smObjLook.m_mesh , mesh, sizeof(WORD)*MAX_SKINPART );
	memcpy( m_osSkinMeshInit.m_smObjLook.m_skin , skin, sizeof(WORD)*MAX_SKINPART );



	return true;
}

const char* CMyChaFile::get_headTexName()
{
	return mHeadPicFileNamStr.c_str();
}

int CMyChaFile::get_totalActTypeNum()
{
	return m_header.mTotalActTypeNum;
}

int CMyChaFile::get_partNum()
{
	return m_header.mPartNum;
}

osVec3D& CMyChaFile::get_boxMax()
{
	return m_header.mBoxMax;
}

osVec3D& CMyChaFile::get_boxMin()
{
	return m_header.mBoxMin;
}

osVec3D& CMyChaFile::get_deathBoxMax()
{
	return m_header.mDeathBoxMax;//m_bboxDeath.set_bbMaxVec( max );
}

osVec3D& CMyChaFile::get_deathBoxMin()
{
	return m_header.mDeathBoxMin;//m_bboxDeath.set_bbMinVec( min );
}

osVec3D& CMyChaFile::get_attackedPos()
{
	return m_header.mAttackedPos;
}

//! 调入人物创建相关的资源
CMyChaFile gc_creature::m_sChaFileLoader;


template<> gc_talkStringMgr* ost_Singleton<gc_talkStringMgr>::m_ptrSingleton = NULL;


/** 得到一个sg_timer的Instance指针.
*/
gc_talkStringMgr* gc_talkStringMgr::Instance( void )
{
	if( !gc_talkStringMgr::m_ptrSingleton )
	{

		gc_talkStringMgr::m_ptrSingleton = new gc_talkStringMgr;
		
		osassert( gc_talkStringMgr::m_ptrSingleton );
	}
	
	return gc_talkStringMgr::m_ptrSingleton;
}

/** 删除一个sg_timer的Instance指针.
*/
void gc_talkStringMgr::DInstance( void )
{
	SAFE_DELETE( m_ptrSingleton );
}


gc_talkStringMgr::gc_talkStringMgr()
{
	m_vecTalkContent.resize( 64 );
}

//! 创建一个talkString. 
int gc_talkStringMgr::create_talkString( const lm_talkContent* _tc )
{
	guard;

	int  t_idx = m_vecTalkContent.get_freeNode();
	m_vecTalkContent[t_idx] = *_tc;

	return t_idx;

	unguard;
}

//! 得到某一个talkString
lm_talkContent* gc_talkStringMgr::get_talkString( int _id )
{
	guard;

	if( m_vecTalkContent.validate_id( _id ) )
		return &m_vecTalkContent[_id];

	return NULL;

	unguard;
}


//! 删除一个talkString
void gc_talkStringMgr::delete_talkString( int _id )
{
	guard;

	if( m_vecTalkContent.validate_id( _id ) )
		m_vecTalkContent.release_node( _id );

	unguard;
}



gc_creature::gc_creature(void)
{
	m_ptrSkinMesh = NULL;
	m_bInRideState = FALSE;
	m_iFakeShadowId = -1;

	reset_creature();
}

gc_creature::~gc_creature(void)
{

}


//! 重设当前的Creature.
void gc_creature::reset_creature( void )
{
	// 处于Ride状态，删除坐骑的指针
	if( m_bInRideState )
		end_ride();

	if( m_ptrSkinMesh )
	{
		// 先反注册声音相关的数据
		g_pCharSoundMgr->unregister_skinMesh(m_ptrSkinMesh);
	//	g_pCharEffectMgr->unregister_skinMesh(m_ptrSkinMesh);
		m_ptrSkinMesh->release_character();
		m_ptrSkinMesh = NULL;
	}

	m_bActive = FALSE;
	m_bInMoveStatus = FALSE;
	m_fMoveSpeed = 0.5f;
	m_fMoveDir = 0.0f;
	m_fRotAgl = 0.0f;

	m_szCurActName[0] = NULL;

	m_szCreDir[0] = NULL;

	m_bInTalkState = false;
	m_szCreatureName[0] = NULL;

	m_bInHideState = false;

	// 骑马相关
	m_bInRideState = FALSE;
	m_szRidesDir[0] = NULL;
	m_ptrRides = NULL;

	m_fEarthOffset = 0.0f;

	// 人物的似阴影
	if( m_iFakeShadowId >= 0 )
	{
		gc_gameScene::m_ptrScene->delete_shadowId( m_iFakeShadowId );
		m_iFakeShadowId = -1;
	}


}

//! 是否隐藏人物
void gc_creature::creatureHideProcess( lm_command& _command )
{
	guard;

	int   t_iHide = *(int*)_command.m_ptrCommandContent;

	if( t_iHide == 0 )
		m_bInHideState = FALSE;
	else
		m_bInHideState = TRUE;

	if( m_ptrSkinMesh )
		m_ptrSkinMesh->set_hideSkinMesh( t_iHide );

	unguard;
}


//! 得到人物应该旋转的方向
float gc_creature::get_charNewDir( osVec3D& _oldpos,osVec3D& _newpos )
{
	guard;

	osVec3D    t_vec3BaseDir( 1.0f,0.0f,0.0f );
	osVec3D    t_vec3Dir;
	BOOL       t_bVice = FALSE;

	t_vec3Dir = _newpos - _oldpos;
	t_vec3Dir.y = 0.0f;
	osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

	float  t_fAgl = osn_mathFunc::get_vectorAngle( t_vec3BaseDir,t_vec3Dir );
	if( t_vec3Dir.z > 0.0f )
		t_fAgl = 2.0f* OS_PI - t_fAgl;

	return t_fAgl;

	unguard;
}

//! 得到怪物的高度
float gc_creature::get_creatureHeight( void )
{
	guard;

	os_bbox*   t_ptrBox;

	t_ptrBox = m_ptrSkinMesh->get_smBBox();
	if( !t_ptrBox )
		return 1.8f;

	return t_ptrBox->get_vecmax()->y;

	unguard;
}



//! 从bfs文件内读入creature的相关创建信息
bool gc_creature::read_bfsInfo( os_skinMeshInit& _smInit  )
{
	guard;

	s_string     t_szBfsName;

	m_szCreatureDir = _smInit.m_strSMDir;

	// 先得到bfs文件的名字
	t_szBfsName = _smInit.m_strSMDir;
	t_szBfsName += ".cha";
	int t_iGBufIdx;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	if( file_exist( (char*)t_szBfsName.c_str() ) )
	{
		if( !m_sChaFileLoader.load_chaFile( t_szBfsName.c_str() ) )
			return false;

		_smInit = m_sChaFileLoader.get_skinMeshInit();

		m_vec3AttackPos = m_sChaFileLoader.get_startPos();
		m_vec3SufferPos = m_sChaFileLoader.get_attackedPos();

	}
	else
	{
		osn_mathFunc::change_extName( t_szBfsName,"bfs" );

		if( file_exist( (char*)t_szBfsName.c_str() ) )
		{
			if( 0 == read_fileToBuf( (char*)t_szBfsName.c_str(), t_ptrStart,TMP_BUFSIZE ) )
				osassert( false );
			sscanf( (const char*)t_ptrStart, "%d %d", 
				&_smInit.m_iTotalActTypeNum, &_smInit.m_smObjLook.m_iPartNum );
		}
		else
		{
			osDebugOut( "读入文件<%s>失败..", t_szBfsName.c_str() );
			MessageBox( NULL,va( "读入文件<%s>失败..",t_szBfsName.c_str() ),"ERROR",MB_OK );
			return false; 
		}

	}
	END_USEGBUF( t_iGBufIdx );

	for( int t_i=0;t_i<_smInit.m_smObjLook.m_iPartNum;t_i ++ )
	{
		_smInit.m_smObjLook.m_mesh[t_i] = 1;
		_smInit.m_smObjLook.m_skin[t_i] = 1;
	}

	_smInit.m_iActIdx = 0;
	_smInit.m_bShadowSupport = false; 

	return true;

	unguard;
}
//! 查找某个动作是否有声音信息
int gc_creature::find_actSndInfo( const char* _actName )
{
	guard;

	osassert( _actName );

	if( NULL == _actName[0] )
		return -1;

	for( int t_i=0;t_i<(int)this->m_vecActSound.size();t_i ++ )
	{
		if( 0 == strcmp( m_vecActSound[t_i].m_szActName,_actName ) )
			return t_i;
	}

	// 找不到声音，返回-1.
	return -1;

	unguard;
}



//! 处理人物的装备信息
void gc_creature::change_euqipment( lm_command& _command )
{
	guard;

	os_equipment*   t_ptrEquip;

	t_ptrEquip = (os_equipment*)_command.m_ptrCommandContent;

	if( m_ptrSkinMesh )
		m_ptrSkinMesh->change_equipment( *t_ptrEquip );

	unguard;
}

//! 开始人物的说话字串显示
void gc_creature::start_talkString( lm_command& _command )
{
	guard;

	const lm_talkContent* t_ptrTc = 
		(const lm_talkContent*)_command.m_ptrCommandContent;

	m_iTalkContentId = gc_talkStringMgr::Instance()->create_talkString( t_ptrTc );
	m_bInTalkState = true;

	unguard;
}
void gc_creature::framemove_talkString( float _eletime )
{
	guard;

	if( !m_bInTalkState )
		return;

	osassert( m_iTalkContentId >= 0 );
	lm_talkContent*  t_ptrTc = 
		gc_talkStringMgr::Instance()->get_talkString( m_iTalkContentId );
	
	t_ptrTc->m_fTcTime -= _eletime;
	if( t_ptrTc->m_fTcTime <= 0.0f )
		m_bInTalkState = false;

	unguard;
}


//! 开始执行怪物的休闲状态
void gc_creature::start_freeStatus( void )
{
	guard;

	if( !m_bActive )
		return;

	os_newAction    t_newAct;

	t_newAct.m_iNewAct = 0;
	t_newAct.m_strActName[0] = NULL;
	t_newAct.m_changeType = OSE_DIS_OLDACT;

	// 加入休闲动作的时间
	t_newAct.m_fFrameTime = m_ptrSkinMesh->get_actUsedTime( 0 );

	if( m_bInRideState )
	{
		osassert( this->m_ptrRides );
		m_ptrRides->change_skinMeshAction( t_newAct );
	}
	else
		m_ptrSkinMesh->change_skinMeshAction( t_newAct );

	m_szCurActName[0] = NULL;

	unguard;
}


//! 根据传入的动作名字和类型播放新的动作
void gc_creature::start_newActFromName( const char* _actName,int _type,float _scale/* = 1.0f*/ )
{
	guard;

	os_newAction        t_newAct;

	t_newAct.m_iNewAct = -1;
	strcpy( t_newAct.m_strActName,_actName );

	if( _type == 0 )
		t_newAct.m_changeType = OSE_DIS_OLDACT;
	else if( _type == 1 )
	{
		t_newAct.m_changeType = OSE_DIS_NEWACT_TIMES;
		t_newAct.m_iPlayNum = 1 ;
	}
	else 
	{
		t_newAct.m_changeType = OSE_DIS_NEWACT_LASTFRAME;
		t_newAct.m_iPlayNum = 1;
	}

	m_bActSndHavePlayed = FALSE;


	// 如果人物动作列表内没有此动作，则使用默认的待机动作
	int  t_iActIdx = 0;
	float t_fActtime = 1.0f;
	t_fActtime = m_ptrSkinMesh->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
	if( t_iActIdx < 0 )
	{
		strcpy( t_newAct.m_strActName,MONSTER_FREE );
		t_fActtime = m_ptrSkinMesh->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
		if( t_iActIdx < 0 )
		{
			strcpy( t_newAct.m_strActName,HERO_FREE );
			t_fActtime = m_ptrSkinMesh->get_actUsedTime( t_newAct.m_strActName,t_iActIdx );
			osassert( t_iActIdx >= 0 );
		}
	}

	t_newAct.m_fFrameTime = _scale * t_fActtime;;

	m_ptrSkinMesh->change_skinMeshAction( t_newAct );

	return;

	unguard;
}


//　由Console设置一个动作，然后调用一个按键后播放。
//  命令行式：caction 0-2 动作名字
//! 开始主角人物新的动作
void gc_creature::start_newAction( lm_command& _command )
{
	guard;

	lm_creatureActChange*   t_ptrActC = (lm_creatureActChange*)_command.m_ptrCommandContent;

	// 
	if( t_ptrActC->m_bUseAgl )
	{
		m_ptrSkinMesh->frame_move( m_vec3EntPos,t_ptrActC->m_fRotAgl );
		m_fRotAgl = t_ptrActC->m_fRotAgl;
	}

	start_newActFromName( t_ptrActC->m_szNewActName,
		t_ptrActC->m_iPlayActType,t_ptrActC->m_fPlayActScale );



	return;

	unguard;
}

//! 得到怪物的动作正常播放时间
float gc_creature::get_creatureActTime( const char* _actName )
{
	guard;

	int   t_iActIdx;
	float t_fActtime = 1.0f;
	t_fActtime = m_ptrSkinMesh->get_actUsedTime( _actName,t_iActIdx );

	if( t_iActIdx >= 0 )
		return t_fActtime;
	else
		return -1.0f;

	unguard;
}


//! 读入怪物声音相关的数据
void gc_creature::read_sndInfo( os_skinMeshInit& _smInit )
{
	guard;

	s_string   t_szSndName;
	BYTE*      t_fstart;
	int        t_size;

	char       t_szFileFlag[4] = {0};
	DWORD      t_dwTmp = 0;

	// 先合成snd文件名
	t_szSndName = _smInit.m_strSMDir;
	t_szSndName += CHAR_SNDFILE;

	m_vecActSound.clear();
	if( !file_exist( (char*)t_szSndName.c_str() ) )
		return;
	

	int t_iGBufIdx;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)t_szSndName.c_str(),t_fstart,TMP_BUFSIZE );
	osassert( t_size > 0 );

	READ_MEM_OFF( t_szFileFlag,t_fstart,sizeof( char )*4 );
	if( strcmp( t_szFileFlag,"cha" ) != 0 )
		osassertex( false,va( "File  <%s> format error..\n",t_szSndName.c_str()) );
	READ_MEM_OFF( &t_dwTmp,t_fstart,sizeof( DWORD ) );
	if( t_dwTmp < 0x100 )
		osassertex( false,va( "File <%s> version error...\n",t_szSndName.c_str() ) );

	// 	
	READ_MEM_OFF( &t_dwTmp,t_fstart,sizeof( DWORD ) );
	m_vecActSound.resize( t_dwTmp );
	for( DWORD  t_dw=0;t_dw<t_dwTmp;t_dw ++ )
	{
		READ_MEM_OFF( m_vecActSound[t_dw].m_szActName,t_fstart,sizeof( char )*32 );
		READ_MEM_OFF( &m_vecActSound[t_dw].m_dwNumSound,t_fstart,sizeof( DWORD ) );

		if( m_vecActSound[t_dw].m_dwNumSound > 0 )
		{
			READ_MEM_OFF( m_vecActSound[t_dw].m_vecSounds,
				t_fstart,sizeof( gcs_sound )*m_vecActSound[t_dw].m_dwNumSound );
		}
	}

	//
	END_USEGBUF( t_iGBufIdx );

	unguard;
}

const char* gc_creature::process_creatureDataDir( const char* _dir )
{
	guard;

	static s_string   t_strDir;

	t_strDir = _dir;
	int t_idx = (int)t_strDir.find( '\\' );
	t_strDir = t_strDir.substr( t_idx + 1,t_strDir.length() );

	return t_strDir.c_str();

	unguard;
}


/** \brief
*  得到旗帜数据的贴图数据
*
*  \param s_string _str 传入的三个stirng数据数组
*/
void gc_creature::get_flagTexName( lm_command& _command,s_string _str[3] )
{
	guard;

	lm_creatureInsert*  t_ptrCreature;
	t_ptrCreature = (lm_creatureInsert*)_command.m_ptrCommandContent;

	s_string   t_szDirName = t_ptrCreature->m_szChrDir;
	char       t_szTexName[64];

	// 底图.
	sprintf( t_szTexName,"%d_01_%02d.hon",
		FLAG_PART,t_ptrCreature->m_btFlagEquip[0] );
	_str[0] = t_ptrCreature->m_szChrDir;
	_str[0] += "\\";
	_str[0] += t_szTexName;

	// 第二层
	sprintf( t_szTexName,"second_%02d.hon",t_ptrCreature->m_btFlagEquip[1] );
	_str[1] = t_ptrCreature->m_szChrDir;
	_str[1] += "\\";
	_str[1] += t_szTexName;

	// 第三层
	sprintf( t_szTexName,"third_%02d.hon",t_ptrCreature->m_btFlagEquip[2] );
	_str[2] = t_ptrCreature->m_szChrDir;
	_str[2] += "\\";
	_str[2] += t_szTexName;

	return;

	unguard;
}


//! 创建当前的人物指针
BOOL gc_creature::create_creature( lm_command& _command )
{
	guard;

	os_skinMeshInit     t_sSmInit;

	lm_creatureInsert*  t_ptrCreature;

	t_ptrCreature = (lm_creatureInsert*)_command.m_ptrCommandContent;

	strcpy( m_szCreDir,process_creatureDataDir( t_ptrCreature->m_szChrDir ) );
	strcpy( t_sSmInit.m_strSMDir,t_ptrCreature->m_szChrDir );
	if( !read_bfsInfo( t_sSmInit ) )
		return false;
	strcpy( t_sSmInit.m_strSMDir,t_ptrCreature->m_szChrDir );

	// 人物的三维位置
	m_bRightCrePos = true;
	m_bRightCrePos = gc_gameScene::m_ptrScene->get_tilePos( 
		t_ptrCreature->m_iXPos,t_ptrCreature->m_iZPos,t_sSmInit.m_vec3Pos );

	// 如果是创建旗帜类型的skinMesh.
	if( t_ptrCreature->m_bFlagType )
		t_sSmInit.m_iMlFlagIdx = FLAG_PART - 1;

	t_sSmInit.m_fAngle = t_ptrCreature->m_fRotAgl;

	// River @ 2007-3-22: 加入缩放选项:
	t_sSmInit.m_Scale *= t_ptrCreature->m_fScale;
	t_sSmInit.m_bInitFade = false;

	//t_sSmInit.m_smObjLook.m_iPartNum = 8;
	//t_sSmInit.m_smObjLook.m_mesh[7] = 0;
	//t_sSmInit.m_smObjLook.m_skin[7] = 0;


	m_ptrSkinMesh = gc_gameScene::m_ptrScene->create_character( &t_sSmInit );
	osassert( m_ptrSkinMesh );

	while( !m_ptrSkinMesh->legal_operatePtr() )
		::Sleep( 1 );



	if( t_ptrCreature->m_bFlagType )
	{
		s_string   t_szTexName[3];
		get_flagTexName( _command,t_szTexName );
		m_ptrSkinMesh->change_mlFlagTexture(
			t_szTexName[0].c_str(),t_szTexName[1].c_str(),t_szTexName[2].c_str() );
	}
	else
	{
		g_pCharSoundMgr->register_skinMesh(m_ptrSkinMesh,t_sSmInit.m_strSMDir);
//		g_pCharEffectMgr->register_skinMesh(m_ptrSkinMesh,t_sSmInit.m_strSMDir);
		// Create 可以使用武器特效
		//m_ptrSkinMesh->playSwordGlossy( true,true );
	}

	// 读入声音相关的信息
	read_sndInfo( t_sSmInit );

	m_vec3EntPos = t_sSmInit.m_vec3Pos;
	m_bActive = TRUE;

	// catch creature name.
	strcpy( m_szCreatureName,t_ptrCreature->m_szCreatureName );



	// 创建怪物假阴影
	m_iFakeShadowId = gc_gameScene::m_ptrScene->create_fakeShadow( m_vec3EntPos,3.0f );
	osassert( m_iFakeShadowId >= 0 );


	return TRUE;

	unguard;
}


//! 执行指令的接口
BOOL gc_creature::process_command( lm_command& _command )
{
	guard;

	return TRUE;

	unguard;
}

//! 得到creatrue的名字
const char* gc_creature::get_creatureName( void )
{ 
	if( m_bInHideState )
		return NULL;

	if( m_bInTalkState )
	{
		lm_talkContent*   t_ptrTc =
			gc_talkStringMgr::Instance()->get_talkString( m_iTalkContentId );

		return t_ptrTc->m_szTalkContent;
	}
	else
		return m_szCreatureName; 
}


/** \brief
 *  怪物骑乘开始
 *
 *  怪物的骑乘，分以下的步骤:
 *  1: 先创建一个骑乘的skinMesh指针。 
 *  2: 设置当前的creature骑乘此skinMesh.
 *  3: 设置当前creature相应的状态，如果需要移动人物，
 *     则移动坐骑的位置即可。
 */ 
BOOL gc_creature::start_ride( lm_command& _command )
{
	guard;

	// 创建骑乘skinMesh的设备资源
	const lm_effectPlay* t_ptrFName = (const lm_effectPlay*)_command.m_ptrCommandContent;
	os_skinMeshInit     t_sSmInit;

	m_fEarthOffset = t_ptrFName->m_vec3Pos.x;

	strcpy( t_sSmInit.m_strSMDir,t_ptrFName->m_szEffName );
	if( !read_bfsInfo( t_sSmInit ) )
		return false;
	strcpy( t_sSmInit.m_strSMDir,t_ptrFName->m_szEffName );

	get_entityPos( t_sSmInit.m_vec3Pos );
	t_sSmInit.m_vec3Pos.y += this->m_fEarthOffset;
	m_vec3EntPos = t_sSmInit.m_vec3Pos;

	t_sSmInit.m_fAngle = m_fRotAgl;

	// River @ 2007-5-25:让骑乘的怪物有阴影显示。
	t_sSmInit.m_bShadowSupport = TRUE;

	// 先释放
	if( m_ptrRides )
	{
		m_ptrRides->release_character();
		m_ptrRides = NULL;
	}
	
	t_sSmInit.m_bIsRide = true;
	t_sSmInit.m_fAngle = 0.0f;	
	t_sSmInit.m_bInitFade = false;

	m_ptrRides = gc_gameScene::m_ptrScene->create_character( &t_sSmInit,false,true );
	osassert( m_ptrRides );

	// 设置当前creature骑乘
	m_ptrSkinMesh->SetRide( m_ptrRides );
	start_newActFromName( RIDE_ACT,0 );

	// 
	m_bInRideState = TRUE;

	g_bHeroShadow = true;

	return true;

	unguard;
}
//! 结束骑乘
BOOL gc_creature::end_ride( void )
{
	guard;

	// 删除相应的坐骑指针关联
	m_ptrSkinMesh->DelRide();
	m_ptrRides->release_character();
	m_ptrRides = NULL;

	m_bInRideState = false;
	m_fEarthOffset = 0.0f;
	m_szRidesDir[0] = NULL;

	g_bHeroShadow = true;

	// 重设人物的位置。
	gc_gameScene::m_ptrScene->get_detailPos( m_vec3EntPos );
	m_ptrSkinMesh->frame_move( m_vec3EntPos,m_fRotAgl );

	start_freeStatus();

	return true;

	unguard;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//
// 场景内Hero相关的处理.
//
//////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////////////////
//
// 整个游戏内的Creature的管理类
//
//////////////////////////////////////////////////////////////////////////////////////////////////
gc_creatureMgr::gc_creatureMgr()
{
	m_arrMonster.resize( INIT_MONSTERSIZE );

	this->m_ptrSelCreature = NULL;

	m_iFlagId = -1;
}

gc_creatureMgr::~gc_creatureMgr()
{

}

//! 得到当前主角人物和选中的怪物的指针
lm_gameEntity* gc_creatureMgr::get_heroEntity( void )
{
	guard;

	return (lm_gameEntity*)&this->m_sHero;

	unguard;
}

lm_gameEntity* gc_creatureMgr::get_selCreatureEntity( void )
{
	guard;

	return (lm_gameEntity*)get_activeSelMonster();

	unguard;
}

//! 从一个handle，得到此怪物所在的资源目录
const char* gc_creatureMgr::get_creatureStr( int _handle )
{
	guard;

	if( m_arrMonster.validate_id( _handle ) )
		return m_arrMonster[_handle].get_creatureDir();
	else
		return NULL;

	unguard;
}

//! 设置管理器内选中的怪物为空
void gc_creatureMgr::set_nullSelCreature( void ) 
{ 
	m_ptrSelCreature = NULL; 
	m_listMultiSel.clear();
}


//! 从一个handle和一个动作名，得到一个动作正常播放需要的时间,如果时间为-1,则没有此动作
float gc_creatureMgr::get_creatureActTime( int _handle,const char* _acttime )
{
	guard;

	// -1表示主角人物的handle.
	if( _handle == -1 )
		return this->m_sHero.get_creatureActTime( _acttime );

	if( m_arrMonster.validate_id( _handle ) )
		return m_arrMonster[_handle].get_creatureActTime( _acttime );
	else
		return -1.0f;

	unguard;
}


//! 得到主角人物的位置.
void gc_creatureMgr::get_heroFocusPos( osVec3D& _pos )
{
	m_sHero.get_entityPos( _pos );
	_pos.y += gc_camera::m_fFocusOffset;
	return;
}

//! 重设主角人物的位置
void gc_creatureMgr::reset_heroPos( int _x,int _z )
{
	m_sHero.reset_heroPos( _x,_z );
}

//! 重设主角人物的位置
void gc_creatureMgr::reset_heroPos( osVec3D& _pos )
{
	m_sHero.reset_heroPos( _pos );
}


//! 场景内插入一个怪物
gc_monster* gc_creatureMgr::get_freeMonstIns( int& _idx )
{
	guard;

	gc_monster*   t_ptrMonster;
	_idx = m_arrMonster.get_freeNodeAUse( &t_ptrMonster );

	return t_ptrMonster;

	unguard;
}

//! 对鼠标左键点击的处理
void gc_creatureMgr::process_lbuttonDown( lm_command& _command )
{
	guard;

	os_tileIdx*     t_sMousePos;
	osVec2D         t_vec2D;
	osVec3D         t_vec3Dir,t_vec3CamPos;
	osMatrix        t_mat;

	t_sMousePos = (os_tileIdx*)_command.m_ptrCommandContent;
	osassert( (t_sMousePos->m_iX>=0)&&(t_sMousePos->m_iY>=0) );

	osMatrixIdentity( &t_mat );
	t_vec2D.x = t_sMousePos->m_iX;
	t_vec2D.y = t_sMousePos->m_iY;
	t_vec3Dir = gc_camera::m_ptrCamera->get_upprojvec( &t_vec2D,t_mat );
	gc_camera::m_ptrCamera->get_curpos( &t_vec3CamPos );

	// 备份当前的选中怪物指针
	gc_creature* t_ptrCreatureBack = m_ptrSelCreature;
	gc_creature* t_ptrNewActive = NULL;

	t_ptrNewActive = NULL;
	m_bPickMonster = FALSE;

	float    t_fMinDis = 1000000.0f;
	CSpaceMgr<gc_monster>::NODE   t_ptrNode;
	for( t_ptrNode = m_arrMonster.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrMonster.next_validNode( &t_ptrNode ) )
	{
		float         t_fDis;

		// 上层删除了此skinMesh,但为了使其它的id保持不变,不删除这个Node.
		if( !t_ptrNode.p->m_ptrSkinMesh )
			continue;

		if( !t_ptrNode.p->m_ptrSkinMesh->get_smBBox()->ray_intersect(
			   t_vec3CamPos,t_vec3Dir,t_fDis ) )
			   continue;

		if( t_fDis < t_fMinDis )
		{
			t_fMinDis = t_fDis;
			t_ptrNewActive = (gc_creature*)t_ptrNode.p;
			m_bPickMonster = TRUE;
		}
	}


	//
	// 使用此次点击前选中的怪物指针.
	if( lm_gvar::g_bCtrlDown )
	{
		if( NULL == m_ptrSelCreature )
			m_ptrSelCreature = t_ptrNewActive;
		else
			m_listMultiSel.push_back( t_ptrNewActive );
	}
	else
	{
		if( t_ptrNewActive == NULL )
			m_ptrSelCreature = t_ptrCreatureBack;
		else
			m_ptrSelCreature = t_ptrNewActive;
	}


	return ;

	unguard;
}


//! 删除怪物管理器所有的怪物
void gc_creatureMgr::del_allCreature( void )
{
	guard;

	CSpaceMgr<gc_monster>::NODE   t_ptrNode;
	for( t_ptrNode = m_arrMonster.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrMonster.next_validNode( &t_ptrNode ) )
	{
		t_ptrNode.p->reset_monster();
		m_arrMonster.release_node( t_ptrNode.idx );
	}

	unguard;
}


//! 根据传入的id来删除怪物
bool gc_creatureMgr::del_creatureByHandle( int _handle )
{
	guard;

	if( m_arrMonster.validate_id( _handle ) )
	{
		m_arrMonster[_handle].reset_monster();
	}

	return true;

	unguard;
}


//! 根据传入的handle，来得到我们需要的怪物
gc_creature* gc_creatureMgr::get_creaturePtrByHandle( int _handle )
{
	guard;

	if( m_arrMonster.validate_id( _handle ) )
		return &m_arrMonster[_handle];
	else
		return NULL;

	unguard;
}


/** \brief
 *  调试用途函数，画出所有人物的bounding box,
 *
 *  这个函数的调用必须在beginScene和endScene之间
 */
void gc_creatureMgr::draw_creatureBBox( void )
{
	guard;

	// 如果
	if( !g_bDrawCreatureBBox )
	{
		if( m_ptrSelCreature && m_ptrSelCreature->m_ptrSkinMesh )
		{
			gc_gameScene::m_ptrScene->draw_bbox( 
				m_ptrSelCreature->m_ptrSkinMesh->get_smBBox(),0xffff0000 );
		}
		
	}

	if( m_sHero.is_activeChar() && g_bDrawCreatureBBox )
	{
		os_bbox   t_sBox;
		
		t_sBox.set_bbMaxVec( osVec3D( 0.1f,0.1f,0.1f ) );
		t_sBox.set_bbMinVec( osVec3D( -0.1f,-0.1f,-0.1f ) );
		osVec3D  t_vec3Pos( 0.0f,0.0f,0.0f );
		m_sHero.m_ptrSkinMesh->get_bonePos( "Bip01 R Toe0Nub",t_vec3Pos );
		t_sBox.set_bbPos( t_vec3Pos );
		gc_gameScene::m_ptrScene->
			draw_bbox( &t_sBox,0xffff0000 );
	}

	// 画怪物		
	if( g_bDrawCreatureBBox )
	{
		CSpaceMgr<gc_monster>::NODE   t_ptrNode;
		for( t_ptrNode = m_arrMonster.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = m_arrMonster.next_validNode( &t_ptrNode ) )
		{
			// 只有指针有效才能画bbox,如果怪物刷在了不可以通过的地方，则此怪物显示成绿色
			if( t_ptrNode.p->m_ptrSkinMesh )
			{
				DWORD       t_dwDisColor = 0xffffff00;
				os_tileIdx  t_sIdx;
				osVec3D     t_vec3Pos;
				float       t_fAngle;

				osassert( t_ptrNode.p->m_ptrSkinMesh );

				t_ptrNode.p->m_ptrSkinMesh->get_posAndAgl( t_vec3Pos,t_fAngle );
				t_sIdx.m_iX = int(t_vec3Pos.x/HALF_TWIDTH);
				t_sIdx.m_iY = int(t_vec3Pos.z/HALF_TWIDTH);

				if( !gc_gameScene::m_ptrScene->collision_detection( t_sIdx ) )
					t_dwDisColor = 0xff00ff00;
				
				gc_gameScene::m_ptrScene->
					draw_bbox( t_ptrNode.p->m_ptrSkinMesh->get_smBBox(),t_dwDisColor );
			}
		}

		/*
		// 画特效的creature box.
		if( lm_gvar::g_iEffectId >= 0 )
		{
			const os_bbox*  t_sBBox;
			osVec3D  t_vec3Pos,t_vec3Size;
			t_sBBox = gc_gameScene::m_ptrScene->get_effectBox( lm_gvar::g_iEffectId );

			gc_gameScene::m_ptrScene->
				draw_bbox( (os_bbox*)t_sBBox,0xffffffff );

		}
		*/

	}

	unguard;
}

//! 画出整个怪物管理器内，所有creature的名字
void gc_creatureMgr::draw_creatureName( I_fdScene* _scene )
{
	guard;

	const char*   t_ptrCName;
	osVec3D       t_vec3Pos,t_vec3ScrPos;
	float         t_fHeight;
	os_stringDisp t_ptrStr;
	int           t_iXOffset,t_iYOffset;

	CSpaceMgr<gc_monster>::NODE   t_ptrNode;
	for( t_ptrNode = m_arrMonster.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrMonster.next_validNode( &t_ptrNode ) )
	{
		// 得到creature的名字
		t_ptrCName = t_ptrNode.p->get_creatureName();

		if( NULL == t_ptrCName )
			continue;

		if( t_ptrCName[0] == NULL )
			continue;

		t_ptrNode.p->get_entityPos( t_vec3Pos );
		t_fHeight = t_ptrNode.p->get_creatureHeight();
		t_vec3Pos.y += t_fHeight;

		gc_camera::m_ptrCamera->get_scrCoodFromVec3( t_vec3Pos,t_vec3ScrPos );	

		// 不需要画，处于相机不可见的位置
		if( t_vec3ScrPos.z >= 1.0f )
			continue;

		t_iXOffset = int(strlen( t_ptrCName ))*12/4;
		t_iYOffset = 6;

		t_ptrStr.charHeight = 14;
		t_ptrStr.color = 0xffffffff;
		t_ptrStr.x = (int)t_vec3ScrPos.x - t_iXOffset;
		t_ptrStr.y = (int)t_vec3ScrPos.y - t_iYOffset;
		t_ptrStr.z = t_vec3ScrPos.z;
		t_ptrStr = t_ptrCName;

		gc_gameScene::m_ptrScene->disp_string( &t_ptrStr,1 ,0);
	}
	//@{

	//windy test
	//!windy test
	static float t_fRot = 0.0f;

	os_decalInit   t_init;

	static int t_id = 0;
	static osVec3D   t_pos;

	m_sHero.get_entityPos( t_init.m_vec3Pos );

	t_init.m_fRotAgl = 0;
	t_init.m_fSize = 40.0f;
	strcpy( t_init.m_szDecalTex,"groupAttack" );
	t_init.m_fVanishTime = 0.f;



	//@}

	// 画主角人物的名字和说话信息
	t_ptrCName = m_sHero.get_creatureName();
	if( NULL == t_ptrCName )
		return;
	if( t_ptrCName[0] == NULL )
		return;
	m_sHero.get_entityPos( t_vec3Pos );
	
	t_fHeight = m_sHero.get_creatureHeight();
	t_vec3Pos.y += t_fHeight;

	gc_camera::m_ptrCamera->get_scrCoodFromVec3( t_vec3Pos,t_vec3ScrPos );	

	// 不需要画，处于相机不可见的位置
	if( t_vec3ScrPos.z >= 1.0f )
		return;

	t_iXOffset = int(strlen( t_ptrCName ))*12/4;
	t_iYOffset = 6;

	t_ptrStr.charHeight = 14;
	t_ptrStr.color = 0xffffffff;
	t_ptrStr.x = (int)t_vec3ScrPos.x - t_iXOffset;
	t_ptrStr.y = (int)t_vec3ScrPos.y - t_iYOffset;
	t_ptrStr.z = t_vec3ScrPos.z;
	t_ptrStr = t_ptrCName;

	gc_gameScene::m_ptrScene->disp_string( &t_ptrStr,1 ,0);
	os_stringDisp t_str;
//	t_str.charHeight = 12;
//	//t_str.color = 0xffffffff;
//	t_str.m_iMaxPixelX = 10030;
//	t_str.m_iMaxPixelY = 10030;
	t_str.x = 25;
	t_str.y = 25;
	t_str.charHeight = 12;
//	t_str.m_iResRowNum = 5;
//	t_str.m_bForceDis = 1;
//	t_str.m_iRowDis = 2;
	t_str.z = t_vec3ScrPos.z;
	char temp[4];
	temp[0] = 'a';
	temp[1] = 'a';
	temp[2] = 'a';
	temp[3] = 0x7b;
	//temp[3] = '\0';
	t_str = temp;
	gc_gameScene::m_ptrScene->disp_string( &t_str,1);


	unguard;
}



//! 对场景内每一个活动的creature进行FrameMove
BOOL gc_creatureMgr::frame_move( void )
{
	guard;

	float   t_fEleTime = sg_timer::Instance()->get_lastelatime();


	
	// 如果激活，FrameMove
	if( m_sHero.is_activeChar() )
		m_sHero.frame_move( t_fEleTime );


	// 
	// FrameMove场景内的怪物,可视范围内。
	CSpaceMgr<gc_monster>::NODE   t_ptrNode;
	for( t_ptrNode = m_arrMonster.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrMonster.next_validNode( &t_ptrNode ) )
		t_ptrNode.p->frame_move( t_fEleTime );


	//
	m_bPickMonster = FALSE;
	
	return true;

	unguard;
}




//! 执行指令的接口
BOOL gc_creatureMgr::process_command( lm_command& _command )
{
	guard;

	//
	// 更新主角人物相关的内容
	if( (_command.m_eCommandId>LM_HEROSTART)&&
		(_command.m_eCommandId<LM_HEROEND) )
	{
		m_sHero.process_command( _command );
	}

	if(_command.m_eCommandId == LM_SCENE_LBUTTONDOWN )
		process_lbuttonDown( _command );


	// 怪物相关
	if( (_command.m_eCommandId>LM_MONSTERSTART)&&
		(_command.m_eCommandId<LM_MONSTEREND) )
	{
		// 插入怪物
		if( LM_MONSTERINSERT == _command.m_eCommandId )
		{
			gc_monster*   t_ptrMonster;
			int           t_iHandle;
			lm_creatureInsert*  t_ptrCreature = (lm_creatureInsert*)_command.m_ptrCommandContent;
			t_ptrMonster = get_freeMonstIns( t_iHandle );

			// 旗帜场景内只出现一个
			if( t_ptrCreature->m_bFlagType )
			{
				if( m_iFlagId != -1 )
				{
					m_arrMonster[m_iFlagId].reset_monster();
					m_arrMonster.release_node( m_iFlagId );

				}

				// 如果是要删除旗帜，则此处处理删除相关的工作
				s_string   t_szStr = t_ptrCreature->m_szChrDir;
				if( t_szStr.find( "delflag" ) != -1 )
				{
					m_iFlagId = -1;
					m_arrMonster.release_node( t_iHandle );
					return true;
				}

				m_iFlagId = t_iHandle;
			}

			if( !t_ptrMonster->process_command( _command ) )
				m_arrMonster.release_node( t_iHandle );
			else
			{
				// 新插入的怪物做为默认的选中怪物
				m_ptrSelCreature = t_ptrMonster;
				lm_gvar::g_iCreatureHandle = t_iHandle;
			}
		}

		// 操作怪物,选中怪物的动作播放
		// 选中怪物的移动
		// 怪物装备切换
		if( (LM_MONSTERACT == _command.m_eCommandId) ||
			(LM_MONSTERWALK == _command.m_eCommandId)||
			(LM_MONSTEREQUIP == _command.m_eCommandId)||
			(LM_MONSTERRUNACTTIME == _command.m_eCommandId) ||
			(LM_MONSTERRUNSPEED == _command.m_eCommandId) ||
			(LM_MONSTERSAY == _command.m_eCommandId ) ||
			(LM_MONSTERHIDE == _command.m_eCommandId ) ||
			(LM_MONSTERRIDE == _command.m_eCommandId)  ||
			(LM_MONSTERUNRIDE == _command.m_eCommandId)  )
		{
			if( _command.m_iSHandle == -1 )
			{
				if( m_ptrSelCreature )
					m_ptrSelCreature->process_command( _command );
			}
			else
				m_arrMonster[_command.m_iSHandle].process_command( _command );
		}

		// 
		// 删除当前的monster.
		if( LM_MONSTERDEL == _command.m_eCommandId )
		{
			osassert( _command.m_iSHandle >= 0 );
			m_arrMonster[_command.m_iSHandle].process_command( _command );
			m_arrMonster.release_node( _command.m_iSHandle );
		}
	}


	return true;

	unguard;
}

//! 测试用
I_skinMeshObj* gc_creatureMgr::get_heroPtr( void )
{
	guard;

	return m_sHero.m_ptrSkinMesh;

	return NULL;

	unguard;
}


