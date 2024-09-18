	//===================================================================
/** \file  
* Filename :   MapChunk.h
* Desc     :   
* His      :   Windy create @2005-12-6 11:13:59
*/
//===================================================================
#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include "../../../common/com_include.h"
# include "../../mfpipe/include/osPolygon.h"
# include "../../effect/include/dlight.h"

class MapTile;
class MapChunk;

class osc_middlePipe;
class Frustum;
#define	MAX_SCREEN_ERROR	 0.010f

//! river @ 2009-9-15:Ԥ������ص�chunk hard tile.
struct osc_chunkHardTile;
struct osc_hardTextureTile;


typedef std::vector<os_renderMeshData> VEC_rMeshData;


//! hardTile���豸�������
struct  chunk_hardTile
{
	//! �����ID.
	int     m_iTexId;

	//! index buffer��id.
	int     m_iIdxBufId;

	//! uv������ص����ݣ��þ����ʾ��

};

/** ���ο�Ļ���
@remarks
	����,�ṩ�ڴ��ж�������,����������ݲ�ͬ��LOD������Ӷ���,
	������Ϣͨ��MapTile�õ�ԭʼ�ĸ߶���Ϣ,�������ź�ƫ�Ƴ�Ϊ���������綥����Ϣ,
	���������ʵ��Init,DeInit,Draw,Setup����
	����UpdateTessellation����ɶ������ݵ��ڴ����.
	UpdateTessellation2,UpdateTessellation3 ������������ĸ���,�����������,�綥����α�
@ps
	
*/
class MapChunk
{
	friend class MapTile;
public:
	struct ChunkVertex{
		osVec3D pos;

		osVec3D normal;

		DWORD	color;
		osVec2D tex;

		//! �����м�ܵ�������̬�Ķ��㻺����
		static DWORD m_dwFvf;
	};
	struct sPassInfo {
		sPassInfo(){
			m_TexIdx = -1;
			m_pAlpha = NULL;
			m_AlphaID = -1;
		}
		int				m_TexIdx;	//!��MapTile�е���������
		unsigned char	*m_pAlpha;	//!���ϲ������ALPHA��С
		int				m_AlphaID;	//!�����������ID
	};

	//! River added @ 2007-1-25:����ϸ������Ŀ���
	static void  OneTimeInit( osc_middlePipe* _ptrPipe );

	//! �м���Ⱦ�ܵ���ָ��,����ʹ�����е���Ⱦ�ײ�
	static osc_middlePipe*   m_ptrMidPipe;


protected:
	ChunkVertex*	m_Vertices;

	unsigned short* m_Indices;
	unsigned short* m_IndexMap;
	int				m_NumVertices,m_NumIndices,
					m_NumNewVertices,m_NumNewIndices;

	MapTile			*m_mt;
	/** ���������ͼ������ƫ��,	*/
	int				m_px, m_py;
	/** �����������ͼ������ռ�λ��ƫ��	*/
	float			m_xbase,m_zbase;

	/** ��־����CHUNK��init,�������ڴ棬��û���������û���и�ֵ
	@remarks
		��LODʵ�ֵ�����CHUNK��,��CHUNK��Ϊ�ȼ��ı�,Ҫ�ı䶥�����������ʱ,Ҳ����Ϊture.
		��֤��FillBuffers��֪���������������Ѿ��ı�
	*/
	bool			m_ForceBufferCreate;

	//! �м�ܵ���صĶ��������������ID
	int            m_iVerBufId;
	int            m_iIdxBufId;


public:
	VEC_rMeshData   m_vecPassMeshData;
	
	//! ver 16 add ��Ʒ��Ӱ������Ϣ
	os_renderMeshData   m_LightMapMestData;
	os_renderMeshData   m_PSPassMeshData;
	
	///@{
	os_renderMeshData   m_DepthPassMeshData;
	///@}

