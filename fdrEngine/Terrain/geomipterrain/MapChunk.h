	//===================================================================
/** \file  
* Filename :   MapChunk.h
* Desc     :   
* His      :   Windy create @2005-12-6 11:13:59
*/
//===================================================================
#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include "../../../common/com_include.h"
# include "../../mfpipe/include/osPolygon.h"
# include "../../effect/include/dlight.h"

class MapTile;
class MapChunk;

class osc_middlePipe;
class Frustum;
#define	MAX_SCREEN_ERROR	 0.010f

//! river @ 2009-9-15:预处理相关的chunk hard tile.
struct osc_chunkHardTile;
struct osc_hardTextureTile;


typedef std::vector<os_renderMeshData> VEC_rMeshData;


//! hardTile的设备相关数据
struct  chunk_hardTile
{
	//! 纹理的ID.
	int     m_iTexId;

	//! index buffer的id.
	int     m_iIdxBufId;

	//! uv坐标相关的数据，用矩阵表示？

};

/** 地形块的基类
@remarks
	该类,提供内存中顶点数据,由派生类根据不同的LOD级别添加顶点,
	顶点信息通过MapTile得到原始的高度信息,经过缩放和偏移成为真正的世界顶点信息,
	派生类必须实现Init,DeInit,Draw,Setup函数
	并在UpdateTessellation里完成顶点数据的内存添加.
	UpdateTessellation2,UpdateTessellation3 用做其它方面的更新,由派生类决定,如顶点的形变
@ps
	
*/
class MapChunk
{
	friend class MapTile;
public:
	struct ChunkVertex{
		osVec3D pos;

		osVec3D normal;

		DWORD	color;
		osVec2D tex;

		//! 用于中间管道创建动态的顶点缓冲区
		static DWORD m_dwFvf;
	};
	struct sPassInfo {
		sPassInfo(){
			m_TexIdx = -1;
			m_pAlpha = NULL;
			m_AlphaID = -1;
		}
		int				m_TexIdx;	//!在MapTile中的纹理索引
		unsigned char	*m_pAlpha;	//!由上层决定的ALPHA大小
		int				m_AlphaID;	//!纹理管理器的ID
	};

	//! River added @ 2007-1-25:加入细节纹理的控制
	static void  OneTimeInit( osc_middlePipe* _ptrPipe );

	//! 中间渲染管道的指针,用于使用现有的渲染底层
	static osc_middlePipe*   m_ptrMidPipe;


protected:
	ChunkVertex*	m_Vertices;

	unsigned short* m_Indices;
	unsigned short* m_IndexMap;
	int				m_NumVertices,m_NumIndices,
					m_NumNewVertices,m_NumNewIndices;

	MapTile			*m_mt;
	/** 相对整个地图的索引偏移,	*/
	int				m_px, m_py;
	/** 相对与整个地图的世界空间位置偏移	*/
	float			m_xbase,m_zbase;

	/** 标志该项CHUNK被init,分配了内存，但没顶点和索引没进行赋值
	@remarks
		在LOD实现的派生CHUNK里,当CHUNK因为等级改变,要改变顶点和索引数据时,也置它为ture.
		保证在FillBuffers里知道顶点索引数据已经改变
	*/
	bool			m_ForceBufferCreate;

	//! 中间管道相关的顶点和索引缓冲区ID
	int            m_iVerBufId;
	int            m_iIdxBufId;


public:
	VEC_rMeshData   m_vecPassMeshData;
	
	//! ver 16 add 物品阴影光照信息
	os_renderMeshData   m_LightMapMestData;
	os_renderMeshData   m_PSPassMeshData;
	
	///@{
	os_renderMeshData   m_DepthPassMeshData;
	///@}

	//@{ 
	//  用于硬接触地表渲染相关的数据
	int               m_iHardTileTypeNum;
	//  以下为硬接地表设备相关的渲染数据
	chunk_hardTile    m_vecHardTile[MAX_HARDTILE_PERCHUNK]; 
	os_renderMeshData m_sHardTile[MAX_HARDTILE_PERCHUNK];
	osMatrix          m_sHtTexMat[MAX_HARDTILE_PERCHUNK];
	//@}

public:
	//!标志该CHUNK是否可见
	bool			m_IsVisible;

	//!标志该CHUNK是否被创建。
	bool			m_IsAcitve;

	//! 标志当前的Chunk是否被动态光影响到
	bool            m_bAffectByDLight;

	MapChunk(MapTile * mt,int x,int y);
	virtual ~MapChunk(void);

	//!子类实现的函数
	virtual bool	Draw(size_t _ipass);

	virtual bool	UpdateTessellation() = 0;

	virtual bool	UpdateTessellation2(){ return false;}

	virtual bool	UpdateTessellation3(){ return false;}
	//!
	//!新文件加载块信息方法
	void			LoadChunk(const BYTE *fpBegin,DWORD offset);

	void			SetVisible(bool Vis);

	//  
	//! 得到设备相关的数据,用于使用现有的渲染底层来渲染Chunk数据
	const os_renderMeshData* get_passRdata( int _pass );

	const os_renderMeshData* get_pspassRdata( );
	const os_renderMeshData* get_depthpassRdata( );

	//! river @ 2009-9-17:加入硬接触的地表.
	const os_renderMeshData* get_hardTileRdata( int _idx );
	

	const os_renderMeshData* get_lightmappassRdata( );

	int getpassCnt(){return (int)m_PassList.size();}

	//! 上层设置此chunk 需要重新填充顶点缓冲区
	void  set_forceCreate( BOOL _fc = TRUE ){ m_ForceBufferCreate = _fc; }

	//! 构建hardTile需要的index数据
	int   construct_hardTileIdxBuf( osc_hardTextureTile* _ht );

	//! 当前Chunk处理对应的硬接触地表块
	bool  process_hardTile( osc_chunkHardTile* _hardTile );




	const BYTE* get_lightMapL8(void)const{return m_LightMapL8;}
protected:
	//! 子类实现的函数
	virtual void	Setup();
	virtual void	Init();
	virtual void	DeInit();
	void			DeInitUnVisible();

	float			GetHeight(int nX,int nY);
	unsigned short	GetIndex(int nX,int nY);
	void			GetVertex(int nX,int nY,osVec3D & pt);
	void			GetTexCoord(int nX,int nY,osVec2D & TCood);
	void			GetDiffuse(int nX,int nY, DWORD &Diffuse);
	void			AddIndex(unsigned short nIdx);
	void			AddLastIndexAgain();

	//! 得到每一层的纹理矩阵
	const osMatrix*  get_chunkTexMatrix( int _idx );

	//! 释放hardTile的设备相关数据
	void             release_hardTile( void );


private:
	void		InitAlphaLMData();
	void		SafeDeInitAlphaLMData();
protected:
	enum {
		MAX_ALPHALAYS = 3,
	};
	//!多层纹理信息
	std::vector<sPassInfo>	m_PassList;
	
	BYTE					*m_AlphaPool[MAX_ALPHALAYS];
	char                   *m_LightMap;   //DDS DXT5 FORMAT
	BYTE					*m_LightMapL8; // RGB 255 format
	//!阴影信息
	int				m_LightMapID;	//!LIGHTMAP在中间管道的ID
	DWORD			m_AlphaSize;	//!由上层传入的ALPHA大小
	int             m_CombineAlphaID;  //!
	bool			m_bIsAlphaInit;
	
};
typedef std::vector<MapChunk *>	MapChunkPtrList;


