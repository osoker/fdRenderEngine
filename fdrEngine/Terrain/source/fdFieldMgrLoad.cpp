//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrLoad.cpp
 *
 *  His:      River created @ 2004-1-8
 *
 *  Des:      ���ڵ����ͼ���ӵ��ࡣ
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
//! �Ƿ���Ⱦ�ر��ϸ������.
OSENGINE_API BOOL     g_bUseTileDetailTex = true;

//! ˮ�浹Ӱ����.0���.2:���.
OSENGINE_API int      g_iReflectionGrade = 0;

//! �Ƿ�ʹ�ó����еĶ������塣
OSENGINE_API BOOL     g_bUseAnimationObj = true;


//! ��ǰ���������еĳ��������֡�
OSENGINE_API char     g_szMapName[16];


# define  DEFAULT_MOVESKY      "dfault_sky"
# define  DEFAULT_BKSKY        "keyani\\default_sky.osa"
# define  DEFAULT_BKTERR       "keyani\\default_terr.osa"
//# define  DEFAULT_BKSKY3        "keyani\\default_sky.osa"

UPDATE_LOADINGSTEPCALLBACKIMM osc_TGManager::m_pUpdate_uiLoadingStep = 0;

//! ע��ص�
void osc_TGManager::register_updateLoadingStepCallbackFunc( UPDATE_LOADINGSTEPCALLBACKIMM _pFunc )
{
	m_pUpdate_uiLoadingStep = _pFunc;
}


//! �ϲ��������Ч��ش���
DWORD  osc_TGManager::create_effect( const char* _ename,osVec3D& _pos,
					float _angle/* = 0.0f*/,BOOL _instance/* = TRUE*/,
					float _scale/* = 1.0f*/,
					float* _effectPlaytime/* = NULL*/  )
{
	guard;

	// �˴�Ϊ�ϲ���Ч�Ľӿڣ��ϲ㴴������Ч�������ǳ������õ��ĳ�����Ч��
	return m_sEffectMgr->create_sEffect( _ename,
		_pos,_angle,false,_instance,_scale,_effectPlaytime );

	unguard;
}

//! �õ�effect��Ӧ��boundingBox,����boxsize�Ǿ���scale���size.��box����ת
bool osc_TGManager::get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize )
{
	return m_sEffectMgr->get_effectBox( _id,_pos,_boxSize );
}

//! �õ�effect�İ�Χ�У�����ת����İ�Χ��.
const os_bbox* osc_TGManager::get_effectBox( DWORD _id )
{
	return m_sEffectMgr->get_effectBox( _id );
}



//! �ڲ�ʹ�õĽӿ�,���ڴ��������ڵ���Ч,�����ڵ���Ч,��������̬��.
DWORD osc_TGManager::create_effect( const char* _ename,
	osVec3D& _pos,float _angle,float _scale,bool _inScene )
{
	return m_sEffectMgr->create_sEffect( _ename,_pos,_angle,_inScene,true,_scale );
}

//! River @ 2010-1-14:�ϲ���Ե�һ��effect��keyTime.
float osc_TGManager::get_effectKeyTime( DWORD _id )
{
	return m_sEffectMgr->get_effectKeyTime( _id );
}


//! ���ϲ�ɾ��һ����Ч
void osc_TGManager::delete_effect( DWORD _id,bool _finalRelease/* = false*/ )
{
	m_sEffectMgr->delete_effect( _id,_finalRelease );
}

//! ����Ч��������,���ڶԳ����ڵĵ��߽������Ŷ������������Ž�����osa�����͹���壬���ӳ������ܶ�̬����
void osc_TGManager::scale_effect( int _id,float _rate,float _time )
{
	m_sEffectMgr->scale_effect( _id,_rate,_time );
}

//! River @ 2010-8-24:����Ч�Ĳ����ٶȽ������š�
void osc_TGManager::scale_effectPlaySpeed( int _id,float _speedRate )
{
	m_sEffectMgr->scale_effectPlaySpeed( _id,_speedRate );
}


