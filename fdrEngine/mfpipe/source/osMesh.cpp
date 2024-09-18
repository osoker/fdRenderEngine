/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osMesh.cpp
 * 
 *  His:      River created @ 2003-12-27
 *
 *  Desc:     �м�ܵ��ɽ��ܵĶ�������Ⱦ��������.a
 *  
 *  "�����������һ��ǿ�ҵľ�����ʶ,���ֲ�������������˾޴�Ķ������Ժ��С�
 *   �������ֲ�Ը���Լ�����˥��Ķ���֧�������ǡ�"
 *  
 *  ����ss,�ٴβ���ss.
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osMesh.h"
# include "../include/osStateBlock.h"
# include "../include/middlePipe.h"
# include "../../backPipe/include/osCamera.h"

# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"
# include "../include/osEffect.h"
//! ˮ�����
osc_effect* osc_meshMgr::m_ptrWaterEffect = NULL;

//! �ڴ�ɽ�����mesh���ݣ�ÿһ����������������Ŀ��mesh,�ɳ�פ�ڴ档
# define MAX_SWAPMESHNUM  100


// 
// ����ļ�����ʹ��ǰ�˹ܵ������������Ⱦ�� 

osc_Mesh::osc_Mesh()
{
	m_ptrPMesh = NULL;
	m_iShaNum = 0;
	m_bAlphaMesh = false;
	m_bOpacityMesh = false;

	m_bVipObj = false;
	m_bTreeObj = false;
	m_bUseDirLAsAmbient = false;
	m_bBridgeObj = false;
	m_bAcceptFog = false;

	m_dwHash = 0;

# if CUSTOM_MESHDRAW
	m_ptrMeshVBuf = NULL;
	m_ptrMeshIBuf = NULL;
# endif 

	// ��ʼ��Ϊ-1;
	for( int t_i = 0;t_i<MAXSUBSET_PERMESH;t_i ++ )
		m_vecShaderId[t_i] = -1;

	m_vecFaceNormal = NULL;

	m_bInMemoryForEver = false;

}

osc_Mesh::~osc_Mesh()
{
	release_mesh();
}


//! �ͷŵ�ǰMesh���豸�����Դ.
void osc_Mesh::release_mesh( void )
{
	// �ͷŴ�dxMesh�ڵõ��Ķ��������������
# if CUSTOM_MESHDRAW
	SAFE_RELEASE( m_ptrMeshVBuf );
	SAFE_RELEASE( m_ptrMeshIBuf );
# endif 

	SAFE_RELEASE( m_ptrPMesh );

	int t_i;
	for( t_i =0;t_i<m_iShaNum;t_i++ )
	{
		g_shaderMgr->release_shader( m_vecShaderId[t_i] );
		m_vecShaderId[t_i] = -1;
	}
	for( t_i = 0;t_i<MAXSUBSET_PERMESH;t_i ++ )
	{
		m_vecShaderId[t_i] = -1;
		m_vecAlphaSubset[t_i] = false;
	}

	m_iShaNum = 0;
	m_bAlphaMesh = false;

	// ������Ʒ����
	if( m_bBridgeObj )
		SAFE_DELETE_ARRAY( m_vecFaceNormal );
}

//! �õ�faceNormal.
bool osc_Mesh::get_faceNormal( int _faceIdx,osVec3D& _normal )
{
	guard;

	if( !m_bBridgeObj )
		return false;

	_normal = m_vecFaceNormal[_faceIdx];
	
	return true;

	unguard;
}


//! ����һ������,���Ƿ��*.x��mesh�ļ��ཻ.
bool osc_Mesh::intersect_mesh( 
		osVec3D& _rayPos,float _dis,osVec3D& _rayDir,float* _resDis/* = NULL*/ )
{
	guard;

	HRESULT    t_hr;
	BOOL       t_bHit = false;
	DWORD      t_dwFaceIdx;
	float      t_u,t_v;
	float      t_dist;
	DWORD      t_dwCOH;
	LPD3DXBUFFER  t_buf;
	
	osassert( m_ptrPMesh );
	t_hr = D3DXIntersect( m_ptrPMesh,&_rayPos,&_rayDir,
		&t_bHit,&t_dwFaceIdx,&t_u,&t_v,&t_dist,&t_buf,&t_dwCOH );
	if( FAILED( t_hr ) )
	{
		//osassert( false );
		return false;
	}

	osVec3D  t_vec3Normal;
	/*
	if( _resDis )
	    *_resDis = t_dist;
	*/
	// ����ϲ���Ҫ�ཻ��λ��,�򷵻�.
	if( _resDis && t_bHit )
	{
		// River @ 2010-3-30:����ཻ������棬����
		if( get_faceNormal( t_dwFaceIdx,t_vec3Normal ) )
		{
			if( osVec3Dot( &t_vec3Normal,&_rayDir ) <= 0.0f )
				*_resDis = t_dist;
		}
		else
			*_resDis = t_dist;
	}

	// ATTENTION TO OPP:
	// ����Ƿ�����ǵ����߶��ཻ,
	// River @ 2009-2-6: �����Ҫ�õ��ཻ��λ�þ���,�����,������ݽ��ֱ�ӷ���.
	if( t_bHit )
	{		
		LPD3DXINTERSECTINFO   t_info;
		t_info = (LPD3DXINTERSECTINFO)t_buf->GetBufferPointer();

		for( DWORD t_dw = 0;t_dw<t_dwCOH;t_dw++ )
		{
			if( _resDis )
			{
				if( t_info[t_dw].Dist < *_resDis )
				{
					// River @ 2010-3-30:����ཻ������棬����
					if( get_faceNormal( t_info[t_dw].FaceIndex,t_vec3Normal ) )
					{
						if( osVec3Dot( &t_vec3Normal,&_rayDir ) >= 0.0f )
							continue;
					}
					
					*_resDis = t_info[t_dw].Dist;
				}
			}
			else
			{
				if( t_info[t_dw].Dist < _dis )
				{
					t_buf->Release();
					return true;
				}
			}
		}
	
		t_buf->Release();
		if( _resDis )
		{
			if( *_resDis < _dis )
				return true;
		}
	}
	
	return false;

	unguard;
}


//! �����豸ָ��.
LPDIRECT3DDEVICE9 osc_Mesh::m_pd3dDevice = NULL;


