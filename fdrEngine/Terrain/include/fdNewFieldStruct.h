//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldStruct.h
 *
 *  His:      River created @ 2006-1-10
 *
 *  Des:      ��ͼ��������صĽṹ��
 *   
 * ����ּ���֮�ǣ����������������ǽ֮��Ҳ���� 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "fdFieldStruct.h"
# include "../geoMipTerrain/newHeader.h"
# include "fdTGQuadtree.h"
# include "../geoMipTerrain/MapTile.h"

typedef std::vector<os_newTGObject>  VEC_newTgObj;
typedef std::vector<os_aabbox>       VEC_aabbox;


//!�����������ϰ汾��
struct MapVersionHead
{
	// �汾��
	unsigned int	m_dwVersion;
public:
	MapVersionHead()
	{
		m_dwVersion = 0;
	}
};

//! ռ������λȷ����Ʒ������
# define OBJ_BRIDGETYPE			0x01
# define OBJ_TREETYPE			(0x01<<1)
# define OBJ_ACCEPTFOGTYPE		(0x01<<2)
# define OBJ_NOALPHATYPE		(0x01<<3)


//! ��һ��chunk�ڲ����ر���ӵ�����
struct osc_tileInChunk
{
	int     m_iX;
	int     m_iY;
};

typedef std::vector<osc_tileInChunk> VEC_tileInChunk;
 
//! River mod @ 2009-9-14:�����°汾�ģ��ر���ӱ߽��Ӳ�����ں�����
//! ÿһ��chunk�����ж����Ӧ��hare texture tile���б�����
struct osc_hardTextureTile
{
	//! �ر���ӵ���Ŀ��
	int     m_iTileNum;

	//! Ӳ�Ӵ��ر���ӵ�����
	VEC_tileInChunk   m_vecTileInChunk;

	//! ��Ӧ���������ֺ�UV����ֵ
	char             m_szTexName[32];
	int              m_wU;
	int              m_wV;
	int				 m_rotation; // 0 (0 rad),1 (pi*0.5f rad),2 (pi rad),3 (pi * 1.5f rad)
};


//! ÿһ��chunk��Ӧ��hardTexture����
struct osc_chunkHardTile
{
	int   m_iHardTileNum;

	osc_hardTextureTile*   m_arrTileInChunk;

public:
	osc_chunkHardTile()
	{
		m_arrTileInChunk = NULL;
		m_iHardTileNum = 0;
	}

	void reset( void )
	{
		m_iHardTileNum = 0;
		SAFE_DELETE_ARRAY( m_arrTileInChunk );
	}

	~osc_chunkHardTile()
	{
		reset();
	}
};

//! ����ÿһ��tileGround�õ������ݽṹ
typedef std::vector<osc_chunkHardTile>  VEC_chunkHardTile;

/** \brief
 *  �µĻ����ؿ���֯��
 *  
 *  ����ؿ���֯����������ɵ�Geomipmap����,�ѵر���ص����ݴ���Geomipmap
 *  ��ص����ݽṹ,����Ʒ����ʾ��Ϣ�Լ���֯����.
 */
class osc_newTileGround
{
	friend class             osc_TGManager;
	friend class             osc_tgLoadReleaseMgr;
	friend class             osc_tgQuadtree;

public:
	//! TG�ڲ�����������ڵĹ�������ָ�롣
	static osc_TGManager*     m_ptrTgMgr;

	//! ȫ�ֵ����ϰ汾��
	static MapVersionHead     m_sMapVersionHead;
	static BOOL               m_bMapVersionChange;

	//! ��ͼ��Ӧ��Ӳ���ļ��ĵ�ͼ���֡�
	s_string          m_strMapFName;

	//! ��ײ��Ϣ
	os_TileCollisionInfo  m_arrCollInfo[GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT];

	// ÿ����ײ���������ϲ�����idx
	BYTE                m_vecUpmostTexIdx[GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT];
	//! ��ͼ��ÿһ���������ԣ�Ŀǰֻ��Ҫ֪���Ƿ���ʾ��
	VEC_BYTE            m_vecTileAttribute;

	//! ����һ�ݵ�ǰtg��Ӧ�ļ����ļ�ͷ��Ϣ 
	os_newFileHeader     m_sTgHeader;

