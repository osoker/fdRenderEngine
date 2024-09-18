//---------------------------------------------------------------------
  /** \file
   * Filename:newAstar.h
   *
   * Desc: ����һ��Astar��
   *
   * His : honghaier Created @  2003 12/18 9:19
   *
   * River comment @ 2003-2-4: �����յ�Ѱ·���Ƚ������ƣ�������Ѱ����
   *               ԭ��24�����ӵ����塣�������ֵ�Ż����յ�A*�㷨���ٶ�
   *               ���ȡ�
   * tyx mod @ 2004-3-26 ��εı䶯�Ƚϴ�,��Ҫ����ȫ����д,��������ǰ��
   *			   �������:
   *          1. ��дһ������������ ͳһ��������е�·������,��ȥ�˳����е�һЩ����Ҫ�����ƶ�.
   *          2. �����ȥPopstack();���� ���㷨�޸ĺ������ȫû�б�Ҫ
   *          3. �����ȥ//! ���������# define MAX_PROCESSTIME 1024*4 ��������޸ĺ� 
   *				�Ѿ�û�ж�������--�����µ�open����������,��������̫���ѭ������,��ȥ
					��������еļ��������Ի����ٵ���߳�����ٶ�.
   *          4. �����ȥ//! ����A*�����Ѱ·������# define MAX_PATHASTAR  48 ��Ϊ���µ����ݹ����¿����޸�
   *				open/close����ܵĴ�С������Ѱ·�������.
   *          5. �޸�OBLIQUE_VALUE ��ֵΪ16(ԭ�ȵ�ֵΪ14),��������ʹ�õ���·�����ӽ����·��.
   *          6. �Գ����е������ɵ�·�� ��һ���̶��ı���m_sTmp ����ÿ��������·��ʱnewһ����
   *			 ����,��߳����ٶ� �����Լ����ڴ���Ƭ
   *          7. �޸�Ѱ·�����ս�� ��Ŀ�겻�ɵ���ʱ,��������������·��.
   */
//---------------------------------------------------------------------
# pragma once

# include "../interface/osInterface.h" 

# include "math.h"
# include "VList.h"

//! ���ĵ�ͼ��Ŀ��
# define MAX_TGNUM 4

template<class _Ty>	class VECTOR
{
	std::vector<_Ty>	m_VECPath;
	int m_nCount;
	int m_nPos;
	int m_nInit;
public:
	VECTOR()
	{
		m_VECPath.clear();
		m_nCount = -1;
		m_nPos = -1;
	}
	void resize(WORD _nSize = 200/*�����������Ǹ�ż��*/)
	{
		m_VECPath.resize(_nSize);
		m_nInit = _nSize/2;
		clear();
	}
	void clear()
	{
		m_nCount = m_nInit;
		m_nPos = m_nInit;
	}
	WORD  count()
	{
		return (m_nCount - m_nPos);
	}
	void push_back(_Ty &_BaseMesh)
	{
		m_VECPath[m_nCount] = _BaseMesh;
		++m_nCount;
	}
	void pop_front()
	{
		if(m_nCount > m_nPos)
		{
			++m_nPos;
			if(m_nPos == m_nCount)
			{
				clear();
			}
		}
	}
	void push_front(_Ty &_BaseMesh)
	{
		--m_nPos;
		if( m_nPos >= 0 )
			m_VECPath[m_nPos] = _BaseMesh;
	}
	bool	empty()
	{
		return m_nPos == m_nCount ? true : false;
	}
	_Ty* front()
	{
		if(!empty())
			return &m_VECPath[m_nPos];
		return NULL;
	}
	_Ty& operator[] ( int i )
	{
		if(!empty())
		{
			osassert(i >= 0 && i < count());
			return m_VECPath[m_nPos + i];
		}

		return m_VECPath[0];

	}

};

/** \brief
//��Ѱ·�õ��ĵ�Ԫ�������ṹ
*/
struct BaseMesh
{
	int nX;
	int nY;
	BOOL operator == ( CONST BaseMesh& t_mesh )
	{
		return (nX == t_mesh.nX && nY == t_mesh.nY);
	}
};

#define PATH  VECTOR<BaseMesh>//tyx:����·��


/** \brief
 *  ʹ��A*�㷨ʱ��Ҫ����Ľṹ��
 */
struct as_colInfo
{
	int         m_iMapNum;//tyx:��ͼ����Ŀ

	//! �����洢���Ǵ���ĵ�ͼ����������ϵ�е�������
	BaseMesh              m_arrTGIdx[MAX_TGNUM];//tyx:��ͼ����

	//! ��ײ��Ϣ��ָ�롣
	const os_TileCollisionInfo*    m_arrColInfo[MAX_TGNUM];


	//! �洢���صĽ��·�����ݡ�
	PATH*             m_resultPtr;

