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
/** 根据高度数据计算光照图
@par
	pfHeightmap [in]高度数据首地址
	_iSize		[in]高度数据宽度和高度也是生成的光照图的宽高。
	VecDir		[in]光线照射方向
	nAmbient	[in]环境光强度?{0.0-1.0}之间的浮点数
	nDiffuse	[in]太阳光强度?{0.0-1.0}之间的浮点数
	_pLightMap	[out]光照图首地址
*/
void createLightmapFromHeightmap(const float* pfHeightmap,
								 const int _iSize ,
								 const osVec3D& VecDir,
									float nAmbient, float nDiffuse,
									 BYTE * _pLightMap);


#endif //__MAPHELPER_H__