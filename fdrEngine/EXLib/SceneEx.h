
//===================================================================
/** \file  
 *  Filename: SceneEX.h
 *  Desc:     装入EX文件和文件信息的类。
 *  His:      王凯 @ 6/12 2003 16:18
 */
//===================================================================
#pragma once

#include <stdio.h>
#include <vector>
#include <D3DX9Math.h>


# include "../interface/osInterface.h"





/** \brief
 *  把我们自己格式的tga文件转化我们需要的内存tga文件格式.
 *
 *  卡安完成的文件格式转化和*.HON文件处理.
 *  本来此函数在tgaLoad的lib库中,提到这个库中.
 */
int     load_tgaFile( const char* _szFileName,BOOL& _alpha,BYTE* _mem,DWORD _maxLength,int& _dOffset ,BOOL& _avoidMipMap);



/** \brief
 * 判断两个浮点数是否相等

 * \param1 _1: 
 * \param2 _2:
 *      要比较的两个浮点数
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
 *  Desc:     表示一个物体占用的单元格。
 *            所有顶点的y值全部为0
 *  \param m_bIsUsed:
 *            m_bIsUsed 表示是否占用。1是占用。0为不用
 */
//===================================================================

typedef struct basemesh{
	//!0,2顶点代表一个单元格.高度值不用.
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

	//!判断一个三角形是否在一个2D的单元格内
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
	//!判断一个射线是否穿过格子
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
 *  Desc:     表示一个三角形。
 *            
 */
//===================================================================

typedef struct {
	//!三角形的三个顶点,用来和地图网格做判断
	D3DXVECTOR3 m_sVer[3];
	//!判断一个2D网格是否在三角形中
	bool  Mesh2dInTri(BASE_MESH &_basemesh)
	{
		//!如果三角形的法线是和X轴相平行的，不做判断
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

		//!将三角形压缩到2D平面
		t_sVer[0].y = 0;
		t_sVer[1].y = 0;
		t_sVer[2].y = 0;
		//!判断格子的四个顶点是否在三角形内
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

///!定义ex文件头
typedef struct {
	//!数据头大小
	DWORD m_dwSize;
	//! .x文件在整个文件中的大小
	DWORD m_dwXSize;
	//! .x文件在整个文件中的位置
	DWORD m_dwPos;
}EX_HEADER;

//!定义物体信息
typedef struct {
	//!物体名称
	char   m_chName[32];
	//!面数量
	DWORD  m_dwNumFace;
	//!顶点数量
	DWORD  m_dwNumVers;
	//!物体的中心点
	D3DXVECTOR3 m_sCenter;
	//!透明色颜色
	DWORD       m_dwColor;
	//!透明度0 -- 255
	int         m_iAlpha;
	//!保存物体在八个方向上占有多少格子
	DWORD  m_dwMeshCount[8];
	//!保存格子在文件中数据块的大小
	DWORD  m_dwMeshSize[8];
	//!物体的纹理数量
	DWORD  m_dwTexCount;
	//!材质缓存的大小
	DWORD  m_dwTexNameSize;

}OBJECT_INFO;


struct obj_bound 
{
	//!物体的bounding box
	D3DXVECTOR3 		m_VECmin;
	D3DXVECTOR3		m_VECmax;
	//!物体的bounding sphere;
	D3DXVECTOR3		m_VECCenter;
	float			m_fRadius;
};


//! 对mesh内原来的vipObj BOOL型变量进行替换，在不改变文件格式的前提下，加入更多的信息
struct  os_meshVipInfo
{
	//! 原来的vip变量，不管多远，物品都在场景内显示。
	bool    m_bVipObj;

	//! River mod @ 2007-4-5:桥类的物品,需要做精确的碰撞检测.
	bool    m_bIsBridge;

	//! 是否受体积雾影响
	bool    m_AcceptFog;

	//! 对是树类的物品，渲染树类物品的透明部分时，不需要写入Z缓冲区
	bool    m_bTreeObj;
};

//===================================================================
/** \class
 *  classname:  CSceneEx
 *  Desc:     用来装入一个EX文件和它的信息文件。
 *
 *       BOOL LoadEXFile(char* _chFileName);
 *       这个函数会把一个X文件从EX文件中分离出来，数据保存在m_XData指针中。
 *       可以从m_sEXHeader.m_dwXSize来得到数据的字节大小
 *            
 */
//===================================================================

class CSceneEx
{
public:
	CSceneEx(void);
	~CSceneEx(void);

	//!释放资源
	void Reset();

	//!得到文件头结构
	EX_HEADER*  GetEXHeader(){return &m_sEXHeader;}
	BYTE *      GetXData(){ return m_XData;}


	//!得到bounding box
	obj_bound*  GetBounding()
	{
		return &m_sBound;
	}

	//!装入一个EX文件
	BOOL LoadEXFile(char* _chFileName,LPDIRECT3DDEVICE9 _dev,
		     LPD3DXBUFFER& _mtrlBuf,DWORD& _numMtl,LPD3DXMESH& _pmesh );

	//! 从内存中装入一个ex文件
	BOOL loadEXFromMem( BYTE* _mem );

	//{@ 2004.3.1 Add
	//!加入物体的bounding box 和 bounding sphere
	//@}

private:

	EX_HEADER     m_sEXHeader;
	BYTE          *m_XData;

	OBJECT_INFO   m_sObjectInfo;
	BASE_MESH     *m_pBaseMesh[8];

	//!物体的属性表
	D3DXATTRIBUTERANGE  *m_pAttribute;
	//!物体的材质信息
	char     **m_pTexName;

public:
	obj_bound  m_sBound;

	//@{
	//! River @ 2007-2-8:把这个大写的BOOL型值修改为四个小写bool的结构体，加入更多的信息
	//! 是否关键物品。
	//BOOL       m_bKeyObj;
	os_meshVipInfo   m_sMeshVipInfo;
	//@}


	//! 是否使用方向光做为全局光。
	BOOL       m_bUseDirLAsAmbient;

	//! 当前ex文件的版本号。
	DWORD      m_dwFileVersion;
};