	//! ������ͼ�༭��������Ʒ����Ŀ
	int                 m_iTGObjectNum;
	VEC_newTgObj         m_vecTGObject;
	VEC_aabbox           m_vecObjAabbox;
	//! ��¼ÿһ����Ʒ��Ӧ�Ķ�̬��,�����ӦΪ�㣬���ܶ�̬���Ӱ��
	VEC_meshDLight       m_vecTGObjDL;



	//! ��osa�ļ����뵽��Ӧ��smInit�ṹ�ڡ�
	BOOL               load_smInitData( const char* _sz,
		                 os_skinMeshInit& _smInit,os_newTGObject*  _ptrObj );

	/** \brief
	 *  ÿһ���������м�ܵ�����Ⱦʱ��Ҫ��ID.
	 */
	VEC_int              m_vecObjectDevIdx;
	//! River added @ 2008-12-16:�������飬��ȷ����ӦID�Ƿ�ΪbrideObj.
	VEC_BYTE             m_vecObjectType;

	//! �ڵ�ǰ��Ⱦ֡�ڣ���Ʒ��ÿһ�������Ƿ�����Ұ�ڡ�
	VEC_BYTE            m_vecObjInView;

	//! ��tileGround�ڲ���ײ���ӵĸ߶�����.
	VEC_float           m_vecHeight;

	//! ��ǰ��tileGround�Ƿ��Ѿ�ʹ��.
	BOOL                m_bInuse;
	//! ��ǰ��Tg�Ƿ��ڶ��߳��Ŷӵ���״̬
	BOOL                m_bInMTQueue;

	//! �������пɼ�����Ʒ�б����Ʒ���뵭��������ݵĹ�����.
	osc_inViewObjMgr     m_ivoMgr;

	//! ���ڹ���ر�������ݵ���Ⱦ����
	MapTile             m_sMapTerrain;

	//! ����ˮ��߶�ʱ�� snowwin add
	static std::vector<float> m_vecWaterHeight;

	//�� �߰汾�ĳ��������ܻ��õ���һЩ���ݽṹ
	VEC_chunkHardTile   m_vecChunkHardTile;

public:
	//! ��tg��Ӧ���Ĳ����ṹ
	osc_tgQuadtree       m_sQuadTree;

	//! �Ƿ�����TGû��һ��Chunk��Ҫ��,�˱��������ϲ������Ĵ���
	BOOL                m_bEntireCullOff;

	//! ��ǰ�����TG�ڵ���һ��顣
	static osn_inTgPos   m_seCamTgPos;

	// windy mod
	float				mWaterHeight;

	/** \brief
	 *  �õ��ɼ�����Ʒ�б�
	 *
	 *  �������Ż�����㷨���ͼ򵥵��㷨��ֻҪ����������Ʒ�����ǿɼ�����Ʒ��
	 */
	void              get_inviewObjList( void );


private:

	//! ����һ���ر������ChunkΪ�ɻ���״̬
	void                set_chunkDraw( int _idx,BOOL _draw = TRUE );

	//! ����һ������Chunk�ܶ�̬���Ӱ��
	void                set_chunkAffectByDL( int _idx );

	/** \brief
	 *  ��һ�ε����ͼʱ�����ݵ�ǰmeshMgr�п����ɵ����mesh��Ŀ�����뵱ǰ
	 *  ��ͼ��mesh.
	 *  
	 *  ��̬�ĵ����ͼ��objʱ���������������
	 *  
	 */
	void                load_tgObj( void );

	//! �õ�ÿһ����Ʒ�Ƿ�Ϊbridge��Ʒ
	void                process_objIsBridge( void );


	//! ������tg�����е���Ʒ���豸�������
	BOOL                create_devdepObj( void );


	/** \brief
	 *  �Ե�ͼ��billBoard��Ʒ���д�����
	 *  
	 */
	BOOL              create_billBoardObj( void );

	/** \brief
	 *  ����������Ʒ��ص����ݡ�
	 */
	BOOL              creaet_keyAniObj( void );

	/** \brief
	 *  ����TileGround�е�Water�����Ʒ��
	 */
	BOOL              create_waterObj( void );

