
//================================================================================
//
//  以下的结构为存储新地图的文件格式
//
//================================================================================
// 
# pragma once
# include "../../interface/osInterface.h"


# define TGFILE_HEADER   "GEO"

struct  os_newFileHeader
{
	//! 文件头标识 TGFILE_HEADER
	char       m_szMagic[4];
	//! 暂时版本号是10.
	DWORD      m_dwVersion;

	//! TileGround在x和z方向上的索引
	WORD       m_wMapPosX;
	WORD       m_wMapPosZ;

	//! 高度信息的偏移
	DWORD      m_dwColHeiInfoOffset;

	//! 物品信息的大小和偏移。
	DWORD      m_dwObjNum;
	DWORD      m_dwObjDataOffset;

	//! 整个TileGround用到的纹理数目和数据偏移
	DWORD      m_dwTextureNum;
	DWORD      m_dwTexDataOffset;

	//! TileGround内的Chunk数据的偏移
	DWORD      m_dwChunkDataOffset;

	//! 渲染用高度信息的偏移
	DWORD      m_dwVerHeightOffset;

	//! 细节贴图层Alpha贴图的大小
	DWORD      m_dwAlphaSize;

	//! TG内每一个Chunk的最高点和最低点数据偏移
	DWORD      m_dwUtmostChunkHeiOffset;

	//! TG内quadTree数据的offset
	DWORD      m_dwQuadTreeDataOffset;

	//! TG内每一个格子属性数据的偏移
	DWORD      m_dwTileAttributeOffset;

	// 顶点法向量
	DWORD      m_dwVertNormalOffset;   // 数据块大小按默认大小

	// 顶点色
	DWORD      m_dwVertColorOffset;

	DWORD      m_dwHeightInfoOffset;

	// 最上层纹理idx偏移，以碰撞格子为最小单位
	DWORD      m_dwUpmostTexIdxOffset;

	//! 保留数据，用于未来扩充文件格式
	DWORD      m_dwWaterInfoOffset;

	DWORD      m_dwLightMapSize;

	//! 用于17版本中，hardtexture size offset的数据，即使早版本的读取这个数据，也不会
	//  出错.
	DWORD      m_dwHardTexSizeOffset;

};

//! 地表用到的纹理数据
struct  os_chunkTexture
{
 	short     m_wRepeatTimesU;
 	short     m_wRepeatTimesV;

	char       m_szTexName[12];
};

//! 最多用到的贴图层数
# define MAX_TEXLAYER 4

//! 一个TileGround内chunk的数目
# define CHUNKNUM_PERTG   16

//! 定义Alpha贴图的大小
# define ALPHAMAP_SIZE    32

//! 地表的Chunk数据
struct  os_chunkData
{
	//! 此Chunk内总的纹理层数
	int        m_dwTexNum;
	//! 每一个纹理层用到纹理数据索引
	int        m_dwTexPtr[MAX_TEXLAYER];
	//! 贴图层用到的alpha数据的偏移
	int        m_dwAlphaDataOffset;
	//! VER 16 ADD 光照图数据
	int        m_dwLightMapDataOffset;
};


/** \brief
 *  地图上物体的信息，此处用到的信息为渲染用信息。
 */
struct  os_newTGObjectOld13
{
  //!　物体的名字
  char             m_szName[32];

  //! 物体的放置角度
  float            m_fAngle;

  //!  物体在整个世界空间中的位置。
  osVec3D          m_vec3ObjPos;

  /** \brief
   *   物体属性,根据不同的物体属性，做出不同的处理。
   * 
   *   目前的划分为：
   *    0: 普通的物体。
   *    1: 动画类物体。skinMesh类的动画
   *    2: 水相关的物体。 Osa动画
   *    3: billBoard相关的物品。 
   *    4: 特效动画。就是我们自己格式的关键帧动画。Osa动画放置到场景中.
   *    5: 粒子动画的特效类型。场景中可以放置固定的粒子动画。
   *    6: 特效的合集。使用客户端的spe文件。River @ 2006-2-14日加入
   */
  DWORD	          m_dwType;

  /** \brief
   *  River @ 2006-6-21:加入地表颜色影响的物品全局色。
   *  
   *  如果地表色不影响物品的光照颜色，则此值为零。
   */
  DWORD           m_dwObjTerrColor;
};

/** \brief
 *  地图上物体的信息，此处用到的信息为渲染用信息。
 */
struct  os_newTGObject
{
  //!　物体的名字
  char             m_szName[32];

  //! 物体的放置角度
  float            m_fAngle;

  //!  物体在整个世界空间中的位置。
  osVec3D          m_vec3ObjPos;

