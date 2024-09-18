///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacter.cpp
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     用于os引擎中的人物动画调用。
 * 
 *  "女人如甘醇，少饮则提神爽气抑性，多饮则耗神消气，濫饮则灭志杀身"
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/deviceInit.h"
# include "../include/frontPipe.h"
# include "../../interface/stringTable.h"

//! 全局用到的skinMethod.
OSENGINE_API ose_skinMethod            g_eSkinMethod = OSE_D3DINDEXEDVS;

osc_skinMesh*   osc_skinMeshMgr::m_ptrShadowSkinPtr[MAX_RTSHADOWNUM];


//! 定义场景中最多可以使用skinMesh数目,第个场景最多64个skinMesh物品
int  MAX_SCENESMNUM = 64*MAXTG_INTGMGR;


//! 一个Frame可以有多个mesh相对应。
# define MAX_MESH_PERFRAME   4

//! 初始的设备相关资源容器大小。
# define  INIT_SMDEVDEPSIZE  384

//! 初始化boneSys容器和BoneAni容器的大小。
# define  INIT_BONESYSNUM    168

//! 每秒动画使用的帧数。
# define  FPS_PERSECOND      20

//! bodyPartMesh数组的初始化大小。
# define  INT_BPMSIZE        496


//! 定义两个旋转之间的插值时间。
# define ROTINTERPOLATE_TIME 0.2f

//! 定义人物BBox的宽度和高度。
# define CHARACTER_WIDTH    0.4f
# define CHARACTER_HEIGHT   1.8f


//! 是否显示人物为线线框模式.
OSENGINE_API BOOL         g_bCharWireFrame = FALSE;

//! 管理场景中所有人物的设备相关资源。
osc_smDepdevResMgr*         g_smDevdepMgr = NULL;

//! 管理场景中所有的骨骼系统的管理器指针。
osc_boneSysMgr*           g_boneSysMgr;

//! 管理场景中所有的骨骼动画数据的管理器指针。
osc_boneAniMgr*           g_boneAniMgr;


//! 管理场景中身体部分的顶点数据的管理器指针。
osc_bpmMgr*               g_bpmMgr;


/** 
 *   Osok引擎目前人物即时阴影的原理:
 * 　1: 在内存中保存身体部位数据的顶点数据和索引数据。
 *   2: 软件计算每一个每一个身体部位的顶点位置，根据
 *      索引数据来确定每一个三角形是面对光源还是背对光源。
 *   3: 根据身体部位每一条边对应的三角形与光源的关系来确定
 *      当前的边是否是人物的阴影边界。
 *   4: 根据阴影边界和计算后的身体部位顶点位置来计算出Stencil渲染
 *      需要的三角形和索引，填充对应的顶点缓冲区。
 *   5: 设置渲染状态，渲染人物阴影。
 *
 */
//! 主角人物是否显示阴影，默认情况下不显示阴影。
OSENGINE_API BOOL  g_bHeroShadow = true;


//! 用于flipDeterminant的Scale矩阵.
osMatrix osc_skinMesh::m_sFDMatrix;




//! 初始化静态变量
template<> osc_skinMeshMgr* ost_Singleton<osc_skinMeshMgr>::m_ptrSingleton = NULL;

/** 得到一个sg_timer的Instance指针.
*/
osc_skinMeshMgr* osc_skinMeshMgr::Instance( void )
{
	if( !osc_skinMeshMgr::m_ptrSingleton )
	{
		osc_skinMeshMgr::m_ptrSingleton = new osc_skinMeshMgr;
		osassert( osc_skinMeshMgr::m_ptrSingleton );
	}
	
	return osc_skinMeshMgr::m_ptrSingleton;
}

/** 删除一个sg_timer的Instance指针.
*/
void osc_skinMeshMgr::DInstance( void )
{
	SAFE_DELETE( osc_skinMeshMgr::m_ptrSingleton );
}


// 
// ATTENTION TO FIX:
//! 动作的开始帧,因为动作的第零和第一帧完全相同,使用第一帧做为开始帧.
# define ACTIDX_START   1


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      开始骨骼系统和他的管理器相关类,调入*.BFS文件
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

osc_boneSysMgr::osc_boneSysMgr()
{

	// 如果不够的话，可以重新分配。
	this->m_vecBoneSys.resize( INIT_BONESYSNUM );
	this->m_iBoneSysNum = 0;
	g_boneSysMgr = this;

}

osc_boneSysMgr::~osc_boneSysMgr()
{
	StringTableDestroy;
}

/** \brief
*  释放当前的骨骼管理器。
*/
void osc_boneSysMgr::release_boneSysMgr( void )
{
	m_vecBoneSys.clear();

	m_vecBoneSys.resize( INIT_BONESYSNUM );
	m_iBoneSysNum = 0;

}



/** \brief
*  从骨骼系统管理器中调入一套骨骼。
*
*  检测如果在管理器中已经存在了我们需要调入的BoneSys，则只需要返回
*  指针就可以了，否则创建一个新的BoneSys,并返回创建的boneSys指针。
*  
*/
os_boneSys* osc_boneSysMgr::create_boneSysFromMgr( const char* _fname )
{
	guard;

	int       t_i;
	DWORD     t_dwHash = string_hash( _fname );
	
	for( t_i=0;t_i<this->m_iBoneSysNum;t_i++ )
	{
		// River added @ 2010-12-21:加快速度
		if( t_dwHash != m_vecBoneSys[t_i].m_dwHash )
			continue;

		if( m_vecBoneSys[t_i].m_strBoneSysName == _fname )
		{
			return &m_vecBoneSys[t_i];
		}
	}

	//
	// 当前的系统中没有这个资源，调入新的BoneSys资源。
	t_i = (int)m_vecBoneSys.size();
	if( m_iBoneSysNum == t_i )
	{
		//osassert( false );
		m_vecBoneSys.resize( t_i*2 );
		return NULL;
	}

	// River @ 2010-12-29:加入读取速度，去掉此段代码
	// 如果文件不存在，返回错误
	//if( !file_exist( (char*)_fname ) )
	//	return NULL;

	m_vecBoneSys[m_iBoneSysNum].load_boneSysFromFile( _fname );

	m_iBoneSysNum ++;
	
	return &m_vecBoneSys[m_iBoneSysNum-1];

	unguard;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   　　　　　　　骨骼动画的动作相关类，所有的动作都从动作管理器中得到.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////


osc_boneAniMgr::osc_boneAniMgr()
{
	this->m_vecBoneAni.resize( INIT_BONESYSNUM );
	this->m_iBoneAniNum = 0;

	g_boneAniMgr = this;
}

osc_boneAniMgr::~osc_boneAniMgr()
{

}

//! 释放当前的boneAniMgr.
void osc_boneAniMgr::release_boneAniMgr( void )
{
	m_vecBoneAni.clear();
	//m_vecBoneAni.resize( INIT_BONESYSNUM );
	m_iBoneAniNum = 0;
}



/** \brief
*  从文件中创建出当前骨骼系统中全部的动作数据。
*  
*  如果当前的管理器中已经有了这个数据，返回数据指针。否则调入当前骨骼的所有的动画。
*  
*  \return os_boneAin* 如果返回空，则创建失败。
*/
os_boneAni* osc_boneAniMgr::create_boneAni( const char* _aniName,int _aniNum )
{
	guard;

	int     t_i;

	osassert( _aniName );
	osassert( _aniNum>0 );

	DWORD   t_dwHash = string_hash( _aniName );


	for( t_i=0;t_i<this->m_iBoneAniNum;t_i++ )
	{
		// River @ 2010-12-23:加入hash值的比较
		if( t_dwHash != m_vecBoneAni[t_i].m_dwHash )
			continue;
		if( strcmp( m_vecBoneAni[t_i].m_szAniName,_aniName ) == 0 )
			return &m_vecBoneAni[t_i];
	}

	//
	// 如果找不到，从硬盘调入
	t_i = (int)m_vecBoneAni.size();
	if( t_i == this->m_iBoneAniNum )
	{
		osassertex( false,"Too many boneAnimtaion num.." );
		m_vecBoneAni.resize( t_i*2 );
		return NULL;
	}

	if( !m_vecBoneAni[m_iBoneAniNum].load_boneAction( _aniName,_aniNum ) )
		return NULL;

	m_iBoneAniNum++;

	return &m_vecBoneAni[m_iBoneAniNum-1];

	unguard;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      开始Mesh数据相关类和他的管理器，调入*.SKN文件
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////


os_smMeshDevdep::os_smMeshDevdep()
{
	this->m_pBuf = NULL;
	this->m_pIdxBuf = NULL;

	m_bUsed = false;

	m_strMFname[0] = NULL;

# if __GLOSSY_EDITOR__
	m_wTriIdx = NULL;
	m_iBackIdxNum = 0;
	m_bFirstSel = true;
# endif 
	m_dwHash = -1;

}

os_smMeshDevdep::~os_smMeshDevdep()
{
	if( m_bUsed )
		release_obj();



}

# if __GLOSSY_EDITOR__
/** brief
*  只显示某个身体部位的部分三角形,公用于调试目的
*
*  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
*  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
*/
int os_smMeshDevdep::displaytri_bysel( int _triidx[24],int _trinum )
{
	guard;

	// 头一次处理的时候，先备份当前的索引缓冲区数据
	HRESULT   t_hr;
	WORD*     t_ptrStart = NULL;
	if( m_bFirstSel )
	{
		t_hr = m_pIdxBuf->Lock( 0,0,(void**)&t_ptrStart,0);
		osassert( !FAILED( t_hr ) );

		m_iBackIdxNum = m_iIdxNum;
		m_wTriIdx = new WORD[m_iIdxNum];
		memcpy( m_wTriIdx,t_ptrStart,sizeof( WORD )*m_iIdxNum );

		m_pIdxBuf->Unlock();

		m_bFirstSel = false;
	}


	// 锁定缓冲区，并操作
	t_hr = m_pIdxBuf->Lock( 0,0,(void**)&t_ptrStart,0);
	osassert( !FAILED( t_hr ) );

	// 根据选择的三角形填充新的缓冲区数据
	if( _trinum == 0 )
	{
		// 使用原始的索引数据
		memcpy( t_ptrStart,m_wTriIdx,sizeof( WORD )*m_iBackIdxNum );
		m_iIdxNum = m_iBackIdxNum;
	}
	else
	{
		for( int t_i=0;t_i<_trinum;t_i ++ )
		{
			memcpy( t_ptrStart + sizeof( WORD )*t_i*3,
				&m_wTriIdx[_triidx[t_i]*3],sizeof( WORD )*3 );
		}
		m_iIdxNum = _trinum*3;
	}
	m_pIdxBuf->Unlock();

	//@{
	// TEST CODE: 输出要显示的三角形对应的顶点数据：
	os_characterVer*   t_ptrVerStart;
	t_hr = m_pBuf->Lock( 0,0,(void**)&t_ptrVerStart,0 );

	for( int t_i=0;t_i<_trinum;t_i ++ )
	{
		for( int t_j=0;t_j<3;t_j ++ )
		{
			osDebugOut( "The %d 个面 %d 个顶点,索引<%d>:<%f,%f,%f><%f,%f,%f>..\n",
				t_i,t_j,
				m_wTriIdx[_triidx[t_i]*3]+t_j,
				t_ptrVerStart[m_wTriIdx[_triidx[t_i]*3]+t_j].m_vec3Pos.x,
				t_ptrVerStart[m_wTriIdx[_triidx[t_i]*3]+t_j].m_vec3Pos.y,
				t_ptrVerStart[m_wTriIdx[_triidx[t_i]*3]+t_j].m_vec3Pos.z,

				t_ptrVerStart[m_wTriIdx[_triidx[t_i]*3]+t_j].m_vec3Normal.x,
				t_ptrVerStart[m_wTriIdx[_triidx[t_i]*3]+t_j].m_vec3Normal.y,
				t_ptrVerStart[m_wTriIdx[_triidx[t_i]*3]+t_j].m_vec3Normal.z );

		}
	}
	m_pBuf->Unlock();
	//@}


	return m_iBackIdxNum;

	unguard;
}
# endif 	


//! on deivce lost.
bool os_smMeshDevdep::onlost_smMesh( void )
{
	SAFE_RELEASE( m_pBuf );
	SAFE_RELEASE( m_pIdxBuf );
	return true;
}


//! restore dev dep resource.
bool os_smMeshDevdep::restore_smMesh( LPDIRECT3DDEVICE9 _dev )
{
	guard;

	HRESULT         t_hr;
	BYTE*           t_pByte;
	BYTE*           t_fstart;
	WORD*           t_pIdx;

	if( m_pBuf && m_pIdxBuf )
		return true;

	osassert( (!m_pBuf)&&(!m_pIdxBuf) );

	// 读入数据到内存。
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf( m_strMFname,t_fstart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
		return false;

	//
	// 读入文件头.
	char      t_str[4];
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,SKN_FILEMAGIC ) != 0 )
	{
		osassertex( false,va( "<%s> file Format error!!!",m_strMFname ) );
		return false;
	}

	// ATTENTION TO FIX:版本号必须是最终文件的版本号：
	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );

	//
	// 读入文件信息
	t_fstart += sizeof( DWORD );
	t_fstart += sizeof( DWORD );

	t_fstart += sizeof( DWORD );
	t_fstart += sizeof( DWORD );

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	t_fstart += sizeof( DWORD );

	t_fstart += sizeof( DWORD );
	t_fstart += sizeof( DWORD );


	//
	// 调入每一个骨骼的matrix.
	t_fstart += sizeof( osMatrix )*t_iSize;
	t_fstart += sizeof( DWORD )*t_iSize;

	//
	// 创建设备相关的数据,这个函数中，不需要使用全局的缓冲区.
	//
	BYTE*   t_ptrDStart = t_fstart;
	WORD*   t_ptrIStart = (WORD*)(t_fstart + m_iVerNum*m_iVerSize);

	//
	// 创建和填充顶点缓冲区。
	D3DPOOL    t_dwPool = D3DPOOL_DEFAULT;
	t_hr = _dev->CreateVertexBuffer( 
		m_iVerSize*m_iVerNum,D3DUSAGE_WRITEONLY,
		0, t_dwPool ,&m_pBuf,NULL );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}

	t_hr = m_pBuf->Lock( 0,0,(void**)&t_pByte,0 );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}
	memcpy( t_pByte,t_ptrDStart,m_iVerSize*m_iVerNum );
	m_pBuf->Unlock();

	//
	// 创建和填充index buffer.
	t_hr = _dev->CreateIndexBuffer( 
		sizeof( WORD )*m_iIdxNum,D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, t_dwPool ,
		&m_pIdxBuf,NULL );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}
	t_hr = m_pIdxBuf->Lock( 0,0,(void**)&t_pIdx,0 );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}
	memcpy( t_pIdx,t_ptrIStart,sizeof( WORD )*m_iIdxNum );
	m_pIdxBuf->Unlock();

	END_USEGBUF( t_iGBufIdx );
	
		
	return true;

	unguard;
}


