//////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: sgLru.cpp
 *
 *  His:      River created @ 12/27 2003.
 *
 *  Desc:     least-recently Used �ڴ���̭�㷨�ͻ���.���ڶ�̬�ķ�������ͷ�ϵͳ
 *            ��Դ.
 *            �ļ���Ҳ����������С�Ĺ����ࡣ
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



//! �� ����һ���µ�ref.
int os_refObj::add_ref( void )
{
	m_iRefNum ++;
	return m_iRefNum;
}

//!    ��ȥһ��ref.
int os_refObj::release_ref( DWORD _ptr/* = NULL*/ )
{
	guard;

	// River mod @ 2009-7-20: �˴�Ӧ���޸�Ϊ����refС���㣬ȷ��bug�����޸�
	m_iRefNum --;

	// �����ǰ�ļ���Ϊ��Ļ������ؿա�
	if( m_iRefNum == 0 )
		release_obj( _ptr );

	osassert( m_iRefNum >= 0 );

	return m_iRefNum;

	unguard;
}

//!    �õ���ǰref����ֵ��
int os_refObj::get_ref( void )
{
	return m_iRefNum;
}


//! lur����Ľ���Ԫ��
os_lruEle::os_lruEle()
{
	m_iNotUsed = 0;
}

os_lruEle::~os_lruEle()
{
}


//! �����ж���֡����ʱ��û���õ���ǰ��Ԫ�ء�
void os_lruEle::add_nu( int _nu/* = 1*/ )
{
	m_iNotUsed += _nu;
}

//! �õ���ǰû��ʹ�õ�ʱ�����֡������
int os_lruEle::get_nu( void )
{
	return m_iNotUsed;
}

/** \brief
 *   ����ʹ��ʹ���˵�ǰ��Ԫ�ء�
 *
 *   ����ʹ�ø�ֵ����Ϊ���ֵʲô��������
 *   ɾ��Ԫ�ص�ʱ��ֻɾ�����ֵ�Ϳ����ˣ�������Сֵ��
 *   
 */
void os_lruEle::set_used( void )
{
	m_iNotUsed = -1;
}



//@{
//! ���ڶԵ�ǰ�ռ�������ڿ��õ����ݽṹ���б���
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

