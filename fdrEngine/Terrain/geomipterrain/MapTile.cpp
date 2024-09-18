//===================================================================
/** \file  
* Filename :   MapTile.cpp
* Desc     :   
* His      :   Windy create @2005-12-6 11:46:00
*/
//===================================================================
# include "stdafx.h"
#include "maptile.h"
#include "MapChunk.h"
//#include "Frustum.h"
#include "FCMapChunk.h"
#include "newHeader.h"
#include "MapHelper.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../mfpipe/include/osShader.h"
# include "../../mfpipe/include/osTexture.h"
#include "../../interface/fileOperate.h"
# include "../../interface/osInterface.h"
#include "../../../common/com_include.h"
//#include "tga.h"
# include "osMapTerrain.h"
# include "../include/fdNewFieldStruct.h"

LPDIRECT3DVERTEXDECLARATION9	MapTile::m_pDeclaration = NULL;

//!��Ⱦ��ʽ
MapTile::RENDERMODE MapTile::s_RenderMode = MapTile::EMT_NOLOD;

int MapTile::ms_LightMapSize = 256;
//! River added @ 2006-1-12:��mapTile֮�ϵ�mapTerrain
osc_mapTerrain* MapTile::m_ptrMapTerrain = NULL;
//! �ļ���������
static osc_mapTerrain t_sMapTerrain;


MapTile::MapTile(void):m_xpos(-1), m_zpos(-1)														
{
	m_HMData = new float [TILE_PIXEL*TILE_PIXEL];
	m_vecNormal = new osVec3D[TILE_PIXEL*TILE_PIXEL];
	m_DiffuseData = new D3DCOLOR[TILE_PIXEL*TILE_PIXEL];
	m_OrigDiffuseData = new D3DCOLOR[TILE_PIXEL*TILE_PIXEL];
	m_AmbientDiffuseData = new D3DCOLOR[TILE_PIXEL*TILE_PIXEL];
	for (int j=0; j<MapTile::CHUNKS; j++) {
		for (int i=0; i<MapTile::CHUNKS; i++) 
		{		
			m_chunks[i][j] = m_ptrMapTerrain->create_chunk( this,i,j );
		}
	}
	
}
//=================================================================================
MapTile::~MapTile(void)
{
	SAFE_DELETE_ARRAY(m_HMData);
	SAFE_DELETE_ARRAY( m_vecNormal );
	SAFE_DELETE_ARRAY(m_DiffuseData);
	SAFE_DELETE_ARRAY(m_OrigDiffuseData);
	SAFE_DELETE_ARRAY(m_AmbientDiffuseData);
	for (int j=0; j<CHUNKS; j++) {
		for (int i=0; i<CHUNKS; i++) {	
			m_chunks[j][i]->DeInit();
			SAFE_DELETE(m_chunks[j][i]);
		}
	}
	SafeReleaseDeviceData();
	m_ptrMapTerrain->m_vecChunkPtr.clear();
	m_ptrMapTerrain->m_vecActiveChunkPtr.clear();
	m_ptrMapTerrain->m_vecMapTilePtr.clear();
	osc_mapTerrain::m_sRenderMesh.m_vecRenderDataPtr.clear();
	osc_mapTerrain::m_sLightMapRenderMesh.m_vecRenderDataPtr.clear();
	for(int i = 0;i< MAX_LIGHTNUM;i++){
		osc_mapTerrain::m_sRenderDLight[i].m_vecRenderDataPtr.clear();
	}
	osc_mapTerrain::m_sRenderMeshPS_PassList[0].m_vecRenderDataPtr.clear();
	osc_mapTerrain::m_sRenderMeshPS_PassList[1].m_vecRenderDataPtr.clear();
	osc_mapTerrain::m_sRenderMeshPS_PassList[2].m_vecRenderDataPtr.clear();
	osc_mapTerrain::m_sRenderMeshPS_PassList[3].m_vecRenderDataPtr.clear();

	osc_mapTerrain::m_sRenderMeshDepth_Pass.m_vecRenderDataPtr.clear();
}