/** \brief
*  ���ļ���ʼ��һ��Mesh.
*
*  \param _fname Ҫ�������ļ�������.
*  \param _dev   �����豸�����Դ��Ҫ��3d�豸��
*  
*/
bool osc_Mesh::create_meshFromFile( const char* _fname, LPDIRECT3DDEVICE9 _dev, BYTE* _meshMem/* = NULL*/ )
{
	guard;

	osassert( _dev );
	osassert( _fname );

	CSceneEx       t_ex;
	LPD3DXBUFFER    t_pMtrlBuffer = NULL;
	DWORD          t_numMtl;
	BYTE*          t_ptrMeshStart = NULL;

	// �ļ�������?
	osassertex( strlen( _fname ) < MAXMESH_FILENAMELEN ,
		   va( "�ļ���<%s>����,����ϵ����...\n",_fname )  );

	if( _meshMem )
		t_ex.loadEXFromMem( _meshMem );
	else
	{
		if( !t_ex.LoadEXFile( (char*)_fname,
			_dev,t_pMtrlBuffer,t_numMtl,m_ptrPMesh ) )
		{
			char temp[250];
			sprintf(temp,"%s����ex�ļ�ʧ��",_fname);
			osassertex(false,temp);
			return false;
		}
	}


	//
	// �õ�Bounding������ݡ�
	m_bsphere.veccen = t_ex.m_sBound.m_VECCenter;
	m_bsphere.radius = t_ex.m_sBound.m_fRadius;
	m_bbox.m_vecMax = t_ex.m_sBound.m_VECmax;
	m_bbox.m_vecMin = t_ex.m_sBound.m_VECmin;

	// 
	// �õ��µ���Ʒ����
	m_bVipObj = t_ex.m_sMeshVipInfo.m_bVipObj;
	m_bTreeObj = t_ex.m_sMeshVipInfo.m_bTreeObj;
	m_bBridgeObj = t_ex.m_sMeshVipInfo.m_bIsBridge;
	m_bAcceptFog = t_ex.m_sMeshVipInfo.m_AcceptFog;
	
	m_bUseDirLAsAmbient = t_ex.m_bUseDirLAsAmbient;


	//
	// ������ǰMesh�õ���shader.
	// ATTENTION: ʹ�õ�ͼԤ�����������Щ���ܡ�
	D3DXMATERIAL*  t_d3dxMtrls = (D3DXMATERIAL*)t_pMtrlBuffer->GetBufferPointer();
	s_string       t_str;
	int           t_idx;


	m_iShaNum = t_numMtl;

# ifdef _DEBUG
	if( m_iShaNum > MAXSUBSET_PERMESH )
		MessageBox( NULL,_fname,"����ģ���õ��˳���12����Mtl",MB_OK );
	osassert( m_iShaNum <= MAXSUBSET_PERMESH );
# endif 

	for( DWORD t_i=0;t_i<t_numMtl;t_i++ )
	{
		
# ifdef _DEBUG
		if( !t_d3dxMtrls[t_i].pTextureFilename )
		{
			char   t_str[256];
			sprintf( t_str,"ģ�� <%s> ����û��ʹ������Ĳ���",_fname );
			MessageBox( NULL,t_str,"ERROR",MB_OK );
			osassertex( false,va("ģ�� <%s> ����û��ʹ������Ĳ���",_fname) );
		}
# endif 

		t_str = t_d3dxMtrls[t_i].pTextureFilename;

		t_idx = (int)t_str.rfind( '.' );
		t_str = t_str.substr( 0,t_idx );
		t_idx = (int)t_str.rfind( '\\' );
		t_str = t_str.substr( t_idx+1,t_str.length() );

		/// ʹ���µ�shader���������ʧ�ܣ������һ���ԭ���ķ�ʽ������ԭ���ķ�ʽӦ��ɾ��
		m_vecShaderId[t_i] = g_shaderMgr->add_shader( 
			t_str.c_str(), osc_engine::get_shaderFile( "object" ).c_str() );
		//----��ɾ��----
		if( m_vecShaderId[t_i] < 0 )
		{
			m_vecShaderId[t_i] = g_shaderMgr->add_shadertomanager( 
				(char*)t_str.c_str(),OBJECT_SHADERINI,osc_engine::m_iObjectShaId );
		}
		//--------------			

		// �õ���ǰ��subset�Ƿ���alpha subset.
		m_vecAlphaSubset[t_i] = g_shaderMgr->has_alphaChannel( m_vecShaderId[t_i] );

	}

	//syq
	t_pMtrlBuffer->Release();



	//
	// ����ǰ��mesh�Ƿ���alphaMesh.
	for( int t_i=0;t_i<(int)t_numMtl;t_i++ )
	{
		if( m_vecAlphaSubset[t_i] )
			m_bAlphaMesh = true;
		else
			m_bOpacityMesh = true;
	}

	//
	// ATTENTION TO OPP: дһ��ת������,�����ڵ�ex�ļ���ת��Ϊ
	//                   2�İ汾����������ת��
	// ���ʹ�û�����ģ�⶯̬���Ч��������Ҫ�ٷ�ת���ǵ�normal.
	// ����Ϸ������ǰ���뷴תnormal��Ϊ�Ժ��������׼����
	// �����ǰex�汾��Ϊ1,��תÿһ�������normalֵ��
	if( t_ex.m_dwFileVersion == 1 )
	{

# if 1
		os_objectVertex*   t_ptrVer;
		m_ptrPMesh->LockVertexBuffer( NULL,(LPVOID*)&t_ptrVer );
		for( int t_i=0;t_i<(int)m_ptrPMesh->GetNumVertices();t_i++ )
		{
			t_ptrVer->m_vecNormal.x = -t_ptrVer->m_vecNormal.x;
			t_ptrVer->m_vecNormal.y = -t_ptrVer->m_vecNormal.y;
			t_ptrVer->m_vecNormal.z = -t_ptrVer->m_vecNormal.z;
			t_ptrVer ++;
		}
		m_ptrPMesh->UnlockVertexBuffer();
# else
		osassert( false );
# endif 

	}

	//! River @ 2010-3-30:�����������Ʒ�������ÿһ��face��normal.
	if( m_bBridgeObj )
	{
		HRESULT  t_hr;
		int t_iNumFace = m_ptrPMesh->GetNumFaces();
		m_vecFaceNormal = new osVec3D[t_iNumFace];

		os_objectVertex*   t_ptrVer = NULL;
		WORD*              t_ptrIdx = NULL;
		t_hr = m_ptrPMesh->LockVertexBuffer( D3DLOCK_READONLY ,(LPVOID*)&t_ptrVer );
		osassert( !FAILED( t_hr ) );
		t_hr = m_ptrPMesh->LockIndexBuffer( D3DLOCK_READONLY ,(LPVOID*)&t_ptrIdx );
		osassert( !FAILED( t_hr ) );

		for( int t_i = 0;t_i<t_iNumFace;t_i ++ )
		{
			osVec3Cross( &m_vecFaceNormal[t_i],
				&(t_ptrVer[t_ptrIdx[t_i*3+2]].m_vecPos - t_ptrVer[t_ptrIdx[t_i*3]].m_vecPos),
				&(t_ptrVer[t_ptrIdx[t_i*3+1]].m_vecPos - t_ptrVer[t_ptrIdx[t_i*3]].m_vecPos) );
			osVec3Normalize( &m_vecFaceNormal[t_i],&m_vecFaceNormal[t_i] );
		}

		m_ptrPMesh->UnlockVertexBuffer();
		m_ptrPMesh->UnlockIndexBuffer();
	}


# if CUSTOM_MESHDRAW
	// 
	//! ��������Լ���xMesh������Ⱦ�����ǵ���drawSubset,��Ҫ�õ���ص���Դ
	DWORD   t_dwAttNum;
	m_ptrPMesh->GetVertexBuffer( &m_ptrMeshVBuf );
	m_ptrPMesh->GetIndexBuffer(  &m_ptrMeshIBuf );
	m_ptrPMesh->GetAttributeTable( NULL,&t_dwAttNum );
	osassert( t_dwAttNum <= (DWORD)m_iShaNum );
	m_vecMeshAttribute.resize( t_dwAttNum );
	m_ptrPMesh->GetAttributeTable( &m_vecMeshAttribute[0],&t_dwAttNum );

# endif 


	m_pd3dDevice = _dev;
	strcpy( m_strMeshName ,_fname );
	m_dwHash = string_hash( _fname );

	return true;

	unguard;
}





/** \brief
*  ��Ⱦ��ǰ��Mesh.
*
*  \param _pos  �����λ������Ⱦ��ǰ��mesh.
*  ATTENTION:
*  River @ 2005-6-3: ��ʹ�õ�ǰ�������Ϊ���������Ʒ,���÷���
*                    ����Ϊ��Ʒ�Ļ�����.
*  
*/
bool osc_Mesh::render_mesh( os_meshRender* _mr,bool _alpha/* = false*/  )
{
	guard;

	int           t_i;
	osc_effect*   t_eff;

	osassert( _mr );

	// ���������Ƿ��ܻ�����Ӱ�죬���m_bUseDirLAsAmbient �����Ѿ����ˡ�
	if(m_bUseDirLAsAmbient){
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, -1);
	}else{
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
			osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
	}

	// ����ܶ�̬��Ӱ��
	if( _mr->m_sDLight.is_usedDl() )
		g_ptrDLightMgr->set_pointLight( _mr->m_sDLight );

	if( _alpha )
	{

		// ��Mesh��ÿһ��subset���д���
		for( t_i=0;t_i<m_iShaNum;t_i++ )
		{
			// �����ǰ��Ⱦalpha subset,���Թ�û��alpha��subset.
			if( !m_vecAlphaSubset[t_i] )
				continue;

			t_eff = g_shaderMgr->get_effect( this->m_vecShaderId[t_i] );
			osassert( t_eff );
			g_shaderMgr->set_EffectInfo( m_vecShaderId[t_i] );

			if( !g_frender->prepare_andrender( this->m_ptrPMesh,t_i,t_eff ) )
				return false;
		}

	}
	else
	{
		// ��Mesh��ÿһ��subset���д���
		for( t_i=0;t_i<this->m_iShaNum;t_i++ )
		{
			// �����ǰ����Ⱦalpha��subset,���Թ���alpha��subset..
			if( m_vecAlphaSubset[t_i] )
				continue;

			t_eff = g_shaderMgr->get_effect( this->m_vecShaderId[t_i] );
			osassert( t_eff );
			g_shaderMgr->set_EffectInfo( m_vecShaderId[t_i] );

			if( !g_frender->prepare_andrender( this->m_ptrPMesh,t_i,t_eff ) )
				return false;
		}
	}	


	// ����ܶ�̬��Ӱ��
	if( _mr->m_sDLight.is_usedDl() )
		g_ptrDLightMgr->set_pointLight( _mr->m_sDLight,false );

	return true;

	unguard;
}