//! ���ػ���ʾ��ǰid��effect
void osc_TGManager::hide_effect(int _id,bool _bHide/* = TRUE*/ )
{
	m_sEffectMgr->hide_effect( _id,_bHide );
}


/** \brief
*  ������Ч��alphaֵ,ֻ��osa��Ч������ȷ������alphaֵ��alphaֵ����0.0��1.0f֮�䡣
*
*  \param int _id   ��Ч��Ӧ��id.
*  \param float _alpha ��ЧҪʹ��alphaֵ
*/
void osc_TGManager::set_effectAlpha( int _id,float _alpha )
{
	m_sEffectMgr->set_effectAlpha( _id,_alpha );
}


//! ����Ч����fadeOut
void osc_TGManager::fade_effect( int _id,float _fadetime,bool _fadeOut/* = true*/ )
{
	m_sEffectMgr->fade_effect( _id,_fadetime,_fadeOut );
}

//! �ϲ�������Ч��λ�ú���ת����
void osc_TGManager::set_effectPos( DWORD _id,osVec3D& _pos,
    float _angle,float _speed/* = 1.0f*/,bool _forceUpdate/* = false*/ )
{
	m_sEffectMgr->set_effectPos( _id,_pos,_angle,_speed,_forceUpdate );
}

//@{
//  �ͻ����õ����ض����ݣ�����ֻ������벢���ϲ���ã������ڲ�������
//! �õ���Ч���ŵ�λ������ 0:��Χ�б������š� 1:����λ�ò��š�2:������λ�ò��š�
DWORD osc_TGManager::get_effectPlayPosType( int _id )
{
	return m_sEffectMgr->get_effectPlayPosType( _id );
}
//! �õ���Ч���ŵİ�Χ�б��������ص�ֵ��0.0��1.0֮�䡣
float osc_TGManager::get_effectPlayBoxScale( int _id )
{
	return m_sEffectMgr->get_effectPlayBoxScale( _id );
}
//@}


//! ��֤һ��effect�Ƿ��Ѿ����ڲ���ɾ��
BOOL osc_TGManager::validate_effectId( DWORD _id )
{
	return m_sEffectMgr->validate_effectId( _id );
}


// ת�������ļ����ֵĺ�������!��ex�ļ���ת��Ϊwav�����ļ���
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
	// ���������б�
	OSSoundItem* pSoundItem = (OSSoundItem*)(t_fstart + pHeader->mdwOfsSoundItem);

	for (DWORD i = 0 ; i < pHeader->mdwNrSoundItem;i++)
	{
		// 
		// River mod @ 2008-3-18:�ȴ����Ӧ�������ļ������⣬��ͼ�༭���洢����ex�ļ�
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
			osassertex(false,"��ɾ����ͼ�ļ����µ�SND�ļ�,���ļ��İ屾̫��!");
		}
		if (equal(box.m_vecMax.x,0)||equal(box.m_vecMax.y,0)||equal(box.m_vecMax.z,0))
		{
			osassertex(false,"��ɾ����ͼ�ļ����µ�SND�ļ�,���ļ��İ屾̫��!");
		}
		box.recalc_aabbox();
		m_TGSoundAABBoxList.push_back(box);
	}	
	
	END_USEGBUF( t_iGBufIdx );
	
	//���������볡�������������û����������������
	//! ��Ϊ������һ�����ʱû�н����������Ĳ�����Χ��
	mSoundQuadTree.Compiler(m_TGSoundList,m_TGSoundAABBoxList);

	return true;

	unguard;
}


