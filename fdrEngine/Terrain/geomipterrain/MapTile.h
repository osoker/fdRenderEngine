//===================================================================
/** \file  
* Filename :   MapTile.h
* Desc     :   
* His      :   Windy create @2005-12-5 14:10:02
*/
//===================================================================
#pragma once
#include <vector>
#include <string>
#include <d3d9.h>
#include "MapChunk.h"
#include "../../../common/com_include.h"
# include "../../effect/include/dlight.h"
#include "vertexTemplate/VertexTemplatesAll.h"
#include "../include/fdTerrainTriangleSelector.h"

class MapTilePVS;
class osc_mapTerrain;


//! river added @ 2009-9-15:硬接触的地表纹理
struct  osc_chunkHardTile;

/** 无LOD方式顶点格式*/
typedef	Vertex::Composer< Vertex::Position,
		Vertex::Composer< Vertex::TexCoords0 > >
	//	Vertex::Composer< Vertex::TexCoords1 > > >
		FCVertex;
/** 软件方式顶点形变的顶点格式*/
typedef	Vertex::Composer< Vertex::Position,
		Vertex::Composer< Vertex::TexCoords0,
		Vertex::Composer< Vertex::TexCoords1 > > >
		SWVertex;
///TODO:硬件顶点形变顶点格式


/** 地图类,每个地图每个地图管理CHUNKS X CHUNKS个地形块.
@remarks
	具体负责地图文件的加载,地形块的视截体剔除和PVS剔除.
	以及地表多遍渲染的组织和优化,
@par
	PVS还有点问题,地表的多遍渲染没有优化,四叉树的建立以后要预处理.
@note
	该类只操作MapChunk类,具体请参考MapChunk
*/
class MapTile
{
	friend class	osc_mapTerrain;
	friend class	MapChunk;
public:
	enum RENDERMODE {
		EMT_BRUTEFORCE,	//!有LOD，没顶点形变,
		EMT_NOLOD,
		EMT_MORPH_SW,	//!	LOD及CPU计算顶点形变方式
		EMT_MORPH_HW	//!	LOD及GPU硬件计算顶点形变方式
	};
	enum {
		GRID_SIZE = 3,	//!XZ方向的缩放
		HEIGHT_SIZE = 1,//!Y方向的缩放
		GRIDS = 16,		//!每个CHUNK的格子数/行(最高级的情况)
		SUBDIV = 4,		//!LOD最高级 2^4 = 16  
		CHUNKS = 4,		//!一个地图块的多少个CHUNK/行
		CHUNK_SIZE = GRID_SIZE*GRIDS,
		TILE_SIZE = CHUNK_SIZE*CHUNKS
	};
	enum {
		TILE_PIXEL = 65,
		CHUNK_PIXEL = GRIDS+1
	};
	enum {
		MAX_VERTICES = CHUNK_PIXEL * CHUNK_PIXEL,
		MAX_INDICES = MAX_VERTICES*3
	};

	/** 渲染设备相关资源,根据不同的渲染模式采用不同的顶点声明*/
	static LPDIRECT3DVERTEXDECLARATION9	m_pDeclaration;


private:
	
	//!地表重复纹理的信息
	struct  PassInfo{
		PassInfo():RepTex(-1){};
		int	RepTex;
		std::string	texname;
		osVec2D		vRepeat;	//!重复次数
		D3DXMATRIX	mat;		//!D3D用到的重复矩阵
	};
	typedef std::vector<PassInfo>	PassList;

	//!点的顶点色
	struct PointDiffuse {
		PointDiffuse():iX(0),iY(0){}
		//int ilightId;
		int iX,
			iY;
		D3DCOLOR	diffuse;
	};
	typedef std::vector<PointDiffuse>	PointDiffuseVec;

	//!渲染用到的高度数据
	float			*m_HMData;
	osVec3D         *m_vecNormal;
	D3DCOLOR		*m_DiffuseData;
	D3DCOLOR		*m_OrigDiffuseData;
	D3DCOLOR		*m_AmbientDiffuseData;

