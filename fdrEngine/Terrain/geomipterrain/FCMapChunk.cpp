//===================================================================
/** \file  
* Filename :   FCMapChunk.cpp
* Desc     :   
* His      :   Windy create @2005-12-6 16:34:55
*/
//===================================================================
# include "stdafx.h"
#include "fcmapchunk.h"
#include "MapTile.h"
# include "../../mfpipe/include/middlePipe.h"

//! ���������,�����е�FcChunkֻ��Ҫʹ��һ����̬������������
int FCMapChunk::m_iSIdxBufId = -1;


FCMapChunk::FCMapChunk(MapTile * mt,int x,int y):MapChunk(mt,x,y)
{
	m_iVerBufId = -1;
	
}
//=================================================================================
FCMapChunk::~FCMapChunk(void)
{
}
void FCMapChunk::Init()
{
	MapChunk::Init();
}

void FCMapChunk::DeInit()
{
	MapChunk::DeInit();
	if (m_iVerBufId ==-1){
		return;
	}

	// River mod @ 2007-3-30:���㻺�������ͷ�.
	/*
	// ��Ϊ����idxBuf,���Բ���Ҫ�ͷ�IdxBuf
	MapChunk::m_ptrMidPipe->release_staticVB( m_iVerBufId );
	m_iVerBufId = -1;
	*/
}

//=================================================================================
bool FCMapChunk::UpdateTessellation()
{
	if (m_ForceBufferCreate){
		CreateTessellation();
	}
	return false;
}
//=================================================================================
bool FCMapChunk::UpdateTessellation2()
{
	//if( m_iVerBufId < 0 )
	FillBuffers();
	return false;
}
//=================================================================================
bool FCMapChunk::UpdateTessellation3()
{
	return false;
}
//=================================================================================
bool FCMapChunk::FillBuffers()
{
	if (!m_ForceBufferCreate)
		return false;

	// ���ڵĶ��㻺����������
	BOOL   t_bFirstCreate;
	if( m_iVerBufId < 0 )
	{
		m_iVerBufId = MapChunk::m_ptrMidPipe->create_dynamicVB(
			MapChunk::ChunkVertex::m_dwFvf,sizeof( MapChunk::ChunkVertex )*m_NumNewVertices,true );
		m_NumVertices = m_NumNewVertices;
		t_bFirstCreate = TRUE;
	}
	else
	{
		t_bFirstCreate = FALSE;
	}

	os_vbufFillData   t_vf;
	t_vf.m_iVerNum = m_NumNewVertices;
	t_vf.m_ptrDstart = (char*)m_Vertices;


	MapChunk::m_ptrMidPipe->fill_dnamicVB(
		m_iVerBufId,m_Vertices,sizeof(ChunkVertex), m_NumNewVertices );


	if( m_iSIdxBufId == -1 )
	{
		m_iIdxBufId = MapChunk::m_ptrMidPipe->create_staticIB( 
			m_Indices,m_NumNewIndices );
		m_iSIdxBufId = m_iIdxBufId;
	}
	else
	{
		m_iIdxBufId = m_iSIdxBufId;
	}

	extern bool gUseTerrainPS;
	if (!gUseTerrainPS)
	{
		for( int t_i=0;t_i<(int)m_vecPassMeshData.size();t_i ++ )
		{   
			m_vecPassMeshData[t_i].m_iIndexBufId = m_iIdxBufId;
			m_vecPassMeshData[t_i].m_iVertexBufId = m_iVerBufId;

			m_vecPassMeshData[t_i].m_dwPriType = TRIANGLESTRIP;

			m_vecPassMeshData[t_i].m_iPrimitiveNum = m_NumNewIndices - 2;
			m_vecPassMeshData[t_i].m_iVertexNum = m_NumNewVertices;

			m_vecPassMeshData[t_i].m_rectangle.x = m_xbase;
			m_vecPassMeshData[t_i].m_rectangle.y = m_zbase;
			m_vecPassMeshData[t_i].m_rectangle.z = m_xbase + MapTile::CHUNK_SIZE;
			m_vecPassMeshData[t_i].m_rectangle.w = m_zbase + MapTile::CHUNK_SIZE;
		}
		//! ver 16 add ��Ʒ��Ӱ��Ϣ
		m_LightMapMestData.m_iIndexBufId = m_iIdxBufId;
		m_LightMapMestData.m_iVertexBufId = m_iVerBufId;
		m_LightMapMestData.m_dwPriType = TRIANGLESTRIP;
		m_LightMapMestData.m_iPrimitiveNum = m_NumNewIndices - 2;
		m_LightMapMestData.m_iVertexNum = m_NumNewVertices;
		m_LightMapMestData.m_rectangle.x = m_xbase;
		m_LightMapMestData.m_rectangle.y = m_zbase;
		m_LightMapMestData.m_rectangle.z = m_xbase + MapTile::CHUNK_SIZE;
		m_LightMapMestData.m_rectangle.w = m_zbase + MapTile::CHUNK_SIZE;
	}
	else
	{
		m_PSPassMeshData.m_iIndexBufId = m_iIdxBufId;
		m_PSPassMeshData.m_iVertexBufId = m_iVerBufId;
		m_PSPassMeshData.m_dwPriType = TRIANGLESTRIP;
		m_PSPassMeshData.m_iPrimitiveNum = m_NumNewIndices - 2;
		m_PSPassMeshData.m_iVertexNum = m_NumNewVertices;
		m_PSPassMeshData.m_rectangle.x = m_xbase;
		m_PSPassMeshData.m_rectangle.y = m_zbase;
		m_PSPassMeshData.m_rectangle.z = m_xbase + MapTile::CHUNK_SIZE;
		m_PSPassMeshData.m_rectangle.w = m_zbase + MapTile::CHUNK_SIZE;

	

		///@{
		m_DepthPassMeshData.m_iIndexBufId = m_iIdxBufId;
		m_DepthPassMeshData.m_iVertexBufId = m_iVerBufId;
		m_DepthPassMeshData.m_dwPriType = TRIANGLESTRIP;
		m_DepthPassMeshData.m_iPrimitiveNum = m_NumNewIndices - 2;
		m_DepthPassMeshData.m_iVertexNum = m_NumNewVertices;
		m_DepthPassMeshData.m_rectangle.x = m_xbase;
		m_DepthPassMeshData.m_rectangle.y = m_zbase;
		m_DepthPassMeshData.m_rectangle.z = m_xbase + MapTile::CHUNK_SIZE;
		m_DepthPassMeshData.m_rectangle.w = m_zbase + MapTile::CHUNK_SIZE;
		////@}

		
	}




	m_ForceBufferCreate = false;

	return true;
}
//=================================================================================

void FCMapChunk::CreateTessellation()
{
	//!�����ڴ涥������
	assert(m_IndexMap);
	assert(m_Vertices);
	assert(m_Indices);
	memset(m_IndexMap, 0xff, MapTile::MAX_VERTICES*sizeof(unsigned short));
	m_NumNewVertices = m_NumNewIndices = 0;

	//!17X17 j��Y�᷽��i��x�᷽��
	for (int j = 0;j<MapTile::GRIDS;j++){
		AddIndex(GetIndex(0,j));
		AddLastIndexAgain();
		AddIndex(GetIndex(0,j+1));
		for (int i = 1 ; i<=MapTile::GRIDS;i++){
			AddIndex(GetIndex(i,j));
			AddIndex(GetIndex(i,j+1));
		}
		AddLastIndexAgain();
	} 

}

