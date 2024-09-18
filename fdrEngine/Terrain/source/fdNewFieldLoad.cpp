//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldLoad.cpp
 *
 *  His:      River created @ 2005-11-28
 *            @ 2006-1-11:修改此文件,使作为最新geo文件格式的调入函数.
 *
 *  Des:      调入新的地图格式的相关函数
 *   
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdNewFieldStruct.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include  "../../mfpipe/include/deviceInit.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../backpipe/include/mainEngine.h"
#include "../../fdrEngine/light.h"

/** \brief
*  释放当前地图的资源.
*/
BOOL osc_newTileGround::release_TG( void )
{
	guard;

	if( !m_bInuse )
		return true;

	// 释放使用骨骼动画的动态的物品.
	relase_tgObj();
	m_bInuse = false;

	return TRUE;

	unguard;
}


//! 释放地图的物品。
BOOL osc_newTileGround::relase_tgObj( void )
{
	guard;

	os_newTGObject*     t_ptrObj = NULL;

	// 释放公告板物品。
	// 释放osa动画物品。
	for( int t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		if( (t_ptrObj->m_dwType ==1)&&(g_bUseAnimationObj) )
		{
			// 根据全局是否使用了sm,来释放。
			((I_skinMeshObj*)m_vecObjectDevIdx[t_i])->release_character( TRUE );
		}

		// 
		// 释放场景中的普通物品
		// 1:种类为普通物品.2:sm物品，但不使用sm动画。3：必须创建了设备相关的数据。
		if(  (t_ptrObj->m_dwType == 0)||
			 ((t_ptrObj->m_dwType==1)&&(!g_bUseAnimationObj) )  )
		{
			continue;
		}

		//
		// 释放水面物品。
		if( (t_ptrObj->m_dwType == 2 ) )
		{
			// 使用osa的水,则水面为osa文件.
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->delete_animationMesh( m_vecObjectDevIdx[t_i],true );
		}


		// 释放公告板物品
		if( (t_ptrObj->m_dwType == 3 ) )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->release_billBoard( m_vecObjectDevIdx[t_i] );
		}

		// 释放osa动画物品。
		if( t_ptrObj->m_dwType == 4 )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->delete_animationMesh( m_vecObjectDevIdx[t_i],true );
		}

		// 释放粒子物品。
		if( t_ptrObj->m_dwType == 5 )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->delete_particle( m_vecObjectDevIdx[t_i] );
		}

		// 释放场景内的特效
		if( t_ptrObj->m_dwType == 6 )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
			{
				m_ptrTgMgr->delete_effect( m_vecObjectDevIdx[t_i] );
				m_vecObjectDevIdx[t_i] = -1;
			}
		}
	}

	return true;

	unguard;
}


//! 得到每一个物品是否为bridge物品
void osc_newTileGround::process_objIsBridge( void )
{
	guard;

	os_newTGObject*   t_ptrObj;

	for( int t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		if( t_ptrObj->m_dwType != 0 )
		{
			m_vecObjectType[t_i] = 0;
			continue;
		}

		if( g_ptrMeshMgr->is_bridgeObj( m_vecObjectDevIdx[t_i] ) )
			m_vecObjectType[t_i] |= OBJ_BRIDGETYPE;
		if( g_ptrMeshMgr->is_treeObj( m_vecObjectDevIdx[t_i] ) )
			m_vecObjectType[t_i] |= OBJ_TREETYPE;
		if( g_ptrMeshMgr->is_AcceptFogObj( m_vecObjectDevIdx[t_i] ) )
			m_vecObjectType[t_i] |= OBJ_ACCEPTFOGTYPE;
		if( g_ptrMeshMgr->is_NoAlphaObj( m_vecObjectDevIdx[t_i] ) )
			m_vecObjectType[t_i] |= OBJ_NOALPHATYPE;

	}

	return;

	unguard;
}

//! 全局的调入进度
extern int g_iLoadProgress;

