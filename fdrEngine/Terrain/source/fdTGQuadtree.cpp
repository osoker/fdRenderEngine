//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTGQuadtree.cpp
 *
 *  His:      River created @ 2005-11-17
 *
 *  Des:      ÿһ��TG��Ӧ���Ĳ���������ݽṹ,sceneGraph����Щ���ݽṹ����.
 *   
 *  �����������㣬������������Stay Hungry,Stay Foolish.�� 
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdTGQuadtree.h"
# include "../include/fdNewFieldStruct.h"
# include "../../mfPipe/include/osCharacter.h"
# include "../geoMipTerrain/MapTile.h"
# include "../../effect/include/dlight.h"


//! Ԥ�����ͼʱ��Ĭ�ϵ�ͼÿһ��bbox�ĸ߶�
# define DEFAULT_TGBOXHEIGHT  1.0f

//! River added @ 2008-12-15:�Ƿ�ʹ��ˮ�浹Ӱ
OSENGINE_API BOOL                   g_bWaterReflect = true;

//! River added @ 2009-8-30: �Ƿ���Ⱦ����������Ʒ��Ӱ,�����ֵΪfalse,����Ⱦ��������Ʒ�͵����е�Ӱ��
//!                          ��ֵĬ��Ϊ�档
OSENGINE_API BOOL                   g_bTerrWaterReflect = TRUE;


//! ȫ�ֵ�vfָ��.
os_ViewFrustum* osc_tgQuadtree::m_ptrVf = NULL;


osc_tgQuadtree::osc_tgQuadtree()
{
	m_ptrTGPtr = NULL;
}

osc_tgQuadtree::~osc_tgQuadtree()
{

}

