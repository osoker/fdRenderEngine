///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_command.cpp
 *
 *  Desc:     魔法demo客户端的消息处理基础，程序内的每一个元素都处理自己能够处理的命令或是消息。
 * 
 *  His:      River created @ 2006-4-14
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "gc_command.h"
# include "gameApp.h"
# include "gc_creature.h"
# include "lm_configInfoReader.h"

//! 默认的怪物所在的目录
# define DEFAULT_MONSTDIR   "character\\"

//! 测试Demo内常用的一些全局变量
int lm_gvar::g_iHeroPosX = 0;
int lm_gvar::g_iHeroPosZ = 0;

//! 主角人物的详细位置信息.
float lm_gvar::g_fHeroPosX = 0.0f;
float lm_gvar::g_fHeroPosZ = 0.0f;


//! 是否显示怪物的名字。
BOOL lm_gvar::g_bDispMonsterName = TRUE;

//! 上次播放正确的技能id.
s_string lm_gvar::g_szLastSkill = "";

//! 当前是否截屏
BOOL lm_gvar::g_bCatchScreen = false;

//! 全局的handle,每创建完一个人物，设置此handle
int  lm_gvar::g_iCreatureHandle = -1;

//! 相机的移动速度
float lm_gvar::g_fCamMoveSpeed = 5.0f;

//! 是否打开怪物的自动发魔法功能
bool  lm_gvar::g_bRandSkillPlay = false;

//! 用于控制人物施法动作的速度
float lm_gvar::g_fSkillActSpeed = 1.0f;

//! 是否显示调试信息
BOOL  lm_gvar::g_bRenderDebugInfo = TRUE;

//! 是否在调入地图后，接下来遍历整个地图，用于检测地图数据的完整性。
BOOL  lm_gvar::g_bSpaceMoveWholeMap = FALSE;

//! 是否需要调整地图的碰撞信息
BOOL  lm_gvar::g_bAdjMapCollInfo = FALSE;

//! 是否鼠标移动时,在鼠标的位置显示周围格子的碰撞信息
BOOL  lm_gvar::g_bDisplayColInfo = FALSE;

//! 画出场景内所有怪物的视野信息
BOOL  lm_gvar::g_bDisplayAllMonInfo = FALSE;

//! 按鼠标键时, 是否按下ctrl键.
BOOL  lm_gvar::g_bCtrlDown = FALSE;


//! 是否显示触发矩离
BOOL  lm_gvar::g_bDisTrigger = TRUE;
//! 是否显示主动视野
BOOL  lm_gvar::g_bDisVision = TRUE;
//! 是否显示逃跑矩离.
BOOL  lm_gvar::g_bDisRun = TRUE;
//! 是否显示呼叫矩离.
BOOL  lm_gvar::g_bDisCall = TRUE;

//! 是否查看怪物的正常步时间.
BOOL  lm_gvar::g_bNormalMoveTime = TRUE;

//! 插入怪物后,是否选中当前插入的怪物
BOOL  lm_gvar::g_bSelLastInsertMonst = FALSE;


//! 是否相机跟随人物
BOOL lm_gvar::g_bCamFollowHero = TRUE;


//! 整个场景时间停止
BOOL lm_gvar::g_bSuspendTime = FALSE;

//
//! 是否显示二维的人物
BOOL lm_gvar::g_bDisplayCharInRect = FALSE;

//! 全局的特效id,用于测试包围盒.
int  lm_gvar::g_iEffectId = -1;


//!
# define C_VARIABLE_KEY   "变量"
# define E_VARIABLE_KEY   "var"
//! 用于必须有两个变量的一些命令
# define C_TARGET_KEY     "目标"
# define E_TARGET_KEY     "target"

static s_string   t_szDictionary[] = 
{
"move","monster","herospeed","camcol","testact",
"monstact","testskill","ambient","dirlight","fogcolor",
"fogdis","playeff","music","job","weapon",
"equip","weather","heromove",// 主角人物跑动
"monstermove",   // 场景内怪物的跑动
"monsterequip",  // 场景内怪物的装备变化
"charhide",      // 隐藏主角人物
"namedis",    // 是否显示插入场景内的怪物名字
"nameequip",  // 使用名字来得到装备
"camfollow",  // 是否使用相机的自动跟随功能
"camspeed",   // 手工控制相机的移动速度
"randskill",  // 场景内的怪物朝人物发出随机的魔法
"statemagic", // 场景内主角人物和怪物的状态魔法
"skillspeed", // 场景内人物和怪物施放魔法时的速度

"setmovetime",   // 设置场景内某个怪物的移动动作的时间
"setmovespeed",  // 设置场景内某个怪物的移动速度。
"flag",          // 在场景内创建一个旗帜
"say",           // 人物说话的功能，即在头顶显示人物要说的话？
"ride",          // 主角人物和怪物的骑乘命令
"unride",        // 结束骑乘状态
"del",           // 删除场景内的一个怪物。
"cursor",        // 切换当前的鼠标光标
"cursortime",    // 光标动画每一帧的切换时间,以秒为单位
"debuginfo",     // 是否在屏幕上显示调试信息

"savecam",       // 存储相机到一个文件
"loadcam",       // 调入相机

};

static s_string   t_szDictionaryChn[] =
{
"自由移动","刷新怪物","改变速度","camcol","查看动画",
"怪物动画",
"测试技能",
"改变全局光",
"改变方向光",
"改变雾色",
"调整雾距",
"查看特效",
"背景音乐",
"改变角色",
"调换武器",
"调换装备",
"改变天气",
"人物移动",
"怪物移动",
"怪物装备",
"隐藏主角",
"名字显示",
"装备名字",
"相机跟随",
"相机速度",
"随机技能",
"状态魔法",
"施法速度",
"移动间隔",
"移动步幅",
"创建旗帜",
"怪物说话",
"怪物骑乘",
"结束骑乘",
"删除怪物",
"切换光标",
"光标时间",
"调试信息",
"存储相机",
"调入相机",

};

lm_talkContent& lm_talkContent::operator=( const lm_talkContent& _tc )
{
	strcpy( m_szTalkContent,_tc.m_szTalkContent );
	m_fTcTime = _tc.m_fTcTime;

	return *this;
}


lm_gvar::lm_gvar()
{
	FILE*   t_file = fopen( "gvar.var","rt" );
	if( t_file != NULL )
	{
		char   t_sz[128];
		fscanf( t_file,"%d\n",t_sz );//&t_ver,sizeof( int ),1,t_file );
		fscanf( t_file,"%s",t_sz );
		lm_gvar::g_szLastSkill = t_sz;
		fclose( t_file );
	}
}
lm_gvar::~lm_gvar()
{
	FILE*   t_file = fopen( "gvar.var","wt" );
	if( t_file != NULL )
	{
		DWORD  t_ver = 1;
		fprintf( t_file,"%d\n",1 );
		fprintf( t_file,"%s",lm_gvar::g_szLastSkill.c_str() );
		fclose( t_file );
	}
}

static lm_gvar   t_sGvar;

const char* lm_cmdFileName::operator=( const char* _fname )
{
	osassert( strlen( _fname ) <= 64 );
	strcpy( m_szFileName,_fname );
	return m_szFileName;
}


//! 整个场景内的entity数目
int lm_gameEntity::m_iEntityNum = 0;

lm_gameEntity::lm_gameEntity()
{
	m_iEntityNum ++;

	m_vec3EntPos = osVec3D( 0.0f,0.0f,0.0f );
}

lm_gameEntity::~lm_gameEntity()
{
	m_iEntityNum --;
}

//! 初始化静态变量
template<> lmc_commandMgr* ost_Singleton<lmc_commandMgr>::m_ptrSingleton = NULL;

lmc_commandMgr::lmc_commandMgr()
{
	m_iCmdPtr = 0;
	m_ptrCmdCntPtr = 0;
	m_vecCmdQueue.resize( INIT_APP_COMMAND );
	m_ptrCmdContentBuf = new BYTE[INIT_APP_CMDBUF];

	reset_autoCmdQueue();
}

