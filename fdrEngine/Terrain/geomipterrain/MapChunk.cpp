//===================================================================
/** \file  
* Filename :   MapChunk.cpp
* Desc     :   
* His      :   Windy create @2005-12-6 11:14:06
*/
//===================================================================
# include "stdafx.h"
#include "mapchunk.h"
#include "MapTile.h"
#include "newHeader.h"
#include "MapHelper.h"
#include "../../interface/fileOperate.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../mfpipe/include/osShader.h"
# include "../../mfpipe/include/osTexture.h"

# include "../include/fdFieldStruct.h"
# include "../include/fdNewFieldStruct.h"


//! 中间渲染管道的指针,用于使用现有的渲染底层
osc_middlePipe* MapChunk::m_ptrMidPipe = NULL;

/**  
 * 地表chunk渲染时的细节控制变量,离相机近的chunk可以渲染多层，远一些chunk可以细节变代。
 * 此变量值为1.0的时候，细节最高，此值为0.0的时候，细节最低,默认为1.0,即细节全开。
 */
OSENGINE_API float                  g_fTerrDetailCoef = 1.0f;
extern bool gUseTerrainPS;

//! 用于中间管道创建动态的顶点缓冲区
DWORD MapChunk::ChunkVertex::m_dwFvf = D3DFVF_XYZ|D3DFVF_DIFFUSE| D3DFVF_TEX1;


//! River added @ 2007-1-25:加入细节纹理的控制
void MapChunk::OneTimeInit( osc_middlePipe* _ptrPipe )
{
	guard;

	osassert( _ptrPipe );

	m_ptrMidPipe = _ptrPipe;


	unguard;
}


MapChunk::MapChunk(MapTile * mt,int x,int y):m_px(x),m_py(y),m_IsAcitve(false),
								m_IsVisible(false),m_ForceBufferCreate(false),
								m_bIsAlphaInit(false),m_LightMap(NULL),m_LightMapL8(NULL)
{
	m_mt = mt;
	m_AlphaSize = ALPHAMAP_SIZE;
	m_Vertices = new ChunkVertex[MapTile::MAX_VERTICES];

	m_Indices = new unsigned short[MapTile::MAX_INDICES];
	m_IndexMap = new unsigned short[MapTile::MAX_VERTICES];
	for (int i=0; i < MAX_ALPHALAYS; i++){
		m_AlphaPool[i] = new BYTE [m_AlphaSize*m_AlphaSize];
	}
	

	// 中间管道相关的顶点和索引缓冲区ID
	m_iVerBufId = -1;
	m_iIdxBufId = -1;
	m_LightMapID = -1;
	m_CombineAlphaID = -1;

	//! River @ 2009-9-18:初始化数据非常重要
	m_iHardTileTypeNum = 0;
	for( int t_i = 0;t_i<MAX_HARDTILE_PERCHUNK;t_i ++ )
	{
		m_sHardTile[t_i].m_iIndexBufId = -1;
		m_sHardTile[t_i].m_wTexId[1] = -1;
	}

	//
	m_bAffectByDLight = false;

}
//=================================================================================
MapChunk::~MapChunk(void)
{
	SAFE_DELETE_ARRAY(m_Vertices);
	SAFE_DELETE_ARRAY(m_Indices);
	SAFE_DELETE_ARRAY(m_IndexMap);
	for (int i=0; i < MAX_ALPHALAYS; i++){
		SAFE_DELETE_ARRAY(m_AlphaPool[i]);
	}
	SAFE_DELETE_ARRAY(m_LightMap);
	SAFE_DELETE(m_LightMapL8);
	m_vecPassMeshData.clear();
}

const osMatrix* MapChunk::get_chunkTexMatrix( int _idx )
{
	int t_id = m_PassList[_idx].m_TexIdx;
	return &m_mt->m_Pass[t_id].mat;
}

