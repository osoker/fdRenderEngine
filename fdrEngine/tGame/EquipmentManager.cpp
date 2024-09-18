//===================================================================
/** \file  
*  Filename: EquipmentManager.cpp
*  Desc:     ���������п�ʰȡװ���Ķ���
*  His:      ���� @ 1/16 2004 15:21
*/
//===================================================================

# include "stdafx.h"
# include "tengine.h"
#include "dxutil.h"
#include <algorithm>
#include "cameractrl.h"
#include "equipmentmanager.h"

CEquipmentManager::CEquipmentManager(void)
{
}

CEquipmentManager::~CEquipmentManager(void)
{
	Reset();
}
/************************************************************************/
/*	!�ͷ���Դ
/************************************************************************/
void CEquipmentManager::Reset()
{
	for(WORD n = 0; n < m_VECFieldEquip.size(); n++)
		delete m_VECFieldEquip[n];
	m_VECFieldEquip.clear();
}

/**brief*****************************************************************/
/*!װ��ȫ���Ŀ���װ��
/************************************************************************/
bool CEquipmentManager::Create(char* _pchDirName)
{
	char t_Path[MAX_PATH];
	WIN32_FIND_DATA t_sfd;
	HANDLE  t_hFind = NULL;

	strcpy(t_Path, _pchDirName);
	strcat(t_Path, "\\*.dat");


	return true;
}


/**brief*****************************************************************/
/* !װ��һ����ͼ�е�װ��
/************************************************************************/
bool LoadEquipment(char *_chPath, FieldEquipList *_pEquipList)
{
	if(!_chPath || !_pEquipList)
		return false;
	CIniFile   t_iniFile;
	char       t_str[256];
	int		   t_nCount;

	return true;
}
/**brief*****************************************************************/
/*!���ݴ���ĵ�ͼ����͸��������õ�װ��
/************************************************************************/
void CEquipmentManager::GetEquipment(int _nX, int _nY, int _x, int _y,
									 os_equipment& _pEquip,s_string& _pstrName)
{
	for(WORD n = 0; n < m_VECFieldEquip.size(); n++)
	{
		if(m_VECFieldEquip[n]->m_VECEquip.size() <=0)
			continue;

		if(m_VECFieldEquip[n]->nX == _nX && m_VECFieldEquip[n]->nY == _nY)
		{
			std::vector<FieldEquip>::iterator t_it;
			FieldEquip t_Equip;
			t_Equip.nx = _x;
			t_Equip.ny = _y;

			t_it = std::find(m_VECFieldEquip[n]->m_VECEquip.begin(),
				m_VECFieldEquip[n]->m_VECEquip.end(),t_Equip);
			if(t_it != m_VECFieldEquip[n]->m_VECEquip.end())
			{				
				_pEquip = m_MAPequipment[t_it->m_strName];
				_pstrName = t_it->m_strName;
				m_VECFieldEquip[n]->m_VECEquip.erase(t_it);
			}
			return;
		}
	}
}

