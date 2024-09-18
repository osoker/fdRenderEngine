//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTGQuadtree.cpp
 *
 *  His:      River created @ 2005-11-17
 *
 *  Des:      每一个TG对应的四叉树类和数据结构,sceneGraph由这些数据结构构成.
 *   
 *  “物有所不足，智有所不明。Stay Hungry,Stay Foolish.” 
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdTGQuadtree.h"
# include "../include/fdNewFieldStruct.h"
# include "../../mfPipe/include/osCharacter.h"
# include "../geoMipTerrain/MapTile.h"
# include "../../effect/include/dlight.h"


//! 预处理地图时，默认地图每一个bbox的高度
# define DEFAULT_TGBOXHEIGHT  1.0f

//! River added @ 2008-12-15:是否使用水面倒影
OSENGINE_API BOOL                   g_bWaterReflect = true;

//! River added @ 2009-8-30: 是否渲染地形桥类物品倒影,如果此值为false,则不渲染场景内物品和地形有倒影。
//!                          此值默认为真。
OSENGINE_API BOOL                   g_bTerrWaterReflect = TRUE;


//! 全局的vf指针.
os_ViewFrustum* osc_tgQuadtree::m_ptrVf = NULL;


osc_tgQuadtree::osc_tgQuadtree()
{
	m_ptrTGPtr = NULL;
}

osc_tgQuadtree::~osc_tgQuadtree()
{

}

//! 为每一个结点分配Chunk索引
void osc_tgQuadtree::allot_chunkIdx( void )
{
	//! 第零个结点是根结点。

	//! 第1到4个结点

	//! 以下结点为叶结点
	m_arrNodeList[5].m_sNodeData.m_wRenderChunkIdx = 0;
	m_arrNodeList[6].m_sNodeData.m_wRenderChunkIdx = 1;
	m_arrNodeList[7].m_sNodeData.m_wRenderChunkIdx = 4;
	m_arrNodeList[8].m_sNodeData.m_wRenderChunkIdx = 5;

	m_arrNodeList[9].m_sNodeData.m_wRenderChunkIdx = 2;
	m_arrNodeList[10].m_sNodeData.m_wRenderChunkIdx = 3;
	m_arrNodeList[11].m_sNodeData.m_wRenderChunkIdx = 6;
	m_arrNodeList[12].m_sNodeData.m_wRenderChunkIdx = 7;

	m_arrNodeList[13].m_sNodeData.m_wRenderChunkIdx = 8;
	m_arrNodeList[14].m_sNodeData.m_wRenderChunkIdx = 9;
	m_arrNodeList[15].m_sNodeData.m_wRenderChunkIdx = 12;
	m_arrNodeList[16].m_sNodeData.m_wRenderChunkIdx = 13;

	m_arrNodeList[17].m_sNodeData.m_wRenderChunkIdx = 10;
	m_arrNodeList[18].m_sNodeData.m_wRenderChunkIdx = 11;
	m_arrNodeList[19].m_sNodeData.m_wRenderChunkIdx = 14;
	m_arrNodeList[20].m_sNodeData.m_wRenderChunkIdx = 15;

}



/*


四叉树四个结点对应地图的四个区域:

z方向
|
|     3    4
|
|     1    2
|
Origin----------x方向

即地图的左下角，为第零个的子结点，右下角区域为第一个子结点


*/

