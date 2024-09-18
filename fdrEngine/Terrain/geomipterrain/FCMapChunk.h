//===================================================================
/** \file  
* Filename :   FCMapChunk.h
* Desc     :   
* His      :   Windy create @2005-12-6 16:34:49
*/
//===================================================================
#pragma once
#include "MapChunk.h"
#include "vertexTemplate/VertexTemplatesAll.h"
class MapTile;


/** 无LOD,只是测试使用.最简单的地形块类
*/
class FCMapChunk :public MapChunk
{
public:
	virtual bool	UpdateTessellation();
	virtual bool	UpdateTessellation2();
	virtual bool	UpdateTessellation3();
	virtual void	DeInit();
	virtual void	Init();
	void			CreateTessellation();
	bool			FillBuffers();

	FCMapChunk(MapTile * mt,int x,int y);
	~FCMapChunk(void);
private:
	//! 如果不变形,则所有的FcChunk只需要使用一个静态的索引缓冲区
	static int     m_iSIdxBufId;
};
