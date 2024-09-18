//===================================================================
/** \file  
* Filename :   MapTile.h
* Desc     :   
* His      :   Windy create @2005-12-5 14:10:02
*/
//===================================================================
#pragma once
#include <vector>
#include <string>
#include <d3d9.h>
#include "MapChunk.h"
#include "../../../common/com_include.h"
# include "../../effect/include/dlight.h"
#include "vertexTemplate/VertexTemplatesAll.h"
#include "../include/fdTerrainTriangleSelector.h"

class MapTilePVS;
class osc_mapTerrain;


//! river added @ 2009-9-15:Ӳ�Ӵ��ĵر�����
struct  osc_chunkHardTile;

/** ��LOD��ʽ�����ʽ*/
typedef	Vertex::Composer< Vertex::Position,
		Vertex::Composer< Vertex::TexCoords0 > >
	//	Vertex::Composer< Vertex::TexCoords1 > > >
		FCVertex;
/** �����ʽ�����α�Ķ����ʽ*/
typedef	Vertex::Composer< Vertex::Position,
		Vertex::Composer< Vertex::TexCoords0,
		Vertex::Composer< Vertex::TexCoords1 > > >
		SWVertex;
///TODO:Ӳ�������α䶥���ʽ


/** ��ͼ��,ÿ����ͼÿ����ͼ����CHUNKS X CHUNKS�����ο�.
@remarks
	���帺���ͼ�ļ��ļ���,���ο���ӽ����޳���PVS�޳�.
	�Լ��ر�����Ⱦ����֯���Ż�,
@par
	PVS���е�����,�ر�Ķ����Ⱦû���Ż�,�Ĳ����Ľ����Ժ�ҪԤ����.
@note
	����ֻ����MapChunk��,������ο�MapChunk
*/
class MapTile
{
	friend class	osc_mapTerrain;
	friend class	MapChunk;
public:
	enum RENDERMODE {
		EMT_BRUTEFORCE,	//!��LOD��û�����α�,
		EMT_NOLOD,
		EMT_MORPH_SW,	//!	LOD��CPU���㶥���α䷽ʽ
		EMT_MORPH_HW	//!	LOD��GPUӲ�����㶥���α䷽ʽ
	};
	enum {
		GRID_SIZE = 3,	//!XZ���������
		HEIGHT_SIZE = 1,//!Y���������
		GRIDS = 16,		//!ÿ��CHUNK�ĸ�����/��(��߼������)
		SUBDIV = 4,		//!LOD��߼� 2^4 = 16  
		CHUNKS = 4,		//!һ����ͼ��Ķ��ٸ�CHUNK/��
		CHUNK_SIZE = GRID_SIZE*GRIDS,
		TILE_SIZE = CHUNK_SIZE*CHUNKS
	};
	enum {
		TILE_PIXEL = 65,
		CHUNK_PIXEL = GRIDS+1
	};
	enum {
		MAX_VERTICES = CHUNK_PIXEL * CHUNK_PIXEL,
		MAX_INDICES = MAX_VERTICES*3
	};

	/** ��Ⱦ�豸�����Դ,���ݲ�ͬ����Ⱦģʽ���ò�ͬ�Ķ�������*/
	static LPDIRECT3DVERTEXDECLARATION9	m_pDeclaration;


private:
	
	//!�ر��ظ��������Ϣ
	struct  PassInfo{
		PassInfo():RepTex(-1){};
		int	RepTex;
		std::string	texname;
		osVec2D		vRepeat;	//!�ظ�����
		D3DXMATRIX	mat;		//!D3D�õ����ظ�����
	};
	typedef std::vector<PassInfo>	PassList;

	//!��Ķ���ɫ
	struct PointDiffuse {
		PointDiffuse():iX(0),iY(0){}
		//int ilightId;
		int iX,
			iY;
		D3DCOLOR	diffuse;
	};
	typedef std::vector<PointDiffuse>	PointDiffuseVec;

	//!��Ⱦ�õ��ĸ߶�����
	float			*m_HMData;
	osVec3D         *m_vecNormal;
	D3DCOLOR		*m_DiffuseData;
	D3DCOLOR		*m_OrigDiffuseData;
	D3DCOLOR		*m_AmbientDiffuseData;