# if __QTANG_VERSION__
//! ���ϲ����õ�ͼ����ײ��Ϣ,��ʵ��Щ������ȫ�����ɿͻ�����ɣ�Ϊ������ǰ�Ľӿڡ�
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
*  ʹ�����ǵĳ�������ṹ���볡����ͼ��
*
*/
bool osc_TGManager::load_scene( os_sceneLoadStruct* _sl )
{
	guard;


# if __QTANG_VERSION__
	// River mod @ 2010-9-2:ȷ�ϴ�ֵΪ�գ������ϲ���ܷ�����ָ���bug.
	m_pMapCollInfo = NULL;
# endif 

	// syq 5-20 add
	if( osc_TGManager::m_pUpdate_uiLoadingStep )
		(*osc_TGManager::m_pUpdate_uiLoadingStep)(0,0);

	osassert( _sl );

	//! ����ˮ��߶�ʱ�� snowwin add
	osc_newTileGround::m_vecWaterHeight.clear();

	// River @ 2007-6-2: �����ڵ����ϰ汾��
	osc_newTileGround::m_bMapVersionChange = TRUE;

	// 
	// ����ȫ�ֵĵ�ͼ��Ϣ����Ҫ��Ϣ�����������ط����õ�g_szMapName����
	strcpy( g_szMapName,_sl->m_szMapName );
	strcat( g_szMapName,"\\" );


	m_strMapName = SCENEMAP_DIR;
	m_strMapName += _sl->m_szMapName;
	m_strMapName += "\\";

	// ��ȡ���Դ�ļ�
	std::string t_lightFile = m_strMapName ;
	t_lightFile += "lightPos";
	g_ptrLightMgr->ReadFromFile(t_lightFile.c_str(),OutputDebugString);

	// River @ 2009-2-26:
	g_ptrMeshMgr->sceneChange_reset();


	//! �����ͼ��������Ϣ.
	if( !load_sceneInfoFile( _sl->m_szMapName ) )
	{
		osassertex( false,"��ͼĿ¼������*.lst�ļ�....\n" );
		return false;
	}

	// River mod :�����ڵ�������Ϣ��������ͳһ�Ĵ���
	if (!load_SoundInfo(_sl->m_szMapName))
	{
		char msg[256];
		sprintf(msg,"��ͼ[%s]�����ڳ�����Ч�ļ�!\n",_sl->m_szMapName );
		osDebugOut(msg);
	}


	if( !m_ptrTerrainMgr->load_scene( _sl,m_iMaxXTgNum,m_iMaxZTgNum ) )
		osassertex( false ,"���̳߳�ʼ����ͼ����...\n" );

	//! �õ���ǰ�ļ���tg�б�.
	memset( m_arrPtrTG,NULL,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );
	m_iActMapNum = m_ptrTerrainMgr->
		init_getActMapList( m_arrPtrTG,MAXTG_INTGMGR );

	// ���õ�ͼ�õ���ȫ�ֹ����ݡ�
	reset_ambientCoeff();


	// ��ǰ�����Ѿ������˵�ͼ��
	m_bLoadedMap = true;

	return true;
	
	unguard;
}





//! ��һ��characterָ���ƽ���ǽ����Ⱦ������
void osc_TGManager::push_behindWallRenderChar( I_skinMeshObj* _ptr )
{
	m_skinMeshMgr->push_behindWallChar( (osc_skinMesh*)_ptr );
}

//! ��һ��characterָ���ǽ����Ⱦ������ɾ��
void osc_TGManager::delete_behindWallRenderChar( I_skinMeshObj* _ptr )
{
	m_skinMeshMgr->delete_behindWallChar( (osc_skinMesh*)_ptr );
}


/** \brief
*  ���ڴ����Ͳ����ϲ���Ҫ��SkinMeshObject.
*
*  \param _smi ���ڳ�ʼ���´�����skinMeshObject.
*  \return     �������ʧ�ܣ�����NULL.
*/
I_skinMeshObj* osc_TGManager::create_character( 
	os_skinMeshInit* _smi,bool _sceneSm/* = false*/,bool _waitForFinish/* = false*/ )
{
	return m_skinMeshMgr->create_character( _smi,_sceneSm,_waitForFinish );
}