/** \brief
 *  使用文件初始化当前的dev dep resource.
 *
 *  如果在当前的mgr中已经存在了我们的资源，返回资源id,并把资源
 *  的ref数目加一。
 *  
 */
bool os_smMeshDevdep::init_smMesh( os_smDevdepDataInit& _vi,LPDIRECT3DDEVICE9 _dev )
{
	guard;
	
	HRESULT         t_hr;
	BYTE*           t_pByte;
	WORD*           t_pIdx;

	osassert( _dev );
	osassert( _vi.m_strFname );
	osassert( _vi.m_ptrDStart );
	osassert( _vi.m_ptrIStart );

	// 
	// 保存当前devdep数据对应的文件名。
	osassert( strlen( _vi.m_strFname ) < 64 );
	strcpy( m_strMFname,_vi.m_strFname );
	m_dwHash = string_hash( m_strMFname );
	m_iVerSize = _vi.m_iVerSize;
	m_iVerNum = _vi.m_iVerNum;
	m_iIdxNum = _vi.m_iIdxNum;

	//
	// 创建和填充顶点缓冲区。
	D3DPOOL    t_dwPool = D3DPOOL_DEFAULT;
	t_hr = _dev->CreateVertexBuffer( 
		m_iVerSize*m_iVerNum,D3DUSAGE_WRITEONLY,
		0, t_dwPool ,&this->m_pBuf,NULL );
	if( FAILED( t_hr ) )
	{
		osassertex( false,va( "错误:<%s>,请单开游戏，并开启最低游戏配置...\n",
			(char*)osn_mathFunc::get_errorStr( t_hr ) ) );
		return false;
	}

	t_hr = m_pBuf->Lock( 0,0,(void**)&t_pByte,0 );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}
	memcpy( t_pByte,_vi.m_ptrDStart,m_iVerSize*m_iVerNum );
	m_pBuf->Unlock();

	//
	// 创建和填充index buffer.
	t_hr = _dev->CreateIndexBuffer( 
		sizeof( WORD )*m_iIdxNum,D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, t_dwPool ,
		&m_pIdxBuf,NULL );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}
	t_hr = m_pIdxBuf->Lock( 0,0,(void**)&t_pIdx,0 );
	if( FAILED( t_hr ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}
	memcpy( t_pIdx,_vi.m_ptrIStart,sizeof( WORD )*m_iIdxNum );
	m_pIdxBuf->Unlock();
	
	m_bUsed = true;
		

	return true;

	unguard;
}

/** \brief
*  释放当前资源的纯虚函数，当当前的引用计数变为0时，自动调用这个
*  函数。
*/
void os_smMeshDevdep::release_obj( DWORD _ptr/* = NULL*/ )
{
	guard;

	//if( m_strMFname[0] )
		//osDebugOut( "The <%s> smMesh release from Device..\n",m_strMFname );

	SAFE_RELEASE( m_pBuf );
	SAFE_RELEASE( m_pIdxBuf );
	m_bUsed = false;
	m_iVerNum = 0;
	m_iVerSize = 0;

	m_strMFname[0] = NULL; 
 
# if __GLOSSY_EDITOR__
	m_bFirstSel = true;
	m_iBackIdxNum = 0;
	SAFE_DELETE_ARRAY( m_wTriIdx );
# endif 
	m_dwHash = -1;

	unguard;
}



osc_smDepdevResMgr::osc_smDepdevResMgr()
{
	g_smDevdepMgr = this;

	// River @ 2010-12-21:使用跟bpm数据大小一样的数据，每一个bpm数据对应一个devdep数据。
	//                    这个不存在继续复用的问题。
	m_vecDevdepRes.resize( INT_BPMSIZE );
}

osc_smDepdevResMgr::~osc_smDepdevResMgr()
{

}


/** \brief
*  初始化当前的manager.
*/
void osc_smDepdevResMgr::init_smDevdepMgr( LPDIRECT3DDEVICE9 _dev )
{
	m_pd3dDevice = _dev;

}


//! 释放当前的skinMesh Devdep Manager.
void osc_smDepdevResMgr::release_smDevdepMgr( bool _finalRelease/* = true*/ )
{

}




/*** \brief
*  释放一个body part mesh的设备相关数据。
*  
*  \param _id 要释放mesh在devdep manager中的id.
*  
*/
void osc_smDepdevResMgr::release_devdepMesh( int _id )
{
	guard;

	if( _id<0 )
		return;

	if( m_vecDevdepRes[_id].get_ref() > 0 )
		m_vecDevdepRes[_id].release_ref();

	unguard;
}

# if __GLOSSY_EDITOR__
/** brief
*  只显示某个身体部位的部分三角形,公用于调试目的
*
*  \param int _resId  要处理的resourceId
*  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
*  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
*/
int osc_smDepdevResMgr::displaytri_bysel( int _resId,int _triidx[24],int _trinum )
{
	guard;

	osassert( m_vecDevdepRes[_resId].m_bUsed );

	return m_vecDevdepRes[_resId].displaytri_bysel( _triidx,_trinum );

	unguard;
}
# endif 


//! lost&restore devdepMeshMgr.
void osc_smDepdevResMgr::onlost_devdepMesh( void )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecDevdepRes.size();t_i++ )
	{
		if( m_vecDevdepRes[t_i].m_bUsed )
			m_vecDevdepRes[t_i].onlost_smMesh();
	}

	unguard;
}
void osc_smDepdevResMgr::restore_devdepMesh( void )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecDevdepRes.size();t_i++ )
	{
		if( m_vecDevdepRes[t_i].m_bUsed )
			m_vecDevdepRes[t_i].restore_smMesh( m_pd3dDevice );
	}

	unguard;
}


/** \brief
*  从Mesh文件创建设备相关的数据。
*  
*  \return int -1表示处理失败。
*/
int osc_smDepdevResMgr::create_devdepMesh( os_smDevdepDataInit& _vi )
{
	guard;

	int     t_i,t_iCreateId;

	//
	// 找到可以创建新的设备相关资源的id. 
	t_iCreateId = -1;
	for( t_i=0;t_i<(int)m_vecDevdepRes.size();t_i++ )
	{
		if( !m_vecDevdepRes[t_i].m_bUsed )
		{
			t_iCreateId = t_i;
			break;
		}
	}
	if( t_iCreateId == -1 )
	{
		// 因为多线程，其它的地方使用了过多的指针引用，所以不能扩大空间
		// 最可能的办法，就是再加一个数组来解决这个问题。
		osassertex( false,"加入更多的人物SM空间...\n" );
	}

	//
	// 使用可用的资源创建新的设备相关资源，并增加ref值。
	if( !m_vecDevdepRes[t_iCreateId].init_smMesh( _vi,m_pd3dDevice ) )
	{
		osassertex( false,"创建人物mesh失败...\n" );
		return -1;
	}
	m_vecDevdepRes[t_iCreateId].add_ref();

	return t_iCreateId;

	unguard;
}


osc_bodyPartMesh::osc_bodyPartMesh()
{

	m_bInuse = false;

	m_strFname[0] = NULL;
	m_dwHash = -1;


	m_ptrSwordGlossy = NULL;

	memset( &m_sMaterial,0,sizeof( D3DMATERIAL9 ) );
	m_sMaterial.Diffuse.r = 1.0f;
	m_sMaterial.Diffuse.g = 1.0f;
	m_sMaterial.Diffuse.b = 1.0f;
	m_sMaterial.Diffuse.a = 1.0f;

	m_sMaterial.Ambient.r = 1.0f;
	m_sMaterial.Ambient.g = 1.0f;
	m_sMaterial.Ambient.b = 1.0f;
	m_sMaterial.Ambient.a = 1.0f;

# if __GLOSSY_EDITOR__
	m_ptrVerData = NULL;

# endif 
		//@{
	// Windy mod @ 2005-8-27 17:12:49
	//Desc: ride 默认不是第二部分
	m_IsRideBox = false;
	m_ptrRide = NULL;
	//@}

}


osc_bodyPartMesh::~osc_bodyPartMesh()
{
	release_obj();

# if __GLOSSY_EDITOR__
	SAFE_DELETE_ARRAY( m_ptrVerData );
# endif 

}


//  释放一个人物的时候，并非释放此人物所有的数据，硬盘数据使用的时候再释放。
//! river added @ 2010-12-21:真正释放
void osc_bodyPartMesh::release_bmpRes( void )
{
	if( m_bInuse )
	{
		//
		// 释放当前BPM在设备相关管理器和shader管理器中的引用。
		g_smDevdepMgr->release_devdepMesh( m_iDevdepResId );

	}

	// 如果有刀光相关的内容。处理。
	if( m_ptrSwordGlossy )
		SAFE_DELETE( m_ptrSwordGlossy );
	//@{
	// Windy mod @ 2005-8-31 9:27:41
	//Desc: ride 处理坐骑的释放
	if(m_ptrRide)
		SAFE_DELETE(m_ptrRide);
	//@}


	// 释放当前BPM分配的内存。
	m_iDevdepResId = -1;
	m_bInuse = false;

	m_dwHash = -1;

}


/** \brief
*  释放当前资源的纯虚函数，当当前的引用计数变为0时，自动调用这个
*  函数。
*/
void osc_bodyPartMesh::release_obj( DWORD _ptr/* = NULL*/ )
{
	static DWORD  t_dwReleaseTimes = 0;

	m_dwReleaseCounter = t_dwReleaseTimes;

	t_dwReleaseTimes ++;

# if 0
	if( m_bInuse )
	{
		//
		// 释放当前BPM在设备相关管理器和shader管理器中的引用。
		g_smDevdepMgr->release_devdepMesh( m_iDevdepResId );

	}

	// 如果有刀光相关的内容。处理。
	if( m_ptrSwordGlossy )
		SAFE_DELETE( m_ptrSwordGlossy );
	//@{
	// Windy mod @ 2005-8-31 9:27:41
	//Desc: ride 处理坐骑的释放
	if(m_ptrRide)
		SAFE_DELETE(m_ptrRide);
	//@}


	// 释放当前BPM分配的内存。
	m_iDevdepResId = -1;
	m_bInuse = false;

	m_dwHash = -1;
# endif 

}



osc_bpmMgr::osc_bpmMgr()
{
	m_vecBPM.resize( INT_BPMSIZE );

	// 全局只有一个bodyPartMesh Manager.
	g_bpmMgr = this;

}

osc_bpmMgr::~osc_bpmMgr()
{

}

//!  释放当前的bpmMgr
void osc_bpmMgr::release_bpmMgr( void )
{
	m_vecBPM.clear();

}



//!  使用3d设备初始化当前的bpmMgr.
void osc_bpmMgr::init_bmpMgr( LPDIRECT3DDEVICE9 _dev )
{
	guard;// osc_bpmMgr::init_bmpMgr() );

	osassert( _dev );
	m_pd3dDevice = _dev;

	unguard;
}


//!  从文件中调入bpm.
int osc_bpmMgr::create_bpm( const char* _bpName,bool _IsRide)
{
	guard;

	osc_bodyPartMesh*  t_ptr;
	int               t_i,t_idx;
	DWORD             t_dwHash;

	osassert( _bpName );
	osassert( strlen( _bpName ) < MAX_BPMFNLEN );

	t_dwHash = string_hash( _bpName );

	//
	// 如果内存中存在我们需要的文件，返回并增加引用计数。
	for( t_i=0;t_i<(int)m_vecBPM.size();t_i++ )
	{
		t_ptr = &m_vecBPM[t_i];
		if( t_ptr->is_inUsed() )
		{
			// River @ 2010-12-21: 先hash值比较一下。
			if( t_ptr->m_dwHash != t_dwHash )
				continue;

			if( strcmp( t_ptr->m_strFname,_bpName ) == 0 )
			{
				if( _IsRide )
				{
					if( t_ptr->m_ptrRide )
					{
						// 增加引用。
						t_ptr->add_ref();
					}
					else
					{
						int t_iRef = t_ptr->get_ref();
						for( int t_ref = 0;t_ref < t_iRef;t_ref ++ )
							t_ptr->release_ref();
						//! 必须释放所对应的资源，否则出错
						t_ptr->release_bmpRes();

						// 如果调入为非骑乘模式，而现在需要骑乘模式，重新调入数据 
						if( !t_ptr->LoadMesh( (char*)_bpName,_IsRide,t_iRef ) )
							return -1;

						t_ptr->add_ref();
					}
				}
				else
					// 增加引用。
					t_ptr->add_ref();

				return t_i;
			}

		}
	}

	//
	// 找到可以创建新的mesh的空间.
	t_idx = -1;
	for( t_i=0;t_i<(int)m_vecBPM.size();t_i++ )
	{
		if( !m_vecBPM[t_i].is_inUsed() )
		{
			t_idx = t_i;
			break;
		}
	}

	//  River @ 2010-12-21:
	//! 如果找不到可以创建的空间，找一个ref为零的空间，释放。
	//  这么做的好处是内存可以缓存一些人物的skinMesh数据，减小对硬盘调入数据的需求。
	DWORD  t_dwRC = 0xffffffff;
	if( t_idx == -1 )
	{
		for( t_i=0;t_i<(int)m_vecBPM.size();t_i++ )
		{
			if( m_vecBPM[t_i].get_ref() <= 0 )
			{
				if( m_vecBPM[t_i].m_dwReleaseCounter < t_dwRC )
				{
					t_dwRC = m_vecBPM[t_i].m_dwReleaseCounter;
					t_idx = t_i;
				}
			}
		}

		if( t_idx != -1 )
			m_vecBPM[t_idx].release_bmpRes();
	}


	//
	// 如果空间不够，再加大
	if( t_idx == -1 )
	{
		// 因为多线程，其它的地方使用了过多的指针引用，所以不能扩大空间
		// 最可能的办法，就是再加一个数组来解决这个问题。
		osassertex( false,"增加更大的bpm空间496...\n" );
	}

	//
	// 调入函数。
	if( !m_vecBPM[t_idx].LoadMesh( (char*)_bpName ,_IsRide) )
		return -1;

	m_vecBPM[t_idx].add_ref();
	return t_idx;

	unguard;
}