//! 为第一个结点分配BBox.
void osc_tgQuadtree::allot_aabbox( qtNode_ptr _nodePtr,
				const os_chunkUtmostHeight* _ptrHei,os_aabbox& _box )
{
	guard;

	osassert( _nodePtr );
	osassert( _ptrHei );

	os_aabbox    t_aabbox;

	_nodePtr->m_sNodeData.m_sNodeBox = _box;

	// 叶结点的情况下，得到此叶结点内最低的顶点高度最高的顶点高度
	if( _nodePtr->is_leafNode() )
	{
		_nodePtr->m_sNodeData.m_sNodeBox.m_vecMax.y = 
			_ptrHei[_nodePtr->m_sNodeData.m_wRenderChunkIdx].m_fMaxHeight;
		_nodePtr->m_sNodeData.m_sNodeBox.m_vecMin.y = 
			_ptrHei[_nodePtr->m_sNodeData.m_wRenderChunkIdx].m_fMinHeight;
	}
	else    // 非叶结点的情况下，递归调用
	{
		osVec3D      t_vec3Center = _box.get_center();
		t_vec3Center.y = _box.m_vecMax.y;
		qtNode_ptr*  t_arrNodePtr = _nodePtr->get_childPtrArray();

	
		// 左下角的结点
		t_aabbox.m_vecMin = _box.m_vecMin;
		t_aabbox.m_vecMax = t_vec3Center;
		allot_aabbox( t_arrNodePtr[0],_ptrHei,t_aabbox );

		// 右下角的结点
		t_aabbox.m_vecMin = _box.m_vecMin;
		t_aabbox.m_vecMin.x = t_vec3Center.x;
		t_aabbox.m_vecMax = _box.m_vecMax;
		t_aabbox.m_vecMax.z = t_vec3Center.z;
		allot_aabbox( t_arrNodePtr[1],_ptrHei,t_aabbox );

		// 左上角的结点
		t_aabbox.m_vecMin = _box.m_vecMin;
		t_aabbox.m_vecMin.z = t_vec3Center.z;
		t_aabbox.m_vecMax = _box.m_vecMax;
		t_aabbox.m_vecMax.x = t_vec3Center.x;
		allot_aabbox( t_arrNodePtr[2],_ptrHei,t_aabbox );

		// 右上角的结点
		t_aabbox.m_vecMin = t_vec3Center;
		t_aabbox.m_vecMin.y = _box.m_vecMin.y;
		t_aabbox.m_vecMax = _box.m_vecMax;
		allot_aabbox( t_arrNodePtr[3],_ptrHei,t_aabbox );

		// 根据子结点得到每一个非叶结点的BBox
		for( int t_i=0;t_i<4;t_i ++ )
		{
			if( _nodePtr->m_sNodeData.m_sNodeBox.m_vecMax.y < 
				t_arrNodePtr[t_i]->m_sNodeData.m_sNodeBox.m_vecMax.y )
			{
				_nodePtr->m_sNodeData.m_sNodeBox.m_vecMax.y = 
					t_arrNodePtr[t_i]->m_sNodeData.m_sNodeBox.m_vecMax.y;
			}

			if( _nodePtr->m_sNodeData.m_sNodeBox.m_vecMin.y > 
				t_arrNodePtr[t_i]->m_sNodeData.m_sNodeBox.m_vecMin.y )
			{
				_nodePtr->m_sNodeData.m_sNodeBox.m_vecMin.y = 
					t_arrNodePtr[t_i]->m_sNodeData.m_sNodeBox.m_vecMin.y;
			}
		}

	}

	return;

	unguard;
}






/** \brief
*  从内存中调入四叉树数据
*
*  算法描述：
*  1: 为四叉树叶结点指定chunk索引,编译最简单的四叉树数据,为每一个叶结点分配BBox
*  2: 调入四叉树的obj数据。
*  3: 根据高度数据，处理每一个结点的aabbox数据
*/
BOOL osc_tgQuadtree::load_quadtreeDataFromMem( 
	      osc_newTileGround* _ptr,const BYTE* _dstart )
{
	guard;

	osassert( _ptr );
	osassert( _dstart );
	const os_newFileHeader*      t_ptrHeader;
	const os_chunkUtmostHeight*   t_ptrHeight;


	m_ptrTGPtr = _ptr;
	t_ptrHeader = (const os_newFileHeader*)_dstart;
	t_ptrHeight = (const os_chunkUtmostHeight*)
		(_dstart + t_ptrHeader->m_dwUtmostChunkHeiOffset );

	allot_chunkIdx();

	// 1: 编译最简单的四叉树数据，每一个四叉树结点的格子开始点和结束点
	os_aabbox      t_aabbox;
	t_aabbox.m_vecMin.x = t_ptrHeader->m_wMapPosX*TILE_WIDTH*MAX_GROUNDWIDTH;
	t_aabbox.m_vecMax.x = t_aabbox.m_vecMin.x + TILE_WIDTH*MAX_GROUNDWIDTH;

	t_aabbox.m_vecMin.z = t_ptrHeader->m_wMapPosZ*TILE_WIDTH*MAX_GROUNDWIDTH;
	t_aabbox.m_vecMax.z = t_aabbox.m_vecMin.z + TILE_WIDTH*MAX_GROUNDWIDTH;

	t_aabbox.m_vecMin.y = -DEFAULT_TGBOXHEIGHT;
	t_aabbox.m_vecMax.y = DEFAULT_TGBOXHEIGHT;

	allot_aabbox( m_ptrRootNode,t_ptrHeight,t_aabbox );


	// 
	// 2: 调入四叉树的aabbox数据和obj数据。
	// 3: 根据高度数据，处理每一个结点的aabbox数据
	WORD          t_w;
	const BYTE*    t_fstart = (const BYTE*)
		(_dstart + t_ptrHeader->m_dwQuadTreeDataOffset );

	// 超出地图的物品索引和Total AABBox.
	READ_MEM_OFF( &t_w,t_fstart,sizeof( WORD ) );
	m_vecExceedObjIdx.resize( t_w );
	for( WORD t_i =0;t_i<t_w;t_i ++ )
	{
		READ_MEM_OFF( &m_vecExceedObjIdx[t_i].m_wObjIdx,
			t_fstart,sizeof( WORD ) );
	}
	READ_MEM_OFF( &m_sTotalBox.m_vecMax,t_fstart,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_sTotalBox.m_vecMin,t_fstart,sizeof( osVec3D ) );

	// 
	// 整个四叉树内的物品索引块
	READ_MEM_OFF( &t_w,t_fstart,sizeof( WORD ) );
	m_vecObjIdx.resize( t_w );
	
	if( t_w > 0 )
		READ_MEM_OFF( &m_vecObjIdx[0],t_fstart,sizeof( WORD )*t_w );

# ifdef _DEBUG 
	for( int t_i=0;t_i<(int)m_vecObjIdx.size();t_i ++ )
		osassert( m_vecObjIdx[t_i] < t_ptrHeader->m_dwObjNum );
# endif 

	// 
	fd_quadTreeNodeData*  t_ptrData;
	DWORD  t_dwNodeNum;
	READ_MEM_OFF( &t_dwNodeNum,t_fstart,sizeof( DWORD ) );
	osassert( t_dwNodeNum == m_iNodeNum );
	for( int t_i=0;t_i<m_iNodeNum;t_i ++ )
	{
		t_ptrData = &m_arrNodeList[t_i].m_sNodeData;
		
		READ_MEM_OFF( &t_ptrData->m_fMaxHeight,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &t_ptrData->m_wObjNum,t_fstart,sizeof( WORD ) );
		READ_MEM_OFF( &t_ptrData->m_wObjStartIdx,t_fstart,sizeof( WORD ) );
	}

	return TRUE;

	unguard;
}

