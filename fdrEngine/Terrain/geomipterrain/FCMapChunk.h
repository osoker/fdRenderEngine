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


/** ��LOD,ֻ�ǲ���ʹ��.��򵥵ĵ��ο���
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
	//! ���������,�����е�FcChunkֻ��Ҫʹ��һ����̬������������
	static int     m_iSIdxBufId;
};
