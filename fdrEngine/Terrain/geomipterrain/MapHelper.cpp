//===================================================================
/** \file  
* Filename :   MapHelper.cpp
* Desc     :   
* His      :   Windy create @2006-1-7 10:31:38
*/
//===================================================================
# include "stdafx.h"
#include "MapHelper.h"


int GetPowerTwo( int _pow )
{
	switch(_pow) {
	case 0:
	default:
		return 1;
	case 1:
		return 2;
	case 2:
		return 4;
	case 3:
		return 8;
	case 4:
		return 16;
	case 5:
		return 32;
	case 6:
		return 64;
	}
}
/** �õ���������·�� */
void fixtexname(std::string &name,std::string _mapname)
{
	/*����: name = "tile0000" _mapname = "vilg"
	�õ�:"texture\\vilg\\tile0000.tga"
	*/
	std::string temp(name);
	name = "data\\texture\\";
	name.append(_mapname);
	name.append("\\");
	name.append(temp);
	name.append(".dds");
}
/** ���ݸ߶����ݼ������ͼ
@par
pfHeightmap [in]�߶������׵�ַ
_iSize		[in]�߶����ݿ�Ⱥ͸߶�Ҳ�����ɵĹ���ͼ�Ŀ�ߡ�
VecDir		[in]�������䷽��
nAmbient	[in]������ǿ��?{0.0-1.0}֮��ĸ�����
nDiffuse	[in]̫����ǿ��?{0.0-1.0}֮��ĸ�����
_pLightMap	[out]����ͼ�׵�ַ
*/
void createLightmapFromHeightmap(const float* pfHeightmap,const int _iSize ,const osVec3D& VecDir,
	  float nAmbient, float nDiffuse,
	  BYTE * _pLightMap)
{
	osassert(pfHeightmap);
	memset((void *)_pLightMap, 0, _iSize*_iSize);

	float	light;
	osVec3D normal;

	for(int y=1; y<_iSize-1; y++)
	{
		for(int x=1; x<_iSize-1; x++)
		{
			int				idx = x + y*_iSize;
			float				//p0 = sdata[idx],
				pt = pfHeightmap[idx-_iSize],
				pb = pfHeightmap[idx+_iSize],
				pl = pfHeightmap[idx-1],
				pr = pfHeightmap[idx+1];

			normal.x = (pl-pr) / (2.0f*255.0f) * _iSize;
			normal.y = 1.0f;
			normal.z = (pt-pb) / (2.0f*255.0f) * _iSize;
			osVec3Normalize(&normal,&normal);
			
			float f = osVec3Dot(&normal ,&VecDir);

			if(f<0.0f)
				f = 0.0f;

			light = nAmbient + nDiffuse * f;
			if(light>1.0f){
				light = 1.0f;
			}
			//osassert(light<1.0f);

			_pLightMap[x + y*_iSize] = (BYTE)(light * 255.0f);
		}
	}
	_pLightMap[0] = _pLightMap[1+_iSize];
	_pLightMap[_iSize-1] = _pLightMap[_iSize+_iSize-2];

	_pLightMap[(_iSize-1)*_iSize] = _pLightMap[(_iSize-2)*_iSize+1];
	_pLightMap[(_iSize-1)*_iSize+_iSize-1] = _pLightMap[(_iSize-2)*_iSize+_iSize-2];

	for(int x=1; x<_iSize-1; x++)
	{
		_pLightMap[x] = _pLightMap[x+_iSize];
		_pLightMap[x+(_iSize-1)*_iSize] = _pLightMap[x+(_iSize-2)*_iSize];
	}

	for(int y=1; y<_iSize-1; y++)
	{
		_pLightMap[y*_iSize] = _pLightMap[y*_iSize+1];
		_pLightMap[y*_iSize+_iSize-1] = _pLightMap[y*_iSize+_iSize-2];
	}
}