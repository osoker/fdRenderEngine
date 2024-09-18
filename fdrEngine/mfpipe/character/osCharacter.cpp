///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacter.cpp
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     ����os�����е����ﶯ�����á�
 * 
 *  "Ů����ʴ�������������ˬ�����ԣ�����������������E������־ɱ��"
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/deviceInit.h"
# include "../include/frontPipe.h"
# include "../../interface/stringTable.h"

//! ȫ���õ���skinMethod.
OSENGINE_API ose_skinMethod            g_eSkinMethod = OSE_D3DINDEXEDVS;

osc_skinMesh*   osc_skinMeshMgr::m_ptrShadowSkinPtr[MAX_RTSHADOWNUM];


//! ���峡����������ʹ��skinMesh��Ŀ,�ڸ��������64��skinMesh��Ʒ
int  MAX_SCENESMNUM = 64*MAXTG_INTGMGR;


//! һ��Frame�����ж��mesh���Ӧ��
# define MAX_MESH_PERFRAME   4

//! ��ʼ���豸�����Դ������С��
# define  INIT_SMDEVDEPSIZE  384

//! ��ʼ��boneSys������BoneAni�����Ĵ�С��
# define  INIT_BONESYSNUM    168

//! ÿ�붯��ʹ�õ�֡����
# define  FPS_PERSECOND      20

//! bodyPartMesh����ĳ�ʼ����С��
# define  INT_BPMSIZE        496


//! ����������ת֮��Ĳ�ֵʱ�䡣
# define ROTINTERPOLATE_TIME 0.2f

//! ��������BBox�Ŀ�Ⱥ͸߶ȡ�
# define CHARACTER_WIDTH    0.4f
# define CHARACTER_HEIGHT   1.8f


//! �Ƿ���ʾ����Ϊ���߿�ģʽ.
OSENGINE_API BOOL         g_bCharWireFrame = FALSE;

//! ������������������豸�����Դ��
osc_smDepdevResMgr*         g_smDevdepMgr = NULL;

//! �����������еĹ���ϵͳ�Ĺ�����ָ�롣
osc_boneSysMgr*           g_boneSysMgr;

//! �����������еĹ����������ݵĹ�����ָ�롣
osc_boneAniMgr*           g_boneAniMgr;


//! �����������岿�ֵĶ������ݵĹ�����ָ�롣
osc_bpmMgr*               g_bpmMgr;


/** 
 *   Osok����Ŀǰ���Ｔʱ��Ӱ��ԭ��:
 * ��1: ���ڴ��б������岿λ���ݵĶ������ݺ��������ݡ�
 *   2: �������ÿһ��ÿһ�����岿λ�Ķ���λ�ã�����
 *      ����������ȷ��ÿһ������������Թ�Դ���Ǳ��Թ�Դ��
 *   3: �������岿λÿһ���߶�Ӧ�����������Դ�Ĺ�ϵ��ȷ��
 *      ��ǰ�ı��Ƿ����������Ӱ�߽硣
 *   4: ������Ӱ�߽�ͼ��������岿λ����λ���������Stencil��Ⱦ
 *      ��Ҫ�������κ�����������Ӧ�Ķ��㻺������
 *   5: ������Ⱦ״̬����Ⱦ������Ӱ��
 *
 */
//! ���������Ƿ���ʾ��Ӱ��Ĭ������²���ʾ��Ӱ��
OSENGINE_API BOOL  g_bHeroShadow = true;


//! ����flipDeterminant��Scale����.
osMatrix osc_skinMesh::m_sFDMatrix;




//! ��ʼ����̬����
template<> osc_skinMeshMgr* ost_Singleton<osc_skinMeshMgr>::m_ptrSingleton = NULL;

/** �õ�һ��sg_timer��Instanceָ��.
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

/** ɾ��һ��sg_timer��Instanceָ��.
*/
void osc_skinMeshMgr::DInstance( void )
{
	SAFE_DELETE( osc_skinMeshMgr::m_ptrSingleton );
}


// 
// ATTENTION TO FIX:
//! �����Ŀ�ʼ֡,��Ϊ�����ĵ���͵�һ֡��ȫ��ͬ,ʹ�õ�һ֡��Ϊ��ʼ֡.
# define ACTIDX_START   1


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      ��ʼ����ϵͳ�����Ĺ����������,����*.BFS�ļ�
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

osc_boneSysMgr::osc_boneSysMgr()
{

	// ��������Ļ����������·��䡣
	this->m_vecBoneSys.resize( INIT_BONESYSNUM );
	this->m_iBoneSysNum = 0;
	g_boneSysMgr = this;

}

osc_boneSysMgr::~osc_boneSysMgr()
{
	StringTableDestroy;
}

/** \brief
*  �ͷŵ�ǰ�Ĺ�����������
*/
void osc_boneSysMgr::release_boneSysMgr( void )
{
	m_vecBoneSys.clear();

	m_vecBoneSys.resize( INIT_BONESYSNUM );
	m_iBoneSysNum = 0;

}



