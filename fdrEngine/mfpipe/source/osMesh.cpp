/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osMesh.cpp
 * 
 *  His:      River created @ 2003-12-27
 *
 *  Desc:     中间管道可接受的独立的渲染数据类型.a
 *  
 *  "罗马贵族总有一种强烈的竞争意识,这种不甘落后的意念构成了巨大的动力和自豪感。
 *   正是这种不愿让自己家族衰落的动力支撑着他们。"
 *  
 *  测试ss,再次测试ss.
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
//! 水面相关
osc_effect* osc_meshMgr::m_ptrWaterEffect = NULL;

//! 内存可交换的mesh数据，每一个场景，超过此数目的mesh,可常驻内存。
# define MAX_SWAPMESHNUM  100


// 
// 这个文件可以使用前端管道对物体进行渲染。 

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

	// 初始化为-1;
	for( int t_i = 0;t_i<MAXSUBSET_PERMESH;t_i ++ )
		m_vecShaderId[t_i] = -1;

	m_vecFaceNormal = NULL;

	m_bInMemoryForEver = false;

}

osc_Mesh::~osc_Mesh()
{
	release_mesh();
}


//! 释放当前Mesh的设备相关资源.
void osc_Mesh::release_mesh( void )
{
	// 释放从dxMesh内得到的顶点和索引缓冲区
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

	// 桥类物品处理
	if( m_bBridgeObj )
		SAFE_DELETE_ARRAY( m_vecFaceNormal );
}

//! 得到faceNormal.
bool osc_Mesh::get_faceNormal( int _faceIdx,osVec3D& _normal )
{
	guard;

	if( !m_bBridgeObj )
		return false;

	_normal = m_vecFaceNormal[_faceIdx];
	
	return true;

	unguard;
}


//! 输入一条射线,看是否和*.x的mesh文件相交.
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
	// 如果上层需要相交的位置,则返回.
	if( _resDis && t_bHit )
	{
		// River @ 2010-3-30:如果相交面的正面，则处理
		if( get_faceNormal( t_dwFaceIdx,t_vec3Normal ) )
		{
			if( osVec3Dot( &t_vec3Normal,&_rayDir ) <= 0.0f )
				*_resDis = t_dist;
		}
		else
			*_resDis = t_dist;
	}

	// ATTENTION TO OPP:
	// 检测是否和我们的射线段相交,
	// River @ 2009-2-6: 如果需要得到相交的位置矩离,则计算,否则根据结果直接返回.
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
					// River @ 2010-3-30:如果相交面的正面，则处理
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


//! 保存设备指针.
LPDIRECT3DDEVICE9 osc_Mesh::m_pd3dDevice = NULL;


