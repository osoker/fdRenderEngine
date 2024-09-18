//---------------------------------------------------------------------
  /** \file
   * Filename:newAstar.h
   *
   * Desc: 定义一个Astar类
   *
   * His : honghaier Created @  2003 12/18 9:19
   *
   * River comment @ 2003-2-4: 对最终的寻路长度进行限制，最多可以寻到离
   *               原点24个格子的物体。根据这个值优化最终的A*算法。速度
   *               优先。
   * tyx mod @ 2004-3-26 这次的变动比较大,重要函数全部重写,但保留以前的
   *			   计算过程:
   *          1. 新写一个容器链表类 统一管理计算中的路点数据,除去了程序中的一些不必要数据移动.
   *          2. 建议除去Popstack();部分 在算法修改后这个完全没有必要
   *          3. 建议除去//! 最多计算次数# define MAX_PROCESSTIME 1024*4 这里程序经修改后 
   *				已经没有多大的意义--由于新的open表数据限制,不可能有太多的循环次数,除去
					这个程序中的计算器可以或多或少的提高程序的速度.
   *          4. 建议除去//! 限制A*的最大寻路步数。# define MAX_PATHASTAR  48 因为在新的数据管理下可以修改
   *				open/close表的总的大小来控制寻路的最大步数.
   *          5. 修改OBLIQUE_VALUE 的值为16(原先的值为14),这样可以使得到的路径更接近最近路径.
   *          6. 对程序中的新生成的路点 用一个固定的变量m_sTmp 避免每次生成新路点时new一个新
   *			 变量,提高程序速度 并可以减少内存碎片
   *          7. 修改寻路的最终结果 当目标不可到达时,程序会计算出最近的路径.
   */
//---------------------------------------------------------------------
# pragma once

# include "../interface/osInterface.h" 

# include "math.h"
# include "VList.h"

//! 最多的地图数目。
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
	void resize(WORD _nSize = 200/*这个参数最好是个偶数*/)
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
//！寻路用到的单元格索引结构
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

#define PATH  VECTOR<BaseMesh>//tyx:定义路径


/** \brief
 *  使用A*算法时需要传入的结构。
 */
struct as_colInfo
{
	int         m_iMapNum;//tyx:地图的数目

	//! 用来存储我们传入的地图在世界坐标系中的索引。
	BaseMesh              m_arrTGIdx[MAX_TGNUM];//tyx:地图索引

	//! 碰撞信息的指针。
	const os_TileCollisionInfo*    m_arrColInfo[MAX_TGNUM];


	//! 存储返回的结果路径数据。
	PATH*             m_resultPtr;

	//! 传入的路径空间的最大数目。
	int                     m_iMaxRes;

};



/** \brief
 *  astar算法的封装类。
 *
 * add by honghaier @2003-12-18
 *
 * Mod by tyx @ 2004-3-20
 *
 * 封装了一个Astar类。通过实例化此类可使用Astar算法
 * 但道路取得后的处理函数由以上的printpath(int *path)来完成
 */
class fd_AstarBase
{
private:

	//! Open close表共用的向量链表.
	static CVList  m_vecList;
	//! 开始点的xy坐标，
	int   m_iSX,m_iSY;
	//! 结果点的xy坐标。
	int   m_iEX,m_iEY;

	//! 记录当前地图的范围。
	int   m_iMaxXNum,m_iMinXNum;
	int   m_iMaxYNum,m_iMinYNum;

	//! 地图宽度和高度。
	int   m_iMapWidth;
	int   m_iMapHeight;

	//! 当前地图的数据。
	const os_TileCollisionInfo*    m_colMapInfo[MAX_TGNUM];
	//!打开地图的数目
	int                    m_iMapNum;

	//! 用来存储我们传入的地图在世界坐标系中的索引。
	BaseMesh   m_arrTGIdx[MAX_TGNUM];//tyx:保存地图的索引值
	
	//! 保证全局只有一个fd_AstarBase出现.
	static fd_AstarBase*   m_instance;

	//! 一个路点(设置此变量的目的是为了提高程序的运算速度以及尽可能的减少内存碎片)
	Link m_sTmp;

private:

	/** \brief
	 *  用于得到当前地图坐标是否可以通过的函数。
	 *
	 *  这个函数是动态地图的核心。由他维护了一个虚拟的地图。
	 */
	bool        is_tilePass( int _x,int _y );


	/** \brief
	 *  评估函数。
	 */
	int         estimate_value( int _x,int _y );

	/** \brief
	*  加入当前结点到open表。
	*
	*  \Link _link 向open表中插入的路点
	*  \return Link* 返回新插入open表中的路点的指针.
	*/
	Link		*add_toOpen(const Link _link);

	/** \brief
	*  从Open表中得到最佳结点。
	*
	*  \return Link* 返回最佳结点在open表中的位置指针。
	*/
	Link		*get_bestElement( void );

	/** \brief
	 *  
	 */
	//! 返回x坐标
	int         tile_x( int _num )          { return _num%(m_iMapWidth)+m_iMinXNum; }
	//! 返回y坐标
	int         tile_y( int _num )          { return _num/(m_iMapWidth)+m_iMinYNum; }
	//! 由xy坐标返回坐标索引
	int         tile_xy( int _x,int _y )     { return (_y-m_iMinYNum)*(m_iMapWidth)+(_x-m_iMinXNum); }

	/** \brief
	*  用来测试结点是否可以入OPEN表, 并将可以插入open表的节点放入open表(并排序)
	*
	*  \param x代表结点的x坐标
	*  \param y代表结点的y坐标(3d坐标系中实际为z)
	*  \param f代表结点的估计值
	*  \param _parent代表结点所属的上一层结点(的指针)
	*  \return Bool 返回是否可以插入open表(在程序中没有实际用途)
	*/
	bool        try_tile(int _x,int _y,int _f, Link *_parent);

	/** \brief
	 *  重设A*算法为初始状态。
	 */
	void        reset_aStar( void );

	/** \brief
	 *  处理值入的地图数据。
	 */
	void        process_mapData( as_colInfo* _colInfo );


	/** \brief
	 *  统计Ａ＊计算出的最佳路径，并填充路径值。
	 */
	int         get_path( as_colInfo* _colInfo );

public:

	//! 保证全局只有一个Astar实例存在.
	fd_AstarBase();
	~fd_AstarBase();

	/** \brief 
	 *  得到一个fd_AstarBase的Instance指针.
	 */
	static fd_AstarBase*    Instance();
	
	/** \brief
	 *  
	 *  删除一个fd_AstarBase的Instance指针.
	 */
	static void            DInstance();

	/** \brief
	 *  调用这个函数计算路径 并得到结果。
	 *
	 *  \return int 返回从源点到目标点需要多长的路。如果返回-1,不能找到路。
	 */
	int         find_path( int _sx, int _sy, int _dx, int _dy, as_colInfo* _colInfo );
	//! 路径
	Link		*m_pPath;
};

