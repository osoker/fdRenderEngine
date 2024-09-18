#pragma once
//===================================================================
/** \file  
*  Filename: EquipmentManager.h
*  Desc:     ���������п�ʰȡװ���Ķ���
*  His:      ���� @ 1/16 2004 15:21
*/
//===================================================================


struct FieldEquip
{
	int nx;
	int ny;
	s_string  m_strName;
	BOOL operator == ( CONST FieldEquip& t_Equip )
	{
		return (nx == t_Equip.nx && ny == t_Equip.ny);
	}
};
struct FieldEquipList
{
	int nX;
	int nY;
	//!��ͼ�е�װ��
	std::vector<FieldEquip>			m_VECEquip;

	BOOL operator == ( CONST FieldEquipList& t_list )
	{
		return (nX == t_list.nX && nY == t_list.nY);
	}
};

class CEquipmentManager
{
public:
	CEquipmentManager(void);
	~CEquipmentManager(void);

public:

	//!װ��ȫ���Ŀ���װ��
	bool	Create(char* _pchDirName);

	//!�ͷ���Դ
	void	Reset();

	//!���ݴ���ĵ�ͼ����͸��������õ�װ��
	void	GetEquipment(int _nX, int _nY, int _x, int _y,
		os_equipment& _pEquip,s_string& _pstrName);
protected:
	//!װ���б�
	std::map<s_string, os_equipment>   m_MAPequipment;

	//!���õ�ȫ��װ��������
	int							m_nTotalCount;

	//!���е�ͼ�ϵ�װ��
	std::vector<FieldEquipList*>  m_VECFieldEquip;

};

/**brief
* !װ��һ����ͼ�е�װ��
*/
bool LoadEquipment(char *_chPath, FieldEquipList *_pEquipList);