lmc_commandMgr::~lmc_commandMgr()
{
	SAFE_DELETE_ARRAY( m_ptrCmdContentBuf );
}


/** 得到一个sg_timer的Instance指针.
*/
lmc_commandMgr* lmc_commandMgr::Instance( void )
{
	if( !m_ptrSingleton )
		m_ptrSingleton = new lmc_commandMgr;
	return m_ptrSingleton;

}

/** 删除一个sg_timer的Instance指针.
*/
void lmc_commandMgr::DInstance( void )
{
	SAFE_DELETE( m_ptrSingleton );
}

//! 立即执行一个命令:
void lmc_commandMgr::do_commandImm( lm_command& _cmd )
{
	guard;

	g_ptrGameApp->do_commandImm( &_cmd );

	unguard;
}


//! 插入一个命令
void lmc_commandMgr::push_command( lm_command& _cmd )
{
	guard;

	// 如果超出，暂不处理此命令
	if( m_iCmdPtr >= INIT_APP_COMMAND )
	{
		MessageBox( NULL,"命令数目过多，请咨询程序...","出错了",MB_OK );
		return;
	}

	lm_command*   t_ptrDst = &m_vecCmdQueue[m_iCmdPtr];

	t_ptrDst->m_eCommandId = _cmd.m_eCommandId;
	t_ptrDst->m_dwCmdContentSize = _cmd.m_dwCmdContentSize;
	t_ptrDst->m_ptrCommandContent = &m_ptrCmdContentBuf[m_ptrCmdCntPtr];
	t_ptrDst->m_iSHandle = _cmd.m_iSHandle;
	t_ptrDst->m_iEHandle = _cmd.m_iEHandle;

	if( 0 == t_ptrDst->m_dwCmdContentSize )
	{
		t_ptrDst->m_ptrCommandContent = NULL;
	}
	else
	{
		osassert( (m_ptrCmdCntPtr + t_ptrDst->m_dwCmdContentSize)<INIT_APP_CMDBUF );
		memcpy( &m_ptrCmdContentBuf[m_ptrCmdCntPtr],
			_cmd.m_ptrCommandContent,_cmd.m_dwCmdContentSize );
	}

	// 移动指针
	m_iCmdPtr ++;
	m_ptrCmdCntPtr += _cmd.m_dwCmdContentSize;

	unguard;
}


//! 得到第n个命令的指针
lm_command* lmc_commandMgr::get_cmdIdx( int _idx ) 
{ 
	guard;
	osassert( _idx >= 0 );
	osassert( _idx < this->m_iCmdPtr );

	return &m_vecCmdQueue[_idx]; 

	unguard;
}


//! 分析命令字串.
const char* lmc_commandMgr::get_cmdStr( const char* _cmd )
{
	guard;

	static s_string    t_str;

	osassert( _cmd );
	osassert( _cmd[0] != NULL );

	t_str = "";
	for( DWORD t_i=0;t_i<strlen( _cmd );t_i ++ )
	{
		if( _cmd[t_i] != ' ' )
			t_str.push_back( _cmd[t_i] );
		else
			break;
	}
	
	return t_str.c_str();

	unguard;
}

const char* lmc_commandMgr::get_cmdContent( const char* _cmd )
{
	guard;

	static s_string    t_str;

	t_str = _cmd;
	int t_idx = (int)t_str.find( ' ' );

	t_str = "";
	while( _cmd[t_idx] == ' ' )
		t_idx ++;

	for( DWORD t_i=t_idx;t_i<strlen( _cmd );t_i ++ )
		t_str.push_back( _cmd[t_i] );

	return t_str.c_str();

	unguard;
}

//! 查找第n个命令数据块的开始位置
int lmc_commandMgr::get_keyStartPos( const char* _str,int _idx )
{
	guard;

	s_string     t_strSource = _str;
	int          t_idx;


	if( strlen( _str ) == 0 )
		return 0;

	t_idx = 0;
	for( int t_i=0;t_i<_idx;t_i ++ )
	{		
		while( t_strSource.at( t_idx ) == ' ' )
			t_idx ++;

		while( t_strSource.at( t_idx ) != ' ' )
			t_idx ++;
	}

	while( t_strSource.at( t_idx ) == ' ' )
		t_idx ++;

	return t_idx;

	unguard;
}


//! 读入字串的第n个数据，直到碰到空格
const char* lmc_commandMgr::get_keyval( const char* _str,int _idx )
{
	guard;

	s_string     t_strSource = _str;
	int          t_idx;

	static  s_string   t_strRes;
	t_strRes = "";

	if( strlen( _str ) == 0 )
		return t_strRes.c_str();

	for( int t_i=0;t_i<_idx;t_i ++ )
	{		
		t_idx = 0;
		while( t_strSource.at( t_idx ) == ' ' )
		{
			t_idx ++;

			// 用于消除最后的空格
			if( t_strSource.length() == t_idx )
				break;
		}

		// 用于消除最后的空格
		if( t_strSource.length() == t_idx )
			break;

		t_strSource = t_strSource.substr( t_idx,t_strSource.length() );

		int t_idx = (int)t_strSource.find( ' ' );
		if( t_idx == -1 )
			return NULL;

		t_strSource = t_strSource.substr( t_idx,t_strSource.length() );
	}

	t_idx = 0;

	if( t_strSource.size() == 0 )
		return t_strRes.c_str();

	// 先略过前面的空格
	if( (int)t_strSource.length() > t_idx )
	{
		while( t_strSource.at( t_idx ) == ' ' )
		{
			t_idx ++;
			if( t_strSource.length() == t_idx )
				break;
		}
	}

	while( t_idx < (int)t_strSource.length() )
	{
		if( t_strSource.at( t_idx ) == ' ' )
			break;

		t_strRes.push_back( t_strSource.at( t_idx ) );
		t_idx ++;
	}

	return t_strRes.c_str();

	unguard;
}


//! 分析自由移动命令
bool lmc_commandMgr::interpret_spaceMoveCmd( lm_spaceMoveCmd& _cmd,const char* _cmdstr )
{
	guard;

	s_string    t_strTmp;

	if( get_keyval( _cmdstr,0 ) == NULL )
		return false;

	// 读入地图名
	strcpy( _cmd.m_szMapName,get_keyval( _cmdstr,0 ) );

	// 
	t_strTmp = get_keyval( _cmdstr,1 );
	if( t_strTmp.length() == 0 )
		return false;
	_cmd.m_iXPos = atoi( t_strTmp.c_str() );

	t_strTmp = get_keyval( _cmdstr,2 );
	if( t_strTmp.length() == 0 )
		return false;
	_cmd.m_iZPos = atoi( t_strTmp.c_str() );


	// 是否在整个地图内自由移动
	if( get_keyval( _cmdstr,3 ) != NULL )
	{
		t_strTmp = get_keyval( _cmdstr,3 );
		if( t_strTmp.length() != 0 )
			lm_gvar::g_bSpaceMoveWholeMap = TRUE;
	}

	return true;

	unguard;
}