/** \brief
*  �õ���ǰ�����е�skinMesh�õ��������ε���Ŀ�� 
*
*  skinMesh��Ҫ�����������Ⱦ�����Դ˴��õ���ֵ��Ϊ�������Ƭ��Ŀ��
*/
int osc_TGManager::get_skinMeshTriNum( void )
{
	return m_skinMeshMgr->get_smMgrFaceNum();
}



/** \brief
*  ��һ�ε��볡����ʱ���ʼ��TGManager�еı�����
*/
bool osc_TGManager::init_sceneWork( void )
{
	guard;


	// ����Tg ptr����Ҫ���ڴ�
	m_arrPtrTG = new osc_TileGroundPtr[MAXTG_INTGMGR];


	m_deviceMgr = ::get_deviceManagerPtr();
	if( !m_middlePipe.init_middlePipe( m_deviceMgr ) )
		return false;

	if( !m_skinMeshMgr->init_smManager( &m_middlePipe ) )
		return false;

	m_bbMgr->init_bbManager( &m_middlePipe );
	m_boltMgr->init_boltMgr( &m_middlePipe );
	m_dlightMgr.init_dlightMgr( &m_middlePipe );


	// ��ʼ����ա��������Ⱦ���Σ��򲻱س�ʼ��
	// ATTENTION TO FIX:������ʼ��ʱ,Ӧ����Ĭ�ϵ��������.
	//! ��Щ��Ĭ�ϵ���պ�Զ������.
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


	//! ��ʼ�����ӳ�����������
	m_particleMgr.init_particleMgr( &m_middlePipe ); 

	//! ��ʼ��������ص����ݡ�
	m_effectRain.init_rainData( &m_middlePipe );
	m_effectSnow.init_snowStormMgr(&m_middlePipe);


	// ��ʼ���������ι�����,�����˵�ǰ�ɼ���TileGround.
	m_ptrTerrainMgr->init_tgLRMgr( this );

	m_ptrDstr = new os_stringDisp[MAX_STRDIS_PERCI];
	//@{
	// Windy mod @ 2005-9-20 10:40:34
	//Desc: ��ʼ��Lens Flare
#ifdef USE_LENSFLARE
	///����Ĭ��λ��
	m_pLensflare = new os_Lensflare;
	osVec3D lfpos(3000,1320,1000);
	m_pLensflare->setPosition(lfpos);
	m_pLensflare->init();
	m_pLensflare->setBaseAlpha(1.0);
#endif
	//@}

	// Ϊdevice Reset�����м�ܵ���ָ��.
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


	//! River mod @ 2008-12-15:����һ����refMap��Ӧ��depth stencil buffer.
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
*  �õ��µ�ͼ�����֡�
*
*  \param _nMname ������ļ�����Ҳʹ������ַ���ָ�뷵��ֵ��
*  \param _idr ����8�����򣬴�0(Y��)��ʼ,˳ʱ�뷽��: y,xy,x,x-y,  -y,-x-y,-x,-xy
*/
bool osc_TGManager::get_newMapName( char* _nMname,int _dir )
{
	guard;

	int       t_iNx,t_iNy;
	char      t_szName[32];
	bool      t_bRes = true;

	// �õ��µ�ͼ������
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

	osassertex( t_bRes,va("Ҫ�õ��ĵ�ͼ����Ϊ:<%s>,����Ϊ:<%d>..\n",_nMname,_dir) );

	sprintf( t_szName,"%02d%02d.map",t_iNx,t_iNy );
	strcat( _nMname,t_szName );

	return t_bRes;

	unguard;
}







/** \brief
*  ��һ������õ���ǰTGMgr�е�ǰ�����osc_TileGroundPtr.
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

		osDebugOut( "�������еĵ�ͼ����<%d>Ϊ��<%d,%d>..\n",
			t_i,m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX,
			m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ );
	}
# endif 


	return NULL;
}
















