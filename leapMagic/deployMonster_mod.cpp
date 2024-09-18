///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: deployMonster_mod.cpp
 *
 *  Desc:     用于修改刷怪信息的头文件
 * 
 *  His:      River created @ 2007-3-8
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "deployMonster_mod.h"



/*

怪物信息调整算法描述:
1: 在对话框内,调入刷怪信息文件.
2: 选择一个怪物区域,然后部署该区域.
3: 在场景内画出当前区域的大体位置,使用画线函数.
4: 从区域信息内,得到要部署的怪物,使用插入怪物命令,在区域内插入怪物.
5: 在场景内选择某一个怪物后,激活此怪物,显示此怪物的视野,鼠标点击场景内的位置时,
   发送怪物移动命令,使激活怪物可以移动到鼠标点击的位置上.
6: 记录人工修改的每一个区域内怪物的位置
7: 按 '~'键调出控制台,存储此区域的怪物刷新信息

*/


// 加入怪物时使用的一些操作信息.
s_string gc_deployMonsterMod::m_sMonsterTypeDir[MAX_MONSTERTYPENUM];
int gc_deployMonsterMod::m_sMonsterTypeId[MAX_MONSTERTYPENUM];
int gc_deployMonsterMod::m_iMonsterTypeNum = 0;



gc_deployMonsterMod::gc_deployMonsterMod()
{
	m_bLoadedDeployData = false;
}

gc_deployMonsterMod::~gc_deployMonsterMod()
{

}

//! 得到当前区域内总共有怪物种类
int gc_deployMonsterMod::get_monsterType( int _areaIdx )
{
	guard;

	// 重设数据:
	m_iMonsterTypeNum = 0;
	for( int t_i=0;t_i<MAX_MONSTERTYPENUM;t_i ++ )
	{
		m_sMonsterTypeDir[t_i] = "";
		m_sMonsterTypeId[t_i] = -1;
	}

	// 处理此区域可以加入的怪物名字
	int      t_iMonNum = get_monsterNum( _areaIdx );
	sAreaInfo*  t_ptrAi = get_randAreaFromIdx( _areaIdx );
	sMonstInfo* t_ptrMonInfo = NULL;
	osassert( t_ptrAi );
	int         t_idx;

	for( int t_i=0;t_i<t_iMonNum;t_i ++ )
	{
		// 得到怪物的信息.
		t_ptrMonInfo = &m_sMonstList[t_ptrAi->m_dwofsMonst + t_i];

		// 是否存在怪物的名字列表内
		for( t_idx = 0;t_idx < m_iMonsterTypeNum;t_idx ++ )
		{
			if( m_sMonsterTypeDir[t_idx] == t_ptrMonInfo->m_szMonstName )
				break;
		}

		// 如果不存在,加入
		if( t_idx == m_iMonsterTypeNum )
		{
			m_sMonsterTypeDir[m_iMonsterTypeNum] = t_ptrMonInfo->m_szMonstName;
			m_sMonsterTypeId[m_iMonsterTypeNum] = t_ptrMonInfo->m_dwMonstId;
			m_iMonsterTypeNum ++;
		}
	}

	return m_iMonsterTypeNum;

	unguard;
}

const char* gc_deployMonsterMod::get_monsterTypeDir( int _areaIdx,int _monIdx )
{
	guard;

	osassertex( _monIdx < m_iMonsterTypeNum,"错误的怪物名字索引" );
	return m_sMonsterTypeDir[_monIdx].c_str();

	unguard;
}



//! 初始化静态变量
template<> gc_deployMonsterMod* ost_Singleton<gc_deployMonsterMod>::m_ptrSingleton = NULL;

/** 得到一个sg_timer的Instance指针.
*/
gc_deployMonsterMod* gc_deployMonsterMod::Instance( void )
{
	if( m_ptrSingleton )
		return m_ptrSingleton;
	
	m_ptrSingleton = new gc_deployMonsterMod();
	return m_ptrSingleton;

}