//! 释放bpm的引用。
void osc_bpmMgr::release_bpm( int _id )
{
	guard;

	osassert( _id<(int)m_vecBPM.size() );
	if( m_vecBPM[_id].get_ref() > 0 )
		m_vecBPM[_id].release_ref();

	unguard;
}



//
// BodyPartMesh的实例，用于共享bodyPartMesh.
osc_bpmInstance::osc_bpmInstance()
{
	m_ptrBodyPartMesh = NULL;
	m_iBpmId = -1;
	m_ptrSGIns = NULL;

	m_ptrShadow = NULL;

	m_ptrSecEffect = NULL;

	m_iShaderID = -1;
	m_ptrBpmTexture = NULL;
	this->m_bAlpha = false;
	//@{
	// Windy mod @ 2005-8-29 10:59:39
	//Desc: ride
	m_ptrRideIns = NULL;
	this->m_iCRNodeIdx = -1;
	//@}

	m_bHide = false;

	m_dwHashId = 0xffffffff;

	m_iNodeIdx = -1;


}

osc_bpmInstance::~osc_bpmInstance()
{

	if( m_iNodeIdx >= 0 )
	{
		if( m_ptrSGIns )
			m_ptrSGIns->delete_devdepRes();
		m_vecDGlossyIns.release_node( m_iNodeIdx );
		m_ptrSGIns = NULL;
		m_iNodeIdx = -1;
	}

	release_bpmIns();

}

//! river added @ 2010-12-22: 加入一次性的内存分配
CSpaceMgr<osc_swordGlossyIns> osc_bpmInstance::m_vecDGlossyIns;


//! 得到一个实例化的指针.
osc_swordGlossyIns* osc_bpmInstance::get_glossyInsPtr( void )
{
	guard;

	m_iNodeIdx = m_vecDGlossyIns.get_freeNode();
	return m_vecDGlossyIns.get_nodePtr( m_iNodeIdx );

	unguard;
}

//! River @ 2010-12-24:预分配坐骑相关的数据指针。
CSpaceMgr<osc_ChrRideInst> osc_bpmInstance::m_vecChrRideIns;
//! 得到一个坐骑的实例化数据
osc_ChrRideInst* osc_bpmInstance::get_chrRidePtr( void )
{
	guard;

	m_iCRNodeIdx = m_vecChrRideIns.get_freeNode();
	return m_vecChrRideIns.get_nodePtr( m_iCRNodeIdx );

	unguard;
}



/** \brief
*  使用这个函数调入bodyPartMesh,osc_skinMesh类使用这个接口来操作bodyPartMesh相关的操作。
*
*  使用bpm的Manager来得到bodypartMesh的指针。
*
*  \param bool _shadowSupport   调入的bmpInstance是否支持人物阴影相关的数据。
*/
bool osc_bpmInstance::loadBpmIns( char *  _file,bool _playSg,bool _playWEff,
						bool _shadowSupport/* = false*/,bool _IsRide/* = false*/,
						bool _topLayer/* = false*/,osc_skinMesh* _mesh/* = NULL*/  )
{
	guard;

	m_iBpmId = g_bpmMgr->create_bpm( _file,_IsRide );
	osassert( m_iBpmId>=0 );
	if( m_iBpmId < 0 )
		return false;

	m_ptrBodyPartMesh = g_bpmMgr->get_bpmPtrFromId( m_iBpmId );

	// 
	// 如果人物的的武器部分有刀光数据，应该在此处设置刀光Ins数据。
	if( m_ptrBodyPartMesh->m_ptrSwordGlossy )
	{
		if( !m_ptrSGIns )
			m_ptrSGIns = get_glossyInsPtr();

		m_ptrSGIns->set_swordGlossyPtr( m_ptrBodyPartMesh->m_ptrSwordGlossy,_mesh );
		m_ptrSGIns->reset_swordGlossy();

		// River mod @ 2006-6-21:上层可控制是否显示武器特效，
		// 在调用相应的接口时，再创建特效的设备相关数据
		// 创建刀光用到的特效设备相关数据
		if( !m_ptrSGIns->create_devdepRes( 0,_playSg,_playWEff,_topLayer ) )
			return false;
	}
	//@{
	// Windy mod @ 2005-8-29 10:56:36
	//Desc: ride 
	if (m_ptrBodyPartMesh->m_ptrRide)
	{
		// River mod @ 2010-12-24:此处内存修改为非动态分配，加快速度
		if (!m_ptrRideIns)
			m_ptrRideIns = get_chrRidePtr();
		m_ptrRideIns->m_ptrRide = m_ptrBodyPartMesh->m_ptrRide;
	}

	//@}

	// 
	// 初始化阴影相关的数据。
	if( _shadowSupport )
	{
		// 必须从硬盘上读入，此顶点缓冲区为只写顶点缓冲.
		if( m_ptrShadow )
			m_ptrShadow->release_shadowData();
		else
			m_ptrShadow = new osc_bpmShadow;
		m_ptrShadow->m_iIdxNum = m_ptrBodyPartMesh->m_dwNumIndices;
		m_ptrShadow->m_iVerNum = m_ptrBodyPartMesh->m_dwNumVertex;
		m_ptrShadow->m_ptrBpmIdx = new WORD[m_ptrShadow->m_iIdxNum];
		m_ptrShadow->m_ptrBpmVer = new os_characterVer[m_ptrShadow->m_iVerNum];
		m_ptrShadow->m_ptrBpmVerAT = new osVec3D[m_ptrShadow->m_iVerNum];

		m_ptrBodyPartMesh->LoadMeshVNI( _file,
			(BYTE*)m_ptrShadow->m_ptrBpmVer,(BYTE*)m_ptrShadow->m_ptrBpmIdx );

		// 调入阴影的边列表数据。
		if( !m_ptrShadow->init_shadowData( _file ) )
		{
			osassert( false );
			return false;
		}
	}

	return true;

	unguard;
}



//@{
//! River @ 2010-12-28:shader的缓冲数据 
VEC_chrShaderBuf osc_bpmInstance::m_vecChrShaderBuf;

//! 释放shader.
void os_chrShaderBuf::reset( void )
{
	if( (m_iShaderId>=0)&&g_shaderMgr )
		g_shaderMgr->release_shader( m_iShaderId );
	m_iShaderId = -1;
	m_dwHashId = 0xffffffff;
	m_dwReleaseCount = 0xffffffff;
}


//! shaderBuf数据相关。
int osc_bpmInstance::get_freeBuf( void )
{
	guard;

	for( int t_i = 0;t_i<(int)m_vecChrShaderBuf.size();t_i ++ )
	{
		if( m_vecChrShaderBuf[t_i].m_iShaderId == -1 )
			return t_i;
	}

	DWORD  t_dwRC = 0xffffffff;
	int    t_idx = -1;

	// 没有空闲，找到最近最少使用的那个，释放掉。
	for( int t_i = 0;t_i<(int)m_vecChrShaderBuf.size();t_i ++ )
	{
		if( t_dwRC > m_vecChrShaderBuf[t_i].m_dwReleaseCount )
		{
			t_dwRC = m_vecChrShaderBuf[t_i].m_dwReleaseCount;
			t_idx = t_i;
		}
	}

	osassert( t_idx >= 0 );
	m_vecChrShaderBuf[t_idx].reset();

	return t_idx;

	unguard;
}

void osc_bpmInstance::release_insShader( int _shaderId,bool _finalRelease/* = false*/ )
{
	guard;

	static DWORD  t_dwReleaseC = 0;

	// River @ 2011-2-17:为了上层的编辑器使用
	if( _finalRelease )
	{
		while( g_shaderMgr->get_shaderRef( _shaderId ) >= 1 )
		{
			if( g_shaderMgr->get_shaderRef( _shaderId ) == 1  )
			{
				g_shaderMgr->release_shader( _shaderId );
				break;
			}
			else
				g_shaderMgr->release_shader( _shaderId );
		}

		for( int t_i = 0;t_i<(int)m_vecChrShaderBuf.size();t_i ++ )
		{
			if( m_dwHashId == m_vecChrShaderBuf[t_i].m_dwHashId )
				m_vecChrShaderBuf[t_i].reset();
		}

		return;
	}


	// 暂不释放，缓冲起来。
	if( g_shaderMgr->get_shaderRef( _shaderId ) == 1 )
	{
		int t_idx = get_freeBuf();
		t_dwReleaseC ++;
		m_vecChrShaderBuf[t_idx].m_dwHashId = m_dwHashId;
		m_vecChrShaderBuf[t_idx].m_dwReleaseCount = t_dwReleaseC;
		m_vecChrShaderBuf[t_idx].m_iShaderId = _shaderId;
	}
	else
		g_shaderMgr->release_shader( _shaderId );

	return;

	unguard;
}

int osc_bpmInstance::get_bufShaderId( DWORD _hashId )
{
	guard;

	int  t_iShaderId = -1;
	for( int t_i = 0;t_i<(int)m_vecChrShaderBuf.size();t_i ++ )
	{
		if( m_vecChrShaderBuf[t_i].m_iShaderId == -1 )
			continue;
		if( _hashId == m_vecChrShaderBuf[t_i].m_dwHashId )
		{
			t_iShaderId = m_vecChrShaderBuf[t_i].m_iShaderId;
			m_vecChrShaderBuf[t_i].m_iShaderId = -1;
			m_vecChrShaderBuf[t_i].m_dwHashId = 0xffffffff;
			m_vecChrShaderBuf[t_i].m_dwReleaseCount = 0xffffffff;
			osassert( g_shaderMgr->get_shaderRef( t_iShaderId )>= 1 );
			break;
		}
	}

	return t_iShaderId;

	unguard;
}
//@} 


/** \brief
*  释放当前的bpmInstance.
*/
void osc_bpmInstance::release_bpmIns( void )
{

	if( m_iBpmId>=0 )
	{
		g_bpmMgr->release_bpm( m_iBpmId );
		m_iBpmId = -1;
	}

	if( m_iShaderID >= 0 )
	{
		//g_shaderMgr->release_shader( m_iShaderID );
		release_insShader( m_iShaderID );
		m_iShaderID = -1;
	}

	m_ptrBpmTexture = NULL;
	m_bAlpha = false;
	
	m_ptrBodyPartMesh = NULL;
	//@{
	// Windy mod @ 2005-9-2 9:38:40
	//Desc: ride 释放坐骑指针
	//SAFE_DELETE(m_ptrRideIns);
	//@}
	// River @ 2010-11-24:
	if( m_ptrRideIns )
	{
		m_vecChrRideIns.release_node( m_iCRNodeIdx );
		m_iCRNodeIdx = -1;
		m_ptrRideIns = NULL;
	}

	//  
	//! River @ 2010-7-1:确认当前部位重设后不隐藏
	m_bHide = false;

	if( m_iNodeIdx >= 0 )
	{
		if( m_ptrSGIns )
			m_ptrSGIns->delete_devdepRes();
		m_vecDGlossyIns.release_node( m_iNodeIdx );
		m_ptrSGIns = NULL;
		m_iNodeIdx = -1;
	}

	SAFE_DELETE( m_ptrShadow );
	if( m_ptrSecEffect )
	{
		m_ptrSecEffect->delete_devdepRes();
		osc_skinMesh::release_secEffectPtr( m_ptrSecEffect );
	}
	m_ptrSecEffect = NULL;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 　　　　　　　　　　　　　　　　START OF bodyFrame 相关类.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
osc_bodyFrame::osc_bodyFrame(DWORD _id)
{
	osMatrixIdentity(&m_matRot);
	osMatrixIdentity(&m_matCombined);	

	m_vecPtrBpmIns.resize( MAX_MESH_PERFRAME );
	 
	Reset_Frame();

}

osc_bodyFrame::~osc_bodyFrame()
{
	//
	// 对子结点的释放由osc_skinMesh类管理，那个类分配了frame子结点的内存。

}

/** \brief
*  重置当前的frame为初始状态，释放占用的资源。
*/
void osc_bodyFrame::Reset_Frame( void )
{
	m_dwID = 0;
	m_pSibling		= NULL;
	m_pFirstChild	= NULL;
	m_pParentSkin	= NULL;

	m_iBMNum = 0;

}


osc_bodyFrame* osc_bodyFrame::FindFrame ( DWORD _dwID )
{
	osc_bodyFrame *pframe;
	if(m_dwID == _dwID){
		return this;
	}

	if(m_pFirstChild != NULL)
	{
		pframe = m_pFirstChild->FindFrame( _dwID );
		if(pframe != NULL)
			return pframe;
	}

	if(m_pSibling != NULL)
	{
		pframe = m_pSibling->FindFrame( _dwID );
		if( pframe != NULL )
			return pframe;
	}

	return NULL;
}






/**  \brief
 *   给一个Frame结构加子Frame
 */
void osc_bodyFrame::AddFrame(osc_bodyFrame *_pframe)
{
	if( m_pFirstChild == NULL )
	{
		m_pFirstChild = _pframe;
	}
	else
	{
		_pframe->m_pSibling = m_pFirstChild->m_pSibling;
		m_pFirstChild->m_pSibling = _pframe;
	}

	return;
}



/** \brief
*  使用设备相关数据的id加入新的设备相关数据。
*/
void osc_bodyFrame::AddMesh( const osc_bpmInstance* _pm )
{
	guard;

	int   t_i = (int)m_vecPtrBpmIns.size();
	if( t_i == m_iBMNum )
	{
		m_vecPtrBpmIns.resize( t_i*2 );
		
		s_string    t_szDirName;
		m_pParentSkin->get_smDir( t_szDirName );
		osassertex( false,(char*)t_szDirName.c_str() );

	}

	m_vecPtrBpmIns[m_iBMNum] = (osc_bpmInstance *)_pm;

	m_iBMNum ++;

	return;
	unguard;
}


/** \brief
*  把Frame中对应的mesh使用新的bpMesh替换。
*
*  \param  _npm 要替换成的新的part mesh Instance.
*/
void osc_bodyFrame::change_Mesh( osc_bpmInstance* _pm,
						  const osc_bpmInstance* _npm )
{
	guard;// 


	osassert( _pm );

	osassertex( m_iBMNum>0,"当前BodyFrame的BM数目不大于零" );

	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		if( m_vecPtrBpmIns[t_i] != _pm )
			continue;

		// 
		// 如果换上的mesh为空，则对应的mesh减一.
		if( _npm == NULL )
		{
			for( int t_j=t_i;t_j<m_iBMNum-1;t_j++ )
				m_vecPtrBpmIns[t_j] = m_vecPtrBpmIns[t_j+1];
			m_iBMNum --;

			return;
		}

		m_vecPtrBpmIns[t_i] = (osc_bpmInstance *)_npm;

		return ;
	}

	// 找不到可替换的Mesh.
	osDebugOut( "The BPM Fname is:<%s>..BM num is:<%d>..\n",
		_pm->get_bpmPtr()->m_strFname,m_iBMNum );
	osassert( false );

	unguard;
}




