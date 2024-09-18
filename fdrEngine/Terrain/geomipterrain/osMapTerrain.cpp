//===================================================================
/** \file  
* Filename :   osMapTerrain.cpp
* Desc     :   对Geomipmap中maptile和Chunk的组织。使整个场景内的Chunk
*              在一起FrameMove和渲染
* 
* His      :   River create @ 2006-1-12
*/
//===================================================================
# include "stdafx.h"
# include "osMapTerrain.h"
# include "FCMapCHunk.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../mfpipe/include/osEffect.h"
#include "../../mfpipe/include/osShader.h"
#include "../../mfpipe/include/ostexture.h"
#include "../../backpipe/include/osCamera.h"
# include "newHeader.h"
#include "../../Terrain/include/fdFieldMgr.h"
#include "../../effect/include/VolumeFog.h"
#include "../../interface/stringTable.h"
#include "../../CardProfile/gfxCardProfile.h"
bool gUseTerrainPS = true;
bool gUseTerrainSpecularLight = false;

float gWaterTransparencyDepth = 10.0f;


//! geoMipmap地表用到的fx文件
# define GEOMIPMAP_EFFECT "effect\\geomipmap.fx"

# define GEOMIPMAP_LIGHTMAP_EFFECT "effect\\geomipmapLM.fx"
//! 使用pixel shader混合帖图 
# define GEOMIPMAP_PS_PASS_EFFECT "effect\\geomipmapPS"

# define GEOMIPMAP_PS_PASS_SPECULAR_EFFECT "effect\\geomipmapPS_SPECULAR"

///@{
# define GEOMIPMAP_DEPTH_PASS_EFFECT "effect\\geomipmapDepth.fx"
///@}


//! 使用时需要处理,加入 "%d.fx"
# define GEOMIPMAP_DLIGHT "effect\\geomipmapDlight"

# define TERRAIN_DIRLPOS			"lhtDir"
# define TERRAIN_DIRLCOLOR			"lightDiffuse"
# define TERRAIN_AMBILCOLOR			"MaterialAmbient"
# define TERRAIN_CAMERA_POSTION		"CameraPostion"

# define TERRAIN_DLPOS         "g_vec3DLPos"
# define TERRAIN_DLCOLOR       "g_vec4DLColor"
# define TERRAIN_DLRANGE       "g_fDLRange"
# define TERRAIN_FOGVEC        "FogVector"

# define TERRAIN_DLPOS1         "g_vec3DLPos[0]"
# define TERRAIN_DLCOLOR1       "g_vec4DLColor[0]"
# define TERRAIN_DLRANGE1       "g_fDLRange[0]"

# define TERRAIN_DLPOS2         "g_vec3DLPos[1]"
# define TERRAIN_DLCOLOR2       "g_vec4DLColor[1]"
# define TERRAIN_DLRANGE2       "g_fDLRange[1]"

# define TERRAIN_DLPOS3         "g_vec3DLPos[2]"
# define TERRAIN_DLCOLOR3       "g_vec4DLColor[2]"
# define TERRAIN_DLRANGE3       "g_fDLRange[2]"

# define TERRAIN_DLPOS4         "g_vec3DLPos[3]"
# define TERRAIN_DLCOLOR4       "g_vec4DLColor[3]"
# define TERRAIN_DLRANGE4       "g_fDLRange[3]"


//! 底层渲染时用到的数据结构
os_meshSet osc_mapTerrain::m_sRenderMesh;
os_meshSet osc_mapTerrain::m_sLightMapRenderMesh;

//! 底层渲染有动态光的Chunk时，用到的meshSet
os_meshSet osc_mapTerrain::m_sRenderDLight[MAX_LIGHTNUM];

//! windy add 底层渲染地形用到Pixel Shader
os_meshSet osc_mapTerrain::m_sRenderMeshPS_PassList[4];

///@{
os_meshSet osc_mapTerrain::m_sRenderMeshDepth_Pass;
IDirect3DSurface9 * osc_mapTerrain::mpTargetZ = NULL;
IDirect3DTexture9 * osc_mapTerrain::m_pSceneDepthTex = 0;
LPDIRECT3DSURFACE9	osc_mapTerrain::m_pSceneDepthSurface = 0;
int  osc_mapTerrain::m_SceneDepthId = 0;
LPDIRECT3DSURFACE9			osc_mapTerrain::mpOldRenderTarget = 0;
LPDIRECT3DSURFACE9			osc_mapTerrain::mpOldStencilZ = 0;
///@}