/** 删除一个sg_timer的Instance指针.
*/
void gc_deployMonsterMod::DInstance( void )
{
	SAFE_DELETE( m_ptrSingleton );
}



#define MAKEFOURCC32(ch0, ch1, ch2, ch3)                \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define CurMDVersion  (MAKEFOURCC32(1,0,0,2))
#define CURMDVERSION3 (MAKEFOURCC32(1,0,0,3))
#define CURMDVERSION4 (MAKEFOURCC32(1,0,0,4)) 

//! 处理怪物的目录数据
const char* gc_deployMonsterMod::process_monsterDir( char* _dir )
{
	guard;

	static s_string    t_szTmp;
	int         t_idx;

	t_szTmp = _dir;
	t_idx = (int)t_szTmp.rfind( '\\' );
	t_szTmp = t_szTmp.substr( t_idx+1,t_szTmp.length() );
	t_idx = (int)t_szTmp.find( '.' );
	t_szTmp = t_szTmp.substr( 0,t_idx );


	return t_szTmp.c_str();

	unguard;
}


//! 重设此信息编辑器
void gc_deployMonsterMod::reset_dmmData( void )
{

	m_szDmmFileName = "";
	m_sAreaList.clear();
	m_sMonstList.clear();
	m_vecDmmMonInfo.clear();
	memset( &m_sHeader,0,sizeof( MonstDistribHeader ) );
	m_iRandomAreaNum = 0;

	m_bLoadedDeployData = false;

}



//! 调入和存储
BOOL gc_deployMonsterMod::lod_dmmData( const char* _fname )
{
	guard;

	osassert( _fname );
	osassert( _fname[0] );

	char      t_info[256];


	// 同名的文件，不用调入
	if( m_szDmmFileName == _fname )
		return TRUE;

	// 重设管理器内的内容
	reset_dmmData();


	if( !file_exist( (char*)_fname ) )
		return FALSE;

	BYTE*       t_fstart = NULL;
	int			t_size;

	m_szDmmFileName = _fname;
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)_fname, t_fstart, TMP_BUFSIZE );
	if( t_size == 0 )
	{
		osassertex( false,va( "打开文件<%s>失败",_fname ) );
		return false;
	}

	READ_MEM_OFF(&m_sHeader,t_fstart,sizeof( MonstDistribHeader) );
	if (m_sHeader.id[0]!='M'||m_sHeader.id[1]!='D')
	{
		sprintf( t_info,"<%s>文件格式出错",_fname );
		MessageBox( NULL,t_info,"ERROR",MB_OK );
		return false;
	}

	// River mod @ 2007-6-5:加入资料版本号
	// River mod @ 2007-7-26:加入areaInfo的扩展数据。
	if(m_sHeader.version >= CURMDVERSION3)
	{
		READ_MEM_OFF( &m_sMVHead,t_fstart,sizeof( MapVersionHead ) );
	}
	if( m_sHeader.version != CURMDVERSION4 )
	{
		sprintf( t_info,"打开文件<%s>版本<%d>不对",_fname,m_sHeader.version );
		MessageBox( NULL,t_info,"ERROR",MB_OK );
		return false;
	}

	m_sAreaList.clear();
	m_sMonstList.clear();
	m_sAreaList.reserve(m_sHeader.nAreas);
	m_sMonstList.reserve(m_sHeader.nMonsts);
	sAreaInfo   area;
	sMonstInfo  sm;
	for (int i = 0 ;i<(int)m_sHeader.nAreas;i++)
	{
		READ_MEM_OFF(&area,t_fstart,sizeof(sAreaInfo) );
		m_sAreaList.push_back(area);
		if( area.m_AreaType == TYPE_RANDOM )
			m_iRandomAreaNum ++;
	}
	for(int i = 0 ; i<(int)m_sHeader.nMonsts;i++)
	{
		READ_MEM_OFF(&sm,t_fstart,sizeof(sMonstInfo) );
		m_sMonstList.push_back(sm);
	}
	END_USEGBUF( t_iGBufIdx );	

	m_bLoadedDeployData = true;

	//! 得到客户端容易使用的怪物信息
	m_vecDmmMonInfo.resize( m_sHeader.nMonsts );
	for( int t_i=0;t_i<(int)m_sHeader.nMonsts;t_i ++ )
	{
		m_vecDmmMonInfo[t_i].m_sInsertPt = m_sMonstList[t_i].m_ptPostion;
		
		const char*   t_sz;
		DWORD         t_dwVisualField = 0;
		DWORD         t_dwTrigger,t_dwEscape,t_dwCallDis;
		float         t_fScale;
		t_sz = gc_monsterIdNameMgr::Instance()->get_monstDirFromId( m_sMonstList[t_i].m_dwMonstId,
			t_dwVisualField,t_dwTrigger,t_dwEscape,t_dwCallDis,t_fScale );

		if( t_sz == NULL )
		{
			MessageBox( NULL,m_sMonstList[t_i].m_szMonstName,"这个怪物找不到对应的目录",
				MB_OK );
		}

		strcpy( m_vecDmmMonInfo[t_i].m_szMonsterDir,t_sz );

		strcpy( m_vecDmmMonInfo[t_i].m_szMonsterDir,
			process_monsterDir( m_vecDmmMonInfo[t_i].m_szMonsterDir ) );

		m_vecDmmMonInfo[t_i].m_iMonsterHandle = -1;

		m_vecDmmMonInfo[t_i].m_fVisualField = (float)t_dwVisualField/1000.0f;
		m_vecDmmMonInfo[t_i].m_fTriggerDis = float(t_dwTrigger/1000.0f);
		m_vecDmmMonInfo[t_i].m_fEscapeDis = float(t_dwEscape/1000.0f);
		m_vecDmmMonInfo[t_i].m_fCallDis = float(t_dwCallDis/1000.0f);
		m_vecDmmMonInfo[t_i].m_fScale = t_fScale;

	}

	return TRUE;

	unguard;
}