//! ���Ե�ǰMesh����������Ŀ.
int osc_Mesh::get_meshTriNum( void )
{
	return this->m_ptrPMesh->GetNumFaces();
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    START osc_meshMgr


osc_meshMgr::osc_meshMgr()
{
	m_vecMesh.resize( MAXMESHNUM_INSCREEN );
	m_iRFaceNum = 0;

	m_vecAniMesh.resize( INIT_ANIMESHSIZE );
	m_bFirstProcessDLight = true;
	m_vecAniMeshIns.resize( MAXEFFECT_OSANUM );

	for( int t_i = 0;t_i<MAX_MESHRSTYLE;t_i ++ )
	{
		m_vecMeshInsPtr[t_i] = new WORD[INIT_ANIMESHSIZE];
		m_vecMeshInsNum[t_i] = 0;
	}
	m_vecColorMeshPtr = new WORD[INIT_ANIMESHSIZE];

	m_iRSBAniMeshId = -1;
	m_iRSBAniMeshAlphaId = -1;

	m_iRsbCVMeshAlphaId = -1;
	m_iRsbCVMeshId = -1;

	m_vecSceneAMIns = NULL;
}

// ˮ�涥���Declaration
extern LPDIRECT3DVERTEXDECLARATION9   g_pWaterVertexDeclaration;
extern LPDIRECT3DVERTEXDECLARATION9   g_pFogVertexDeclaration;

osc_meshMgr::~osc_meshMgr()
{
	os_stateBlockMgr* t_ptr = os_stateBlockMgr::Instance();

	if( m_iRSBAniMeshAlphaId >= 0 )
		t_ptr->release_stateBlock( m_iRSBAniMeshAlphaId );
	if( m_iRSBAniMeshId>= 0 )
		t_ptr->release_stateBlock( m_iRSBAniMeshId );
	if( m_iRsbCVMeshAlphaId >= 0 )
		t_ptr->release_stateBlock( m_iRsbCVMeshAlphaId );
	if( m_iRsbCVMeshId>= 0 )
		t_ptr->release_stateBlock( m_iRsbCVMeshId );

	// River @ 2008-12-16: �ͷ�ˮ���Declaration
	SAFE_RELEASE( g_pWaterVertexDeclaration );
	SAFE_RELEASE(g_pFogVertexDeclaration);

	for( int t_i = 0;t_i<MAX_MESHRSTYLE;t_i ++ )
		SAFE_DELETE_ARRAY( m_vecMeshInsPtr[t_i] );
	SAFE_DELETE_ARRAY( m_vecColorMeshPtr );


	m_vecSceneAMIns.destroy_mgr();
}


//! �ͷŵ�ǰ��meshMgr.
void osc_meshMgr::release_meshMgr( bool _finalRelease/* = true*/ )
{
	guard;

	CSpaceMgr<osc_Mesh>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecMesh.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecMesh.next_validNode( &t_ptrNode ) )
	{
		if( !t_ptrNode.p->is_inUse() )
			continue;

		t_ptrNode.p->release_mesh();
		m_vecMesh.release_node( t_ptrNode.idx );
	}

	//
	// �����finalRelease,���ͷ����е�mesh,���ͷ�mesh��Ӧ�Ŀռ䡣
	if( _finalRelease )
		m_vecMesh.destroy_mgr();

	unguard;
}





//! ��ʼ����ǰMeshManager.
bool osc_meshMgr::init_meshMgr( LPDIRECT3DDEVICE9 _dev,osc_middlePipe* _pipe  )
{
	guard;

	osassert( _dev );
	osassert( _pipe );
    
	m_pd3dDevice = _dev;
	m_ptrMPipe = _pipe;

	m_vecSceneAMIns.resize( MAXEFFECT_OSANUM );

	osc_aniMeshIns::m_ptrMPipe = _pipe;

	return true;

	unguard;
}


/** \brief
 *  ���ļ�����һ��mesh.
 *
 *  ATTENTION TO FIX: ������Զ�����滻����ǰ��ͼ���Ѿ������Obj
 */
int  osc_meshMgr::creat_meshFromFile( const char* _filename,
									 BOOL& _replace  )
{
	guard;

	int           t_idx;
	DWORD         t_dwHash = string_hash( _filename );

	osassert( _filename );

	_replace = FALSE;
	// 
	// ����ڴ����Ѿ���������Ҫ������mesh,�������mesh��ID.
	CSpaceMgr<osc_Mesh>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecMesh.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecMesh.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->is_inUse() );
		osassert( t_ptrNode.p->get_hashVal() != 0 );

		if( t_ptrNode.p->get_hashVal() == t_dwHash )		
			return t_ptrNode.idx;
	}

	// �õ���ǰҪ������mesh��id.
	t_idx = m_vecMesh.get_freeNode();
	if( !m_vecMesh[t_idx].create_meshFromFile( _filename,m_pd3dDevice ) )
		return -1;

	return t_idx;

	unguard;
}


//! ȫ���ͷž�̬��mesh.
void osc_meshMgr::reset_staticMesh( void )
{
	guard;

	CSpaceMgr<osc_Mesh>::NODE  t_ptrNode;

	// River 2010-12-30:����һ������������MAX_SWAPMESHNUM����Ŀ��
	//                  mesh,������Զ��פ�����ڴ��С�
	int t_iSMeshNum = 0;
	int t_iInMemoyMeshNum = 0;
	for( t_ptrNode = m_vecMesh.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecMesh.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->is_inUse() );

		if( t_iSMeshNum >= MAX_SWAPMESHNUM )
		{
			t_ptrNode.p->set_inMemory();
			t_iInMemoyMeshNum ++;
			continue;
		}

		if( t_ptrNode.p->get_inMemory() )
		{
			t_iInMemoyMeshNum ++;
			continue;
		}
		else
		{
			t_ptrNode.p->release_mesh();
			m_vecMesh.release_node( t_ptrNode.idx );
			t_iSMeshNum ++;
		}
	}

	osDebugOut( "�ͷ�<%d>����̬��mesh.��פ�ڴ�<%d>��Mesh\n",t_iSMeshNum,t_iInMemoyMeshNum );

	unguard;
}

//! ȫ���ͷ�osaMesh.
void osc_meshMgr::reset_osaMesh( void )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	CSpaceMgr<osc_aniMeshIns>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecSceneAMIns.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecSceneAMIns.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->is_amInsInuse() );
		if( !t_ptrNode.p->get_insInit() )
			continue;

		t_ptrNode.p->delete_curIns();
		m_vecSceneAMIns.release_node( t_ptrNode.idx );
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	unguard;
}



//! ��meshMgr��aniMeshIns��Ⱦ�����м��볡����Ҫ��Ⱦ��aniMeshIns.
void osc_meshMgr::add_sceAniMeshIns( int _idx )
{
	guard;

	int   t_idx = WORD(_idx) - MAXEFFECT_OSANUM;
	int   t_k;
	
	osassert( t_idx >= 0 );
	osassert( m_vecSceneAMIns[t_idx].is_amInsInuse() );

	m_iRAniMeshInsNum ++;

	//
	// �����ǰ��mesh�ǵ���ɫ��Mesh.
	if( m_vecSceneAMIns[t_idx].is_colorVertexMesh() )
	{
		m_vecColorMeshPtr[m_iColorVerMeshNum] = _idx;
		m_iColorVerMeshNum ++;
		return;
	}

	// �������Ұ�ڣ����ݲ�ͬ����Ⱦ���ͣ�����ins��������ͬ�������С�
	t_k = m_vecSceneAMIns[t_idx].get_RenderStyle();

	osassert( m_vecMeshInsNum[t_k] < INIT_ANIMESHSIZE );
	m_vecMeshInsPtr[t_k][m_vecMeshInsNum[t_k]] = WORD(_idx);
	m_vecMeshInsNum[t_k] ++;

	unguard;
}