//! 分析怪物插入的命令
bool lmc_commandMgr::interpret_monsterInsert( 
	lm_creatureInsert& _cmd,const char* _cmdstr,bool _flag/* = false*/ )
{
	guard;

	s_string   t_strTmp;

	// 
	// 怪物的目录x,y位置
	if( get_keyval( _cmdstr,0 ) == NULL )
		return false;

	t_strTmp = DEFAULT_MONSTDIR;
	t_strTmp += get_keyval( _cmdstr,0 );
	strcpy( _cmd.m_szChrDir,t_strTmp.c_str() );

	if( get_keyval( _cmdstr,1 ) == NULL )
		return false;

	if( get_keyval( _cmdstr,1 )[0] == NULL )
		return false;
	_cmd.m_iXPos = atoi( get_keyval( _cmdstr,1 ) );
	if( _cmd.m_iXPos < 0 )
		return false;
	
	if( get_keyval( _cmdstr,2 ) == NULL )
		return false;

	if( get_keyval( _cmdstr,2 )[0] == NULL )
		return false;
	_cmd.m_iZPos = atoi( get_keyval( _cmdstr,2 ) );
	if( _cmd.m_iZPos < 0 )
		return false;

	// 
	// 当前人物的插入场景时的面对方向,在内部转化为弧度
	if( get_keyval( _cmdstr,3) == NULL )
		return true;
	_cmd.m_fRotAgl = (float)atof( get_keyval( _cmdstr,3 ) );
	_cmd.m_fRotAgl = (_cmd.m_fRotAgl/180.0f)*OS_PI;

	//! 加入一个插入怪物的缩放选项
	if( !_flag )
	{
		if( get_keyval( _cmdstr,4 ) == NULL )
			return true;

		_cmd.m_fScale = (float)atof( get_keyval( _cmdstr,4 ) );
		if( _cmd.m_fScale < 0.1 )
			_cmd.m_fScale = 1.0f;
	}


	// 如果是创建旗帜的解晰模式，
	if( _flag )
	{
		if( NULL == get_keyval( _cmdstr,4 ) )
			return false;
		_cmd.m_btFlagEquip[0] = (BYTE)atoi( get_keyval( _cmdstr,4 ) );
		if( NULL == get_keyval( _cmdstr,5 ) )
			return false;
		_cmd.m_btFlagEquip[1] = (BYTE)atoi( get_keyval( _cmdstr,5 ) );
		if( NULL == get_keyval( _cmdstr,6 ) )
			return false;
		_cmd.m_btFlagEquip[2] = (BYTE)atoi( get_keyval( _cmdstr,6 ) );
		_cmd.m_bFlagType = TRUE;
	}

	return true;

	unguard;
}

//! 分析人物的动作变化
bool lmc_commandMgr::interpret_actChange( lm_creatureActChange& _cmd,const char* _cmdstr )
{
	guard;

	s_string   t_strTmp;

	if( get_keyval( _cmdstr,0 ) == NULL )
		return false;

	strcpy( _cmd.m_szNewActName,get_keyval( _cmdstr,0 ) );
	
	if( get_keyval( _cmdstr,1 ) == NULL )
	{
		// 默认只播放一次
		_cmd.m_iPlayActType = 1;
		return true;
	}
	_cmd.m_iPlayActType = atoi( get_keyval( _cmdstr,1 ) );
	if( (_cmd.m_iPlayActType != 0)&&
		(_cmd.m_iPlayActType != 1)&&
		(_cmd.m_iPlayActType != 2)   )
		return false;

	// 接下确认是否修改了动作的播放速度
	if( get_keyval( _cmdstr,2 ) == NULL )
	{
		return true;
	}
	else
	{
		_cmd.m_fPlayActScale = (float)atof( get_keyval( _cmdstr,2 ) );
		if( _cmd.m_fPlayActScale <= 0.0f )
			_cmd.m_fPlayActScale = 1.0f;
	}

	return true;

	unguard;
}

//! 分析出技能测试的命令
bool lmc_commandMgr::interpret_skillPlay( int& _idx,const char* _cmdstr )
{
	guard;

	s_string    t_strTmp;

	//! 分析出要播放特效的index
	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;

	_idx = atoi( get_keyval( _cmdstr,0 ) );
	if( _idx < 0 )
		return false;

	return true;

	unguard;
}

//! 分析出特效测试的命令
bool lmc_commandMgr::interpret_effPlay( lm_effectPlay& _cmd,const char* _cmdstr )
{
	guard;

	s_string     t_strTmp;

	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	t_strTmp = "speff\\";
	t_strTmp += get_keyval( _cmdstr,0 );
	t_strTmp += ".spe";
	if( t_strTmp.size() > 64 )
		return false;
	strcpy( _cmd.m_szEffName,t_strTmp.c_str() );

	if( NULL == get_keyval( _cmdstr,1 ) )
	{
		_cmd.m_vec3Pos.x = -1.0f;
		_cmd.m_vec3Pos.z = -1.0f;

		return true;
	}
	if( NULL == get_keyval( _cmdstr,2 ) )
	{
		_cmd.m_vec3Pos.x = -1.0f;
		_cmd.m_vec3Pos.z = -1.0f;

		return true;
	}

	_cmd.m_vec3Pos.x = float( atoi( get_keyval( _cmdstr,1 ) ) );
	_cmd.m_vec3Pos.z = float( atoi( get_keyval( _cmdstr,2 ) ) );

	// 
	// 要创建特效的位置为负
	if( (_cmd.m_vec3Pos.x < 0.0f) ||
		(_cmd.m_vec3Pos.z < 0.0f) )
	{
		_cmd.m_vec3Pos.x = -1.0f;
		_cmd.m_vec3Pos.z = -1.0f;

		return false;
	}


	return true;

	unguard;
}

//! 分析出装备相关的数据
int lmc_commandMgr::interpret_weapon( const char* _cmdstr )
{
	if( NULL == get_keyval( _cmdstr,0 ) )
		return -1;

	return atoi( get_keyval( _cmdstr,0 ) );
}

//! 分析出身体部位装备相关的数据。
bool lmc_commandMgr::interpret_equip( const char* _cmdstr,os_equipment& _equip )
{
	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;

	_equip.m_iCPartNum = 1;
	_equip.m_arrId[0] = atoi( get_keyval( _cmdstr,0 ) );
	if( _equip.m_arrId[0] < 0 )
		return false;

	if( NULL == get_keyval( _cmdstr,1 ) )
		return false;
	_equip.m_arrMesh[0] = atoi( get_keyval( _cmdstr,1 ) );
	if( _equip.m_arrMesh[0] <= 0 )
		return false;

	if( NULL == get_keyval( _cmdstr,2 ) )
		return false;
	_equip.m_arrSkin[0] = atoi( get_keyval( _cmdstr,2 ) );
	if( _equip.m_arrSkin[0] <= 0 )
		return false;

	return true;

}


//! 分析出全局光的改变命令
bool lmc_commandMgr::interpret_ambient( const char* _cmdstr )
{
	guard;

	osVec3D     t_vec3RGB;
	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	t_vec3RGB.x = (float)atof( get_keyval( _cmdstr,0 ) );
	if( NULL == get_keyval( _cmdstr,1 ) )
		return false;
	t_vec3RGB.y = (float)atof( get_keyval( _cmdstr,1 ) );
	if( NULL == get_keyval( _cmdstr,2 ) )
		return false;
	t_vec3RGB.z = (float)atof( get_keyval( _cmdstr,2 ) );

	g_fAmbiR = t_vec3RGB.x;
	g_fAmbiG = t_vec3RGB.y;
	g_fAmbiB = t_vec3RGB.z;

	if( g_fAmbiR < 0.0f ) g_fAmbiR = 0.0f;
	if( g_fAmbiR > 1.0f ) g_fAmbiR = 1.0f;

	if( g_fAmbiG < 0.0f ) g_fAmbiG = 0.0f;
	if( g_fAmbiG > 1.0f ) g_fAmbiG = 1.0f;

	if( g_fAmbiB < 0.0f ) g_fAmbiB = 0.0f;
	if( g_fAmbiB > 1.0f ) g_fAmbiB = 1.0f;

	return true;

	unguard;
}

//! 分析出全局方向光的命令.
bool lmc_commandMgr::interpret_dirlight( const char* _cmdstr )
{
	guard;

	osVec3D     t_vec3RGB;
	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	t_vec3RGB.x = (float)atof( get_keyval( _cmdstr,0 ) );
	if( NULL == get_keyval( _cmdstr,1 ) )
		return false;
	t_vec3RGB.y = (float)atof( get_keyval( _cmdstr,1 ) );
	if( NULL == get_keyval( _cmdstr,2 ) )
		return false;
	t_vec3RGB.z = (float)atof( get_keyval( _cmdstr,2 ) );

	g_fDirectLR = t_vec3RGB.x;
	g_fDirectLG = t_vec3RGB.y;
	g_fDirectLB = t_vec3RGB.z;

	if( g_fDirectLR < 0.0f ) g_fDirectLR = 0.0f;
	if( g_fDirectLR > 1.0f ) g_fDirectLR = 1.0f;

	if( g_fDirectLG < 0.0f ) g_fDirectLG = 0.0f;
	if( g_fDirectLG > 1.0f ) g_fDirectLG = 1.0f;

	if( g_fDirectLB < 0.0f ) g_fDirectLB = 0.0f;
	if( g_fDirectLB > 1.0f ) g_fDirectLB = 1.0f;


	return true;

	unguard;
}

