//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldLoad.cpp
 *
 *  His:      River created @ 2005-11-28
 *            @ 2006-1-11:�޸Ĵ��ļ�,ʹ��Ϊ����geo�ļ���ʽ�ĵ��뺯��.
 *
 *  Des:      �����µĵ�ͼ��ʽ����غ���
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
*  �ͷŵ�ǰ��ͼ����Դ.
*/
BOOL osc_newTileGround::release_TG( void )
{
	guard;

	if( !m_bInuse )
		return true;

	// �ͷ�ʹ�ù��������Ķ�̬����Ʒ.
	relase_tgObj();
	m_bInuse = false;

	return TRUE;

	unguard;
}


//! �ͷŵ�ͼ����Ʒ��
BOOL osc_newTileGround::relase_tgObj( void )
{
	guard;

	os_newTGObject*     t_ptrObj = NULL;

	// �ͷŹ������Ʒ��
	// �ͷ�osa������Ʒ��
	for( int t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];
		if( (t_ptrObj->m_dwType ==1)&&(g_bUseAnimationObj) )
		{
			// ����ȫ���Ƿ�ʹ����sm,���ͷš�
			((I_skinMeshObj*)m_vecObjectDevIdx[t_i])->release_character( TRUE );
		}

		// 
		// �ͷų����е���ͨ��Ʒ
		// 1:����Ϊ��ͨ��Ʒ.2:sm��Ʒ������ʹ��sm������3�����봴�����豸��ص����ݡ�
		if(  (t_ptrObj->m_dwType == 0)||
			 ((t_ptrObj->m_dwType==1)&&(!g_bUseAnimationObj) )  )
		{
			continue;
		}

		//
		// �ͷ�ˮ����Ʒ��
		if( (t_ptrObj->m_dwType == 2 ) )
		{
			// ʹ��osa��ˮ,��ˮ��Ϊosa�ļ�.
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->delete_animationMesh( m_vecObjectDevIdx[t_i],true );
		}


		// �ͷŹ������Ʒ
		if( (t_ptrObj->m_dwType == 3 ) )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->release_billBoard( m_vecObjectDevIdx[t_i] );
		}

		// �ͷ�osa������Ʒ��
		if( t_ptrObj->m_dwType == 4 )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->delete_animationMesh( m_vecObjectDevIdx[t_i],true );
		}

		// �ͷ�������Ʒ��
		if( t_ptrObj->m_dwType == 5 )
		{
			if( m_vecObjectDevIdx[t_i]>=0 )
				m_ptrTgMgr->delete_particle( m_vecObjectDevIdx[t_i] );
		}

		// �ͷų����ڵ���Ч
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


//! �õ�ÿһ����Ʒ�Ƿ�Ϊbridge��Ʒ
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

//! ȫ�ֵĵ������
extern int g_iLoadProgress;

/** \brief
*  �����ͼ�����еľ�̬��Ʒ
*/
void osc_newTileGround::load_tgObj( void )
{
	guard;

	int               t_i;
	os_newTGObject*   t_ptrObj;
	BOOL             t_bRelplace;
	os_bbox           t_sBox;
	int              t_iLastUpdate = 0;

	// ÿһ����̬��obj����Ӧһ��AABBox.
	m_vecObjAabbox.resize( m_iTGObjectNum );
		
	for( t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		t_ptrObj = &m_vecTGObject[t_i];

		if(t_ptrObj->m_dwType == 9){
			// ���Դ ��Ʒ
			g_ptrLightMgr->AddPointLight(t_ptrObj->m_vec3ObjPos);
			continue;
		}

		//
		// ���������ʹ�ù���������Ʒ��������ĵط�������
		if( g_bUseAnimationObj )
		{
			if( t_ptrObj->m_dwType != 0 )
				continue;
		}
		
		// ˮ���,�����,��Ч��Ʒ,�ݲ�����.
		if( (t_ptrObj->m_dwType == 2)||
			(t_ptrObj->m_dwType == 3)||
			(t_ptrObj->m_dwType == 4)||
			(t_ptrObj->m_dwType == 5)||
			(t_ptrObj->m_dwType == 6) ) // windy add 5-20
			continue;

		m_vecObjectDevIdx[t_i] = 
			g_ptrMeshMgr->creat_meshFromFile(t_ptrObj->m_szName,t_bRelplace );

		osassertex( (m_vecObjectDevIdx[t_i] != -1),
			va( "������Ʒ<%s>����...\n",t_ptrObj->m_szName ) );
		osassertex( !t_bRelplace,"��ͼ��û�п��õ���Ʒ�����ռ�....\n" );

		// ������Ʒ��aaBBox
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
*  ����������Ʒ��ص����ݡ�
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
		// ���typeΪ4,����keyAniObject.
		if( t_ptrObj->m_dwType !=4 )
			continue;

		//
		// �õ�keyAni���ļ�����
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
			va( "������Ч�����ļ�<%s>����",t_str3.c_str() ) );

		t_iKeyNum ++;

		// 
		// ȷ��ÿһ��С��ͼ���ʹ�õ�osa��Ʒ������
		osassert( t_iKeyNum < MAX_OSANUM_PERTG );
	}

	osDebugOut( "The map <%s>'s keyAniObj num is:<%d>..\n",
		m_strMapFName.c_str(),t_iKeyNum );

	return TRUE;

	unguard;
}