# ifdef _DEBUG
//! 渲染每一个结点的aabbox.
void osc_tgQuadtree::draw_nodeAabbox( qtNode_ptr _nodePtr,bool _solid/* = false*/ )
{
	guard;

	osassert( _nodePtr );
	
	if( _nodePtr->is_leafNode() )
	{
		os_bbox   t_box;

		t_box.set_bbMinVec( _nodePtr->m_sNodeData.m_sNodeBox.m_vecMin );
		t_box.set_bbMaxVec( _nodePtr->m_sNodeData.m_sNodeBox.m_vecMax );

		I_fdScene*  t_ptrScene = ::get_sceneMgr();
		t_ptrScene->draw_bbox( &t_box,0xffff0000,_solid );

	}
	else
	{
		qtNode_ptr*   t_arrNode;
		t_arrNode = _nodePtr->get_childPtrArray();
		
		draw_nodeAabbox( t_arrNode[0] );
		draw_nodeAabbox( t_arrNode[1] );
		draw_nodeAabbox( t_arrNode[2] );
		draw_nodeAabbox( t_arrNode[3] );
	}

	unguard;
}
# endif 
/*


*/ 
//! 从一个顶点查找此结点当前所在的Quadtree leaf node.
qtNode_ptr osc_tgQuadtree::find_node( qtNode_ptr _node,osVec3D& _pt,bool _useY/* = true*/ )
{
	guard;

	bool     t_bInCurBox = false;
	osVec3D  t_vec3Pt = _pt;
 
	// 顶点是否在当前的BBox,加上物品的高度BBox
	os_aabbox t_box = _node->m_sNodeData.m_sNodeBox;
	t_box.m_vecMax.y = _node->m_sNodeData.m_fMaxHeight;
	if( !_useY )
		t_vec3Pt.y = osn_mathFunc::float_lerp( t_box.m_vecMin.y,t_box.m_vecMax.y,0.5f);
	t_bInCurBox = t_box.pt_inBox( t_vec3Pt );

	if( !t_bInCurBox )
		return NULL;

	if( _node->is_leafNode() )
	{
		return _node;
	}
	else
	{
		qtNode_ptr*   t_arrNode;
		t_arrNode = _node->get_childPtrArray();

		qtNode_ptr    t_nodePtr;
		// 依次测试四个子结点
		for( int t_i=0;t_i<4;t_i ++ )
		{
			t_nodePtr = find_node( t_arrNode[t_i],t_vec3Pt,_useY );
			if( t_nodePtr )
				return t_nodePtr;
		}
	}

	return NULL;

	unguard;
}


//! Check&Add quadtree Obj显示数据
void osc_tgQuadtree::check_nodeObjDis( qtNode_ptr _node,osMatrix& _reflMat )
{
	guard;

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox*           t_sAabbox = NULL;

	// 测试每一个物品的aaBBox是否在场景内
	for( int t_i=0;t_i<t_ptrData->m_wObjNum;t_i ++ )
	{
		if( 0 != m_ptrTGPtr->get_objType( 
			 m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] ) )
		{
			m_ptrTGPtr->push_objIdx(
				m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );
			continue;
		}

		t_sAabbox = m_ptrTGPtr->get_objAaabbox( 
			m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );

		if( m_ptrVf->objInVF( *t_sAabbox ) )
		{
			m_ptrTGPtr->push_objIdx(
				m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );
		}
		else
		{
			// 如果全局使用倒影，则处理桥类物品的倒影是否在vf内
			if( g_bWaterReflect && g_bTerrWaterReflect )
			{
				if( !m_ptrTGPtr->is_bridgeObj( 
					m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] ) )
					continue;

				os_aabbox   t_sBox;
				t_sAabbox->get_transNewAABBox( _reflMat,t_sBox );
				if( !m_ptrVf->objInVF( t_sBox ) )
					continue;

				// 
				// River @ 2008-12-16: 桥类物品的倒影出现在了vf内
				m_ptrTGPtr->push_objIdx(
					m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );

			}
		}

	}

	unguard;
}



