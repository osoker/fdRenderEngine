/** \file fdTerrainTriangleSelector.h
\n 功能描述 :   
\author         windy   
\date           2006-6-15 10:10:08*/
#ifndef __FDTERRAINTRIANGLESELECTOR_H__
#define __FDTERRAINTRIANGLESELECTOR_H__
#include "../../interface/osInterface.h"

class MapTile;

class osTerrainTriangleSelector : public ITriangleSelector
{
public:

	osTerrainTriangleSelector();

	virtual ~osTerrainTriangleSelector();
	//! 返回所有可用的三角片个数
	virtual int getTriangleCount() const;
	//! 得到所有的三角片列表
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const osMatrix* transform=0) const;
	//! 得到位于一个包围盒的三角片列表
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_bbox& box, const osMatrix* transform=0) const;
	//! 得到和射线相交的三角片列表
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_Ray& line, const osMatrix* transform=0) const;

	virtual void release_obj( DWORD _ptr = NULL );

public:
	void LoadData(MapTile* _mapTile);
private:
	//!相对于世界地图的偏移。是真实的距离。
	float	mXbase, mZbase;
	const float* mHeightMapData;

};

#endif //__FDTERRAINTRIANGLESELECTOR_H__