osc_mapTerrain::osc_mapTerrain()
{
	MapTile::m_ptrMapTerrain = this;
	m_iActChunkNum = 0;
}
osc_mapTerrain::~osc_mapTerrain(void)
{
	m_vecChunkPtr.clear();
}

//! 加入一个MapChunkPtr
void osc_mapTerrain::push_mapTilePtr( MapTile* _ptr )
{
	guard;

	osassert( _ptr );
	for( DWORD t_dw=0;t_dw<m_vecMapTilePtr.size();t_dw ++ )
	{
		if( _ptr == m_vecMapTilePtr[t_dw] ) 
			return;
	}

	m_vecMapTilePtr.push_back( _ptr );

	// 分配激活ChunkPtr的指针列表
	m_vecActiveChunkPtr.resize( (int)m_vecMapTilePtr.size()*CHUNKNUM_PERTG );


	unguard;
}
void  osc_mapTerrain::onLostMapTerrain( void )
{
	guard;


	osc_texture*  t_tex = g_shaderMgr->getTexMgr()->get_textureById( m_SceneDepthId );
	if( t_tex )
		t_tex->release_texture();	

	
	SAFE_RELEASE(mpTargetZ);
	SAFE_RELEASE(mpOldRenderTarget);
	SAFE_RELEASE(mpOldStencilZ);
	unguard;
}
//! 设备lost后,需要重新填充每一个chunk的动态顶点缓冲区
void osc_mapTerrain::onRestoreMapTerrain( void )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecChunkPtr.size();t_i ++ )
		m_vecChunkPtr[t_i]->set_forceCreate( TRUE );

	//m_SceneDepthId = g_shaderMgr->getTexMgr()->addnew_texture( "SceneDepth.texture",
	//	400,300,D3DFMT_A8R8G8B8,10);

	osc_texture*  t_tex = g_shaderMgr->getTexMgr()->get_textureById( m_SceneDepthId );
	
	//! River @ 2010-10-8:加入更多的判断，使用重设设备不会出错。
	if( t_tex )
	{
		t_tex->create_texture( "SceneDepth.texture",
			400,300,D3DFMT_A8R8G8B8,g_frender->get_d3dDevice(),10);
		m_pSceneDepthTex = (IDirect3DTexture9*)g_shaderMgr->getTexMgr()->get_texfromid(m_SceneDepthId);
	}

	g_frender->get_d3dDevice()->CreateDepthStencilSurface( 400,300,
		D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &mpTargetZ, NULL );

	g_frender->get_d3dDevice()->GetRenderTarget( 0, &mpOldRenderTarget );
	g_frender->get_d3dDevice()->GetDepthStencilSurface( &mpOldStencilZ );

	unguard;
}


//! 推入激活的Chunk队列。此函数在MapTile的PushActiveChunk调用
void osc_mapTerrain::push_activeChunkPtr( MapChunk* _chunk )
{
	guard;

	m_vecActiveChunkPtr[m_iActChunkNum] = _chunk;
	m_iActChunkNum ++;
	osassert( m_iActChunkNum <= (int)m_vecActiveChunkPtr.size() );

	unguard;
}



//! 得到一个Chunk,mapTile内部使用
MapChunk* osc_mapTerrain::create_chunk( MapTile* _mt,int _x,int _z )
{
	guard;

	MapChunk*   t_ptrChunk ;
	switch(MapTile::s_RenderMode) {
	case MapTile::EMT_NOLOD:
		t_ptrChunk = new FCMapChunk(_mt,_x,_z);
		break;
	case MapTile::EMT_BRUTEFORCE: 
		osassert( false );
		break;
	case MapTile::EMT_MORPH_SW:
		osassert( false );
		break;
	default:
		osassert(false);
	}

	osassert( _mt );
	osassert( _x<MapTile::CHUNKS );
	osassert( _z<MapTile::CHUNKS );

	m_vecChunkPtr.push_back( t_ptrChunk );
	push_mapTilePtr( _mt );

	return t_ptrChunk;
	
	unguard;
}