//! 从一个结点开始得到当前结点内在vf内的地图格子索引.
void osc_tgQuadtree::get_inVfTile( qtNode_ptr _node,osMatrix& _reflMat )
{
	guard;

	osassert( m_ptrVf );

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox             t_box = t_ptrData->m_sNodeBox;
	bool                  t_bReflectBoxInView = false;


	// 使用最大的bbox做检测
	float   t_fChunkHei = t_box.m_vecMax.y;
	t_box.m_vecMax.y = _node->m_sNodeData.m_fMaxHeight;
	if( !m_ptrVf->objInVF( t_box ) )
	{
		// 水面倒影需要计算更多的可视terr
		if( g_bWaterReflect && g_bTerrWaterReflect )
		{
			os_aabbox   t_sReflBox;
			t_box.get_transNewAABBox( _reflMat,t_sReflBox );

			if( !m_ptrVf->objInVF( t_sReflBox ) )
				return;
			else
				t_bReflectBoxInView = true;
		}
		else
			return;
	}




	//! windy mod 12-26
	// 当前结点的bbox全部在vf内或是当前的结点在叶结点.
	if( _node->is_leafNode() )
	{
		osassert( _node->m_sNodeData.m_wRenderChunkIdx != 0xffff );
		t_box.m_vecMax.y = t_fChunkHei;

		// River added @ 2008-12-15:如果仅仅是reflect in view.
		if( t_bReflectBoxInView )
		{
			m_ptrTGPtr->set_chunkDraw( _node->m_sNodeData.m_wRenderChunkIdx );
		}
		// 一:
		// 如果地表的aabbox全部在视域内,渲染当前结点内的物品,
		// 不需要再做判断bbox数据，直接推入.地表推入渲染.
		else if( m_ptrVf->objCompleteInVF( t_box ) )
		{
			for( int t_i=0;t_i<t_ptrData->m_wObjNum;t_i ++ )
			{
				m_ptrTGPtr->push_objIdx(
					m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );
			}

			m_ptrTGPtr->set_chunkDraw( _node->m_sNodeData.m_wRenderChunkIdx );
		}
		else
		{

			// 二:
			// 如果地表的aabbox部分在视域内,对每一个obj的bbox跟vf做检测后推入
			// 地表推入渲染.
			if( m_ptrVf->objInVF( t_box ) )
			{
				check_nodeObjDis( _node,_reflMat );
				m_ptrTGPtr->set_chunkDraw( _node->m_sNodeData.m_wRenderChunkIdx );

			}
			else
			{
				// 三:
				// 如果地表的aabbox不在视域内,而物品的bbox在视域内.对每一个
				// obj的bbox跟vf做检测后推入.地表不需要再推入渲染.
				check_nodeObjDis( _node,_reflMat );
			}
		}
	}
	else
	{
		// 
		// 非叶结点,aabbox部分在vf内的情况,需要接下来的弟归处理
		qtNode_ptr*   t_arrNode;
		t_arrNode = _node->get_childPtrArray();

		get_inVfTile( t_arrNode[0],_reflMat );
		get_inVfTile( t_arrNode[1],_reflMat );
		get_inVfTile( t_arrNode[2],_reflMat );
		get_inVfTile( t_arrNode[3],_reflMat );

	}

	// 
	// River @ 2006-6-17: 对超出地图边界的物品进行处理.如果超出边界的物品没
	// 有覆盖某个可见的结点,而此物品又在视域内,就出现可见性的问题
	for( int t_i=0;t_i<(int)m_vecExceedObjIdx.size();t_i ++ )
	{
		int t_idx = m_vecExceedObjIdx[t_i].m_wObjIdx;

		// 如果已经可见,不需要处理
		if( m_ptrTGPtr->is_objInView( t_idx ) )
			continue;
		if( 0 != m_ptrTGPtr->get_objType( t_idx ) )
		{
			m_ptrTGPtr->push_objIdx( t_idx );
			continue;
		}
		os_bbox     t_bbox;
		m_ptrTGPtr->get_objBBox( t_idx,t_bbox );
		if( m_ptrVf->objInVF( t_bbox ) )
			m_ptrTGPtr->push_objIdx( t_idx );
	}


	unguard;
}