const os_renderMeshData* MapChunk::get_passRdata( int _pass )
{
	guard;

	osassert( m_vecPassMeshData.size() >= 1 );

	if( _pass >= (int)m_vecPassMeshData.size() )
		return NULL;
	else
	{
		return &m_vecPassMeshData[_pass];
	}

	unguard;
}
const os_renderMeshData* MapChunk::get_pspassRdata( )
{
	return &m_PSPassMeshData;
}
const os_renderMeshData* MapChunk::get_depthpassRdata( )
{
	return &m_DepthPassMeshData;
}
const os_renderMeshData* MapChunk::get_lightmappassRdata( )
{
	return &m_LightMapMestData;
}


//=================================================================================
bool MapChunk::Draw(size_t _ipass)
{
	//! never use these code!!!
	osassert(false);
	
	return true;
}
//=================================================================================
void MapChunk::SetVisible(bool Vis)
{
	guard;
	if(Vis)
	{
		if(!m_IsAcitve)
		{
			Init();
			m_IsAcitve = true;
		}

		/*
		// River added @ 2007-1-25: 加入相细节参数的计算
		if( !m_IsVisible )
		{
			osVec3D   t_vec3CamPos;
			g_ptrCamera->get_curpos( &t_vec3CamPos );
			m_fCamDisCoef = osVec3Length( &(t_vec3CamPos - m_vec3CentralPt) );
			m_fCamDisCoef /= g_ptrCamera->get_farClipPlaneDis();
		}
		*/
	}

	m_IsVisible = Vis;
	unguard;
}
//=================================================================================
void MapChunk::DeInitUnVisible()
{
	if (!m_IsVisible&&m_IsAcitve){
		DeInit();
		m_IsAcitve = false;
	}
}
//=================================================================================
void MapChunk::Setup()
{
	assert(m_mt);
	

}
//=================================================================================
void MapChunk::Init()
{
	guard;
	m_NumIndices = m_NumVertices = 0;
	m_NumNewVertices = m_NumNewIndices = 0;
	m_ForceBufferCreate = true;
	if (!m_bIsAlphaInit){
		InitAlphaLMData();
		m_bIsAlphaInit = true;
	}

	unguard;
}
//=================================================================================
void MapChunk::DeInit()
{
}

/*
River @ 2009-9-18:
Map Chunk内的m_Vertices顶点索引数据结构如下图所示:

68  69...............

51  52...............67

34  34...............50

1   3  5  7..........33

0   2  4  6..........32

可参考MapChunk::GetIndex函数，在函数内下断点，可以查看m_Vertices
的数据计算和来源。
*/
//! 构建hardTile需要的index数据
int MapChunk::construct_hardTileIdxBuf( osc_hardTextureTile* _ht )
{
	guard;

	int  t_iIdxNum = _ht->m_iTileNum*6;

	int   t_iGBufIdx = -1;
	WORD* t_ptrWordBuf = (WORD*)START_USEGBUF( t_iGBufIdx );
	int   t_idx = 0;
	int   t_iIdx[4];
	for( int t_i=0;t_i<_ht->m_iTileNum;t_i ++ )
	{
		int t_x = _ht->m_vecTileInChunk[t_i].m_iX;
		int t_y = _ht->m_vecTileInChunk[t_i].m_iY;

		if( _ht->m_vecTileInChunk[t_i].m_iY == 0 )
		{
			t_iIdx[0] = _ht->m_vecTileInChunk[t_i].m_iX*2;
			t_iIdx[1] = t_iIdx[0]+1;
			t_iIdx[2] = t_iIdx[1]+1;
			t_iIdx[3] = t_iIdx[2]+1;
		}
		else if( _ht->m_vecTileInChunk[t_i].m_iY == 1 )
		{
			t_iIdx[0] = _ht->m_vecTileInChunk[t_i].m_iX*2+1;
			t_iIdx[2] = _ht->m_vecTileInChunk[t_i].m_iX*2+3;
			t_iIdx[1] = MapTile::CHUNK_PIXEL*2+_ht->m_vecTileInChunk[t_i].m_iX;
			t_iIdx[3] = MapTile::CHUNK_PIXEL*2+_ht->m_vecTileInChunk[t_i].m_iX + 1;
		}
		else
		{
			t_iIdx[0] = _ht->m_vecTileInChunk[t_i].m_iX + 
				_ht->m_vecTileInChunk[t_i].m_iY*MapTile::CHUNK_PIXEL;
			t_iIdx[2] = t_iIdx[0] + 1;
			t_iIdx[1] = _ht->m_vecTileInChunk[t_i].m_iX + 
				(_ht->m_vecTileInChunk[t_i].m_iY+1)*MapTile::CHUNK_PIXEL;
			t_iIdx[3] = t_iIdx[1] + 1;
		}

		t_ptrWordBuf[t_idx]   = t_iIdx[0];
		t_ptrWordBuf[t_idx+1] = t_iIdx[2];
		t_ptrWordBuf[t_idx+2] = t_iIdx[1];
		t_ptrWordBuf[t_idx+3] = t_iIdx[1];
		t_ptrWordBuf[t_idx+4] = t_iIdx[2];
		t_ptrWordBuf[t_idx+5] = t_iIdx[3];
		t_idx += 6;
	}

	//! 创建静态的索引缓冲区
	int   t_iIdxBufId = m_ptrMidPipe->create_staticIB( t_ptrWordBuf,t_iIdxNum );
	END_USEGBUF( t_iGBufIdx );

	return t_iIdxBufId;

	unguard;
}

