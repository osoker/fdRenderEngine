//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrLoad.cpp
 *
 *  His:      River created @ 2004-1-8
 *
 *  Des:      用于调入地图格子的类。
 *   
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../mfpipe/include/osStateBlock.h"
# include "../../effect/include/fdEleBolt.h"
# include "../include/fdQuadtreeTemplate.h"
#include "../../mfpipe/include/osShader.h"
#include "../../mfpipe/include/ostexture.h"
#include "../../effect/include/VolumeFog.h"
//! 是否渲染地表的细节纹理.
OSENGINE_API BOOL     g_bUseTileDetailTex = true;

//! 水面倒影级别.0最高.2:最低.
OSENGINE_API int      g_iReflectionGrade = 0;

//! 是否使用场景中的动画物体。
OSENGINE_API BOOL     g_bUseAnimationObj = true;


//! 当前引擎中运行的场景的名字。
OSENGINE_API char     g_szMapName[16];


# define  DEFAULT_MOVESKY      "dfault_sky"
# define  DEFAULT_BKSKY        "keyani\\default_sky.osa"
# define  DEFAULT_BKTERR       "keyani\\default_terr.osa"
//# define  DEFAULT_BKSKY3        "keyani\\default_sky.osa"

UPDATE_LOADINGSTEPCALLBACKIMM osc_TGManager::m_pUpdate_uiLoadingStep = 0;

//! 注册回调
void osc_TGManager::register_updateLoadingStepCallbackFunc( UPDATE_LOADINGSTEPCALLBACKIMM _pFunc )
{
	m_pUpdate_uiLoadingStep = _pFunc;
}


//! 上层引擎的特效相关代码
DWORD  osc_TGManager::create_effect( const char* _ename,osVec3D& _pos,
					float _angle/* = 0.0f*/,BOOL _instance/* = TRUE*/,
					float _scale/* = 1.0f*/,
					float* _effectPlaytime/* = NULL*/  )
{
	guard;

	// 此处为上层特效的接口，上层创建的特效，都不是场景内用到的场景特效。
	return m_sEffectMgr->create_sEffect( _ename,
		_pos,_angle,false,_instance,_scale,_effectPlaytime );

	unguard;
}

//! 得到effect对应的boundingBox,其中boxsize是经过scale后的size.此box无旋转
bool osc_TGManager::get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize )
{
	return m_sEffectMgr->get_effectBox( _id,_pos,_boxSize );
}

//! 得到effect的包围盒，有旋转方向的包围盒.
const os_bbox* osc_TGManager::get_effectBox( DWORD _id )
{
	return m_sEffectMgr->get_effectBox( _id );
}



//! 内部使用的接口,用于创建场景内的特效,场景内的特效,不创建动态光.
DWORD osc_TGManager::create_effect( const char* _ename,
	osVec3D& _pos,float _angle,float _scale,bool _inScene )
{
	return m_sEffectMgr->create_sEffect( _ename,_pos,_angle,_inScene,true,_scale );
}

//! River @ 2010-1-14:上层可以到一个effect的keyTime.
float osc_TGManager::get_effectKeyTime( DWORD _id )
{
	return m_sEffectMgr->get_effectKeyTime( _id );
}


//! 在上层删除一个特效
void osc_TGManager::delete_effect( DWORD _id,bool _finalRelease/* = false*/ )
{
	m_sEffectMgr->delete_effect( _id,_finalRelease );
}

//! 对特效进行缩放,用于对场景内的道具进行缩放动画，但此缩放仅限于osa动画和公告板，粒子场景不能动态缩放
void osc_TGManager::scale_effect( int _id,float _rate,float _time )
{
	m_sEffectMgr->scale_effect( _id,_rate,_time );
}

//! River @ 2010-8-24:对特效的播放速度进行缩放。
void osc_TGManager::scale_effectPlaySpeed( int _id,float _speedRate )
{
	m_sEffectMgr->scale_effectPlaySpeed( _id,_speedRate );
}


//! 隐藏或显示当前id的effect
void osc_TGManager::hide_effect(int _id,bool _bHide/* = TRUE*/ )
{
	m_sEffectMgr->hide_effect( _id,_bHide );
}


/** \brief
*  设置特效的alpha值,只有osa特效才能正确的设置alpha值。alpha值介于0.0和1.0f之间。
*
*  \param int _id   特效对应的id.
*  \param float _alpha 特效要使用alpha值
*/
void osc_TGManager::set_effectAlpha( int _id,float _alpha )
{
	m_sEffectMgr->set_effectAlpha( _id,_alpha );
}