/** \brief
*  调入地图内所有的静态物品
*/
void osc_newTileGround::load_tgObj( void )
{
	guard;

	int               t_i;
	os_newTGObject*   t_ptrObj;
	BOOL             t_bRelplace;
	os_bbox           t_sBox;
	int              t_iLastUpdate = 0;

	// 每一个静态的obj都对应一个AABBox.
	m_vecObjAabbox.resize( m_iTGObjectNum );
		
	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];

		if(t_ptrObj->m_dwType == 9){
			// 点光源 物品
			g_ptrLightMgr->AddPointLight(t_ptrObj->m_vec3ObjPos);
			continue;
		}

		//
		// 如果场景中使用骨骼动画物品，则另外的地方创建。
		if( g_bUseAnimationObj )
		{
			if( t_ptrObj->m_dwType != 0 )
				continue;
		}
		
		// 水相关,公告板,特效物品,暂不处理.
		if( (t_ptrObj->m_dwType == 2)||
			(t_ptrObj->m_dwType == 3)||
			(t_ptrObj->m_dwType == 4)||
			(t_ptrObj->m_dwType == 5)||
			(t_ptrObj->m_dwType == 6) ) // windy add 5-20
			continue;

		m_vecObjectDevIdx[t_i] = 
			g_ptrMeshMgr->creat_meshFromFile(t_ptrObj->m_szName,t_bRelplace );

		osassertex( (m_vecObjectDevIdx[t_i] != -1),
			va( "调入物品<%s>出错...\n",t_ptrObj->m_szName ) );
		osassertex( !t_bRelplace,"地图内没有可用的物品创建空间....\n" );

		// 处理物品的aaBBox
		get_objBBox( t_i,t_sBox );
		t_sBox.to_aabbox( m_vecObjAabbox[t_i] );

		// syq 5-20 add
		int t_iUpdate = g_iLoadProgress + int(float(t_i)/float(m_iTGObjectNum)*30);
		if( abs( t_iUpdate - t_iLastUpdate )>=4 )
		{
			if( osc_TGManager::m_pUpdate_uiLoadingStep )
				(*osc_TGManager::m_pUpdate_uiLoadingStep)( t_iUpdate,0);
			t_iLastUpdate = t_iUpdate;
		}


	}

	return ;

	unguard;
}



/** \brief
*  创建动画物品相关的数据。
* 
*   
*/
BOOL osc_newTileGround::creaet_keyAniObj( void )
{
	guard;

	int             t_i,t_idx;
	os_newTGObject*     t_ptrObj;
	s_string         t_str1,t_str2,t_str3;

	int             t_iKeyNum = 0;

	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		// 如果type为4,则是keyAniObject.
		if( t_ptrObj->m_dwType !=4 )
			continue;

		//
		// 得到keyAni的文件名。
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = KEYANIOBJ_DIR;
		t_str3 += t_str1;
		t_str3 += ".osa";

		
		os_aniMeshCreate  t_mc;

		t_mc.m_ePlayType = OSE_PLAYFOREVER;
		t_mc.m_fAngle = t_ptrObj->m_fAngle;
		t_mc.m_vec3Pos = t_ptrObj->m_vec3ObjPos;

		strcpy( t_mc.m_szAMName,t_str3.c_str() );
		t_mc.m_dwAmbientL = m_ptrTgMgr->get_ambientLight();
		
		m_vecObjectDevIdx[t_i] = m_ptrTgMgr->create_keyAniMesh( t_mc );


		osassertex( m_vecObjectDevIdx[t_i] >= 0,
			va( "创建特效动画文件<%s>出错",t_str3.c_str() ) );

		t_iKeyNum ++;

		// 
		// 确认每一张小地图最多使用的osa物品个数。
		osassert( t_iKeyNum < MAX_OSANUM_PERTG );
	}

	osDebugOut( "The map <%s>'s keyAniObj num is:<%d>..\n",
		m_strMapFName.c_str(),t_iKeyNum );

	return TRUE;

	unguard;
}


/** \brief
*  对地图上billBoard物品进行创建。
*  
*/
BOOL osc_newTileGround::create_billBoardObj( void )
{
	guard;

	int             t_i,t_idx;
	os_newTGObject*     t_ptrObj;
	s_string         t_str1,t_str2,t_str3;
	osVec3D          t_pos;

	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		if( t_ptrObj->m_dwType !=3 )
			continue;

		t_pos = t_ptrObj->m_vec3ObjPos;

		//
		// 得到billBoard的文件名。
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = BILLBOARD_DIR;
		t_str3 += t_str1;
		t_str3 += ".bb";

		m_vecObjectDevIdx[t_i] = 
			m_ptrTgMgr->create_billBoard( t_str3.c_str(),t_pos );

		osassertex( (m_vecObjectDevIdx[t_i] >= 0),
			va( "创建公告板文件<%s>出错",t_str3.c_str() ) );
	}

	return true;

	unguard;
}


