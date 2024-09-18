//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdQuadtreeTemplate.h
 *
 *  His:      River created @ 2005-11-15
 *
 *  Des:      ��ͼʹ�õ��Ĳ���ģ����,�˴�����Ϊģ�壬��Ҫ����ʹ�ô�ģ��ʵ�ֵ���
 *   
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

//! ��������Ľӿ�ͷ�ļ�
# include "../../interface/osInterface.h"
# include "math.h"

//! �Ĳ��������
# define QUADTREE_DEPTH   3

//! ������ģ����
template <class type> class os_quadtreeTemplate;



//! �Ĳ�������template
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

//! �Ĳ�����template����򵥵Ĺ��ܣ�������ʹ������
template <class type> class os_quadtreeTemplate
{
protected:
	os_qtNodeTemplate<type>*     m_arrNodeList;
	
	static int                 m_iNodeNum;
	static void                cal_nodeNum( void );


	//! ��ÿһ�����ָ���ӵ�͸����
	void                      deploy_quadTree( void );

protected:

	os_qtNodeTemplate<type>*     m_ptrRootNode;

	//! Ҷ��㿪ʼ������
	static int                 m_iLeafNodeSidx;

public:
	os_quadtreeTemplate();
	~os_quadtreeTemplate();

	//! �õ��ɲ����ĸ����
	os_qtNodeTemplate<type>*    get_rootNodePtr( void )
	{
		return m_ptrRootNode;
	}

};

//! ÿһ���Ĳ������еĽ�����Ŀ��.
template <class type> int os_quadtreeTemplate<type>::m_iNodeNum = 0;
//! Ҷ��㿪ʼ������
template <class type> int os_quadtreeTemplate<type>::m_iLeafNodeSidx = 0;



template <class type> os_quadtreeTemplate<type>::os_quadtreeTemplate()
{
	if( m_iNodeNum == 0 )
		cal_nodeNum();

	m_arrNodeList = NULL;
	m_arrNodeList = new os_qtNodeTemplate<type>[m_iNodeNum];

	// ��ʼ����״�ṹ
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

//! ��ÿһ�����ָ���ӵ�͸����
template <class type> void os_quadtreeTemplate<type>::deploy_quadTree( void )
{
	// �ȴ���ͷһ��
	m_arrNodeList[0].m_ptrChild[0] = &m_arrNodeList[1];
	m_arrNodeList[0].m_ptrChild[1] = &m_arrNodeList[2];
	m_arrNodeList[0].m_ptrChild[2] = &m_arrNodeList[3];
	m_arrNodeList[0].m_ptrChild[3] = &m_arrNodeList[4];
	m_arrNodeList[1].m_ptrParent = &m_arrNodeList[0];
	m_arrNodeList[2].m_ptrParent = &m_arrNodeList[0];
	m_arrNodeList[3].m_ptrParent = &m_arrNodeList[0];
	m_arrNodeList[4].m_ptrParent = &m_arrNodeList[0];


	// �����Ĳ�����ÿһ����
	int   t_iCurLayerNodeIdx = 1;
	int   t_iCurLayerNodeNum = 0;
	int   t_iNextLayerNodeIdx = 0;
	for( int t_i=1;t_i<QUADTREE_DEPTH-1;t_i ++ )
	{
		// �õ���ǰ�������Ŀ
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