/** \brief
*  �ӹ���ϵͳ�������е���һ�׹�����
*
*  �������ڹ��������Ѿ�������������Ҫ�����BoneSys����ֻ��Ҫ����
*  ָ��Ϳ����ˣ����򴴽�һ���µ�BoneSys,�����ش�����boneSysָ�롣
*  
*/
os_boneSys* osc_boneSysMgr::create_boneSysFromMgr( const char* _fname )
{
	guard;

	int       t_i;
	DWORD     t_dwHash = string_hash( _fname );
	
	for( t_i=0;t_i<this->m_iBoneSysNum;t_i++ )
	{
		// River added @ 2010-12-21:�ӿ��ٶ�
		if( t_dwHash != m_vecBoneSys[t_i].m_dwHash )
			continue;

		if( m_vecBoneSys[t_i].m_strBoneSysName == _fname )
		{
			return &m_vecBoneSys[t_i];
		}
	}

	//
	// ��ǰ��ϵͳ��û�������Դ�������µ�BoneSys��Դ��
	t_i = (int)m_vecBoneSys.size();
	if( m_iBoneSysNum == t_i )
	{
		//osassert( false );
		m_vecBoneSys.resize( t_i*2 );
		return NULL;
	}

	// River @ 2010-12-29:�����ȡ�ٶȣ�ȥ���˶δ���
	// ����ļ������ڣ����ش���
	//if( !file_exist( (char*)_fname ) )
	//	return NULL;

	m_vecBoneSys[m_iBoneSysNum].load_boneSysFromFile( _fname );

	m_iBoneSysNum ++;
	
	return &m_vecBoneSys[m_iBoneSysNum-1];

	unguard;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   �����������������������Ķ�������࣬���еĶ������Ӷ����������еõ�.
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

//! �ͷŵ�ǰ��boneAniMgr.
void osc_boneAniMgr::release_boneAniMgr( void )
{
	m_vecBoneAni.clear();
	//m_vecBoneAni.resize( INIT_BONESYSNUM );
	m_iBoneAniNum = 0;
}



/** \brief
*  ���ļ��д�������ǰ����ϵͳ��ȫ���Ķ������ݡ�
*  
*  �����ǰ�Ĺ��������Ѿ�����������ݣ���������ָ�롣������뵱ǰ���������еĶ�����
*  
*  \return os_boneAin* ������ؿգ��򴴽�ʧ�ܡ�
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
		// River @ 2010-12-23:����hashֵ�ıȽ�
		if( t_dwHash != m_vecBoneAni[t_i].m_dwHash )
			continue;
		if( strcmp( m_vecBoneAni[t_i].m_szAniName,_aniName ) == 0 )
			return &m_vecBoneAni[t_i];
	}

	//
	// ����Ҳ�������Ӳ�̵���
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
//                      ��ʼMesh�������������Ĺ�����������*.SKN�ļ�
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
*  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
*
*  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
*  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
*/
int os_smMeshDevdep::displaytri_bysel( int _triidx[24],int _trinum )
{
	guard;

	// ͷһ�δ����ʱ���ȱ��ݵ�ǰ����������������
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


	// ������������������
	t_hr = m_pIdxBuf->Lock( 0,0,(void**)&t_ptrStart,0);
	osassert( !FAILED( t_hr ) );

	// ����ѡ�������������µĻ���������
	if( _trinum == 0 )
	{
		// ʹ��ԭʼ����������
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
	// TEST CODE: ���Ҫ��ʾ�������ζ�Ӧ�Ķ������ݣ�
	os_characterVer*   t_ptrVerStart;
	t_hr = m_pBuf->Lock( 0,0,(void**)&t_ptrVerStart,0 );

	for( int t_i=0;t_i<_trinum;t_i ++ )
	{
		for( int t_j=0;t_j<3;t_j ++ )
		{
			osDebugOut( "The %d ���� %d ������,����<%d>:<%f,%f,%f><%f,%f,%f>..\n",
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

	// �������ݵ��ڴ档
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf( m_strMFname,t_fstart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
		return false;

	//
	// �����ļ�ͷ.
	char      t_str[4];
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,SKN_FILEMAGIC ) != 0 )
	{
		osassertex( false,va( "<%s> file Format error!!!",m_strMFname ) );
		return false;
	}

	// ATTENTION TO FIX:�汾�ű����������ļ��İ汾�ţ�
	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );

	//
	// �����ļ���Ϣ
	t_fstart += sizeof( DWORD );
	t_fstart += sizeof( DWORD );

	t_fstart += sizeof( DWORD );
	t_fstart += sizeof( DWORD );

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	t_fstart += sizeof( DWORD );

	t_fstart += sizeof( DWORD );
	t_fstart += sizeof( DWORD );


	//
	// ����ÿһ��������matrix.
	t_fstart += sizeof( osMatrix )*t_iSize;
	t_fstart += sizeof( DWORD )*t_iSize;

	//
	// �����豸��ص�����,��������У�����Ҫʹ��ȫ�ֵĻ�����.
	//
	BYTE*   t_ptrDStart = t_fstart;
	WORD*   t_ptrIStart = (WORD*)(t_fstart + m_iVerNum*m_iVerSize);

	//
	// ��������䶥�㻺������
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
	// ���������index buffer.
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
 *  ʹ���ļ���ʼ����ǰ��dev dep resource.
 *
 *  ����ڵ�ǰ��mgr���Ѿ����������ǵ���Դ��������Դid,������Դ
 *  ��ref��Ŀ��һ��
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
	// ���浱ǰdevdep���ݶ�Ӧ���ļ�����
	osassert( strlen( _vi.m_strFname ) < 64 );
	strcpy( m_strMFname,_vi.m_strFname );
	m_dwHash = string_hash( m_strMFname );
	m_iVerSize = _vi.m_iVerSize;
	m_iVerNum = _vi.m_iVerNum;
	m_iIdxNum = _vi.m_iIdxNum;

	//
	// ��������䶥�㻺������
	D3DPOOL    t_dwPool = D3DPOOL_DEFAULT;
	t_hr = _dev->CreateVertexBuffer( 
		m_iVerSize*m_iVerNum,D3DUSAGE_WRITEONLY,
		0, t_dwPool ,&this->m_pBuf,NULL );
	if( FAILED( t_hr ) )
	{
		osassertex( false,va( "����:<%s>,�뵥����Ϸ�������������Ϸ����...\n",
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
	// ���������index buffer.
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
*  �ͷŵ�ǰ��Դ�Ĵ��麯��������ǰ�����ü�����Ϊ0ʱ���Զ��������
*  ������
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

	// River @ 2010-12-21:ʹ�ø�bpm���ݴ�Сһ�������ݣ�ÿһ��bpm���ݶ�Ӧһ��devdep���ݡ�
	//                    ��������ڼ������õ����⡣
	m_vecDevdepRes.resize( INT_BPMSIZE );
}

osc_smDepdevResMgr::~osc_smDepdevResMgr()
{

}


/** \brief
*  ��ʼ����ǰ��manager.
*/
void osc_smDepdevResMgr::init_smDevdepMgr( LPDIRECT3DDEVICE9 _dev )
{
	m_pd3dDevice = _dev;

}


//! �ͷŵ�ǰ��skinMesh Devdep Manager.
void osc_smDepdevResMgr::release_smDevdepMgr( bool _finalRelease/* = true*/ )
{

}




/*** \brief
*  �ͷ�һ��body part mesh���豸������ݡ�
*  
*  \param _id Ҫ�ͷ�mesh��devdep manager�е�id.
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
*  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
*
*  \param int _resId  Ҫ�����resourceId
*  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
*  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
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
*  ��Mesh�ļ������豸��ص����ݡ�
*  
*  \return int -1��ʾ����ʧ�ܡ�
*/
int osc_smDepdevResMgr::create_devdepMesh( os_smDevdepDataInit& _vi )
{
	guard;

	int     t_i,t_iCreateId;

	//
	// �ҵ����Դ����µ��豸�����Դ��id. 
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
		// ��Ϊ���̣߳������ĵط�ʹ���˹����ָ�����ã����Բ�������ռ�
		// ����ܵİ취�������ټ�һ�����������������⡣
		osassertex( false,"������������SM�ռ�...\n" );
	}

	//
	// ʹ�ÿ��õ���Դ�����µ��豸�����Դ��������refֵ��
	if( !m_vecDevdepRes[t_iCreateId].init_smMesh( _vi,m_pd3dDevice ) )
	{
		osassertex( false,"��������meshʧ��...\n" );
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
	//Desc: ride Ĭ�ϲ��ǵڶ�����
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


//  �ͷ�һ�������ʱ�򣬲����ͷŴ��������е����ݣ�Ӳ������ʹ�õ�ʱ�����ͷš�
//! river added @ 2010-12-21:�����ͷ�
void osc_bodyPartMesh::release_bmpRes( void )
{
	if( m_bInuse )
	{
		//
		// �ͷŵ�ǰBPM���豸��ع�������shader�������е����á�
		g_smDevdepMgr->release_devdepMesh( m_iDevdepResId );

	}

	// ����е�����ص����ݡ�����
	if( m_ptrSwordGlossy )
		SAFE_DELETE( m_ptrSwordGlossy );
	//@{
	// Windy mod @ 2005-8-31 9:27:41
	//Desc: ride ����������ͷ�
	if(m_ptrRide)
		SAFE_DELETE(m_ptrRide);
	//@}


	// �ͷŵ�ǰBPM������ڴ档
	m_iDevdepResId = -1;
	m_bInuse = false;

	m_dwHash = -1;

}


/** \brief
*  �ͷŵ�ǰ��Դ�Ĵ��麯��������ǰ�����ü�����Ϊ0ʱ���Զ��������
*  ������
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
		// �ͷŵ�ǰBPM���豸��ع�������shader�������е����á�
		g_smDevdepMgr->release_devdepMesh( m_iDevdepResId );

	}

	// ����е�����ص����ݡ�����
	if( m_ptrSwordGlossy )
		SAFE_DELETE( m_ptrSwordGlossy );
	//@{
	// Windy mod @ 2005-8-31 9:27:41
	//Desc: ride ����������ͷ�
	if(m_ptrRide)
		SAFE_DELETE(m_ptrRide);
	//@}


	// �ͷŵ�ǰBPM������ڴ档
	m_iDevdepResId = -1;
	m_bInuse = false;

	m_dwHash = -1;
# endif 

}



osc_bpmMgr::osc_bpmMgr()
{
	m_vecBPM.resize( INT_BPMSIZE );

	// ȫ��ֻ��һ��bodyPartMesh Manager.
	g_bpmMgr = this;

}

osc_bpmMgr::~osc_bpmMgr()
{

}

//!  �ͷŵ�ǰ��bpmMgr
void osc_bpmMgr::release_bpmMgr( void )
{
	m_vecBPM.clear();

}



//!  ʹ��3d�豸��ʼ����ǰ��bpmMgr.
void osc_bpmMgr::init_bmpMgr( LPDIRECT3DDEVICE9 _dev )
{
	guard;// osc_bpmMgr::init_bmpMgr() );

	osassert( _dev );
	m_pd3dDevice = _dev;

	unguard;
}


//!  ���ļ��е���bpm.
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
	// ����ڴ��д���������Ҫ���ļ������ز��������ü�����
	for( t_i=0;t_i<(int)m_vecBPM.size();t_i++ )
	{
		t_ptr = &m_vecBPM[t_i];
		if( t_ptr->is_inUsed() )
		{
			// River @ 2010-12-21: ��hashֵ�Ƚ�һ�¡�
			if( t_ptr->m_dwHash != t_dwHash )
				continue;

			if( strcmp( t_ptr->m_strFname,_bpName ) == 0 )
			{
				if( _IsRide )
				{
					if( t_ptr->m_ptrRide )
					{
						// �������á�
						t_ptr->add_ref();
					}
					else
					{
						int t_iRef = t_ptr->get_ref();
						for( int t_ref = 0;t_ref < t_iRef;t_ref ++ )
							t_ptr->release_ref();
						//! �����ͷ�����Ӧ����Դ���������
						t_ptr->release_bmpRes();

						// �������Ϊ�����ģʽ����������Ҫ���ģʽ�����µ������� 
						if( !t_ptr->LoadMesh( (char*)_bpName,_IsRide,t_iRef ) )
							return -1;

						t_ptr->add_ref();
					}
				}
				else
					// �������á�
					t_ptr->add_ref();

				return t_i;
			}

		}
	}

	//
	// �ҵ����Դ����µ�mesh�Ŀռ�.
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
	//! ����Ҳ������Դ����Ŀռ䣬��һ��refΪ��Ŀռ䣬�ͷš�
	//  ��ô���ĺô����ڴ���Ի���һЩ�����skinMesh���ݣ���С��Ӳ�̵������ݵ�����
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
	// ����ռ䲻�����ټӴ�
	if( t_idx == -1 )
	{
		// ��Ϊ���̣߳������ĵط�ʹ���˹����ָ�����ã����Բ�������ռ�
		// ����ܵİ취�������ټ�һ�����������������⡣
		osassertex( false,"���Ӹ����bpm�ռ�496...\n" );
	}

	//
	// ���뺯����
	if( !m_vecBPM[t_idx].LoadMesh( (char*)_bpName ,_IsRide) )
		return -1;

	m_vecBPM[t_idx].add_ref();
	return t_idx;

	unguard;
}


//! �ͷ�bpm�����á�
void osc_bpmMgr::release_bpm( int _id )
{
	guard;

	osassert( _id<(int)m_vecBPM.size() );
	if( m_vecBPM[_id].get_ref() > 0 )
		m_vecBPM[_id].release_ref();

	unguard;
}



//
// BodyPartMesh��ʵ�������ڹ���bodyPartMesh.
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

//! river added @ 2010-12-22: ����һ���Ե��ڴ����
CSpaceMgr<osc_swordGlossyIns> osc_bpmInstance::m_vecDGlossyIns;


//! �õ�һ��ʵ������ָ��.
osc_swordGlossyIns* osc_bpmInstance::get_glossyInsPtr( void )
{
	guard;

	m_iNodeIdx = m_vecDGlossyIns.get_freeNode();
	return m_vecDGlossyIns.get_nodePtr( m_iNodeIdx );

	unguard;
}

//! River @ 2010-12-24:Ԥ����������ص�����ָ�롣
CSpaceMgr<osc_ChrRideInst> osc_bpmInstance::m_vecChrRideIns;
//! �õ�һ�������ʵ��������
osc_ChrRideInst* osc_bpmInstance::get_chrRidePtr( void )
{
	guard;

	m_iCRNodeIdx = m_vecChrRideIns.get_freeNode();
	return m_vecChrRideIns.get_nodePtr( m_iCRNodeIdx );

	unguard;
}



/** \brief
*  ʹ�������������bodyPartMesh,osc_skinMesh��ʹ������ӿ�������bodyPartMesh��صĲ�����
*
*  ʹ��bpm��Manager���õ�bodypartMesh��ָ�롣
*
*  \param bool _shadowSupport   �����bmpInstance�Ƿ�֧��������Ӱ��ص����ݡ�
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
	// �������ĵ����������е������ݣ�Ӧ���ڴ˴����õ���Ins���ݡ�
	if( m_ptrBodyPartMesh->m_ptrSwordGlossy )
	{
		if( !m_ptrSGIns )
			m_ptrSGIns = get_glossyInsPtr();

		m_ptrSGIns->set_swordGlossyPtr( m_ptrBodyPartMesh->m_ptrSwordGlossy,_mesh );
		m_ptrSGIns->reset_swordGlossy();

		// River mod @ 2006-6-21:�ϲ�ɿ����Ƿ���ʾ������Ч��
		// �ڵ�����Ӧ�Ľӿ�ʱ���ٴ�����Ч���豸�������
		// ���������õ�����Ч�豸�������
		if( !m_ptrSGIns->create_devdepRes( 0,_playSg,_playWEff,_topLayer ) )
			return false;
	}
	//@{
	// Windy mod @ 2005-8-29 10:56:36
	//Desc: ride 
	if (m_ptrBodyPartMesh->m_ptrRide)
	{
		// River mod @ 2010-12-24:�˴��ڴ��޸�Ϊ�Ƕ�̬���䣬�ӿ��ٶ�
		if (!m_ptrRideIns)
			m_ptrRideIns = get_chrRidePtr();
		m_ptrRideIns->m_ptrRide = m_ptrBodyPartMesh->m_ptrRide;
	}

	//@}

	// 
	// ��ʼ����Ӱ��ص����ݡ�
	if( _shadowSupport )
	{
		// �����Ӳ���϶��룬�˶��㻺����Ϊֻд���㻺��.
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

		// ������Ӱ�ı��б����ݡ�
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
//! River @ 2010-12-28:shader�Ļ������� 
VEC_chrShaderBuf osc_bpmInstance::m_vecChrShaderBuf;

//! �ͷ�shader.
void os_chrShaderBuf::reset( void )
{
	if( (m_iShaderId>=0)&&g_shaderMgr )
		g_shaderMgr->release_shader( m_iShaderId );
	m_iShaderId = -1;
	m_dwHashId = 0xffffffff;
	m_dwReleaseCount = 0xffffffff;
}


//! shaderBuf������ء�
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

	// û�п��У��ҵ��������ʹ�õ��Ǹ����ͷŵ���
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

	// River @ 2011-2-17:Ϊ���ϲ�ı༭��ʹ��
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


	// �ݲ��ͷţ�����������
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
*  �ͷŵ�ǰ��bpmInstance.
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
	//Desc: ride �ͷ�����ָ��
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
	//! River @ 2010-7-1:ȷ�ϵ�ǰ��λ���������
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
// ��������������������������������START OF bodyFrame �����.
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
	// ���ӽ����ͷ���osc_skinMesh������Ǹ��������frame�ӽ����ڴ档

}

/** \brief
*  ���õ�ǰ��frameΪ��ʼ״̬���ͷ�ռ�õ���Դ��
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
 *   ��һ��Frame�ṹ����Frame
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
*  ʹ���豸������ݵ�id�����µ��豸������ݡ�
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
*  ��Frame�ж�Ӧ��meshʹ���µ�bpMesh�滻��
*
*  \param  _npm Ҫ�滻�ɵ��µ�part mesh Instance.
*/
void osc_bodyFrame::change_Mesh( osc_bpmInstance* _pm,
						  const osc_bpmInstance* _npm )
{
	guard;// 


	osassert( _pm );

	osassertex( m_iBMNum>0,"��ǰBodyFrame��BM��Ŀ��������" );

	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		if( m_vecPtrBpmIns[t_i] != _pm )
			continue;

		// 
		// ������ϵ�meshΪ�գ����Ӧ��mesh��һ.
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

	// �Ҳ������滻��Mesh.
	osDebugOut( "The BPM Fname is:<%s>..BM num is:<%d>..\n",
		_pm->get_bpmPtr()->m_strFname,m_iBMNum );
	osassert( false );

	unguard;
}




/** \brief
 *  ��ÿһ��bodyPartMesh��matrix Palette��������BodyFrameMatrix����������
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
	// ���ӽ������ֵܽ����еݹ鴦��
	if(m_pFirstChild)
		m_pFirstChild->LinkBones( _root );

	if(m_pSibling)
		m_pSibling->LinkBones( _root );

	return;
	
	unguard;
}





/** \brief
*  ʹ�þ���Ĳ㼶��ϵȥ���µ�ǰ��Frame.
*  
*/
void osc_bodyFrame::UpdateFrames(osMatrix & _matCur,bool _catchBoneMat/* = false*/ )
{

	m_matCombined = _matCur;	

	osMatrixMultiply(&m_matCombined, &m_matRot, &_matCur);	

	// River added @ 2009-5-31:
	if( _catchBoneMat )
		m_matBonePos = m_matCombined;

	// ���ӽ����и���.
	if(m_pFirstChild != NULL)
	{
		m_pFirstChild->UpdateFrames( m_matCombined,_catchBoneMat );
	}

	// ���ӽ����и���
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

//! ���ص�ǰ�������ϰ󶨵���Ч
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
*  ��ʼ����ǰ��skin Mesh.
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
	// river added @ 2004-4-22: ������תʱ����ֵ��
	m_fLastRotAgl = 0.0f;
	m_fInterpolationTime = 0.0f;
	m_bRotInterpolateStatus = false;
	//@}


	m_dwFPS = FPS_PERSECOND;
	m_fTPF = 1.0f / m_dwFPS;
	m_fNewActSpeed = -1.0f;

	m_bInView = false;

	m_bHasAlphaPart = false;


	// ��ǰʹ�õ�bodyframe��ĿΪ0.
	m_iBFNum = 0;


	//���趨���ֵ��ʹ�ÿ�һֱ���ų�ʼ���Ķ�����
	m_actChange.m_changeType = OSE_DIS_OLDACT;
	m_bFreezeAct = false;
	m_bChangeActState = false;


	m_bWriteZbuf = true;

# if __GLOSSY_EDITOR__
	// ����༭��ģʽ�£�Ĭ�ϲ���	
	m_bPlaySGlossy = true;
	m_bPlayWeaponEffect = true;
# else
	// Ĭ�ϲ����ŵ���.
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

	// ��ʼ��Ϊ�����������ʵʱ��Ӱ.
	m_bHideRealShadow = false;


	//
	// ���������Bounding Box.
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
	//Desc: Ĭ�ϵ�ǰģ�Ͳ�������.
	m_bIsRide = false;
	m_vec3RideOffset = osVec3D(0.0f,0.0f,0.0f);
	m_vec3RidePos =  osVec3D(0.0f,0.0f,0.0f);
	//@}

	// River @ 2007-7-7:�����Ļظ�
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


	//! River @ 2011-2-25:��ǰ�����Ƿ���ɾ��״̬�����ڶ��̲߳�����.
	m_bToBeRelease = false;
	//! River @ 2011-2-25:��ǰ�����Ƿ��ڵ����д��ڱ�־����״̬��
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

	// ����ȡ��������� ��ӰЧ�������ܻ����ĵͶ��Կ����Դ档
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

//!��Ⱦ�����Post trail 
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

//! ���ڼ�����������Ƿ���Բ�ֵ,���flipDeternminant��һ�£������л���
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
*  �ı䵱ǰskinMeshObject�Ķ���.
*
*  �Զ������б任ʱ����Ҫ��һ�������任��ʱ�䣬������Ҫʹ�����ʱ�䣬
*  �Ե�ǰ������Ҫ�任�Ķ������в�ֵ����.
*
*	\param _bSetFrameTime
*/
float osc_skinMesh::change_skinMeshAction( os_newAction&  _act/* ,bool _bSetFrameTime = false */)
{
	guard;

	// 
	// ���ʱ����̣���ʹ��һ�����ٵĶ���
	if (_act.m_fFrameTime<0.01f)
	{
		_act.m_fFrameTime = 0.01f;
	}

	if( (_act.m_changeType == OSE_DIS_NEWACT_TIMES)||
		(_act.m_changeType == OSE_DIS_NEWACT_LASTFRAME) )
		osassert( _act.m_iPlayNum > 0 );

	// 
	// �����һ�κ͵�ǰ�ζ���OSE_DIS_NEWACT_TIMES�Ĳ��ŷ�ʽ,
	// ��ʹ����һ�εĶ�����Ϊ������ɲ��ŵĶ�����
	// ��������в�Ҫ�����ʹ��ԭ���������ŵĶ�����Ҳ��Ҫʹ���µĶ������ŷ�ʽ��
	if( (_act.m_changeType != OSE_DIS_NEWACT_TIMES)||
		(m_actChange.m_changeType != OSE_DIS_NEWACT_TIMES)||
		(!_act.m_bPlayLoopAct) )
	{
		m_iLastActId = m_iAniId;
	}

	m_actChange = _act;
	m_bFreezeAct = false;


	// River @ 2010-7-1:��ȷ������û������.
	hide_weapon( false );
	m_bDisWeaponActionOver = true;

	// 
	// ���ʹ�ö������ֲ��Ŷ���������Ҫ�Ӷ������ֵõ������������š�
	if( _act.m_iNewAct >= 0 )
	{
		m_iAniId = _act.m_iNewAct;
		m_iAniNameIdx = -1;
		osassert( m_iAniId < m_ptrBoneAni->m_iBANum );


		// ʹ��id���Ŷ���ʱ������Ϊ��ʹ�õ��⡣
		if( m_ptrSGBpmIns )
			m_dwFinalGlossyColor = m_ptrSGBpmIns->set_disSGlossy( -1 );

	}
	else
	{
		m_iAniId = m_ptrBoneAni->
			get_actIdxFromName( _act.m_strActName,m_iAniNameIdx );

		//! river @ 2010-7-1:�Ƿ���������.
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
		//	va( "����Id������ڵ�����,��������<%s><%d>....\n",_act.m_strActName,m_iAniNameIdx ) );
		//osassertex( (m_iAniId < m_ptrBoneAni->m_iBANum),"����Id����С�����Ķ�����Ŀ...\n" );
		if (m_iAniId<0 || m_iAniId >= m_ptrBoneAni->m_iBANum)
		{
            osDebugOut(va( "����Id������ڵ�����,��������<%s><%d>....\n",_act.m_strActName,m_iAniNameIdx ));
			m_iAniId = 0;
			return 0;
		}
	}


	// 
	// ���������任ʱ�Ĳ�ֵ������
	m_bChangeActState = can_lerpInDiffAction( m_iAniId,m_iLastActId );
	if( m_bChangeActState )
		m_fActLerpVal = m_fLastUsedTime/m_fTPF;
	else
		m_fActLerpVal = 0.0f;
	m_fLastUsedTime = 0;

	// 
	// ���ò����ٶ���Ϣ��
	set_actSpeedInfo( _act.m_fFrameTime );


	// River @ 2005-8-15:���浱ǰ������֡�������ڸ��µĶ�����ֵ,���ö�������Ϊͷһ֡����.
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


//! ��������߳�������changeEquipment������
bool osc_skinMesh::change_equipmentThread( const os_equipment& _equi )
{
	guard;

	int       t_idx;

	for( int t_i=0;t_i<_equi.m_iCPartNum;t_i++ )
	{
		t_idx = _equi.m_arrId[t_i];

		//
		// ���Ҫ�滻��meshId��ԭ����Mesh��һ�����滻�µ�mesh.
		if( _equi.m_arrMesh[t_i] != m_Look.m_mesh[t_idx] )
		{
			if( !change_bodyMesh( t_idx,_equi.m_arrMesh[t_i] ) )
				return false;

			//
			// ����滻��mesh,������϶���Ҫ������
			if( !change_bodySkin( t_idx,_equi.m_arrMesh[t_i],_equi.m_arrSkin[t_i] ) )
				return false;

			m_Look.m_mesh[t_idx] = _equi.m_arrMesh[t_i];
			m_Look.m_skin[t_idx] = _equi.m_arrSkin[t_i];

			// 
			// ������һ����λ���滻��
			continue;
		}

		//
		// ���Ҫ�滻��skin��ԭ���Ĳ�һ�����滻�µ�skin.
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
*  �滻skinMesh��װ����
*
*  �㷨������
*  ��ÿһ��Ҫ�滻�����岿�֣����Ŀǰ��Ϊ�գ����ٵ�ǰ���岿λ��Դ�����ã�
*��Ȼ�󴴽��µ���Դ��
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

//! Mgr�ڹ���skinMesh���ڴ������ָ�룬Ϊ��ʹ����release_character
//! �ӿڵ�ʱ�򣬿��ԶԹ�������������
CSpaceMgr<osc_skinMesh>* osc_skinMesh::m_ptrVecDadded = NULL;
CSpaceMgr<osc_skinMesh>* osc_skinMesh::m_ptrSceneSM = NULL;


//! ��������߳����ͷ�����.
bool osc_skinMesh::release_characterThread( BOOL _sceneSm )
{
	guard;

	// 
	// �ͷ�frame��Դ��
	if( m_ptrBoneSys )
	{
		osassertex( m_ptrBoneSys->m_iBoneNum <MAX_FRAMENUM,m_strSMDir );
		for( int t_i=0;t_i<m_ptrBoneSys->m_iBoneNum;t_i++ )
			m_pframeToAnimate[t_i]->Reset_Frame();
	}

	// 
	// �ͷ�bodyPartMesh��Դ
	for( int t_i=0;t_i<m_iNumParts;t_i++ )
		m_vecBpmIns[t_i].release_bpmIns();

	reset_skinMesh();


	// Ӧ�����ⲿ���ã��˴�������Ϊ����ǰ�Ĵ������
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
			// River @ 2009-6-6:�ͷ�Ҳ��cs.
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
*  �ͷŵ�ǰCharacter��ռ�õ���Դ��
*
*  ������Ϸ�����п�����Ҫ������ͷŴ�������Դ�����������������ʵ�֡�
*  
*  ATTENTION: 
*/
void osc_skinMesh::release_character( BOOL _sceneC/* = false*/ )
{
	guard;
	
	//  River @ 2011-2-12:������û�е��������£�Ҳ����ɾ����
	//                    ÿһ֡�Ե�����н��д����������֮ǰ����ɾ����skin�Ĳ�����
	//                    ��Ѵ�����ɾ��һ��ȥ����
	// 
	//! ȷ����ǰ��skinMesh�Ѿ�����
	//osassertex( !m_bLoadLock,
	//	"��ǰ��characterָ�������ڵ����ɾ��״̬���߼���ǳ����û���ԭ��?" );

	//! �ͷŹ��������loadLockΪ��
	m_bLoadLock = true;

	// ������߳�ͬ���˱�����
	::EnterCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );
	m_bToBeRelease = true;
	::LeaveCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

	//@{
	// River added @ 2010-9-29��ȷ��.
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

	// ���ͷ���Ч
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

//! �õ�һ������ʹ�õ�bodyFrame�ռ䡣
osc_bodyFrame* osc_skinMesh::get_freeBF( void )
{
	osassert( m_iBFNum < (MAX_FRAMENUM-1) );
	osc_bodyFrame* t_ptr = &m_arrBodyFrame[m_iBFNum];
	m_iBFNum ++;
	return t_ptr;
}


//!  �����ʱ�������ڵ�ǰ���������Ĳ��Űٷֱ�
float osc_skinMesh::cal_posePercent( void )
{
	guard;

	// �����仯�ڼ䷵����
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


//! �õ�ĳ�������¼���İٷֱȣ������ܴ���Ч��Ҫ���ܴ������Ĳ��Ŷ���ʱ���š�
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
		osassertex( (t_idx != -1),va( "�Ҳ�������<%s>...\n",_name ) );
		t_idx = m_ptrBoneAni->m_vecActName[t_iNameIdx].m_vecActIdx;
	}

	os_boneAction*  t_ptrBA = &m_ptrBoneAni->m_vecBATable[t_idx];

	osassertex( t_ptrBA->m_iStopFrameIdx <= t_ptrBA->m_iNumFrame,
		va( "����ֹͣ֡�����ݲ���<%d,%d>..\n",t_ptrBA->m_iStopFrameIdx,t_ptrBA->m_iNumFrame ) );

	t_fPercent = float( t_ptrBA->m_iStopFrameIdx ) / float(t_ptrBA->m_iNumFrame );

	return t_fPercent;

	unguard;
}


//! �õ���ǰ���ڲ��ŵĶ������ֺ�����
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

//! ���Եõ�ĳһ��������λ��.
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



//! ��������������λ�õĸ���
float osc_skinMesh::frame_move( osVec3D& _npos,
								   float _nangle,bool _rideSta )
{
	guard;

	// River @ 2011-3-3����ѡ�˳��������ܻ�ֱ��frameMove.
	if( !legal_operatePtr() )
		return 0.0f;

	float t_fPosePercent = cal_posePercent();

	// 
	// �����ǰ����Ķ�������ֹͣ״̬�������ƶ�����ת��
	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	if( m_bActStopStatus &&(m_wAniIndex >= t_ptrBA->m_iStopFrameIdx) )
		return t_fPosePercent;

	m_vec3SMPos = _npos;

	//
	// ���������תֵΪ0-2PI֮��.
	if( _nangle < 0 )
		_nangle += (2.0*OS_PI); 

	//
	// ���µ�ǰ�������Ӧ��Bounding Box������ֵ��
	m_sBBox.set_bbYRot( _nangle );
	osVec3D   t_vec3Offset;
	m_sAdjBBox.get_smOffsetAdjBBox( t_vec3Offset,_nangle );
	m_sBBox.set_bbPos( m_vec3SMPos + t_vec3Offset );


	// ����µ���ת�����ԭ���ķ�����ͬ���򲻽��д���
	if( float_equal( _nangle,m_fRotAgl ) )
		return t_fPosePercent;

	//
	// �����ǰ���Ǵ�����ת��ֵ״̬��
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
	// ���������ת�Ƕ�֮��Ĳ�ֵ�ľ���ֵ����OS_PI,��Ѵ���OS_PI��ֵ
	// ��Ϊ0����OS_PI֮���ֵ����Ϊ����ֻ�У����������Ա������OS_PI+0.1
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
*  ���µ�ǰ��skinmeshObj��λ�á�
*
*  �˺����������޸�skinMesh�����λ�ú���ת�Ƕ�.
*  �����ڲ���skinObjFramemove������skinMesh��������
*  
*  
*/
float osc_skinMesh::frame_move( osVec3D& _npos,float _nangle )
{
	guard;

	// River @ 2011-3-3����ѡ�˳��������ܻ�ֱ��frameMove.
	if( !legal_operatePtr() )
		return 0.0f;

	float t_fPosePercent = cal_posePercent();

	//! �����ǰ��������˵����ﲻΪ�գ�����ִ���������
	if( m_RideObj != NULL )
	{
		osDebugOut( "The error frame move...\n" );
		return 0.0f;
	}

	// 
	// �����ǰ����Ķ�������ֹͣ״̬�������ƶ�����ת��
	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	if( m_bActStopStatus &&(m_wAniIndex >= t_ptrBA->m_iStopFrameIdx) )
		return t_fPosePercent;

	m_vec3SMPos = _npos;

	//
	// ���������תֵΪ0-2PI֮��.
	if( _nangle < 0 )
		_nangle += (2.0*OS_PI); 

	//
	// ���µ�ǰ�������Ӧ��Bounding Box������ֵ��
	m_sBBox.set_bbYRot( _nangle );
	osVec3D   t_vec3Offset;
	m_sAdjBBox.get_smOffsetAdjBBox( t_vec3Offset,_nangle );
	m_sBBox.set_bbPos( m_vec3SMPos + t_vec3Offset );


	// ����µ���ת�����ԭ���ķ�����ͬ���򲻽��д���
	if( float_equal( _nangle,m_fRotAgl ) )
		return t_fPosePercent;

	//
	// �����ǰ���Ǵ�����ת��ֵ״̬��
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
	// ���������ת�Ƕ�֮��Ĳ�ֵ�ľ���ֵ����OS_PI,��Ѵ���OS_PI��ֵ
	// ��Ϊ0����OS_PI֮���ֵ����Ϊ����ֻ�У����������Ա������OS_PI+0.1
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



//! �Ӷ��������У��õ�����Ҫ������һ�����������ܴ������������������
int osc_skinMesh::get_aniNameIdx( void )
{
	guard;


	// û������Ķ���
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
*  ÿ������һ�׶���ʱ�����ݵ�ǰ��actChange�ṹ�Զ������д���
*/
os_boneAction* osc_skinMesh::process_ActionOver( void )
{
	guard;

	os_boneAction*  t_ptrBA;
	osassert( m_ptrBoneAni );
	osassert( m_iAniId >= 0 && (m_iAniId < int(m_ptrBoneAni->m_vecBATable.size()) ) );
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	//! river @ 2010-7-1:�Ƿ���������
	if( m_bDisWeaponActionOver )
		hide_weapon( false );


	// �õ����µ�BoneAction table.
	if( m_iAniNameIdx == -1 )
	{
		t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	}
	else
	{
		//
		// ����µĶ����������Ե��滻ԭ���Ķ���,
		// �������ѡ��ǰ���ֵĶ���. 
		if( m_actChange.m_changeType == OSE_DIS_OLDACT )
		{
			// 
			// River added @ 2006-6-7:�������������֣�����������Ŷ���
			m_iAniNameIdx = get_aniNameIdx();
			if( m_iAniNameIdx == -1 )
				m_iAniNameIdx = 0;

			// 
			// ͬһ���ֵĶ�������������һ��,�ڶ������������л���
			int t_idx = m_ptrBoneAni->
				get_actBoneAction( m_iAniNameIdx );

			osassert( t_idx>=0 && (t_idx<int(m_ptrBoneAni->m_vecBATable.size())) );

			// 
			// �������ID�ı䣬��ʼ�任�����Ĵ���,�����������
			if( m_iAniId != t_idx )
			{
				// River fixed @ 2004-10-20:�п��ܴ�ֵ����ƻ��ڴ�.
				// ���ܵ�ǰ�Ķ������������¶�����֡��.
				m_wAniIndex %= t_ptrBA->m_iNumFrame;

				m_iLastActId = m_iAniId;
				m_bChangeActState = true;
				m_fActLerpVal = m_fLastUsedTime/m_fTPF;
				m_fLastUsedTime = 0;
				m_iAniId = t_idx;

				// ���ò����ٶ���Ϣ��
				set_actSpeedInfo( m_fNewActSpeed );

				// 
				// River @ 2006-6-7:�ƶ��¾���뵽�˴�����Ȼ
				// t_ptrBA = &m_ptrBoneAni->m_vecBATable[t_idx];
				// �����󣬶����л����ܻ���ִ��� .  ���������л���Ķ���Ϊ��֡
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
		// River mod @ 2004-2-4:ʹ�ü�������ʱ�����
		// ������⣬����ʹ�����ࡣ
		m_wAniIndex %= t_ptrBA->m_iNumFrame;
		//@}
		return t_ptrBA;

	}

	// ���β��Ŷ�����
	if( (m_actChange.m_changeType == OSE_DIS_NEWACT_TIMES) )
	{
		m_actChange.m_iPlayNum --;

		if( m_actChange.m_iPlayNum == 0 )
		{
			// 
			// ������µĶ�������һֱ������,ʹ���µĶ���.
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
			// �����ڲ����������֮ǰһֱ���ŵĶ���.
			m_actChange.m_changeType = OSE_DIS_OLDACT;
			m_iAniId = m_iLastActId;
			m_wAniIndex = 0;

			//! River @ 2006-6-10:Ϊ�˻ظ�����ǰ�Ķ������Ѷ�������������Ϊ-1
			m_iAniNameIdx = -1;

			//
			// ���ò����ٶ���Ϣ��River mod @ 2006-8-8:�������ö���ʱ�䣬��Ϊ�˴���
			// m_fNewActSpeed����ͼ�ǣ���һ��������ʱ�䡣
			set_actSpeedInfo( m_ptrBoneAni->m_vecBATable[m_iAniId].m_fTimeToPlay );
		}
		else
		{
			m_wAniIndex %= t_ptrBA->m_iNumFrame;
		}

		return t_ptrBA;
	}

	// 
	// ���β��Ŷ����������Ŷ��������һ֡��
	// Ȼ�󵭳���
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
*  �õ���ǰskinMesh������ռ��е�λ�ú���ת�Ƕ�.
* 
*  \param _pos    ����skinMesh������ռ��λ��.
*  \param _fangle ����skinMesh������ռ����ת�Ƕ�.
*/
void osc_skinMesh::get_posAndAgl( osVec3D& _pos,float& _fangle )
{
	guard;

	_pos = m_vec3SMPos;

	//! River mod @ 2008-5-12:��Ϊ��ת�������в�ֵ�����أ����Ա��봦��
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
			// �õ�������ת�Ƕȼ�Ĳ�ֵ��
			t_fRotAgl = t_fInterTime/ROTINTERPOLATE_TIME;
			t_fRotAgl = t_fRotAgl*(m_fRotAgl-m_fLastRotAgl)+m_fLastRotAgl;
		}
		_fangle = t_fRotAgl;
	}
	unguard;
}

//! ���ڸ������������Ч��λ�úͷ���
void osc_skinMesh::update_followEffect( void )
{
	guard;

	// ���������������ת������ȷ����ǰ������Ч��ʵ��λ��
	osMatrix   t_sRotMat;
	osVec3D    t_vec3Offset;

	// ��Ϊm_fAdjRgl�ڳ�ʼ����ʱ��-OS_PI/2.0f;
	osMatrixRotationY( &t_sRotMat,m_fFinalRotAgl+OS_PI/2.0f );	

	osVec3Transform( &t_vec3Offset,&mEffectOffset,&t_sRotMat );
	get_sceneMgr()->set_effectPos( mEffectId,m_vec3SMPos + t_vec3Offset,m_fFinalRotAgl);

	unguard;
}

// River @ 2011-2-10:����ȫ�ֵı��������ڴ�������ʹ����ĳ����Ժÿ�һЩ��
OSENGINE_API BOOL g_bCharRotInterpolateStatus = true;

/** \brief 
*  ����ǰskinmesh��ÿһ��frame�ĸ��¡�
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
	// River @ 2011-2-10:����ȫ�ֵı��������ڴ�������ʹ����ĳ����Ժÿ�һЩ��
	// ���ݵ�ǰ�Ƿ�����ת��ֵ״̬���д���
	if( m_bRotInterpolateStatus && g_bCharRotInterpolateStatus )
	{
		t_fRotAgl = sg_timer::Instance()->get_lastelatime();

		m_fInterpolationTime += t_fRotAgl;
		if( m_fInterpolationTime >= ROTINTERPOLATE_TIME )
		{
			// �ı�״̬��
			m_bRotInterpolateStatus = false;
			t_fRotAgl = m_fRotAgl;
		}
		else
		{
			// �õ�������ת�Ƕȼ�Ĳ�ֵ��
			t_fRotAgl = m_fInterpolationTime/ROTINTERPOLATE_TIME;
			t_fRotAgl = t_fRotAgl*(m_fRotAgl-m_fLastRotAgl)+m_fLastRotAgl;
		}
	}
	else
		t_fRotAgl = m_fRotAgl;

	// ������������ת΢��.
	m_fFinalRotAgl = t_fRotAgl + m_fAdjRgl;
	osMatrixRotationY( &t_matRot,m_fFinalRotAgl );	

	if(m_vec3AdjPos.x || m_vec3AdjPos.y || m_vec3AdjPos.z){
		osMatrix		 t_adjWorld;
		// Set world matrix.����������λ��΢����λ�����ݡ�
		osMatrixTranslation(&t_adjWorld,m_vec3AdjPos.x,m_vec3AdjPos.y,m_vec3AdjPos.z);
		osMatrixMultiply(&t_matRot,&t_adjWorld,&t_matRot);
	}	

	osMatrixMultiply( &t_matWorld,&t_matRot,&t_matWorld );
	
	m_pRoot->UpdateFrames( t_matWorld,_catchBoneMat );
	//@{
	// Windy mod @ 2005-8-29 10:20:04
	//Desc: ride ���������϶�λ�е�λ��
	if (m_bIsRide){
		m_pRoot->UpdateRideBoxPos();
	}
	//@}

	//@{
	// Windy mod @ 2005-8-31 10:08:15
	//Desc: ride ����ÿһ֡�����������λ���ƶ�
	if( (m_HumObj!=NULL)&&(!m_HumObj->m_bLoadLock) )
	{
		osVec3D t_3d;
		get_RideBoxPos(t_3d);
		m_HumObj->frame_move(t_3d,m_fRotAgl,true);
		m_HumObj->update_frame( _catchBoneMat );
	}

	//@}

	// 
	//! ��������ĸ�����Ч
	if (mEffectId != -1)
		update_followEffect();
}


//! �õ���ǰm_iNegAniIndex�ؼ�֡��Ӧ�ĵĶ�����
void osc_skinMesh::get_keyFramePose( void )
{
	guard;

	os_boneAction*   t_ptrBA;


	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	//
	// ��ÿһ��Frame���д���,ֻ��Ҫ�õ��ؼ�֡����������Ҫ��ֵ��
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

	// ����ÿһ��frame�ڵľ���
	update_frame();

	return ;


	unguard;
}


/** \brief
*  ����"��ʱ��"��skinMesh�������㡣
*
*  ���ڵ���ĵ��ƣ���Ӱ�ļ���ȵ�Ч����
*  ������������Ƕ����ĸı䣬�ڵ���֡��ʱ�򣬲��ͽ���֡����ֵ��
*  ���֡�����˿�ʼ֡������һֱ֡�Ӵӽ���֡��λ�ÿ�ʼ��
*  
*  \param float _ftime ��ֵ����С�ڵ����㣬�������д˺��������塣
*/
void osc_skinMesh::negative_frameMoveAction( float _ftime )
{
	guard;


	os_boneAction*   t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	// River:����ʹ��assert,�ݴ����.
	if( _ftime > 0.0f )
		_ftime = 0.0f;

	m_fNegLastUsedTime = m_fNegLastUsedTime + _ftime;
	if( m_fNegLastUsedTime < 0 )
	{
		//����ֱ���ҵ��ؼ�֡���ڵ�λ�á�
		m_iNegAniIndex --;

		while( (m_fNegLastUsedTime += m_fTPF)<0 )
			m_iNegAniIndex --;

		// ��ʼ֡.
		if( m_iNegAniIndex < 0 )
		{
			m_iNegAniIndex = 0;
			m_fNegLastUsedTime = 0.0f;
		}

	}


	//
	// ��ÿһ��Frame���д���,���ݵ�ǰ��ֵϵ������֡���в�ֵ��
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
		// ��������һ֡����͵�һ֡���в�ֵ��
		if( m_iNegAniIndex == (t_ptrBA->m_iNumFrame-1) )
			t_iNextActIdx = m_iNegAniIndex;
		else
			t_iNextActIdx = m_iNegAniIndex+1;
		t_iNextActIdx = t_ptrBA->m_iBoneNum*t_iNextActIdx;
		t_iNextActIdx += t_j;

		t_matPtr = &t_ptrBA->m_vecMatrix[t_iMatPtr];
		t_nextMatPtr = &t_ptrBA->m_vecMatrix[t_iNextActIdx];

		//
		// �޸ĵ��������Ĳ��,ʹ����ʹ��quaternion��ʾ��
		lerp_actionPose( m_pframeToAnimate[t_j]->m_matRot,
			t_matPtr,t_nextMatPtr,m_fNegLastUsedTime/m_fTPF );

	}

	// ����ÿһ��frame�ڵľ���
	update_frame();

	return ;


	unguard;
}



//! ���ռ��㵱ǰ���Ƶľ�����������������ڶ����仯״̬��
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
		// ��������һ֡����͵�һ֡���в�ֵ��
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
		// �޸ĵ��������Ĳ��,ʹ����ʹ��quaternion��ʾ��
		// 
		osassertex( m_pframeToAnimate[t_j],
			va( "��ȷ������<%s>�Ĺ���bis�ļ�.��������<%d>..load��־<%d>..\n",m_strSMDir,t_j,(int)m_bLoadLock ) );
		lerp_actionPose( m_pframeToAnimate[t_j]->m_matRot,
			t_matPtr,t_nextMatPtr,m_fLastUsedTime/m_fTPF );

	}

	// ����ÿһ��frame�ڵľ���
	update_frame( true );


	unguard;
}


/** \brief
*  �Ե�ǰ�Ĺ�������frame move
*  WORK START:�˴������?
*/
void osc_skinMesh::frame_moveAction( float _ftime )
{
	guard;

	float    t_f;
	int      t_i;
	// ����Ƿ��ڶ���ֹͣ״̬��
	os_boneAction*  t_ptrBA;

	// River @ 2011-3-1:
	m_bMustUpdateNextFrame = false;

	//  River @ 2009-6-7:
	//! ��������ڵ����lock״̬��ֱ�ӷ���
	if( m_bLoadLock )
		return;

	osassertex( m_ptrBoneAni,va( "��������ָ��<0x%x>...\n",DWORD(this) ) ); 

	osassert( m_iAniId >= 0 );
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	if( m_wAniIndex >= t_ptrBA->m_iStopFrameIdx )
	{
		m_bArriveStopFrame = true;
		if( m_bActStopStatus )
			_ftime = 0.0f;
	}

	// ������ڶ����任״̬������
	if( m_bChangeActState )
	{
		frame_moveActChange( _ftime );

		// ATTENTION TO FIX: �������̣�
		// m_iAniId��ֵ���ܸı䣬����ʹ���µ�t_ptrBA.
		t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];
	}

	//
	// �����ǰ�ǲ��Ŷ������һ֡�Ķ��������ڶ����л�״̬��������زд���
	if( (!m_bFreezeAct)&&(!m_bChangeActState) )
	{

		// ����Ǵ��ڶ���ֹͣ״̬����ʹʱ�䲻����
		if( m_bSuspendState )
			_ftime = 0.0f;

		t_f = m_fLastUsedTime + _ftime;
		t_i = int(t_f/m_fTPF);
		m_wAniIndex += t_i;

		if( t_i < 1 )
			m_fLastUsedTime += _ftime;
		else
			m_fLastUsedTime = (t_f-t_i*m_fTPF);


		// �����ǰ����֡��������ǰ�����Ķ���֡��,����
		if( m_wAniIndex >= (t_ptrBA->m_iNumFrame-1) )
		{
			os_boneAction* pNew = process_ActionOver();

			// �������л�ʱ�����¾ɵ�ָ�� syq
			if( pNew != t_ptrBA )
				t_ptrBA = pNew;

			// �����ǰ�Ƕ������һ֡�Ķ�����ʹ��t_i����0,
			// �����һ�θ��¶�����
			if( m_bFreezeAct )
			{
				m_wAniIndex = t_ptrBA->m_iNumFrame-1;

				// ����Ƕ��Ტ�������һ֡����,����Ҫ�͵���֡���в�ֵ.
				m_fLastUsedTime = 0;
			}
		}

		// 
		// River added @ 2006-6-7: �������ʹ�ö����������������ź�
		// ʹ�ô˶δ��룬��ֹ����л�����ʱ����������
		if( m_bChangeActState )
		{
			frame_moveActChange( _ftime );

			update_frame( true );

			// �����������²�����
			if( _ftime > 0.0f ) m_bRenderGlossy = false;

			return;
		}
	}
	else
	{
		update_frame( true );
		
		// �����������²�����
		if( _ftime > 0.0f ) m_bRenderGlossy = false;

		return;
	}

	osassert( t_ptrBA );


	//
	// ��ÿһ��Frame���д���,���ݵ�ǰ��ֵϵ������֡���в�ֵ��
	// ����ִ�е��˴�ʱ������һ�������ڵĲ�ͬ�����ؼ�֮֡����в�ֵ��
	// �������õ����Ƿ��Ѿ�����Ϊfalse
	osassert( t_ptrBA->m_iBoneNum>=0 );
	cal_finalPose();

	// �����������²�����
	if( _ftime > 0.0f ) m_bRenderGlossy = false;


	return;

	unguard;

}


/** \brief
*  �����ǰ���ڶ����任״̬����Ҫ�������������û�������
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

		// ���µ��ö������º�����
		frame_moveAction( 0.0f );

		// 
		// ����µĶ���ʹ�õ��⣬�ڴ˴���ʼʹ���¶����ĵ��⡣
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
	// ��ÿһ��Frame���д���,���ݵ�ǰ��ֵϵ������֡���в�ֵ��
	for( int t_i=0;t_i<t_ptrBA->m_iBoneNum;t_i++ )
	{

		//
		// �ɶ���������֮֡����һ�ζ����ۺ�.
		// �ȵõ���һ�����������վ���.��������һ֡����͵�һ֡���в�ֵ��
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
		// ԭ����ֵ�ɾ���,�ٷ��س�quat,����ֱ�Ӳ�ֵ��quat.
		/*
		lerp_actionPose( t_lastMat,t_matPtr,t_nextMatPtr,m_fActLerpVal );
		*/
		lerp_actionPose( &t_tmpActPose,t_matPtr,t_nextMatPtr,m_fActLerpVal );


		// 
		// ���¶����ĵ�0֡���л�ϡ�
		t_iMatPtr = 0;
		t_iMatPtr += t_i;
		t_matPtr = &t_ptrBA->m_vecMatrix[t_iMatPtr];

# ifdef _DEBUG
		if( t_matPtr->m_bFlipDeterminant != t_nextMatPtr->m_bFlipDeterminant )
		{
			osDebugOut( "SM����Ϊ:<%s>,Դ����<%s>,Ҫ�ı䵽����<%s>,����...\n",
				m_strSMDir, t_ptrBALast->m_szActName,t_ptrBA->m_szActName );
		}
# endif 
		osassertex( (t_matPtr->m_bFlipDeterminant == t_nextMatPtr->m_bFlipDeterminant),
			"����������Ӧͬһ������FlipDeterminant����һ��...\n" );

		/*
		// River mod @ 2007-4-20:
		// ��֮֡�������bone��ϵ�ľ�����в�ֵ��
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
*  ��ǰ�����ܹ��ж����׶������Բ��š�
*/
int osc_skinMesh::get_totalActNum( void )
{
	guard;

	osassert( m_ptrBoneAni );
	return m_ptrBoneAni->m_iBANum;

	unguard;
}



/** \brief
*  �Ӷ��������ֵõ����ŵ�ǰ������Ҫ��ʱ�䡣
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
		osDebugOut( "������<%s>�Ķ����б���,�Ҳ�������<%s>...\n",
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
*  �Ӷ��������ֵõ����ŵ�ǰ������Ҫ��ʱ�䡣
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
		osDebugOut( "������<%s>�Ķ����б���,�Ҳ�������<%s>...\n",
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
*  �Ӷ�����id�õ����ŵ�ǰ�Ķ�����Ҫ��ʱ��.
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
//! ������ǰskinMesh�ڵ����ݡ�
void osc_skinMesh::dump_debugStr( void )
{
	osDebugOut( "\n\n\n\nStart DUMP SKINMESH INFO\n" );
	osDebugOut( "Skin Num Parts is:<%d>..\n",this->m_iNumParts );
	osDebugOut( "Skin bf Num is:<%d>..\n",m_iBFNum );
	osDebugOut( "\n\n" );

}
# endif 

//! �������ⷢ���仯ʱ����skinMeshMgr���д���
void osc_skinMeshMgr::reset_ambient( void )
{
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;
	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		//! River mod @ 2009-6-8:��������Ķ��̴߳���
		if( t_ptrNode.p->m_bLoadLock )
			continue;
		osassertex( t_ptrNode.p->m_bInUse,
			va( "The Idx is:<%d>.Dir is:<%s>..",t_ptrNode.idx,t_ptrNode.p->m_strSMDir ) );


		// 
		// ���ڸ�����������������û�����.
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
		//River mod @ 2007-4-10:ȷ���������.ȷ�����������������.
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
River @ 2009-6-5:�¿�һ���̣߳����ڵ������

�����ǰ�Ĺ����У�û����Ҫ��������ѧϰ��ͼ����Ĺ�������˯����
����Ҫ���������ʱ�����ѣ�Ȼ����롣���һ������ĵ���󣬲鿴�Ƿ����µ�����,
����У����������û�У�˯���ȴ���
 


*/


/** \brief
*  ���ڴ����Ͳ����ϲ���Ҫ��SkinMeshObject.
*
*  �ɳ�����������������ϲ��ṩ����ʹ�õ�skinMeshָ�롣
*  �ϲ������Ĳ�����ͨ������ӿڽ��С�
*
*  \param _smi ���ڳ�ʼ���´�����skinMeshObject.
*  \return     �������ʧ�ܣ�����NULL.
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
		    "����������һ�����岿λ���ܴ���....\n" );

	//
	// �ҵ����õ�characterָ��
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





