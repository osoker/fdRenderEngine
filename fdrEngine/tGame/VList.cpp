//=======================================================================
/** \file
* Filename:VList.cpp
*
* Desc: ����������,ר����A*Ѱ·�����ݴ���봦��
*
* His : tyx Created @  2004-3-19
*/
//========================================================================

#include "StdAfx.h"
//! ͷ�ļ�
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
* �������ݱ��е�����Ԫ�ظ���
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
* ȡ��open���ͷ��ַ
*
* \return Link* ����open��ͷ��ַ
*/
Link *CVList::GetOpenHead()
{
	 return &item[0];
}

/** \brief
* ȡ��close���ͷ��ַ
*
* \return Link* ����close��ͷ��ַ
*/
Link *CVList::GetCloseHead()
{
	return &item[1];
}

/** \brief
* ��һ��Ԫ�ز������ݱ���
*
* \_newLink Ҫ�����Ԫ��
* \return Link* �����²����Ԫ�������ݱ��еĵ�ַ
*/
Link *CVList::PushBack(const Link *_newLink)
{
	if( (count+1) > GetSize() )
		return NULL;//���Ԫ�س�������������Ŀ.

	count++;
	memcpy(&item[count+1], _newLink,sizeof(Link));
	item[count+1].next = NULL;
	item[count+1].prev = NULL;

	return &item[count+1];
}

/** \brief
* ��һ��Ԫ����ӵ�open����
*
* \_newLink Ҫ�����Ԫ�صĵ�ַָ��
*
* return Link* ���ظ�Ԫ�������ݱ��еĵ�ַָ��
*/
Link *CVList::AddToOpen(const Link *_newLink)
{
	//CString strMsg;
	//strMsg.Format("����(%d,%d)", _newLink->m_iCurIdx%100 , _newLink->m_iCurIdx/100 );
	//AfxMessageBox(strMsg);

	Link *t_pNew = PushBack(_newLink);
	Link *t_pTemp = GetOpenHead();

	if(t_pNew == NULL) return NULL;

	countOpen++;//open���е�Ԫ�ؼ�1

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

	//����ﵽopen��Ľ�β
	t_pNew->next = NULL;
	t_pNew->prev = t_pTemp;
	t_pTemp->next = t_pNew;
	return t_pNew;
}

/** \brief
* ��open����ȡ��һ����õĵ�(����ֵ��С),���Ѹõ����close����
*
* \return Link* ���ظõ�ĵ�ַָ��
*/
Link *CVList::GetBestLink(void)
{

	Link *t_pTemp = GetOpenHead()->next;

	if(!t_pTemp)//���open�����
		return NULL;

	//�����Ԫ�ط���close����
	countOpen--;//open���е�Ԫ����Ŀ��1.
	//��t_pTemp �������
	GetOpenHead()->next = t_pTemp->next;
	if(t_pTemp->next)
		t_pTemp->next->prev = GetOpenHead();

	//�ٰ�������Close����
	t_pTemp->prev = GetCloseHead();
	t_pTemp->next = GetCloseHead()->next;
	GetCloseHead()->next = t_pTemp;
	if(t_pTemp->next)
		t_pTemp->next->prev = t_pTemp;

	return t_pTemp;
}

/** \brief
* ���ݱ�����
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
* �ж�һ��·���Ƿ���open����
*
* \_iIdx �����жϵ�·�����������
* return Link* ���ظ�·�������ݱ��еĵ�ַָ��,��û�����ݱ��з���NULL
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
* �ж�һ��·���Ƿ���close����
*
* \_iIdx �����жϵ�·�����������
* return Link* ���ظ�·�������ݱ��еĵ�ַָ��,��û�����ݱ��з���NULL
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