//! Ϊÿһ��������Chunk����
void osc_tgQuadtree::allot_chunkIdx( void )
{
	//! ���������Ǹ���㡣

	//! ��1��4�����

	//! ���½��ΪҶ���
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


�Ĳ����ĸ�����Ӧ��ͼ���ĸ�����:

z����
|
|     3    4
|
|     1    2
|
Origin----------x����

����ͼ�����½ǣ�Ϊ��������ӽ�㣬���½�����Ϊ��һ���ӽ��


*/

//! Ϊ��һ��������BBox.
void osc_tgQuadtree::allot_aabbox( qtNode_ptr _nodePtr,
				const os_chunkUtmostHeight* _ptrHei,os_aabbox& _box )
{
	guard;

	osassert( _nodePtr );
	osassert( _ptrHei );

	os_aabbox    t_aabbox;

	_nodePtr->m_sNodeData.m_sNodeBox = _box;

	// Ҷ��������£��õ���Ҷ�������͵Ķ���߶���ߵĶ���߶�
	if( _nodePtr->is_leafNode() )
	{
		_nodePtr->m_sNodeData.m_sNodeBox.m_vecMax.y = 
			_ptrHei[_nodePtr->m_sNodeData.m_wRenderChunkIdx].m_fMaxHeight;
		_nodePtr->m_sNodeData.m_sNodeBox.m_vecMin.y = 
			_ptrHei[_nodePtr->m_sNodeData.m_wRenderChunkIdx].m_fMinHeight;
	}
	else    // ��Ҷ��������£��ݹ����
	{
		osVec3D      t_vec3Center = _box.get_center();
		t_vec3Center.y = _box.m_vecMax.y;
		qtNode_ptr*  t_arrNodePtr = _nodePtr->get_childPtrArray();

	
		// ���½ǵĽ��
		t_aabbox.m_vecMin = _box.m_vecMin;
		t_aabbox.m_vecMax = t_vec3Center;
		allot_aabbox( t_arrNodePtr[0],_ptrHei,t_aabbox );

		// ���½ǵĽ��
		t_aabbox.m_vecMin = _box.m_vecMin;
		t_aabbox.m_vecMin.x = t_vec3Center.x;
		t_aabbox.m_vecMax = _box.m_vecMax;
		t_aabbox.m_vecMax.z = t_vec3Center.z;
		allot_aabbox( t_arrNodePtr[1],_ptrHei,t_aabbox );

		// ���ϽǵĽ��
		t_aabbox.m_vecMin = _box.m_vecMin;
		t_aabbox.m_vecMin.z = t_vec3Center.z;
		t_aabbox.m_vecMax = _box.m_vecMax;
		t_aabbox.m_vecMax.x = t_vec3Center.x;
		allot_aabbox( t_arrNodePtr[2],_ptrHei,t_aabbox );

		// ���ϽǵĽ��
		t_aabbox.m_vecMin = t_vec3Center;
		t_aabbox.m_vecMin.y = _box.m_vecMin.y;
		t_aabbox.m_vecMax = _box.m_vecMax;
		allot_aabbox( t_arrNodePtr[3],_ptrHei,t_aabbox );

		// �����ӽ��õ�ÿһ����Ҷ����BBox
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
*  ���ڴ��е����Ĳ�������
*
*  �㷨������
*  1: Ϊ�Ĳ���Ҷ���ָ��chunk����,������򵥵��Ĳ�������,Ϊÿһ��Ҷ������BBox
*  2: �����Ĳ�����obj���ݡ�
*  3: ���ݸ߶����ݣ�����ÿһ������aabbox����
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

	// 1: ������򵥵��Ĳ������ݣ�ÿһ���Ĳ������ĸ��ӿ�ʼ��ͽ�����
	os_aabbox      t_aabbox;
	t_aabbox.m_vecMin.x = t_ptrHeader->m_wMapPosX*TILE_WIDTH*MAX_GROUNDWIDTH;
	t_aabbox.m_vecMax.x = t_aabbox.m_vecMin.x + TILE_WIDTH*MAX_GROUNDWIDTH;

	t_aabbox.m_vecMin.z = t_ptrHeader->m_wMapPosZ*TILE_WIDTH*MAX_GROUNDWIDTH;
	t_aabbox.m_vecMax.z = t_aabbox.m_vecMin.z + TILE_WIDTH*MAX_GROUNDWIDTH;

	t_aabbox.m_vecMin.y = -DEFAULT_TGBOXHEIGHT;
	t_aabbox.m_vecMax.y = DEFAULT_TGBOXHEIGHT;

	allot_aabbox( m_ptrRootNode,t_ptrHeight,t_aabbox );


	// 
	// 2: �����Ĳ�����aabbox���ݺ�obj���ݡ�
	// 3: ���ݸ߶����ݣ�����ÿһ������aabbox����
	WORD          t_w;
	const BYTE*    t_fstart = (const BYTE*)
		(_dstart + t_ptrHeader->m_dwQuadTreeDataOffset );

	// ������ͼ����Ʒ������Total AABBox.
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
	// �����Ĳ����ڵ���Ʒ������
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
//! ��Ⱦÿһ������aabbox.
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
//! ��һ��������Ҵ˽�㵱ǰ���ڵ�Quadtree leaf node.
qtNode_ptr osc_tgQuadtree::find_node( qtNode_ptr _node,osVec3D& _pt,bool _useY/* = true*/ )
{
	guard;

	bool     t_bInCurBox = false;
	osVec3D  t_vec3Pt = _pt;
 
	// �����Ƿ��ڵ�ǰ��BBox,������Ʒ�ĸ߶�BBox
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
		// ���β����ĸ��ӽ��
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


//! Check&Add quadtree Obj��ʾ����
void osc_tgQuadtree::check_nodeObjDis( qtNode_ptr _node,osMatrix& _reflMat )
{
	guard;

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox*           t_sAabbox = NULL;

	// ����ÿһ����Ʒ��aaBBox�Ƿ��ڳ�����
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
			// ���ȫ��ʹ�õ�Ӱ������������Ʒ�ĵ�Ӱ�Ƿ���vf��
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
				// River @ 2008-12-16: ������Ʒ�ĵ�Ӱ��������vf��
				m_ptrTGPtr->push_objIdx(
					m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i] );

			}
		}

	}

	unguard;
}