	//! �����·���ռ�������Ŀ��
	int                     m_iMaxRes;

};



/** \brief
 *  astar�㷨�ķ�װ�ࡣ
 *
 * add by honghaier @2003-12-18
 *
 * Mod by tyx @ 2004-3-20
 *
 * ��װ��һ��Astar�ࡣͨ��ʵ���������ʹ��Astar�㷨
 * ����·ȡ�ú�Ĵ����������ϵ�printpath(int *path)�����
 */
class fd_AstarBase
{
private:

	//! Open close���õ���������.
	static CVList  m_vecList;
	//! ��ʼ���xy���꣬
	int   m_iSX,m_iSY;
	//! ������xy���ꡣ
	int   m_iEX,m_iEY;

	//! ��¼��ǰ��ͼ�ķ�Χ��
	int   m_iMaxXNum,m_iMinXNum;
	int   m_iMaxYNum,m_iMinYNum;

	//! ��ͼ��Ⱥ͸߶ȡ�
	int   m_iMapWidth;
	int   m_iMapHeight;

	//! ��ǰ��ͼ�����ݡ�
	const os_TileCollisionInfo*    m_colMapInfo[MAX_TGNUM];
	//!�򿪵�ͼ����Ŀ
	int                    m_iMapNum;

	//! �����洢���Ǵ���ĵ�ͼ����������ϵ�е�������
	BaseMesh   m_arrTGIdx[MAX_TGNUM];//tyx:�����ͼ������ֵ
	
	//! ��֤ȫ��ֻ��һ��fd_AstarBase����.
	static fd_AstarBase*   m_instance;

	//! һ��·��(���ô˱�����Ŀ����Ϊ����߳���������ٶ��Լ������ܵļ����ڴ���Ƭ)
	Link m_sTmp;

private:

	/** \brief
	 *  ���ڵõ���ǰ��ͼ�����Ƿ����ͨ���ĺ�����
	 *
	 *  ��������Ƕ�̬��ͼ�ĺ��ġ�����ά����һ������ĵ�ͼ��
	 */
	bool        is_tilePass( int _x,int _y );


	/** \brief
	 *  ����������
	 */
	int         estimate_value( int _x,int _y );

	/** \brief
	*  ���뵱ǰ��㵽open��
	*
	*  \Link _link ��open���в����·��
	*  \return Link* �����²���open���е�·���ָ��.
	*/
	Link		*add_toOpen(const Link _link);

	/** \brief
	*  ��Open���еõ���ѽ�㡣
	*
	*  \return Link* ������ѽ����open���е�λ��ָ�롣
	*/
	Link		*get_bestElement( void );

	/** \brief
	 *  
	 */
	//! ����x����
	int         tile_x( int _num )          { return _num%(m_iMapWidth)+m_iMinXNum; }
	//! ����y����
	int         tile_y( int _num )          { return _num/(m_iMapWidth)+m_iMinYNum; }
	//! ��xy���귵����������
	int         tile_xy( int _x,int _y )     { return (_y-m_iMinYNum)*(m_iMapWidth)+(_x-m_iMinXNum); }

	/** \brief
	*  �������Խ���Ƿ������OPEN��, �������Բ���open��Ľڵ����open��(������)
	*
	*  \param x�������x����
	*  \param y�������y����(3d����ϵ��ʵ��Ϊz)
	*  \param f������Ĺ���ֵ
	*  \param _parent��������������һ����(��ָ��)
	*  \return Bool �����Ƿ���Բ���open��(�ڳ�����û��ʵ����;)
	*/
	bool        try_tile(int _x,int _y,int _f, Link *_parent);

	/** \brief
	 *  ����A*�㷨Ϊ��ʼ״̬��
	 */
	void        reset_aStar( void );

	/** \brief
	 *  ����ֵ��ĵ�ͼ���ݡ�
	 */
	void        process_mapData( as_colInfo* _colInfo );


	/** \brief
	 *  ͳ�ƣ�������������·���������·��ֵ��
	 */
	int         get_path( as_colInfo* _colInfo );

public:

	//! ��֤ȫ��ֻ��һ��Astarʵ������.
	fd_AstarBase();
	~fd_AstarBase();

	/** \brief 
	 *  �õ�һ��fd_AstarBase��Instanceָ��.
	 */
	static fd_AstarBase*    Instance();
	
	/** \brief
	 *  
	 *  ɾ��һ��fd_AstarBase��Instanceָ��.
	 */
	static void            DInstance();

	/** \brief
	 *  ���������������·�� ���õ������
	 *
	 *  \return int ���ش�Դ�㵽Ŀ�����Ҫ�೤��·���������-1,�����ҵ�·��
	 */
	int         find_path( int _sx, int _sy, int _dx, int _dy, as_colInfo* _colInfo );
	//! ·��
	Link		*m_pPath;
};