/** \brief
*  ������Ani����Ⱦ���У�
*
*  �൱�ڸ���ͬ�Ķ���ani��Ⱦ��ʽ��������,�Լ�����Ⱦ״̬�ĸı������
*/
void osc_meshMgr::sort_aniMeshPtr( os_ViewFrustum* _vf )
{
	guard;

	osassert( _vf );

	float   t_fTime = sg_timer::Instance()->get_lastelatime();

	// River @ 2010-9-27:ȷ�������ͷ��߳���˺�������ͬʱ�ͷš�
	// ������ܻ�����refС��������η�����
	CCriticalSec  t_osaCS( &osc_mapLoadThread::m_sOsaCS );

	int   t_idx = 0;
	CSpaceMgr<osc_aniMeshIns>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecAniMeshIns.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecAniMeshIns.next_validNode( &t_ptrNode ) )
	{
		// River @ 2006-7-17:���amIns����ʹ���У�ɾ��
		//osassert( t_ptrNode.p->is_amInsInuse() );
		if( !t_ptrNode.p->get_insInit() )
			continue;

		if( !t_ptrNode.p->is_amInsInuse() )
		{
			m_vecAniMeshIns.release_node( t_ptrNode.idx );
			continue;
		}

		// River @ 2009-12-31:
		// top render layer����Ҫ�жϿɼ��ԣ�ȫ���ɼ�
		if( !t_ptrNode.p->get_topLayer() )
		{
			// �鿴��ǰ��aniMeshIns�Ƿ�����Ұ��.
			if( !_vf->objInVF( *t_ptrNode.p->get_bsphere() ) )
			{
				// River mod @ 2010-5-7:
				osassert( t_ptrNode.p->is_amInsInuse() );

				// River added @ 2007-6-25:���������Ұ�ڣ��������osa�������ڲ�����
				// ������ɼ�������£��������µ���Ҫ�ͷţ��ͷ����osa����
				t_ptrNode.p->update_osaAnimation( t_fTime );
				if( !t_ptrNode.p->is_amInsInuse() )
					m_vecAniMeshIns.release_node( t_ptrNode.idx );

				continue;
			}
		}

		m_iRAniMeshInsNum ++;

		// �����ǰ��mesh�ǵ���ɫ��Mesh.
		if( t_ptrNode.p->is_colorVertexMesh() )
		{
			m_vecColorMeshPtr[m_iColorVerMeshNum] = t_ptrNode.idx;
			m_iColorVerMeshNum ++;
			continue;
		}

		// �������Ұ�ڣ����ݲ�ͬ����Ⱦ���ͣ�����ins��������ͬ�������С�
		t_idx = t_ptrNode.p->get_RenderStyle();

		m_vecMeshInsPtr[t_idx][m_vecMeshInsNum[t_idx]] = t_ptrNode.idx;
		m_vecMeshInsNum[t_idx] ++;
	}

	unguard;
}


void osc_meshMgr::set_colorVertexRState( bool _alpha )
{
	guard;

	os_stateBlockMgr*   t_ptrSBM = os_stateBlockMgr::Instance();
	if( _alpha )
	{
		if( (m_iRsbCVMeshAlphaId >= 0)&&
			(t_ptrSBM->validate_stateBlockId( m_iRsbCVMeshAlphaId )) )
		{
			t_ptrSBM->apply_stateBlock( m_iRsbCVMeshAlphaId );
			return;
		}
	}
	else
	{
		if( (m_iRsbCVMeshId >= 0)&&(t_ptrSBM->validate_stateBlockId(m_iRsbCVMeshId)) )
		{
			t_ptrSBM->apply_stateBlock( m_iRsbCVMeshId );
			return;
		}
	}

	create_colorVertexRSB( _alpha );
	set_colorVertexRState( _alpha );

	unguard;
}

void osc_meshMgr::create_colorVertexRSB( bool _alpha )
{
	guard;
	osassert( m_pd3dDevice );
	os_stateBlockMgr*  t_ptrSBMgr = os_stateBlockMgr::Instance();

	if( _alpha )
	{
		m_iRsbCVMeshAlphaId = t_ptrSBMgr->create_stateBlock();

		t_ptrSBMgr->start_stateBlock();

		m_pd3dDevice->SetFVF( FVF_OBJECTCOLVERTEX );
		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );

		m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );

		m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,GLOBAL_MIDALPHAREF );

		t_ptrSBMgr->end_stateBlock( m_iRsbCVMeshAlphaId );
	}
	else
	{
		m_iRsbCVMeshId = t_ptrSBMgr->create_stateBlock();

		t_ptrSBMgr->start_stateBlock();

		m_pd3dDevice->SetFVF( FVF_OBJECTCOLVERTEX );
		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );

		t_ptrSBMgr->end_stateBlock( m_iRsbCVMeshId );
	}

	unguard;
}


/** \brief
 *   ��Ⱦ����ɫ��ص�Mesh.
 *
 *   ATTENTION TO OPP:������ܵĻ�������VertexShader
 *   VertexShader���ݵ�ǰ�Ķ������µĻ�������ɫ���õ�
 *  ��������Ⱦ��Ʒʱ�Ĺ��ߵ���ɫ
 *  
 */
void osc_meshMgr::render_colorVertexMesh( bool _alpha )
{
	guard;

	if( m_iColorVerMeshNum <= 0 )
		return ;

	float   t_fEtime = sg_timer::Instance()->get_lastelatime();


	//@{
	//! River @ 2010-6-22:��Ⱦ��Ļ�ƿ�Ч��,����backSurface��alphaΪ����ɫ
	for( int t_j = 0;t_j<m_iColorVerMeshNum;t_j ++ )
	{
		int t_idx = m_vecColorMeshPtr[t_j];

		if( t_idx == WORD(-1) )
			continue;

		if( t_idx < MAXEFFECT_OSANUM )
		{		
			osc_d3dManager*  t_ptrMgr1 = (osc_d3dManager*)::get_deviceManagerPtr();
			t_ptrMgr1->draw_fullscrAlpha( 0.0f );	
			//! ��Ǵ˱���������Ⱦ��������Ⱦ��Ļ�Ⲩ
			g_bRenderScreenWave = TRUE;
			break;
		}
	}
	//@}


	set_colorVertexRState( _alpha );

	// ������Ⱦÿһ��instanceMesh���ݡ�
	for( int t_j = 0;t_j<m_iColorVerMeshNum;t_j ++ )
	{
		int t_idx = m_vecColorMeshPtr[t_j];

		if( t_idx == WORD(-1) )
			continue;

		if( t_idx < MAXEFFECT_OSANUM )
		{
			m_vecAniMeshIns[t_idx].m_bIsScreenWave = true;
			m_vecAniMeshIns[t_idx].render_aniMeshIns( t_fEtime,_alpha );
			if( !m_vecAniMeshIns[t_idx].is_amInsInuse() )
			{
				m_vecAniMeshIns.release_node( t_idx );
				m_vecColorMeshPtr[t_j] = -1;
			}

			// River added @ 2010-6-23:��Ļ����Ч���ļ���.
			// �����Ż���ʹ��d3dCaps���õ�д��Щֵ�Ƿ�ɹ�
			m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 
				D3DCOLORWRITEENABLE_BLUE |
				D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED    );

		}
		else
		{
			//! River @ 2009-6-18:��Ե�����д��󣬴˴�����csһ��
			//��River @ 2011-1-30:��Ե�Ƕ�̬�����ͼ����Q���Ǿ�̬�ĵ����ͼ������
			//                    �˴�����Ҫ��CS��
			//::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );

			m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].
		     	   render_aniMeshIns( t_fEtime,_alpha );
			if( !m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_amInsInuse() )
			{
				m_vecSceneAMIns.release_node( t_idx-MAXEFFECT_OSANUM );
				m_vecColorMeshPtr[t_j] = -1;
			}

			//::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
		}
	}


	unguard;
}