/** \brief
*  从文件初始化一个Mesh.
*
*  \param _fname 要创建的文件的名字.
*  \param _dev   创建设备相关资源需要的3d设备。
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

	// 文件名过长?
	osassertex( strlen( _fname ) < MAXMESH_FILENAMELEN ,
		   va( "文件名<%s>过长,请联系程序...\n",_fname )  );

	if( _meshMem )
		t_ex.loadEXFromMem( _meshMem );
	else
	{
		if( !t_ex.LoadEXFile( (char*)_fname,
			_dev,t_pMtrlBuffer,t_numMtl,m_ptrPMesh ) )
		{
			char temp[250];
			sprintf(temp,"%s调入ex文件失败",_fname);
			osassertex(false,temp);
			return false;
		}
	}


	//
	// 得到Bounding相关数据。
	m_bsphere.veccen = t_ex.m_sBound.m_VECCenter;
	m_bsphere.radius = t_ex.m_sBound.m_fRadius;
	m_bbox.m_vecMax = t_ex.m_sBound.m_VECmax;
	m_bbox.m_vecMin = t_ex.m_sBound.m_VECmin;

	// 
	// 得到新的物品属性
	m_bVipObj = t_ex.m_sMeshVipInfo.m_bVipObj;
	m_bTreeObj = t_ex.m_sMeshVipInfo.m_bTreeObj;
	m_bBridgeObj = t_ex.m_sMeshVipInfo.m_bIsBridge;
	m_bAcceptFog = t_ex.m_sMeshVipInfo.m_AcceptFog;
	
	m_bUseDirLAsAmbient = t_ex.m_bUseDirLAsAmbient;


	//
	// 创建当前Mesh用到的shader.
	// ATTENTION: 使用地图预处理功能完成这些功能。
	D3DXMATERIAL*  t_d3dxMtrls = (D3DXMATERIAL*)t_pMtrlBuffer->GetBufferPointer();
	s_string       t_str;
	int           t_idx;


	m_iShaNum = t_numMtl;

# ifdef _DEBUG
	if( m_iShaNum > MAXSUBSET_PERMESH )
		MessageBox( NULL,_fname,"以下模型用到了超过12个的Mtl",MB_OK );
	osassert( m_iShaNum <= MAXSUBSET_PERMESH );
# endif 

	for( DWORD t_i=0;t_i<t_numMtl;t_i++ )
	{
		
# ifdef _DEBUG
		if( !t_d3dxMtrls[t_i].pTextureFilename )
		{
			char   t_str[256];
			sprintf( t_str,"模型 <%s> 中有没有使用纹理的材质",_fname );
			MessageBox( NULL,t_str,"ERROR",MB_OK );
			osassertex( false,va("模型 <%s> 中有没有使用纹理的材质",_fname) );
		}
# endif 

		t_str = t_d3dxMtrls[t_i].pTextureFilename;

		t_idx = (int)t_str.rfind( '.' );
		t_str = t_str.substr( 0,t_idx );
		t_idx = (int)t_str.rfind( '\\' );
		t_str = t_str.substr( t_idx+1,t_str.length() );

		/// 使用新的shader，如果创建失败，则暂且还用原来的方式，最终原来的方式应该删除
		m_vecShaderId[t_i] = g_shaderMgr->add_shader( 
			t_str.c_str(), osc_engine::get_shaderFile( "object" ).c_str() );
		//----可删除----
		if( m_vecShaderId[t_i] < 0 )
		{
			m_vecShaderId[t_i] = g_shaderMgr->add_shadertomanager( 
				(char*)t_str.c_str(),OBJECT_SHADERINI,osc_engine::m_iObjectShaId );
		}
		//--------------			

		// 得到当前的subset是否是alpha subset.
		m_vecAlphaSubset[t_i] = g_shaderMgr->has_alphaChannel( m_vecShaderId[t_i] );

	}

	//syq
	t_pMtrlBuffer->Release();



	//
	// 处理当前的mesh是否是alphaMesh.
	for( int t_i=0;t_i<(int)t_numMtl;t_i++ )
	{
		if( m_vecAlphaSubset[t_i] )
			m_bAlphaMesh = true;
		else
			m_bOpacityMesh = true;
	}

	//
	// ATTENTION TO OPP: 写一个转化工具,把现在的ex文件都转化为
	//                   2的版本，法向量反转。
	// 如果使用环增光模拟动态光的效果，则不需要再反转我们的normal.
	// 但游戏被下载前必须反转normal，为以后的升级做准备。
	// 如果当前ex版本号为1,则反转每一个顶点的normal值。
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

	//! River @ 2010-3-30:如果是桥类物品，则计算每一个face的normal.
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
	//! 如果我们自己对xMesh进行渲染，而非调用drawSubset,需要得到相关的资源
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
*  渲染当前的Mesh.
*
*  \param _pos  在这个位置上渲染当前的mesh.
*  ATTENTION:
*  River @ 2005-6-3: 对使用当前方向光做为环境光的物品,设置方向
*                    光做为物品的环境光.
*  
*/
bool osc_Mesh::render_mesh( os_meshRender* _mr,bool _alpha/* = false*/  )
{
	guard;

	int           t_i;
	osc_effect*   t_eff;

	osassert( _mr );

	// 设置物体是否受环境光影响，这个m_bUseDirLAsAmbient 意义已经变了。
	if(m_bUseDirLAsAmbient){
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, -1);
	}else{
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
			osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
	}

	// 如果受动态光影响
	if( _mr->m_sDLight.is_usedDl() )
		g_ptrDLightMgr->set_pointLight( _mr->m_sDLight );

	if( _alpha )
	{

		// 对Mesh的每一个subset进行处理。
		for( t_i=0;t_i<m_iShaNum;t_i++ )
		{
			// 如果当前渲染alpha subset,则略过没有alpha的subset.
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
		// 对Mesh的每一个subset进行处理。
		for( t_i=0;t_i<this->m_iShaNum;t_i++ )
		{
			// 如果当前不渲染alpha的subset,则略过有alpha的subset..
			if( m_vecAlphaSubset[t_i] )
				continue;

			t_eff = g_shaderMgr->get_effect( this->m_vecShaderId[t_i] );
			osassert( t_eff );
			g_shaderMgr->set_EffectInfo( m_vecShaderId[t_i] );

			if( !g_frender->prepare_andrender( this->m_ptrPMesh,t_i,t_eff ) )
				return false;
		}
	}	


	// 如果受动态光影响
	if( _mr->m_sDLight.is_usedDl() )
		g_ptrDLightMgr->set_pointLight( _mr->m_sDLight,false );

	return true;

	unguard;
}