/** \brief
 *  把每一个bodyPartMesh的matrix Palette和真正的BodyFrameMatrix关联起来。
 *  
 */
void osc_bodyFrame::LinkBones( osc_bodyFrame * _root )
{
	guard;// osc_bodyFrame::LinkBones() );

	osassert( _root );


	osc_bpmInstance*   t_ptrBpmIns;
	osc_bodyPartMesh*  t_ptrBodyMesh;
	osMatrix**         t_ptrOffsetMat;

	

	for( int t_i=0;t_i<this->m_iBMNum;t_i++ )
	{
		t_ptrBpmIns = this->m_vecPtrBpmIns[t_i];
		t_ptrBodyMesh = t_ptrBpmIns->get_bpmPtr();
		osassert( t_ptrBodyMesh );

		t_ptrOffsetMat = t_ptrBpmIns->get_bpmBoneOffset();

		//! init bone 
		for (int i = 0; i < MAX_BONEPERBPM;i++)
		{
			t_ptrOffsetMat[i] =NULL;
		}

		int t_num = t_ptrBodyMesh->m_dwNumPalette;	 
		for(int i = 0; i<t_num; i++)
		{
			osc_bodyFrame* pFrame = _root->FindFrame( t_ptrBodyMesh->m_arrBoneId[i] );
			if(pFrame==NULL) 
				continue;		

			t_ptrOffsetMat[i] = &(pFrame->m_matCombined);
		}
	}

	// 
	// 对子结点和子兄弟结点进行递归处理。
	if(m_pFirstChild)
		m_pFirstChild->LinkBones( _root );

	if(m_pSibling)
		m_pSibling->LinkBones( _root );

	return;
	
	unguard;
}





/** \brief
*  使用矩阵的层级关系去更新当前的Frame.
*  
*/
void osc_bodyFrame::UpdateFrames(osMatrix & _matCur,bool _catchBoneMat/* = false*/ )
{

	m_matCombined = _matCur;	

	osMatrixMultiply(&m_matCombined, &m_matRot, &_matCur);	

	// River added @ 2009-5-31:
	if( _catchBoneMat )
		m_matBonePos = m_matCombined;

	// 对子结点进行更新.
	if(m_pFirstChild != NULL)
	{
		m_pFirstChild->UpdateFrames( m_matCombined,_catchBoneMat );
	}

	// 对子结点进行更新
	if(m_pSibling != NULL)
	{
		m_pSibling->UpdateFrames( _matCur,_catchBoneMat );
	}

	return;
}





/** 
 *
 */
osc_skinMesh::osc_skinMesh()
{
	reset_skinMesh();
}

//! 隐藏当前人物身上绑定的特效
void osc_skinMesh::hide_followEffect( bool _hide )
{
	guard;

	if( _hide )
	{
		if( mEffectId != -1 )
		{
			get_sceneMgr()->delete_effect( mEffectId );
			mEffectId = -1;
		}
	}
	else
	{
		if( (mEffectId == -1)&&(mszSpeFileName.length()>0) )
		{
			mEffectId = get_sceneMgr()->
				create_effect( mszSpeFileName.c_str(),osVec3D(0.0f,0.0f,0.0f),m_fRotAgl );
		}
	}

	unguard;
}



/** \brief
*  初始化当前的skin Mesh.
*/
void osc_skinMesh::reset_skinMesh( void )
{
	m_vScale = osVec3D( 1.0f,1.0f,1.0f );
	m_pRoot = NULL;
	for( int t_i=0;t_i<MAX_FRAMENUM;t_i++ )
		m_pframeToAnimate[t_i] = NULL;
	//@{
	// Windy mod @ 2005-8-29 12:29:12
	//Desc: ride
	m_HumObj = NULL;
	m_RideObj = NULL;
	//@}
	m_ptrBoneSys = NULL;
	m_ptrBoneAni = NULL;

	m_iNumParts = 0;

	m_bInUse = false;


	m_fLastUsedTime = 0;
	m_wAniIndex = 0;
	m_iAniId = 0;

	//@{
	// river added @ 2004-4-22: 人物旋转时，插值。
	m_fLastRotAgl = 0.0f;
	m_fInterpolationTime = 0.0f;
	m_bRotInterpolateStatus = false;
	//@}


	m_dwFPS = FPS_PERSECOND;
	m_fTPF = 1.0f / m_dwFPS;
	m_fNewActSpeed = -1.0f;

	m_bInView = false;

	m_bHasAlphaPart = false;


	// 当前使用的bodyframe数目为0.
	m_iBFNum = 0;


	//　设定这个值，使用可一直播放初始化的动作。
	m_actChange.m_changeType = OSE_DIS_OLDACT;
	m_bFreezeAct = false;
	m_bChangeActState = false;


	m_bWriteZbuf = true;

# if __GLOSSY_EDITOR__
	// 刀光编辑器模式下，默认播放	
	m_bPlaySGlossy = true;
	m_bPlayWeaponEffect = true;
# else
	// 默认不播放刀光.
	m_bPlaySGlossy = false;
	m_bPlayWeaponEffect = false;
	m_dwFinalGlossyColor = 0;
# endif 

	m_ptrSGBpmIns = NULL;
	m_ptrSGBpmIns8 = NULL; 

# if __GLOSSY_EDITOR__
	m_ptrWeaponBpm = NULL;
	m_ptrSGIns = NULL;
	m_ptrWeaponBpm8 = NULL;
	m_ptrSGIns8 = NULL;



# endif 

	m_bSuspendState = false;
	m_bFadeState = false;
	m_fAlphaVal = 1.0f;

	// 初始化为不隐藏人物的实时阴影.
	m_bHideRealShadow = false;


	//
	// 处理人物的Bounding Box.
	m_sBBox.reset_bbox();

	m_iAniNameIdx = -1;

	m_bLerpLastFAndFirstF = true;

	m_bHideSm = false;

	m_bActStopStatus = false;

	m_bShadowSupport = false;
	
	// syq
	memset( &m_Look,0,sizeof(os_bodyLook));

	m_bRenderGlossy = false;

	m_bUseDirLAsAmbient = false;
	//@{
	// Windy mod @ 2005-9-21 11:08:09
	//Desc: 默认当前模型不是坐骑.
	m_bIsRide = false;
	m_vec3RideOffset = osVec3D(0.0f,0.0f,0.0f);
	m_vec3RidePos =  osVec3D(0.0f,0.0f,0.0f);
	//@}

	// River @ 2007-7-7:高亮的回复
	m_bSetHighLight = false;

	m_bInViewLastFrame = true;
	mEffectId = -1;
	mszSpeFileName.clear();

# if GALAXY_VERSION
	m_bProjectShadow = FALSE;
# endif

	m_dwLastRenderTime = 0;

	m_bLoadLock = true;

	m_bFakeGlowDraw = false;

	m_bPostTrailState = false;

	m_bExcludeLightState = false;

	m_fBaseSkinScale = 1.0f;

	m_bDisWeaponActionOver = true;


	//! River @ 2011-2-25:当前人物是否处于删除状态，用于多线程不出错.
	m_bToBeRelease = false;
	//! River @ 2011-2-25:当前人物是否在调入中处于标志优先状态。
	m_iVipMTLoadLevel = 0;

	m_bMustUpdateNextFrame = false;

	m_iMT_PlayWeaponEffect = 0;

}


osc_skinMesh::~osc_skinMesh()
{

}

extern LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration3;

CPostTrailEff osc_skinMesh::sm_postTrailEff;

//! former render target(non-postTrail surface)
LPDIRECT3DSURFACE9 osc_skinMesh::sm_formerSurBeforePostTrail = NULL;

//! is former render target?
BOOL osc_skinMesh::sm_bIsFormerSurBeforePostTrail = TRUE;


//! enable post trail state
void osc_skinMesh::enable_postTrail(bool _enabled /*= true*/)
{
	m_bPostTrailState = _enabled;
}
//! exclude light effect
void osc_skinMesh::exclude_light(bool _exclude /*= true*/)
{
	m_bExcludeLightState = _exclude;
}

//! is exclude light effect 
bool osc_skinMesh::is_excludeLight(void)
{
	return m_bExcludeLightState;
}
void osc_skinMesh::init_postTrail(void)
{
	guard;

	// 现在取消掉人物的 残影效果，可能会消耗低端显卡的显存。
	//
	// 
	// 	I_deviceManager* t_device  = get_deviceManagerPtr();
	// 
	// 	osassert(t_device);
	// 	const D3DSURFACE_DESC* t_desc = t_device->get_backBufDesc();
	// 
	// 	sm_postTrailEff.InitPostTrailScene(m_pd3dDevice,t_desc->Width,t_desc->Height,t_desc->Format,g_bUseFSAA);

	unguard;
}

void osc_skinMesh::restore_postTrail(void)
{
	guard;

	//sm_postTrailEff.CreateRes(m_pd3dDevice);

	unguard;
}
void osc_skinMesh::release_postTrail(void)
{
	guard;

// 	sm_postTrailEff.DestroyEffRes();
// 
// 	SAFE_RELEASE(sm_formerSurBeforePostTrail);

	unguard;
}
void osc_skinMesh::clear_postTrail(void)
{
	guard;

// 	LPDIRECT3DSURFACE9 t_surface = NULL;
// 
// 	HRESULT t_hr = m_pd3dDevice->GetRenderTarget(0,&t_surface);
// 	osassert(!FAILED(t_hr));
// 
// 	sm_postTrailEff.ClearCurrFrameTex(m_pd3dDevice);
// 
// 	t_hr = m_pd3dDevice->SetRenderTarget(0,t_surface);
// 	SAFE_RELEASE(t_surface);

	unguard;
}

//!渲染人物的Post trail 
void osc_skinMesh::start_renderPostTrail(osc_skinMesh* _ptrSkinMesh)
{
	guard;

// 	osassert(_ptrSkinMesh);
// 
// 
// 	if(_ptrSkinMesh->is_postTrailEnabled()){
// 
// 		sm_bIsFormerSurBeforePostTrail = FALSE;
// 
// 		if(sm_formerSurBeforePostTrail == NULL){
// 			const HRESULT t_hr = m_pd3dDevice->GetRenderTarget(0,&sm_formerSurBeforePostTrail);
// 			osassert(!FAILED(t_hr));
// 		}	
// 
// 		sm_postTrailEff.SetCurrFrameTarget(m_pd3dDevice);
// 
// 		sm_postTrailEff.SetEffective(m_pd3dDevice,TRUE);
// 
// 	}else{
// 
// 		// set back the render target...
// 		//
// 		if(sm_bIsFormerSurBeforePostTrail == FALSE){
// 
// 			sm_bIsFormerSurBeforePostTrail = TRUE;
// 		
// 			osassert(sm_formerSurBeforePostTrail);
// 			const HRESULT t_hr = m_pd3dDevice->SetRenderTarget(0,sm_formerSurBeforePostTrail);
// 			osassert(!FAILED(t_hr));
// 			
// 		}
// 		
// 	}

	unguard;
}
void osc_skinMesh::end_renderPostTrail(bool _alpha)
{
	guard;

	
// 	if(sm_formerSurBeforePostTrail == NULL){
// 		const HRESULT t_hr = m_pd3dDevice->GetRenderTarget(0,&sm_formerSurBeforePostTrail);
// 		osassert(!FAILED(t_hr));
// 	}
// 
// 	// force the alpha blend... to render the post trail...
// 	//
// 	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
// 	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
// 
// 	sm_postTrailEff.SetMainRenderSurface(sm_formerSurBeforePostTrail);
// 	sm_postTrailEff.RenderEffScene(m_pd3dDevice,_alpha);
// 
// 	if(sm_bIsFormerSurBeforePostTrail == FALSE){
// 
// 		sm_bIsFormerSurBeforePostTrail = TRUE;
// 
// 		osassert(sm_formerSurBeforePostTrail);
// 		const HRESULT t_hr = m_pd3dDevice->SetRenderTarget(0,sm_formerSurBeforePostTrail);
// 		osassert(!FAILED(t_hr));
// 
// 	}

	unguard;
}