  /** \brief
   *   物体属性,根据不同的物体属性，做出不同的处理。
   * 
   *   目前的划分为：
   *    0: 普通的物体。
   *    1: 动画类物体。skinMesh类的动画
   *    2: 水相关的物体。 Osa动画
   *    3: billBoard相关的物品。 
   *    4: 特效动画。就是我们自己格式的关键帧动画。Osa动画放置到场景中.
   *    5: 粒子动画的特效类型。场景中可以放置固定的粒子动画。
   *    6: 特效的合集。使用客户端的spe文件。River @ 2006-2-14日加入
   *	8: 雾类
   *	9: 点光源
   */
  DWORD	          m_dwType;

  /** \brief
   *  River @ 2006-6-21:加入地表颜色影响的物品全局色。
   *  
   *  如果地表色不影响物品的光照颜色，则此值为零。
   */
  DWORD           m_dwObjTerrColor;

  //!  River @ 2009-4-16:加入物品的缩放信息
  osVec3D          m_vec3ObjectScale;

};


//! Chunk内的Alpha和LightMap数据
//struct  os_alphaALmpData
//{
//	//! 最大可能的alpha数目,存储的时候，根据Chunk内Tex的数目，
//	//! 存储不同的数据大小
//	BYTE         m_vecAlpha[MAX_TEXLAYER-1][ALPHAMAP_SIZE*ALPHAMAP_SIZE];
//
//	//! lightmap的象素,对应ALPHAMAP_SIZE*ALPHAMAP_SIZE个RGB的Byte值
//	//BYTE         m_vecLmpPixel[ALPHAMAP_SIZE*ALPHAMAP_SIZE*3];
//
//public:
//	os_alphaALmpData()
//	{
//		for(int t_i=0;t_i<MAX_TEXLAYER-1;t_i ++ )
//			memset( m_vecAlpha[t_i],0,sizeof( BYTE )*ALPHAMAP_SIZE*ALPHAMAP_SIZE );
//	//	memset( m_vecLmpPixel,0,sizeof( ALPHAMAP_SIZE*ALPHAMAP_SIZE*3 ) );
//	}
//
//};



/** \brief
 *  四叉树相关的数据结构.
 *
 *  这两个数据结构仅用于表示文件内的数据格式,四叉树的数据结构由场景内的四叉树
 *  管理结构来调入
 */
struct os_quadNodeDataT
{
	//! 加入了物品后，此结点的最高高度
	float    m_fMaxHeight;
	//! 此结点的物品数据
	WORD     m_wNodeObjNum;
	//! 此结点的物品开始索引
	WORD     m_dwObjStartIdx;
};
struct os_quadTreeDataT
{
	//! 跨地图的物品的数目
	WORD     m_wExceedObjNum;
	//! 跨地图的物品的索引
	VEC_word m_vecExceedObj;
	//! 加入了跨地图的物品后，当前TG的bbox
	osVec3D  m_vecMax;
	osVec3D  m_vecMin;

	//! 整个四叉树内物品的数据索引数目
	WORD     m_wTGObjIdxNum;
	VEC_word  m_vecObjIdx;

	//! 四叉树结点的数目，用于确认
	DWORD     m_dwNodeNum;
	std::vector<os_quadNodeDataT>  m_vecNodeData;

};

//! 每一个数据Chunk对应的最高顶点高度和最低顶点高度
struct os_chunkUtmostHeight
{
	float      m_fMaxHeight;
	float      m_fMinHeight;
};

//! 存储文件数据
struct  os_tgFileData
{
	//! 碰撞和高度信息.
	os_TileCollisionInfo   m_vecColInfo[128*128];

	//! 每一个格子的属性数据
	BYTE                  m_vecAttribute[64*64];

	//! 物品的数据块，在文件头中有物品的数目
	os_newTGObject*        m_vecObj;

	//! 整个TileGround用到的ChunkTexture数据,数目在文件头内
	os_chunkTexture*       m_vecChunkTex;

	//! TileGround内的Chunk数据
	os_chunkData           m_dwChunkData[CHUNKNUM_PERTG];

	//! 渲染到用到的高度信息数据
	float                 m_vecVerHeight[65*65];

	//! 每一个TG内的Chunk极限高度信息
	os_chunkUtmostHeight   m_vecUtmostHeight[CHUNKNUM_PERTG];

	//! Alpha数据和光照图数据的数据块，以Chunk为单位进行组织
	//os_alphaALmpData       m_vecAlphaLmpData[CHUNKNUM_PERTG];

	//! 四叉树数据相关的数据，应该由场景的四叉树结构直接调入，此处仅用于示意
	os_quadTreeDataT       m_sQuadTree;

	os_tgFileData(){ m_vecObj = NULL;m_vecChunkTex = NULL; }
	~os_tgFileData()
	{
		SAFE_DELETE_ARRAY( m_vecObj );
		SAFE_DELETE_ARRAY( m_vecChunkTex );
	}
	
};