	/** \brief
	 *  ����������������ص���Ʒ��
	 */
	BOOL              create_particleObj( void );

	/** \brief
	 *  ���������ڵ�������Ч��Ʒ����spe��Ʒ
	 */
	BOOL              create_effectObj( void );


	/** \brief
	*  ����TileGround�е�����������Ʒ��
	*/
	BOOL              create_FogObj( void );

	/** \brief
	 *  ������ͼ��ʹ��skinMesh���ж��������塣
	 *
	 *  ���ʹ��skinMesh������д�����
	 */
	BOOL              create_smAniObj( void );

	//! �����Ĵ���һ��objMesh.
	BOOL              create_meshImmediate( int _idx );

	//! �ͷŵ�ͼ����Ʒ��
	BOOL              relase_tgObj( void );

	/** \brief
	*  ����Ⱦ�����ڵĿɽ����������Ʒ
	*/
	BOOL              render_AcceptFogObject( );

	//! ��һ����Ʒ���벻͸����Ⱦ�б��У���Ҫ����������Ʒ
	void PushFogObject( int _idx );

	/** \brief
	 *  ����Ⱦ�����ڵ�������Ʒ
	 */
	BOOL              render_bridgeObject( osc_middlePipe* _mpipe );

	/** \brief
	 *  ��Ⱦ������������Ʒ��͸�����֡�
	 */
	BOOL              render_bridgeObjAlphaPart( osc_middlePipe* _mpipe );

	/** \brief
	 *  ��ͼ������Ʒ��Ⱦ�ĺ���. 
	 *
	 *  \param _alpha ���Ƶ�ǰ��Ⱦ������Ʒ��͸���Ĳ��ֻ��ǲ�͸���Ĳ��֡�
	 *                ͸���Ĳ�����Ҫ�ŵ������Ⱦ��
	 */
	BOOL              render_TGsObject( osc_middlePipe* _mpipe,
		                                        BOOL _alpha = false );

	//! ��Ⱦ�����Object.
	BOOL              render_TGsFadeInObject( osc_middlePipe* _mpipe,
		BOOL _alpha = false );

	//! ��Ⱦ������Object.
	BOOL              render_TGsFadeOutObject( osc_middlePipe* _mpipe,
		BOOL _alpha = false );

	/** \brief
	 *  ��Ⱦ��͸����object.
	 *
	 *  ATTENTION TO OPP:���������������Ժϲ���һ��.
	 *  û��ǰ��������alphaΪtrue���������.
	 */
	BOOL              render_halfAlphaObj( 
		osc_middlePipe* _mpipe,BOOL _alpha );

	/** ���ݵ�ͼ��ȫ��ͼ·����,�õ���ͼ��
	*	��:_szfullname map\vilg\vilg0000.geo,
	*	_szmapname vilg
	*/
	void             getmapname( const char* _szfullname,s_string& _szmapname);

	//! ���뵱ǰtileGround�ڵ�Ӳ�ر����ݡ�
	void             load_hardTileData( const BYTE* _fstart );

public:
	osc_newTileGround();
	~osc_newTileGround();
	

	/** \brief
	 *  ���ڴ��е���һ����ͼ�ļ���
	 *
	 *  \param _fload  �Ƿ�����Ϸ��ʼ��ʱ��һ�ε����ͼ��
	 */
	BOOL              load_TGData( const char* _fname,
		                   const osc_middlePipe* _mpipe,bool _fload = false );

	/** \brief
	 *  �ͷŵ�ǰ��ͼ����Դ.
	 */
	BOOL              release_TG( void );


	//! FrameSet,ÿһ֡���õ�ǰtg�ڵ�����
	void              frame_setTG( void );

	/** \brief
	 *  �ѵ�ǰ֡�ɼ���object�ŵ���Ⱦ�б���.
	 */ 
	void              push_objIdx( int _oidx );
	//! ��ǰ��obj�����Ƿ�ɼ�
	bool              is_objInView( int _oidx ){ return m_vecObjInView[_oidx]; }

	//! ���õ�ǰ֡�ܶ�̬��Ӱ���object�Ķ�̬����ɫ
	void              set_objDlight( int _oidx,int _dlidx );