//! 递归得到所有和射线相交的Chunk列表
void osc_tgQuadtree::get_chunkList( qtNode_ptr _node )
{
	guard;

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox*   t_ptrBox = &t_ptrData->m_sNodeBox;

	// 先做Ray检测
	float  t_fInterDis;
	

	// River @ 2006-7-13: 如果是线段相交
	if( m_bPickWithLs )
	{
		if( !t_ptrBox->ls_intersect( m_vec3RayStart,m_vec3LsEnd ) )
			return;
	}
	else
	{
		if( !t_ptrBox->ray_intersect( m_vec3RayStart,m_vec3RayDir,t_fInterDis ) )
		{
			return;
		}

		// 去除负方向上的BBox.
		if( ( t_fInterDis < 0.0f )&&
			( !t_ptrBox->pt_inBox( m_vec3RayStart ) ) )
		{
			return;
		}
	}

	// 当前结点的bbox全部在vf内或是当前的结点在叶结点.
	if( _node->is_leafNode() )
	{
		osassert( _node->m_sNodeData.m_wRenderChunkIdx != 0xffff );

		m_arrPickChunk[m_iPickChunkNum].m_iY = 
			_node->m_sNodeData.m_wRenderChunkIdx / MapTile::CHUNKS;
		m_arrPickChunk[m_iPickChunkNum].m_iX = 
			_node->m_sNodeData.m_wRenderChunkIdx % MapTile::CHUNKS;

		m_arrChunkXZ[m_iPickChunkNum].x = 
			_node->m_sNodeData.m_sNodeBox.m_vecMin.x;
		m_arrChunkXZ[m_iPickChunkNum].y = 
			_node->m_sNodeData.m_sNodeBox.m_vecMin.z;

		m_iPickChunkNum ++;
	}
	else
	{
		// 
		// 非叶结点,弟归处理
		qtNode_ptr*   t_arrNode;
		t_arrNode = _node->get_childPtrArray();

		get_chunkList( t_arrNode[0] );
		get_chunkList( t_arrNode[1] );
		get_chunkList( t_arrNode[2] );
		get_chunkList( t_arrNode[3] );
	}

	return;

	unguard;
}



//! 计算当前在vf内的结点数目
void osc_tgQuadtree::cal_visualTileSet( osMatrix& _reflMat )
{
	guard;

	// 
	if( !m_ptrVf&&g_ptrCamera)
		m_ptrVf = g_ptrCamera->get_frustum();
	if (!m_ptrVf)  return;

	
	// 
	// 先处理超出地图边界的物品,这些物品必须预处理到四叉树树中存储
	if(  m_ptrVf->objInVF( m_sTotalBox ) &&
		(!m_ptrVf->objInVFAccurate( m_ptrRootNode->m_sNodeData.m_sNodeBox ) ) )
	{
		for( int t_i=0;t_i<(int)m_vecExceedObjIdx.size();t_i ++ )
		{
			int t_idx = m_vecExceedObjIdx[t_i].m_wObjIdx;
			if( 0 != m_ptrTGPtr->get_objType( t_idx ) )
			{
				m_ptrTGPtr->push_objIdx( t_idx );
				continue;
			}

			os_bbox     t_bbox;

			m_ptrTGPtr->get_objBBox( t_idx,t_bbox );
			if( m_ptrVf->objInVF( t_bbox ) )
				m_ptrTGPtr->push_objIdx( t_idx );

		}
	}

	// 正常处理流程
	get_inVfTile( m_ptrRootNode,_reflMat );


	unguard;
}

//! TEST CODE: 测试渲染当前的quadTree.
void osc_tgQuadtree::draw_quadTree( void )
{
	guard;

	if( !m_ptrTGPtr )
		return;

# ifdef _DEBUG
	draw_nodeAabbox( m_ptrRootNode,false );
# endif 
	unguard;
}

//! get_chunkObj函数的补充，用于得到超出chunk边界的物品列表
int osc_tgQuadtree::get_exceedChunkObj( osVec3D& _spos,osVec3D& _epos,
									   int* _objList,int& _objNum,int _maxObjNum )
{
	guard;

	osVec3D   t_vec3Dir = _epos - _spos;
	float     t_fLength = osVec3Length( &t_vec3Dir );
	float     t_fTmp;
	
	osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

	// 有可能相交，进一步的处理
	if( m_sTotalBox.ray_intersect( _spos,t_vec3Dir,t_fTmp ) ||
		m_sTotalBox.pt_inBox( _spos ) )
	{
		os_bbox   t_bbox;
		int       t_iObjDevIdx;
		// 跟超出TileGround范围的物品一个个做碰撞检测。
		for( int t_i=0;t_i<(int)m_vecExceedObjIdx.size();t_i ++ )
		{
			t_iObjDevIdx = m_vecExceedObjIdx[t_i].m_wObjIdx;
			m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );

			if( t_bbox.ray_intersect( _spos,t_vec3Dir,t_fLength ) ||
				t_bbox.pt_inBBox( _spos ) )
			{
				_objList[_objNum] = t_iObjDevIdx;
				_objNum ++;
				osassert( _objNum < _maxObjNum );
			}
		}
	}

	return 0;

	unguard;
}