/** \brief
*  创建TileGround中的Water相关物品。
*/
BOOL osc_newTileGround::create_waterObj( void )
{
	guard;

	int                t_i,t_idx;
	os_newTGObject*     t_ptrObj;
	s_string         t_str1,t_str2,t_str3;

	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];

		// 2为水的类型。
		if( t_ptrObj->m_dwType !=2 )
			continue;

		// 得到keyAni的文件名。
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = KEYANIOBJ_DIR;
		t_str3 += t_str1;
		t_str3 += ".osa";

		os_aniMeshCreate  t_mc;

		t_mc.m_ePlayType = OSE_PLAYFOREVER;
		t_mc.m_fAngle = t_ptrObj->m_fAngle;
		t_mc.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		strcpy( t_mc.m_szAMName,t_str3.c_str() );
		t_mc.m_dwAmbientL = m_ptrTgMgr->get_ambientLight();

		//River added @ 2008-12-5:用于全场景的水面效果
		//!wxd add 如果水面近似取整，凡大于最低水面高度的，都不做边缘透明化处理
		float t_FloorWaterHeight= (float)floor(t_mc.m_vec3Pos.y);

		if (fabs(t_FloorWaterHeight-t_mc.m_vec3Pos.y)<0.003f)
		{
			t_mc.m_vec3Pos.y = t_FloorWaterHeight;
		}
		if (m_ptrTgMgr->m_sSceneTexData.m_fWaterHeight>t_mc.m_vec3Pos.y)
		{
			m_ptrTgMgr->m_sSceneTexData.m_fWaterHeight = t_mc.m_vec3Pos.y;
		}

		m_ptrTgMgr->m_sSceneTexData.push_waterHeight( t_mc.m_vec3Pos.y );
		

		t_mc.m_bWaterAni = true;

		m_vecObjectDevIdx[t_i] = m_ptrTgMgr->create_keyAniMesh( t_mc );

		osassertex( (m_vecObjectDevIdx[t_i]>=0), va( "创建<%s>物品失败..",t_mc.m_szAMName ) );
	}

	return true;

	unguard;
}
BOOL osc_newTileGround::create_FogObj( void )
{
	guard;
	int                t_i,t_idx;
	os_newTGObject*     t_ptrObj;
	s_string         t_str1,t_str2,t_str3;

	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];

		// 8为雾类型。
		if( t_ptrObj->m_dwType != 8 )
			continue;

		// 得到keyAni的文件名。
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = KEYANIOBJ_DIR;
		t_str3 += t_str1;
		t_str3 += ".osa";

		os_aniMeshCreate  t_mc;

		t_mc.m_ePlayType = OSE_PLAYFOREVER;
		t_mc.m_fAngle = t_ptrObj->m_fAngle;
		t_mc.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		strcpy( t_mc.m_szAMName,t_str3.c_str() );
		t_mc.m_dwAmbientL = m_ptrTgMgr->get_ambientLight();

		//River added @ 2008-12-5:用于全场景的水面效果
		//m_ptrTgMgr->m_sSceneTexData.m_fWaterHeight = t_mc.m_vec3Pos.y;	
		t_mc.m_bFogAni = true;
		g_bVolumeFogInited = TRUE;

		m_vecObjectDevIdx[t_i] = m_ptrTgMgr->create_keyAniMesh( t_mc );

		osassertex( (m_vecObjectDevIdx[t_i]>=0), va( "创建<%s>物品失败..",t_mc.m_szAMName ) );
	}
	return true;
	unguard;
}

/** \brief
*  创建场景中粒子相关的物品。
*/
BOOL osc_newTileGround::create_particleObj( void )
{
	guard;

	int             t_i,t_idx;
	os_newTGObject*     t_ptrObj;
	s_string         t_str1,t_str2,t_str3;
	os_particleCreate   t_pCreate;


	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		
		// 5为粒子的类型。
		if( t_ptrObj->m_dwType !=5 )
			continue;

		//
		// 得到要创建粒子的文件名。
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = PARTICLE_DIR;
		t_str3 += t_str1;
		t_str3 += ".fdp";

		t_pCreate.m_bAutoDelete = true;
		t_pCreate.m_vec3Offset = t_ptrObj->m_vec3ObjPos;
		strcpy( t_pCreate.m_szParticleName,t_str3.c_str() );
		m_vecObjectDevIdx[t_i] = m_ptrTgMgr->create_particle( t_pCreate,true );
		osassertex( m_vecObjectDevIdx[t_i]>=0,
			va( "创建粒子<%s>失败...",t_pCreate.m_szParticleName ) );

	}

	return true;

	unguard;
}
struct ChaHeader
{
	ChaHeader():mbUseDirLAsAmbient(FALSE),mAttackedPos(0.0f,0.0f,0.0f)
	{
		mStartPos = osVec3D( 0.0f,0.0f,0.0f );
	}

	DWORD	mID;		//!< 文件ID
	DWORD	mVersion;	//!< 版本信息

