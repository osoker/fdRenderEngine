/** \file fdTerrainTriangleSelector.cpp
\n 功能描述 :   
\author         windy   
\date           2006-6-15 10:10:30*/
# include "stdafx.h"
#include "../include/fdTerrainTriangleSelector.h"
#include "../GeoMipTerrain/MapTile.h"

osTerrainTriangleSelector::osTerrainTriangleSelector()
{
}
osTerrainTriangleSelector::~osTerrainTriangleSelector()
{
}
int osTerrainTriangleSelector::getTriangleCount() const
{
	return 0;
}

void osTerrainTriangleSelector::getTriangles(osVec3D* triangles, int arraySize,
	int& outTriangleCount, const osMatrix* transform/*=0*/) const
{
}

void osTerrainTriangleSelector::getTriangles(osVec3D* triangles, int arraySize,
	int& outTriangleCount, const os_bbox& box, const osMatrix* transform/*=0*/) const
{
	const osVec3D* minBox = box.get_vecmin();
	const osVec3D* maxBox = box.get_vecmax();
	//!1 对包围盒是否在地图内做检测,如果不在,返回空.
	if (!((maxBox->x > mXbase)&&(maxBox->z > mZbase)&&
		(minBox->x<mXbase+192)&&(minBox->z<mZbase+192)))
	{
		outTriangleCount = 0;
		return;
	}
	//!2 以包围盒为中心,长度为半径选取三角片.
	const osVec3D* pCenterPoint = box.get_bbPos();
	osVec2D ReleativeCenter;
	ReleativeCenter.x = pCenterPoint->x - mXbase;
	ReleativeCenter.x = pCenterPoint->z - mZbase;
	
	
	
	
	/*if ()
	{
	}*/
	//!3 返回选取的三角片.
}

void osTerrainTriangleSelector::getTriangles(osVec3D* triangles, int arraySize,
	int& outTriangleCount, const os_Ray& line, const osMatrix* transform/*=0*/) const
{

}
void osTerrainTriangleSelector::LoadData(MapTile* _mapTile)
{
	mHeightMapData = _mapTile->GetRawHMData();
	_mapTile->GetWorldOffset(mXbase,mZbase);



}
void osTerrainTriangleSelector::release_obj( DWORD _ptr /*= NULL*/ )
{
}