/** \brief
*  根据传入的位置，得到当前位置所在的quadTree Chunk内的物品索引列表
*
*  这个函数主要用于挡住相机物品的半透明化
*  \param _pos 传入主角人物所在的位置。
*  \param _objList 位置所在的Chunk的物品的索引列表
*  \param _objNum  传入队列所能容纳的最大物品数，返回位置所在的Chunk的物品索引的数目。
*  \return int     返回已经找到的位置数目,如果人物和相机都在此tg,则返回2,如果
*                  都不在，返回零。
*/
int osc_tgQuadtree::get_chunkObj( osVec3D& _pos,
								   int* _objList,int& _objNum,int _maxObjNum )
{
	guard;

	osassert( _pos.x >= 0.0f );
	osassert( _pos.z >= 0.0f );
	osassert( _objList );
	osassert( m_ptrTGPtr );

	qtNode_ptr   t_ptrNodeChar = NULL,t_ptrNodeCam = NULL;

	osVec3D            t_vec3CamDir,t_vec3CamPos;
	float              t_fDis;

	//! 得到当前人物和相机相关的参数。
	g_ptrCamera->get_curpos( &t_vec3CamPos );

	t_ptrNodeChar = find_node( m_ptrRootNode,_pos,false );
	t_ptrNodeCam = find_node( m_ptrRootNode,t_vec3CamPos,false );
	if( (!t_ptrNodeChar)&&(!t_ptrNodeCam) ) 
	{
		// river @ 2009-2-8: 是否跟超出chunk边界的物品相交，处理此列表
		get_exceedChunkObj( _pos,t_vec3CamPos,_objList,_objNum,_maxObjNum );
		return 0;
	}

	os_bbox     t_bbox;
	int         t_iObjDevIdx;


	t_vec3CamDir = _pos - t_vec3CamPos;
	t_fDis = osVec3Length( &t_vec3CamDir );
	osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );


	// 人物所有的Chunk和相机所在的Chunk都需要处理一下
	if( t_ptrNodeChar )
	{
		for( int t_i=0;t_i<t_ptrNodeChar->m_sNodeData.m_wObjNum;t_i ++ )
		{
			if( _objNum >= _maxObjNum )
				break;
	
			t_iObjDevIdx = m_vecObjIdx[t_ptrNodeChar->m_sNodeData.m_wObjStartIdx+t_i];
			m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );

			if( t_bbox.ray_intersect( _pos,t_vec3CamDir,t_fDis ) ||
				t_bbox.pt_inBBox( t_vec3CamPos ) )
			{
				// 往队列内加入物品，直到队列满
				_objList[_objNum] = t_iObjDevIdx;
				_objNum ++;
				osassert( _objNum < _maxObjNum );
			}
		}
	}


	if( t_ptrNodeCam )
	{
		// 如果人物跟相机在一个chunk内，返回
		if( t_ptrNodeChar )
		{
			if( t_ptrNodeCam->m_sNodeData.m_wRenderChunkIdx == 
				t_ptrNodeChar->m_sNodeData.m_wRenderChunkIdx )
				return 2;
		}

		for( int t_i=0;t_i<t_ptrNodeCam->m_sNodeData.m_wObjNum;t_i ++ )
		{
			t_iObjDevIdx = m_vecObjIdx[t_ptrNodeCam->m_sNodeData.m_wObjStartIdx+t_i];
			if( _objNum >= _maxObjNum )
				break;

			m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );
			if( t_bbox.ray_intersect( _pos,t_vec3CamDir,t_fDis ) ||
				t_bbox.pt_inBBox( t_vec3CamPos ) )
			{
				// 往队列内加入物品，直到队列满
				_objList[_objNum] = t_iObjDevIdx;
				_objNum ++;
				osassert( _objNum < _maxObjNum );
			}
		}
	}

	// 相机和人物都在此TG内
	if( t_ptrNodeCam && t_ptrNodeChar )
		return 2;
	else
		return 1;

	unguard;
}

/** \brief
*  传入一个位置，得到此位置是否跟quadTree超出tileGround的包围盒相交，并得到
*  跟哪个物品相交。
*
*/
int osc_tgQuadtree::get_posExceedQuadObj( osVec3D& _pos,int* _objList,int _maxObjNum )
{
	guard;

	osVec3D  t_vec3RayDir = osVec3D( 0.0f,-1.0f,0.0f );
	osVec3D  t_vec3RayStart = _pos;
	int      t_iObjNum = 0;
	float    t_fDis;
	int      t_iObjDevIdx;
	os_bbox  t_bbox;



	t_vec3RayStart.y += 400.0f;

	if( m_sTotalBox.ray_intersect( t_vec3RayStart,t_vec3RayDir,t_fDis ) ||
		m_sTotalBox.pt_inBox( t_vec3RayStart ) )
	{
		// 跟超出TileGround范围的物品一个个做碰撞检测。
		for( int t_i=0;t_i<(int)m_vecExceedObjIdx.size();t_i ++ )
		{
			t_iObjDevIdx = m_vecExceedObjIdx[t_i].m_wObjIdx;

			// ATTENTION: 非桥类物品,不参加碰撞检测:这个检测其实应该放到其它的地方:
			if( !m_ptrTGPtr->is_bridgeObj( t_iObjDevIdx ) )
				continue;

			m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );
			if( t_bbox.ray_intersect( t_vec3RayStart,t_vec3RayDir,t_fDis ) ||
				t_bbox.pt_inBBox( t_vec3RayStart ) )
			{
				_objList[t_iObjNum] = t_iObjDevIdx;
				t_iObjNum ++;
				
				// River @ 2011-10-18:更温和的出错处理方式
				//osassert( t_iObjNum < 32 );
				if( t_iObjNum >= 32 )
					t_iObjNum --;
			}
		}

	}

	return t_iObjNum;

	unguard;
}


