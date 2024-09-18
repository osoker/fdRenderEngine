
//===================================================================
/** \file  
 *  Filename: SceneEX.h
 *  Desc:     װ��EX�ļ����ļ���Ϣ���ࡣ
 *  His:      ���� @ 6/12 2003 16:18
 */
//===================================================================
#pragma once

#include <stdio.h>
#include <vector>
#include <D3DX9Math.h>


# include "../interface/osInterface.h"





/** \brief
 *  �������Լ���ʽ��tga�ļ�ת��������Ҫ���ڴ�tga�ļ���ʽ.
 *
 *  ������ɵ��ļ���ʽת����*.HON�ļ�����.
 *  �����˺�����tgaLoad��lib����,�ᵽ�������.
 */
int     load_tgaFile( const char* _szFileName,BOOL& _alpha,BYTE* _mem,DWORD _maxLength,int& _dOffset ,BOOL& _avoidMipMap);



/** \brief
 * �ж������������Ƿ����

 * \param1 _1: 
 * \param2 _2:
 *      Ҫ�Ƚϵ�����������
 *
 */
//extern bool   fEqual(  float _1, float _2  );
inline bool   fEqual(  float _1, float _2  )
{
	return( fabs( _1 - _2 ) < 0.0001f )?true:false;
}

//===================================================================
/** \struct
 *  structname: basemesh
 *  Desc:     ��ʾһ������ռ�õĵ�Ԫ��
 *            ���ж����yֵȫ��Ϊ0
 *  \param m_bIsUsed:
 *            m_bIsUsed ��ʾ�Ƿ�ռ�á�1��ռ�á�0Ϊ����
 */
//===================================================================

typedef struct basemesh{
	//!0,2�������һ����Ԫ��.�߶�ֵ����.
	D3DXVECTOR3  m_sVer2d[4];
	BOOL         m_bIsUsed;
	float        m_fHeight;

	BOOL operator == ( CONST basemesh& tes )
	{
		if((m_sVer2d[0] == tes.m_sVer2d[0])&&(m_sVer2d[2] == tes.m_sVer2d[2]))
			return TRUE;
		return FALSE;
	}
	void operator = ( CONST basemesh& tes )
	{
		m_sVer2d[0] = tes.m_sVer2d[0];
		m_sVer2d[1] = tes.m_sVer2d[1];
		m_sVer2d[2] = tes.m_sVer2d[2];
		m_sVer2d[3] = tes.m_sVer2d[3];
		m_bIsUsed = tes.m_bIsUsed;
		m_fHeight = tes.m_fHeight;
	}

	//!�ж�һ���������Ƿ���һ��2D�ĵ�Ԫ����
	bool PointInMesh(D3DXVECTOR3 *_sPoint)
	{
		for(int n = 0; n < 3; n++)
		{
			if((_sPoint[n].x > m_sVer2d[0].x) && (_sPoint[n].z > m_sVer2d[0].z)
				&& (_sPoint[n].x < m_sVer2d[2].x) && (_sPoint[n].z < m_sVer2d[2].z))
			{
				return true;
			 }
		}
		return false;
	}
	//!�ж�һ�������Ƿ񴩹�����
	bool RayInMesh(D3DXVECTOR3 *vRayPos, D3DXVECTOR3 *vRayDir)
	{
		if(D3DXIntersectTri(&m_sVer2d[0],&m_sVer2d[1],&m_sVer2d[2],
			vRayPos, vRayDir,NULL,NULL,NULL) || 
			D3DXIntersectTri(&m_sVer2d[0],&m_sVer2d[2],&m_sVer2d[3],
			vRayPos, vRayDir,NULL,NULL,NULL))
			return true;
		return false;
	}
	void FindHeight(D3DXVECTOR3 &vVer)
	{
		if((vVer.x > m_sVer2d[0].x) && (vVer.z > m_sVer2d[0].z)
				&& (vVer.x < m_sVer2d[2].x) && (vVer.z < m_sVer2d[2].z))
		{
			if(vVer.y > m_fHeight)
				m_fHeight = vVer.y;
			return;
		}
	}
}BASE_MESH;



//===================================================================
/** \struct
 *  structname: XFACE
 *  Desc:     ��ʾһ�������Ρ�
 *            
 */
//===================================================================

typedef struct {
	//!�����ε���������,�����͵�ͼ�������ж�
	D3DXVECTOR3 m_sVer[3];
	//!�ж�һ��2D�����Ƿ�����������
	bool  Mesh2dInTri(BASE_MESH &_basemesh)
	{
		//!��������εķ����Ǻ�X����ƽ�еģ������ж�
		if((fEqual(  m_sVer[0].x, m_sVer[1].x  ) && fEqual(  m_sVer[1].x, m_sVer[2].x  ))
			||(fEqual(  m_sVer[0].z, m_sVer[1].z  ) && fEqual(  m_sVer[1].z, m_sVer[2].z  )))
			return false;
	//	if((m_sVer[0].x == m_sVer[1].x == m_sVer[2].x)||
	//		(m_sVer[0].z == m_sVer[1].z == m_sVer[2].z))
	//		return false;
		D3DXVECTOR3 t_sVer[3];
		t_sVer[0] = m_sVer[0];
		t_sVer[1] = m_sVer[1];
		t_sVer[2] = m_sVer[2];

		//!��������ѹ����2Dƽ��
		t_sVer[0].y = 0;
		t_sVer[1].y = 0;
		t_sVer[2].y = 0;
		//!�жϸ��ӵ��ĸ������Ƿ�����������
		for(int n = 0; n < 4; n++)
		{
			D3DXVECTOR3 temp(_basemesh.m_sVer2d[n].x, 0, _basemesh.m_sVer2d[n].z);
			if(D3DXIntersectTri(&m_sVer[0],&m_sVer[1],&m_sVer[2],
				&temp, &D3DXVECTOR3(0,1,0),NULL,NULL,NULL))
			{
				return true;
			}
		}
		
		return false;
	}
}XFACE;