	//!保存动态光修改的点的原始DIFFUSE,
	PointDiffuseVec m_LitDiffList[MAX_LIGHTNUM];
	PassList		m_Pass;	
	std::string		m_mapname;
	int				m_AlphaSize;
	static int             ms_LightMapSize;
	//!相对于世界地图中的坐标位置。
	int			m_xpos, m_zpos;
	//!相对于世界地图的偏移。是真实的距离。
	float		m_xbase, m_zbase;
	MapChunk	*m_chunks[CHUNKS][CHUNKS];
	//!地形渲染方式
	static RENDERMODE s_RenderMode;

	/** 释放设备相关数据,如D3D纹理*/
	void		SafeReleaseDeviceData();

	/** 根据  SetChunkVisible 设置的可见块，把这些块提交到上层管理器中，由上层统一处理*/
	void		PushActiveChunk();

	void		ClearChunks();

public:
	const float * GetRawHMData()const{return m_HMData; };
	void GetWorldOffset(float& xbase,float& zbase) const {xbase = m_xbase; zbase = m_zbase;};

	static void OneTimeDeinit();
	static void OneTimeInit( osc_middlePipe* _pipe );

	//! River added @ 2006-1-12:在mapTile之上的mapTerrain
	static osc_mapTerrain*  m_ptrMapTerrain;

	/** \brief 
	 * 更新地形,画整个地表的格子
	 *
	 * 引擎渲染地表的时候，只需要调用一次
	 *
	 * \param _envMap 是否渲染地形为环境贴图使用
	 */
	static void		DrawTerrain( BOOL _envMap = FALSE ,BOOL _depth = FALSE);

	//! 清空可见的地形块
	static void     ClearVisibleTerr( void );


	/** 加载地图片接口
	@par 
		_pFileBegin 地图数据的头指针
		_mapName	地图所在的名字如vilg,test
		_x0,_z0		地图片的相对的索引如 1,0 
	@remarks
	以下是加载一个地图片的代码
		MapTile *  pTile = new MapTile();
		BYTE*	t_fstart;
		BYTE*	fileBegin;
		int     t_iSize;
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_iSize = read_fileToBuf( name,t_fstart,TMP_BUFSIZE );

		pTile->LoadMapTile(t_fstart,mapname,x,z);
		END_USEGBUF( t_iGBufIdx );
		pTile->InitDeviceData();
	*/
	bool		LoadMapTile(const BYTE * _pFileBegin,const char* _szmapName);

	//! 处理当前maptile内每一个chunk内的硬接触地表纹理。
	bool        process_hardTex( osc_chunkHardTile* _hardTile );


	/** 初始化所有设备相关的资源,如纹理等
	@remarks 在加载完地图后调用*/
	bool		InitDeviceData();

	void		ClearDynamic_Light();
	void		ReCalcChunkDynamic_Light();
	void		SetambientLCoeff(float _r,float _g,float _b);
	//!设置动态光 ,成功返回 TRUE
	BOOL		SetDynamic_Light(osc_dlight* _dLight,int _lightID);
	
	/** 由上层根据 可见性设置 */
	void		SetChunkVisible(int _idxX,int _idxY,bool _IsbVisible);

	/** \brief
	 *  上层设置某一个Chunk是否受动态光的影响
	 */
	void        setChunkAffectByDl( int _idxX,int _idxY );


	/** 得到高度,一般由MapChunk类调用, */
	float		GetHeight(int nX,int nY);

	//! 得到一个顶点索引对应的Normal
	void        GetNormal( int _x,int _y,osVec3D& _normal );

	void		GetDiffuse(int nX, int nY, D3DCOLOR &_diffuse);

	void		GetDiffuseOrig(int nX, int nY, D3DXCOLOR &_diffuse);

	D3DCOLOR &	GetDiffuse(int nX, int nY);
	/** 计算纹理坐标 */
	void		GetTexCoord(int nX,int nY,osVec2D &vTCood);


	// River mod @ 2007-6-8:上层得纯地表的高度信息
	const float* get_heightInfoPtr( void ) { return m_HMData; } 

public:
	MapTile(void);
	~MapTile(void);
};