//! 得以当前Mesh的三角形数目.
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

// 水面顶点的Declaration
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

	// River @ 2008-12-16: 释放水面的Declaration
	SAFE_RELEASE( g_pWaterVertexDeclaration );
	SAFE_RELEASE(g_pFogVertexDeclaration);

	for( int t_i = 0;t_i<MAX_MESHRSTYLE;t_i ++ )
		SAFE_DELETE_ARRAY( m_vecMeshInsPtr[t_i] );
	SAFE_DELETE_ARRAY( m_vecColorMeshPtr );


	m_vecSceneAMIns.destroy_mgr();
}


//! 释放当前的meshMgr.
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
	// 如果是finalRelease,则释放所有的mesh,并释放mesh对应的空间。
	if( _finalRelease )
		m_vecMesh.destroy_mgr();

	unguard;
}





//! 初始化当前MeshManager.
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
 *  从文件创建一个mesh.
 *
 *  ATTENTION TO FIX: 整理永远不会替换掉当前地图内已经调入的Obj
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
	// 如果内存中已经存在我们要创建的mesh,返回这个mesh的ID.
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

	// 得到当前要创建的mesh的id.
	t_idx = m_vecMesh.get_freeNode();
	if( !m_vecMesh[t_idx].create_meshFromFile( _filename,m_pd3dDevice ) )
		return -1;

	return t_idx;

	unguard;
}


//! 全部释放静态的mesh.
void osc_meshMgr::reset_staticMesh( void )
{
	guard;

	CSpaceMgr<osc_Mesh>::NODE  t_ptrNode;

	// River 2010-12-30:任意一个场景，超过MAX_SWAPMESHNUM个数目的
	//                  mesh,都会永远的驻留在内存中。
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

	osDebugOut( "释放<%d>个静态的mesh.常驻内存<%d>个Mesh\n",t_iSMeshNum,t_iInMemoyMeshNum );

	unguard;
}

//! 全部释放osaMesh.
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



//! 往meshMgr的aniMeshIns渲染队列中加入场景中要渲染的aniMeshIns.
void osc_meshMgr::add_sceAniMeshIns( int _idx )
{
	guard;

	int   t_idx = WORD(_idx) - MAXEFFECT_OSANUM;
	int   t_k;
	
	osassert( t_idx >= 0 );
	osassert( m_vecSceneAMIns[t_idx].is_amInsInuse() );

	m_iRAniMeshInsNum ++;

	//
	// 如果当前的mesh是点着色的Mesh.
	if( m_vecSceneAMIns[t_idx].is_colorVertexMesh() )
	{
		m_vecColorMeshPtr[m_iColorVerMeshNum] = _idx;
		m_iColorVerMeshNum ++;
		return;
	}

	// 如果在视野内，根据不同的渲染类型，分配ins索引到不同的数组中。
	t_k = m_vecSceneAMIns[t_idx].get_RenderStyle();

	osassert( m_vecMeshInsNum[t_k] < INIT_ANIMESHSIZE );
	m_vecMeshInsPtr[t_k][m_vecMeshInsNum[t_k]] = WORD(_idx);
	m_vecMeshInsNum[t_k] ++;

	unguard;
}



