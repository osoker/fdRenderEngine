#pragma once
//===================================================================
/** \file  
*  Filename: EquipmentManager.h
*  Desc:     包含场景中可拾取装备的定义
*  His:      王凯 @ 1/16 2004 15:21
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
	//!地图中的装备
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

	//!装入全部的可用装备
	bool	Create(char* _pchDirName);

	//!释放资源
	void	Reset();

	//!根据传入的地图坐标和格子索引得到装备
	void	GetEquipment(int _nX, int _nY, int _x, int _y,
		os_equipment& _pEquip,s_string& _pstrName);
protected:
	//!装备列表
	std::map<s_string, os_equipment>   m_MAPequipment;

	//!可用的全部装备的数量
	int							m_nTotalCount;

	//!所有地图上的装备
	std::vector<FieldEquipList*>  m_VECFieldEquip;

};

/**brief
* !装入一个地图中的装备
*/
bool LoadEquipment(char *_chPath, FieldEquipList *_pEquipList);