//! 分析出全局雾的开始和结束矩离
bool lmc_commandMgr::interpret_fogDis( const char* _cmdstr )
{
	guard;


	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;

	g_fFogStart = (float)atof( get_keyval( _cmdstr,0 ) );
	if( g_fFogStart < 0.0f )
		g_fFogStart = 200.0f;
	if( g_fFogStart == 0 )
	{
		g_bUseFog = FALSE;
		return true;
	}
	else
		g_bUseFog = TRUE;

	if( NULL == get_keyval( _cmdstr,1 ) )
		return false;
	g_fFogEnd = (float)atof( get_keyval( _cmdstr,1 ) );
	if( g_fFogEnd < g_fFogStart )
		g_fFogEnd = g_fFogStart + 1.0f;

	return true;

	unguard;
}

//! 分析出全局的雾的颜色的命令
bool lmc_commandMgr::interpret_fogColor( const char* _cmdstr )
{
	guard;

	osColor   t_sColor;

	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	t_sColor.r = (float)atof( get_keyval( _cmdstr,0 ) );
	if( NULL == get_keyval( _cmdstr,1 ) )
		return false;
	t_sColor.g = (float)atof( get_keyval( _cmdstr,1 ) );
	if( NULL == get_keyval( _cmdstr,2 ) )
		return false;
	t_sColor.b = (float)atof( get_keyval( _cmdstr,2 ) );

	g_dwFogColor = t_sColor;

	return true;

	unguard;
}

//! 分析出有两个整型值后跟的命令类型
bool lmc_commandMgr::interpret_int2( const char* _cmdstr,int& _v1,int& _v2 )
{
	guard;

	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	_v1 = atoi( get_keyval( _cmdstr,0 ) );

	if( NULL == get_keyval( _cmdstr,1 ) )
		return false;
	_v2 = atoi( get_keyval( _cmdstr,1 ) );

	return true;

	unguard;
}

//! 要析出字串，后跟一个浮点型的数据类型
bool lmc_commandMgr::interpret_str1_float1( const char* _cmdstr,lm_effectPlay& _cmd )
{
	guard;

	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	if( NULL == get_keyval( _cmdstr,0)[0] )
		return false;
	strcpy( _cmd.m_szEffName,get_keyval( _cmdstr,0 ) );

	if( NULL == get_keyval( _cmdstr,1 ) )
	{
		_cmd.m_vec3Pos.x = 0.0f;
		return true;
	}
	if( NULL == get_keyval( _cmdstr,1 )[0] )
	{
		_cmd.m_vec3Pos.x = 0.0f;
		return true;
	}

	_cmd.m_vec3Pos.x = (float)atof( get_keyval( _cmdstr,1 ) );

	return true;

	unguard;
}

//! 解析出字串,后跟一浮点值
bool lmc_commandMgr::interpret_str1_float( const char* _cmdstr,lm_cameraSaveLoad& _cmd )
{
	if( NULL == get_keyval( _cmdstr,0 ) )
		return false;
	if( NULL == get_keyval( _cmdstr,0)[0] )
		return false;
	strcpy( _cmd.m_szCameraFile,get_keyval( _cmdstr,0 ) );

	if( NULL == get_keyval( _cmdstr,1 ) )
	{
		_cmd.m_fCamAniTime = 2.0f;
		return true;
	}
	if( NULL == get_keyval( _cmdstr,1 )[0] )
	{
		_cmd.m_fCamAniTime = 2.0f;
		return true;
	}

	_cmd.m_fCamAniTime = (float)atof( get_keyval( _cmdstr,1 ) );

	return true;
}


//! 调入地图，并插入主角人物
void lmc_commandMgr::load_map( lm_spaceMoveCmd& _cmd )
{
	guard;

	os_sceneLoadStruct   t_sl;
	lm_command           t_sCommand;


	//　先是调入地图的命令
	t_sl.m_iX = _cmd.m_iXPos;
	t_sl.m_iY = _cmd.m_iZPos;
	strcpy( t_sl.m_szMapName,_cmd.m_szMapName );
	t_sl.m_szSkyName[0] = NULL;

	t_sCommand.m_eCommandId = LM_SCENELOAD;
	t_sCommand.m_dwCmdContentSize = sizeof( os_sceneLoadStruct );
	t_sCommand.m_ptrCommandContent = (const void*)&t_sl;

	lmc_commandMgr::Instance()->push_command( t_sCommand );


	// 插入主角人物的命令
	lm_heroInsert        t_sHeroInsert;

	g_ptrConfigInfoReader->read_heroInsertData( &t_sHeroInsert );
	t_sHeroInsert.m_iXPos = _cmd.m_iXPos;
	t_sHeroInsert.m_iZPos = _cmd.m_iZPos;
	t_sHeroInsert.m_sInitEquip.m_iCPartNum = 0;
	t_sCommand.m_dwCmdContentSize = sizeof( lm_heroInsert );
	t_sCommand.m_eCommandId = LM_HEROINSERT;
	t_sCommand.m_ptrCommandContent = &t_sHeroInsert;

	lmc_commandMgr::Instance()->push_command( t_sCommand );


	unguard;
}

//@{
//! 当自动命令队列为空的时候,重设相关的数据.
void lmc_commandMgr::reset_autoCmdQueue( void )
{
	this->m_fAutoCmdExeTime = 0.0f;
	this->m_fAccuTime = 0.0f;

	m_bAutoCmdExeStatus = FALSE;
	
	m_sAutoCmdQueue.empty();

}

//!  用于批量执行命令的一些功能
void lmc_commandMgr::frame_moveAutoCmd( float _eleTime )
{
	guard;

	if( m_sAutoCmdQueue.size() > 0 )
	{
		if( !m_bAutoCmdExeStatus )
			m_bAutoCmdExeStatus = TRUE;
		else
			m_fAutoCmdExeTime += _eleTime;

		// 
		// 对队列内的命令进行处理.
		do
		{
			const lm_autoCmd& t_cmd = m_sAutoCmdQueue.front();
			if( m_fAutoCmdExeTime >= (t_cmd.m_fOffsetExecTime + m_fAccuTime) )
			{
				if( !interpret_strToCmd( t_cmd.m_szCmdStr ) )
				{
					reset_autoCmdQueue();
					return;
				}
				else
				{
					// 加入累加的时间,并弹出已经执行过的命令,
					// 调入场景的命令费时较长，对这个命令不处理时间
					const char* t_szCmd = get_cmdStr( t_cmd.m_szCmdStr );
					if( (strcmp( t_szCmd,t_szDictionary[0].c_str() ) != 0) &&
						(strcmp( t_szCmd,t_szDictionaryChn[0].c_str() ) != 0 )  )
						m_fAccuTime += t_cmd.m_fOffsetExecTime;

					m_sAutoCmdQueue.pop();
				}
			}
			else
				break;

		} while( m_sAutoCmdQueue.size()>0 );

		if( m_sAutoCmdQueue.size() == 0 )
			reset_autoCmdQueue();
	}

	unguard;
}