//! 对特效进行fadeOut
void osc_TGManager::fade_effect( int _id,float _fadetime,bool _fadeOut/* = true*/ )
{
	m_sEffectMgr->fade_effect( _id,_fadetime,_fadeOut );
}

//! 上层设置特效的位置和旋转方向
void osc_TGManager::set_effectPos( DWORD _id,osVec3D& _pos,
    float _angle,float _speed/* = 1.0f*/,bool _forceUpdate/* = false*/ )
{
	m_sEffectMgr->set_effectPos( _id,_pos,_angle,_speed,_forceUpdate );
}

//@{
//  客户端用到的特定数据，引擎只负责调入并由上层调用，引擎内不做处理。
//! 得到特效播放的位置类型 0:包围盒比例播放。 1:攻击位置播放。2:被攻击位置播放。
DWORD osc_TGManager::get_effectPlayPosType( int _id )
{
	return m_sEffectMgr->get_effectPlayPosType( _id );
}
//! 得到特效播放的包围盒比例，返回的值在0.0到1.0之间。
float osc_TGManager::get_effectPlayBoxScale( int _id )
{
	return m_sEffectMgr->get_effectPlayBoxScale( _id );
}
//@}


//! 验证一个effect是否已经在内部被删除
BOOL osc_TGManager::validate_effectId( DWORD _id )
{
	return m_sEffectMgr->validate_effectId( _id );
}


// 转化声音文件名字的函数测试!把ex文件名转化为wav声音文件名
static bool convert_soundString( s_string& _sndstr )
{
	guard;
	
	static s_string   t_strTmp;

	t_strTmp = _sndstr;
	int t_idx = t_strTmp.rfind( '.' );
	t_strTmp = t_strTmp.substr( 0,t_idx );
	t_idx = t_strTmp.rfind( '\\' );
	t_strTmp = t_strTmp.substr( t_idx+1,t_strTmp.length() );

	_sndstr = BKSOUND_DIR;
	_sndstr += t_strTmp;
	_sndstr += ".wav";

	return true;

	unguard;
}


bool osc_TGManager::load_SoundInfo(const char* fileName)
{
	guard;

	m_TGSoundList.clear();
	m_TGSoundAABBoxList.clear();

	s_string   t_szName;
	int        t_iSize;
	BYTE*      t_fstart;

	t_szName = SCENEMAP_DIR;
	t_szName += fileName;
	t_szName += "\\";
	t_szName += fileName;
	t_szName += ".snd";
	SndHeader* pHeader = NULL;
	if (!file_exist((char*)t_szName.c_str()))
	{
		return false;
	}

	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)t_szName.c_str(),t_fstart,TMP_BUFSIZE );
	if( t_iSize < 0 )
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}
	pHeader = (SndHeader*)t_fstart;
	if (strcmp(pHeader->mFileFlag,"GEO"))
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}
	// 加载声音列表
	OSSoundItem* pSoundItem = (OSSoundItem*)(t_fstart + pHeader->mdwOfsSoundItem);

	for (DWORD i = 0 ; i < pHeader->mdwNrSoundItem;i++)
	{
		// 
		// River mod @ 2008-3-18:先处理对应的声音文件名问题，地图编辑器存储的是ex文件
		s_string    t_str = pSoundItem[i].m_SoundName;
		convert_soundString( t_str );
		strcpy( pSoundItem[i].m_SoundName,t_str.c_str() );

		m_TGSoundList.push_back(pSoundItem[i]);
		os_aabbox box;
		osVec3D diag(pSoundItem[i].m_flMaxDistance/2.0f,pSoundItem[i].m_flMaxDistance/2.0f,pSoundItem[i].m_flMaxDistance/2.0f);
		box.m_vecMax = pSoundItem[i].m_pos + diag;
		box.m_vecMin = pSoundItem[i].m_pos - diag;
		if (_isnan(box.m_vecMax.x)||_isnan(box.m_vecMax.y)||_isnan(box.m_vecMax.z)||
			_isnan(box.m_vecMin.x)||_isnan(box.m_vecMin.y)||_isnan(box.m_vecMin.z))
		{
			osassertex(false,"请删除地图文件夹下的SND文件,该文件的板本太低!");
		}
		if (equal(box.m_vecMax.x,0)||equal(box.m_vecMax.y,0)||equal(box.m_vecMax.z,0))
		{
			osassertex(false,"请删除地图文件夹下的SND文件,该文件的板本太低!");
		}
		box.recalc_aabbox();
		m_TGSoundAABBoxList.push_back(box);
	}	
	
	END_USEGBUF( t_iGBufIdx );
	
	//！这样编译场景的声音会出现没有声音淡入的情况，
	//! 因为在另外一个结点时没有进入声音的四叉树范围内
	mSoundQuadTree.Compiler(m_TGSoundList,m_TGSoundAABBoxList);

	return true;

	unguard;
}