/** \brief
*  传入一个位置,得到此位置所在chunk内所有物品的列表
*
*  \param osVec3D& _pos  传入想要得到的物品列表的位置.
*  \param int* _objList  返回物品的id列表.
*  \return int           返回的物品数量.
*/
int osc_tgQuadtree::get_posInChunkObj( osVec3D& _pos,int* _objList,int _maxObjNum )
{
	guard;

	osassert( _pos.x >= 0.0f );
	osassert( _pos.z >= 0.0f );
	osassert( _objList );
	osassert( m_ptrTGPtr );

	qtNode_ptr         t_ptrNodePos = NULL;
	float              t_fDis;

	t_ptrNodePos = find_node( m_ptrRootNode,_pos,false );
	if( !t_ptrNodePos )
		return 0;

	os_bbox     t_bbox;
	int         t_iObjDevIdx;


	osVec3D  t_vec3RayDir = osVec3D( 0.0f,-1.0f,0.0f );
	osVec3D  t_vec3RayStart = _pos;
	int      t_iObjNum = 0;

	t_vec3RayStart.y += 400.0f;
	for( int t_i=0;t_i<t_ptrNodePos->m_sNodeData.m_wObjNum;t_i ++ )
	{
		if( t_iObjNum >= _maxObjNum )
			break;

		t_iObjDevIdx = m_vecObjIdx[t_ptrNodePos->m_sNodeData.m_wObjStartIdx+t_i];

		// ATTENTION: 非桥类物品,不参加碰撞检测:这个检测其实应该放到其它的地方:
		if( !m_ptrTGPtr->is_bridgeObj( t_iObjDevIdx ) )
			continue;

		m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );

		if( t_bbox.ray_intersect( t_vec3RayStart,t_vec3RayDir,t_fDis ) ||
			t_bbox.pt_inBBox( t_vec3RayStart ) )
		{
			// 往队列内加入物品，直到队列满
			_objList[t_iObjNum] = t_iObjDevIdx;
			t_iObjNum ++;
		}
	}

	return t_iObjNum;

	unguard;
}

/** \brief
*  鼠标点选时，四叉树处理出射线相交过的Chunk列表
*  
*  \param const os_tileIdx*&  上层传入的指针，用于得到可能相交的chunk列表
*                             如果可能相交的chunk为零，此值在函数内赋为NULL
*/
int osc_tgQuadtree::get_pickChunkList( osVec3D& _rayStart,osVec3D& _rayDir,
					const os_tileIdx*& _chunkArr,const osVec2D*& _chunkXZStart,
					osVec3D* _lineSegEnd/* = NULL*/ )
{
	guard;

	osassert( m_ptrTGPtr );

	m_iPickChunkNum = 0;

	m_vec3RayStart = _rayStart;
	m_vec3RayDir = _rayDir;

	if( _lineSegEnd )
	{
		m_bPickWithLs = true;
		m_vec3LsEnd = *_lineSegEnd;
	}
	else
		m_bPickWithLs = false;

	// 如果跟当前quadTree node的aabbox不相交，则直接返回
	float      t_fInterDis = 0.0f;

	m_ptrRootNode->m_sNodeData.m_sNodeBox.m_vecMax.y = 
		m_ptrRootNode->m_sNodeData.m_fMaxHeight;

	// 是否使用线段做测试:
	if( m_bPickWithLs )
	{
		if( !m_ptrRootNode->m_sNodeData.m_sNodeBox.ls_intersect(
			m_vec3RayStart,m_vec3LsEnd ) )
		{
			_chunkArr = NULL;
			_chunkXZStart = NULL;
			return m_iPickChunkNum;
		}
	}
	else
	{
		if( !m_ptrRootNode->m_sNodeData.m_sNodeBox.ray_intersect(
			m_vec3RayStart,m_vec3RayDir,t_fInterDis ) )
		{
			_chunkArr = NULL;
			_chunkXZStart = NULL;
			return m_iPickChunkNum;
		}

		// 用于去除在射线的负方向上相交的BBox
		if( (t_fInterDis < 0.0f)&&
			(!m_ptrRootNode->m_sNodeData.m_sNodeBox.pt_inBox( _rayStart )) )
		{
			_chunkArr = NULL;
			_chunkXZStart = NULL;

			return m_iPickChunkNum;
		}

	}

	// 
	// 递归得到所有可能相交的Chunk索引
	qtNode_ptr*   t_arrNode;
	t_arrNode = m_ptrRootNode->get_childPtrArray();
	get_chunkList( t_arrNode[0] );
	get_chunkList( t_arrNode[1] );
	get_chunkList( t_arrNode[2] );
	get_chunkList( t_arrNode[3] );

	_chunkArr = this->m_arrPickChunk;
	_chunkXZStart = this->m_arrChunkXZ;

	return m_iPickChunkNum;

	unguard;
}