//! ��һ����㿪ʼ�õ���ǰ�������vf�ڵĵ�ͼ��������.
void osc_tgQuadtree::get_inVfTile( qtNode_ptr _node,osMatrix& _reflMat )
{
	guard;

	osassert( m_ptrVf );

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox             t_box = t_ptrData->m_sNodeBox;
	bool                  t_bReflectBoxInView = false;


	// ʹ������bbox�����
	float   t_fChunkHei = t_box.m_vecMax.y;
	t_box.m_vecMax.y = _node->m_sNodeData.m_fMaxHeight;
	if( !m_ptrVf->objInVF( t_box ) )
	{
		// ˮ�浹Ӱ��Ҫ�������Ŀ���terr
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
	// ��ǰ����bboxȫ����vf�ڻ��ǵ�ǰ�Ľ����Ҷ���.
	if( _node->is_leafNode() )
	{
		osassert( _node->m_sNodeData.m_wRenderChunkIdx != 0xffff );
		t_box.m_vecMax.y = t_fChunkHei;

		// River added @ 2008-12-15:���������reflect in view.
		if( t_bReflectBoxInView )
		{
			m_ptrTGPtr->set_chunkDraw( _node->m_sNodeData.m_wRenderChunkIdx );
		}
		// һ:
		// ����ر��aabboxȫ����������,��Ⱦ��ǰ����ڵ���Ʒ,
		// ����Ҫ�����ж�bbox���ݣ�ֱ������.�ر�������Ⱦ.
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

			// ��:
			// ����ر��aabbox������������,��ÿһ��obj��bbox��vf����������
			// �ر�������Ⱦ.
			if( m_ptrVf->objInVF( t_box ) )
			{
				check_nodeObjDis( _node,_reflMat );
				m_ptrTGPtr->set_chunkDraw( _node->m_sNodeData.m_wRenderChunkIdx );

			}
			else
			{
				// ��:
				// ����ر��aabbox����������,����Ʒ��bbox��������.��ÿһ��
				// obj��bbox��vf����������.�ر���Ҫ��������Ⱦ.
				check_nodeObjDis( _node,_reflMat );
			}
		}
	}
	else
	{
		// 
		// ��Ҷ���,aabbox������vf�ڵ����,��Ҫ�������ĵܹ鴦��
		qtNode_ptr*   t_arrNode;
		t_arrNode = _node->get_childPtrArray();

		get_inVfTile( t_arrNode[0],_reflMat );
		get_inVfTile( t_arrNode[1],_reflMat );
		get_inVfTile( t_arrNode[2],_reflMat );
		get_inVfTile( t_arrNode[3],_reflMat );

	}

	// 
	// River @ 2006-6-17: �Գ�����ͼ�߽����Ʒ���д���.��������߽����Ʒû
	// �и���ĳ���ɼ��Ľ��,������Ʒ����������,�ͳ��ֿɼ��Ե�����
	for( int t_i=0;t_i<(int)m_vecExceedObjIdx.size();t_i ++ )
	{
		int t_idx = m_vecExceedObjIdx[t_i].m_wObjIdx;

		// ����Ѿ��ɼ�,����Ҫ����
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

//! �ݹ�õ����к������ཻ��Chunk�б�
void osc_tgQuadtree::get_chunkList( qtNode_ptr _node )
{
	guard;

	fd_quadTreeNodeData*  t_ptrData = &_node->m_sNodeData;
	os_aabbox*   t_ptrBox = &t_ptrData->m_sNodeBox;

	// ����Ray���
	float  t_fInterDis;
	

	// River @ 2006-7-13: ������߶��ཻ
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

		// ȥ���������ϵ�BBox.
		if( ( t_fInterDis < 0.0f )&&
			( !t_ptrBox->pt_inBox( m_vec3RayStart ) ) )
		{
			return;
		}
	}

	// ��ǰ����bboxȫ����vf�ڻ��ǵ�ǰ�Ľ����Ҷ���.
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
		// ��Ҷ���,�ܹ鴦��
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



//! ���㵱ǰ��vf�ڵĽ����Ŀ
void osc_tgQuadtree::cal_visualTileSet( osMatrix& _reflMat )
{
	guard;

	// 
	if( !m_ptrVf&&g_ptrCamera)
		m_ptrVf = g_ptrCamera->get_frustum();
	if (!m_ptrVf)  return;

	
	// 
	// �ȴ�������ͼ�߽����Ʒ,��Щ��Ʒ����Ԥ�����Ĳ������д洢
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

	// ������������
	get_inVfTile( m_ptrRootNode,_reflMat );


	unguard;
}

//! TEST CODE: ������Ⱦ��ǰ��quadTree.
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

//! get_chunkObj�����Ĳ��䣬���ڵõ�����chunk�߽����Ʒ�б�
int osc_tgQuadtree::get_exceedChunkObj( osVec3D& _spos,osVec3D& _epos,
									   int* _objList,int& _objNum,int _maxObjNum )
{
	guard;

	osVec3D   t_vec3Dir = _epos - _spos;
	float     t_fLength = osVec3Length( &t_vec3Dir );
	float     t_fTmp;
	
	osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

	// �п����ཻ����һ���Ĵ���
	if( m_sTotalBox.ray_intersect( _spos,t_vec3Dir,t_fTmp ) ||
		m_sTotalBox.pt_inBox( _spos ) )
	{
		os_bbox   t_bbox;
		int       t_iObjDevIdx;
		// ������TileGround��Χ����Ʒһ��������ײ��⡣
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
*  ���ݴ����λ�ã��õ���ǰλ�����ڵ�quadTree Chunk�ڵ���Ʒ�����б�
*
*  ���������Ҫ���ڵ�ס�����Ʒ�İ�͸����
*  \param _pos ���������������ڵ�λ�á�
*  \param _objList λ�����ڵ�Chunk����Ʒ�������б�
*  \param _objNum  ��������������ɵ������Ʒ��������λ�����ڵ�Chunk����Ʒ��������Ŀ��
*  \return int     �����Ѿ��ҵ���λ����Ŀ,��������������ڴ�tg,�򷵻�2,���
*                  �����ڣ������㡣
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

	//! �õ���ǰ����������صĲ�����
	g_ptrCamera->get_curpos( &t_vec3CamPos );

	t_ptrNodeChar = find_node( m_ptrRootNode,_pos,false );
	t_ptrNodeCam = find_node( m_ptrRootNode,t_vec3CamPos,false );
	if( (!t_ptrNodeChar)&&(!t_ptrNodeCam) ) 
	{
		// river @ 2009-2-8: �Ƿ������chunk�߽����Ʒ�ཻ��������б�
		get_exceedChunkObj( _pos,t_vec3CamPos,_objList,_objNum,_maxObjNum );
		return 0;
	}

	os_bbox     t_bbox;
	int         t_iObjDevIdx;


	t_vec3CamDir = _pos - t_vec3CamPos;
	t_fDis = osVec3Length( &t_vec3CamDir );
	osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );


	// �������е�Chunk��������ڵ�Chunk����Ҫ����һ��
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
				// �������ڼ�����Ʒ��ֱ��������
				_objList[_objNum] = t_iObjDevIdx;
				_objNum ++;
				osassert( _objNum < _maxObjNum );
			}
		}
	}


	if( t_ptrNodeCam )
	{
		// �������������һ��chunk�ڣ�����
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
				// �������ڼ�����Ʒ��ֱ��������
				_objList[_objNum] = t_iObjDevIdx;
				_objNum ++;
				osassert( _objNum < _maxObjNum );
			}
		}
	}

	// ��������ﶼ�ڴ�TG��
	if( t_ptrNodeCam && t_ptrNodeChar )
		return 2;
	else
		return 1;

	unguard;
}