//! 得到当前MapTile的相对偏移的MapTile
MapTile* osc_mapTerrain::get_offsetMapTile( MapTile* _ptr,int _xoff,int _zoff )
{
	guard;

	osassert( abs( _xoff ) <= 1 );
	osassert( abs( _zoff ) <= 1 );
	osassert( _ptr );

	for( DWORD t_dw=0;t_dw<m_vecMapTilePtr.size();t_dw ++ )
	{
		if( ((m_vecMapTilePtr[t_dw]->m_xpos - _xoff) == _ptr->m_xpos)&&
			((m_vecMapTilePtr[t_dw]->m_zpos - _zoff) == _ptr->m_zpos) )
			return m_vecMapTilePtr[t_dw];
	}

	return NULL;

	unguard;
}


//! 根据地图的索引，得到此mapTile的地表高度数据。
const float* osc_mapTerrain::get_mapTileHeightInfo( int _x,int _z )
{
	guard;

	for( DWORD t_dw=0;t_dw<m_vecMapTilePtr.size();t_dw ++ )
	{
		if( ((m_vecMapTilePtr[t_dw]->m_xpos) == _x)&&
			((m_vecMapTilePtr[t_dw]->m_zpos) == _z) )
			return m_vecMapTilePtr[t_dw]->get_heightInfoPtr();
	}

	return NULL;

	unguard;
}

//! 根据世界坐标，得到此chunk的光照图颜色
DWORD osc_mapTerrain::get_mapTileLightInfo(const osVec3D& _pos)
{
	guard;

	const os_tileIdx t_tgIdx(WORD((int)(_pos.x / TILE_WIDTH) / MAX_GROUNDWIDTH),
							WORD((int)(_pos.z / TILE_WIDTH) / MAX_GROUNDWIDTH));

	// chuck 截取值
	const float t_chuckSize = (TILE_WIDTH * MAX_GROUNDWIDTH / MapTile::CHUNKS);

	// 计算出mapTile的截取坐标
	const float t_mapTile_x = (_pos.x - t_tgIdx.m_iX * TILE_WIDTH * MAX_GROUNDWIDTH);
	const float t_mapTile_y = (_pos.z - t_tgIdx.m_iY * TILE_WIDTH * MAX_GROUNDWIDTH);

	// 计算出chuck 的索引
	const int t_chuck_x = (int)(t_mapTile_x / t_chuckSize);
	const int t_chuck_y = (int)(t_mapTile_y / t_chuckSize);

	const BYTE* t_lightMap;

	DWORD t_color = 0xff000000;
	
	if( t_chuck_x < MapTile::CHUNKS && t_chuck_y < MapTile::CHUNKS){
		
		for( DWORD t_dw=0;t_dw<m_vecMapTilePtr.size();t_dw ++ )
		{
			if( ((m_vecMapTilePtr[t_dw]->m_xpos) == t_tgIdx.m_iX)&&
			((m_vecMapTilePtr[t_dw]->m_zpos) == t_tgIdx.m_iY) ){
				
				if(m_vecMapTilePtr[t_dw]->m_chunks[t_chuck_x][t_chuck_y]->m_IsVisible){

					t_lightMap = (m_vecMapTilePtr[t_dw]->m_chunks[t_chuck_x][t_chuck_y]->get_lightMapL8());

					if(t_lightMap){

						const int t_lightMap_x = (int)((t_mapTile_x - (t_chuck_x * t_chuckSize)) / t_chuckSize * MapTile::ms_LightMapSize);
						const int t_lightMap_y = (int)((t_mapTile_y - (t_chuck_y * t_chuckSize)) / t_chuckSize * MapTile::ms_LightMapSize);

						osassert(t_lightMap_x >= 0 && t_lightMap_y >= 0);
						osassert(t_lightMap_x < MapTile::ms_LightMapSize && t_lightMap_y < MapTile::ms_LightMapSize);

						BYTE t_colorByte = t_lightMap[t_lightMap_x + t_lightMap_y * MapTile::ms_LightMapSize];
						t_color = D3DCOLOR_RGBA(t_colorByte,t_colorByte,t_colorByte,0xff);

					}

				}
				break;
			}
		}
	}
	

	return t_color;

	unguard;
}


//! 设置当前mapTile内每一个Chunk之间的关联关系,另加相连接的MapTile.
void osc_mapTerrain::SetNeighbors( MapTile* _ptr )
{
	return;
}

void osc_mapTerrain::clearVisible()
{
	for (size_t i = 0; i < m_vecMapTilePtr.size();i++){
		m_vecMapTilePtr[i]->ClearChunks();
	}

	// Clear...
	m_iActChunkNum = 0;
}