//=================================================================================
void MapTile::PushActiveChunk()
{
	guard;
	for (int i = 0;i <CHUNKS;i++)
	{
		for (int j = 0 ;j<CHUNKS;j++)
		{
			m_chunks[j][i]->DeInitUnVisible();
			if (m_chunks[j][i]->m_IsVisible)
			{		
				// River @ 2007-1-27:�˴�����Ҫ���еڶ������á�
				// m_chunks[j][i]->SetVisible(true);

				m_ptrMapTerrain->push_activeChunkPtr( m_chunks[j][i] );
			}
		}
	}
	unguard;
}
//=================================================================================
void MapTile::DrawTerrain( BOOL _envMap/* = FALSE*/ ,BOOL _depth/* = FALSE*/)
{
	guard;

	m_ptrMapTerrain->frame_processTerrain( _envMap,_depth );
	
	unguard;
}


//! ��տɼ��ĵ��ο�
void MapTile::ClearVisibleTerr( void )
{
	guard;

	m_ptrMapTerrain->clearVisible();

	unguard;
}


//=================================================================================
float MapTile::GetHeight(int nX,int nY)
{
	assert(nX>=0&&nX<TILE_PIXEL);
	assert(nY>=0&&nY<TILE_PIXEL);
	return (float)m_HMData[nX+nY*TILE_PIXEL]*HEIGHT_SIZE;
}
//! �õ�һ������������Ӧ��Normal
void MapTile::GetNormal( int _x,int _y,osVec3D& _normal )
{
	guard;

	assert( (_x>=0) && (_x<TILE_PIXEL) );
	assert( (_y>=0) && (_y<TILE_PIXEL) );
	_normal = this->m_vecNormal[_y*TILE_PIXEL + _x];
	return;

	unguard;
}


//=================================================================================
void MapTile::GetTexCoord(int nX,int nY,osVec2D &vTCood)
{
	assert(false);
}
//=================================================================================
void MapTile::GetDiffuse(int nX, int nY, D3DCOLOR &_diffuse)
{
	assert(nX>=0&&nX<TILE_PIXEL);
	assert(nY>=0&&nY<TILE_PIXEL);


	_diffuse = m_DiffuseData[nX+nY*TILE_PIXEL];

}
void MapTile::GetDiffuseOrig(int nX, int nY, D3DXCOLOR &_diffuse)
{
	assert(nX>=0&&nX<TILE_PIXEL);
	assert(nY>=0&&nY<TILE_PIXEL);
	_diffuse = m_DiffuseData[nX+nY*TILE_PIXEL];
}
D3DCOLOR &	MapTile::GetDiffuse(int nX, int nY)
{
	assert(nX>=0&&nX<TILE_PIXEL);
	assert(nY>=0&&nY<TILE_PIXEL);
	return m_DiffuseData[nX+nY*TILE_PIXEL];
}

//! ����ǰmaptile��ÿһ��chunk�ڵ�Ӳ�Ӵ��ر�����
bool MapTile::process_hardTex( osc_chunkHardTile* _hardTile )
{
	guard;	
	osassert( _hardTile );

	osc_chunkHardTile* t_ptrHT;
	//! ��ÿһ��chunk���д���
	for (int j=0; j<MapTile::CHUNKS; j++) 
	{
		for (int i=0; i<MapTile::CHUNKS; i++) 
		{		
			int     t_idx = j*MapTile::CHUNKS+i;
			
			t_ptrHT = &_hardTile[t_idx];
			if( !m_chunks[i][j]->process_hardTile( t_ptrHT ) )
				return false;
		}
	}

	return true;

	unguard;
}