	DWORD	mHeadPicFileNameOfs;	//!< 人物头像文件名偏移,Char类型
	DWORD	mHeadPicFileNameNr;		

	int		mTotalActTypeNum;
	int		mPartNum;

	osVec3D	mBoxMax;
	osVec3D	mBoxMin;

	osVec3D	mPos;
	float		mRot;

	osVec3D	mDeathBoxMax;
	osVec3D	mDeathBoxMin;

	BOOL		mbUseDirLAsAmbient;

	osVec3D	mAttackedPos;
	//! River added @ 2007-6-4:加入怪物攻击的开始位置
	osVec3D	mStartPos;


	BOOL		mbIsRide;	//!< 是否是坐骑
	osVec3D	mRidePos;	//!< 坐骑微调节

	DWORD	mEffectOfs;
	DWORD	mEffectNr;
	float	mScale;
	DWORD	mReserved4;
};


// 人物伴随特效 
struct SChaEffectItem
{
	DWORD mNameOfs;	
	DWORD mNameNr;
	osVec3D	mEffectOffset;
};


class CMyChaFile
{
public:
	CMyChaFile();
	~CMyChaFile();
	bool load_chaFile( const char* _filename );

	os_skinMeshInit& get_skinMeshInit()	{return m_osSkinMeshInit;}

	const char* get_headTexName();
	int get_totalActTypeNum();
	int get_partNum();
	osVec3D& get_boxMax();
	osVec3D& get_boxMin();
	osVec3D& get_deathBoxMax();
	osVec3D& get_deathBoxMin();
	osVec3D& get_attackedPos();
	osVec3D& get_startPos( void ) { return m_header.mStartPos; } 

private:
	ChaHeader       m_header;
	std::string		mHeadPicFileNamStr;
	os_skinMeshInit m_osSkinMeshInit;

};

CMyChaFile::CMyChaFile()
{
}

CMyChaFile::~CMyChaFile()
{
}


bool CMyChaFile::load_chaFile( const char* _filename )
{
	guard;

	LPBYTE   t_fstart;

	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );

	if( 0 >= read_fileToBuf( (char*)_filename,t_fstart,TMP_BUFSIZE ) )
	{
		osassert( false );
		return false;
	}

	memcpy(&m_header,t_fstart,sizeof(ChaHeader));

	//if (m_header.mID==ChaFileID)
	//{
	mHeadPicFileNamStr = (char*)(t_fstart + m_header.mHeadPicFileNameOfs);
	//}
	END_USEGBUF( t_iGBufIdx );

	// River added @ 2007-6-5:加入最新cha版本的验证:
	osassertex( m_header.mVersion >= 0x109,
		va( "<%s>文件不是最新的109版本...\n",_filename ) );

	if(m_header.mVersion>=0x108)
		m_osSkinMeshInit.m_Scale = m_header.mScale;


	m_osSkinMeshInit.m_iTotalActTypeNum = m_header.mTotalActTypeNum;

	m_osSkinMeshInit.m_smObjLook.m_iPartNum = m_header.mPartNum;
	osassert( m_osSkinMeshInit.m_smObjLook.m_iPartNum > 0 );
	osassert( m_osSkinMeshInit.m_smObjLook.m_iPartNum < MAX_SKINPART );

	m_osSkinMeshInit.m_vec3Max = m_header.mBoxMax;
	m_osSkinMeshInit.m_vec3Min = m_header.mBoxMin;
	m_osSkinMeshInit.m_vec3AdjPos = m_header.mPos;
	m_osSkinMeshInit.m_fAdjRotAgl = m_header.mRot;
	m_osSkinMeshInit.m_bUseDirLAsAmbient = m_header.mbUseDirLAsAmbient;
	m_osSkinMeshInit.m_bIsRide = m_header.mbIsRide;
	m_osSkinMeshInit.m_vec3RideOffset = m_header.mRidePos;
	m_osSkinMeshInit.m_iActIdx = OSRAND%m_header.mTotalActTypeNum;

	// 人物是否有伴随的特效
	if (m_header.mEffectNr >= 1)
	{
		SChaEffectItem* pEffectItem;
		pEffectItem = (SChaEffectItem*)(t_fstart + m_header.mEffectOfs);
		ZeroMemory(m_osSkinMeshInit.m_szspeFileName,64);
		strncpy(m_osSkinMeshInit.m_szspeFileName,(char*)(t_fstart + pEffectItem->mNameOfs),pEffectItem->mNameNr);
		m_osSkinMeshInit.m_spePostion = pEffectItem->mEffectOffset;
	}
	else
		m_osSkinMeshInit.m_szspeFileName[0] = NULL;


	WORD mesh[MAX_SKINPART] = { 1,1,0,0,0,0,0,0 };
	WORD skin[MAX_SKINPART] = { 1,1,0,0,0,0,0,0 };
	memcpy( m_osSkinMeshInit.m_smObjLook.m_mesh , mesh, sizeof(WORD)*MAX_SKINPART );
	memcpy( m_osSkinMeshInit.m_smObjLook.m_skin , skin, sizeof(WORD)*MAX_SKINPART );

	
	return true;

	unguard;
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
//! 把osa文件调入到相应的smInit结构内。
BOOL osc_newTileGround::load_smInitData( const char* _sz,
				os_skinMeshInit& _smInit,os_newTGObject*  _ptrObj )
{
	guard;


	s_string    t_szFname;

	// 优先调入*.cha文件。
	t_szFname = _sz;
	t_szFname += ".cha";
	if( file_exist( (char*)t_szFname.c_str() ) )
	{
		//
		CMyChaFile chaFileLoader;
		chaFileLoader.load_chaFile(t_szFname.c_str());
		_smInit = chaFileLoader.get_skinMeshInit();
	}
	else
	{
		t_szFname = _sz;
		t_szFname += ".bfs";

		int   t_iGBufIdx = -1;
		BYTE*  t_fstart = START_USEGBUF( t_iGBufIdx );
		read_fileToBuf( (char*)t_szFname.c_str(), t_fstart, TMP_BUFSIZE );
		sscanf( (const char*)t_fstart, "%d%d", 
			&_smInit.m_iTotalActTypeNum,&_smInit.m_smObjLook.m_iPartNum );
		
		END_USEGBUF( t_iGBufIdx );
	}



	_smInit.m_fAngle = _ptrObj->m_fAngle+OS_PI/2;
	_smInit.m_iActIdx = 0;
	_smInit.m_iTotalActTypeNum = 1;
	for( int t_j=0;t_j<_smInit.m_smObjLook.m_iPartNum;t_j++ )
	{
		_smInit.m_smObjLook.m_mesh[t_j] = 1;
		_smInit.m_smObjLook.m_skin[t_j] = 1;
	}

	strcpy( _smInit.m_strSMDir,_sz );

	return TRUE;

	unguard;
}


