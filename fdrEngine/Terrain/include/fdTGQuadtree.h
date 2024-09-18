//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTGQuadtree.h
 *
 *  His:      River created @ 2005-11-16
 *
 *  Des:      每一个TG对应的四叉树类和数据结构,sceneGraph由这些数据结构构成.
 *   
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "fdQuadtreeTemplate.h"

# include "../geoMipTerrain/MapTile.h"

class  osc_newTileGround;
struct os_ViewFrustum;
struct os_chunkUtmostHeight;
struct os_dlInfo;


/** \brief
 *  使用这个数据结构，预处理TG内的物品可见性信息
 *
 *  对于跨64*64地图的物品，会存储这样的物品列表，使正确的计算物品的可见性信息
 */
struct fd_sceneObjBbData
{
	//! obj的bbox.
	os_bbox    m_sObjBox;

	//! obj在整个场景的索引
	WORD      m_wObjIdx;

	//! 构建.
	fd_sceneObjBbData()
	{
		m_sObjBox.reset_bbox();
		m_wObjIdx = -1;
	}
};
typedef std::vector<fd_sceneObjBbData> VEC_objBbData;


//! 每一个quadTree结点对应的数据结构
class fd_quadTreeNodeData
{
public:
	//! 当前结点的aabbox,不加入物品的高度
	os_aabbox       m_sNodeBox;

	/** \brief
	 *  加入物品后的高度.
	 *
	 *  一个QuadtreeNode有两个最高高度数据,一个是纯粹地表的最高高度数据,
	 *  一个是加入了物品之后的最高高度数据.这样可能会出现我们需要对一个
	 *  Node内的物品进行渲染,但不需要对这个chunk的地表进行渲染的情况,优化
	 *  渲染流程.
	 */
	float           m_fMaxHeight;

	//! 如果是最终的叶结点，则此索引为这对应在整个mapBlock内的Chunk索引
	WORD            m_wRenderChunkIdx;

	//! 当前结点对应的可渲染物品Id的起始索引和数目。
	WORD            m_wObjStartIdx;
	WORD            m_wObjNum;

	fd_quadTreeNodeData()
	{
		memset( &m_sNodeBox,0,sizeof( os_aabbox ) );
		m_fMaxHeight = 0.0f;
		m_wRenderChunkIdx = 0xffff;
		m_wObjStartIdx = 0xffff;
		m_wObjNum = 0xffff;
	}

};




typedef os_qtNodeTemplate<fd_quadTreeNodeData>*  qtNode_ptr;


//! 每一个TG对应的四叉树数据结构
class osc_tgQuadtree : public os_quadtreeTemplate<fd_quadTreeNodeData>
{
private:
	//! 当前quadtree对应的TG指针
	osc_newTileGround*      m_ptrTGPtr;

	//! 全局的vf指针.
	static os_ViewFrustum*  m_ptrVf;


	//! 整个四叉树用到的物品数据索引
	VEC_word               m_vecObjIdx;


	/** \brief
	 *  每一个64*64的地图如果地图内的物品超过了地图的bbox,根据超出的物品的bbox,
	 *  记算新的地图bbox.
	 *  
	 *  再记录一个超出地图bbox的物品列表，跟vf比较，用于在不渲染的地形的条件下，
	 *  渲染这些物品。
	 */
	os_aabbox               m_sTotalBox;

	//! ATTENTION TO OPP:最终的数据，可以在四叉树内只存储os_bbox的指针，节省内存　
	VEC_objBbData           m_vecExceedObjIdx;

	//! 用于鼠标点选时，找出可能和射线相交的Chunk列表
	int                    m_iPickChunkNum;
	os_tileIdx              m_arrPickChunk[MapTile::CHUNKS*MapTile::CHUNKS];
	osVec2D                 m_arrChunkXZ[MapTile::CHUNKS*MapTile::CHUNKS];
	osVec3D                 m_vec3RayStart,m_vec3RayDir;



	//! River @ 2006-7-13:优化相机跟地表间的碰撞检测用..是否是使用线段做相交测试.
	bool                    m_bPickWithLs;
	osVec3D                 m_vec3LsEnd;

	//! 动态光相关的数据
	int                     m_iAffectDlNum;
	os_bsphere              m_vecDlBSphere[MAX_LIGHTNUM];
	osColor                 m_vecLightColor[MAX_LIGHTNUM];
	int                     m_iDlIdx[MAX_LIGHTNUM];

private:

	//! 为第一个结点分配BBox.
	void                 allot_aabbox( qtNode_ptr _nodePtr,
		                  const os_chunkUtmostHeight* _ptrHei,os_aabbox& _box );

# ifdef _DEBUG
	//! 渲染每一个结点的aabbox.
	void                 draw_nodeAabbox( 
		                   qtNode_ptr _nodePtr,bool _solid = false );
# endif 