//=================================================================================
void osc_mapTerrain::SetupTextureStages(size_t _nPass)
{
	LPDIRECT3DDEVICE9 pd3d = g_frender->get_d3dDevice();

	// 
	// River Mod: 第一层地表不需要AlphaBlend.
	if( 0 == _nPass )
		pd3d->SetRenderState( D3DRS_ALPHABLENDENABLE,false );
	else
		pd3d->SetRenderState( D3DRS_ALPHABLENDENABLE,true );

	//
	//!wireframe
	if (g_bTileWireFrame){
		pd3d->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
	}
	else{
		pd3d->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	}
	// 都是CCW的渲染格式?
	if (MapTile::s_RenderMode==MapTile::EMT_NOLOD)
	{
		pd3d->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
	else{
		pd3d->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}
}

//=================================================================================
void osc_mapTerrain::UnSetupTextureStages()
{
	g_frender->get_d3dDevice()->SetTextureStageState(0,
		    D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	g_frender->get_d3dDevice()->SetTextureStageState(1,
		    D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	g_frender->get_d3dDevice()->SetTexture(0,NULL);
	g_frender->get_d3dDevice()->SetTexture(1,NULL);
	g_frender->get_d3dDevice()->SetPixelShader( NULL );
}



//! 得到实例的指针。
osc_mapTerrain* osc_mapTerrain::get_mapTerrainIns( void )
{
	return MapTile::m_ptrMapTerrain;
}



void osc_mapTerrain::oneTimeDeinit( void )
{
	guard;
	SAFE_RELEASE(mpTargetZ);
	SAFE_RELEASE(mpOldRenderTarget);
	SAFE_RELEASE(mpOldStencilZ);

	unguard;
}
//! 初始化MapTerrain整体的数据
void osc_mapTerrain::oneTimeInit( void )
{
	guard;

	char  t_sz[128];
	if (!gUseTerrainPS)
	{
		// 初始化有动态光和无动态光的两个meshSet
		m_sRenderMesh.init( GEOMIPMAP_EFFECT,	
			sizeof( MapChunk::ChunkVertex ),24,128 );

		m_sLightMapRenderMesh.init( GEOMIPMAP_LIGHTMAP_EFFECT,	
			sizeof( MapChunk::ChunkVertex ),24,128 );

		// 动态光的渲染使用多个MeshSet,如果场景内的动态光少，则加快渲染速度
		for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
		{
			sprintf( t_sz,"%s%d.fx",GEOMIPMAP_DLIGHT,t_i+1 );
			m_sRenderDLight[t_i].init( t_sz,
				sizeof( MapChunk::ChunkVertex ),24,128 );
		}
	}
	else
	{

		//! wxd add
		for (int i = 0;i < 4;i++)
		{
			if (gUseTerrainSpecularLight)
			{
				sprintf( t_sz,"%s_%d.fx",GEOMIPMAP_PS_PASS_SPECULAR_EFFECT,i+1 );
			}
			else
			{
				sprintf( t_sz,"%s_%d.fx",GEOMIPMAP_PS_PASS_EFFECT,i+1 );
			}
			
			m_sRenderMeshPS_PassList[i].init(t_sz,
				sizeof( MapChunk::ChunkVertex ),24,128 );
		}
		m_sRenderMeshDepth_Pass.init(GEOMIPMAP_DEPTH_PASS_EFFECT,sizeof( MapChunk::ChunkVertex ),24,128);

	}


	//@{ tzz added for galaxy effect
#if GALAXY_VERSION
	gala_initExplode();
	memset(sm_arrExplode,0,sizeof(sm_arrExplode));
#endif //GALAXY_VERSION
	//@}

	///@{Water Depth
	m_SceneDepthId = g_shaderMgr->getTexMgr()->addnew_texture( "SceneDepth.texture",
		400,300,D3DFMT_A8R8G8B8,10);

	m_pSceneDepthTex = (IDirect3DTexture9*)g_shaderMgr->getTexMgr()->get_texfromid(m_SceneDepthId);

	g_frender->get_d3dDevice()->CreateDepthStencilSurface( 400,300,
		D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &mpTargetZ, NULL );

	g_frender->get_d3dDevice()->GetRenderTarget( 0, &mpOldRenderTarget );
	g_frender->get_d3dDevice()->GetDepthStencilSurface( &mpOldStencilZ );

	
	///@}

	unguard;
}


//! 设置地表的全局光照信息
void osc_mapTerrain::set_terrainLightInfo( void )
{
	guard;

	if (gUseTerrainPS){
		for (int i = 0;i < 4;i++){
			set_meshSetLightInfo(m_sRenderMeshPS_PassList[i]);
		}
	}else{
		set_meshSetLightInfo(m_sRenderMesh);
		set_meshSetLightInfo(m_sLightMapRenderMesh);
	}

	unguard;
}


//! 设置全局的动态光信息
void osc_mapTerrain::set_terrainDLight( void )
{
	guard;

	if (gUseTerrainPS){
		for(size_t i = 0; i < MAX_LIGHTNUM;i++){
			set_meshSetLightInfo(m_sRenderMeshPS_PassList[i]);
		}
	}else{

		for(size_t i = 0; i < MAX_LIGHTNUM;i++){
			set_meshSetLightInfo(m_sRenderDLight[i]);
		}
	}
	
	unguard;
}


//! set the mesh set light information
void osc_mapTerrain::set_meshSetLightInfo(os_meshSet& _meshSet)
{
	guard;

	osVec3Normalize( &g_vec3LPos,&g_vec3LPos );

	osVec4D t_vec4Tmp(g_vec3LPos.x,g_vec3LPos.y,g_vec3LPos.z,1.0f);
	_meshSet.set_effectValue( TERRAIN_DIRLPOS,&t_vec4Tmp,sizeof( t_vec4Tmp ) );

	t_vec4Tmp.x = g_fDirectLR;
	t_vec4Tmp.y = g_fDirectLG;
	t_vec4Tmp.z = g_fDirectLB;
	t_vec4Tmp.w = 1.0f;
	
	_meshSet.set_effectValue( TERRAIN_DIRLCOLOR,&t_vec4Tmp,sizeof( t_vec4Tmp ));

	t_vec4Tmp.x = g_fAmbiR;
	t_vec4Tmp.y = g_fAmbiG;
	t_vec4Tmp.z = g_fAmbiB;
	t_vec4Tmp.w = 1.0f;

	_meshSet.set_effectValue( TERRAIN_AMBILCOLOR,&t_vec4Tmp,sizeof( t_vec4Tmp ) );

	// 设置地形有雾信息
	t_vec4Tmp.x = 1.0f/(g_fFogEnd-g_fFogStart);
	t_vec4Tmp.y = 0.0f;
	t_vec4Tmp.z = -g_fFogStart;
	t_vec4Tmp.w = 1.0f;
	_meshSet.set_effectValue( TERRAIN_FOGVEC,&t_vec4Tmp,sizeof( t_vec4Tmp ) );
	
	
	t_vec4Tmp.x = g_ptrCamera->get_curpos().x;
	t_vec4Tmp.y = g_ptrCamera->get_curpos().y;
	t_vec4Tmp.z = g_ptrCamera->get_curpos().z;
	t_vec4Tmp.w = 1.0f;
	
	_meshSet.set_effectValue( TERRAIN_CAMERA_POSTION,&t_vec4Tmp,sizeof( t_vec4Tmp ) );

	unguard;
}

void osc_mapTerrain::render_terrain_Depth()
{
	guard;

	m_pSceneDepthTex->GetSurfaceLevel(0,&m_pSceneDepthSurface);
	g_frender->get_d3dDevice()->SetRenderTarget( 0, m_pSceneDepthSurface );
	g_frender->get_d3dDevice()->SetDepthStencilSurface( mpTargetZ );
	
	g_frender->get_d3dDevice()->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0,0);

	int      i;
	const os_renderMeshData*   t_sr;

	// 画所有的激活地形块
	g_frender->get_d3dDevice()->SetVertexDeclaration(
		MapTile::m_pDeclaration );

	osc_TGManager* t_pScene = (osc_TGManager*)get_sceneMgr();
	
	//osVec4D wClip(0,(0),(float)t_pScene->m_sSceneTexData.m_fWaterHeight-gWaterTransparencyDepth,1.0f/gWaterTransparencyDepth);
	osVec4D wClip(0,(0),(float)t_pScene->m_sSceneTexData.get_waterHeight()-gWaterTransparencyDepth,1.0f/gWaterTransparencyDepth);
	

	m_sRenderMeshDepth_Pass.set_effectVector("wClip",&wClip);
	for( i=0;i<m_iActChunkNum;i ++ )
	{
		t_sr = m_vecActiveChunkPtr[i]->get_depthpassRdata();
		if( !t_sr  )
			continue;
		m_sRenderMeshDepth_Pass.push(t_sr);
	}

	if( !MapChunk::m_ptrMidPipe->render_andClearMeshSet( &m_sRenderMeshDepth_Pass) )
		osassert( false );


     //D3DXSaveSurfaceToFileA("TerrainDepth.TGA",D3DXIFF_TGA,m_pSceneDepthSurface,NULL,NULL);


	SAFE_RELEASE(m_pSceneDepthSurface);
	
	osc_TGManager*    t_ptrScene = (osc_TGManager*)::get_sceneMgr();
	if( !t_ptrScene->m_growEff.IsEffective() )
		g_frender->get_d3dDevice()->SetRenderTarget(0, mpOldRenderTarget);
	else
		t_ptrScene->m_growEff.reset_renderTarget( g_frender->get_d3dDevice() );

	HRESULT  t_hr = g_frender->get_d3dDevice()->SetDepthStencilSurface( mpOldStencilZ );
	osassert( !FAILED( t_hr ) );

	//static int t_i = 0;
	//::D3DXSaveTextureToFile( va( "d:/lightmap/tex%d.png",t_i ++ ),D3DXIFF_PNG,m_pSceneDepthTex,NULL );

	unguard;
}
//! 画地表地表
void osc_mapTerrain::render_terrain( BOOL _envMap )
{
	guard;

	int      i;
	const os_renderMeshData*   t_sr;
	// 画所有的激活地形块
	g_frender->get_d3dDevice()->SetVertexDeclaration(
		MapTile::m_pDeclaration );

	g_frender->get_d3dDevice()->SetFVF( 0 );

	//
	//!wireframe
	if (g_bTileWireFrame)
		g_frender->get_d3dDevice()->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
	else
		g_frender->get_d3dDevice()->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);


	if( _envMap )
		g_frender->get_d3dDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW );
	else
		g_frender->get_d3dDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW );


	if( gUseTerrainPS )
	{	//! 为真表示用PIXEL SHADER优化
		//! 
		if (gUseTerrainSpecularLight)
		{	//!如果采用地表高光，则要设置相机位置

			D3DXVECTOR4 pos(g_vec3ViewPos.x,g_vec3ViewPos.y,g_vec3ViewPos.z,1);
			for (int i = 0;i < 4;i++)
			{
				
				m_sRenderMeshPS_PassList[i].set_effectValue(TERRAIN_CAMERA_POSTION,&pos,sizeof(D3DXVECTOR4));
			}
		}
		
		//! 先没有动态光的地表
		for( i=0;i<m_iActChunkNum;i ++ )
		{
			if( !m_vecActiveChunkPtr[i]->m_bAffectByDLight )
			{
				t_sr = m_vecActiveChunkPtr[i]->get_pspassRdata();
				if( !t_sr  )
					continue;
				int PassNr = m_vecActiveChunkPtr[i]->getpassCnt();

				osassert(PassNr>=1&&PassNr<=4);
				m_sRenderMeshPS_PassList[PassNr-1].push(t_sr);
				
			}
		}
		set_terrainLightInfo();
		for (int i = 0;i < 4;i++)
		{
			m_sRenderMeshPS_PassList[i].m_effectTechniqueLevel = 0;
			if( !MapChunk::m_ptrMidPipe->render_andClearMeshSet( &m_sRenderMeshPS_PassList[i] ) )
				osassert( false );
		}


		//@{
		// River @ 2009-9-18: 加入hardTile的渲染，使用1层贴图的ps.
		for( int t_i=0;t_i<m_iActChunkNum;t_i ++ )
		{
			if( m_vecActiveChunkPtr[t_i]->m_iHardTileTypeNum <= 0 )
				continue;
			for( int t_k = 0;t_k<m_vecActiveChunkPtr[t_i]->m_iHardTileTypeNum;t_k ++ )
			{
				m_sRenderMeshPS_PassList[0].push( 
					m_vecActiveChunkPtr[t_i]->get_hardTileRdata( t_k ) );
			}
		}
		m_sRenderMeshPS_PassList[0].m_effectTechniqueLevel = 0;
		if( !MapChunk::m_ptrMidPipe->render_andClearMeshSet( &m_sRenderMeshPS_PassList[0] ) )
			osassert( false );
		//@}

		//! 渲染有动态光的地表
		for( int i=0;i<m_iActChunkNum;i ++ )
		{
			if( m_vecActiveChunkPtr[i]->m_bAffectByDLight )
			{
				t_sr = m_vecActiveChunkPtr[i]->get_pspassRdata();
				if( !t_sr  )
					continue;
				int PassNr = m_vecActiveChunkPtr[i]->getpassCnt();

				// 根据是否受动态光的影响，推入到不同的meshSet内渲染
				osassert(PassNr>=1&&PassNr<=4);
				m_sRenderMeshPS_PassList[PassNr-1].push(t_sr);
			}
		}

		set_terrainLightInfo();
		set_terrainDLight();

		for (int i = 0;i < 4;i++)
		{
			if( !MapChunk::m_ptrMidPipe->render_andClearMeshSet( &m_sRenderMeshPS_PassList[i] ) ){
				osassert( false );
			}
		}

	}
	else
	{

		// 中间管道排序渲染推入的Mesh.
		set_terrainLightInfo();

		for (size_t pass = 0 ; pass < 4; pass++)
		{
			SetupTextureStages(pass);

			// 根据不同的情况：
			if( (MapTile::s_RenderMode == MapTile::EMT_NOLOD) ||
				(MapTile::s_RenderMode == MapTile::EMT_BRUTEFORCE ) )/**/
			{
				for( i=0;i<m_iActChunkNum;i ++ )
				{
					t_sr = m_vecActiveChunkPtr[i]->get_passRdata( int(pass) );
					if( !t_sr  ) continue;

					m_sRenderMesh.push( t_sr );		

				}

				

				if( !MapChunk::m_ptrMidPipe->render_andClearMeshSet( &m_sRenderMesh ) )
					osassert( false );					
			}
		}
		//! ver 16 add 物品阴影信息
		for( int i=0;i<m_iActChunkNum;i ++ )
		{
			t_sr = m_vecActiveChunkPtr[i]->get_lightmappassRdata();
			m_sLightMapRenderMesh.push(t_sr);
		}

		if( !MapChunk::m_ptrMidPipe->render_andClearMeshSet( &m_sLightMapRenderMesh ) )
			osassert( false );				
	}
	UnSetupTextureStages();

	unguard;
}