/** \brief
*  �Ե�ͼ��billBoard��Ʒ���д�����
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
		// �õ�billBoard���ļ�����
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
			va( "����������ļ�<%s>����",t_str3.c_str() ) );
	}

	return true;

	unguard;
}


/** \brief
*  ����TileGround�е�Water�����Ʒ��
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

		// 2Ϊˮ�����͡�
		if( t_ptrObj->m_dwType !=2 )
			continue;

		// �õ�keyAni���ļ�����
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

		//River added @ 2008-12-5:����ȫ������ˮ��Ч��
		//!wxd add ���ˮ�����ȡ�������������ˮ��߶ȵģ���������Ե͸��������
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

		osassertex( (m_vecObjectDevIdx[t_i]>=0), va( "����<%s>��Ʒʧ��..",t_mc.m_szAMName ) );
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

		// 8Ϊ�����͡�
		if( t_ptrObj->m_dwType != 8 )
			continue;

		// �õ�keyAni���ļ�����
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

		//River added @ 2008-12-5:����ȫ������ˮ��Ч��
		//m_ptrTgMgr->m_sSceneTexData.m_fWaterHeight = t_mc.m_vec3Pos.y;	
		t_mc.m_bFogAni = true;
		g_bVolumeFogInited = TRUE;

		m_vecObjectDevIdx[t_i] = m_ptrTgMgr->create_keyAniMesh( t_mc );

		osassertex( (m_vecObjectDevIdx[t_i]>=0), va( "����<%s>��Ʒʧ��..",t_mc.m_szAMName ) );
	}
	return true;
	unguard;
}

/** \brief
*  ����������������ص���Ʒ��
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
		
		// 5Ϊ���ӵ����͡�
		if( t_ptrObj->m_dwType !=5 )
			continue;

		//
		// �õ�Ҫ�������ӵ��ļ�����
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
			va( "��������<%s>ʧ��...",t_pCreate.m_szParticleName ) );

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

	DWORD	mID;		//!< �ļ�ID
	DWORD	mVersion;	//!< �汾��Ϣ

	DWORD	mHeadPicFileNameOfs;	//!< ����ͷ���ļ���ƫ��,Char����
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
	//! River added @ 2007-6-4:������﹥���Ŀ�ʼλ��
	osVec3D	mStartPos;


	BOOL		mbIsRide;	//!< �Ƿ�������
	osVec3D	mRidePos;	//!< ����΢����

	DWORD	mEffectOfs;
	DWORD	mEffectNr;
	float	mScale;
	DWORD	mReserved4;
};


// ���������Ч 
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

	// River added @ 2007-6-5:��������cha�汾����֤:
	osassertex( m_header.mVersion >= 0x109,
		va( "<%s>�ļ��������µ�109�汾...\n",_filename ) );

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

	// �����Ƿ��а������Ч
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
//! ��osa�ļ����뵽��Ӧ��smInit�ṹ�ڡ�
BOOL osc_newTileGround::load_smInitData( const char* _sz,
				os_skinMeshInit& _smInit,os_newTGObject*  _ptrObj )
{
	guard;


	s_string    t_szFname;

	// ���ȵ���*.cha�ļ���
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
*  ������ͼ��ʹ��skinMesh���ж��������塣
*
*  ���ʹ��skinMesh������д������������������Ϊ1.
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
		// �õ�skinMeshAinmation���ļ�����
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = SMAINOBJ_DIR;
		t_str3 += t_str1;

		// 
		// ��ȡbfs�ļ����õ���ǰskinMesh��part��Ŀ��
		load_smInitData( t_str3.c_str(),t_smInit,t_ptrObj );

		t_smInit.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		t_smInit.m_bWriteZbuf = false;

		// 
		// ת��ΪDWORDֵ�������豸������ݡ�
		m_vecObjectDevIdx[t_i] = 
			(DWORD)(m_ptrTgMgr->create_character( &t_smInit,true ));
		osassertex( m_vecObjectDevIdx[t_i],
			va( "����sm��Ʒ<%s>ʧ��...",t_str3.c_str() ) );

		t_iAniNum ++;

# ifdef _DEBUG
		if( t_iAniNum >= MAX_SMOBJPERMAP )
		{
			char   t_str[128];

			sprintf( t_str,
				"The <%s> map ������64��skinMesh��Ʒ..\n",m_strMapFName.c_str() );
			MessageBox( NULL,t_str,"Error",MB_OK );
			osassert( false );
		}
# endif 

	}

	osDebugOut( "��ǰ�����д�����< %d >����������\n",t_iAniNum );

	return true;

	unguard;
}

/** \brief
*  ���������ڵ�������Ч��Ʒ����spe��Ʒ
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

		// �õ�spe�ļ����ļ���  SCEEFFECT_DIR
		t_str1 = t_ptrObj->m_szName;
		t_idx = (int)t_str1.rfind( '.' );
		t_str2 = t_str1.substr( 0,t_idx );
		t_idx = (int)t_str2.rfind( '\\' );
		t_str1 = t_str2.substr( t_idx+1,t_str2.length() );
		t_str3 = SCEEFFECT_DIR;
		t_str3 += t_str1;
		t_str3 += ".spe";

		// River @ 2009-5-4:ʹ����������������������Ч��Ʒ
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


//! ������tg�����е���Ʒ���豸�������
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
	// ��������ÿһ����̬����Ʒ
	load_tgObj();

	// ȷ��ÿһ��obj�Ƿ�Ϊbride Obj.
	process_objIsBridge();


	//
	// ���������еĶ�����Ʒ��
	if( g_bUseAnimationObj )
	{
		if( !create_smAniObj() )
			return FALSE;
	}

	// 
	//  ����billBoard��Ʒ.
	if( !create_billBoardObj() )
		return FALSE;

	//������keyAnimationMesh��Ʒ��
	if( !creaet_keyAniObj() )
		return FALSE;

	//
	// ����ˮ��ص���Ʒ��
	if( !this->create_waterObj() )
		return FALSE;

	// ��������ص���Ʒ��
	if( !this->create_FogObj() )
		return FALSE;
	

	//
	// ����������������ص���Ʒ��
	if( !this->create_particleObj() )
		return FALSE; 

	// ���������ڵ�������Ч��Ʒ
	if( !this->create_effectObj() ) 
		return FALSE;

	return TRUE;

	unguard;
}

//! �����Ĵ���һ��objMesh.
BOOL osc_newTileGround::create_meshImmediate( int _idx )
{
	guard;

	osassert( _idx < m_iTGObjectNum );

	BOOL   t_b = false;
	// ������Ʒ��id.
	m_vecObjectDevIdx[_idx ] = 
		g_ptrMeshMgr->creat_meshFromFile( m_vecTGObject[_idx].m_szName,t_b );

	return TRUE;

	unguard;
}

/** ���ݵ�ͼ��ȫ��ͼ·����,�õ���ͼ��
*	��:_szfullname map\vilg\vilg0000.geo,
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
		osassertex( false,va( "������ļ���<%s>...\n",_szfullname ) );

	unguard;
}

//! ���뵱ǰtileGround�ڵ�Ӳ�ر����ݡ�
void osc_newTileGround::load_hardTileData( const BYTE* _fstart )
{
	guard;

	osassert( _fstart );

	int   t_iSize = 0;
	int   t_i;

	m_vecChunkHardTile.resize( CHUNKNUM_PERTG );
	for( t_i= 0;t_i<CHUNKNUM_PERTG;t_i ++ )
		m_vecChunkHardTile[t_i].reset();

	// �汾�ŵ���17,ֱ�ӷ���
	if( m_sTgHeader.m_dwVersion<17 )
		return;

	BYTE*  t_bStart = (BYTE*)(_fstart + m_sTgHeader.m_dwHardTexSizeOffset);
	for( t_i=0;t_i<CHUNKNUM_PERTG;t_i ++ )
	{
		READ_MEM_OFF( &m_vecChunkHardTile[t_i].m_iHardTileNum,t_bStart,sizeof( int ) );

		if( m_vecChunkHardTile[t_i].m_iHardTileNum == 0 )
			continue;

		osassertex( m_vecChunkHardTile[t_i].m_iHardTileNum< 10000,
			va("�Ƿ���Ӳ��������<%d>..\n",m_vecChunkHardTile[t_i].m_iHardTileNum) );

		m_vecChunkHardTile[t_i].m_arrTileInChunk = 
			new osc_hardTextureTile[m_vecChunkHardTile[t_i].m_iHardTileNum];
		
		
		for( int t_j = 0;t_j<m_vecChunkHardTile[t_i].m_iHardTileNum;t_j ++ )
		{
			osc_hardTextureTile*  t_ptrTT = &m_vecChunkHardTile[t_i].m_arrTileInChunk[t_j];
			READ_MEM_OFF( &t_ptrTT->m_iTileNum,t_bStart,sizeof( int ) );
			t_ptrTT->m_vecTileInChunk.resize( t_ptrTT->m_iTileNum );
			READ_MEM_OFF( &t_ptrTT->m_vecTileInChunk[0],
				t_bStart,sizeof( osc_tileInChunk )*t_ptrTT->m_iTileNum );

			// ��Ӧ���������ֺ�uv����
			READ_MEM_OFF( t_ptrTT->m_szTexName,t_bStart,sizeof( char )*32 );
			READ_MEM_OFF( &t_ptrTT->m_wU,t_bStart,sizeof( t_ptrTT->m_wU ) );
			READ_MEM_OFF( &t_ptrTT->m_wV,t_bStart,sizeof( t_ptrTT->m_wV ) );
			READ_MEM_OFF( &t_ptrTT->m_rotation,t_bStart,sizeof( t_ptrTT->m_rotation) );
		}
	}

	//! ����
	return;

	unguard;
}


//! ȫ�ֵ����ϰ汾��
MapVersionHead osc_newTileGround::m_sMapVersionHead;
BOOL osc_newTileGround::m_bMapVersionChange = TRUE;


/** \brief
*  ���ڴ��е���һ����ͼ�ļ���
*
*  \param _fload  �Ƿ�����Ϸ��ʼ��ʱ��һ�ε����ͼ��
*/
BOOL osc_newTileGround::load_TGData( const char* _fname,
					const osc_middlePipe* _mpipe,bool _fload/* = false*/ )
{
	guard;
	s_string  mapname;
	
	getmapname(_fname,mapname);

	// �ȵ�������
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
		osassertex(false,"��ͼ�汾�Ź���,��ת�������°屾");
	}

