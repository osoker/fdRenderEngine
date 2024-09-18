//////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: sgLru.cpp
 *
 *  His:      River created @ 12/27 2003.
 *
 *  Desc:     least-recently Used 内存淘汰算法和基类.用于动态的分配和秋释放系统
 *            资源.
 *            文件中也包括了其它小的工具类。
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/sgLru.h"
# include "../include/sgDebug.h"

os_refObj::os_refObj()
{
	this->m_iRefNum = 0;
}



//! 　 加入一个新的ref.
int os_refObj::add_ref( void )
{
	m_iRefNum ++;
	return m_iRefNum;
}

//!    减去一个ref.
int os_refObj::release_ref( DWORD _ptr/* = NULL*/ )
{
	guard;

	// River mod @ 2009-7-20: 此处应该修改为不能ref小于零，确认bug后再修改
	m_iRefNum --;

	// 如果当前的计数为零的话，返回空。
	if( m_iRefNum == 0 )
		release_obj( _ptr );

	osassert( m_iRefNum >= 0 );

	return m_iRefNum;

	unguard;
}

//!    得到当前ref的数值。
int os_refObj::get_ref( void )
{
	return m_iRefNum;
}


//! lur基类的建构元。
os_lruEle::os_lruEle()
{
	m_iNotUsed = 0;
}

os_lruEle::~os_lruEle()
{
}


//! 设置有多少帧或是时间没有用到当前的元素。
void os_lruEle::add_nu( int _nu/* = 1*/ )
{
	m_iNotUsed += _nu;
}

//! 得到当前没有使用的时间或是帧数。　
int os_lruEle::get_nu( void )
{
	return m_iNotUsed;
}

/** \brief
 *   设置使用使用了当前的元素。
 *
 *   可以使用负值，因为这个值什么都不代表。
 *   删除元素的时候只删除最大值就可以了，不管最小值。
 *   
 */
void os_lruEle::set_used( void )
{
	m_iNotUsed = -1;
}



//@{
//! 用于对当前空间管理器内可用的数据结构进行遍历
CSpaceMgr<class T>::NODE CSpaceMgr<class T>::begin_usedNode( void )
{
	int   t_i;
	NODE  t_n;

	for( t_i=0;t_i<(int)m_use.size();t_i ++ )
	{
		if( NULL != m_use[t_i] )
		{
			t_n.idx = t_i;
			t_n.p = m_use[t_i];
			return t_n;
		}
	}

	t_n.idx = -1;
	t_n.p = NULL;

	return t_n;
}

CSpaceMgr<class T>::NODE CSpaceMgr<T>::next_validNode( NODE* _node )
{
	int t_i;
	NODE  t_n;

	for( t_i=_node->idx+1;t_i<(int)m_use.size();t_i ++ )
	{
		if( NULL != m_use[t_i] )
		{
			t_n.idx = t_i;
			t_n.p = m_use[t_i];
			return t_n;
		}
	}
	t_n.idx = -1;
	t_n.p = NULL;

	return t_n;
}
//@} 