//! 更新场景内所有的刷怪的位置信息
BOOL gc_deployMonsterMod::update_dmmPosInfo( void )
{
	for( int t_i=0;t_i<(int)m_sMonstList.size();t_i ++ )
	{
		// 更新位置信息并存储
		m_sMonstList[t_i].m_ptPostion = m_vecDmmMonInfo[t_i].m_sInsertPt;
	}
	return true;
}


//! 存储刷怪信息文件
BOOL gc_deployMonsterMod::save_dmmData( void )
{
	guard;

	FILE*   t_file;

	// 还没有调入相关的数据
	if( !this->is_dmmDataLoaded() )
		return true;

	t_file = fopen( this->m_szDmmFileName.c_str(),"wb" );
	if( !t_file )
	{
		MessageBox( NULL,m_szDmmFileName.c_str(),"写入文件失败",MB_OK );
		return false;
	}

	// 存入文件头和区域，怪物信息。
	fwrite( &this->m_sHeader,sizeof( MonstDistribHeader ),1,t_file );

	// 写入103版本特有的数据
	if( m_sHeader.version == CURMDVERSION4 )
		fwrite( &m_sMVHead,sizeof( MapVersionHead ),1,t_file );
	else
		MessageBox( NULL,"错误的版本号...","出错了",MB_OK );



	for( int t_i=0;t_i<(int)m_sAreaList.size();t_i ++ )
		fwrite( &m_sAreaList[t_i],sizeof( sAreaInfo ),1,t_file );
	for( int t_i=0;t_i<(int)m_sMonstList.size();t_i ++ )
		fwrite( &this->m_sMonstList[t_i],sizeof( sMonstInfo ),1,t_file );

	fclose( t_file );


	return true;

	unguard;
}

//! 得到区域的数目
int gc_deployMonsterMod::get_monsterAreaNum( void )
{
	guard;

	return m_iRandomAreaNum;

	unguard;
}