//! ��Ⱦ���ǵõ���aniMeshInstance�Ķ��С�
bool osc_meshMgr::render_aniMeshInsArr( bool _alpha,bool _alphaTestOpatity,
							os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;
	
	float   t_fEtime = sg_timer::Instance()->get_lastelatime();

	osassertex( t_fEtime >= 0.0f,va("The Neg Etime is:<%f>..\n",t_fEtime) );


	// SrcBlend��destBlend��ֵ��
	static  DWORD   t_sSrcBlend[MAX_MESHRSTYLE] = 
	{
		D3DBLEND_SRCALPHA,D3DBLEND_SRCALPHA,
		D3DBLEND_SRCALPHA,D3DBLEND_SRCALPHA,
		D3DBLEND_ONE,D3DBLEND_ONE
	};
	static DWORD   t_sDstBlend[MAX_MESHRSTYLE] =
	{
		D3DBLEND_INVSRCALPHA,D3DBLEND_INVSRCALPHA,
		D3DBLEND_ONE,D3DBLEND_ONE,D3DBLEND_ONE,D3DBLEND_ONE
	};
	

	for( int t_i = 0;t_i<MAX_MESHRSTYLE;t_i ++ )
	{
		if( m_vecMeshInsNum[t_i] <= 0 )
			continue;

		//
		// ���ݲ�ͬ���ܺϷ�ʽ�����ò�ͬ����Ⱦ״̬��
		if( _alpha )
		{
			m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,t_sDstBlend[t_i] );
			m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,t_sSrcBlend[t_i] );
		}

		// �����ǲ��Ǵ���Alpha״̬������Ҫ�����Ƿ����дZBuffer��
		// ���粻������ȥ��ˮ�棬�ǲ�͸���ģ���Ҳ����д��Zbuffer.
		if( _alpha )
		{
			if( (t_i%2)== 0 )
				m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
			else
				m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );

		}
		else // windy add 	
		{
			m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		}

		// 
		// ��ʼ��ȾaniMeshInstance,�����ٽ�������ȾaniMeshʱ������ɾ���ʹ���Mesh
		::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );

		for( int t_j = 0;t_j<m_vecMeshInsNum[t_i];t_j ++ )
		{
			int t_idx = m_vecMeshInsPtr[t_i][t_j];

			if( t_idx == WORD(-1) )
				continue;

			if( t_idx < MAXEFFECT_OSANUM )
			{
				
				//  River @ 2007-5-10:
				//  ��Ϊ���̵߳Ĺ�ϵ��һЩaniMeshIns�����Ѿ��ͷţ�����Ҫ��validate.
				if( !m_vecAniMeshIns.validate_id( t_idx ) )
					continue;

				// �������״̬
				if( m_vecAniMeshIns[t_idx].get_hide() )
					continue;

				// top zlayer��AniMesh���������Ⱦ,commonRender��ȫ����Ⱦ
				if( _rtype != OS_COMMONRENDER )
				{
					if( _rtype == OS_RENDER_LAYER )
					{
						if( m_vecAniMeshIns[t_idx].get_topLayer() )
							continue;
					}
					else
					{
						if( !m_vecAniMeshIns[t_idx].get_topLayer() )
							continue;

						//! River @ 2010-1-20:ȷ��������������Ч��ȷ��Ⱦ��
						if( m_vecAniMeshIns[t_idx].m_bRenderedInCurFrame )
							continue;
					}
				}

				m_vecAniMeshIns[t_idx].render_aniMeshIns( t_fEtime,_alpha );

				if( !m_vecAniMeshIns[t_idx].is_amInsInuse() )
				{
					m_vecAniMeshIns.release_node( t_idx );
					m_vecMeshInsPtr[t_i][t_j] = -1;
				}

				//  
				//! River @ 2010-1-20:ȷ��������������Ч��ȷ��Ⱦ
				if( _alpha )
					m_vecAniMeshIns[t_idx].m_bRenderedInCurFrame = true;

			}
			else
			{
				
				// top Layer��Ⱦ����Ҫ��Ⱦ�����ڵ�osa��Ч
				if( _rtype > OS_RENDER_LAYER )
					continue;

				//  River @ 2007-5-10:
				//  ��Ϊ���̵߳Ĺ�ϵ��һЩaniMeshIns�����Ѿ��ͷţ�����Ҫ��validate.
				if( !m_vecSceneAMIns.validate_id( t_idx-MAXEFFECT_OSANUM ) )
					continue;
				
				// River Mod @ 2008-12-10:ˮ�治�ڴ���Ⱦ 
				if( g_bWaterReflect &&
					(m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_waterAni()) )
					continue;

				///!���ڴ���Ⱦ
				if(m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_FogAni()) 
					continue;


				m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].
					     render_aniMeshIns( t_fEtime,_alpha );

				if( !m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_amInsInuse() )
				{
					m_vecSceneAMIns.release_node( t_idx-MAXEFFECT_OSANUM );
					m_vecMeshInsPtr[t_i][t_j] = -1;
				}
			}
		}
		::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );


	}

	return true;

	unguard;
}


//! ����aniMesh��Ⱦʱʹ�õ���Ⱦ״̬��.
void osc_meshMgr::create_amRSB( bool _alpha )
{
	guard;

	osassert( m_pd3dDevice );
	os_stateBlockMgr*  t_ptrSBMgr = os_stateBlockMgr::Instance();

	if( _alpha )
	{
		m_iRSBAniMeshAlphaId = t_ptrSBMgr->create_stateBlock();

		t_ptrSBMgr->start_stateBlock();

		m_pd3dDevice->SetFVF( FVF_OBJECTVERTEX );
		//m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,TRUE );

		// 
		// ��ȾAniMesh��ʱ������SampleState
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );

		m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );

		//@{
		//m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,GLOBAL_MINALPHAREF );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, GLOBAL_MINALPHAREF );
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);		
		//@}


		// ֻ����ʹ��none cull����������Ч�ĵ���Ч�������ԣ�������������һЩ��
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_CW );
		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,TRUE );

		// Ĭ��ʹ��Clamp������ʽ
		m_ptrMPipe->set_ssClamp();
	
		// ����Ⱦ͸����aniMesh��ʱ��ʹFog Enable
		m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,TRUE );

		t_ptrSBMgr->end_stateBlock( m_iRSBAniMeshAlphaId );

	}
	else
	{
		m_iRSBAniMeshId = t_ptrSBMgr->create_stateBlock();

		t_ptrSBMgr->start_stateBlock();

		m_pd3dDevice->SetFVF( FVF_OBJECTVERTEX );
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE );
		// 
		// ��ȾAniMesh��ʱ������SampleState
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_CW );

		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,TRUE );


		t_ptrSBMgr->end_stateBlock( m_iRSBAniMeshId );
	}

	unguard;
}


void osc_meshMgr::set_animeshRState( bool _alpha )
{
	guard;

	os_stateBlockMgr*  t_ptrSBM = os_stateBlockMgr::Instance();
	if( _alpha )
	{
		if( (m_iRSBAniMeshAlphaId >= 0)&&
			(t_ptrSBM->validate_stateBlockId( m_iRSBAniMeshAlphaId )) )
		{
			t_ptrSBM->apply_stateBlock( m_iRSBAniMeshAlphaId );
			return;
		}
	}
	else
	{
		if( (m_iRSBAniMeshId >= 0)&&
			(t_ptrSBM->validate_stateBlockId(m_iRSBAniMeshId)) )
		{
			t_ptrSBM->apply_stateBlock( m_iRSBAniMeshId );
			return;
		}
	}

	create_amRSB( _alpha );
	set_animeshRState( _alpha );

	unguard;
}



