//---------------------------------------------------------------------
  /** \file
   * Filename:newAstar.cpp
   *
   * Desc: 定义一个Astar类
   *
   * His : honghaier Created @  2003 12/18 9:19
   *		tyx Mod @ 2004-3-20
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
# include "stdafx.h"

# include "tengine.h"
# include "dxutil.h"
# include "newAstar.h"

//! 数据表的容量 可以修改这个值来限定A算法的计算路点数目,运算次数,速度
# define INIT_VECSIZE   1024*2
# define PERMAP_SIZE        128

//! 正向移动的评估值增加数目。
# define DIRECT_VALUE       10

//! 斜向移动的评估值增加数目。
# define OBLIQUE_VALUE      16

CVList  fd_AstarBase::m_vecList;

//! 初始化static变量。
fd_AstarBase* fd_AstarBase::m_instance = NULL;

/** \brief
* A*算法类的构造函数
*/
fd_AstarBase::fd_AstarBase()
{
	m_vecList.ReSize(INIT_VECSIZE);//初始化数据表的尺寸
	m_pPath = NULL;
}

/** \brief
* A*算法类的析构函数
*/
fd_AstarBase::~fd_AstarBase()
{

}


/** \brief
* 生成一个类的实例指针.
*/
fd_AstarBase* fd_AstarBase::Instance()
{
	if( !m_instance )
		m_instance = new fd_AstarBase;		
	return m_instance;

}
/** \brief
* 删除一个实例对象指针.
*/
void fd_AstarBase::DInstance()
{
	if( m_instance )
		delete m_instance;
}


/** \brief
*  重设A*算法为初始状态。
*/
void fd_AstarBase::reset_aStar( void )
{
	//将数据表表重置
	m_vecList.ReSet();
}

/** \brief
*  评估函数。
*/
int fd_AstarBase::estimate_value( int _x,int _y )
{
	return abs(m_iEX-_x)+abs(m_iEY-_y);
}

/** \brief
*  加入当前结点到open表。
*/
Link *fd_AstarBase::add_toOpen(const Link _link)
{
	return m_vecList.AddToOpen(&_link);
}


/** \brief
*  从Open表中得到最佳结点,并且把该点放入close表中。
*
*  \param Link 返回最佳结点在open表中的地址指针。
*/
Link *fd_AstarBase::get_bestElement( void )
{
	return m_vecList.GetBestLink();
}

/** \brief
*  用来测试结点是否可以入OPEN表, 并将可以插入open表的节点放入open表(并排序)
*
*  \param x代表结点的x坐标
*  \param y代表结点的y坐标(3d坐标系中实际为z)
*  \param f代表结点的估计值
*  \param _parent代表结点所属的上一层结点(的指针)
*  \return Bool 返回是否可以插入open表(在程序中没有实际用途)
*/
bool fd_AstarBase::try_tile(int _x,int _y,int _f, Link *_parent)
{
	if( (_x<m_iMinXNum )||(_x>=m_iMaxXNum) )//超出当前地图的范围
		return false;
	if( (_y<m_iMinYNum )||(_y>=m_iMaxYNum) )
		return false;

	// 如果tile不可通过，返回，
	if( !is_tilePass( _x,_y ) )
		return false;

	int t_iTile = this->tile_xy( _x,_y );//(_x,_y)的数字坐标(坐标索引)

	Link *t_pOld;
	//如果在open表中
	if( (t_pOld=m_vecList.IsInOpen( t_iTile )) != NULL)
	{
		if(t_pOld->m_iValue > _f)//更新估计值
		{
			t_pOld->m_iValue = _f;
			t_pOld->parent = _parent;
		}
	}
	//如果在close表中
	else if( (t_pOld=m_vecList.IsInClose(t_iTile)) != NULL )
	{
		if(t_pOld->m_iValue > _f)//更新估计值 父节点
		{
			t_pOld->m_iValue = _f;
			t_pOld->parent = _parent;
		}

	}
	else//不在open表中 也不在close表中 添加新元素到open表中
	{
		m_sTmp.m_iValue = _f;
		m_sTmp.m_iCurIdx = t_iTile;
		m_sTmp.parent = _parent;

		if( !add_toOpen( m_sTmp ) )//加入到open表中
			return false;
	}
	return true;
}