//! 得到第n个随机区域
sAreaInfo* gc_deployMonsterMod::get_randAreaFromIdx( int _idx )
{
	guard;

	osassert( _idx < this->m_iRandomAreaNum );

	int   t_idx = 0;
	for( int t_i=0;t_i<(int)m_sHeader.nAreas;t_i ++ )
	{
		if( m_sAreaList[t_i].m_AreaType != TYPE_RANDOM )
			continue;

		if( _idx == t_idx )
			return &m_sAreaList[t_i];
		else
			t_idx ++;
	}

	return NULL;

	unguard;
}

//! 得到随机区域的索引
bool gc_deployMonsterMod::get_randAreaFromIdx( int _idx,int& _resIdx )
{
	guard;

	osassert( _idx < this->m_iRandomAreaNum );

	int   t_idx = 0;
	for( int t_i=0;t_i<(int)m_sHeader.nAreas;t_i ++ )
	{
		if( m_sAreaList[t_i].m_AreaType != TYPE_RANDOM )
			continue;

		if( _idx == t_idx )
		{
			_resIdx = t_i;
			return true;
		}
		else
			t_idx ++;
	}

	return false;

	unguard;
}


//! 得到此区域内每一个怪物信息
gc_monsterInsert* gc_deployMonsterMod::get_monsterInfoFromIdx( int _areaIdx,int _monIdx )
{
	guard;

	sAreaInfo*   t_ptrAi = get_randAreaFromIdx( _areaIdx );

	if( !t_ptrAi )
		return NULL;

	osassert( _monIdx < (int)t_ptrAi->m_dwnMonsts );
	osassert( m_vecDmmMonInfo.size() >= (t_ptrAi->m_dwofsMonst+_monIdx) );

	return &m_vecDmmMonInfo[t_ptrAi->m_dwofsMonst+_monIdx];

	unguard;
}


const char* gc_deployMonsterMod::get_monsterAreaName( int _idx )
{
	guard;

	return get_randAreaFromIdx( _idx )->m_szName;

	unguard;
}

//! 得到区域的几何数据
const SPoints* gc_deployMonsterMod::get_monAreaGeoData( int _idx )
{
	guard;

	return &get_randAreaFromIdx(_idx)->m_PointList;

	unguard;
}

//! 得到此区域内怪物的数目
int gc_deployMonsterMod::get_monsterNum( int _idx )
{
	guard;

	return get_randAreaFromIdx( _idx )->m_dwnMonsts;

	unguard;
}

//! 在一个区域内删除一个怪物
bool gc_deployMonsterMod::delete_monsterInArea( int _areaIdx,int _monIdx,int _handle )
{
	guard;

	sAreaInfo*   t_ptrAi = get_randAreaFromIdx( _areaIdx );

	if( !t_ptrAi )
		return NULL;


	// 需要删除两个列表
	std::vector<gc_monsterInsert>::iterator   t_iter;
	t_iter = m_vecDmmMonInfo.begin() + t_ptrAi->m_dwofsMonst + _monIdx;
	m_vecDmmMonInfo.erase( t_iter );

	std::vector<sMonstInfo>::iterator   t_iterRealMon;
	t_iterRealMon = m_sMonstList.begin() + t_ptrAi->m_dwofsMonst + _monIdx;
	m_sMonstList.erase( t_iterRealMon );

	// 
	// 数目减少1.
	// 此区域以后的区域,每一个的初始化位移都提前一个
	int   t_idxStart;	
	if( !get_randAreaFromIdx( _areaIdx,t_idxStart ) )
		osassert( false );

	// 两份数据怪物数目都减少为1
	m_sAreaList[t_idxStart].m_dwnMonsts --;
	m_sHeader.nMonsts --;


	for( int t_i=t_idxStart+1;t_i<(int)m_sAreaList.size();t_i ++ )
	{
		// river @ 2007-3-16: 任意的区域类型都应该处理.
		//if( m_sAreaList[t_i].m_AreaType == TYPE_RANDOM )
		m_sAreaList[t_i].m_dwofsMonst --;
	}
	

	return true;

	unguard;
}