/** \brief
*  处理动画Ani的渲染队列，
*
*  相当于给不同的动画ani渲染方式进行排序,以减少渲染状态的改变次数。
*/
void osc_meshMgr::sort_aniMeshPtr( os_ViewFrustum* _vf )
{
	guard;

	osassert( _vf );

	float   t_fTime = sg_timer::Instance()->get_lastelatime();

	// River @ 2010-9-27:确保人物释放线程与此函数不会同时释放。
	// 否则可能会引起ref小于零的情形发生。
	CCriticalSec  t_osaCS( &osc_mapLoadThread::m_sOsaCS );

	int   t_idx = 0;
	CSpaceMgr<osc_aniMeshIns>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecAniMeshIns.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecAniMeshIns.next_validNode( &t_ptrNode ) )
	{
		// River @ 2006-7-17:如果amIns不在使用中，删除
		//osassert( t_ptrNode.p->is_amInsInuse() );
		if( !t_ptrNode.p->get_insInit() )
			continue;

		if( !t_ptrNode.p->is_amInsInuse() )
		{
			m_vecAniMeshIns.release_node( t_ptrNode.idx );
			continue;
		}

		// River @ 2009-12-31:
		// top render layer不需要判断可见性，全部可见
		if( !t_ptrNode.p->get_topLayer() )
		{
			// 查看当前的aniMeshIns是否在视野内.
			if( !_vf->objInVF( *t_ptrNode.p->get_bsphere() ) )
			{
				// River mod @ 2010-5-7:
				osassert( t_ptrNode.p->is_amInsInuse() );

				// River added @ 2007-6-25:如果不在视野内，则仅更新osa动画的内部数据
				// 如果不可见的情况下，动画更新到需要释放，释放这个osa动画
				t_ptrNode.p->update_osaAnimation( t_fTime );
				if( !t_ptrNode.p->is_amInsInuse() )
					m_vecAniMeshIns.release_node( t_ptrNode.idx );

				continue;
			}
		}

		m_iRAniMeshInsNum ++;

		// 如果当前的mesh是点着色的Mesh.
		if( t_ptrNode.p->is_colorVertexMesh() )
		{
			m_vecColorMeshPtr[m_iColorVerMeshNum] = t_ptrNode.idx;
			m_iColorVerMeshNum ++;
			continue;
		}

		// 如果在视野内，根据不同的渲染类型，分配ins索引到不同的数组中。
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
 *   渲染点着色相关的Mesh.
 *
 *   ATTENTION TO OPP:如果可能的话，加入VertexShader
 *   VertexShader根据当前的顶点光和新的环境光颜色，得到
 *  　最终渲染物品时的光线的颜色
 *  
 */
void osc_meshMgr::render_colorVertexMesh( bool _alpha )
{
	guard;

	if( m_iColorVerMeshNum <= 0 )
		return ;

	float   t_fEtime = sg_timer::Instance()->get_lastelatime();


	//@{
	//! River @ 2010-6-22:渲染屏幕破空效果,先清backSurface的alpha为纯白色
	for( int t_j = 0;t_j<m_iColorVerMeshNum;t_j ++ )
	{
		int t_idx = m_vecColorMeshPtr[t_j];

		if( t_idx == WORD(-1) )
			continue;

		if( t_idx < MAXEFFECT_OSANUM )
		{		
			osc_d3dManager*  t_ptrMgr1 = (osc_d3dManager*)::get_deviceManagerPtr();
			t_ptrMgr1->draw_fullscrAlpha( 0.0f );	
			//! 标记此变量，主渲染流程来渲染屏幕光波
			g_bRenderScreenWave = TRUE;
			break;
		}
	}
	//@}


	set_colorVertexRState( _alpha );

	// 依次渲染每一个instanceMesh数据。
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

			// River added @ 2010-6-23:屏幕波动效果的加入.
			// 后期优化：使用d3dCaps来得到写这些值是否成功
			m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 
				D3DCOLORWRITEENABLE_BLUE |
				D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED    );

		}
		else
		{
			//! River @ 2009-6-18:边缘运行中错误，此处必须cs一下
			//！River @ 2011-1-30:边缘是动态调入地图，而Q唐是静态的调入地图，所以
			//                    此处不需要再CS。
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





//! 渲染我们得到的aniMeshInstance的队列。
bool osc_meshMgr::render_aniMeshInsArr( bool _alpha,bool _alphaTestOpatity,
							os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;
	
	float   t_fEtime = sg_timer::Instance()->get_lastelatime();

	osassertex( t_fEtime >= 0.0f,va("The Neg Etime is:<%f>..\n",t_fEtime) );


	// SrcBlend和destBlend的值。
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
		// 根据不同的溶合方式，设置不同的渲染状态。
		if( _alpha )
		{
			m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,t_sDstBlend[t_i] );
			m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,t_sSrcBlend[t_i] );
		}

		// 不管是不是处于Alpha状态，都需要设置是否可以写ZBuffer，
		// 比如不能走上去的水面，是不透明的，但也不能写入Zbuffer.
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
		// 开始渲染aniMeshInstance,进入临界区，渲染aniMesh时，不能删除和创建Mesh
		::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );

		for( int t_j = 0;t_j<m_vecMeshInsNum[t_i];t_j ++ )
		{
			int t_idx = m_vecMeshInsPtr[t_i][t_j];

			if( t_idx == WORD(-1) )
				continue;

			if( t_idx < MAXEFFECT_OSANUM )
			{
				
				//  River @ 2007-5-10:
				//  因为多线程的关系，一些aniMeshIns可能已经释放，所以要先validate.
				if( !m_vecAniMeshIns.validate_id( t_idx ) )
					continue;

				// 如果隐藏状态
				if( m_vecAniMeshIns[t_idx].get_hide() )
					continue;

				// top zlayer的AniMesh必须最后渲染,commonRender下全部渲染
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

						//! River @ 2010-1-20:确保界面上人物特效正确渲染。
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
				//! River @ 2010-1-20:确保界面上人物特效正确渲染
				if( _alpha )
					m_vecAniMeshIns[t_idx].m_bRenderedInCurFrame = true;

			}
			else
			{
				
				// top Layer渲染不需要渲染场景内的osa特效
				if( _rtype > OS_RENDER_LAYER )
					continue;

				//  River @ 2007-5-10:
				//  因为多线程的关系，一些aniMeshIns可能已经释放，所以要先validate.
				if( !m_vecSceneAMIns.validate_id( t_idx-MAXEFFECT_OSANUM ) )
					continue;
				
				// River Mod @ 2008-12-10:水面不在此渲染 
				if( g_bWaterReflect &&
					(m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_waterAni()) )
					continue;

				///!雾不在此渲染
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


//! 创建aniMesh渲染时使用的渲染状态块.
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
		// 渲染AniMesh的时候，设置SampleState
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );

		m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );

		//@{
		//m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,GLOBAL_MINALPHAREF );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, GLOBAL_MINALPHAREF );
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);		
		//@}


		// 只所以使用none cull，是想让特效的叠加效果更明显，看起来更艳丽一些。
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_CW );
		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,TRUE );

		// 默认使用Clamp的纹理方式
		m_ptrMPipe->set_ssClamp();
	
		// 在渲染透明的aniMesh的时候，使Fog Enable
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
		// 渲染AniMesh的时候，设置SampleState
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
*  渲染动态的mesh.
*  
*  检测mesh是否在vf中，并根据alpha参数进行渲染。
*/
bool osc_meshMgr::render_aniMesh( os_ViewFrustum* _vf,
		bool _alpha,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;

	//
	// 先对我们要渲染的AniMesh进行序。
	if( (!_alpha) && (_rtype != OS_RENDER_TOPLAYER) )
		sort_aniMeshPtr( _vf );
	if( m_iRAniMeshInsNum <= 0 )
		return true;

	set_animeshRState( _alpha );

	//@{ 
	//  River added @ 2008-12-23:跟树类物品的透明一样，分两次渲染，这样才能
	//  正确的渲染出半透明的物品
	//  最后决定还是一次性渲染，节省计算量，但需要美术做资源的时候，多注意一些。
	if( !render_aniMeshInsArr( _alpha,true,_rtype ) )
		return false;
	//@} 

	//
	// 渲染点着的Mesh数据。
	// 非顶层渲染的时候渲染点着色物品
	if( _rtype != OS_RENDER_TOPLAYER )
		render_colorVertexMesh( _alpha );

	// 使下一帧可以重设所有物品的动态光。
	m_bFirstProcessDLight = true;

	return true;

	unguard;
}