//=================================================================================
bool MapTile::LoadMapTile(const BYTE * _pFileBegin,const char* _szmapName)
{
	guard;

	osassert( _pFileBegin );
	osassert( _szmapName );

	SafeReleaseDeviceData();
	const BYTE*	fileBegin = _pFileBegin;
	m_mapname = _szmapName;
	const os_newFileHeader *pHeader = (const os_newFileHeader *)(fileBegin);
	
	if (strcmp(pHeader->m_szMagic,TGFILE_HEADER)){
		osassertex(false,"��ͼ�ļ���ʶID����ȷ");
	}
	m_xpos = pHeader->m_wMapPosX;
	m_zpos = pHeader->m_wMapPosZ;
	m_xbase = (float)m_xpos*TILE_SIZE;
	m_zbase = (float)m_zpos*TILE_SIZE;

	
	READ_MEM(m_HMData,fileBegin + pHeader->m_dwVerHeightOffset,TILE_PIXEL*TILE_PIXEL*sizeof(float));
	READ_MEM(m_DiffuseData,fileBegin + pHeader->m_dwVertColorOffset,
		TILE_PIXEL*TILE_PIXEL*sizeof(D3DCOLOR) );

	// River: ����Normal����
	READ_MEM( m_vecNormal,fileBegin + pHeader->m_dwVertNormalOffset,
		TILE_PIXEL*TILE_PIXEL*sizeof(osVec3D) );

	memcpy(m_OrigDiffuseData,m_DiffuseData,TILE_PIXEL*TILE_PIXEL*sizeof(D3DCOLOR));
	memcpy(m_AmbientDiffuseData,m_DiffuseData,TILE_PIXEL*TILE_PIXEL*sizeof(D3DCOLOR));



	const os_chunkTexture *pTexinfo;
	
	for (DWORD i = 0 ; i< pHeader->m_dwTextureNum; i++)
	{
		PassInfo pass;
		//READ_MEM(&texinfo,fileBegin + header.m_dwTexDataOffset +i*sizeof(os_chunkTexture),sizeof(os_chunkTexture));
		pTexinfo = (const os_chunkTexture *)(fileBegin + pHeader->m_dwTexDataOffset +i*sizeof(os_chunkTexture));
		std::string strfname(pTexinfo->m_szTexName);
		fixtexname(strfname,_szmapName);
		pass.texname = strfname;
		//��ȡ���������
		int t_i = sizeof( short );
		pass.vRepeat.x = float(pTexinfo->m_wRepeatTimesU);
		pass.vRepeat.y = float(pTexinfo->m_wRepeatTimesV);
		osMatrixIdentity(&pass.mat);

		pass.mat._11 = pass.vRepeat.x;
		pass.mat._22 = pass.vRepeat.y;
		m_Pass.push_back(pass);
	}
	m_AlphaSize = pHeader->m_dwAlphaSize;
	//! ver 16 add  �����Ʒ��Ӱ��Ϣ ,�˴��������ͼ��С
	if (pHeader->m_dwVersion>=15)
	{
		ms_LightMapSize = pHeader->m_dwLightMapSize;
	}
	

	int os_chunk_size = sizeof(os_chunkData);
	if (pHeader->m_dwVersion<=15)
	{
		os_chunk_size -=  sizeof(int);
	}

	for (int j=0; j<MapTile::CHUNKS; j++) {
		for (int i=0; i<MapTile::CHUNKS; i++) {		
			m_chunks[i][j]->LoadChunk(fileBegin , pHeader->m_dwChunkDataOffset + os_chunk_size*(i+j*MapTile::CHUNKS));
			//to do........
			if (MapTile::s_RenderMode!=EMT_NOLOD){
				m_chunks[i][j]->Setup();
			}
		}
	}
	//!��������CHUNK֮��Ĺ�ϵ
	if (MapTile::s_RenderMode!=EMT_NOLOD){
		m_ptrMapTerrain->SetNeighbors( this );
	}


	return true;
	
	unguard;
}
//=================================================================================
bool MapTile::InitDeviceData()
{
	guard;

	for (size_t i =0 ; i< m_Pass.size();i++)
	{
		m_Pass[i].RepTex = g_shaderMgr->getTexMgr()->
			dadd_textomanager((char *)m_Pass[i].texname.c_str(),0);

		if (m_Pass[i].RepTex == -1){
			return false;
		}
	}


	return true;

	unguard;
}
//=================================================================================
void MapTile::SetChunkVisible(int _idxX,int _idxY,bool _IsbVisible)
{
	guard;
	assert(_idxX>=0&&_idxX<CHUNKS);
	assert(_idxY>=0&&_idxY<CHUNKS);
	m_chunks[_idxX][_idxY]->SetVisible( _IsbVisible );
	unguard;
}

/** \brief
*  �ϲ�����ĳһ��Chunk�Ƿ��ܶ�̬���Ӱ��
*/
void MapTile::setChunkAffectByDl( int _idxX,int _idxY )
{
	assert(_idxX>=0&&_idxX<CHUNKS);
	assert(_idxY>=0&&_idxY<CHUNKS);
	m_chunks[_idxX][_idxY]->m_bAffectByDLight = true;
}


//=================================================================================
void MapTile::ClearChunks()
{
	for (int i = 0;i <CHUNKS;i++){
		for (int j = 0 ;j<CHUNKS;j++){
			m_chunks[i][j]->m_IsVisible = false;
			m_chunks[i][j]->m_bAffectByDLight = false;
		}
	}
}


