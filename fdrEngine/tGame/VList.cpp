//=======================================================================
/** \file
* Filename:VList.cpp
*
* Desc: 向量链表类,专用于A*寻路的数据存放与处理
*
* His : tyx Created @  2004-3-19
*/
//========================================================================

#include "StdAfx.h"
//! 头文件
#include "vlist.h"


CVList::CVList(void)
{
	count = 0;
	countOpen =0;
}

CVList::~CVList(void)
{
	item.clear();
}

/** \brief
* 设置数据表中的最多的元素个数
*/
void CVList::ReSize(int _size/* = 200*/)
{
	item.resize(_size+2);

	item[0].next = NULL;
	item[0].prev = NULL;
	item[1].next = NULL;
	item[1].next = NULL;
}

/** brief
* 取得open表的头地址
*
* \return Link* 返回open的头地址
*/
Link *CVList::GetOpenHead()
{
	 return &item[0];
}

/** \brief
* 取得close表的头地址
*
* \return Link* 返回close的头地址
*/
Link *CVList::GetCloseHead()
{
	return &item[1];
}

/** \brief
* 将一个元素插入数据表中
*
* \_newLink 要插入的元素
* \return Link* 返回新插入的元素在数据表中的地址
*/
Link *CVList::PushBack(const Link *_newLink)
{
	if( (count+1) > GetSize() )
		return NULL;//如果元素超过数组的最大数目.

	count++;
	memcpy(&item[count+1], _newLink,sizeof(Link));
	item[count+1].next = NULL;
	item[count+1].prev = NULL;

	return &item[count+1];
}

/** \brief
* 把一个元素添加到open表中
*
* \_newLink 要插入的元素的地址指针
*
* return Link* 返回该元素在数据表中的地址指针
*/
Link *CVList::AddToOpen(const Link *_newLink)
{
	//CString strMsg;
	//strMsg.Format("放入(%d,%d)", _newLink->m_iCurIdx%100 , _newLink->m_iCurIdx/100 );
	//AfxMessageBox(strMsg);

	Link *t_pNew = PushBack(_newLink);
	Link *t_pTemp = GetOpenHead();

	if(t_pNew == NULL) return NULL;

	countOpen++;//open表中的元素加1

	while(t_pTemp->next)
	{
		if(t_pTemp->next->m_iValue  > t_pNew->m_iValue)
		{
			t_pNew->prev = t_pTemp;
			t_pNew->next = t_pTemp->next;
			t_pTemp->next->prev = t_pNew;
			t_pTemp->next = t_pNew;
			return t_pNew;
		}
		t_pTemp = t_pTemp->next;
	}

	//如果达到open表的结尾
	t_pNew->next = NULL;
	t_pNew->prev = t_pTemp;
	t_pTemp->next = t_pNew;
	return t_pNew;
}

/** \brief
* 从open表中取得一个最好的点(估计值最小),并把该点放入close表中
*
* \return Link* 返回该点的地址指针
*/
Link *CVList::GetBestLink(void)
{

	Link *t_pTemp = GetOpenHead()->next;

	if(!t_pTemp)//如果open表空了
		return NULL;

	//如果有元素放入close表中
	countOpen--;//open表中的元素数目减1.
	//把t_pTemp 分离出来
	GetOpenHead()->next = t_pTemp->next;
	if(t_pTemp->next)
		t_pTemp->next->prev = GetOpenHead();

	//再把它放入Close表中
	t_pTemp->prev = GetCloseHead();
	t_pTemp->next = GetCloseHead()->next;
	GetCloseHead()->next = t_pTemp;
	if(t_pTemp->next)
		t_pTemp->next->prev = t_pTemp;

	return t_pTemp;
}

/** \brief
* 数据表重置
*/
void CVList::ReSet(void)
{
	item[0].next = NULL;
	item[0].prev = NULL;
	item[1].next = NULL;
	item[1].next = NULL;
	count = 0;
	countOpen = 0;
}

/** \brief
* 判断一个路点是否在open表中
*
* \_iIdx 用来判断的路点的索引坐标
* return Link* 返回该路点在数据表中的地址指针,如没在数据表中返回NULL
*/
Link *CVList::IsInOpen(int _iIdx)
{
	Link *t_pTemp = GetOpenHead();
	while(t_pTemp->next)
	{
		if(t_pTemp->next->m_iCurIdx == _iIdx)
			return t_pTemp->next;
		t_pTemp = t_pTemp->next;
	}
	return NULL;
}

/** \brief
* 判断一个路点是否在close表中
*
* \_iIdx 用来判断的路点的索引坐标
* return Link* 返回该路点在数据表中的地址指针,如没在数据表中返回NULL
*/
Link *CVList::IsInClose(int _iIdx)
{
	Link *t_pTemp = GetCloseHead();
	int t=0; 
	while(t_pTemp->next)
	{
		if(_iIdx == t_pTemp->next->m_iCurIdx)
			return t_pTemp->next;
		t_pTemp = t_pTemp->next;
	}
	return NULL;
}



/*

*/