# if __QTANG_VERSION__
	//! River added @ 2009-9-14:�Ͷ˵İ汾��ȥ��һЩ����
	if( m_sTgHeader.m_dwVersion<16 )
	{
		osassertex( false,"��ͼ�汾�Ź���,��ת�������16�屾");
	}
# endif 


# if !__QTANG_VERSION__
	// River @ 2007-6-2:�������ϰ汾����Ϣ
	if( m_sTgHeader.m_dwVersion >= 15 )
	{
		MapVersionHead  t_sMapVersionHead;
		memcpy( &t_sMapVersionHead, t_fstart+sizeof(os_newFileHeader),
			sizeof(MapVersionHead) );
	}
# endif 

	// ���õ������ݽӿ�,�����������
	if( !m_sMapTerrain.LoadMapTile( t_fstart,mapname.c_str() ) )
	{
		END_USEGBUF( t_iGBufIdx );
		return FALSE;
	}
	memset(m_vecUpmostTexIdx,0,sizeof(BYTE)*GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT);
	memcpy(m_vecUpmostTexIdx,t_fstart + m_sTgHeader.m_dwUpmostTexIdxOffset,
		sizeof(BYTE)*GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT);
	
	// ��ײ��Ϣ.
	memcpy( m_arrCollInfo,t_fstart + m_sTgHeader.m_dwColHeiInfoOffset,
		sizeof( os_TileCollisionInfo )*GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT );

	// ÿһ�����ӵ�������Ϣ
	m_vecTileAttribute.resize( MAX_GROUNDWIDTH*MAX_GROUNDWIDTH );
	memcpy( &m_vecTileAttribute[0],t_fstart + m_sTgHeader.m_dwTileAttributeOffset,
		sizeof( BYTE )*MAX_GROUNDWIDTH*MAX_GROUNDWIDTH );

	//! ����obj��ص�����
	m_iTGObjectNum = m_sTgHeader.m_dwObjNum;
	m_vecTGObject.resize( m_iTGObjectNum );
	m_vecTGObjDL.resize( m_iTGObjectNum );