	//!���涯̬���޸ĵĵ��ԭʼDIFFUSE,
	PointDiffuseVec m_LitDiffList[MAX_LIGHTNUM];
	PassList		m_Pass;	
	std::string		m_mapname;
	int				m_AlphaSize;
	static int             ms_LightMapSize;
	//!����������ͼ�е�����λ�á�
	int			m_xpos, m_zpos;
	//!����������ͼ��ƫ�ơ�����ʵ�ľ��롣
	float		m_xbase, m_zbase;
	MapChunk	*m_chunks[CHUNKS][CHUNKS];
	//!������Ⱦ��ʽ
	static RENDERMODE s_RenderMode;

	/** �ͷ��豸�������,��D3D����*/
	void		SafeReleaseDeviceData();

	/** ����  SetChunkVisible ���õĿɼ��飬����Щ���ύ���ϲ�������У����ϲ�ͳһ����*/
	void		PushActiveChunk();

	void		ClearChunks();

public:
	const float * GetRawHMData()const{return m_HMData; };
	void GetWorldOffset(float& xbase,float& zbase) const {xbase = m_xbase; zbase = m_zbase;};

	static void OneTimeDeinit();
	static void OneTimeInit( osc_middlePipe* _pipe );

	//! River added @ 2006-1-12:��mapTile֮�ϵ�mapTerrain
	static osc_mapTerrain*  m_ptrMapTerrain;

	/** \brief 
	 * ���µ���,�������ر�ĸ���
	 *
	 * ������Ⱦ�ر��ʱ��ֻ��Ҫ����һ��
	 *
	 * \param _envMap �Ƿ���Ⱦ����Ϊ������ͼʹ��
	 */
	static void		DrawTerrain( BOOL _envMap = FALSE ,BOOL _depth = FALSE);

	//! ��տɼ��ĵ��ο�
	static void     ClearVisibleTerr( void );


	/** ���ص�ͼƬ�ӿ�
	@par 
		_pFileBegin ��ͼ���ݵ�ͷָ��
		_mapName	��ͼ���ڵ�������vilg,test
		_x0,_z0		��ͼƬ����Ե������� 1,0 
	@remarks
	�����Ǽ���һ����ͼƬ�Ĵ���
		MapTile *  pTile = new MapTile();
		BYTE*	t_fstart;
		BYTE*	fileBegin;
		int     t_iSize;
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_iSize = read_fileToBuf( name,t_fstart,TMP_BUFSIZE );

		pTile->LoadMapTile(t_fstart,mapname,x,z);
		END_USEGBUF( t_iGBufIdx );
		pTile->InitDeviceData();
	*/
	bool		LoadMapTile(const BYTE * _pFileBegin,const char* _szmapName);

	//! ����ǰmaptile��ÿһ��chunk�ڵ�Ӳ�Ӵ��ر�����
	bool        process_hardTex( osc_chunkHardTile* _hardTile );


	/** ��ʼ�������豸��ص���Դ,�������
	@remarks �ڼ������ͼ�����*/
	bool		InitDeviceData();

	void		ClearDynamic_Light();
	void		ReCalcChunkDynamic_Light();
	void		SetambientLCoeff(float _r,float _g,float _b);
	//!���ö�̬�� ,�ɹ����� TRUE
	BOOL		SetDynamic_Light(osc_dlight* _dLight,int _lightID);
	
	/** ���ϲ���� �ɼ������� */
	void		SetChunkVisible(int _idxX,int _idxY,bool _IsbVisible);

	/** \brief
	 *  �ϲ�����ĳһ��Chunk�Ƿ��ܶ�̬���Ӱ��
	 */
	void        setChunkAffectByDl( int _idxX,int _idxY );


	/** �õ��߶�,һ����MapChunk�����, */
	float		GetHeight(int nX,int nY);

	//! �õ�һ������������Ӧ��Normal
	void        GetNormal( int _x,int _y,osVec3D& _normal );

	void		GetDiffuse(int nX, int nY, D3DCOLOR &_diffuse);

	void		GetDiffuseOrig(int nX, int nY, D3DXCOLOR &_diffuse);

	D3DCOLOR &	GetDiffuse(int nX, int nY);
	/** ������������ */
	void		GetTexCoord(int nX,int nY,osVec2D &vTCood);


	// River mod @ 2007-6-8:�ϲ�ô��ر�ĸ߶���Ϣ
	const float* get_heightInfoPtr( void ) { return m_HMData; } 

public:
	MapTile(void);
	~MapTile(void);
};