//! 根据动态光的颜色和位置范围，来计算当前node的动态光颜色
void osc_tgQuadtree::cal_nodeObjDLight( qtNode_ptr _node )
{
	guard;

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox*            t_ptrObjBox;

	osassert( _node->is_leafNode() );

	for( int t_i=0;t_i<t_ptrData->m_wObjNum;t_i ++ )
	{
		t_ptrObjBox = m_ptrTGPtr->get_objAaabbox(
			m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );

		for( int t_j=0;t_j<m_iAffectDlNum;t_j ++ )
		{			
			// 不受此动态光的影响
			if( !t_ptrObjBox->collision( m_vecDlBSphere[t_j] ) )
				continue;

			m_ptrTGPtr->set_objDlight( 
				m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i],m_iDlIdx[t_j] );

		}
		
	}


	return;

	unguard;
}


//! 递归检测每一个结点，是否和传入的sphere列表碰撞
void osc_tgQuadtree::col_withSphere( qtNode_ptr _node )
{
	guard;


	for( int t_i=0;t_i<m_iAffectDlNum;t_i ++ )
	{
		if( _node->m_sNodeData.m_sNodeBox.collision( m_vecDlBSphere[t_i] ) )
		{
			// 如果是叶结点,处理
			if( _node->is_leafNode() )
			{
				osassert( _node->m_sNodeData.m_wRenderChunkIdx != 0xffff );
				osassert( m_ptrTGPtr );

				m_ptrTGPtr->set_chunkAffectByDL( _node->m_sNodeData.m_wRenderChunkIdx );

				// 当前结点内动态物品的动态光
				cal_nodeObjDLight( _node );
			}
			else	
			{
				// 
				// 非叶结点,弟归处理
				qtNode_ptr*   t_arrNode;
				t_arrNode = _node->get_childPtrArray();

				col_withSphere( t_arrNode[0] );
				col_withSphere( t_arrNode[1] );
				col_withSphere( t_arrNode[2] );
				col_withSphere( t_arrNode[3] );
			}

			break;
		}
	}

	return;

	unguard;
}


/** \brief
*  处理动态光影响哪一个地图内的Chunk.
*
*  根据每一个动态光的影响范围来确认当前的Chunk是否受动态光影响
*/
void osc_tgQuadtree::set_dlAffectChunk(  const os_dlInfo* _dlInfo )
{
	guard;


	bool   t_bCol = false;

	m_iAffectDlNum = 0;
	for( int t_i=0;t_i<_dlInfo->m_iDLNum;t_i ++ )
	{
		m_vecDlBSphere[m_iAffectDlNum].radius = _dlInfo->m_vecDLPtr[t_i]->m_fRadius;
		m_vecDlBSphere[m_iAffectDlNum].veccen = _dlInfo->m_vecDLPtr[t_i]->m_vec3Pos;

		if( m_ptrRootNode->m_sNodeData.m_sNodeBox.collision( m_vecDlBSphere[m_iAffectDlNum] ) )
		{
			m_vecLightColor[m_iAffectDlNum] = _dlInfo->m_vecDLPtr[t_i]->m_dlightColor;
			m_iDlIdx[m_iAffectDlNum] = _dlInfo->m_iDLId[t_i];
			m_iAffectDlNum ++;
		}

	}

	// 没有碰撞的sphere
	if( 0 == m_iAffectDlNum )
		return;


	// 对每一个子结点进行碰撞检测
	qtNode_ptr*   t_arrNode;
	t_arrNode = m_ptrRootNode->get_childPtrArray();

	col_withSphere( t_arrNode[0] );
	col_withSphere( t_arrNode[1] );
	col_withSphere( t_arrNode[2] );
	col_withSphere( t_arrNode[3] );

	return;

	unguard;
}


//! 传入的射线是否跟当前的QuadTree相交
bool osc_tgQuadtree::ray_intersect( osVec3D& _rayStart,osVec3D& _rayDir )
{
	guard;

	float t_fDis;

	// 如果相交，返回true.
	if( m_sTotalBox.ray_intersect( _rayStart,_rayDir,t_fDis ) ||
		m_sTotalBox.pt_inBox( _rayStart ) )
		return true;
	else
		return false;

	unguard;
}