/** \brief
*  用于得到当前地图坐标是否可以通过的函数。
*
*  这个函数是动态地图的核心。由他维护了一个虚拟的地图。
*　
*  地图中0为通过，1为不通过，所以使用非操作返回。
*/
bool fd_AstarBase::is_tilePass( int _x,int _y )
{
	int    t_x,t_y;

	//当前索引值所在的地图。
	int  t_mapIdx,t_mapIdy;
	t_mapIdx = _x/PERMAP_SIZE;
	t_mapIdy = _y/PERMAP_SIZE;

	const os_TileCollisionInfo*  t_ptrTI = NULL;//地图碰撞信息

	for( int t_i=0;t_i<m_iMapNum;t_i++ )
	{
		if( (m_arrTGIdx[t_i].nX == t_mapIdx)&&
			(m_arrTGIdx[t_i].nY == t_mapIdy) )
		{
			t_ptrTI = m_colMapInfo[t_i];//取得当前地图碰撞信息
			break;
		}
	}

	// 如果当前地图没有在内存中,则当前的路点不能通过.
	if( t_ptrTI == NULL )
		return false;

	//路点在其所在的地图上的坐标
	t_x = _x%PERMAP_SIZE;
	t_y = _y%PERMAP_SIZE;

	return !t_ptrTI[t_y*PERMAP_SIZE+t_x].m_bHinder;//返回能否通过的值
}



/** \brief
*  处理值入的地图数据。
*/
void fd_AstarBase::process_mapData( as_colInfo* _colInfo )
{
	int   t_x,t_y;
	//
	// 处理当前的地图数目,用于得到一个虚拟的大地图。
	m_iMapNum = _colInfo->m_iMapNum;//tyx:当前地图的数目
	memcpy( m_arrTGIdx,_colInfo->m_arrTGIdx,sizeof( BaseMesh )*m_iMapNum );
	for( int t_i=0;t_i<m_iMapNum;t_i++ )
		this->m_colMapInfo[t_i] = _colInfo->m_arrColInfo[t_i];
	//
	// 得到当前地图的最大值和最小值。
	this->m_iMaxXNum = -1,this->m_iMaxYNum = -1;
	this->m_iMinXNum = 10000,this->m_iMinYNum = 10000;
	for( int t_i=0;t_i<_colInfo->m_iMapNum;t_i++ )
	{
		t_x = _colInfo->m_arrTGIdx[t_i].nX*PERMAP_SIZE;
		if( t_x < this->m_iMinXNum )
			m_iMinXNum = t_x;
		t_x = (_colInfo->m_arrTGIdx[t_i].nX+1)*PERMAP_SIZE;
		if( t_x > this->m_iMaxXNum )
			m_iMaxXNum = t_x;

		t_y = _colInfo->m_arrTGIdx[t_i].nY*PERMAP_SIZE;
		if( t_y < this->m_iMinYNum )
			m_iMinYNum = t_y;
		t_y = (_colInfo->m_arrTGIdx[t_i].nY+1)*PERMAP_SIZE;
		if( t_y > this->m_iMaxYNum )
			m_iMaxYNum = t_y;
	}
	//
	// 得到当前地图的宽度。
	this->m_iMapWidth = this->m_iMaxXNum-this->m_iMinXNum;
	this->m_iMapHeight = this->m_iMaxYNum-this->m_iMinYNum;

	return;
}


/** \brief
*  统计Ａ＊计算出的最佳路径，并填充路径值。
*
*  \return int 如果返回-1，则找不到路径。
*/
int fd_AstarBase::get_path( as_colInfo* _colInfo )
{
	PATH* t_pathPtr;//定义要返回的路径的指针
	t_pathPtr = _colInfo->m_resultPtr;

	int t_pathIdx = 0;//路点的个数

	BaseMesh t_mesh;
	while (m_pPath) 
	{
		if(!m_pPath->parent) //计算到开始点(开始点没有父点)
			break;
		t_mesh.nX = tile_x(m_pPath->m_iCurIdx);
		t_mesh.nY = tile_y(m_pPath->m_iCurIdx);
		t_pathPtr->push_front(t_mesh);

		t_pathIdx++;//路点个数加一
		m_pPath = m_pPath->parent;
	}

	return t_pathIdx;//tyx:返回路点数
}