//! 从区域索引和怪物索引得到怪物的信息
bool gc_deployMonsterMod::get_monInfoFromAreaTypeIdx( int _areaIdx,int _monIdx,sMonstInfo& _res )
{
	guard;

	sAreaInfo*       t_ptrAi = get_randAreaFromIdx( _areaIdx );
	sMonstInfo*      t_ptrMi;
	
	for( int t_i=0;t_i<(int)t_ptrAi->m_dwnMonsts;t_i ++ )
	{
		t_ptrMi = &m_sMonstList[t_ptrAi->m_dwofsMonst+t_i];
		if( t_ptrMi->m_dwMonstId == m_sMonsterTypeId[_monIdx] )
			break;
	}

	memcpy( &_res,t_ptrMi,sizeof( sMonstInfo ) );
	
	return true;

	unguard;
}

//
bool gc_deployMonsterMod::get_monInsertFromAreaTypeIdx( int _areaIdx,int _monIdx,gc_monsterInsert& _res )
{
	guard;

	sAreaInfo*       t_ptrAi = get_randAreaFromIdx( _areaIdx );
	sMonstInfo*      t_ptrMi;
	gc_monsterInsert*      t_ptrI;

	int t_i;
	for( t_i=0;t_i<(int)t_ptrAi->m_dwnMonsts;t_i ++ )
	{
		t_ptrMi = &m_sMonstList[t_ptrAi->m_dwofsMonst+t_i];
		if( t_ptrMi->m_dwMonstId == m_sMonsterTypeId[_monIdx] )
			break;
	}

	t_ptrI = &m_vecDmmMonInfo[t_i+t_ptrAi->m_dwofsMonst];
	osassert( t_ptrI );

	memcpy( &_res,t_ptrI,sizeof( gc_monsterInsert ) );
	
	return true;

	unguard;
}


/** \brief
*  在一个区域内添加一个怪物
*
*  \return  gc_monsterInsert* 返回怪物的插入结构,用于在上层处理怪物的handle信息
*/
gc_monsterInsert* gc_deployMonsterMod::insert_monsterInArea( int _areaIdx,int _monIdx,int _x,int _z )
{
	guard;

	sMonstInfo       t_sMonInfo;

	get_monInfoFromAreaTypeIdx( _areaIdx,_monIdx,t_sMonInfo );
	t_sMonInfo.m_ptPostion.x = _x;
	t_sMonInfo.m_ptPostion.y = _z;

	// 加入到怪物队列内
	std::vector<sMonstInfo>::iterator   t_iter;
	sAreaInfo*       t_ptrAi = get_randAreaFromIdx( _areaIdx );

	// 加入怪物插入的数据结构
	gc_monsterInsert                          t_sInsert;
	std::vector<gc_monsterInsert>::iterator   t_iterI;
	get_monInsertFromAreaTypeIdx( _areaIdx,_monIdx,t_sInsert );
	t_sInsert.m_sInsertPt.x = _x;
	t_sInsert.m_sInsertPt.y = _z;

	// 两个数据同是插入 
	t_iter = m_sMonstList.begin() + (t_ptrAi->m_dwofsMonst + 1);
	m_sMonstList.insert( t_iter,t_sMonInfo );

	t_iterI = m_vecDmmMonInfo.begin() + (t_ptrAi->m_dwofsMonst + 1 );
	m_vecDmmMonInfo.insert( t_iterI,t_sInsert );


	// 更每一个随机区域的怪物数目信息
	int   t_idxStart;	

	if( !get_randAreaFromIdx( _areaIdx,t_idxStart ) )
		osassert( false );

	// 当前的区域和整体的怪物数目加1
	m_sAreaList[t_idxStart].m_dwnMonsts ++;
	m_sHeader.nMonsts ++;

	for( int t_i=t_idxStart+1;t_i<(int)m_sAreaList.size();t_i ++ )
	{
		// river @ 2007-3-16: 任意的区域类型都应该处理.
		//if( m_sAreaList[t_i].m_AreaType == TYPE_RANDOM )
		m_sAreaList[t_i].m_dwofsMonst ++;
	}


	return &m_vecDmmMonInfo[t_ptrAi->m_dwofsMonst + 1];

	unguard;
}