/** \brief
*  ��Ⱦ��̬��mesh.
*  
*  ���mesh�Ƿ���vf�У�������alpha����������Ⱦ��
*/
bool osc_meshMgr::render_aniMesh( os_ViewFrustum* _vf,
		bool _alpha,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;

	//
	// �ȶ�����Ҫ��Ⱦ��AniMesh������
	if( (!_alpha) && (_rtype != OS_RENDER_TOPLAYER) )
		sort_aniMeshPtr( _vf );
	if( m_iRAniMeshInsNum <= 0 )
		return true;

	set_animeshRState( _alpha );

	//@{ 
	//  River added @ 2008-12-23:��������Ʒ��͸��һ������������Ⱦ����������
	//  ��ȷ����Ⱦ����͸������Ʒ
	//  ����������һ������Ⱦ����ʡ������������Ҫ��������Դ��ʱ�򣬶�ע��һЩ��
	if( !render_aniMeshInsArr( _alpha,true,_rtype ) )
		return false;
	//@} 

	//
	// ��Ⱦ���ŵ�Mesh���ݡ�
	// �Ƕ�����Ⱦ��ʱ����Ⱦ����ɫ��Ʒ
	if( _rtype != OS_RENDER_TOPLAYER )
		render_colorVertexMesh( _alpha );

	// ʹ��һ֡��������������Ʒ�Ķ�̬�⡣
	m_bFirstProcessDLight = true;

	return true;

	unguard;
}


/** \brief
*  ��Ⱦһ��Mesh,ʹ��meshID�Ϳ�����
* 
*  ��ָ����λ������Ⱦ��ǰ��Mesh.
*  �Ժ�����ڴ˴���ģ�͵�lod��Ϣ��
*  
*/
bool osc_meshMgr::render_mesh( os_meshRender* _mr,bool _alpha/* = false */ )			
{
	guard;

	osassert( _mr );

	// ������Ⱦ��Ҫ��ȫ�������á�

	mesh_renderSet( _mr );

	// ͳ����Ⱦ����.
	if( !_alpha )
		m_iRFaceNum += m_vecMesh[_mr->m_iMeshId].get_meshTriNum();

	// ��Ⱦ.
	if( !m_vecMesh[_mr->m_iMeshId].render_mesh( _mr,_alpha ) )
		return false;

	return true;

	unguard;
}



//! �ӵ�ǰ��aniMeshArray�еõ����Ǵ���һ��osc_aniMesh.
osc_aniMesh* osc_meshMgr::get_aniMeshFromArr( os_aniMeshCreate* _mc,BOOL& _replace )
{
	guard;

	int        t_idx;
	DWORD      t_dwHash = string_hash( _mc->m_szAMName );

	osassert( _mc );

	_replace = FALSE;

	// 
	// �����aniMesh�п����ҵ���ǰҪ������Mesh,����Ҫ�����µ�mesh.
	CSpaceMgr<osc_aniMesh>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecAniMesh.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecAniMesh.next_validNode( &t_ptrNode ) )
	{
		// 
		if( !t_ptrNode.p->is_aniMeshInUse() )
		{
			m_vecAniMesh.release_node( t_ptrNode.idx );
			continue;
		}

		osassert( t_ptrNode.p->get_nameHash() != 0 );
		if( t_ptrNode.p->get_nameHash() == t_dwHash )
		{
			osassert( 0==strcmp( _mc->m_szAMName,t_ptrNode.p->get_meshName() ) );
			return t_ptrNode.p;
		}
	}

	t_idx = m_vecAniMesh.get_freeNode();

	// �����ǵõ���meshָ���д���mesh���ݡ�
	if( !m_vecAniMesh[t_idx].create_meshFromfile( *_mc ) )
		osassertex( false,va( "����<%s>osaMesh����..\n",_mc->m_szAMName ) );

	osassert( m_vecAniMesh[t_idx].get_rootNodePtr()->m_arrNodelist );

	return &m_vecAniMesh[t_idx];

	unguard;
}


/** \brief
*  �Ӵ����ṹ�д������ǵ�aniMesh.
*
*  ATTENTION TO FIX: ����²��滻��һaniMeshIns�����ϲ��ٴ�ʹ�õ�ǰ��idʱ
*  ����ִ��󣬴������������
*  River: ͨ���ϳ�ID�Ļ��ƣ��������Ѿ������
*  
*/
int osc_meshMgr::create_aniMeshFromFile( os_aniMeshCreate* _mc,
				BOOL& _replace,bool _sceC/* = false*/,bool _topLayer/* = false*/ )
{
	guard;

	osc_aniMesh*    t_meshPtr;
	int            t_iIdx = -1;
	WORD            t_wCreateId = 0;

	osassert( _mc );
	::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );
	t_meshPtr = get_aniMeshFromArr( _mc,_replace );
	if( !_mc->m_bCreateIns )
	{
		t_meshPtr->set_swap( false );
		t_meshPtr->add_ref();
		::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
		return -1;
	}

	//
	// ����滻ĳһ��mesh�����ģ���������meshInstance Array.
	osassert( (!_replace) );
	osassert( t_meshPtr->get_rootNodePtr()->m_arrNodelist );

	//
	// ����Ǵӳ����д����Ķ�����Ʒ,��ʹ���µĶ�����Ʒ���С�
	if( _sceC )
	{
		t_iIdx = m_vecSceneAMIns.get_freeNode();
		::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );

		t_wCreateId = m_vecSceneAMIns[t_iIdx].create_aniMeshIns( t_meshPtr,*_mc );
		t_iIdx =  MAXEFFECT_OSANUM+t_iIdx;	
	}
	else
	{
		t_iIdx = m_vecAniMeshIns.get_freeNode();
		if( t_iIdx >= MAXEFFECT_OSANUM )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
			osassertex( false,"�������ܴ��������osa.....\n" );
			return -1;
		}
		// River @ 2011-2-26:�Ƶ����Ϊ�˼����߳�ͬ���ĵȴ�ʱ�䡣
		::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );

		t_wCreateId = m_vecAniMeshIns[t_iIdx].create_aniMeshIns( t_meshPtr,*_mc,_topLayer );
	}

	//! �����ˮ���mesh,��õ�ˮ���effect.
	if( (!m_ptrWaterEffect)&& t_meshPtr && _mc->m_bWaterAni )
		m_ptrWaterEffect = t_meshPtr->get_effect();


	return osn_mathFunc::syn_dword( t_wCreateId,t_iIdx );

	unguard;
}

const char* osc_meshMgr::get_aniNameFromId( int _id )
{
	guard;

	int  t_id = WORD(_id);
	static char  t_sz[4];

	// River @ 2011-4-11:�����жϣ�ȷ���˴���ID�������
	if( m_vecAniMeshIns.validate_id( t_id ) )
	{
		if( m_vecAniMeshIns[t_id].validate_cval( _id ) )
		{
			if( m_vecAniMeshIns[t_id].m_ptrMesh )
				return m_vecAniMeshIns[t_id].m_ptrMesh->get_meshName();
		}
	}
	
	t_sz[0] = NULL;
	return t_sz;

	unguard;
}

//! ɾ��һ������.
void osc_meshMgr::delete_animationMesh( int _id,bool _sceC/* = false*/ )
{
	guard;

	int  t_id = WORD(_id);

	::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );
	if( _sceC )
	{
		t_id -= MAXEFFECT_OSANUM;

		// 
		// ATTENTION TO FIX:����˴��Ĺ�ϵ���������µ����ͼ��Ӱ��˴��Ľ�׳�ԡ�
		if( t_id >= MAXMESHINS_PERSCE )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
			return;
		}

		osassert( (t_id < MAXMESHINS_PERSCE) && m_vecSceneAMIns[t_id].is_amInsInuse() );
	}
	else
	{
		// River @ 2011-10-18:ȥ����assert.
		//osassertex( (t_id < MAXEFFECT_OSANUM),"osaObject ID ����..\n"  );
		if( t_id >= MAXEFFECT_OSANUM )
			return;

		osassertex( (t_id >=0 ),"osaObject ID ����Ϊ��..\n" );
		//osassertex( m_vecAniMeshIns[t_id].is_amInsInuse(),"osaObject is not Inuse!!!" );

		//! River @ 2011-3-24: �˴��ڼ��˵�����£����ǻ���˽�㱻ɾ�����Σ���ȥ��OK.
		//                     �����жϣ���������˵.
		if( m_vecAniMeshIns.validate_id( t_id ) )
		{
			// ATTENTION TO FIX:Ϊʲô�ᱻɾ��???
			if( !m_vecAniMeshIns[t_id].is_amInsInuse() )
			{
				::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
				return;
			}
		}
		else
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
			return;
		}
	}

	if( _sceC )
	{
		if( m_vecSceneAMIns[t_id].validate_cval( _id ) )
		{
			m_vecSceneAMIns[t_id].delete_curIns();
			m_vecSceneAMIns.release_node( t_id );
		}
	}
	else
	{
		if( m_vecAniMeshIns[t_id].validate_cval( _id ) )
		{
			m_vecAniMeshIns[t_id].delete_curIns();
			m_vecAniMeshIns.release_node( t_id );
		}
	}
	
	::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	unguard;
}