# if __QTANG_VERSION__
//! 由上层设置地图的碰撞信息,其实这些工作完全可以由客户端完成，为兼容以前的接口。
bool osc_TGManager::set_sceneCollInfo( char* _coll )
{
	guard;

	osassert( _coll );
	m_pMapCollInfo = _coll;
	

	return true;

	unguard;
}
# endif 


/** \brief
*  使用我们的场景调入结构调入场景地图。
*
*/
bool osc_TGManager::load_scene( os_sceneLoadStruct* _sl )
{
	guard;


# if __QTANG_VERSION__
	// River mod @ 2010-9-2:确认此值为空，否则上层可能发生空指针的bug.
	m_pMapCollInfo = NULL;
# endif 

	// syq 5-20 add
	if( osc_TGManager::m_pUpdate_uiLoadingStep )
		(*osc_TGManager::m_pUpdate_uiLoadingStep)(0,0);

	osassert( _sl );

	//! 调整水面高度时用 snowwin add
	osc_newTileGround::m_vecWaterHeight.clear();

	// River @ 2007-6-2: 场景内的资料版本号
	osc_newTileGround::m_bMapVersionChange = TRUE;

	// 
	// 保存全局的地图信息，重要信息！！！其它地方会用到g_szMapName变量
	strcpy( g_szMapName,_sl->m_szMapName );
	strcat( g_szMapName,"\\" );


	m_strMapName = SCENEMAP_DIR;
	m_strMapName += _sl->m_szMapName;
	m_strMapName += "\\";

	// 读取点光源文件
	std::string t_lightFile = m_strMapName ;
	t_lightFile += "lightPos";
	g_ptrLightMgr->ReadFromFile(t_lightFile.c_str(),OutputDebugString);

	// River @ 2009-2-26:
	g_ptrMeshMgr->sceneChange_reset();


	//! 调入地图的整体信息.
	if( !load_sceneInfoFile( _sl->m_szMapName ) )
	{
		osassertex( false,"地图目录下少了*.lst文件....\n" );
		return false;
	}

	// River mod :场景内的声音信息，必须做统一的处理。
	if (!load_SoundInfo(_sl->m_szMapName))
	{
		char msg[256];
		sprintf(msg,"地图[%s]不存在场景音效文件!\n",_sl->m_szMapName );
		osDebugOut(msg);
	}


	if( !m_ptrTerrainMgr->load_scene( _sl,m_iMaxXTgNum,m_iMaxZTgNum ) )
		osassertex( false ,"多线程初始化地图出错...\n" );

	//! 得到当前的激活tg列表.
	memset( m_arrPtrTG,NULL,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );
	m_iActMapNum = m_ptrTerrainMgr->
		init_getActMapList( m_arrPtrTG,MAXTG_INTGMGR );

	// 设置地图用到的全局光数据。
	reset_ambientCoeff();


	// 当前场景已经调入了地图。
	m_bLoadedMap = true;

	return true;
	
	unguard;
}





//! 把一个character指针推进到墙后渲染队列中
void osc_TGManager::push_behindWallRenderChar( I_skinMeshObj* _ptr )
{
	m_skinMeshMgr->push_behindWallChar( (osc_skinMesh*)_ptr );
}

//! 把一个character指针从墙后渲染队列中删除
void osc_TGManager::delete_behindWallRenderChar( I_skinMeshObj* _ptr )
{
	m_skinMeshMgr->delete_behindWallChar( (osc_skinMesh*)_ptr );
}


/** \brief
*  用于创建和操作上层需要的SkinMeshObject.
*
*  \param _smi 用于初始化新创建的skinMeshObject.
*  \return     如果创建失败，返回NULL.
*/
I_skinMeshObj* osc_TGManager::create_character( 
	os_skinMeshInit* _smi,bool _sceneSm/* = false*/,bool _waitForFinish/* = false*/ )
{
	return m_skinMeshMgr->create_character( _smi,_sceneSm,_waitForFinish );
}