//! river @ 2009-9-17:加入硬接触的地表.
const os_renderMeshData* MapChunk::get_hardTileRdata( int _idx)
{
	guard;

	osassert( _idx < m_iHardTileTypeNum );

	static osMatrix  t_sMatAll;
	
	osMatrixMultiply( &t_sMatAll, &g_matView, &g_matProj );

	m_sHardTile[_idx].m_iVertexBufId = m_iVerBufId;
	m_sHardTile[_idx].m_iVertexNum = m_NumNewVertices;
	m_sHardTile[_idx].m_wTexId[0] = m_LightMapID;
	m_sHardTile[_idx].m_pTexMatrix[0] = &t_sMatAll;
	m_sHardTile[_idx].m_pTexMatrix[1] = &m_sHtTexMat[_idx];
	m_sHardTile[_idx].m_rectangle.x = m_xbase;
	m_sHardTile[_idx].m_rectangle.y = m_zbase;
	m_sHardTile[_idx].m_rectangle.z = m_xbase + MapTile::CHUNK_SIZE;
	m_sHardTile[_idx].m_rectangle.w = m_zbase + MapTile::CHUNK_SIZE;
	
	osassert( m_sHardTile[_idx].m_iIndexBufId >= 0 );

	return &m_sHardTile[_idx];

	unguard;
}


//! 释放hardTile的设备相关数据
void MapChunk::release_hardTile( void )
{
	guard;

	for( int t_i=0;t_i<m_iHardTileTypeNum;t_i ++ )
	{
		if( m_sHardTile[t_i].m_iIndexBufId >= 0 )
		{
			m_ptrMidPipe->release_staticIB( m_sHardTile[t_i].m_iIndexBufId );
			m_sHardTile[t_i].m_iIndexBufId = -1;
		}
		if( m_sHardTile[t_i].m_wTexId[1]>=0 )
		{
			g_shaderMgr->getTexMgr()->release_texture( m_sHardTile[t_i].m_wTexId[1] );		
			m_sHardTile[t_i].m_wTexId[1] = -1;
		}
	}

	unguard;
}