///!����ex�ļ�ͷ
typedef struct {
	//!����ͷ��С
	DWORD m_dwSize;
	//! .x�ļ��������ļ��еĴ�С
	DWORD m_dwXSize;
	//! .x�ļ��������ļ��е�λ��
	DWORD m_dwPos;
}EX_HEADER;

//!����������Ϣ
typedef struct {
	//!��������
	char   m_chName[32];
	//!������
	DWORD  m_dwNumFace;
	//!��������
	DWORD  m_dwNumVers;
	//!��������ĵ�
	D3DXVECTOR3 m_sCenter;
	//!͸��ɫ��ɫ
	DWORD       m_dwColor;
	//!͸����0 -- 255
	int         m_iAlpha;
	//!���������ڰ˸�������ռ�ж��ٸ���
	DWORD  m_dwMeshCount[8];
	//!����������ļ������ݿ�Ĵ�С
	DWORD  m_dwMeshSize[8];
	//!�������������
	DWORD  m_dwTexCount;
	//!���ʻ���Ĵ�С
	DWORD  m_dwTexNameSize;

}OBJECT_INFO;


struct obj_bound 
{
	//!�����bounding box
	D3DXVECTOR3 		m_VECmin;
	D3DXVECTOR3		m_VECmax;
	//!�����bounding sphere;
	D3DXVECTOR3		m_VECCenter;
	float			m_fRadius;
};


//! ��mesh��ԭ����vipObj BOOL�ͱ��������滻���ڲ��ı��ļ���ʽ��ǰ���£�����������Ϣ
struct  os_meshVipInfo
{
	//! ԭ����vip���������ܶ�Զ����Ʒ���ڳ�������ʾ��
	bool    m_bVipObj;

	//! River mod @ 2007-4-5:�������Ʒ,��Ҫ����ȷ����ײ���.
	bool    m_bIsBridge;

	//! �Ƿ��������Ӱ��
	bool    m_AcceptFog;

	//! �����������Ʒ����Ⱦ������Ʒ��͸������ʱ������Ҫд��Z������
	bool    m_bTreeObj;
};

//===================================================================
/** \class
 *  classname:  CSceneEx
 *  Desc:     ����װ��һ��EX�ļ���������Ϣ�ļ���
 *
 *       BOOL LoadEXFile(char* _chFileName);
 *       ����������һ��X�ļ���EX�ļ��з�����������ݱ�����m_XDataָ���С�
 *       ���Դ�m_sEXHeader.m_dwXSize���õ����ݵ��ֽڴ�С
 *            
 */
//===================================================================

class CSceneEx
{
public:
	CSceneEx(void);
	~CSceneEx(void);

	//!�ͷ���Դ
	void Reset();

	//!�õ��ļ�ͷ�ṹ
	EX_HEADER*  GetEXHeader(){return &m_sEXHeader;}
	BYTE *      GetXData(){ return m_XData;}


	//!�õ�bounding box
	obj_bound*  GetBounding()
	{
		return &m_sBound;
	}

	//!װ��һ��EX�ļ�
	BOOL LoadEXFile(char* _chFileName,LPDIRECT3DDEVICE9 _dev,
		     LPD3DXBUFFER& _mtrlBuf,DWORD& _numMtl,LPD3DXMESH& _pmesh );

	//! ���ڴ���װ��һ��ex�ļ�
	BOOL loadEXFromMem( BYTE* _mem );

	//{@ 2004.3.1 Add
	//!���������bounding box �� bounding sphere
	//@}

private:

	EX_HEADER     m_sEXHeader;
	BYTE          *m_XData;

	OBJECT_INFO   m_sObjectInfo;
	BASE_MESH     *m_pBaseMesh[8];

	//!��������Ա�
	D3DXATTRIBUTERANGE  *m_pAttribute;
	//!����Ĳ�����Ϣ
	char     **m_pTexName;

public:
	obj_bound  m_sBound;

	//@{
	//! River @ 2007-2-8:�������д��BOOL��ֵ�޸�Ϊ�ĸ�Сдbool�Ľṹ�壬����������Ϣ
	//! �Ƿ�ؼ���Ʒ��
	//BOOL       m_bKeyObj;
	os_meshVipInfo   m_sMeshVipInfo;
	//@}


	//! �Ƿ�ʹ�÷������Ϊȫ�ֹ⡣
	BOOL       m_bUseDirLAsAmbient;

	//! ��ǰex�ļ��İ汾�š�
	DWORD      m_dwFileVersion;
};