/** \brief
*  得到当前场景中的skinMesh用到的三角形的数目。 
*
*  skinMesh主要用于人物的渲染，所以此处得到的值，为人物的面片数目。
*/
int osc_TGManager::get_skinMeshTriNum( void )
{
	return m_skinMeshMgr->get_smMgrFaceNum();
}



/** \brief
*  第一次调入场景的时候初始化TGManager中的变量。
*/
bool osc_TGManager::init_sceneWork( void )
{
	guard;


	// 分配Tg ptr内需要的内存
	m_arrPtrTG = new osc_TileGroundPtr[MAXTG_INTGMGR];


	m_deviceMgr = ::get_deviceManagerPtr();
	if( !m_middlePipe.init_middlePipe( m_deviceMgr ) )
		return false;

	if( !m_skinMeshMgr->init_smManager( &m_middlePipe ) )
		return false;

	m_bbMgr->init_bbManager( &m_middlePipe );
	m_boltMgr->init_boltMgr( &m_middlePipe );
	m_dlightMgr.init_dlightMgr( &m_middlePipe );


	// 初始化天空。如果不渲染地形，则不必初始化
	// ATTENTION TO FIX:场景初始化时,应该有默认的天空数据.
	//! 这些是默认的天空和远景数据.
	if( g_bRendSky ) //zmg:2006-11-21 2006-12-18
	{
		os_skyInit   t_sInit;
		t_sInit.m_fSkyHeight = 100;
		t_sInit.m_fSkyMoveSpeed = 0.01f;
		t_sInit.m_mdPipe = &m_middlePipe;
		strcpy( t_sInit.m_szIni, SKY_SHADERINI );
		strcpy( t_sInit.m_szSkyName, DEFAULT_MOVESKY);  
		strcpy( t_sInit.m_szBkSkyOsa,DEFAULT_BKSKY );
		strcpy( t_sInit.m_szBkTerrOsa,DEFAULT_BKTERR );
		t_sInit.m_vec3CloudDir = osVec3D( 1,0,2 );
		m_skyMgr.init_skyMgr( t_sInit );
	}


	//! 初始化粒子场景管理器。
	m_particleMgr.init_particleMgr( &m_middlePipe ); 

	//! 初始化下雨相关的数据。
	m_effectRain.init_rainData( &m_middlePipe );
	m_effectSnow.init_snowStormMgr(&m_middlePipe);


	// 初始化场景地形管理器,管理了当前可见的TileGround.
	m_ptrTerrainMgr->init_tgLRMgr( this );

	m_ptrDstr = new os_stringDisp[MAX_STRDIS_PERCI];
	//@{
	// Windy mod @ 2005-9-20 10:40:34
	//Desc: 初始化Lens Flare
#ifdef USE_LENSFLARE
	///设置默认位置
	m_pLensflare = new os_Lensflare;
	osVec3D lfpos(3000,1320,1000);
	m_pLensflare->setPosition(lfpos);
	m_pLensflare->init();
	m_pLensflare->setBaseAlpha(1.0);
#endif
	//@}

	// 为device Reset设置中间管道的指针.
	osc_d3dManager*    t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	t_ptrMgr->set_deviceResetNeedRes( &m_middlePipe,m_skinMeshMgr );

	//init HDRLight
	init_hdrLight();
	set_hdrParameter( 300, 300, 0.58f, 1.05f );
	
	///@{ Init Volume Fog
	//gVolumeFogPtr->InitVolumeFog(m_deviceMgr,m_middlePipe.get_device());
	///@}

	osc_specEffMgr::onceinit_specEffMgr();
	osc_decalMgr::OneTimeInit( this,&m_middlePipe );

	// windy add
	const D3DSURFACE_DESC* t_ptrDesc = m_deviceMgr->get_backBufDesc();
	mrefMapId = g_shaderMgr->getTexMgr()->addnew_texture( "g_refMap.texture",
		t_ptrDesc->Width>>g_iReflectionGrade,
		t_ptrDesc->Height>>g_iReflectionGrade,D3DFMT_A8R8G8B8,10);
	if( mrefMapId == -1 )
		g_bWaterReflect = false;
	else
		m_pRefMapTexture = (IDirect3DTexture9*)g_shaderMgr->getTexMgr()->get_texfromid(mrefMapId);

	//! River @ 2009-6-29: refraction map.
	m_iRefractionMapId = g_shaderMgr->getTexMgr()->addnew_texture( "g_refractionMap",
		t_ptrDesc->Width>>1,t_ptrDesc->Height>>1,D3DFMT_A8R8G8B8,10);
	if( m_iRefractionMapId == -1 )
	{
		osDebugOut("Failed Create Ref Map");
		g_bWaterReflect = false;
	}
	else
	{
		m_pRefractionTexture = (IDirect3DTexture9*)
			g_shaderMgr->getTexMgr()->get_texfromid( m_iRefractionMapId );
	}


	//! River mod @ 2008-12-15:创建一个与refMap相应的depth stencil buffer.
	HRESULT  t_hr;
	t_hr = m_middlePipe.get_device()->CreateDepthStencilSurface( 
		t_ptrDesc->Width>>g_iReflectionGrade,t_ptrDesc->Height>>g_iReflectionGrade,
		D3DFMT_D24S8,D3DMULTISAMPLE_NONE,0,true,&m_pRefMapDepth,NULL );
	if( FAILED( t_hr ) )
		g_bWaterReflect = false;

	m_middlePipe.get_device()->GetRenderTarget(0,&mBackUpSurface);
	m_middlePipe.get_device()->GetDepthStencilSurface( &m_pDepthStencilBackSur );

	//! initlize the light manager
	m_lightManager.Initlize(m_deviceMgr->get_d3ddevice());
	

	//@{ tzz add for some galaxy initializing
	#if GALAXY_VERSION
	
	m_iCurrBlood = 0;
	CBlood::LoadBloodTexture(m_deviceMgr->get_d3ddevice());
	m_vecBlood.resize(64);

	int t_size = m_vecBlood.size();
	for(int i = 0;i< t_size;i++){
		m_vecBlood[i].InitBlood(m_deviceMgr->get_d3ddevice());
	}

	#endif //GALAXY_VERSION
	//@}

	return true;

	unguard;
}