//! 用于检测两个动作是否可以插值,如果flipDeternminant不一致，则不能切换。
bool osc_skinMesh::can_lerpInDiffAction( int _actIdx1,int _actIdx2 )
{
	guard;

	osassert( (_actIdx1>=0)&&(_actIdx2>=0) );
	osassert( (_actIdx1<int(m_ptrBoneAni->m_vecBATable.size()))&&
		(_actIdx2<int(m_ptrBoneAni->m_vecBATable.size())) );

	os_boneAction    *t_ptrBA1,*t_ptrBA2;

	t_ptrBA1 = &m_ptrBoneAni->m_vecBATable[_actIdx1];
	t_ptrBA2 = &m_ptrBoneAni->m_vecBATable[_actIdx2];

	for( int t_i=0;t_i<t_ptrBA1->m_iBoneNum;t_i ++ )
	{
		if( t_ptrBA1->m_vecMatrix[t_i].m_bFlipDeterminant !=
			t_ptrBA2->m_vecMatrix[t_i].m_bFlipDeterminant )
		{
			return false;
		}
	}

	return true;

	unguard;
}




/** \brief
*  改变当前skinMeshObject的动作.
*
*  对动作进行变换时，需要有一个动作变换的时间，并且需要使用这个时间，
*  对当前动作和要变换的动作进行插值动画.
*
*	\param _bSetFrameTime
*/
float osc_skinMesh::change_skinMeshAction( os_newAction&  _act/* ,bool _bSetFrameTime = false */)
{
	guard;

	// 
	// 如果时间过短，则使用一个快速的动作
	if (_act.m_fFrameTime<0.01f)
	{
		_act.m_fFrameTime = 0.01f;
	}

	if( (_act.m_changeType == OSE_DIS_NEWACT_TIMES)||
		(_act.m_changeType == OSE_DIS_NEWACT_LASTFRAME) )
		osassert( _act.m_iPlayNum > 0 );

	// 
	// 如果上一次和当前次都是OSE_DIS_NEWACT_TIMES的播放方式,
	// 则使用上一次的动作做为播放完成播放的动作。
	// 如果参数中不要求必须使用原来连续播放的动作，也需要使用新的动作播放方式。
	if( (_act.m_changeType != OSE_DIS_NEWACT_TIMES)||
		(m_actChange.m_changeType != OSE_DIS_NEWACT_TIMES)||
		(!_act.m_bPlayLoopAct) )
	{
		m_iLastActId = m_iAniId;
	}

	m_actChange = _act;
	m_bFreezeAct = false;


	// River @ 2010-7-1:先确认武器没有隐藏.
	hide_weapon( false );
	m_bDisWeaponActionOver = true;

	// 
	// 如果使用动作名字播放动作，则需要从动作名字得到动作的索引号。
	if( _act.m_iNewAct >= 0 )
	{
		m_iAniId = _act.m_iNewAct;
		m_iAniNameIdx = -1;
		osassert( m_iAniId < m_ptrBoneAni->m_iBANum );


		// 使用id播放动作时，设置为不使用刀光。
		if( m_ptrSGBpmIns )
			m_dwFinalGlossyColor = m_ptrSGBpmIns->set_disSGlossy( -1 );

	}
	else
	{
		m_iAniId = m_ptrBoneAni->
			get_actIdxFromName( _act.m_strActName,m_iAniNameIdx );

		//! river @ 2010-7-1:是否隐藏武器.
		if( m_ptrBoneAni->is_actNameHideWeapon( _act.m_strActName ) )
		{
			hide_weapon( true );
			if( _act.m_changeType == OSE_DIS_OLDACT )
				m_bDisWeaponActionOver = false;
		}
		else
		{
			hide_weapon( false );
			m_bDisWeaponActionOver = true;
		}


		//osassertex( (m_iAniId >= 0),
		//	va( "动作Id必须大于等于零,动作名字<%s><%d>....\n",_act.m_strActName,m_iAniNameIdx ) );
		//osassertex( (m_iAniId < m_ptrBoneAni->m_iBANum),"动作Id必须小于最大的动作数目...\n" );
		if (m_iAniId<0 || m_iAniId >= m_ptrBoneAni->m_iBANum)
		{
            osDebugOut(va( "动作Id必须大于等于零,动作名字<%s><%d>....\n",_act.m_strActName,m_iAniNameIdx ));
			m_iAniId = 0;
			return 0;
		}
	}


	// 
	// 两个动作变换时的插值动画。
	m_bChangeActState = can_lerpInDiffAction( m_iAniId,m_iLastActId );
	if( m_bChangeActState )
		m_fActLerpVal = m_fLastUsedTime/m_fTPF;
	else
		m_fActLerpVal = 0.0f;
	m_fLastUsedTime = 0;

	// 
	// 设置播放速度信息。
	set_actSpeedInfo( _act.m_fFrameTime );


	// River @ 2005-8-15:保存当前动作的帧数，用于跟新的动作插值,设置动作播放为头一帧动作.
	m_wLastAniIndex = m_wAniIndex;
	m_wAniIndex = 0;

	if( m_actChange.m_bStopAtStopFrame )
		m_bActStopStatus = true;
	else
		m_bActStopStatus = false;
	m_bArriveStopFrame = false;


	return  m_ptrBoneAni->m_vecBATable[m_iAniId].m_fTimeToPlay;

	unguard;
}


//! 由另外的线程来调用changeEquipment的数据
bool osc_skinMesh::change_equipmentThread( const os_equipment& _equi )
{
	guard;

	int       t_idx;

	for( int t_i=0;t_i<_equi.m_iCPartNum;t_i++ )
	{
		t_idx = _equi.m_arrId[t_i];

		//
		// 如果要替换的meshId和原来的Mesh不一样，替换新的mesh.
		if( _equi.m_arrMesh[t_i] != m_Look.m_mesh[t_idx] )
		{
			if( !change_bodyMesh( t_idx,_equi.m_arrMesh[t_i] ) )
				return false;

			//
			// 如果替换了mesh,则纹理肯定需要更换。
			if( !change_bodySkin( t_idx,_equi.m_arrMesh[t_i],_equi.m_arrSkin[t_i] ) )
				return false;

			m_Look.m_mesh[t_idx] = _equi.m_arrMesh[t_i];
			m_Look.m_skin[t_idx] = _equi.m_arrSkin[t_i];

			// 
			// 进入下一个部位的替换。
			continue;
		}

		//
		// 如果要替换的skin和原来的不一样，替换新的skin.
		if( _equi.m_arrSkin[t_i] != m_Look.m_skin[t_idx] )
		{
			if( !change_bodySkin( t_idx,_equi.m_arrMesh[t_i],_equi.m_arrSkin[t_i] ) )
				return false;
			m_Look.m_skin[t_idx] = _equi.m_arrSkin[t_i];
		}

	}


	return true;

	unguard;
}


/** \brief
*  替换skinMesh的装备。
*
*  算法描述：
*  对每一个要替换的身体部分，如果目前不为空，减少当前身体部位资源的引用，
*　然后创建新的资源。
*  
*/
bool osc_skinMesh::change_equipment( const os_equipment& _equi )
{
	guard;

	osc_skinMeshMgr*   t_ptrMgr = osc_skinMeshMgr::Instance();

	os_charLoadData   t_s;
	t_s.m_ptrSkinPtr = this;
	t_s.m_bChangeEquipent = true;
	memcpy( &t_s.m_sEquipent,&_equi,sizeof( os_equipment ) );


	t_ptrMgr->m_sCharLoadThread.push_loadChar( &t_s,true );

	return true;

	unguard;
}

//! Mgr内管理skinMesh的内存管理器指针，为了使调用release_character
//! 接口的时候，可以对管理器做出处理
CSpaceMgr<osc_skinMesh>* osc_skinMesh::m_ptrVecDadded = NULL;
CSpaceMgr<osc_skinMesh>* osc_skinMesh::m_ptrSceneSM = NULL;


//! 由另外的线程来释放人物.
bool osc_skinMesh::release_characterThread( BOOL _sceneSm )
{
	guard;

	// 
	// 释放frame资源。
	if( m_ptrBoneSys )
	{
		osassertex( m_ptrBoneSys->m_iBoneNum <MAX_FRAMENUM,m_strSMDir );
		for( int t_i=0;t_i<m_ptrBoneSys->m_iBoneNum;t_i++ )
			m_pframeToAnimate[t_i]->Reset_Frame();
	}

	// 
	// 释放bodyPartMesh资源
	for( int t_i=0;t_i<m_iNumParts;t_i++ )
		m_vecBpmIns[t_i].release_bpmIns();

	reset_skinMesh();


	// 应该由外部调用，此处调用是为跟以前的代码兼容
	CSpaceMgr<osc_skinMesh>::NODE  t_ptrNode;
	CSpaceMgr<osc_skinMesh>*       t_ptrArray;
	if( _sceneSm )
		t_ptrArray = m_ptrSceneSM;
	else
		t_ptrArray = m_ptrVecDadded;
	for( t_ptrNode = t_ptrArray->begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = t_ptrArray->next_validNode(&t_ptrNode) )
	{
		if( t_ptrNode.p == this )
		{
			// River @ 2009-6-6:释放也加cs.
			::EnterCriticalSection( &osc_skinMeshMgr::m_sChrCreateCS );
			t_ptrArray->release_node( t_ptrNode.idx );
			::LeaveCriticalSection( &osc_skinMeshMgr::m_sChrCreateCS );
			
			break;
		}
	}

	//osDebugOut( "REAL RELEASE CHARACTER<%d>...\n",int( this ) );

	return true;

	unguard;
}


/** \brief
*  释放当前Character所占用的资源。
*
*  网络游戏过程中可能需要调入和释放大量的资源，所以这个函数必须实现。
*  
*  ATTENTION: 
*/
void osc_skinMesh::release_character( BOOL _sceneC/* = false*/ )
{
	guard;
	
	//  River @ 2011-2-12:人物有没有调入的情况下，也可以删除。
	//                    每一帧对调入队列进行处理，如果创建之前就有删除此skin的操作，
	//                    则把创建和删除一起去掉。
	// 
	//! 确保当前此skinMesh已经调入
	//osassertex( !m_bLoadLock,
	//	"当前的character指针正处于调入或删除状态，逻辑层非常调用或别的原因?" );

	//! 释放怪物后，设置loadLock为真
	m_bLoadLock = true;

	// 与调入线程同步此变量。
	::EnterCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );
	m_bToBeRelease = true;
	::LeaveCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

	//@{
	// River added @ 2010-9-29：确认.
	int t_i = 0;
	os_ridePtr*   t_ptrRide = osc_skinMeshMgr::Instance()->m_vecRidePtrArray;
	for( t_i =0 ;t_i<MAX_RIDENUM;t_i ++ )
	{
		if( (t_ptrRide[t_i].m_ptrHuman == this)||
			(t_ptrRide[t_i].m_ptrRide == this) )
		{
			t_ptrRide[t_i].m_ptrHuman = NULL;
			t_ptrRide[t_i].m_ptrRide = NULL;
			break;
		}
	}
	//@}

	// 先释放特效
	if (mEffectId!=-1)
	{
		get_sceneMgr()->delete_effect(mEffectId);
		mEffectId = -1;
		mszSpeFileName.clear();
	}

	os_charLoadData   t_ptrChar;

	t_ptrChar.m_ptrSkinPtr = this;
	t_ptrChar.m_bReleaseChar = true;
	t_ptrChar.m_bSceneSm = _sceneC;

	osc_skinMeshMgr*   t_ptrMgr = osc_skinMeshMgr::Instance();

	//osDebugOut( "PUSH RELEASE CHARACTER<%d>...\n",int(t_ptrChar.m_ptrSkinPtr) );

	t_ptrMgr->m_sCharLoadThread.push_loadChar( &t_ptrChar );


	unguard;
}

//! 得到一个可以使用的bodyFrame空间。
osc_bodyFrame* osc_skinMesh::get_freeBF( void )
{
	osassert( m_iBFNum < (MAX_FRAMENUM-1) );
	osc_bodyFrame* t_ptr = &m_arrBodyFrame[m_iBFNum];
	m_iBFNum ++;
	return t_ptr;
}


//!  算出此时动作点在当前整个动作的播放百分比
float osc_skinMesh::cal_posePercent( void )
{
	guard;

	// 动作变化期间返回零
	if( m_bChangeActState )
		return 0.0f;

	os_boneAction*  t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	int   t_iNextActIdx;

	if( m_wAniIndex == (t_ptrBA->m_iNumFrame-1) )
			t_iNextActIdx = 0;
		else
			t_iNextActIdx = m_wAniIndex+1;

	float     t_fPosePercent = float(m_wAniIndex) / t_ptrBA->m_iNumFrame;
	t_fPosePercent +=  ( (m_fLastUsedTime/m_fTPF)/t_ptrBA->m_iNumFrame );

	return t_fPosePercent;

	unguard;
}