	/** \brief
	 *  从一个顶点查找此结点当前所在的Quadtree leaf node.
	 *
	 *  \param _useY  _pt的Y值是否参与测试，如果不参与，则只要x,z值在结点内，此顶点在结点内
	 */ 
	qtNode_ptr            find_node( qtNode_ptr _node,osVec3D& _pt,bool _useY = true );

	//! 从一个结点开始得到当前结点内在vf内的地图格子索引.
	void                  get_inVfTile( qtNode_ptr _node,osMatrix& _reflMat );

	//! 为每一个结点分配Chunk索引
	void                 allot_chunkIdx( void );

	//! Check&Add quadtree Obj显示数据
	void                 check_nodeObjDis( qtNode_ptr _node,osMatrix& _reflMat );


	//! 递归得到所有和射线相交的Chunk列表
	void                 get_chunkList( qtNode_ptr _node );

	//! 根据动态光的颜色和位置范围，来计算当前node的动态光颜色
	void                 cal_nodeObjDLight( qtNode_ptr _node );

	//! 递归检测每一个结点，是否和传入的sphere列表碰撞
	void                 col_withSphere( qtNode_ptr _node );

public:
	osc_tgQuadtree();
	~osc_tgQuadtree();

	/** \brief
	 *  从内存中调入四叉树数据
	 *
	 *  算法描述：
	 *  1: 编译最简单的四叉树数据，每一个四叉树结点的格子开始点和结束点
	 *  2: 调入四叉树的aabbox数据和obj数据。
	 *  3: 根据高度数据，处理每一个结点的aabbox数据
	 */
	BOOL                 load_quadtreeDataFromMem( 
		                    osc_newTileGround* _ptr,const BYTE* _dstart );

	//! 计算当前在vf内的结点数目
	void                 cal_visualTileSet( osMatrix& _reflMat );

	//! TEST CODE:测试渲染当前的quadTree. 
	void                 draw_quadTree( void );

	/** \brief
	 *  根据传入的位置，得到当前位置所在的quadTree Chunk内的物品索引列表
	 *
	 *  这个函数主要用于挡住相机物品的半透明化
	 *  \param _pos 传入的主角人物所在位置。
	 *  \param _objList 位置所在的Chunk的物品的索引列表
	 *  \param _objNum  传入队列所能容纳的最大物品数，返回所在的Chunk的物品索引数目
	 *  \return int     返回已经找到的位置数目,如果人物和相机都在此tg,则返回2,如果
	 *                  都不在，返回零。
	 */
	int         get_chunkObj( osVec3D& _pos,
		                   int* _objList,int& _objNum,int _maxObjNum );

	//! get_chunkObj函数的补充，用于得到超出chunk边界的物品列表
	int         get_exceedChunkObj( osVec3D& _spos,
		osVec3D& _epos,int* _objList,int& _objNum,int _maxObjNum );


	/** \brief
	 *  传入一个位置,得到此位置所在chunk内所有物品的列表
	 *
	 *  \param osVec3D& _pos  传入想要得到的物品列表的位置.
	 *  \param int* _objList  返回物品的id列表.
	 *  \param int _maxObjNum 返回物品id列表的最大空间.
	 *  \return int           返回的物品数量.
	 */
	int         get_posInChunkObj( osVec3D& _pos,int* _objList,int _maxObjNum );

	/** \brief
	 *  传入一个位置，得到此位置是否跟quadTree超出tileGround的包围盒相交，并得到
	 *  跟哪个物品相交。
	 *
	 */
	int         get_posExceedQuadObj( osVec3D& _pos,int* _objList,int _maxObjNum );

	
	/** \brief
	 *  鼠标点选时，四叉树处理出射线相交过的Chunk列表
	 *  
	 *  \param const os_tileIdx*&  上层传入的指针，用于得到可能相交的chunk列表
	 *                             如果可能相交的chunk为零，此值在函数内赋为NULL
	 *  \param const osVec2D*& _chunkXZStart 上层传入的指针，用于得到每一个相交Chunk
	 *                                       在x,z方向上的最小值,减少上层的计算
	 *  \param osVec3D* _lineSegEnd 如果不为空,则使用线段和当前的chunkList做相交测试.
	 */
	int                    get_pickChunkList( osVec3D& _rayStart,osVec3D& _rayDir,
		                      const os_tileIdx*& _chunkArr,const osVec2D*& _chunkXZStart,
							  osVec3D* _lineSegEnd = NULL );


	/** \brief
	 *  处理动态光影响哪一个地图内的Chunk.
	 *
	 *  根据每一个动态光的影响范围来确认当前的Chunk是否受动态光影响
	 *
	 *  \param _bsphere 传入要碰撞检测的bsphere数据，最多4个
	 */
	void                   set_dlAffectChunk( const os_dlInfo* _dlInfo );

	//! 传入的射线是否跟当前的QuadTree相交
	bool                   ray_intersect( osVec3D& _rayStart,osVec3D& _rayDir );

};