	//! ��ǰ�ĵ�ͼ���Ƿ��Ѿ�Ͷ��ʹ�á�
	BOOL              is_tgInuse( void ) { return m_bInuse; }
	//! �õ������õ�ǰͼ��Ķ��߳��Ŷ�״̬,m_bInuseһ��״̬�����ܱ�֤���̵߳���ʱ��ͼ����ȷ״̬
	void              set_mtQueueState( BOOL _s ) { m_bInMTQueue = _s; }
	BOOL              get_mtQueueState( void )    { return m_bInMTQueue;} 


	//@{
	//! ��һ����������õ���ǰ���Ӷ�Ӧ����Ⱦpolygon���㡣
	void               get_polyVertex( osVec3D* _vec,int _x,int _z ); 
	//@} 

	/** \brief
	 *  �õ���ͼ��Ӧ��Ӳ�̵�ͼ�ļ�����
	 */
	const char*         get_mapFName( void ){ return m_strMapFName.c_str(); }

	//! �õ���Ʒ������BoundingBox��aabbox
	void               get_objBBox( int _objIdx,os_bbox& _box );
	//! River @ 2007-4-11:�õ��Ƿ����������Ʒ,������Ʒ��Ҫ��ȷ��ײ���.
	BOOL               is_bridgeObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_BRIDGETYPE;
	}
	//! RIVER @ 2009-3-3: �õ��Ƿ�������Ʒ��Ȼ��ȷ����ײ���
	BOOL               is_treeObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_TREETYPE;
	}
	BOOL               is_AcceptFogObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_ACCEPTFOGTYPE;
	}
	BOOL               is_NoAlphaObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_NOALPHATYPE;
	}


	os_aabbox*          get_objAaabbox( int _objIdx )
	{
		return &m_vecObjAabbox[_objIdx];
	}
	DWORD              get_objType( int _objIdx )
	{
		return m_vecTGObject[_objIdx].m_dwType; 
	}

	//! ��������Ƿ����Ʒ�����ཻ.
	BOOL               is_intersectObj( int _objIdx,osVec3D& _rayS,
		                    float _dis,osVec3D& _rayE,float* _resDis = NULL );


	



	//@{
	//  ��̬����صĺ�����
	/** \brief
	*  ʹ�ø����ĵƹ�ϵ���Գ�����ÿһ������ĵƹ�������š�
	*/
	void              set_ambientLCoeff( float _r,float _g,float _b );

	/** \brief
	 *  ����̬�⡣
	 */
	BOOL              process_dLight( osc_dlight* _dl ,int _lightID);

	

	/** \brief
	 *  ���ݴ����λ�ã��ж�������ڵ�TG����һ��顣
	 *
	 *  ����Ĳ���Ϊ��ײ�������������ͼ�ϵ�λ�á�
	 */
	static void   frame_moveCamInTgPos( int _camx,int _camy );   

	//! �õ������tg�ڵ���һ��.
	static osn_inTgPos get_camInTgPos( void );

	/** \brief
	*  �鿴һ��һ�������Ƿ��һ��Tile��os_polygon�ཻ��
	*
	*  \param _tileIdx  ����͵�ǰ���ӵ��������ཻ�����غ͵�ǰ�����ڵ���
	*                   һ����ײ�����ཻ����ײ���ӵ�������zֵ���ģ�xֵ��С�ķ�λΪ0,
	*                   x,z���ķ�λΪ1,z��С��x���ķ�λΪ2��
	*  \param _poly     ����ĸ��Ӷ�Ӧ����Ⱦ�����Ρ�
	*  
	*/
	static bool intersect_polygon( 
		const osVec3D* _arrVerData,osVec3D& _s,osVec3D& _e );

	//! ��һ�������õ�һ�����ӵ���ײ�͸߶���Ϣ
	const os_TileCollisionInfo*  get_tileColInfo( int _x,int _z )
	{
		return &m_arrCollInfo[_z*GROUNDMASK_WIDTH+_x];
	}
	const os_TileCollisionInfo*  get_tgColInfo( void )
	{
		return m_arrCollInfo;
	}
	const float* get_tgColHeightInfo(void)
	{
		if (!m_vecHeight.size())
		{
			return NULL;
		}
		return &m_vecHeight[0];
	}
	const int get_tgMaterialIdxInfo(WORD dx,WORD dz)
	{
		osassert(dx>=0);
		osassert(dz>=0);
		osassert(dx<128);
		osassert(dz<128);
		return (int)m_vecUpmostTexIdx[dz*128+dx];
	}
	const float get_tgWaterHeight()
	{
		return mWaterHeight;
	}

	//! �Ƿ�����TG����ViewFrustum Cull����
	BOOL         is_vfEntireCullOff( void ) { return m_bEntireCullOff; }



};