/** \brief
*  ����aniMesh�Ķ�̬�����⡣��Ҫ�Գ����е�ÿһ����̬��������������
*
*  ��̬��ȫ����ȫ�ֹ�����ģ�⡣
*  
*  ATTENTION TO OPP: �õ�����Ұ�ڵ���Ʒ�б�Ȼ�����б��ڵ���Ʒ��
*/
bool osc_meshMgr::process_aniMeshDlight( os_dLightData& _dlight )
{
	guard;

	osc_aniMeshIns*   t_ptrMins;
	os_bsphere*       t_ptrBs;
	osVec3D           t_vec3;
	float             t_dis;
	osColor           t_srcColor,t_dstColor;

	CSpaceMgr<osc_aniMeshIns>::NODE  t_ptrNode;
	//
	// ����ڵ�ǰ֡���ǵ�һ�δ���̬�⣬������е�aniMeshIns����
	// ȫ�ֹ⣬���Ժ�Ĵ����У���̬��ֻ���ڵ�ǰ��Ʒ��ȵĻ�������ӣ�
	// �������衣
	if( m_bFirstProcessDLight )
	{
		m_bFirstProcessDLight = false;
		for( t_ptrNode = m_vecAniMeshIns.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = m_vecAniMeshIns.next_validNode( &t_ptrNode ) )
		{
			// 
			if( !t_ptrNode.p->get_insInit() )
				continue;

			// River mod @ 2010-9-28:������.
			if( !t_ptrNode.p->is_amInsInuse() )
				continue;

			t_ptrNode.p->set_light( _dlight.m_sAmbientLight );
		}
	}

	//
	// ����ÿһ����Ʒ�ľ��룬�Զ�̬����д���
	for( t_ptrNode = m_vecAniMeshIns.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecAniMeshIns.next_validNode( &t_ptrNode ) )
	{
		// 
		if( !t_ptrNode.p->get_insInit() )
			continue;

		// River mod @ 2010-9-28:������.
		if( !t_ptrNode.p->is_amInsInuse() )
			continue;

		t_ptrMins = &m_vecAniMeshIns[t_ptrNode.idx];
		t_ptrBs = t_ptrMins->get_bsphere();
		t_dis = osVec3Length( 
			&(t_ptrBs->veccen - _dlight.m_vec3DLightPos) );

		if( t_dis >= _dlight.m_fAttenuationDis )
			continue;

		//
		//  ���Լ��㶯̬�⡣
		t_dis /= _dlight.m_fAttenuationDis;
		t_dis = 1.0f - t_dis;

		t_srcColor = osColor( t_ptrMins->get_light() );
		t_dstColor = _dlight.m_sDLight;

		osColorLerp( &t_dstColor,&t_srcColor,&t_dstColor,t_dis );

		t_ptrMins->set_light( t_dstColor );
	}

	return true;

	unguard;
}

/** \brief
*  ����Ⱦһ��Mesh֮ǰ�Դ�������ݽ������á�
* 
*  ��ǰ��Ҫ�������������ת������
*  
*  River mod @ 2009-4-16:����ÿ��Ʒ�����Ŵ���
*  
*  \param _mr  ��Ҫ���õ����ݡ�
*/
void osc_meshMgr::mesh_renderSet( os_meshRender* _mr,osMatrix* pOut )
{	
	guard;

	osMatrix  t_mat,t_matScale;
	osassert( _mr );

	osMatrixScaling( &t_matScale,_mr->m_vec3Scale.x,
		_mr->m_vec3Scale.y,_mr->m_vec3Scale.z );

	///@{ Ϊ�˽�ʡģ�ͣ���Щģ��ֻ�Ǿ������ԾͰ�x��������д��-1�Ϳ���ʵ�֣���Ҫ��CullMode��һ��.
	static bool t_bLastCCWSet = false;
	if (_mr->m_vec3Scale.x<0)
	{
		g_frender->get_d3dDevice()->SetRenderState( D3DRS_CULLMODE,D3DCULL_CCW );
		t_bLastCCWSet = true; 
	}
	else
	{
		if( t_bLastCCWSet )
		{
			g_frender->get_d3dDevice()->SetRenderState( D3DRS_CULLMODE,D3DCULL_CW );
			t_bLastCCWSet = false; 
		}
	}
	///@}

	osMatrixRotationY( &t_mat,_mr->m_fAngle );
	t_mat = t_matScale * t_mat;

	// ������Ʒ��λ��
	t_mat._41 = _mr->m_vec3Pos.x;
	t_mat._42 = _mr->m_vec3Pos.y;
	t_mat._43 = _mr->m_vec3Pos.z;
	if (pOut)
	{
		memcpy(pOut,t_mat,sizeof(osMatrix));
	}

	 

	g_frender->get_d3dDevice()->SetTransform( D3DTS_WORLD,&t_mat );

	//@{�����������

	osc_Mesh * t_mesh = &m_vecMesh[_mr->m_iMeshId];

	for( int t_i=0;t_i<t_mesh->m_iShaNum;t_i++ )
	{	// ��Mesh��ÿһ��subset���д���
		if( t_mesh->m_vecAlphaSubset[t_i] )
			continue;

		osc_effect* t_eff = g_shaderMgr->get_effect( t_mesh->m_vecShaderId[t_i] );
		if (t_eff)
		{
			t_eff->m_peffect->SetMatrix("WorlMatrix",&t_mat);
		}
	}
	//@}

	unguard;
}


// �Ƿ��Լ���֯ÿһ��Mesh����Ⱦ�����ٶ��������������������ȵ��л�ʱ��
# if CUSTOM_MESHDRAW 

/** \brief
 *  ������Ⱦ.ex�ļ���ʹ�����ڿ������ɸ������Ʒ
 *
 *  �㷨������
 *  һ: ���ڷ�͸������Ʒ������Ⱦ:
 *      1���ȸ�����Ʒ��id���������ϲ㴫��������Ѿ��Ź���
 *      2: ��Ⱦ��Ʒʱ���ֶ����ö�����������壬�Լ�shader�������ǰ����Ⱦ����һ����ͬ��
 *         ����Ҫ���ã�ֻ��Ҫ����dip�Ϳ����ˡ�
 *      3: ������ϴβ�ͬ������Ҫ����shader.
 *      4: ��ÿһ��meshRender�ṹ����Ҫ��������ת������
 *   
 *  ��������͸�����ֵ���Ʒ��Ⱦ��
 *      1: ������Ʒ��λ�ý�������,����������Ѿ��Ź���
 *      2: ��Ⱦʱ�������һ����Ⱦ����Ʒ����ǰ��Ʒid��ͬ����ʡȥ������������������á�
 *      3: ����ϴε�shaderId�͵�ǰ��shaderID��ͬ��ʡȥ���á�
 *
 *  FVF��Ҫ��ʼ��Ⱦʱ��һ��������.�����������е�.ex�ļ�ʹ����ͬ��fvfֵ��
 *  
 */