//! River @ 2009-9-15: 当前Chunk处理对应的硬接触地表块
bool MapChunk::process_hardTile( osc_chunkHardTile* _hardTile )
{
	guard;

	//! 每一chunk最多3个硬地块
	osassert( _hardTile->m_iHardTileNum<=MAX_HARDTILE_PERCHUNK );

	//! 先释放.
	release_hardTile();

	if( _hardTile->m_iHardTileNum <= 0 )
	{
		m_iHardTileTypeNum = 0;
		return true;
	}

	m_iHardTileTypeNum = _hardTile->m_iHardTileNum;

	// 如果当前的chunk内hardTile大于零，则需要处理,对每一个hardtile
	// 处理对应的设备相关数据
	/*
	1: 创建对应的索引缓冲区。
	2: 创建对应的硬接触纹理。
	*/
	s_string  t_szFilename;
	for( int t_i=0;t_i<_hardTile->m_iHardTileNum;t_i ++ )
	{
		osc_hardTextureTile*   t_ptrTT = &_hardTile->m_arrTileInChunk[t_i];

		m_sHardTile[t_i].m_iIndexBufId = 
			construct_hardTileIdxBuf( t_ptrTT );
		m_sHardTile[t_i].m_dwPriType = TRIANGLELIST;
		m_sHardTile[t_i].m_iPrimitiveNum = t_ptrTT->m_iTileNum*2;

		t_szFilename = "data\\";
		t_szFilename += t_ptrTT->m_szTexName;
		osn_mathFunc::change_extName( t_szFilename,"dds" ); 
		m_sHardTile[t_i].m_wTexId[1] = g_shaderMgr->getTexMgr()->
			dadd_textomanager( (char*)t_szFilename.c_str(),0 );	
		osMatrixIdentity( &m_sHtTexMat[t_i] );

		// 
		//! River @ 2010-4-6:根据不同的PixelShader版本号来做出对应的处理。
		//!                  如果pixel的shader版本号低于2.0,则只处理缩放。
		if( g_iMajorPSVersion >= 2 )
		{
			// River Added @ 2009-10-23:fx文件内直接使用旋转矩阵来处理。 
			D3DXMatrixAffineTransformation2D( &m_sHtTexMat[t_i],
				float(MapTile::GRIDS*t_ptrTT->m_wU),
				&D3DXVECTOR2( 0.5f,0.5f ),
				OS_PI*t_ptrTT->m_rotation/2.0f,
				&D3DXVECTOR2( 0.0f,0.0f ) );
		}
		else
		{
			m_sHtTexMat[t_i]._11 = float(MapTile::GRIDS*t_ptrTT->m_wU);
			m_sHtTexMat[t_i]._22 = float(MapTile::GRIDS*t_ptrTT->m_wV);
		}

	}

	return true;

	unguard;
}