/** \brief
*  创建地图上使用skinMesh进行动画的物体。
*
*  真接使用skinMesh管理进行创建。动画物体的类型为1.
*/
BOOL osc_newTileGround::create_smAniObj( void )
{
	guard;

	int             t_i,t_idx,t_iAniNum;
	os_newTGObject*     t_ptrObj;
	s_string         t_str1,t_str2,t_str3;
	os_skinMeshInit  t_smInit;

	t_iAniNum = 0;
	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		if( t_ptrObj->m_dwType !=1 )
			continue;

		//
		// 得到skinMeshAinmation的文件名。
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = SMAINOBJ_DIR;
		t_str3 += t_str1;

		// 
		// 读取bfs文件，得到当前skinMesh的part数目。
		load_smInitData( t_str3.c_str(),t_smInit,t_ptrObj );

		t_smInit.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		t_smInit.m_bWriteZbuf = false;

		// 
		// 转化为DWORD值，赋给设备相关数据。
		m_vecObjectDevIdx[t_i] = 
			(DWORD)(m_ptrTgMgr->create_character( &t_smInit,true ));
		osassertex( m_vecObjectDevIdx[t_i],
			va( "创建sm物品<%s>失败...",t_str3.c_str() ) );

		t_iAniNum ++;

# ifdef _DEBUG
		if( t_iAniNum >= MAX_SMOBJPERMAP )
		{
			char   t_str[128];

			sprintf( t_str,
				"The <%s> map 超过了64个skinMesh物品..\n",m_strMapFName.c_str() );
			MessageBox( NULL,t_str,"Error",MB_OK );
			osassert( false );
		}
# endif 

	}

	osDebugOut( "当前场景中创建了< %d >个动画物体\n",t_iAniNum );

	return true;

	unguard;
}