//! 得到某个动作事件点的百分比，比如受创特效需要在受创动作的播放多少时播放。
float osc_skinMesh::get_actionTriggerPoint( 
	             char _name[32],int _idx/* = -1*/ )
{
	guard;

	float   t_fPercent;
	int     t_idx;

	if( _idx != -1 )
		t_idx = _idx;
	else
	{
		int   t_iNameIdx;

		t_idx = m_ptrBoneAni->get_actIdxFromName( _name,t_iNameIdx );
		osassertex( (t_idx != -1),va( "找不到动作<%s>...\n",_name ) );
		t_idx = m_ptrBoneAni->m_vecActName[t_iNameIdx].m_vecActIdx;
	}

	os_boneAction*  t_ptrBA = &m_ptrBoneAni->m_vecBATable[t_idx];

	osassertex( t_ptrBA->m_iStopFrameIdx <= t_ptrBA->m_iNumFrame,
		va( "动作停止帧的数据不对<%d,%d>..\n",t_ptrBA->m_iStopFrameIdx,t_ptrBA->m_iNumFrame ) );

	t_fPercent = float( t_ptrBA->m_iStopFrameIdx ) / float(t_ptrBA->m_iNumFrame );

	return t_fPercent;

	unguard;
}


//! 得到当前正在播放的动作名字和索引
int osc_skinMesh::get_curPlayActIdx( char _name[32] )const
{
	guard;

	_name[0] = NULL;

	for( DWORD t_i=0;t_i<m_ptrBoneAni->m_vecActName.size();t_i ++ )
	{
		if( m_iAniId == m_ptrBoneAni->m_vecActName[t_i].m_vecActIdx )
		{
			strcpy( _name,m_ptrBoneAni->m_vecActName[t_i].m_szActName );
			break;
		}
	}

	return m_iAniId;

	unguard;
}

//! 测试得到某一根骨骼的位置.
void osc_skinMesh::get_bonePos( const char* _boneName,osVec3D& _pos )
{
	guard;

	const char* boneName = StringTable->insert(_boneName,true);
	//m_ptrBoneSys
	osVec3D       t_vec3Pos( 0.0f,0.0f,0.0f );
	const char*   t_ptrBoneName = NULL;

	for( int t_i=0;t_i<MAX_FRAMENUM;t_i ++ )
	{
		if( m_pframeToAnimate[t_i] == NULL )
			break;
		t_ptrBoneName = 
			m_ptrBoneSys->get_boneNameFromId( m_pframeToAnimate[t_i]->m_dwID );
		if( t_ptrBoneName == boneName )
		{
			osVec3Transform( &_pos,&_pos,&m_pframeToAnimate[t_i]->m_matBonePos );
			return;
		}
	}

	return ;

	unguard;
}



//! 用于坐骑上人物位置的更改
float osc_skinMesh::frame_move( osVec3D& _npos,
								   float _nangle,bool _rideSta )
{
	guard;

	// River @ 2011-3-3：在选人场景，可能会直接frameMove.
	if( !legal_operatePtr() )
		return 0.0f;

	float t_fPosePercent = cal_posePercent();

	// 
	// 如果当前人物的动作处于停止状态，则不能移动和旋转。
	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	if( m_bActStopStatus &&(m_wAniIndex >= t_ptrBA->m_iStopFrameIdx) )
		return t_fPosePercent;

	m_vec3SMPos = _npos;

	//
	// 变人物的旋转值为0-2PI之间.
	if( _nangle < 0 )
		_nangle += (2.0*OS_PI); 

	//
	// 更新当前人物相对应的Bounding Box的数传值。
	m_sBBox.set_bbYRot( _nangle );
	osVec3D   t_vec3Offset;
	m_sAdjBBox.get_smOffsetAdjBBox( t_vec3Offset,_nangle );
	m_sBBox.set_bbPos( m_vec3SMPos + t_vec3Offset );


	// 如果新的旋转方向和原来的方向相同，则不进行处理。
	if( float_equal( _nangle,m_fRotAgl ) )
		return t_fPosePercent;

	//
	// 如果当前不是处于旋转插值状态。
	if( !m_bRotInterpolateStatus )
	{
		m_bRotInterpolateStatus = true;
		m_fLastRotAgl = m_fRotAgl;
		m_fInterpolationTime = 0.0f;
	}
	else
	{
		float  t_f;
		t_f = m_fInterpolationTime/ROTINTERPOLATE_TIME;
		m_fLastRotAgl = t_f*(m_fRotAgl-m_fLastRotAgl)+m_fLastRotAgl;
		m_fInterpolationTime = 0.0f;
	}

	m_fRotAgl = _nangle;
	//
	// 如果两个旋转角度之间的差值的绝对值大于OS_PI,则把大于OS_PI的值
	// 变为0到负OS_PI之间的值，因为我们只有８个方向，所以必须大于OS_PI+0.1
	if( fabs( m_fRotAgl-m_fLastRotAgl )>= ( OS_PI+0.1) )
	{
		if( m_fRotAgl>=OS_PI )
			m_fRotAgl -= (2.0f*OS_PI);
		else
		{
			if( m_fLastRotAgl>0 )
				m_fLastRotAgl -= (2.0f*OS_PI);
			else
				m_fLastRotAgl += (2.0f*OS_PI );
		}
	}


	return t_fPosePercent;
	
	unguard;
}

/** \brief
*  更新当前的skinmeshObj的位置。
*
*  此函数仅用于修改skinMesh物体的位置和旋转角度.
*  引擎内部的skinObjFramemove函数对skinMesh做出处理。
*  
*  
*/
float osc_skinMesh::frame_move( osVec3D& _npos,float _nangle )
{
	guard;

	// River @ 2011-3-3：在选人场景，可能会直接frameMove.
	if( !legal_operatePtr() )
		return 0.0f;

	float t_fPosePercent = cal_posePercent();

	//! 如果当前的人物骑乘的坐骑不为空，则不能执行这个函数
	if( m_RideObj != NULL )
	{
		osDebugOut( "The error frame move...\n" );
		return 0.0f;
	}

	// 
	// 如果当前人物的动作处于停止状态，则不能移动和旋转。
	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	if( m_bActStopStatus &&(m_wAniIndex >= t_ptrBA->m_iStopFrameIdx) )
		return t_fPosePercent;

	m_vec3SMPos = _npos;

	//
	// 变人物的旋转值为0-2PI之间.
	if( _nangle < 0 )
		_nangle += (2.0*OS_PI); 

	//
	// 更新当前人物相对应的Bounding Box的数传值。
	m_sBBox.set_bbYRot( _nangle );
	osVec3D   t_vec3Offset;
	m_sAdjBBox.get_smOffsetAdjBBox( t_vec3Offset,_nangle );
	m_sBBox.set_bbPos( m_vec3SMPos + t_vec3Offset );


	// 如果新的旋转方向和原来的方向相同，则不进行处理。
	if( float_equal( _nangle,m_fRotAgl ) )
		return t_fPosePercent;

	//
	// 如果当前不是处于旋转插值状态。
	if( (!m_bRotInterpolateStatus) )
	{
		m_bRotInterpolateStatus = true;
		m_fLastRotAgl = m_fRotAgl;
		m_fInterpolationTime = 0.0f;
	}
	else
	{
		float  t_f;
		t_f = m_fInterpolationTime/ROTINTERPOLATE_TIME;
		m_fLastRotAgl = t_f*(m_fRotAgl-m_fLastRotAgl)+m_fLastRotAgl;
		m_fInterpolationTime = 0.0f;
	}

	m_fRotAgl = _nangle;
	//
	// 如果两个旋转角度之间的差值的绝对值大于OS_PI,则把大于OS_PI的值
	// 变为0到负OS_PI之间的值，因为我们只有８个方向，所以必须大于OS_PI+0.1
	if( fabs( m_fRotAgl-m_fLastRotAgl )>= ( OS_PI+0.1) )
	{
		if( m_fRotAgl>=OS_PI )
			m_fRotAgl -= (2.0f*OS_PI);
		else
		{
			if( m_fLastRotAgl>0 )
				m_fLastRotAgl -= (2.0f*OS_PI);
			else
				m_fLastRotAgl += (2.0f*OS_PI );
		}
	}


	return t_fPosePercent;
	
	unguard;
}



//! 从动作名字中，得到我们要播放哪一个动作，可能传入多个动作名随机播放
int osc_skinMesh::get_aniNameIdx( void )
{
	guard;


	// 没有随机的动作
	if( m_actChange.m_strRandActName[0][0] == NULL )
		return m_iAniNameIdx;

	int   t_iRandActNum = 1;

	for( int t_i=0;t_i<3;t_i ++ )
	{
		if( m_actChange.m_strRandActName[t_i][0] == NULL )
			break;
		else
			t_iRandActNum ++;
	}

	int t_idx = ::getrand_fromintscope( 0,t_iRandActNum-1 );

	if( t_idx == 0 )
	{
		m_ptrBoneAni->
			get_actIdxFromName( m_actChange.m_strActName,m_iAniNameIdx );
	}
	else
	{
		t_idx -= 1;
		m_ptrBoneAni->
			get_actIdxFromName( m_actChange.m_strRandActName[t_idx],m_iAniNameIdx );
	}

	return m_iAniNameIdx;

	unguard;
}



/** \brief
*  每播放完一套动作时，根据当前的actChange结构对动作进行处理。
*/
os_boneAction* osc_skinMesh::process_ActionOver( void )
{
	guard;

	os_boneAction*  t_ptrBA;
	osassert( m_ptrBoneAni );
	osassert( m_iAniId >= 0 && (m_iAniId < int(m_ptrBoneAni->m_vecBATable.size()) ) );
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	//! river @ 2010-7-1:是否隐藏武器
	if( m_bDisWeaponActionOver )
		hide_weapon( false );


	// 得到最新的BoneAction table.
	if( m_iAniNameIdx == -1 )
	{
		t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	}
	else
	{
		//
		// 如果新的动作是永久性的替换原来的动作,
		// 则随机的选择当前名字的动作. 
		if( m_actChange.m_changeType == OSE_DIS_OLDACT )
		{
			// 
			// River added @ 2006-6-7:传入多个动作名字，可以随机播放动作
			m_iAniNameIdx = get_aniNameIdx();
			if( m_iAniNameIdx == -1 )
				m_iAniNameIdx = 0;

			// 
			// 同一名字的动作，但动作不一样,在多个动作间进行切换。
			int t_idx = m_ptrBoneAni->
				get_actBoneAction( m_iAniNameIdx );

			osassert( t_idx>=0 && (t_idx<int(m_ptrBoneAni->m_vecBATable.size())) );

			// 
			// 如果动作ID改变，则开始变换动作的处理,否则继续处理。
			if( m_iAniId != t_idx )
			{
				// River fixed @ 2004-10-20:有可能此值会大到破坏内存.
				// 可能当前的动作索引大于新动作的帧数.
				m_wAniIndex %= t_ptrBA->m_iNumFrame;

				m_iLastActId = m_iAniId;
				m_bChangeActState = true;
				m_fActLerpVal = m_fLastUsedTime/m_fTPF;
				m_fLastUsedTime = 0;
				m_iAniId = t_idx;

				// 设置播放速度信息。
				set_actSpeedInfo( m_fNewActSpeed );

				// 
				// River @ 2006-6-7:移动下句代码到此处，不然
				// t_ptrBA = &m_ptrBoneAni->m_vecBATable[t_idx];
				// 操作后，动作切换可能会出现错误 .  并且设置切换后的动作为零帧
				t_ptrBA = &m_ptrBoneAni->m_vecBATable[t_idx];
				m_wLastAniIndex = m_wAniIndex;
				m_wAniIndex = 0;

			
				return  t_ptrBA;
			}
		} 
	}

	if( m_actChange.m_changeType == OSE_DIS_OLDACT )
	{
		//
		//@{
		// River mod @ 2004-2-4:使用减如果间隔时间过长
		// 会出问题，必须使用求余。
		m_wAniIndex %= t_ptrBA->m_iNumFrame;
		//@}
		return t_ptrBA;

	}

	// 按次播放动画。
	if( (m_actChange.m_changeType == OSE_DIS_NEWACT_TIMES) )
	{
		m_actChange.m_iPlayNum --;

		if( m_actChange.m_iPlayNum == 0 )
		{
			// 
			// 如果有新的动作可以一直播放下,使用新的动作.
			if( m_actChange.m_strNextActName[0] )
			{
				os_newAction  t_newAct;

				t_newAct.m_changeType = OSE_DIS_OLDACT;
				t_newAct.m_iNewAct = -1;
				t_newAct.m_fFrameTime = m_fNewActSpeed;
				strcpy( t_newAct.m_strActName,m_actChange.m_strNextActName );

				m_bLerpLastFAndFirstF = false;
				change_skinMeshAction( t_newAct );

				return t_ptrBA;
			}

			//
			// 播放在播放这个动作之前一直播放的动作.
			m_actChange.m_changeType = OSE_DIS_OLDACT;
			m_iAniId = m_iLastActId;
			m_wAniIndex = 0;

			//! River @ 2006-6-10:为了回复到以前的动作，把动作名字索引设为-1
			m_iAniNameIdx = -1;

			//
			// 设置播放速度信息。River mod @ 2006-8-8:不能设置动作时间，因为此处的
			// m_fNewActSpeed的意图是，上一个动作的时间。
			set_actSpeedInfo( m_ptrBoneAni->m_vecBATable[m_iAniId].m_fTimeToPlay );
		}
		else
		{
			m_wAniIndex %= t_ptrBA->m_iNumFrame;
		}

		return t_ptrBA;
	}

	// 
	// 按次播放动画，并播放动画的最后一帧。
	// 然后淡出。
	if( m_actChange.m_changeType == OSE_DIS_NEWACT_LASTFRAME )
	{
		m_actChange.m_iPlayNum --;

		if( m_actChange.m_iPlayNum == 0 )
			m_bFreezeAct = true;
		else
		{
			m_wAniIndex %= t_ptrBA->m_iNumFrame;
		}

		return t_ptrBA;
	}

	return t_ptrBA;
	unguard;
}