/** \brief
*  渲染一个Mesh,使用meshID就可以了
* 
*  在指定的位置上渲染当前的Mesh.
*  以后可以在此处理模型的lod信息。
*  
*/
bool osc_meshMgr::render_mesh( os_meshRender* _mr,bool _alpha/* = false */ )			
{
	guard;

	osassert( _mr );

	// 设置渲染需要的全局性设置。

	mesh_renderSet( _mr );

	// 统计渲染面数.
	if( !_alpha )
		m_iRFaceNum += m_vecMesh[_mr->m_iMeshId].get_meshTriNum();

	// 渲染.
	if( !m_vecMesh[_mr->m_iMeshId].render_mesh( _mr,_alpha ) )
		return false;

	return true;

	unguard;
}



//! 从当前的aniMeshArray中得到或是创建一个osc_aniMesh.
osc_aniMesh* osc_meshMgr::get_aniMeshFromArr( os_aniMeshCreate* _mc,BOOL& _replace )
{
	guard;

	int        t_idx;
	DWORD      t_dwHash = string_hash( _mc->m_szAMName );

	osassert( _mc );

	_replace = FALSE;

	// 
	// 如果从aniMesh中可以找到当前要创建的Mesh,则不需要创建新的mesh.
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

	// 从我们得到的mesh指针中创建mesh数据。
	if( !m_vecAniMesh[t_idx].create_meshFromfile( *_mc ) )
		osassertex( false,va( "创建<%s>osaMesh出错..\n",_mc->m_szAMName ) );

	osassert( m_vecAniMesh[t_idx].get_rootNodePtr()->m_arrNodelist );

	return &m_vecAniMesh[t_idx];

	unguard;
}