/** \brief
*  创建场景内的整体特效物品，即spe物品
*/
BOOL osc_newTileGround::create_effectObj( void )
{
	guard;

	os_newTGObject*     t_ptrObj;
	s_string            t_str1,t_str2,t_str3;
	int                 t_idx;

	for( int t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		if( t_ptrObj->m_dwType != 6 )
			continue;

		// 得到spe文件的文件名  SCEEFFECT_DIR
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = SCEEFFECT_DIR;
		t_str3 += t_str1;
		t_str3 += ".spe";

		// River @ 2009-5-4:使用最大的缩放数据来缩放特效物品
		float t_f = t_ptrObj->m_vec3ObjectScale.x;
		if( t_f < t_ptrObj->m_vec3ObjectScale.y )
			t_f = t_ptrObj->m_vec3ObjectScale.y;
		if( t_f < t_ptrObj->m_vec3ObjectScale.z )
			t_f = t_ptrObj->m_vec3ObjectScale.z;

		m_vecObjectDevIdx[t_i] = (DWORD)m_ptrTgMgr->create_effect(
			t_str3.c_str(),t_ptrObj->m_vec3ObjPos,t_ptrObj->m_fAngle,t_f,true );

		osassertex( m_vecObjectDevIdx[t_i]>=0,
			va( "Create spe file Failed <%s>..\n",t_str3.c_str() ) );
	}

	return TRUE;

	unguard;
}


//! 创建此tg内所有的物品的设备相关数据
BOOL osc_newTileGround::create_devdepObj( void )
{
	guard;

	m_vecObjInView.resize( m_iTGObjectNum );
	m_vecObjectDevIdx.resize( m_iTGObjectNum );
	m_vecObjectType.resize( m_iTGObjectNum );
	if( m_iTGObjectNum > 0 )
		memset( &m_vecObjectType[0],0,sizeof( BYTE )*m_iTGObjectNum );

	for( int t_i=0;t_i<m_iTGObjectNum;t_i ++ )
		m_vecObjectDevIdx[t_i] = -1;
	
	// 
	// 创建调入每一个静态的物品
	load_tgObj();

	// 确认每一个obj是否为bride Obj.
	process_objIsBridge();


	//
	// 创建场景中的动画物品。
	if( g_bUseAnimationObj )
	{
		if( !create_smAniObj() )
			return FALSE;
	}

	// 
	//  创建billBoard物品.
	if( !create_billBoardObj() )
		return FALSE;

	//　创建keyAnimationMesh物品。
	if( !creaet_keyAniObj() )
		return FALSE;

	//
	// 创建水相关的物品。
	if( !this->create_waterObj() )
		return FALSE;

	// 创建雾相关的物品。
	if( !this->create_FogObj() )
		return FALSE;
	

	//
	// 创建场景中粒子相关的物品。
	if( !this->create_particleObj() )
		return FALSE; 

	// 创建场景内的整体特效物品
	if( !this->create_effectObj() ) 
		return FALSE;

	return TRUE;

	unguard;
}

//! 立即的创建一个objMesh.
BOOL osc_newTileGround::create_meshImmediate( int _idx )
{
	guard;

	osassert( _idx < m_iTGObjectNum );

	BOOL   t_b = false;
	// 创建物品的id.
	m_vecObjectDevIdx[_idx ] = 
		g_ptrMeshMgr->creat_meshFromFile( m_vecTGObject[_idx].m_szName,t_b );

	return TRUE;

	unguard;
}

/** 根据地图的全地图路径名,得到地图名
*	如:_szfullname map\vilg\vilg0000.geo,
*	_szmapname vilg
*/
void osc_newTileGround::getmapname( const char* _szfullname,s_string &_szmapname)
{
	guard;
	static const std::basic_string <char>::size_type npos = -1;

	s_string   t_szfullname = _szfullname;
	
	size_t indexCh = t_szfullname.rfind('\\',t_szfullname.size());
	if (indexCh!=npos)
	{
		_szmapname = t_szfullname.substr(0,indexCh);
		size_t indexCh2 = _szmapname.rfind('\\',_szmapname.size());
		if (indexCh2 != npos)
		{
			_szmapname = _szmapname.substr(indexCh2+1,indexCh);
		}

		return;
	}
	else
		osassertex( false,va( "错误的文件名<%s>...\n",_szfullname ) );

	unguard;
}