//! 每一帧对整个Chunk列表进行FrameMove和画
BOOL osc_mapTerrain::frame_processTerrain( BOOL _envMap/* = FALSE*/,BOOL _depth/* = FALSE*/ )
{
	guard;

	if( !_envMap && !_depth )
	{
		for (size_t i = 0; i < m_vecMapTilePtr.size();i++){
			m_vecMapTilePtr[i]->PushActiveChunk();
		}
	}

	extern osc_camera*         g_ptrCamera;

	// 
	// River @ 2009-7-16:如果不加此判断，反射就不正常
	if( !_envMap )
		g_ptrCamera->get_viewmat(g_matView);
	osMatrixMultiply( &m_ViewProjmat,&g_matView,&g_matProj );
	
	if (gUseTerrainPS)
	{
		for ( int t_i=0; t_i<this->m_iActChunkNum;t_i++)
		{
			m_vecActiveChunkPtr[t_i]->m_PSPassMeshData.m_pTexMatrix[0] = &m_ViewProjmat;
			//@{
			m_vecActiveChunkPtr[t_i]->m_DepthPassMeshData.m_pTexMatrix[0] = &m_ViewProjmat;
			//@}
		}
	}
	else
	{
		for ( int t_i=0; t_i<this->m_iActChunkNum;t_i++)
		{
			for (size_t i = 0;i < m_vecActiveChunkPtr[t_i]->m_vecPassMeshData.size();i++ )
			{
				m_vecActiveChunkPtr[t_i]->m_vecPassMeshData[i].m_pTexMatrix[1] = &m_ViewProjmat;
			}
			//! ver 16 add 物品阴影信息
			m_vecActiveChunkPtr[t_i]->m_LightMapMestData.m_pTexMatrix[0] = &m_ViewProjmat;

		}
	}
		
	//!第一次更新,
	for ( int t_i=0; t_i<this->m_iActChunkNum;t_i++){
		m_vecActiveChunkPtr[t_i]->UpdateTessellation();
	}
	for (int t_i=0; t_i<this->m_iActChunkNum; t_i++){
		m_vecActiveChunkPtr[t_i]->UpdateTessellation2();
	}
	for ( int t_i=0; t_i<this->m_iActChunkNum; t_i++){
		m_vecActiveChunkPtr[t_i]->UpdateTessellation3();
	}

	//@{tzz added exploding effect for galaxy game
#if GALAXY_VERSION
	gala_frameMove();
#endif //GALAXY_VERSION
	//@}

	
	if (_depth)
	{
		render_terrain_Depth();
	}
	else
	{
		// 画地形
		render_terrain( _envMap );
	}
	

	
	

	return TRUE;
	unguard;
}