/** \brief
*  得到当前skinMesh在世界空间中的位置和旋转角度.
* 
*  \param _pos    返回skinMesh在世界空间的位置.
*  \param _fangle 返回skinMesh在世界空间的旋转角度.
*/
void osc_skinMesh::get_posAndAgl( osVec3D& _pos,float& _fangle )
{
	guard;

	_pos = m_vec3SMPos;

	//! River mod @ 2008-5-12:因为旋转过程中有插值的因素，所以必须处理
	if( !m_bRotInterpolateStatus )
		_fangle = m_fRotAgl;
	else
	{
		float t_fRotAgl;
		float t_fInterTime;

		t_fRotAgl = sg_timer::Instance()->get_lastelatime();

		t_fInterTime = m_fInterpolationTime + t_fRotAgl;
		if( t_fInterTime >= ROTINTERPOLATE_TIME )
		{
			t_fRotAgl = m_fRotAgl;
		}
		else
		{
			// 得到两个旋转角度间的插值。
			t_fRotAgl = t_fInterTime/ROTINTERPOLATE_TIME;
			t_fRotAgl = t_fRotAgl*(m_fRotAgl-m_fLastRotAgl)+m_fLastRotAgl;
		}
		_fangle = t_fRotAgl;
	}
	unguard;
}

//! 用于更新人物跟随特效的位置和方向
void osc_skinMesh::update_followEffect( void )
{
	guard;

	// 根据主角人物的旋转方向，来确定当前跟随特效的实际位置
	osMatrix   t_sRotMat;
	osVec3D    t_vec3Offset;

	// 因为m_fAdjRgl在初始化的时候-OS_PI/2.0f;
	osMatrixRotationY( &t_sRotMat,m_fFinalRotAgl+OS_PI/2.0f );	

	osVec3Transform( &t_vec3Offset,&mEffectOffset,&t_sRotMat );
	get_sceneMgr()->set_effectPos( mEffectId,m_vec3SMPos + t_vec3Offset,m_fFinalRotAgl);

	unguard;
}

// River @ 2011-2-10:加入全局的变量，用于创建人物使人物的出现稍好看一些。
OSENGINE_API BOOL g_bCharRotInterpolateStatus = true;

/** \brief 
*  处理当前skinmesh内每一个frame的更新。
*/
void osc_skinMesh::update_frame( bool _catchBoneMat/* = false*/ )
{
	osMatrix         t_matWorld;
	osMatrix         t_matRot;
	osMatrix         t_matScale;
	float            t_fRotAgl;


	
	osMatrixScaling(&t_matScale,m_vScale.x,m_vScale.y,m_vScale.z );
	osMatrixTranslation( &t_matWorld, m_vec3SMPos.x,m_vec3SMPos.y,m_vec3SMPos.z);

	osMatrixMultiply( &t_matWorld,&t_matScale,&t_matWorld );

	//
	// River @ 2011-2-10:加入全局的变量，用于创建人物使人物的出现稍好看一些。
	// 根据当前是否处于旋转插值状态进行处理。
	if( m_bRotInterpolateStatus && g_bCharRotInterpolateStatus )
	{
		t_fRotAgl = sg_timer::Instance()->get_lastelatime();

		m_fInterpolationTime += t_fRotAgl;
		if( m_fInterpolationTime >= ROTINTERPOLATE_TIME )
		{
			// 改变状态。
			m_bRotInterpolateStatus = false;
			t_fRotAgl = m_fRotAgl;
		}
		else
		{
			// 得到两个旋转角度间的插值。
			t_fRotAgl = m_fInterpolationTime/ROTINTERPOLATE_TIME;
			t_fRotAgl = t_fRotAgl*(m_fRotAgl-m_fLastRotAgl)+m_fLastRotAgl;
		}
	}
	else
		t_fRotAgl = m_fRotAgl;

	// 加入了人物旋转微调.
	m_fFinalRotAgl = t_fRotAgl + m_fAdjRgl;
	osMatrixRotationY( &t_matRot,m_fFinalRotAgl );	

	if(m_vec3AdjPos.x || m_vec3AdjPos.y || m_vec3AdjPos.z){
		osMatrix		 t_adjWorld;
		// Set world matrix.加入了人物位置微调的位置数据。
		osMatrixTranslation(&t_adjWorld,m_vec3AdjPos.x,m_vec3AdjPos.y,m_vec3AdjPos.z);
		osMatrixMultiply(&t_matRot,&t_adjWorld,&t_matRot);
	}	

	osMatrixMultiply( &t_matWorld,&t_matRot,&t_matWorld );
	
	m_pRoot->UpdateFrames( t_matWorld,_catchBoneMat );
	//@{
	// Windy mod @ 2005-8-29 10:20:04
	//Desc: ride 更新坐骑上定位盒的位置
	if (m_bIsRide){
		m_pRoot->UpdateRideBoxPos();
	}
	//@}

	//@{
	// Windy mod @ 2005-8-31 10:08:15
	//Desc: ride 处理每一帧坐骑上人物的位置移动
	if( (m_HumObj!=NULL)&&(!m_HumObj->m_bLoadLock) )
	{
		osVec3D t_3d;
		get_RideBoxPos(t_3d);
		m_HumObj->frame_move(t_3d,m_fRotAgl,true);
		m_HumObj->update_frame( _catchBoneMat );
	}

	//@}

	// 
	//! 更新人物的跟随特效
	if (mEffectId != -1)
		update_followEffect();
}


//! 得到当前m_iNegAniIndex关键帧对应的的动作。
void osc_skinMesh::get_keyFramePose( void )
{
	guard;

	os_boneAction*   t_ptrBA;


	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	//
	// 对每一个Frame进行处理,只需要得到关键帧动作，不需要插值。
	// 
	osassert( t_ptrBA->m_iBoneNum>=0 );
	for( int t_j=0;t_j<t_ptrBA->m_iBoneNum;t_j++ )
	{

		int       t_iMatPtr;

		osassert( m_iNegAniIndex < t_ptrBA->m_iNumFrame );

		t_iMatPtr = t_ptrBA->m_iBoneNum*m_iNegAniIndex;
		t_iMatPtr += t_j;

		t_ptrBA->m_vecMatrix[t_iMatPtr].get_posMatrix( 
			m_pframeToAnimate[t_j]->m_matRot );

	}

	// 更新每一个frame内的矩阵。
	update_frame();

	return ;


	unguard;
}


/** \brief
*  对于"负时间"的skinMesh动作计算。
*
*  用于刀光的倒推，残影的计算等等效果。
*  这个函数不考虑动作的改变，在第零帧的时候，不和结束帧做插值。
*  如果帧到达了开始帧，则下一帧直接从结束帧的位置开始。
*  
*  \param float _ftime 此值必须小于等于零，这样才有此函数的意义。
*/
void osc_skinMesh::negative_frameMoveAction( float _ftime )
{
	guard;


	os_boneAction*   t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	// River:　不使用assert,容错处理吧.
	if( _ftime > 0.0f )
		_ftime = 0.0f;

	m_fNegLastUsedTime = m_fNegLastUsedTime + _ftime;
	if( m_fNegLastUsedTime < 0 )
	{
		//处理直到找到关键帧所在的位置。
		m_iNegAniIndex --;

		while( (m_fNegLastUsedTime += m_fTPF)<0 )
			m_iNegAniIndex --;

		// 开始帧.
		if( m_iNegAniIndex < 0 )
		{
			m_iNegAniIndex = 0;
			m_fNegLastUsedTime = 0.0f;
		}

	}


	//
	// 对每一个Frame进行处理,根据当前插值系数对两帧进行插值。
	// 
	osassert( t_ptrBA->m_iBoneNum>=0 );
	for( int t_j=0;t_j<t_ptrBA->m_iBoneNum;t_j++ )
	{

		int       t_iMatPtr;
		int       t_iNextActIdx;
		os_bactPose*  t_matPtr;
		os_bactPose*  t_nextMatPtr;

		osassert( m_iNegAniIndex < t_ptrBA->m_iNumFrame );

		t_iMatPtr = t_ptrBA->m_iBoneNum*m_iNegAniIndex;
		t_iMatPtr += t_j;

		//
		// 如果是最后一帧，则和第一帧进行插值。
		if( m_iNegAniIndex == (t_ptrBA->m_iNumFrame-1) )
			t_iNextActIdx = m_iNegAniIndex;
		else
			t_iNextActIdx = m_iNegAniIndex+1;
		t_iNextActIdx = t_ptrBA->m_iBoneNum*t_iNextActIdx;
		t_iNextActIdx += t_j;

		t_matPtr = &t_ptrBA->m_vecMatrix[t_iMatPtr];
		t_nextMatPtr = &t_ptrBA->m_vecMatrix[t_iNextActIdx];

		//
		// 修改导出动作的插件,使动作使用quaternion表示。
		lerp_actionPose( m_pframeToAnimate[t_j]->m_matRot,
			t_matPtr,t_nextMatPtr,m_fNegLastUsedTime/m_fTPF );

	}

	// 更新每一个frame内的矩阵。
	update_frame();

	return ;


	unguard;
}



//! 最终计算当前资势的矩阵。这个函数不能用于动作变化状态。
void osc_skinMesh::cal_finalPose( void )
{
	guard;

	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	for( int t_j=0;t_j<t_ptrBA->m_iBoneNum;t_j++ )
	{

		int       t_iMatPtr;
		int       t_iNextActIdx;
		os_bactPose*  t_matPtr;
		os_bactPose*  t_nextMatPtr;

		osassert( m_wAniIndex < t_ptrBA->m_iNumFrame );

		t_iMatPtr = t_ptrBA->m_iBoneNum*m_wAniIndex;
		t_iMatPtr += t_j;

		//
		// 如果是最后一帧，则和第一帧进行插值。
		if( m_wAniIndex == (t_ptrBA->m_iNumFrame-1) )
			t_iNextActIdx = 0;
		else
			t_iNextActIdx = m_wAniIndex+1;
		t_iNextActIdx = t_ptrBA->m_iBoneNum*t_iNextActIdx;
		t_iNextActIdx += t_j;

		t_matPtr = &t_ptrBA->m_vecMatrix[t_iMatPtr];
		t_nextMatPtr = &t_ptrBA->m_vecMatrix[t_iNextActIdx];

		/*
		// TEST COCE:
		osassertex( (t_matPtr->m_bFlipDeterminant == t_nextMatPtr->m_bFlipDeterminant),
			va( "Current sm str is:<%s>..Current Bone Num is:<%d>,Frame num is:<%d>..Crrurnet Pose idx:<%d>,Next pose Idx:<%d>...m_wAniIndex & t_j val :<%d,%d>...\n",
			m_strSMDir,t_ptrBA->m_iBoneNum,
			t_ptrBA->m_iNumFrame,t_iMatPtr,t_iNextActIdx,m_wAniIndex,t_j  ) );\
		*/

		//
		// 修改导出动作的插件,使动作使用quaternion表示。
		// 
		osassertex( m_pframeToAnimate[t_j],
			va( "请确认人物<%s>的骨骼bis文件.骨骼索引<%d>..load标志<%d>..\n",m_strSMDir,t_j,(int)m_bLoadLock ) );
		lerp_actionPose( m_pframeToAnimate[t_j]->m_matRot,
			t_matPtr,t_nextMatPtr,m_fLastUsedTime/m_fTPF );

	}

	// 更新每一个frame内的矩阵。
	update_frame( true );


	unguard;
}


/** \brief
*  对当前的骨骼动作frame move
*  WORK START:此处会出错?
*/
void osc_skinMesh::frame_moveAction( float _ftime )
{
	guard;

	float    t_f;
	int      t_i;
	// 检测是否处于动作停止状态。
	os_boneAction*  t_ptrBA;

	// River @ 2011-3-1:
	m_bMustUpdateNextFrame = false;

	//  River @ 2009-6-7:
	//! 如果正处于调入的lock状态，直接返回
	if( m_bLoadLock )
		return;

	osassertex( m_ptrBoneAni,va( "出错人物指针<0x%x>...\n",DWORD(this) ) ); 

	osassert( m_iAniId >= 0 );
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	if( m_wAniIndex >= t_ptrBA->m_iStopFrameIdx )
	{
		m_bArriveStopFrame = true;
		if( m_bActStopStatus )
			_ftime = 0.0f;
	}

	// 如果处于动作变换状态，处理。
	if( m_bChangeActState )
	{
		frame_moveActChange( _ftime );

		// ATTENTION TO FIX: 整理流程：
		// m_iAniId的值可能改变，所以使用新的t_ptrBA.
		t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	}

	//
	// 如果当前是播放冻结最后一帧的动画或是在动作切换状态，则进行特残处理
	if( (!m_bFreezeAct)&&(!m_bChangeActState) )
	{

		// 如果是处于动作停止状态，则使时间不流逝
		if( m_bSuspendState )
			_ftime = 0.0f;

		t_f = m_fLastUsedTime + _ftime;
		t_i = int(t_f/m_fTPF);
		m_wAniIndex += t_i;

		if( t_i < 1 )
			m_fLastUsedTime += _ftime;
		else
			m_fLastUsedTime = (t_f-t_i*m_fTPF);


		// 如果当前动画帧数超过当前动作的动画帧数,处理。
		if( m_wAniIndex >= (t_ptrBA->m_iNumFrame-1) )
		{
			os_boneAction* pNew = process_ActionOver();

			// 当动作切换时，更新旧的指针 syq
			if( pNew != t_ptrBA )
				t_ptrBA = pNew;

			// 如果当前是冻结最后一帧的动作，使用t_i大于0,
			// 以最后一次更新动作。
			if( m_bFreezeAct )
			{
				m_wAniIndex = t_ptrBA->m_iNumFrame-1;

				// 如果是冻结并播放最后一帧动作,则不需要和第零帧进行插值.
				m_fLastUsedTime = 0;
			}
		}

		// 
		// River added @ 2006-6-7: 加入可以使用多个动作名字随机播放后，
		// 使用此段代码，防止随机切换动作时，动作抖动
		if( m_bChangeActState )
		{
			frame_moveActChange( _ftime );

			update_frame( true );

			// 正常动作更新才重设
			if( _ftime > 0.0f ) m_bRenderGlossy = false;

			return;
		}
	}
	else
	{
		update_frame( true );
		
		// 正常动作更新才重设
		if( _ftime > 0.0f ) m_bRenderGlossy = false;

		return;
	}

	osassert( t_ptrBA );


	//
	// 对每一个Frame进行处理,根据当前插值系数对两帧进行插值。
	// 流程执行到此处时，是在一个动作内的不同动作关键帧之间进行插值。
	// 并且设置刀光是否已经播放为false
	osassert( t_ptrBA->m_iBoneNum>=0 );
	cal_finalPose();

	// 正常动作更新才重设
	if( _ftime > 0.0f ) m_bRenderGlossy = false;


	return;

	unguard;

}