	//@{ 
	//  ����Ӳ�Ӵ��ر���Ⱦ��ص�����
	int               m_iHardTileTypeNum;
	//  ����ΪӲ�ӵر��豸��ص���Ⱦ����
	chunk_hardTile    m_vecHardTile[MAX_HARDTILE_PERCHUNK]; 
	os_renderMeshData m_sHardTile[MAX_HARDTILE_PERCHUNK];
	osMatrix          m_sHtTexMat[MAX_HARDTILE_PERCHUNK];
	//@}

public:
	//!��־��CHUNK�Ƿ�ɼ�
	bool			m_IsVisible;

	//!��־��CHUNK�Ƿ񱻴�����
	bool			m_IsAcitve;

	//! ��־��ǰ��Chunk�Ƿ񱻶�̬��Ӱ�쵽
	bool            m_bAffectByDLight;

	MapChunk(MapTile * mt,int x,int y);
	virtual ~MapChunk(void);

	//!����ʵ�ֵĺ���
	virtual bool	Draw(size_t _ipass);

	virtual bool	UpdateTessellation() = 0;

	virtual bool	UpdateTessellation2(){ return false;}

	virtual bool	UpdateTessellation3(){ return false;}
	//!
	//!���ļ����ؿ���Ϣ����
	void			LoadChunk(const BYTE *fpBegin,DWORD offset);

	void			SetVisible(bool Vis);

	//  
	//! �õ��豸��ص�����,����ʹ�����е���Ⱦ�ײ�����ȾChunk����
	const os_renderMeshData* get_passRdata( int _pass );

	const os_renderMeshData* get_pspassRdata( );
	const os_renderMeshData* get_depthpassRdata( );

	//! river @ 2009-9-17:����Ӳ�Ӵ��ĵر�.
	const os_renderMeshData* get_hardTileRdata( int _idx );
	

	const os_renderMeshData* get_lightmappassRdata( );

	int getpassCnt(){return (int)m_PassList.size();}

	//! �ϲ����ô�chunk ��Ҫ������䶥�㻺����
	void  set_forceCreate( BOOL _fc = TRUE ){ m_ForceBufferCreate = _fc; }

	//! ����hardTile��Ҫ��index����
	int   construct_hardTileIdxBuf( osc_hardTextureTile* _ht );

	//! ��ǰChunk�����Ӧ��Ӳ�Ӵ��ر��
	bool  process_hardTile( osc_chunkHardTile* _hardTile );




	const BYTE* get_lightMapL8(void)const{return m_LightMapL8;}
protected:
	//! ����ʵ�ֵĺ���
	virtual void	Setup();
	virtual void	Init();
	virtual void	DeInit();
	void			DeInitUnVisible();

	float			GetHeight(int nX,int nY);
	unsigned short	GetIndex(int nX,int nY);
	void			GetVertex(int nX,int nY,osVec3D & pt);
	void			GetTexCoord(int nX,int nY,osVec2D & TCood);
	void			GetDiffuse(int nX,int nY, DWORD &Diffuse);
	void			AddIndex(unsigned short nIdx);
	void			AddLastIndexAgain();

	//! �õ�ÿһ����������
	const osMatrix*  get_chunkTexMatrix( int _idx );

	//! �ͷ�hardTile���豸�������
	void             release_hardTile( void );


private:
	void		InitAlphaLMData();
	void		SafeDeInitAlphaLMData();
protected:
	enum {
		MAX_ALPHALAYS = 3,
	};
	//!���������Ϣ
	std::vector<sPassInfo>	m_PassList;
	
	BYTE					*m_AlphaPool[MAX_ALPHALAYS];
	char                   *m_LightMap;   //DDS DXT5 FORMAT
	BYTE					*m_LightMapL8; // RGB 255 format
	//!��Ӱ��Ϣ
	int				m_LightMapID;	//!LIGHTMAP���м�ܵ���ID
	DWORD			m_AlphaSize;	//!���ϲ㴫���ALPHA��С
	int             m_CombineAlphaID;  //!
	bool			m_bIsAlphaInit;
	
};
typedef std::vector<MapChunk *>	MapChunkPtrList;