/** \brief
*  从创建结构中创建我们的aniMesh.
*
*  ATTENTION TO FIX: 如果下层替换了一aniMeshIns，则上层再次使用当前的id时
*  会出现错误，此问题必须解决。
*  River: 通过合成ID的机制，此问题已经解决。
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
	// 如果替换某一个mesh产生的，则处理我们meshInstance Array.
	osassert( (!_replace) );
	osassert( t_meshPtr->get_rootNodePtr()->m_arrNodelist );

	//
	// 如果是从场景中创建的动画物品,则使用新的动画物品队列。
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
			osassertex( false,"场景不能创建更多的osa.....\n" );
			return -1;
		}
		// River @ 2011-2-26:移到这里，为了减少线程同步的等待时间。
		::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );

		t_wCreateId = m_vecAniMeshIns[t_iIdx].create_aniMeshIns( t_meshPtr,*_mc,_topLayer );
	}

	//! 如果是水面的mesh,则得到水面的effect.
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

	// River @ 2011-4-11:加入判断，确保此处的ID不会出错。
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

//! 删除一个动画.
void osc_meshMgr::delete_animationMesh( int _id,bool _sceC/* = false*/ )
{
	guard;

	int  t_id = WORD(_id);

	::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );
	if( _sceC )
	{
		t_id -= MAXEFFECT_OSANUM;

		// 
		// ATTENTION TO FIX:整理此处的关系，可能重新调入地图会影响此处的健壮性。
		if( t_id >= MAXMESHINS_PERSCE )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );
			return;
		}

		osassert( (t_id < MAXMESHINS_PERSCE) && m_vecSceneAMIns[t_id].is_amInsInuse() );
	}
	else
	{
		// River @ 2011-10-18:去掉此assert.
		//osassertex( (t_id < MAXEFFECT_OSANUM),"osaObject ID 过大..\n"  );
		if( t_id >= MAXEFFECT_OSANUM )
			return;

		osassertex( (t_id >=0 ),"osaObject ID 不能为负..\n" );
		//osassertex( m_vecAniMeshIns[t_id].is_amInsInuse(),"osaObject is not Inuse!!!" );

		//! River @ 2011-3-24: 此处在极端的情况下，还是会出此结点被删的情形，过去就OK.
		//                     加入判断，不出错再说.
		if( m_vecAniMeshIns.validate_id( t_id ) )
		{
			// ATTENTION TO FIX:为什么会被删除???
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
*  处理aniMesh的动态光问题。需要对场景中的每一个动态光调用这个函数。
*
*  动态光全部由全局光照来模拟。
*  
*  ATTENTION TO OPP: 得到在视野内的物品列表，然后处理列表内的物品。
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
	// 如果在当前帧中是第一次处理动态光，则对所有的aniMeshIns设置
	// 全局光，在以后的处理中，动态光只能在当前物品光度的基础上相加，
	// 不能重设。
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

			// River mod @ 2010-9-28:不出错.
			if( !t_ptrNode.p->is_amInsInuse() )
				continue;

			t_ptrNode.p->set_light( _dlight.m_sAmbientLight );
		}
	}

	//
	// 根据每一个物品的矩离，对动态光进行处理。
	for( t_ptrNode = m_vecAniMeshIns.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecAniMeshIns.next_validNode( &t_ptrNode ) )
	{
		// 
		if( !t_ptrNode.p->get_insInit() )
			continue;

		// River mod @ 2010-9-28:不出错.
		if( !t_ptrNode.p->is_amInsInuse() )
			continue;

		t_ptrMins = &m_vecAniMeshIns[t_ptrNode.idx];
		t_ptrBs = t_ptrMins->get_bsphere();
		t_dis = osVec3Length( 
			&(t_ptrBs->veccen - _dlight.m_vec3DLightPos) );

		if( t_dis >= _dlight.m_fAttenuationDis )
			continue;

		//
		//  线性计算动态光。
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
*  在渲染一个Mesh之前对传入的数据进行设置。
* 
*  当前主要设置物体的世界转化矩阵。
*  
*  River mod @ 2009-4-16:加入每物品的缩放处理
*  
*  \param _mr  需要设置的数据。
*/
void osc_meshMgr::mesh_renderSet( os_meshRender* _mr,osMatrix* pOut )
{	
	guard;

	osMatrix  t_mat,t_matScale;
	osassert( _mr );

	osMatrixScaling( &t_matScale,_mr->m_vec3Scale.x,
		_mr->m_vec3Scale.y,_mr->m_vec3Scale.z );

	///@{ 为了节省模型，有些模型只是镜像，所以就把x方向缩放写成-1就可以实现，但要把CullMode改一下.
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

	// 设置物品的位置
	t_mat._41 = _mr->m_vec3Pos.x;
	t_mat._42 = _mr->m_vec3Pos.y;
	t_mat._43 = _mr->m_vec3Pos.z;
	if (pOut)
	{
		memcpy(pOut,t_mat,sizeof(osMatrix));
	}

	 

	g_frender->get_d3dDevice()->SetTransform( D3DTS_WORLD,&t_mat );

	//@{设置世界矩阵

	osc_Mesh * t_mesh = &m_vecMesh[_mr->m_iMeshId];

	for( int t_i=0;t_i<t_mesh->m_iShaNum;t_i++ )
	{	// 对Mesh的每一个subset进行处理。
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


// 是否自己组织每一个Mesh的渲染，减少顶点和索引缓冲区，纹理等的切换时间
# if CUSTOM_MESHDRAW 

/** \brief
 *  批量渲染.ex文件，使场景内可以容纳更多的物品
 *
 *  算法描述：
 *  一: 对于非透明的物品部分渲染:
 *      1：先根据物品的id进行排序。上层传入的数据已经排过序。
 *      2: 渲染物品时，手动设置顶点和索引缓冲，以及shader，如果当前次渲染跟上一次相同，
 *         则不需要设置，只需要调用dip就可以了。
 *      3: 如果跟上次不同，则需要设置shader.
 *      4: 对每一个meshRender结构都需要设置世界转化矩阵。
 *   
 *  二：对于透明部分的物品渲染：
 *      1: 根据物品的位置进行排序,传入的数据已经排过序。
 *      2: 渲染时，如果上一个渲染的物品跟当前物品id相同，则省去顶点和索引缓冲区设置。
 *      3: 如果上次的shaderId和当前的shaderID相同，省去设置。
 *
 *  FVF需要开始渲染时，一次性设置.函数假设所有的.ex文件使用相同的fvf值。
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


	//! river added @ 2009-2-19:重设vbuffer Catch,
	//  在某些情况下，不重设会出现错误数据。 
	g_frender->reset_vbufCatch();

	D3DXATTRIBUTERANGE*   t_ptrAtt;
	// 遍历mesh
	for( int i = 0; i < _mshCount; ++i )
	{
		t_mr = &((*_vec)[i]);
		t_ptrMesh = &m_vecMesh[t_mr->m_iMeshId];


		// 设置每一个renderSet相关的数据
		mesh_renderSet( t_mr );

		// River mod @ 2009-1-10:对这种特殊物品，使用dx mesh的渲染方式,不需要设置顶点和索引
		// 缓冲区
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


		// 统计渲染面数.
		if( !_alpha )
			m_iRFaceNum += t_ptrMesh->get_meshTriNum();

		// 如果受动态光影响
		if( t_mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( t_mr->m_sDLight );

		// 遍历subset
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

			// 遍历pass
			for( int j = 0; j < (int)uPasses; ++j )
			{
				if( eff->pass( j ) )continue;


				// River mod @ 2009-1-10:对这种特殊的物品，使用dx mesh的渲染方式
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

		// River @ 2006-3-27:如果当前的Mesh使用方向光做全局光，则重设全局光
		// River @ 2006-6-21:如果物品使用地表颜色做为点着色，也需要重设全局光
// 		if( (t_mr->m_dwTerrColor != 0xffffffff) ||
// 			(t_ptrMesh->m_bUseDirLAsAmbient && _alpha) )
// 		{
// 				m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
// 					osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );			
// 		}

		// 如果受动态光影响
		if( t_mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( t_mr->m_sDLight,false );	

	}

	if( eff )eff->end();
	
	return true;

	unguard;
}


# else

//! 渲染一堆Mesh
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

	// 使用一个状态变量，看是否已经是不受环境光影响了。
	// 减少，SetRenderState的调用
	BOOL t_bConstAmbient = FALSE;

	// 遍历mesh
	for( int i = 0; i < _mshCount; ++i )
	{
		mr = &((*_vec)[i]);
		t_ptrMesh = &m_vecMesh[mr->m_iMeshId];
		mesh_renderSet( mr );

		/*
		//! river added @ 2007-2-8:树类的物品，渲染透明部分时，不需要写入zbuffer
		if( t_ptrMesh->is_treeObj() && _alpha )
			g_frender->get_d3dDevice()->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
		*/

		// 统计渲染面数.
		if( !_alpha )
			m_iRFaceNum += m_vecMesh[mr->m_iMeshId].get_meshTriNum();

		if(t_bConstAmbient != m_vecMesh[ mr->m_iMeshId ].m_bUseDirLAsAmbient){
			
			t_bConstAmbient = m_vecMesh[ mr->m_iMeshId ].m_bUseDirLAsAmbient;

			// 设置物体是否受环境光影响，这个m_bUseDirLAsAmbient 意义已经变了。
			if(t_bConstAmbient){
				m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, -1);
			}else{
				m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
					osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
			}

		}
		

		// 如果受动态光影响
		if( mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( mr->m_sDLight );


		// 遍历subset
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

			// 遍历pass
			for( int j = 0; j < (int)uPasses; ++j )
			{
				if( eff->pass( j ) )continue;

				if( !g_frender->render( m_vecMesh[ mr->m_iMeshId ].m_ptrPMesh, t_i ) )
					return false;

				eff->endPass();
			}
		}

		


		// 如果受动态光影响
		if( mr->m_sDLight.is_usedDl() )
			g_ptrDLightMgr->set_pointLight( mr->m_sDLight,false );

		/*
		//! river added @ 2007-2-8:树类的物品，渲染透明部分时，回复写入zbuffer的状态
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

	// 遍历mesh
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