/** \brief
*  如果当前处于动作变换状态，需要调用这个动作变得换函数。
*
*/
void osc_skinMesh::frame_moveActChange( float _ftime )
{
	guard;

	float          t_fLerpVal;
	os_boneAction*  t_ptrBA;
	os_boneAction*  t_ptrBALast;

	m_fLastUsedTime += _ftime;

	if( (m_fLastUsedTime > ACT_CHANGESTATETIME)||
		(!m_bLerpLastFAndFirstF )  )
	{

		if( !m_bLerpLastFAndFirstF )
			m_fLastUsedTime = 0.0f;
		else
			m_fLastUsedTime -= ACT_CHANGESTATETIME;

		m_bChangeActState = false;
		m_bLerpLastFAndFirstF = true;
		m_wAniIndex = 0;	

		// 重新调用动作更新函数。
		frame_moveAction( 0.0f );

		// 
		// 如果新的动作使用刀光，在此处开始使用新动作的刀光。
		if( m_ptrSGBpmIns )
		{
			if( m_bPlaySGlossy )
				m_dwFinalGlossyColor = m_ptrSGBpmIns->set_disSGlossy( m_iAniId );
		}
		if( m_ptrSGBpmIns8 )
		{
			if( m_bPlaySGlossy )
				m_dwFinalGlossyColor = m_ptrSGBpmIns8->set_disSGlossy( m_iAniId );
		}
		


		return;
	}

	t_fLerpVal = m_fLastUsedTime/ACT_CHANGESTATETIME;

	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	t_ptrBALast = &m_ptrBoneAni->m_vecBATable[m_iLastActId];

	int           t_iMatPtr;
	int           t_iNextActIdx;
	osMatrix       t_lastMat;
	os_bactPose*      t_matPtr;
	os_bactPose*      t_nextMatPtr;
	os_bactPose       t_tmpActPose;

	//
	// 对每一个Frame进行处理,根据当前插值系数对两帧进行插值。
	for( int t_i=0;t_i<t_ptrBA->m_iBoneNum;t_i++ )
	{

		//
		// 旧动作的两个帧之间做一次动作熔合.
		// 先得到上一个动作的最终矩阵.如果是最后一帧，则和第一帧进行插值。
		t_iMatPtr = t_ptrBALast->m_iBoneNum*m_wLastAniIndex;
		t_iMatPtr += t_i;

		if( m_wLastAniIndex >= (t_ptrBALast->m_iNumFrame-1) )
			t_iNextActIdx = 0;
		else
			t_iNextActIdx = m_wLastAniIndex+1;
		t_iNextActIdx = t_ptrBALast->m_iBoneNum*t_iNextActIdx;
		t_iNextActIdx += t_i;

		t_matPtr = &t_ptrBALast->m_vecMatrix[t_iMatPtr];
		t_nextMatPtr = &t_ptrBALast->m_vecMatrix[t_iNextActIdx];


		// River mod @ 2007-4-20:
		// 原来插值成矩阵,再返回成quat,现在直接插值成quat.
		/*
		lerp_actionPose( t_lastMat,t_matPtr,t_nextMatPtr,m_fActLerpVal );
		*/
		lerp_actionPose( &t_tmpActPose,t_matPtr,t_nextMatPtr,m_fActLerpVal );


		// 
		// 与新动作的第0帧进行混合。
		t_iMatPtr = 0;
		t_iMatPtr += t_i;
		t_matPtr = &t_ptrBA->m_vecMatrix[t_iMatPtr];

# ifdef _DEBUG
		if( t_matPtr->m_bFlipDeterminant != t_nextMatPtr->m_bFlipDeterminant )
		{
			osDebugOut( "SM名字为:<%s>,源动作<%s>,要改变到动作<%s>,出错...\n",
				m_strSMDir, t_ptrBALast->m_szActName,t_ptrBA->m_szActName );
		}
# endif 
		osassertex( (t_matPtr->m_bFlipDeterminant == t_nextMatPtr->m_bFlipDeterminant),
			"两个动作对应同一骨骼的FlipDeterminant必须一致...\n" );

		/*
		// River mod @ 2007-4-20:
		// 在帧之间对两个bone关系的矩阵进行插值。
		osQuaternionRotationMatrix( &t_tmpActPose.m_actQuat,&t_lastMat );
		t_tmpActPose.m_vec3Offset.x = t_lastMat._41;
		t_tmpActPose.m_vec3Offset.y = t_lastMat._42;
		t_tmpActPose.m_vec3Offset.z = t_lastMat._43;
		t_tmpActPose.m_bFlipDeterminant = t_matPtr->m_bFlipDeterminant;
		*/
		
		// 
		osassert( m_pframeToAnimate[t_i] );

		lerp_actionPose( m_pframeToAnimate[t_i]->m_matRot,
			&t_tmpActPose,t_matPtr,t_fLerpVal );
	}

	return ;

	unguard;
}







/** \brief
*  当前人物总共有多少套动作可以播放。
*/
int osc_skinMesh::get_totalActNum( void )
{
	guard;

	osassert( m_ptrBoneAni );
	return m_ptrBoneAni->m_iBANum;

	unguard;
}



/** \brief
*  从动作的名字得到播放当前动作需要的时间。
*/
float osc_skinMesh::get_actUsedTime( const char* _actName,int& _actIdx )
{
	guard;


	osassert( _actName!=NULL );
	osassert( _actName[0]!=NULL );
	osassert( this->m_ptrBoneAni );

	
	int        t_iNameIdx = -1;
	float       t_fTime;

	int   t_idx = m_ptrBoneAni->get_actIdxFromName( _actName,t_iNameIdx );

	if( t_iNameIdx < 0 )
	{
		osDebugOut( "在人物<%s>的动作列表中,找不到动作<%s>...\n",
			m_strSMDir,_actName );
		t_fTime = 0.0f;
		_actIdx = -1;
	}
	else
	{
		t_fTime = m_ptrBoneAni->m_vecBATable[t_idx].m_fTimeToPlay;
		_actIdx = t_idx;
	}

	return t_fTime;

	unguard;
}

/** \brief
*  从动作的名字得到播放当前动作需要的时间。
*/
float osc_skinMesh::get_actUsedTime( const char* _actName )
{
	guard;


	osassert( _actName!=NULL );
	osassert( _actName[0]!=NULL );
	
	if( m_bLoadLock )
		return -1.0f;
	
	osassert( m_ptrBoneAni );

	
	int        t_iNameIdx = -1;
	float       t_fTime = 1.0f;

	int   t_idx = m_ptrBoneAni->get_actIdxFromName( _actName,t_iNameIdx );

	if( t_iNameIdx < 0 )
	{
		osDebugOut( "在人物<%s>的动作列表中,找不到动作<%s>...\n",
			m_strSMDir,_actName );
		t_fTime = 0.0f;
	}
	else
	{
		osassert(t_idx != -1);
		t_fTime = m_ptrBoneAni->m_vecBATable[t_idx].m_fTimeToPlay;
	}

	return t_fTime;

	unguard;
}

/** \brief
*  从动作的id得到播放当前的动作需要的时间.
*/
float osc_skinMesh::get_actUsedTime( int _id )
{
	guard;

	os_boneAction*  t_ptrBA;
	osassert( _id<m_ptrBoneAni->m_iBANum );
	osassert( this->m_ptrBoneAni );

	t_ptrBA = &m_ptrBoneAni->m_vecBATable[_id];
	return t_ptrBA->m_fTimeToPlay;

	unguard;
}
void	osc_skinMesh::get_Default_ActName(std::string& _ActName)
{
	guard;
	osassert( m_ptrBoneAni );
	osassert( m_ptrBoneAni->m_vecActName.size() > 0 );

	osassert( strlen( m_ptrBoneAni->m_vecActName[0].m_szActName ) > 0 );
	_ActName = m_ptrBoneAni->m_vecActName[0].m_szActName;

	unguard;
}

void	osc_skinMesh::get_Act_List( std::vector<std::string>& _ActNameList )
{
	guard;
	_ActNameList.clear();
	for (size_t i =0 ;i < m_ptrBoneAni->m_vecActName.size();i++){
		_ActNameList.push_back( m_ptrBoneAni->m_vecActName[i].m_szActName );
	}
	unguard;
}

# ifdef _DEBUG
//! 导出当前skinMesh内的内容。
void osc_skinMesh::dump_debugStr( void )
{
	osDebugOut( "\n\n\n\nStart DUMP SKINMESH INFO\n" );
	osDebugOut( "Skin Num Parts is:<%d>..\n",this->m_iNumParts );
	osDebugOut( "Skin bf Num is:<%d>..\n",m_iBFNum );
	osDebugOut( "\n\n" );

}
# endif 

//! 当环境光发生变化时，对skinMeshMgr进行处理。
void osc_skinMeshMgr::reset_ambient( void )
{
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;
	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		//! River mod @ 2009-6-8:加入人物的多线程处理。
		if( t_ptrNode.p->m_bLoadLock )
			continue;
		osassertex( t_ptrNode.p->m_bInUse,
			va( "The Idx is:<%d>.Dir is:<%s>..",t_ptrNode.idx,t_ptrNode.p->m_strSMDir ) );


		// 
		// 对于高亮的人物，不能再设置环境光.
		if( t_ptrNode.p->m_bSetHighLight )
			continue;

		t_ptrNode.p->m_sCurSkinMeshAmbi.a = 1.0f;
		t_ptrNode.p->m_sCurSkinMeshAmbi.r = g_fAmbiR*CHAR_AMBISCALE;
		t_ptrNode.p->m_sCurSkinMeshAmbi.g = g_fAmbiG*CHAR_AMBISCALE;
		t_ptrNode.p->m_sCurSkinMeshAmbi.b = g_fAmbiB*CHAR_AMBISCALE;
	}


	for( t_ptrNode = m_vecSceneSM.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecSceneSM.next_validNode( &t_ptrNode ) )
	{
		//River mod @ 2007-4-10:确保不会出错.确保不会出其它的问题.
		if( (!t_ptrNode.p->m_bInUse)&&(!t_ptrNode.p->m_bLoadLock) )
		{
			m_vecSceneSM.release_node( t_ptrNode.idx );
			continue;
		}

		t_ptrNode.p->m_sCurSkinMeshAmbi.a = 1.0f;
		t_ptrNode.p->m_sCurSkinMeshAmbi.r = g_fAmbiR*CHAR_AMBISCALE;
		t_ptrNode.p->m_sCurSkinMeshAmbi.g = g_fAmbiG*CHAR_AMBISCALE;
		t_ptrNode.p->m_sCurSkinMeshAmbi.b = g_fAmbiB*CHAR_AMBISCALE;
	}

}

/** 
River @ 2009-6-5:新开一个线程，用于调入人物。

如果当前的过程中，没有需要调入的人物，学习地图调入的管理器，睡觉。
有需要调入的人物时，唤醒，然后调入。完成一个人物的调入后，查看是否有新的任务,
如果有，继续。如果没有，睡觉等待。
 


*/


/** \brief
*  用于创建和操作上层需要的SkinMeshObject.
*
*  由场景调用这个函数给上层提供可以使用的skinMesh指针。
*  上层对人物的操作都通过这个接口进行。
*
*  \param _smi 用于初始化新创建的skinMeshObject.
*  \return     如果创建失败，返回NULL.
*/
I_skinMeshObj* osc_skinMeshMgr::create_character( os_skinMeshInit* _smi,
			bool _sceneSm/* = false*/,bool _waitForFinish/* = false*/ )
{
	guard;

	osc_skinMesh*  t_ptrMesh;


	osassert( _smi );
	osassert( (_smi->m_iActIdx!=-1) || (_smi->m_szDefaultAct[0] != NULL) );
	osassertex(_smi->m_smObjLook.m_iPartNum > 0,_smi->m_strSMDir);
	osassertex( _smi->m_smObjLook.m_iPartNum > 0,
		    "必须有至于一个身体部位才能创建....\n" );

	//
	// 找到可用的character指针
	::EnterCriticalSection( &m_sChrCreateCS );
	if( _sceneSm )
		m_vecSceneSM.get_freeNodeAUse( &t_ptrMesh );
	else
		m_vecDadded.get_freeNodeAUse( &t_ptrMesh );
	::LeaveCriticalSection( &m_sChrCreateCS );
	
	os_charLoadData   t_s;
	memcpy( &t_s.m_sMeshInit,_smi,sizeof( os_skinMeshInit ) );
	t_s.m_ptrSkinPtr = t_ptrMesh;
	t_s.m_bChangeEquipent = false;

	//osDebugOut( "PUSH CREATE CHARACTER...<%d><%s>...\n",int( t_ptrMesh ),_smi->m_strSMDir );

	m_sCharLoadThread.push_loadChar( &t_s,_waitForFinish );


	return t_ptrMesh;

	unguard;
}