/** \brief
*  得到新地图的名字。
*
*  \param _nMname 传入的文件名，也使用这个字符串指针返回值。
*  \param _idr 共有8个方向，从0(Y轴)开始,顺时针方向: y,xy,x,x-y,  -y,-x-y,-x,-xy
*/
bool osc_TGManager::get_newMapName( char* _nMname,int _dir )
{
	guard;

	int       t_iNx,t_iNy;
	char      t_szName[32];
	bool      t_bRes = true;

	// 得到新地图的坐标
	t_iNx = this->m_vec2CamInIdx.m_iX;
	t_iNy = this->m_vec2CamInIdx.m_iY;

	// 
	// y,xy,x,x-y,  -y,-x-y,-x,-xy
	switch( _dir )
	{
	case 0:
		t_iNy ++;
		break;
	case 1:
		t_iNx++;
		t_iNy++;
		break;
	case 2:
		t_iNx++;
		break;
	case 3:
		t_iNx++;
		t_iNy--;
		break;
	case 4:
		t_iNy--;
		break;
	case 5:
		t_iNx--;
		t_iNy--;
		break;
	case 6:
		t_iNx--;
		break;
	case 7:
		t_iNx--;
		t_iNy++;
		break;
	default:
		osassert( false );
	}

	if( ((t_iNx+1)*MAX_GROUNDWIDTH) > this->m_iMaxX )
		t_bRes = false;
	if( ((t_iNy+1)*MAX_GROUNDWIDTH) > this->m_iMaxZ )
		t_bRes = false;
	
	if( (t_iNx < 0)||(t_iNy<0 ) )
		t_bRes = false;

	osassertex( t_bRes,va("要得到的地图名字为:<%s>,方向为:<%d>..\n",_nMname,_dir) );

	sprintf( t_szName,"%02d%02d.map",t_iNx,t_iNy );
	strcat( _nMname,t_szName );

	return t_bRes;

	unguard;
}







/** \brief
*  从一个坐标得到当前TGMgr中当前坐标的osc_TileGroundPtr.
*/
osc_TileGroundPtr osc_TGManager::get_tground( int _x,int _y )
{
	if( !is_tgLoaded( _x, _y ) )
		return NULL;

	for( int t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );

		if(  (m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX == _x)&&
			(m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ == _y ) )
			return m_arrPtrTG[t_i];
	}
	

# ifdef _DEBUG
	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{

		osDebugOut( "管理器中的地图索引<%d>为：<%d,%d>..\n",
			t_i,m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX,
			m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ );
	}
# endif 


	return NULL;
}
