//!  推入需要自动执行的cmd
bool lmc_commandMgr::push_autoCmd( lm_autoCmd& _autocmd )
{
	int t_idx = 0;

	int t_iCmdNum = sizeof( t_szDictionary )/sizeof( s_string);

	// 优化处理.
	for( t_idx = 0;t_idx<t_iCmdNum;t_idx ++ )
	{
		if( t_szDictionary[t_idx] == get_cmdStr( _autocmd.m_szCmdStr ) )
			break;
		if( t_szDictionaryChn[t_idx] == get_cmdStr( _autocmd.m_szCmdStr ) )
			break;
		if( strcmp( C_VARIABLE_KEY,get_cmdStr( _autocmd.m_szCmdStr ) ) == 0 )
			break;
		if( strcmp( E_VARIABLE_KEY,get_cmdStr( _autocmd.m_szCmdStr ) ) == 0 )
			break;
	}

	if( t_idx == t_iCmdNum )
	{
		MessageBox( NULL,_autocmd.m_szCmdStr,"不能执行以下的命令",MB_OK );
		return false;
	}

	m_sAutoCmdQueue.push( _autocmd );
	return true;
}
//!  调入和执行一个命令文件列表
bool lmc_commandMgr::load_execCmdFile( const char* _filename )
{
	guard;

	if( !_filename )
		return false;
	if( _filename[0] == NULL )
		return false;

	FILE*   t_file = fopen( _filename,"rt" );

	if( !t_file )
		return false;

	char    t_szTime[128];
	char*   t_szRes = fgets( t_szTime,128,t_file );
	while( t_szRes )
	{
		if( t_szRes[0] == NULL )
			break;

		// 推入一条自动执行的命令
		lm_autoCmd   t_sAutoCmd;
		t_sAutoCmd.m_fOffsetExecTime = (float)atof( t_szTime );
		fgets( t_sAutoCmd.m_szCmdStr,64 ,t_file );
		
		//! 去掉此行中出现的'\n'换行符
		int t_iLen = (int)strlen( t_sAutoCmd.m_szCmdStr );
		if( t_sAutoCmd.m_szCmdStr[t_iLen-1] == '\n' )
			t_sAutoCmd.m_szCmdStr[t_iLen-1] = NULL;

		//  可能碰上错误的命令.
		if( t_iLen >= 2 )
		{
			if( !push_autoCmd( t_sAutoCmd ) )
				break;
		}

		t_szRes = fgets( t_szTime,128,t_file );
	}
	fclose( t_file );

	return true;

	unguard;
}
//@} 



//! 字串的开头是否是变量,并处理相应的内容
bool lmc_commandMgr::interpret_varCmd( const char* _str,int& _handle,int& _target )
{
	guard;

	s_string    t_strCmd = get_cmdStr( _str );

	if( (t_strCmd == C_VARIABLE_KEY)||
		(t_strCmd == E_VARIABLE_KEY) )
	{
		s_string    t_strCmdContent = get_cmdContent( _str );

		if( NULL == get_keyval( t_strCmdContent.c_str(),0 ) )
			return false;

		_handle = m_sVariableMgr.get_sceneVariable( 
			get_keyval( t_strCmdContent.c_str(),0 )  );

		// 是否有第二个变量
		s_string    t_szSecVar;
		t_szSecVar = get_keyval( t_strCmdContent.c_str(),1 );
		if( (t_szSecVar == C_TARGET_KEY) ||
			(t_szSecVar == E_TARGET_KEY)  )
		{
			_target = m_sVariableMgr.get_sceneVariable(
				get_keyval( t_strCmdContent.c_str(),2 ) );
		}
		else
			_target = -1;

		return true;
	}

	return false;

	unguard;
}




