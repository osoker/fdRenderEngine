//=======================================================================
/** \file
* Filename:VList.h
*
* Desc: 向量链表类,专用于A*寻路的数据存放与处理
*
* His : tyx Created @  2004-3-19
*/
//========================================================================
#pragma once

//! 包含向量容器的头文件
#include <vector>

using namespace std;

/** \brief
 *	 路点结构,用于存放 地图中的路点
 */
struct Link
{
	//! 上一个元素(路点)
	Link *prev;
	//! 下一个元素(路点)
	Link *next;
	//! 当前元素在树中的父元素
	Link *parent;
	//! 当前路点的评估值
	int   m_iValue;
	//! 当前格子。
	int   m_iCurIdx;
	////! 上一步的格子。
	//int   m_iLastIdx;
};

/** \brief
* A*算法的open表close表类,用于数据的存放与处理
*/
class CVList
{
public:
	//! 构造函数
	CVList(void);
	//! 析构函数
	~CVList(void);
private:
	//! 向量链表中已经插入的元素个数
	int count;
	//! open表中插入的元素个数
	int countOpen;
	//! 用来存放数据的向量容器
	vector<Link> item;

	/** \brief
	* 将一个元素插入数据表中
	*
	* \_newLink 要插入的元素
	* \return Link* 返回新插入的元素在数据表中的地址
	*/
	Link *PushBack(const Link *_newLink);

public:
	//////////////////////////////////////////////////////
	/** \brief
	* 设置数据表中的最多的元素个数
	*/
	void ReSize(int _size = 200);
	/** \brief
	* 数据表重置
	*/
	void ReSet(void);
	//////////////////////////////////////////////////////
	/** \brief
	* 取得数据表中的元素个数
	*
	* \return int 返回元素个数
	*/
	int GetCount() { return count; }
	/** \brief
	* 取得open表中的元素个数
	*
	* \return int 返回open表中的元素个数
	*/
	int GetOpenCount() { return countOpen; }
	/**
	* 取得数据表的最大尺寸
	*
	*  \return int 返回可以存放最多元素的个数
	*/
	int GetSize() { return (int)item.size()-2; }
	//////////////////////////////////////////////////////
	/** \brief
	* 取得open表的头地址
	*
	* \return Link* 返回open的头地址
	*/
	Link *GetOpenHead();
	/** \brief
	* 取得close表的头地址
	*
	* \return Link* 返回close的头地址
	*/
	Link *GetCloseHead();
	/** \brief
	* 把一个元素添加到open表中
	*
	* \_newLink 要插入的元素的地址指针
	*
	* return Link* 返回该元素在数据表中的地址指针
	*/
	Link *AddToOpen(const Link *_newLink);
	/** \brief
	* 从open表中取得一个最好的点(估计值最小),并把该点放入close表中
	*
	* \return Link* 返回该点的地址指针
	*/
	Link *GetBestLink(void);
	//////////////////////////////////////////////////////
	/** \brief
	* 判断一个路点是否在open表中
	*
	* \_iIdx 用来判断的路点的索引坐标
	* return Link* 返回该路点在数据表中的地址指针,如没在数据表中返回NULL
	*/
	Link *IsInOpen(int _iIdx);
	/** \brief
	* 判断一个路点是否在close表中
	*
	* \_iIdx 用来判断的路点的索引坐标
	* return Link* 返回该路点在数据表中的地址指针,如没在数据表中返回NULL
	*/
	Link *IsInClose(int _iIdx);
};
