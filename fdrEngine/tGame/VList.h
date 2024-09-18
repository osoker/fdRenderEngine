//=======================================================================
/** \file
* Filename:VList.h
*
* Desc: ����������,ר����A*Ѱ·�����ݴ���봦��
*
* His : tyx Created @  2004-3-19
*/
//========================================================================
#pragma once

//! ��������������ͷ�ļ�
#include <vector>

using namespace std;

/** \brief
 *	 ·��ṹ,���ڴ�� ��ͼ�е�·��
 */
struct Link
{
	//! ��һ��Ԫ��(·��)
	Link *prev;
	//! ��һ��Ԫ��(·��)
	Link *next;
	//! ��ǰԪ�������еĸ�Ԫ��
	Link *parent;
	//! ��ǰ·�������ֵ
	int   m_iValue;
	//! ��ǰ���ӡ�
	int   m_iCurIdx;
	////! ��һ���ĸ��ӡ�
	//int   m_iLastIdx;
};

/** \brief
* A*�㷨��open��close����,�������ݵĴ���봦��
*/
class CVList
{
public:
	//! ���캯��
	CVList(void);
	//! ��������
	~CVList(void);
private:
	//! �����������Ѿ������Ԫ�ظ���
	int count;
	//! open���в����Ԫ�ظ���
	int countOpen;
	//! ����������ݵ���������
	vector<Link> item;

	/** \brief
	* ��һ��Ԫ�ز������ݱ���
	*
	* \_newLink Ҫ�����Ԫ��
	* \return Link* �����²����Ԫ�������ݱ��еĵ�ַ
	*/
	Link *PushBack(const Link *_newLink);

public:
	//////////////////////////////////////////////////////
	/** \brief
	* �������ݱ��е�����Ԫ�ظ���
	*/
	void ReSize(int _size = 200);
	/** \brief
	* ���ݱ�����
	*/
	void ReSet(void);
	//////////////////////////////////////////////////////
	/** \brief
	* ȡ�����ݱ��е�Ԫ�ظ���
	*
	* \return int ����Ԫ�ظ���
	*/
	int GetCount() { return count; }
	/** \brief
	* ȡ��open���е�Ԫ�ظ���
	*
	* \return int ����open���е�Ԫ�ظ���
	*/
	int GetOpenCount() { return countOpen; }
	/**
	* ȡ�����ݱ�����ߴ�
	*
	*  \return int ���ؿ��Դ�����Ԫ�صĸ���
	*/
	int GetSize() { return (int)item.size()-2; }
	//////////////////////////////////////////////////////
	/** \brief
	* ȡ��open���ͷ��ַ
	*
	* \return Link* ����open��ͷ��ַ
	*/
	Link *GetOpenHead();
	/** \brief
	* ȡ��close���ͷ��ַ
	*
	* \return Link* ����close��ͷ��ַ
	*/
	Link *GetCloseHead();
	/** \brief
	* ��һ��Ԫ����ӵ�open����
	*
	* \_newLink Ҫ�����Ԫ�صĵ�ַָ��
	*
	* return Link* ���ظ�Ԫ�������ݱ��еĵ�ַָ��
	*/
	Link *AddToOpen(const Link *_newLink);
	/** \brief
	* ��open����ȡ��һ����õĵ�(����ֵ��С),���Ѹõ����close����
	*
	* \return Link* ���ظõ�ĵ�ַָ��
	*/
	Link *GetBestLink(void);
	//////////////////////////////////////////////////////
	/** \brief
	* �ж�һ��·���Ƿ���open����
	*
	* \_iIdx �����жϵ�·�����������
	* return Link* ���ظ�·�������ݱ��еĵ�ַָ��,��û�����ݱ��з���NULL
	*/
	Link *IsInOpen(int _iIdx);
	/** \brief
	* �ж�һ��·���Ƿ���close����
	*
	* \_iIdx �����жϵ�·�����������
	* return Link* ���ظ�·�������ݱ��еĵ�ַָ��,��û�����ݱ��з���NULL
	*/
	Link *IsInClose(int _iIdx);
};