//! 把字串解析为内部的命令,并推入到命令队列内
bool lmc_commandMgr::interpret_strToCmd( const char* _str )
{
	guard;

	// 空命令不需要处理
	if( !_str[0] )
		return true;

	lm_command  t_cmd;

	// 没有调入地图时，只接受move命令
	s_string    t_strCmd;

	t_strCmd = get_cmdStr( _str );
	if( !gc_gameScene::m_ptrScene->isLoadMap() )
	{
		if( !( ( t_strCmd == t_szDictionary[0] ) || 
			   (t_strCmd == t_szDictionaryChn[0].c_str() ) ) )
			return false;
	}

	// 此命令是否使用变量。
	int         t_iCmdHandle = -1,t_iCmdTargetHandle = -1;
	bool        t_bVarCmd = false;
	s_string    t_strVariable;
	s_string    t_strCmdContent;

	if( interpret_varCmd( _str,t_iCmdHandle,t_iCmdTargetHandle ) )
	{
		s_string   t_strRealCmd;
		int        t_idx;
		int        t_iVarKeyNum = 2;

		t_bVarCmd = true;

		t_strVariable = get_keyval( _str,1 );
		osassertex( t_strVariable.length() < 16,va("怪物<%s>名字过长..\n",t_strVariable.c_str() ) );

		if( t_iCmdTargetHandle >= 0 )
			t_iVarKeyNum = 4;
		t_idx = get_keyStartPos( _str,t_iVarKeyNum );
		t_strRealCmd = _str;
		t_strRealCmd = t_strRealCmd.substr( t_idx,t_strRealCmd.size() );

		t_strCmd = get_cmdStr( t_strRealCmd.c_str() );
		t_strCmdContent = get_cmdContent( t_strRealCmd.c_str() );

		// 
		if( t_bVarCmd )
		{
			t_cmd.m_iSHandle = t_iCmdHandle;
			t_cmd.m_iEHandle = t_iCmdTargetHandle;
		}
	}
	else
	{
		// 分析字串，是否有相应的命令
		t_strCmd = get_cmdStr( _str );
		t_strCmdContent = get_cmdContent( _str );
	}

	// 加入命令的错误处理机制
	if( (t_strCmd == t_szDictionary[0])||
		(t_strCmd == t_szDictionaryChn[0]) )
	{

		lm_spaceMoveCmd    t_move;
		// 把字串内容分析成实际数据
		if( !interpret_spaceMoveCmd( t_move,t_strCmdContent.c_str() ) )
			return false;

		// 为了调试方便，如果此时还没有调入地图，则这个命令为调入地图的命令
		if( gc_gameScene::m_ptrScene->isLoadMap() )
		{
			t_cmd.m_dwCmdContentSize = sizeof( lm_spaceMoveCmd );
			t_cmd.m_eCommandId = LM_SPACEMOVE;
			t_cmd.m_ptrCommandContent = &t_move;
			push_command( t_cmd );		
		}
		else
		{
			load_map( t_move );
		}
	}
	else
	// 插入怪物
	if( (t_strCmd == t_szDictionary[1]) ||
		(t_strCmd == t_szDictionaryChn[1] )  )
	{
		lm_creatureInsert   t_sMonsterInsert;

		if( !interpret_monsterInsert( t_sMonsterInsert,t_strCmdContent.c_str() ) )
			return false;

		// 怪物的名字
		if( t_bVarCmd && (t_iCmdHandle==-1) )
			strcpy( t_sMonsterInsert.m_szCreatureName,t_strVariable.c_str() );
		else
			t_sMonsterInsert.m_szCreatureName[0] = NULL;

		t_cmd.m_dwCmdContentSize = sizeof( lm_creatureInsert );
		t_cmd.m_eCommandId = LM_MONSTERINSERT;
		t_cmd.m_ptrCommandContent = &t_sMonsterInsert;

		// 
		// 实时的处理命令
		do_commandImm( t_cmd );
	}
	else
	// 主角人物跑步加速
	if( (t_strCmd == t_szDictionary[2])||
		(t_strCmd == t_szDictionaryChn[2]) )
	{
		float  t_fSM = (float)atof( t_strCmdContent.c_str() );
		if( t_fSM > 0 )
			gc_hero::m_fRunSpeed *= t_fSM;
		else
			return false;
	}
	else
	// 相机是否跟地表做碰撞
	if( (t_strCmd == t_szDictionary[3])||
		(t_strCmd == t_szDictionaryChn[3]) )
	{
		int   t_iCol = atoi( t_strCmdContent.c_str() );
		g_bCamTerrCol = t_iCol;
	}
	else
	// 主角人物的测试动作
	if( (t_strCmd == t_szDictionary[4]) ||
		(t_strCmd == t_szDictionaryChn[4] ) )
	{
		lm_creatureActChange    t_sActChange;

		if( !this->interpret_actChange( t_sActChange,t_strCmdContent.c_str() ) )
			return false;

		if( t_sActChange.m_szNewActName[0] != NULL )
		{
			t_cmd.m_dwCmdContentSize = sizeof( lm_creatureActChange );
			t_cmd.m_eCommandId = LM_HERONEWACTION;
			t_cmd.m_ptrCommandContent = &t_sActChange;

			push_command( t_cmd );
		}
	}
	else
	// 怪物的动作测试:
	if( (t_strCmd == t_szDictionary[5])||
		(t_strCmd == t_szDictionaryChn[5]) )
	{
		lm_creatureActChange    t_sActChange;
		if( !this->interpret_actChange( t_sActChange,t_strCmdContent.c_str() ) )
			return false;

		if( t_sActChange.m_szNewActName[0] != NULL )
		{
			t_cmd.m_dwCmdContentSize = sizeof( lm_creatureActChange );
			t_cmd.m_eCommandId = LM_MONSTERACT;
			t_cmd.m_ptrCommandContent = &t_sActChange;
			push_command( t_cmd );
		}
	}
	else
	// 
	// 技能的播放处理,用于模拟客户端的特效testskill
	if( (t_strCmd == t_szDictionary[6])||
		(t_strCmd == t_szDictionaryChn[6])  )
	{

/*
		lm_cmdSkillName   t_sSkillName;

		strcpy( t_sSkillName.m_szSkillName,t_strCmdContent.c_str() );
		if( t_sSkillName.m_szSkillName[0] == NULL )
			return false;

		t_cmd.m_dwCmdContentSize = sizeof( lm_cmdSkillName );
		t_cmd.m_eCommandId = LM_SKILLPLAY;
		t_cmd.m_ptrCommandContent = &t_sSkillName;
		push_command( t_cmd );
*/
		lm_effectPlay   t_skill;

		if( !interpret_str1_float1( t_strCmdContent.c_str(),t_skill ) )
			return false;
		lm_cmdSkillName   t_sSkillName;

		t_sSkillName.m_fSkillHeight = t_skill.m_vec3Pos.x;
		strcpy( t_sSkillName.m_szSkillName,t_skill.m_szEffName );
		if( t_sSkillName.m_szSkillName[0] == NULL )
			return false;

		t_cmd.m_dwCmdContentSize = sizeof( lm_cmdSkillName );
		t_cmd.m_eCommandId = LM_SKILLPLAY;
		t_cmd.m_ptrCommandContent = &t_sSkillName;
		push_command( t_cmd );
		
	}
	else // 改变全局光的命令
	if( (t_strCmd == t_szDictionary[7])||
		(t_strCmd == t_szDictionaryChn[7]) )
	{
		if( !interpret_ambient( t_strCmdContent.c_str() ) )
			return false;
	}
	else 
	if( (t_strCmd == t_szDictionary[8])||
		(t_strCmd == t_szDictionaryChn[8])  )
	{
		if( !interpret_dirlight( t_strCmdContent.c_str() ) )
			return false;
	}
	else 
	if( (t_strCmd == t_szDictionary[9]) ||
		(t_strCmd == t_szDictionaryChn[9]) )
	{
		if( !interpret_fogColor( t_strCmdContent.c_str() ) )
			return false;
	}
	else 
	if( (t_strCmd == t_szDictionary[10])||
		(t_strCmd == t_szDictionaryChn[10]) )
	{
		if( !interpret_fogDis( t_strCmdContent.c_str() ) )
			return false;
	}
	else
	// 单独特效的播放处理，在Demo内主角人物的位置上播放一个特效
	if( (t_strCmd == t_szDictionary[11])||
		(t_strCmd == t_szDictionaryChn[11])  )
	{
		lm_effectPlay    t_sEffPlay;

		if( !interpret_effPlay( t_sEffPlay,t_strCmdContent.c_str() ) )
			return false;

		if( !::file_exist( t_sEffPlay.m_szEffName ) )
			return false;

		t_cmd.m_dwCmdContentSize = sizeof( t_sEffPlay );
		t_cmd.m_eCommandId = LM_EFFECTPLAY;
		t_cmd.m_ptrCommandContent = &t_sEffPlay;
		push_command( t_cmd );
	}
	else
	if( (t_strCmd == t_szDictionary[12])||
		(t_strCmd == t_szDictionaryChn[12]) )
	{
		lm_cmdFileName    t_ptrName;

		if( t_strCmdContent.size() <= 0 )
			return false;

		strcpy( t_ptrName.m_szFileName,t_strCmdContent.c_str() );
		t_cmd.m_dwCmdContentSize = sizeof( t_ptrName );
		t_cmd.m_eCommandId = LM_SCENEMUSIC;
		t_cmd.m_ptrCommandContent = &t_ptrName;
		push_command( t_cmd );

	}
	else
	if( (t_strCmd == t_szDictionary[13])||
		(t_strCmd == t_szDictionaryChn[13]) )
	{
		lm_heroInsert        t_sHeroInsert;

		if( t_strCmdContent.size() <= 0 )
			return false;

		s_string   t_str = "character\\";
		t_str += t_strCmdContent;
		strcpy( t_sHeroInsert.m_szChrDir,t_str.c_str() );
		t_sHeroInsert.m_sInitEquip.m_iCPartNum = 0;
		
		t_cmd.m_dwCmdContentSize = sizeof( lm_heroInsert );
		t_cmd.m_eCommandId = LM_HEROINSERT;
		t_cmd.m_ptrCommandContent = &t_sHeroInsert;
		
		push_command( t_cmd );
	}
	else
	if( (t_strCmd == t_szDictionary[14])||
		(t_strCmd == t_szDictionaryChn[14])   )
	{
		os_equipment    t_equip;

		t_equip.m_iCPartNum = 1;
		t_equip.m_arrMesh[0] = interpret_weapon( t_strCmdContent.c_str() );
		if( t_equip.m_arrMesh[0] == -1 )
			return false;
		t_equip.m_arrId[0] = 6;
		t_equip.m_arrSkin[0] = 1;

		t_cmd.m_dwCmdContentSize = sizeof( os_equipment );
		t_cmd.m_eCommandId = LM_HEROEQUIPCHANGE;
		t_cmd.m_ptrCommandContent = &t_equip;

		push_command( t_cmd );
	}
	else
	if( (t_strCmd == t_szDictionary[15])||
		(t_strCmd == t_szDictionaryChn[15])   )
	{
		os_equipment   t_equip;

		if( !this->interpret_equip( t_strCmdContent.c_str(),t_equip ) )
			return false;

		t_cmd.m_dwCmdContentSize = sizeof( os_equipment );
		t_cmd.m_eCommandId = LM_HEROEQUIPCHANGE;
		t_cmd.m_ptrCommandContent = &t_equip;

		push_command( t_cmd );
	}
	else
	if( (t_strCmd == t_szDictionary[16])||
		(t_strCmd == t_szDictionaryChn[16])   )
	{
		lm_cmdFileName   t_name;
	
		t_name = t_strCmdContent.c_str();
		t_cmd.m_eCommandId = LM_WEATHERCHANGE;
		t_cmd.m_dwCmdContentSize = sizeof( lm_cmdFileName );
		t_cmd.m_ptrCommandContent = (void*)&t_name;
		lmc_commandMgr::Instance()->push_command( t_cmd );
	}
	else
	// 主角人物的自动移动命令
	if( (t_strCmd == t_szDictionary[17])||
		(t_strCmd == t_szDictionaryChn[17])   )
	{
		int          t_x,t_z;
		osVec3D      t_vec3Pos;

		if( !interpret_int2( t_strCmdContent.c_str(),t_x,t_z ) )
			return false;

		t_vec3Pos.x = t_x*TILE_WIDTH/2.0f;
		t_vec3Pos.z = t_z*TILE_WIDTH/2.0f;
		t_vec3Pos.y = 0.0f;

		t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
		t_cmd.m_eCommandId = LM_HEROWALK;
		t_cmd.m_ptrCommandContent = &t_vec3Pos;
		
		lmc_commandMgr::Instance()->push_command( t_cmd );
	}
	else
	// 当前选中怪物的移动命令
	if( (t_strCmd == t_szDictionary[18])||
		(t_strCmd == t_szDictionaryChn[18])   )
	{
		int          t_x,t_z;
		osVec3D      t_vec3Pos;

		if( !interpret_int2( t_strCmdContent.c_str(),t_x,t_z ) )
			return false;

		t_vec3Pos.x = t_x*TILE_WIDTH/2.0f;
		t_vec3Pos.z = t_z*TILE_WIDTH/2.0f;
		t_vec3Pos.y = 0.0f;

		t_cmd.m_dwCmdContentSize = sizeof( osVec3D );
		t_cmd.m_eCommandId = LM_MONSTERWALK;
		t_cmd.m_ptrCommandContent = &t_vec3Pos;

		lmc_commandMgr::Instance()->push_command( t_cmd );
	}
	else
	if( (t_strCmd == t_szDictionary[19]) ||
		(t_strCmd == t_szDictionaryChn[19]) )
	{
		os_equipment   t_equip;

		if( !this->interpret_equip( t_strCmdContent.c_str(),t_equip ) )
			return false;

		t_cmd.m_dwCmdContentSize = sizeof( os_equipment );
		t_cmd.m_eCommandId = LM_MONSTEREQUIP;
		t_cmd.m_ptrCommandContent = &t_equip;

		push_command( t_cmd );		
	}
	else
	if( (t_strCmd == t_szDictionary[20]) ||
		(t_strCmd == t_szDictionaryChn[20]) )
	{
		int t_iHide = 0;

		t_iHide = atoi( t_strCmdContent.c_str() );

		t_cmd.m_dwCmdContentSize = sizeof( int );

		if( t_cmd.m_iSHandle >= 0 )
			t_cmd.m_eCommandId = LM_MONSTERHIDE;
		else
			t_cmd.m_eCommandId = LM_HEROHIDE;
		t_cmd.m_ptrCommandContent = &t_iHide;

		push_command( t_cmd );
	}
	else // 是否在场景内的人物头上显示此人物的名字
	if( (t_strCmd == t_szDictionary[21]) ||
		(t_strCmd == t_szDictionaryChn[21]) )
	{
		int t_iNum = atoi( t_strCmdContent.c_str() );
		if( t_iNum == 0 )
			lm_gvar::g_bDispMonsterName = FALSE;
		else
			lm_gvar::g_bDispMonsterName = TRUE;
	}
	else // 使用名字来修改主角人物相关的装备
	if( (t_strCmd == t_szDictionary[22]) ||
		(t_strCmd == t_szDictionaryChn[22]) )
	{
		lm_cmdFileName    t_ptrName;

		if( t_strCmdContent.size() <= 0 )
			return false;

		strcpy( t_ptrName.m_szFileName,t_strCmdContent.c_str() );
		t_cmd.m_dwCmdContentSize = sizeof( t_ptrName );
		t_cmd.m_eCommandId = LM_NAMEEQUIPCHANGE;
		t_cmd.m_ptrCommandContent = &t_ptrName;		
		
		push_command( t_cmd );
		
	}
	else  // 是否使用相机的自动跟随功能
	if( (t_strCmd == t_szDictionary[23]) ||
		(t_strCmd == t_szDictionaryChn[23]) )
	{
		int t_iNum = atoi( t_strCmdContent.c_str() );
		if( t_iNum == 0 )
		{
			// 关掉跟地表的碰撞，物品折挡透明
			lm_gvar::g_bCamFollowHero = FALSE;
			g_bAlphaShelter = false;
			g_bCamTerrCol = false;
		}
		else
		{
			// 
			lm_gvar::g_bCamFollowHero = TRUE;
			g_bAlphaShelter = TRUE;
			g_bCamTerrCol = true;
		}
	}
	else  // 相机的移动速度
	if( (t_strCmd == t_szDictionary[24]) ||
		(t_strCmd == t_szDictionaryChn[24]) )
	{
		lm_gvar::g_fCamMoveSpeed = (float)atof( t_strCmdContent.c_str() );
	}
	else // 怪物朝人以发随机技能的魔法
	if( (t_strCmd == t_szDictionary[25]) ||
		(t_strCmd == t_szDictionaryChn[25]) )
	{
		int   t_k = atoi( t_strCmdContent.c_str() );
		lm_gvar::g_bRandSkillPlay = true;
		if( t_k == 0 ) lm_gvar::g_bRandSkillPlay = false;
	}
	else // 状态魔法的播放
	if( (t_strCmd == t_szDictionary[26]) ||
		(t_strCmd == t_szDictionaryChn[26]) )
	{
		lm_cmdFileName    t_ptrName;

		osassert( t_strCmdContent.size() < 64 );
		strcpy( t_ptrName.m_szFileName,t_strCmdContent.c_str() );
		t_cmd.m_dwCmdContentSize = sizeof( t_ptrName );
		t_cmd.m_eCommandId = LM_MAGICSTATE;
		t_cmd.m_ptrCommandContent = &t_ptrName;		

		push_command( t_cmd );
	}
	else // 施放魔法的动作速度
	if( (t_strCmd == t_szDictionary[27]) ||
		(t_strCmd == t_szDictionaryChn[27]) )
	{
		lm_gvar::g_fSkillActSpeed = (float)atof( t_strCmdContent.c_str() );
		if( lm_gvar::g_fSkillActSpeed<0.0f ) lm_gvar::g_fSkillActSpeed = 1.0f;
	}
	else // 怪物的跑步动作时间设置
	if( (t_strCmd == t_szDictionary[28]) ||
		(t_strCmd == t_szDictionaryChn[28]) )
	{
		float   t_fRunActTime = (float)atof( t_strCmdContent.c_str() );
		
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MONSTERRUNACTTIME;
		t_cmd.m_ptrCommandContent = &t_fRunActTime;

		if( -1 == t_cmd.m_iSHandle )
		{
			MessageBox( NULL,"此命令必须配合变量使用...","ERROR",MB_OK );
			return false;
		}

		push_command( t_cmd );
	}
	else // 怪物的跑步移动步幅的设置
	if( (t_strCmd == t_szDictionary[29]) ||
		(t_strCmd == t_szDictionaryChn[29]) )
	{
		float   t_fRunSpeed = (float)atof( t_strCmdContent.c_str() );
		
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MONSTERRUNSPEED;
		t_cmd.m_ptrCommandContent = &t_fRunSpeed;

		if( -1 == t_cmd.m_iSHandle )
		{
			MessageBox( NULL,"此命令必须配合变量使用...","ERROR",MB_OK );
			return false;
		}

		push_command( t_cmd );
	}
	else // 在场景内创建一个旗帜
	if( (t_strCmd == t_szDictionary[30]) ||
		(t_strCmd == t_szDictionaryChn[30]) )
	{
		lm_creatureInsert   t_sFlagInsert;

		if( !interpret_monsterInsert( t_sFlagInsert,t_strCmdContent.c_str(),true ) )
			return false;

		// 怪物的名字
		if( t_bVarCmd && (t_iCmdHandle==-1) )
			strcpy( t_sFlagInsert.m_szCreatureName,t_strVariable.c_str() );
		else
			t_sFlagInsert.m_szCreatureName[0] = NULL;

		t_cmd.m_dwCmdContentSize = sizeof( lm_creatureInsert );
		t_cmd.m_eCommandId = LM_MONSTERINSERT;
		t_cmd.m_ptrCommandContent = &t_sFlagInsert;

		// 
		// 实时的处理命令
		do_commandImm( t_cmd );		
	}
	else // 主角人物或是怪物说话的功能
	if( (t_strCmd == t_szDictionary[31]) ||
		(t_strCmd == t_szDictionaryChn[31]) )
	{
		lm_talkContent    t_sTalkContent;

		strcpy( t_sTalkContent.m_szTalkContent,t_strCmdContent.c_str() );

		if( -1 == t_cmd.m_iSHandle )
			t_cmd.m_eCommandId = LM_HEROSAY;
		else
			t_cmd.m_eCommandId = LM_MONSTERSAY;
		t_cmd.m_ptrCommandContent = &t_sTalkContent;
		t_cmd.m_dwCmdContentSize = sizeof( lm_talkContent );

		// 
		push_command( t_cmd );

	}
	else // 主角人物和怪物的骑乘命令
	if( (t_strCmd == t_szDictionary[32]) ||
		(t_strCmd == t_szDictionaryChn[32]) )
	{
		lm_effectPlay   t_sRideChar;

		if( !interpret_str1_float1( t_strCmdContent.c_str(),t_sRideChar ) )
			return false;
		s_string   t_str = "character\\";
		t_str += t_sRideChar.m_szEffName;
		strcpy( t_sRideChar.m_szEffName,t_str.c_str() );

		if( -1 == t_cmd.m_iSHandle )
			t_cmd.m_eCommandId = LM_HERORIDE;
		else
			t_cmd.m_eCommandId = LM_MONSTERRIDE;
		t_cmd.m_ptrCommandContent = &t_sRideChar;
		t_cmd.m_dwCmdContentSize = sizeof( lm_effectPlay );

		push_command( t_cmd );

	}
	else //结束骑乘的命令
	if( (t_strCmd == t_szDictionary[33]) ||
		(t_strCmd == t_szDictionaryChn[33]) )
	{
		if( -1 == t_cmd.m_iSHandle )
			t_cmd.m_eCommandId = LM_HEROUNRIDE;
		else
			t_cmd.m_eCommandId = LM_MONSTERUNRIDE;
		t_cmd.m_ptrCommandContent = NULL;
		t_cmd.m_dwCmdContentSize = 0;

		push_command( t_cmd );
	}
	else // 删除场景内怪物的命令
	if( (t_strCmd == t_szDictionary[34]) ||
		(t_strCmd == t_szDictionaryChn[34]) )
	{
		if( -1 == t_cmd.m_iSHandle )
		{
			MessageBox( NULL,"此命令必须配合怪物名字使用..","INFO",MB_OK );
			return true;
		}

		t_cmd.m_eCommandId = LM_MONSTERDEL;
		t_cmd.m_ptrCommandContent = NULL;
		t_cmd.m_dwCmdContentSize = 0;
		
		push_command( t_cmd );

		m_sVariableMgr.delete_variable( t_cmd.m_iSHandle );

	}
	else // 切换光标的命令.
	if( (t_strCmd == t_szDictionary[35]) ||
		(t_strCmd == t_szDictionaryChn[35]) )
	{
		// 直接解晰
		g_ptrGameApp->reset_cursor( t_strCmdContent.c_str() );
	}
	else // 光标的动画时间
	if( (t_strCmd == t_szDictionary[36]) ||
		(t_strCmd == t_szDictionaryChn[36]) )
	{
		g_fMouseCursorFrameAniTime = (float)atof( t_strCmdContent.c_str() );
	}
	else // 是否在屏幕上显示调试信息
	if( (t_strCmd == t_szDictionary[37]) ||
		(t_strCmd == t_szDictionaryChn[37]) )
	{
		if( atoi( t_strCmdContent.c_str() ) != 0 )
			lm_gvar::g_bRenderDebugInfo = TRUE;
		else
			lm_gvar::g_bRenderDebugInfo = FALSE;
	}
	else // 存储相机.
	if( (t_strCmd == t_szDictionary[38]) ||
		(t_strCmd == t_szDictionaryChn[38]) )
	{
		lm_cameraSaveLoad    t_sCamera;

		if( t_strCmdContent.size() <= 0 )
			return false;

		strcpy( t_sCamera.m_szCameraFile,t_strCmdContent.c_str() );
		t_cmd.m_dwCmdContentSize = sizeof( t_sCamera );
		t_cmd.m_eCommandId = LM_SAVECAMERA;
		t_cmd.m_ptrCommandContent = &t_sCamera;		
		
		push_command( t_cmd );

	}
	else // 调入相机
	if( (t_strCmd == t_szDictionary[39]) ||
		(t_strCmd == t_szDictionaryChn[39]) )
	{
		lm_cameraSaveLoad    t_sCamera;

		if( t_strCmdContent.size() <= 0 )
			return false;

		interpret_str1_float( t_strCmdContent.c_str(),t_sCamera );
		t_cmd.m_dwCmdContentSize = sizeof( t_sCamera );
		t_cmd.m_eCommandId = LM_LOADCAMERA;
		t_cmd.m_ptrCommandContent = &t_sCamera;		
		
		push_command( t_cmd );
		
	}
	else 	
		return false;


	// 把字串分析后的相应数据产生的命令推进到命令队列内


	// 加入此全局变量
	if( t_bVarCmd && (t_iCmdHandle==-1) )
		m_sVariableMgr.add_sceneVariable( t_strVariable.c_str(),lm_gvar::g_iCreatureHandle );


	return true;

	unguard;
}