//=================================================================================
void MapTile::SafeReleaseDeviceData()
{
	guard;

	for (unsigned int pass = 0;pass<m_Pass.size();pass++)
	{
		g_shaderMgr->getTexMgr()->release_texture(m_Pass[pass].RepTex);
	}

	// River mod @ 2006-2-25:ɾ��mapTile��pass��ص�����
	m_Pass.clear();
	

	unguard;
}
void MapTile::OneTimeDeinit()
{
	guard;

	SAFE_RELEASE( MapTile::m_pDeclaration );
	osc_mapTerrain::oneTimeDeinit();
	unguard;
}
//=================================================================================
void MapTile::OneTimeInit( osc_middlePipe* _pipe )
{
	guard;
	osassert( _pipe );

	MapChunk::OneTimeInit( _pipe );
	osc_mapTerrain::oneTimeInit();

	switch( s_RenderMode ) {
	case EMT_BRUTEFORCE:
	case EMT_NOLOD:
		{
			D3DVERTEXELEMENT9 t_decl2[] =
			{
				// λ��.
				{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, 
				D3DDECLUSAGE_POSITION, 0 },

				// ������.
				{ 0, 12,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,
				D3DDECLUSAGE_NORMAL, 0 },

				// ����ɫ
				{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,
				D3DDECLUSAGE_COLOR, 0 },

				// ��������
				{ 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
				D3DDECLUSAGE_TEXCOORD, 0 },
				

				D3DDECL_END()
			};

			if( FAILED( g_frender->get_d3dDevice()->CreateVertexDeclaration( 
				t_decl2, &m_pDeclaration ) ) )
				osassertex( false,"�����ر��õ��Ķ�������ʧ��...\n" );
		}

		break;
	case EMT_MORPH_SW:
		m_pDeclaration = SWVertex::vertexDescription().createDecl(g_frender->get_d3dDevice());
		break;
	default:
		return ;
	}

	unguard;
}
void MapTile::ClearDynamic_Light()
{
//	memcpy(m_DiffuseData,m_AmbientDiffuseData,TILE_PIXEL*TILE_PIXEL*4);
}
//=================================================================================
BOOL MapTile::SetDynamic_Light(osc_dlight* _dLight,int _lightID)
{
	guard;
	//!�������궼���Ե�ǰTILE��0,0��Ϊԭ��,��3��Ϊһ����λ,�Ķ�ά����ϵ
	int t_lightMaxX,
		t_lightMaxZ,
		t_lightMinX,
		t_lightMinZ;
	int t_lightCenterX,
		t_lightCenterZ;
	int t_lightExtenX,
		t_lightExtenZ;

	int t_MapMinX =0,
		t_MapMinZ = 0,
		t_MapMaxX = 64,
		t_MapMaxZ = 64;

	//!�ѹ����������ת������ͼ�ľֲ�����ϵ��
	t_lightCenterX = (int)(_dLight->m_vec3Pos.x - m_xbase)/3;
	if ((_dLight->m_vec3Pos.x-(t_lightCenterX+m_xpos*t_MapMaxX)*3.0f)>1.5f)
	{
		t_lightCenterX++;
	}
	t_lightCenterZ = (int)(_dLight->m_vec3Pos.z - m_zbase)/3;
	if ((_dLight->m_vec3Pos.z-(t_lightCenterZ+m_zpos*t_MapMaxZ)*3.0f)>1.5f)
	{
		t_lightCenterZ++;
	}
	
	t_lightExtenX = (int)_dLight->m_fRadius/3;
	t_lightExtenZ = t_lightExtenX;

	t_lightMaxX = t_lightCenterX+t_lightExtenX;
	t_lightMaxZ = t_lightCenterZ+t_lightExtenZ;
	t_lightMinX = t_lightCenterX-t_lightExtenX;
	t_lightMinZ = t_lightCenterZ-t_lightExtenZ;

	//!�жϹ�Ӱ��ľ����ǲ����ڵ�ǰ���ο���.
	/*if ((t_lightMaxZ<t_MapMinZ||t_lightMaxX<t_MapMinX
		||t_MapMaxX<t_lightMinX||t_MapMaxZ<t_lightMinZ)){
		return false;
	}*/
	t_lightMinX = t_lightMinX<0?0:t_lightMinX;
	t_lightMinZ = t_lightMinZ<0?0:t_lightMinZ;
	t_lightMaxX = t_lightMaxX>64?64:t_lightMaxX;
	t_lightMaxZ = t_lightMaxZ>64?64:t_lightMaxZ;
	//!�����ķ�Χ
	float         t_dis;
	osVec3D       t_vec3;
	osColor       t_dstColor,t_srcColor;

	PointDiffuseVec::iterator it,
		end = m_LitDiffList[_lightID].end();
	for (it = m_LitDiffList[_lightID].begin();it!=end;++it)
	{
		GetDiffuse(it->iX,it->iY) =it->diffuse;
	}

	m_LitDiffList[_lightID].clear();
	for (int x = t_lightMinX; x <= t_lightMaxX ; ++x)
	{
		for (int y = t_lightMinZ ; y <= t_lightMaxZ ; ++y)
		{
			t_vec3.x = _dLight->m_vec3Pos.x - (x*3.0f + m_xbase);
			//t_vec3.y = _dLight->m_vec3Pos.y - GetHeight(x,y);
			t_vec3.y = 0.0f;
			t_vec3.z = _dLight->m_vec3Pos.z - (y*3.0f + m_zbase);
		//	t_vec3.x = (float)(t_lightCenterX - x) ;
			//t_vec3.z = (float)(t_lightCenterZ - y) ;
			t_dis = osVec3Length( &t_vec3 );
			t_dis /= _dLight->m_fRadius;
			if( t_dis > 1.0f )
				t_dis = 1.0f;

			t_dis = 1.0f - t_dis;
			t_dstColor = _dLight->m_dlightColor;

			GetDiffuseOrig(x,y,t_srcColor);
			// �뵱ǰ��������ɫ���бȽϣ�ȡ�ϸߵ���ɫ��Ϊ���յ���ɫ��
			if( t_dstColor.r < t_srcColor.r )
				t_dstColor.r = t_srcColor.r;
			if( t_dstColor.g < t_srcColor.g )
				t_dstColor.g = t_srcColor.g;
			if( t_dstColor.b < t_srcColor.b )
				t_dstColor.b = t_srcColor.b;

			osColorLerp( &t_dstColor,&t_srcColor,&t_dstColor,t_dis );
			
			GetDiffuse(x,y) = t_dstColor;

			PointDiffuse pointColor;
			pointColor.iX = x;
			pointColor.iY = y;
			pointColor.diffuse = t_srcColor;
			m_LitDiffList[_lightID].push_back(pointColor);
		}
	}
	for (int ChunkX = t_lightMinX/16; ChunkX<=t_lightMaxX/16;++ChunkX)
	{
		for (int ChunkY = t_lightMinZ/16; ChunkY<=t_lightMaxZ/16;++ChunkY)
		{
			//ChunkX = ChunkX>=3?3:ChunkX;
			//ChunkY = ChunkY>=3?3:ChunkY;
			if (ChunkX<4&&ChunkY<4)
			{
				m_chunks[ChunkX][ChunkY]->m_ForceBufferCreate = true;
			}
		}
	}
	return true;
	unguard;

}
void MapTile::ReCalcChunkDynamic_Light()
{
	for (int i =0 ; i< CHUNKS;++i)
	{
		for (int j =0 ; j < CHUNKS; ++j)
		{
			m_chunks[i][j]->m_ForceBufferCreate = true;
		}
	}

}
void MapTile::SetambientLCoeff(float _r,float _g,float _b)
{
	guard;
	
	for( int t_y=0;t_y<TILE_PIXEL;t_y++ )
	{
		for( int t_x=0;t_x<TILE_PIXEL;t_x++ )
		{
			osColor   t_color( m_OrigDiffuseData[t_y*TILE_PIXEL+t_x]);

			t_color.r *= _r;
			t_color.g *= _g;
			t_color.b *= _b;

			// d3dxcolor�Ѿ�������ÿһ��ɫ��������255�����Ρ�
			m_AmbientDiffuseData[t_y*TILE_PIXEL+t_x] = t_color;
		}
	}
//	memcpy(m_DiffuseData,m_AmbientDiffuseData,TILE_PIXEL*TILE_PIXEL*4);
	for (int i =0 ; i< CHUNKS;++i)
	{
		for (int j =0 ; j < CHUNKS; ++j)
		{
			m_chunks[i][j]->m_ForceBufferCreate = true;
		}
	}

	unguard;
}