//====================================================================================================================
//
//	tzz added for galaxy effect @ 2008-6-5
//
//====================================================================================================================
#if GALAXY_VERSION
//!static member initialize
gala_spaceHole osc_mapTerrain::sm_spaceHole	;
gala_explode osc_mapTerrain::sm_arrExplode[scm_iMaxExplode] ;
int	osc_mapTerrain::sm_iCurrExplode							= 0;
//int osc_mapTerrain::sm_iCurrBullet							= 0;

//--------------------------------------------------------------------------------------------------------------------
//Name: gala_initExplode
//Desc: initialize the effect shader file(.fx) for galaxy
//--------------------------------------------------------------------------------------------------------------------
void osc_mapTerrain::gala_initExplode(gala_explode* _ptrExplode /*= NULL*/)
{
	//guard;

	if(_ptrExplode){
		//reset the time
		_ptrExplode->fTime = 0.0f;

		//set the value 
		memcpy(sm_arrExplode + sm_iCurrExplode,_ptrExplode,sizeof(gala_explode));

		//set the ptr position
		sm_iCurrExplode++;
		
		if(sm_iCurrExplode == scm_iMaxExplode){
			sm_iCurrExplode = 0;
		}
	}

	//unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: gala_initSpaceHole
//Desc: initialize the effect shader file(.fx) for galaxy
//--------------------------------------------------------------------------------------------------------------------
void osc_mapTerrain::gala_initSpaceHole(gala_spaceHole* _ptrHole /*= NULL*/)
{
	//guard;

	if(_ptrHole){
		//reset the time
		//_ptrBullet->fTime = 0.0f;
		
		//it is unnecessary to be normalized ,because the 
		//direction vector is normalized 
		//normalize the direction
		//const D3DXVECTOR3 t_vec3Tmp = _ptrBullet->vecDir;
		//D3DXVec3Normalize(&(_ptrBullet->vecDir),&t_vec3Tmp);

		//set the value 
		

		//sm_iCurrBullet++;
		//if(sm_iCurrBullet == scm_iMaxBullet){
		//	sm_iCurrBullet = 0;
		//}
		ZeroMemory(_ptrHole,sizeof(gala_spaceHole));
	}

	//unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: gala_delBullet
//Desc: delete the effect shader file(.fx) for galaxy
//--------------------------------------------------------------------------------------------------------------------
void osc_mapTerrain::gala_delSpaceHole(int _index)
{
	guard;

	//osassert(_index < scm_iMaxBullet);
	//for(int i = 0;i < scm_iMaxBullet; i++){
	//	sm_arrBullet[i].fRadiusOffset = 0;
	//}
	//ZeroMemory(&sm_spaceHole,sizeof(sm_spaceHole));
	m_sRenderMesh.set_effectValue("g_spaceHole",&sm_spaceHole,sizeof(sm_spaceHole));

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: gala_frameMove
//Desc: call it every frame
//--------------------------------------------------------------------------------------------------------------------
void osc_mapTerrain::gala_frameMove(gala_spaceHole* _hole /*=NULL*/)
{
	//guard;
	
	if(_hole){
		//memcpy(sm_arrBullet + sm_iCurrBullet,_bullet,sizeof(gala_spaceHole));
		//m_sRenderMesh.set_effectValue("g_arrBullet",sm_arrBullet,sizeof(sm_arrBullet));
		m_sRenderMesh.set_effectValue("g_spaceHole",_hole,sizeof(gala_spaceHole));
	}else{
		
		//refresh the time;
		for(int i = 0;i < scm_iMaxExplode; i++){

			//if this explode position is validate
			if(sm_arrExplode[i].fSpeed != 0.0f){
				sm_arrExplode[i].fTime += g_ptrSmoothTimer->get_smoothTime();
			}
		}

		//set the value for fx data
		m_sRenderMesh.set_effectValue("g_arrExplode",sm_arrExplode,sizeof(sm_arrExplode));
		
	}

	//unguard;
}

#endif  //GALAXY_VERSION