// 
//! �����ڵ��ѡ�еر����εİ�����
class osc_fdChunkPick
{
private:
	//! �洢��ǰChunk��Ҫ���Ե�Tile����
	os_tileIdx       m_arrTestTile[MapTile::GRIDS*MapTile::GRIDS];
	int             m_iTestTileNum;

	//! ���ڴ洢Ҫ�ж�chunk���ĸ��߽������
	osPlane          m_arrPlane[4];

	//! 
	osVec3D          m_vec3RayStart;
	osVec3D          m_vec3RayEnd;

private:
	//! ����Chunk���ܵ��ĸ���
	void          construct_clipPlanes( float _minX,float _minZ );

	void          push_testTile( int _x,int _z );

	//! ���ݴ�������ߺͿ�ʼ�������꣬���β��ҿ�����صĸ���
	int           find_testTileArr( float _minZ,int _startX );


public:

	/** \brief
	 *  ����chunk�ڱ����߷ָ��tileIdx
	 *
	 *  \param _rayStart,_rayDir ���pick���ߵķ���
	 *  \param _minX,_minZ       Ҫ����Chunk��x,z������Сֵ
	 *  ���µ��㷨������       
	 *     
	 *         Z����
	 *         |      1/
	 *   pt[1] -------/----------- pt[2]
	 *         |  |  /  |        |
	 *         -----/-------------
	 *         |  |/ |  |        |
	 *         ---/---------------
	 *      0  | /|  |           |  2
	 *         -/-----------------   
	 *         /  |  |           |
	 *        /-------------------
	 *Ray start|  |  |           | pt[3]
	 *   pt[0] ------------------------X����
	 *               3
	 *
	 *  һ���Ȱ�����Chunk������������߱��Ϊ0,1,2,3.
	 *  �������ı߱߹����ĸ���ֱ��xzƽ�����,��ķ����������ı��ε��ڲ���
	 *  �������Ray���������Ľ����ڵ�1,3��֮ǰ�����ҳ��˽����Ӧ�ĸ���������Ȼ��
	 *      ���ô˸��ӿ��ܸ������ཻ��
	 *      ������Ϊ����߱��ұ߽����ŵı߹���һ��ֱ��xzƽ����棬Ȼ���ҳ��������
	 *      �Ľ������,������Ӧ���ܸ������ཻ�ĸ��ӡ�
	 *      ֱ�����·�������������1��3֮�⣬���߼����˱��Ϊ2���档
	 *  �ģ��������������������Ե�3������д����ҵ���ȷ�Ľ���,��Ϊ���һ�����߸�
	 *      һ��Chunk��أ���ô����������������ཻ������ֻ��Ҫ�������͵�1,3���߾�ok�ˡ�
	 */
	int      get_pickTile( osVec3D&  _rayStart,
		         osVec3D& _rayDir,float _minX,float _minZ );


	//! �õ�Ҫ�����TileIdx�б�
	const os_tileIdx* get_testTileArr( void ) { return m_arrTestTile ; } 

};


//! ���õ�ǰ֡�ܶ�̬��Ӱ���object�Ķ�̬����ɫ
inline void osc_newTileGround::set_objDlight( int _oidx,int _dlidx )
{ 
	osassert( (_oidx>=0)&&(_oidx<m_iTGObjectNum) ); 

	for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
	{
		if( -1 == m_vecTGObjDL[_oidx].m_btDLIdx[t_i] )
		{
			m_vecTGObjDL[_oidx].m_btDLIdx[t_i] = _dlidx;
			return;
		}
	}

	return;

}