/////////////////////////////////////////////////////////////////////////////////////////////
//
//  场景内脚本变量相关的类
//
/////////////////////////////////////////////////////////////////////////////////////////////
lm_cmdVariableObj::lm_cmdVariableObj()
{
	reset();
}

# define MAX_VARNUM   128
lmc_cmdVarObjMgr::lmc_cmdVarObjMgr()
{
	m_vecVariable.resize( MAX_VARNUM );
}

lmc_cmdVarObjMgr::~lmc_cmdVarObjMgr()
{

}

//! 在场景内删除一个变量
void lmc_cmdVarObjMgr::delete_variable( int _handle )
{
	guard;

	osassert( _handle >= 0 );

	for( int t_i=0;t_i<(int)m_vecVariable.size();t_i ++ )
	{
		if( m_vecVariable[t_i].m_iObjHandle == _handle )
		{
			m_vecVariable[t_i].reset();
			return;
		}
	}

	unguard;
}


//! 在场景内删除一个变量
void lmc_cmdVarObjMgr::delete_sceneVariable( const char* _vname )
{
	guard;

	DWORD   t_dwHv = string_hash( _vname );

	for( int t_i=0;t_i<(int)m_vecVariable.size();t_i ++ )
	{
		if( m_vecVariable[t_i].m_dwObjNameHash == t_dwHv )
		{
			if( 0 == strcmp( _vname,m_vecVariable[t_i].m_szObjName ) )
			{
				m_vecVariable[t_i].reset();
				return;
			}
		}
	}

	unguard;
}


