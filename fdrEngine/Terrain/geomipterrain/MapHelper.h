//===================================================================
/** \file  
* Filename :   MapHelper.h
* Desc     :   
* His      :   Windy create @2006-1-7 10:31:10
*/
//===================================================================
#ifndef __MAPHELPER_H__
#define __MAPHELPER_H__
#include <string>
#include "../../../common/com_include.h"
int GetPowerTwo( int _pow );
void fixtexname(std::string &name,std::string _mapname);
/** ���ݸ߶����ݼ������ͼ
@par
	pfHeightmap [in]�߶������׵�ַ
	_iSize		[in]�߶����ݿ�Ⱥ͸߶�Ҳ�����ɵĹ���ͼ�Ŀ�ߡ�
	VecDir		[in]�������䷽��
	nAmbient	[in]������ǿ��?{0.0-1.0}֮��ĸ�����
	nDiffuse	[in]̫����ǿ��?{0.0-1.0}֮��ĸ�����
	_pLightMap	[out]����ͼ�׵�ַ
*/
void createLightmapFromHeightmap(const float* pfHeightmap,
								 const int _iSize ,
								 const osVec3D& VecDir,
									float nAmbient, float nDiffuse,
									 BYTE * _pLightMap);


#endif //__MAPHELPER_H__