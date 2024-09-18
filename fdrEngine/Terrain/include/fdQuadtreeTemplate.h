//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdQuadtreeTemplate.h
 *
 *  His:      River created @ 2005-11-15
 *
 *  Des:      地图使用的四叉树模板类,此处仅仅为模板，主要功能使用此模板实现的类
 *   
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

//! 包含引擎的接口头文件
# include "../../interface/osInterface.h"
# include "math.h"

//! 四叉树的深度
# define QUADTREE_DEPTH   3

//! 先声明模板类
template <class type> class os_quadtreeTemplate;



//! 四叉树结点的template
template <class type> class os_qtNodeTemplate
{
	friend class os_quadtreeTemplate<type>;
private:
	os_qtNodeTemplate<type>*  m_ptrChild[4];
	os_qtNodeTemplate<type>*  m_ptrParent;

public:
	type                    m_sNodeData;

public:
	os_qtNodeTemplate()
	{
		for( int t_i=0;t_i<4;t_i ++ )
			m_ptrChild[t_i] = NULL;
		m_ptrParent = NULL;
	}

	os_qtNodeTemplate<type>**   get_childPtrArray( void )
	{
		return m_ptrChild;
	}
	os_qtNodeTemplate<type>*    get_parentPtr( void )
	{
		return m_ptrParent;
	}
	BOOL                      is_leafNode( void )
	{
		if( NULL == m_ptrChild[0] )
			return true;
		else
			return false;
	}
	BOOL                      is_rootNode( void )
	{
		if( NULL == m_ptrParent )
			return true;
		else
			return false;
	}
	//! 
	type*                     get_nodeDataPtr( void )
	{
		return &m_sNodeData;
	}


};

//! 四叉树的template，最简单的功能，程序中使用数组
template <class type> class os_quadtreeTemplate
{
protected:
	os_qtNodeTemplate<type>*     m_arrNodeList;
	
	static int                 m_iNodeNum;
	static void                cal_nodeNum( void );


	//! 对每一个结点指定子点和父结点
	void                      deploy_quadTree( void );

protected:

	os_qtNodeTemplate<type>*     m_ptrRootNode;

	//! 叶结点开始的索引
	static int                 m_iLeafNodeSidx;

public:
	os_quadtreeTemplate();
	~os_quadtreeTemplate();

	//! 得到可操作的根结点
	os_qtNodeTemplate<type>*    get_rootNodePtr( void )
	{
		return m_ptrRootNode;
	}

};

//! 每一个四叉树所有的结点的数目和.
template <class type> int os_quadtreeTemplate<type>::m_iNodeNum = 0;
//! 叶结点开始的索引
template <class type> int os_quadtreeTemplate<type>::m_iLeafNodeSidx = 0;



template <class type> os_quadtreeTemplate<type>::os_quadtreeTemplate()
{
	if( m_iNodeNum == 0 )
		cal_nodeNum();

	m_arrNodeList = NULL;
	m_arrNodeList = new os_qtNodeTemplate<type>[m_iNodeNum];

	// 初始化树状结构
	m_ptrRootNode = &m_arrNodeList[0];
	deploy_quadTree();

}

template <class type> void os_quadtreeTemplate<type>::cal_nodeNum( void )
{
	m_iNodeNum = 0;
	
	for( int t_i=QUADTREE_DEPTH-1;t_i>=0;t_i -- )
		m_iNodeNum += round_float( float(pow( 4.0f,t_i )) );

}

template <class type> os_quadtreeTemplate<type>::~os_quadtreeTemplate()
{
	SAFE_DELETE_ARRAY( m_arrNodeList );
}

//! 对每一个结点指定子点和父结点
template <class type> void os_quadtreeTemplate<type>::deploy_quadTree( void )
{
	// 先处理头一层
	m_arrNodeList[0].m_ptrChild[0] = &m_arrNodeList[1];
	m_arrNodeList[0].m_ptrChild[1] = &m_arrNodeList[2];
	m_arrNodeList[0].m_ptrChild[2] = &m_arrNodeList[3];
	m_arrNodeList[0].m_ptrChild[3] = &m_arrNodeList[4];
	m_arrNodeList[1].m_ptrParent = &m_arrNodeList[0];
	m_arrNodeList[2].m_ptrParent = &m_arrNodeList[0];
	m_arrNodeList[3].m_ptrParent = &m_arrNodeList[0];
	m_arrNodeList[4].m_ptrParent = &m_arrNodeList[0];


	// 处理四叉树的每一层结点
	int   t_iCurLayerNodeIdx = 1;
	int   t_iCurLayerNodeNum = 0;
	int   t_iNextLayerNodeIdx = 0;
	for( int t_i=1;t_i<QUADTREE_DEPTH-1;t_i ++ )
	{
		// 得到当前层结点的数目
		t_iCurLayerNodeNum = round_float( float(pow( 4.0f,t_i )) );
		
		t_iNextLayerNodeIdx = t_iCurLayerNodeIdx + t_iCurLayerNodeNum;
		int  t_iIdx = t_iNextLayerNodeIdx;
		for( int t_j = t_iCurLayerNodeIdx;t_j < t_iNextLayerNodeIdx; t_j ++ )
		{
			m_arrNodeList[t_j].m_ptrChild[0] = &m_arrNodeList[t_iIdx];
			m_arrNodeList[t_iIdx].m_ptrParent = &m_arrNodeList[t_j];
			t_iIdx ++ ;

			m_arrNodeList[t_j].m_ptrChild[1] = &m_arrNodeList[t_iIdx];
			m_arrNodeList[t_iIdx].m_ptrParent = &m_arrNodeList[t_j];
			t_iIdx ++ ;

			m_arrNodeList[t_j].m_ptrChild[2] = &m_arrNodeList[t_iIdx];
			m_arrNodeList[t_iIdx].m_ptrParent = &m_arrNodeList[t_j];
			t_iIdx ++ ;

			m_arrNodeList[t_j].m_ptrChild[3] = &m_arrNodeList[t_iIdx];
			m_arrNodeList[t_iIdx].m_ptrParent = &m_arrNodeList[t_j];
			t_iIdx ++ ;
		}

		t_iCurLayerNodeIdx = t_iNextLayerNodeIdx;
	}

	m_iLeafNodeSidx = t_iCurLayerNodeIdx;

	return;

}