/** \brief
*  ����һ��λ�ã��õ���λ���Ƿ��quadTree����tileGround�İ�Χ���ཻ�����õ�
*  ���ĸ���Ʒ�ཻ��
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
		// ������TileGround��Χ����Ʒһ��������ײ��⡣
		for( int t_i=0;t_i<(int)m_vecExceedObjIdx.size();t_i ++ )
		{
			t_iObjDevIdx = m_vecExceedObjIdx[t_i].m_wObjIdx;

			// ATTENTION: ��������Ʒ,���μ���ײ���:��������ʵӦ�÷ŵ������ĵط�:
			if( !m_ptrTGPtr->is_bridgeObj( t_iObjDevIdx ) )
				continue;

			m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );
			if( t_bbox.ray_intersect( t_vec3RayStart,t_vec3RayDir,t_fDis ) ||
				t_bbox.pt_inBBox( t_vec3RayStart ) )
			{
				_objList[t_iObjNum] = t_iObjDevIdx;
				t_iObjNum ++;
				
				// River @ 2011-10-18:���º͵ĳ�����ʽ
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
*  ����һ��λ��,�õ���λ������chunk��������Ʒ���б�
*
*  \param osVec3D& _pos  ������Ҫ�õ�����Ʒ�б��λ��.
*  \param int* _objList  ������Ʒ��id�б�.
*  \return int           ���ص���Ʒ����.
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

		// ATTENTION: ��������Ʒ,���μ���ײ���:��������ʵӦ�÷ŵ������ĵط�:
		if( !m_ptrTGPtr->is_bridgeObj( t_iObjDevIdx ) )
			continue;

		m_ptrTGPtr->get_objBBox( t_iObjDevIdx,t_bbox );

		if( t_bbox.ray_intersect( t_vec3RayStart,t_vec3RayDir,t_fDis ) ||
			t_bbox.pt_inBBox( t_vec3RayStart ) )
		{
			// �������ڼ�����Ʒ��ֱ��������
			_objList[t_iObjNum] = t_iObjDevIdx;
			t_iObjNum ++;
		}
	}

	return t_iObjNum;

	unguard;
}

/** \brief
*  ����ѡʱ���Ĳ�������������ཻ����Chunk�б�
*  
*  \param const os_tileIdx*&  �ϲ㴫���ָ�룬���ڵõ������ཻ��chunk�б�
*                             ��������ཻ��chunkΪ�㣬��ֵ�ں����ڸ�ΪNULL
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

	// �������ǰquadTree node��aabbox���ཻ����ֱ�ӷ���
	float      t_fInterDis = 0.0f;

	m_ptrRootNode->m_sNodeData.m_sNodeBox.m_vecMax.y = 
		m_ptrRootNode->m_sNodeData.m_fMaxHeight;

	// �Ƿ�ʹ���߶�������:
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

		// ����ȥ�������ߵĸ��������ཻ��BBox
		if( (t_fInterDis < 0.0f)&&
			(!m_ptrRootNode->m_sNodeData.m_sNodeBox.pt_inBox( _rayStart )) )
		{
			_chunkArr = NULL;
			_chunkXZStart = NULL;

			return m_iPickChunkNum;
		}

	}

	// 
	// �ݹ�õ����п����ཻ��Chunk����
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


//! ���ݶ�̬�����ɫ��λ�÷�Χ�������㵱ǰnode�Ķ�̬����ɫ
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
			// ���ܴ˶�̬���Ӱ��
			if( !t_ptrObjBox->collision( m_vecDlBSphere[t_j] ) )
				continue;

			m_ptrTGPtr->set_objDlight( 
				m_vecObjIdx[t_ptrData->m_wObjStartIdx+t_i],m_iDlIdx[t_j] );

		}
		
	}


	return;

	unguard;
}


//! �ݹ���ÿһ����㣬�Ƿ�ʹ����sphere�б���ײ
void osc_tgQuadtree::col_withSphere( qtNode_ptr _node )
{
	guard;


	for( int t_i=0;t_i<m_iAffectDlNum;t_i ++ )
	{
		if( _node->m_sNodeData.m_sNodeBox.collision( m_vecDlBSphere[t_i] ) )
		{
			// �����Ҷ���,����
			if( _node->is_leafNode() )
			{
				osassert( _node->m_sNodeData.m_wRenderChunkIdx != 0xffff );
				osassert( m_ptrTGPtr );

				m_ptrTGPtr->set_chunkAffectByDL( _node->m_sNodeData.m_wRenderChunkIdx );

				// ��ǰ����ڶ�̬��Ʒ�Ķ�̬��
				cal_nodeObjDLight( _node );
			}
			else	
			{
				// 
				// ��Ҷ���,�ܹ鴦��
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
*  ����̬��Ӱ����һ����ͼ�ڵ�Chunk.
*
*  ����ÿһ����̬���Ӱ�췶Χ��ȷ�ϵ�ǰ��Chunk�Ƿ��ܶ�̬��Ӱ��
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

	// û����ײ��sphere
	if( 0 == m_iAffectDlNum )
		return;


	// ��ÿһ���ӽ�������ײ���
	qtNode_ptr*   t_arrNode;
	t_arrNode = m_ptrRootNode->get_childPtrArray();

	col_withSphere( t_arrNode[0] );
	col_withSphere( t_arrNode[1] );
	col_withSphere( t_arrNode[2] );
	col_withSphere( t_arrNode[3] );

	return;

	unguard;
}


//! ����������Ƿ����ǰ��QuadTree�ཻ
bool osc_tgQuadtree::ray_intersect( osVec3D& _rayStart,osVec3D& _rayDir )
{
	guard;

	float t_fDis;

	// ����ཻ������true.
	if( m_sTotalBox.ray_intersect( _rayStart,_rayDir,t_fDis ) ||
		m_sTotalBox.pt_inBox( _rayStart ) )
		return true;
	else
		return false;

	unguard;
}