//! 调入当前tileGround内的硬地表数据。
void osc_newTileGround::load_hardTileData( const BYTE* _fstart )
{
	guard;

	osassert( _fstart );

	int   t_iSize = 0;
	int   t_i;

	m_vecChunkHardTile.resize( CHUNKNUM_PERTG );
	for( t_i= 0;t_i<CHUNKNUM_PERTG;t_i ++ )
		m_vecChunkHardTile[t_i].reset();

	// 版本号低于17,直接返回
	if( m_sTgHeader.m_dwVersion<17 )
		return;

	BYTE*  t_bStart = (BYTE*)(_fstart + m_sTgHeader.m_dwHardTexSizeOffset);
	for( t_i=0;t_i<CHUNKNUM_PERTG;t_i ++ )
	{
		READ_MEM_OFF( &m_vecChunkHardTile[t_i].m_iHardTileNum,t_bStart,sizeof( int ) );

		if( m_vecChunkHardTile[t_i].m_iHardTileNum == 0 )
			continue;

		osassertex( m_vecChunkHardTile[t_i].m_iHardTileNum< 10000,
			va("非法的硬纹理数据<%d>..\n",m_vecChunkHardTile[t_i].m_iHardTileNum) );

		m_vecChunkHardTile[t_i].m_arrTileInChunk = 
			new osc_hardTextureTile[m_vecChunkHardTile[t_i].m_iHardTileNum];
		
		
		for( int t_j = 0;t_j<m_vecChunkHardTile[t_i].m_iHardTileNum;t_j ++ )
		{
			osc_hardTextureTile*  t_ptrTT = &m_vecChunkHardTile[t_i].m_arrTileInChunk[t_j];
			READ_MEM_OFF( &t_ptrTT->m_iTileNum,t_bStart,sizeof( int ) );
			t_ptrTT->m_vecTileInChunk.resize( t_ptrTT->m_iTileNum );
			READ_MEM_OFF( &t_ptrTT->m_vecTileInChunk[0],
				t_bStart,sizeof( osc_tileInChunk )*t_ptrTT->m_iTileNum );

			// 对应的纹理名字和uv缩放
			READ_MEM_OFF( t_ptrTT->m_szTexName,t_bStart,sizeof( char )*32 );
			READ_MEM_OFF( &t_ptrTT->m_wU,t_bStart,sizeof( t_ptrTT->m_wU ) );
			READ_MEM_OFF( &t_ptrTT->m_wV,t_bStart,sizeof( t_ptrTT->m_wV ) );
			READ_MEM_OFF( &t_ptrTT->m_rotation,t_bStart,sizeof( t_ptrTT->m_rotation) );
		}
	}

	//! 返回
	return;

	unguard;
}


//! 全局的资料版本号
MapVersionHead osc_newTileGround::m_sMapVersionHead;
BOOL osc_newTileGround::m_bMapVersionChange = TRUE;


/** \brief
*  从内存中调入一个地图文件。
*
*  \param _fload  是否是游戏初始化时第一次调入地图。
*/
BOOL osc_newTileGround::load_TGData( const char* _fname,
					const osc_middlePipe* _mpipe,bool _fload/* = false*/ )
{
	guard;
	s_string  mapname;
	
	getmapname(_fname,mapname);

	// 先调入数据
	BYTE*     t_fstart;
	int       t_size;

	assert( _fname );
	osassert( m_bInuse == false );

	//
	// 
	m_strMapFName = _fname;

	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)m_strMapFName.c_str(),t_fstart,TMP_BUFSIZE );
	osassert( t_size > 0 );

	memcpy( &m_sTgHeader,t_fstart,sizeof( os_newFileHeader ) );
	if (m_sTgHeader.m_dwVersion<11)
	{
		osassertex(false,"地图版本号过低,请转化到最新板本");
	}

# if __QTANG_VERSION__
	//! River added @ 2009-9-14:低端的版本，去掉一些处理。
	if( m_sTgHeader.m_dwVersion<16 )
	{
		osassertex( false,"地图版本号过低,请转化到最低16板本");
	}
# endif 


# if !__QTANG_VERSION__
	// River @ 2007-6-2:加入资料版本的信息
	if( m_sTgHeader.m_dwVersion >= 15 )
	{
		MapVersionHead  t_sMapVersionHead;
		memcpy( &t_sMapVersionHead, t_fstart+sizeof(os_newFileHeader),
			sizeof(MapVersionHead) );
	}
# endif 

	// 调用地形数据接口,分配地形数据
	if( !m_sMapTerrain.LoadMapTile( t_fstart,mapname.c_str() ) )
	{
		END_USEGBUF( t_iGBufIdx );
		return FALSE;
	}
	memset(m_vecUpmostTexIdx,0,sizeof(BYTE)*GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT);
	memcpy(m_vecUpmostTexIdx,t_fstart + m_sTgHeader.m_dwUpmostTexIdxOffset,
		sizeof(BYTE)*GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT);
	
	// 碰撞信息.
	memcpy( m_arrCollInfo,t_fstart + m_sTgHeader.m_dwColHeiInfoOffset,
		sizeof( os_TileCollisionInfo )*GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT );

	// 每一个格子的属性信息
	m_vecTileAttribute.resize( MAX_GROUNDWIDTH*MAX_GROUNDWIDTH );
	memcpy( &m_vecTileAttribute[0],t_fstart + m_sTgHeader.m_dwTileAttributeOffset,
		sizeof( BYTE )*MAX_GROUNDWIDTH*MAX_GROUNDWIDTH );

	//! 保存obj相关的数据
	m_iTGObjectNum = m_sTgHeader.m_dwObjNum;
	m_vecTGObject.resize( m_iTGObjectNum );
	m_vecTGObjDL.resize( m_iTGObjectNum );