bool osc_meshMgr::render_meshVector( VEC_meshRender* _vec, int _mshCount, bool _alpha /*= false*/ )
{
	guard;

	osassert( _vec );
	osassert( _mshCount>= 0 );

	os_meshRender*   t_mr;
	osc_Mesh*        t_ptrMesh;
	int              t_i;

	osc_effect*   eff = 0;
	UINT          uPasses;

	int shaId = -1, t_iLastShaderId = -1;
	int effId = -1, t_iLastEffId = -1;
	int t_iLastMeshId = -1;


	//! river added @ 2009-2-19:����vbuffer Catch,
	//  ��ĳЩ����£����������ִ������ݡ� 
	g_frender->reset_vbufCatch();

	D3DXATTRIBUTERANGE*   t_ptrAtt;
	// ����mesh
	for( int i = 0; i < _mshCount; ++i )
	{
		t_mr = &((*_vec)[i]);
		t_ptrMesh = &m_vecMesh[t_mr->m_iMeshId];


		// ����ÿһ��renderSet��ص�����
		mesh_renderSet( t_mr );

		// River mod @ 2009-1-10:������������Ʒ��ʹ��dx mesh����Ⱦ��ʽ,����Ҫ���ö��������
		// ������
		if( t_ptrMesh->m_iShaNum <= (int)t_ptrMesh->m_vecMeshAttribute.size() )
		{
			if( t_mr->m_iMeshId != t_iLastMeshId )
			{
				t_iLastMeshId = t_mr->m_iMeshId;
				g_frender->set_streamsource( 0,sizeof( os_objectVertex ),t_ptrMesh->m_ptrMeshVBuf );
				g_frender->set_indexStream( t_ptrMesh->m_ptrMeshIBuf );
				m_ptrMPipe->set_fvf( t_ptrMesh->m_ptrPMesh->GetFVF() );
			}
		}
		else
			t_iLastMeshId = -1;


		// ͳ����Ⱦ����.
		if( !_alpha )
			m_iRFaceNum += t_ptrMesh->get_meshTriNum();

		// ����ܶ�̬��Ӱ��
		if( t_mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( t_mr->m_sDLight );

		// ����subset
		for( t_i=0;t_i< t_ptrMesh->m_iShaNum;t_i++ )
		{
			if( _alpha != t_ptrMesh->m_vecAlphaSubset[t_i] )
				continue;
		
			shaId = t_ptrMesh->m_vecShaderId[t_i];
			effId = g_shaderMgr->get_effectId( shaId );
			if( shaId != t_iLastShaderId )
			{
				if( effId != t_iLastEffId )
				{
					if( eff )eff->end();

					eff = g_shaderMgr->get_effect( shaId );
					osassert( eff );
					uPasses = eff->begin();

					t_iLastEffId = effId;
				}
				g_shaderMgr->set_EffectInfo( shaId );
				t_iLastShaderId = shaId;
			}

			t_ptrAtt = &t_ptrMesh->m_vecMeshAttribute[t_i];

			// ����pass
			for( int j = 0; j < (int)uPasses; ++j )
			{
				if( eff->pass( j ) )continue;


				// River mod @ 2009-1-10:�������������Ʒ��ʹ��dx mesh����Ⱦ��ʽ
				if( t_ptrMesh->m_iShaNum > (int)t_ptrMesh->m_vecMeshAttribute.size() )
				{				
					if( !g_frender->render( m_vecMesh[ t_mr->m_iMeshId ].m_ptrPMesh, t_i ) )
						return false;
				}
				else
				{
					g_frender->render( D3DPT_TRIANGLELIST,0,t_ptrAtt->VertexStart,
						t_ptrAtt->VertexCount,t_ptrAtt->FaceStart*3,t_ptrAtt->FaceCount );	
				}

				eff->endPass();

			}
		}

		// River @ 2006-3-27:�����ǰ��Meshʹ�÷������ȫ�ֹ⣬������ȫ�ֹ�
		// River @ 2006-6-21:�����Ʒʹ�õر���ɫ��Ϊ����ɫ��Ҳ��Ҫ����ȫ�ֹ�
// 		if( (t_mr->m_dwTerrColor != 0xffffffff) ||
// 			(t_ptrMesh->m_bUseDirLAsAmbient && _alpha) )
// 		{
// 				m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
// 					osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );			
// 		}

		// ����ܶ�̬��Ӱ��
		if( t_mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( t_mr->m_sDLight,false );	

	}

	if( eff )eff->end();
	
	return true;

	unguard;
}


# else

//! ��Ⱦһ��Mesh
bool osc_meshMgr::render_meshVector( VEC_meshRender* _vec, int _mshCount, bool _alpha /*= false*/ )
{
	guard;

	osassert( _vec );
	os_meshRender* mr;
	osc_Mesh*      t_ptrMesh;
	int           t_i;

	osc_effect* eff = 0;
	UINT      uPasses;

	int shaId = -1, perShaId = -1;
	int effId = -1, perEffId = -1;

	// ʹ��һ��״̬���������Ƿ��Ѿ��ǲ��ܻ�����Ӱ���ˡ�
	// ���٣�SetRenderState�ĵ���
	BOOL t_bConstAmbient = FALSE;

	// ����mesh
	for( int i = 0; i < _mshCount; ++i )
	{
		mr = &((*_vec)[i]);
		t_ptrMesh = &m_vecMesh[mr->m_iMeshId];
		mesh_renderSet( mr );

		/*
		//! river added @ 2007-2-8:�������Ʒ����Ⱦ͸������ʱ������Ҫд��zbuffer
		if( t_ptrMesh->is_treeObj() && _alpha )
			g_frender->get_d3dDevice()->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
		*/

		// ͳ����Ⱦ����.
		if( !_alpha )
			m_iRFaceNum += m_vecMesh[mr->m_iMeshId].get_meshTriNum();

		if(t_bConstAmbient != m_vecMesh[ mr->m_iMeshId ].m_bUseDirLAsAmbient){
			
			t_bConstAmbient = m_vecMesh[ mr->m_iMeshId ].m_bUseDirLAsAmbient;

			// ���������Ƿ��ܻ�����Ӱ�죬���m_bUseDirLAsAmbient �����Ѿ����ˡ�
			if(t_bConstAmbient){
				m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, -1);
			}else{
				m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
					osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
			}

		}
		

		// ����ܶ�̬��Ӱ��
		if( mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( mr->m_sDLight );


		// ����subset
		for( t_i=0;t_i< m_vecMesh[ mr->m_iMeshId ].m_iShaNum;t_i++ )
		{
			if( _alpha != m_vecMesh[ mr->m_iMeshId ].m_vecAlphaSubset[t_i] )
				continue;

			shaId = m_vecMesh[ mr->m_iMeshId ].m_vecShaderId[t_i];
			effId = g_shaderMgr->get_effectId( shaId );

			if( shaId != perShaId )
			{
				if( effId != perEffId )
				{
					if( eff )eff->end();

					eff = g_shaderMgr->get_effect( shaId );
					osassert( eff );
					uPasses = eff->begin();

					perEffId = effId;
				}

				g_shaderMgr->set_EffectInfo( shaId );
				perShaId = shaId;
			}

			// ����pass
			for( int j = 0; j < (int)uPasses; ++j )
			{
				if( eff->pass( j ) )continue;

				if( !g_frender->render( m_vecMesh[ mr->m_iMeshId ].m_ptrPMesh, t_i ) )
					return false;

				eff->endPass();
			}
		}

		


		// ����ܶ�̬��Ӱ��
		if( mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( mr->m_sDLight,false );

		/*
		//! river added @ 2007-2-8:�������Ʒ����Ⱦ͸������ʱ���ظ�д��zbuffer��״̬
		if( t_ptrMesh->is_treeObj() && _alpha )
			g_frender->get_d3dDevice()->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		*/

	}

	if( eff )eff->end();

	return true;

	unguard;
}


# endif 
bool osc_meshMgr::render_meshVectorNoMaterial( VEC_meshRender* _vec, int _mshCount,ID3DXEffect* _pEffect,osMatrix& _ViewProjMat )
{
	guard;

	osassert( _vec );
	os_meshRender* mr;
	osc_Mesh*      t_ptrMesh;
	int           t_i;

	osc_effect* eff = 0;
//	UINT      uPasses;

	int shaId = -1, perShaId = -1;
	int effId = -1, perEffId = -1;

	// ����mesh
	for( int i = 0; i < _mshCount; ++i )
	{
		mr = &((*_vec)[i]);
		t_ptrMesh = &m_vecMesh[mr->m_iMeshId];
		osMatrix mat;
		mesh_renderSet( mr , &mat);


		osMatrixMultiply( &mat,&mat,&_ViewProjMat );
		 

		UINT iPass, cPasses;
		_pEffect->SetMatrix("Matrix1",&mat);
		_pEffect->Begin(&cPasses, 0) ;


		for (iPass = 0; iPass < cPasses; iPass++)
		{
			_pEffect->BeginPass(iPass) ;

			for( t_i=0;t_i< m_vecMesh[ mr->m_iMeshId ].m_iShaNum;t_i++ )
			{
				g_frender->render( m_vecMesh[ mr->m_iMeshId ].m_ptrPMesh, t_i );
			}

			_pEffect->EndPass() ;
		}
		_pEffect->End() ;

	}


	return true;

	unguard;
}