/** \brief
*  调用这个函数计算路径 并得到结果。
*
*  \return int 返回从源点到目标点需要多长的路。如果返回-1,不能找到路。
*/
int fd_AstarBase::find_path( int _sx, int _sy, int _dx, int _dy, as_colInfo* _colInfo )
{
	int        t_idx,t_x,t_y;

	this->m_iSX = _sx;
	this->m_iSY = _sy;
	this->m_iEX = _dx;
	this->m_iEY = _dy;

	m_pPath = NULL;

	// 开始查找最佳路径。
	reset_aStar();//tyx:初始化

	// 处理传入的地图数据。
	process_mapData( _colInfo );

	Link t_sLink;
	//转换为虚拟的坐标--然后再得到索引
	t_sLink.m_iCurIdx = tile_xy(m_iSX, m_iSY);
	t_sLink.m_iValue = estimate_value(m_iSX, m_iSY);
	t_sLink.parent = NULL;
	add_toOpen(t_sLink);

	// 循环使用open表。
	int   t_iProcessNum = 0;
	Link	*t_pBextLink = NULL;
	while( 1 ) 
	{
		t_pBextLink = get_bestElement();//取出估计值最好的点并放入close表中

		//如果open表空了
		if(!t_pBextLink)
		{
			//在close表中找个最近的点
			Link *t_pTemp = m_pPath = m_vecList.GetCloseHead()->next;
			int t_iDistance = estimate_value(tile_x(m_pPath->m_iCurIdx), tile_y(m_pPath->m_iCurIdx));
			while(t_pTemp->next)
			{
				t_x = tile_x(t_pTemp->next->m_iCurIdx);
				t_y = tile_y(t_pTemp->next->m_iCurIdx);
				if( estimate_value(t_x, t_y) < t_iDistance )
				{
					m_pPath = t_pTemp;
					t_iDistance = estimate_value(tile_x(m_pPath->m_iCurIdx), tile_y(m_pPath->m_iCurIdx));
				}
				t_pTemp = t_pTemp->next;
			}
			break;
		}
		//

		t_x = tile_x( t_pBextLink->m_iCurIdx );
		t_y = tile_y( t_pBextLink->m_iCurIdx );

		//!如果已经到达目标结点退出
 		if( (t_x == this->m_iEX)&&(t_y == this->m_iEY) )////tyx:达到目标
		{
			m_pPath = t_pBextLink;
			break;
		}
		////尝试向上移动
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x, t_y+1);
		try_tile( t_x, t_y+1, t_idx, t_pBextLink );
		//// 尝试向下移动
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x, t_y-1);
		try_tile( t_x, t_y-1, t_idx, t_pBextLink );
		//// 尝试向左移动
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x-1, t_y);
		try_tile( t_x-1, t_y, t_idx, t_pBextLink );
		//// 尝试向右移动
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x+1, t_y);
		try_tile( t_x+1, t_y, t_idx, t_pBextLink );
		//// 尝试向右上移动
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x+1,t_y+1); 
		try_tile( t_x+1,t_y+1,t_idx, t_pBextLink );
		//// 尝试向右下移动
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x+1,t_y-1); 
		try_tile( t_x+1,t_y-1,t_idx, t_pBextLink );
		//// 尝试向左下移动
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x-1,t_y-1); 
		try_tile( t_x-1,t_y-1,t_idx, t_pBextLink );
		//// 尝试向左上移动
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x-1,t_y+1); 
		try_tile( t_x-1,t_y+1,t_idx, t_pBextLink );

	};//end while

	// 统计当前的路径有多少步，并返回。
	return get_path( _colInfo );
}