//=================================================================================
void MapChunk::LoadChunk(const BYTE *fpBegin,DWORD offset)
{
	guard;

	osassert( fpBegin );

	m_xbase = (float)m_px*MapTile::CHUNK_SIZE+m_mt->m_xbase;
	m_zbase = (float)m_py*MapTile::CHUNK_SIZE+m_mt->m_zbase;

	const  os_chunkData *pChunkheader;

	m_PassList.clear();
	pChunkheader = (const os_chunkData*)(fpBegin+offset);
	
	for (int i = 0; i < pChunkheader->m_dwTexNum; i++)
	{
		sPassInfo	pass;
		pass.m_TexIdx = pChunkheader->m_dwTexPtr[i];
		if (i ==0){	//!第0层没有ALPHA
			m_PassList.push_back(pass);
			continue;
		}
		pass.m_pAlpha = m_AlphaPool[i-1];
		READ_MEM(pass.m_pAlpha,fpBegin + 
			pChunkheader->m_dwAlphaDataOffset + (i-1)*m_AlphaSize*m_AlphaSize,m_AlphaSize*m_AlphaSize);

		m_PassList.push_back(pass);
	}
	
	const os_newFileHeader *pHeader = (const os_newFileHeader *)(fpBegin);


	// 
	// River mod 2006-2-25: 必须设为false,不然地图释放后再调入，执行不到一些初始化操作。
	// 初始化的可视性为false
	m_IsAcitve = false;
	m_bIsAlphaInit = false;
	m_IsVisible = false;
	if (!m_LightMap)
	{
		m_LightMap = new char [MapTile::ms_LightMapSize*MapTile::ms_LightMapSize];
		m_LightMapL8 = new BYTE[MapTile::ms_LightMapSize*MapTile::ms_LightMapSize];
	}

	if (pHeader->m_dwVersion>=15)
	{
		READ_MEM(m_LightMap,fpBegin + pChunkheader->m_dwLightMapDataOffset,
			MapTile::ms_LightMapSize*MapTile::ms_LightMapSize*sizeof(char));
	}
	else
	{
		ZeroMemory(m_LightMap,MapTile::ms_LightMapSize*MapTile::ms_LightMapSize*sizeof(char));
		ZeroMemory(m_LightMapL8,MapTile::ms_LightMapSize*MapTile::ms_LightMapSize);
	}

	unguard;
}
//=================================================================================
unsigned short	MapChunk::GetIndex(int nX,int nY)
{
	int idx = nX+nY*MapTile::CHUNK_PIXEL;
	assert(nX>=0&&nX<MapTile::CHUNK_PIXEL);
	assert(nY>=0&&nY<MapTile::CHUNK_PIXEL);
	assert(idx<MapTile::MAX_VERTICES);
	if (m_IndexMap[idx]==0xffff){
		m_IndexMap[idx] = (unsigned short)m_NumNewVertices;
		GetVertex(nX,nY,m_Vertices[m_NumNewVertices].pos);
		GetTexCoord(nX,nY,m_Vertices[m_NumNewVertices].tex);
		GetDiffuse(nX,nY,m_Vertices[m_NumNewVertices].color);
		
		// River Mod: 加入顶点的Normal信息
		m_mt->GetNormal( m_px*MapTile::GRIDS+nX,m_py*MapTile::GRIDS+nY,
			m_Vertices[m_NumNewVertices].normal );

		m_NumNewVertices++;
	}
	return m_IndexMap[idx];
}
//=================================================================================
void MapChunk::GetVertex(int nX,int nY,osVec3D & pt)
{
	assert( nX>=0&&nX<MapTile::CHUNK_PIXEL );
	assert( nY>=0&&nY<MapTile::CHUNK_PIXEL );
	pt.x = m_xbase+nX*MapTile::GRID_SIZE;
	pt.y = GetHeight(nX,nY);
	pt.z = m_zbase+nY*MapTile::GRID_SIZE;
}
//=================================================================================
float MapChunk::GetHeight(int nX,int nY)
{
	assert(nX>=0 && nX<MapTile::CHUNK_PIXEL);
	assert(nY>=0 && nY<MapTile::CHUNK_PIXEL);
	assert(m_mt);
	return m_mt->GetHeight(m_px*MapTile::GRIDS+nX,m_py*MapTile::GRIDS+nY);
}
//=================================================================================
void MapChunk::GetTexCoord(int nX,int nY,osVec2D & TCood)
{
	assert(nX>=0 && nX<MapTile::CHUNK_PIXEL);
	assert(nY>=0 && nY<MapTile::CHUNK_PIXEL);
	assert(m_mt);
	TCood.x = (float)nX/(MapTile::CHUNK_PIXEL-1);
	TCood.y = (float)nY/(MapTile::CHUNK_PIXEL-1);
}
//=================================================================================
void MapChunk::GetDiffuse(int nX,int nY, DWORD &Diffuse)
{
	assert(nX>=0 && nX<MapTile::CHUNK_PIXEL);
	assert(nY>=0 && nY<MapTile::CHUNK_PIXEL);
	assert(m_mt);

	int pos = nY*MapTile::CHUNK_PIXEL+nX;
	
	m_mt->GetDiffuse(m_px*MapTile::GRIDS+nX,m_py*MapTile::GRIDS+nY,(D3DCOLOR)Diffuse);

}
//=================================================================================
void MapChunk::AddIndex(unsigned short nIdx)
{
	m_Indices[m_NumNewIndices++] = nIdx;
}
//=================================================================================
void MapChunk::AddLastIndexAgain()
{
	if (m_NumNewIndices > 0){
		m_Indices[m_NumNewIndices++] = m_Indices[m_NumNewIndices-1];
	}
}
//=================================================================================
void MapChunk::InitAlphaLMData()
{
	guard;


	SafeDeInitAlphaLMData();
	
	osassert( m_PassList[0].m_pAlpha==NULL );

	char sztexname[MAX_PATH];
	BYTE tempchar[ALPHAMAP_SIZE*ALPHAMAP_SIZE*4*4];
	ZeroMemory(tempchar,ALPHAMAP_SIZE*ALPHAMAP_SIZE*4*4);
	
	//!
	//!
	///TO DO FIXED,使用 DXT5 代替A8R8G8B8
	sprintf(sztexname,"%s_%d_%d_%d_%d_lightMap",m_mt->m_mapname.c_str(),
		m_mt->m_xpos,m_mt->m_zpos,m_px,m_py);
	m_LightMapID = g_shaderMgr->getTexMgr()->
		addnew_texture(sztexname,MapTile::ms_LightMapSize,MapTile::ms_LightMapSize,D3DFMT_DXT5);

	osc_texture * pTex  = g_shaderMgr->getTexMgr()->get_textureById(m_LightMapID);

	D3DLOCKED_RECT   t_rect;
	IDirect3DTexture9*  pD3DLightMap = (IDirect3DTexture9* )pTex->get_texpointer();
	pD3DLightMap->LockRect( 0,&t_rect,NULL,NULL );
	
	unsigned char * pDstRow = (unsigned char *)t_rect.pBits;
	unsigned char * pSrcBuf = (unsigned char * )m_LightMap;

	int Pitch = t_rect.Pitch;
	int ofsWidth = 0 ;

	//DXT5 format
	static const int block_width = 4;
	static const int block_height = 4;
	static const int block_size = 16;
	int texture_width = (MapTile::ms_LightMapSize/block_width)*block_size; 
	for ( int i = 0 ;i < MapTile::ms_LightMapSize/block_height; i++ ){
		memcpy(pDstRow+ofsWidth,pSrcBuf+texture_width*i,texture_width);
		ofsWidth+=Pitch;
	}
	pD3DLightMap->UnlockRect( 0 );


	//@{ prepare the light map L8 fromat
	//
	LPDIRECT3DTEXTURE9 t_tmpTex = NULL;
	D3DXCreateTexture(m_ptrMidPipe->get_device(),MapTile::ms_LightMapSize,MapTile::ms_LightMapSize,
										1,0,D3DFMT_A8,D3DPOOL_MANAGED,&t_tmpTex);
	if(t_tmpTex){

		LPDIRECT3DSURFACE9 t_dest;
		LPDIRECT3DSURFACE9 t_src;

		t_tmpTex->GetSurfaceLevel(0,&t_dest);
		pD3DLightMap->GetSurfaceLevel(0,&t_src);

		D3DXLoadSurfaceFromSurface(t_dest,NULL,NULL,t_src,NULL,NULL,D3DX_DEFAULT,0);

		SAFE_RELEASE(t_dest);
		SAFE_RELEASE(t_src);

		D3DLOCKED_RECT   t_rect;
		t_tmpTex->LockRect(0,&t_rect,NULL,NULL);

		Pitch = t_rect.Pitch;
		pDstRow = m_LightMapL8;
		pSrcBuf = (BYTE*)t_rect.pBits;
		for(int i = 0;i < MapTile::ms_LightMapSize; i++){
			CopyMemory(pDstRow,pSrcBuf,MapTile::ms_LightMapSize);
			pSrcBuf += Pitch;
			pDstRow += MapTile::ms_LightMapSize;
		}
		
		t_tmpTex->UnlockRect(0);
		SAFE_RELEASE(t_tmpTex);
	}
	//@}

	//pTex->setPixels((const unsigned char*)m_LightMap);


	if (gUseTerrainPS)
	{
		for( int t_i = 0;t_i<MAX_TEXNUM;t_i ++ )
			m_PSPassMeshData.m_wTexId[t_i] = -1;

		//!geomipmapPS.fx文件中,第0层是光照图
		m_PSPassMeshData.m_wTexId[0] = m_LightMapID;

	
		for( int t_i=0;t_i<(int)m_PassList.size();t_i ++ )
		{
			m_PSPassMeshData.m_wTexId[t_i+1] = m_mt->m_Pass[m_PassList[t_i].m_TexIdx].RepTex;
			m_PSPassMeshData.m_pTexMatrix[t_i+1] = get_chunkTexMatrix( t_i );;
		}
		//!混合四层ALPHA信息到一个贴图中
	
		sprintf(sztexname,"%s_%d_%d_%d_%d_combineAlpha",m_mt->m_mapname.c_str(),
			m_mt->m_xpos,m_mt->m_zpos,m_px,m_py);

		m_CombineAlphaID = g_shaderMgr->getTexMgr()->
			addnew_texture(sztexname,m_AlphaSize,m_AlphaSize,D3DFMT_A8R8G8B8);
		//! 设置混合alpha贴图EFFECT 相关ID
		m_PSPassMeshData.m_wTexId[m_PassList.size()+1] = m_CombineAlphaID;
		osc_texture * pTex = g_shaderMgr->getTexMgr()->get_textureById(m_CombineAlphaID);

		
		osassert(m_PassList.size()<=4);   //!alpha图层不能大于4
		//! 生成多层地表alpha混合后的贴图,RED-1层,GREEN-2层,BLUE-3层.
		for (DWORD pos = 0; pos < m_AlphaSize*m_AlphaSize;pos++)
		{
			for (size_t i = 1; i < m_PassList.size();i++)
			{
				tempchar[pos*4+i-1] = m_PassList[i].m_pAlpha[pos];
			}
		}
	
		
		pTex->setPixels(tempchar);
	
	}
	else
	{
		// 初始化渲染相关的数据
		for (size_t i = 1; i < m_PassList.size();i++)
		{

			sprintf(sztexname,"%s_%d_%d_%d_%d_%d",m_mt->m_mapname.c_str(),
				m_mt->m_xpos,m_mt->m_zpos,m_px,m_py,i);
			m_PassList[i].m_AlphaID = g_shaderMgr->getTexMgr()->
				addnew_texture(sztexname,m_AlphaSize,m_AlphaSize,D3DFMT_A8R8G8B8);

			for (DWORD pos = 0; pos < m_AlphaSize*m_AlphaSize;pos++)
			{
				//memcpy(tempchar+pos*4,m_LightMap+pos*3,3*sizeof(BYTE));
				*(tempchar+pos*4+3)=m_PassList[i].m_pAlpha[pos];
			}
			osc_texture * pTex = g_shaderMgr->getTexMgr()->get_textureById(m_PassList[i].m_AlphaID);
			pTex->setPixels(tempchar);
		}

		//! ver 16 add 光照信息
		m_vecPassMeshData.resize( m_PassList.size());

		for( int t_i=0;t_i<(int)m_PassList.size();t_i ++ )
		{

			// 纹理id和纹理矩阵
			m_vecPassMeshData[t_i].m_wTexId[0] = 
				m_mt->m_Pass[m_PassList[t_i].m_TexIdx].RepTex;

			if( t_i == 0 )
				m_vecPassMeshData[t_i].m_wTexId[1] = -1;
			else
				m_vecPassMeshData[t_i].m_wTexId[1] = m_PassList[t_i].m_AlphaID;
			m_vecPassMeshData[t_i].m_pTexMatrix[0] = get_chunkTexMatrix( t_i );
		}

		//! ver 16 add
		m_LightMapMestData.m_wTexId[0] = m_LightMapID;
	}

	unguard;
}
//=================================================================================
void MapChunk::SafeDeInitAlphaLMData()
{
	guard;
	g_shaderMgr->getTexMgr()->release_texture(m_LightMapID);
	if (gUseTerrainPS)
	{
		g_shaderMgr->getTexMgr()->release_texture(m_CombineAlphaID);
		
	}
	else
	{
		for ( size_t i = 1; i < m_PassList.size();i++ )
		{
			g_shaderMgr->getTexMgr()->release_texture(m_PassList[i].m_AlphaID);
		}
		// River mod 2006-2-25:
		m_vecPassMeshData.clear();
	}
	unguard;

}