# if !__QTANG_VERSION__
	// 13��ǰ�İ汾û����Ʒ�ĵر����ɫ��Ϣ
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
			// Ĭ��ʹ��1.0�����ű�׼��
			m_vecTGObject[t_idx].m_vec3ObjectScale = osVec3D( 1.0f,1.0f,1.0f );

		}
	}
	// River mod @ 2009-4-16:������Ʒ��������Ϣ����Ҫ����汾13
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
			// Ĭ��ʹ��1.0�����ű�׼��
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
	// ����ÿһ�����ӵĶ���߶���Ϣ
	m_vecHeight.resize( (MAX_GROUNDWIDTH+1)*(MAX_GROUNDWIDTH+1) );
	memcpy( &m_vecHeight[0],t_fstart + m_sTgHeader.m_dwHeightInfoOffset,
		sizeof( float )*(MAX_GROUNDWIDTH+1)*(MAX_GROUNDWIDTH+1) );
	
	// �����Ĳ�������
	if( !m_sQuadTree.load_quadtreeDataFromMem( this,t_fstart ) )
	{
		END_USEGBUF( t_iGBufIdx );
		return FALSE;
	}

	//! River added @ 2009-9-14:����Ӳ������ʾ��
	load_hardTileData( t_fstart );

	END_USEGBUF( t_iGBufIdx );


	//
	// River mod @ 2007-1-5:�����޸ĺ�ĵ�ͼ��ײ����,�����յİ汾�У���ȥ����ccl���ݣ���
	// ���������ײ���ݼ��뵽��ͼ���ļ���.
# if __COLHEI_MODIFY__

	osassert( m_ptrTgMgr );
	m_ptrTgMgr->load_colheiAdjData( _fname,m_arrCollInfo );

# endif 
	
	// �����豸��ص����ݵ���
	if( !m_sMapTerrain.InitDeviceData() )
		return FALSE;

	// ��Ʒ���豸�������
	if( !create_devdepObj() )
		return FALSE;

	//! River @ 2009-9-15:����Ӧ���������ݣ�������Ӧ����Ⱦ���ݽṹ�ڡ�
	if( !m_sMapTerrain.process_hardTex( &m_vecChunkHardTile[0] ) )
		osassertex( false,"����Ӳ�ر�����ʧ��...\n" );

	// ��ʼ��ivoMgr
	m_ivoMgr.init_ivoMgr( m_iTGObjectNum );

	m_bInuse = TRUE;
	set_mtQueueState( false );

	return TRUE;

	unguard;
}