# if !__QTANG_VERSION__
	// 13以前的版本没有物品的地表点着色信息
	if( m_sTgHeader.m_dwVersion < 13 )
	{
		for( int t_idx=0;t_idx < m_iTGObjectNum;t_idx ++ )
		{
			DWORD   t_dwOffset;
			t_dwOffset = m_sTgHeader.m_dwObjDataOffset + 
				t_idx * (sizeof( os_newTGObjectOld13 )-sizeof(DWORD));
			memcpy( &m_vecTGObject[t_idx],t_fstart + t_dwOffset,
				sizeof( os_newTGObjectOld13 )-sizeof( DWORD ) );

			m_vecTGObject[t_idx].m_dwObjTerrColor = 0xffffffff;

			// 
			// 默认使用1.0的缩放标准。
			m_vecTGObject[t_idx].m_vec3ObjectScale = osVec3D( 1.0f,1.0f,1.0f );

		}
	}
	// River mod @ 2009-4-16:加入物品的缩放信息，主要处理版本13
	else if( m_sTgHeader.m_dwVersion < 14 )
	{
		for( int t_idx=0;t_idx < m_iTGObjectNum;t_idx ++ )
		{
			DWORD   t_dwOffset;
			t_dwOffset = m_sTgHeader.m_dwObjDataOffset + 
				t_idx * (sizeof( os_newTGObjectOld13 ) );

			memcpy( &m_vecTGObject[t_idx],t_fstart + t_dwOffset,
				   sizeof( os_newTGObjectOld13 ) );

			// 
			// 默认使用1.0的缩放标准。
			m_vecTGObject[t_idx].m_vec3ObjectScale = osVec3D( 1.0f,1.0f,1.0f );

		}
	}
	else
	{
		if( m_iTGObjectNum > 0 )
		{
			memcpy( &m_vecTGObject[0],t_fstart + m_sTgHeader.m_dwObjDataOffset,
				sizeof( os_newTGObject )*this->m_iTGObjectNum );
		}
	}
# else

	if( m_iTGObjectNum > 0 )
	{
		memcpy( &m_vecTGObject[0],t_fstart + m_sTgHeader.m_dwObjDataOffset,
			sizeof( os_newTGObject )*this->m_iTGObjectNum );
	}

# endif 
	

	if (m_sTgHeader.m_dwWaterInfoOffset!=0)
		memcpy(&mWaterHeight,t_fstart + m_sTgHeader.m_dwWaterInfoOffset,sizeof(float));


	// 
	// 保存每一个格子的顶点高度信息
	m_vecHeight.resize( (MAX_GROUNDWIDTH+1)*(MAX_GROUNDWIDTH+1) );
	memcpy( &m_vecHeight[0],t_fstart + m_sTgHeader.m_dwHeightInfoOffset,
		sizeof( float )*(MAX_GROUNDWIDTH+1)*(MAX_GROUNDWIDTH+1) );
	
	// 处理四叉树数据
	if( !m_sQuadTree.load_quadtreeDataFromMem( this,t_fstart ) )
	{
		END_USEGBUF( t_iGBufIdx );
		return FALSE;
	}

	//! River added @ 2009-9-14:加入硬纹理显示层
	load_hardTileData( t_fstart );

	END_USEGBUF( t_iGBufIdx );


	//
	// River mod @ 2007-1-5:调入修改后的地图碰撞数据,在最终的版本中，会去掉此ccl数据，把
	// 调整后的碰撞数据加入到地图的文件中.
# if __COLHEI_MODIFY__

	osassert( m_ptrTgMgr );
	m_ptrTgMgr->load_colheiAdjData( _fname,m_arrCollInfo );

# endif 
	
	// 创建设备相关的数据地形
	if( !m_sMapTerrain.InitDeviceData() )
		return FALSE;

	// 物品的设备相关数据
	if( !create_devdepObj() )
		return FALSE;

	//! River @ 2009-9-15:把相应的内容数据，整理到相应的渲染数据结构内。
	if( !m_sMapTerrain.process_hardTex( &m_vecChunkHardTile[0] ) )
		osassertex( false,"处理硬地表纹理失败...\n" );

	// 初始化ivoMgr
	m_ivoMgr.init_ivoMgr( m_iTGObjectNum );

	m_bInuse = TRUE;
	set_mtQueueState( false );

	return TRUE;

	unguard;
}