//! 往场景内增加一个变量
void lmc_cmdVarObjMgr::add_sceneVariable( const char* _vname,int _vvalue )
{
	guard;

	osassert( strlen( _vname ) < 16 );

	if( is_variableInScene( _vname ) )
	{
		osassertex( false,va( "当前已经存在变量<%s>..\n",_vname ) );
		return;
	}

	for( int t_i=0;t_i<(int)m_vecVariable.size();t_i ++ )
	{
		if( m_vecVariable[t_i].m_szObjName[0] == NULL )
		{
			strcpy( m_vecVariable[t_i].m_szObjName,_vname );
			m_vecVariable[t_i].m_dwObjNameHash = string_hash( _vname );
			m_vecVariable[t_i].m_iObjHandle = _vvalue;

			return;
		}
	}

	// 加入到最后
	lm_cmdVariableObj    t_var;
	strcpy( t_var.m_szObjName,_vname );
	t_var.m_dwObjNameHash = string_hash( _vname );
	t_var.m_iObjHandle = _vvalue;

	m_vecVariable.push_back( t_var );

	return;

	unguard;
}

//! 根据名字，得到一个变量
int lmc_cmdVarObjMgr::get_sceneVariable( const char* _vname )
{
	guard;

	DWORD   t_dwHv = string_hash( _vname );

	for( int t_i=0;t_i<(int)m_vecVariable.size();t_i ++ )
	{
		if( m_vecVariable[t_i].m_dwObjNameHash == t_dwHv )
		{
			if( 0 == strcmp( _vname,m_vecVariable[t_i].m_szObjName ) )
				return m_vecVariable[t_i].m_iObjHandle;
		}	
	}

	return -1;

	unguard;
}

//! 变量是否存在于场景内
bool lmc_cmdVarObjMgr::is_variableInScene( const char* _vname )
{
	guard;

	DWORD   t_dwHv = string_hash( _vname );

	for( int t_i=0;t_i<(int)m_vecVariable.size();t_i ++ )
	{
		if( m_vecVariable[t_i].m_dwObjNameHash == t_dwHv )
		{
			if( 0 == strcmp( _vname,m_vecVariable[t_i].m_szObjName ) )
				return true;
		}	
	}

	return false;

	unguard;
}


