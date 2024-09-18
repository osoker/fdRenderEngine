/** \file fdTerrainTriangleSelector.h
\n �������� :   
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
	//! �������п��õ�����Ƭ����
	virtual int getTriangleCount() const;
	//! �õ����е�����Ƭ�б�
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const osMatrix* transform=0) const;
	//! �õ�λ��һ����Χ�е�����Ƭ�б�
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_bbox& box, const osMatrix* transform=0) const;
	//! �õ��������ཻ������Ƭ�б�
	virtual void getTriangles(osVec3D* triangles, int arraySize,
		int& outTriangleCount, const os_Ray& line, const osMatrix* transform=0) const;

	virtual void release_obj( DWORD _ptr = NULL );

public:
	void LoadData(MapTile* _mapTile);
private:
	//!����������ͼ��ƫ�ơ�����ʵ�ľ��롣
	float	mXbase, mZbase;
	const float* mHeightMapData;

};

#endif //__FDTERRAINTRIANGLESELECTOR_H__