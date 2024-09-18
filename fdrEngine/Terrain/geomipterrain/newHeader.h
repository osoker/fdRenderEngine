
//================================================================================
//
//  ���µĽṹΪ�洢�µ�ͼ���ļ���ʽ
//
//================================================================================
// 
# pragma once
# include "../../interface/osInterface.h"


# define TGFILE_HEADER   "GEO"

struct  os_newFileHeader
{
	//! �ļ�ͷ��ʶ TGFILE_HEADER
	char       m_szMagic[4];
	//! ��ʱ�汾����10.
	DWORD      m_dwVersion;

	//! TileGround��x��z�����ϵ�����
	WORD       m_wMapPosX;
	WORD       m_wMapPosZ;

	//! �߶���Ϣ��ƫ��
	DWORD      m_dwColHeiInfoOffset;

	//! ��Ʒ��Ϣ�Ĵ�С��ƫ�ơ�
	DWORD      m_dwObjNum;
	DWORD      m_dwObjDataOffset;

	//! ����TileGround�õ���������Ŀ������ƫ��
	DWORD      m_dwTextureNum;
	DWORD      m_dwTexDataOffset;

	//! TileGround�ڵ�Chunk���ݵ�ƫ��
	DWORD      m_dwChunkDataOffset;

	//! ��Ⱦ�ø߶���Ϣ��ƫ��
	DWORD      m_dwVerHeightOffset;

	//! ϸ����ͼ��Alpha��ͼ�Ĵ�С
	DWORD      m_dwAlphaSize;

	//! TG��ÿһ��Chunk����ߵ����͵�����ƫ��
	DWORD      m_dwUtmostChunkHeiOffset;

	//! TG��quadTree���ݵ�offset
	DWORD      m_dwQuadTreeDataOffset;

	//! TG��ÿһ�������������ݵ�ƫ��
	DWORD      m_dwTileAttributeOffset;

	// ���㷨����
	DWORD      m_dwVertNormalOffset;   // ���ݿ��С��Ĭ�ϴ�С

	// ����ɫ
	DWORD      m_dwVertColorOffset;

	DWORD      m_dwHeightInfoOffset;

	// ���ϲ�����idxƫ�ƣ�����ײ����Ϊ��С��λ
	DWORD      m_dwUpmostTexIdxOffset;

	//! �������ݣ�����δ�������ļ���ʽ
	DWORD      m_dwWaterInfoOffset;

	DWORD      m_dwLightMapSize;

	//! ����17�汾�У�hardtexture size offset�����ݣ���ʹ��汾�Ķ�ȡ������ݣ�Ҳ����
	//  ����.
	DWORD      m_dwHardTexSizeOffset;

};

//! �ر��õ�����������
struct  os_chunkTexture
{
 	short     m_wRepeatTimesU;
 	short     m_wRepeatTimesV;

	char       m_szTexName[12];
};

//! ����õ�����ͼ����
# define MAX_TEXLAYER 4

//! һ��TileGround��chunk����Ŀ
# define CHUNKNUM_PERTG   16

//! ����Alpha��ͼ�Ĵ�С
# define ALPHAMAP_SIZE    32

//! �ر��Chunk����
struct  os_chunkData
{
	//! ��Chunk���ܵ��������
	int        m_dwTexNum;
	//! ÿһ��������õ�������������
	int        m_dwTexPtr[MAX_TEXLAYER];
	//! ��ͼ���õ���alpha���ݵ�ƫ��
	int        m_dwAlphaDataOffset;
	//! VER 16 ADD ����ͼ����
	int        m_dwLightMapDataOffset;
};


/** \brief
 *  ��ͼ���������Ϣ���˴��õ�����ϢΪ��Ⱦ����Ϣ��
 */
struct  os_newTGObjectOld13
{
  //!�����������
  char             m_szName[32];

  //! ����ķ��ýǶ�
  float            m_fAngle;

  //!  ��������������ռ��е�λ�á�
  osVec3D          m_vec3ObjPos;

  /** \brief
   *   ��������,���ݲ�ͬ���������ԣ�������ͬ�Ĵ���
   * 
   *   Ŀǰ�Ļ���Ϊ��
   *    0: ��ͨ�����塣
   *    1: ���������塣skinMesh��Ķ���
   *    2: ˮ��ص����塣 Osa����
   *    3: billBoard��ص���Ʒ�� 
   *    4: ��Ч���������������Լ���ʽ�Ĺؼ�֡������Osa�������õ�������.
   *    5: ���Ӷ�������Ч���͡������п��Է��ù̶������Ӷ�����
   *    6: ��Ч�ĺϼ���ʹ�ÿͻ��˵�spe�ļ���River @ 2006-2-14�ռ���
   */
  DWORD	          m_dwType;

  /** \brief
   *  River @ 2006-6-21:����ر���ɫӰ�����Ʒȫ��ɫ��
   *  
   *  ����ر�ɫ��Ӱ����Ʒ�Ĺ�����ɫ�����ֵΪ�㡣
   */
  DWORD           m_dwObjTerrColor;
};

/** \brief
 *  ��ͼ���������Ϣ���˴��õ�����ϢΪ��Ⱦ����Ϣ��
 */
struct  os_newTGObject
{
  //!�����������
  char             m_szName[32];

  //! ����ķ��ýǶ�
  float            m_fAngle;

  //!  ��������������ռ��е�λ�á�
  osVec3D          m_vec3ObjPos;

  /** \brief
   *   ��������,���ݲ�ͬ���������ԣ�������ͬ�Ĵ���
   * 
   *   Ŀǰ�Ļ���Ϊ��
   *    0: ��ͨ�����塣
   *    1: ���������塣skinMesh��Ķ���
   *    2: ˮ��ص����塣 Osa����
   *    3: billBoard��ص���Ʒ�� 
   *    4: ��Ч���������������Լ���ʽ�Ĺؼ�֡������Osa�������õ�������.
   *    5: ���Ӷ�������Ч���͡������п��Է��ù̶������Ӷ�����
   *    6: ��Ч�ĺϼ���ʹ�ÿͻ��˵�spe�ļ���River @ 2006-2-14�ռ���
   *	8: ����
   *	9: ���Դ
   */
  DWORD	          m_dwType;

  /** \brief
   *  River @ 2006-6-21:����ر���ɫӰ�����Ʒȫ��ɫ��
   *  
   *  ����ر�ɫ��Ӱ����Ʒ�Ĺ�����ɫ�����ֵΪ�㡣
   */
  DWORD           m_dwObjTerrColor;

  //!  River @ 2009-4-16:������Ʒ��������Ϣ
  osVec3D          m_vec3ObjectScale;

};


//! Chunk�ڵ�Alpha��LightMap����
//struct  os_alphaALmpData
//{
//	//! �����ܵ�alpha��Ŀ,�洢��ʱ�򣬸���Chunk��Tex����Ŀ��
//	//! �洢��ͬ�����ݴ�С
//	BYTE         m_vecAlpha[MAX_TEXLAYER-1][ALPHAMAP_SIZE*ALPHAMAP_SIZE];
//
//	//! lightmap������,��ӦALPHAMAP_SIZE*ALPHAMAP_SIZE��RGB��Byteֵ
//	//BYTE         m_vecLmpPixel[ALPHAMAP_SIZE*ALPHAMAP_SIZE*3];
//
//public:
//	os_alphaALmpData()
//	{
//		for(int t_i=0;t_i<MAX_TEXLAYER-1;t_i ++ )
//			memset( m_vecAlpha[t_i],0,sizeof( BYTE )*ALPHAMAP_SIZE*ALPHAMAP_SIZE );
//	//	memset( m_vecLmpPixel,0,sizeof( ALPHAMAP_SIZE*ALPHAMAP_SIZE*3 ) );
//	}
//
//};



/** \brief
 *  �Ĳ�����ص����ݽṹ.
 *
 *  ���������ݽṹ�����ڱ�ʾ�ļ��ڵ����ݸ�ʽ,�Ĳ��������ݽṹ�ɳ����ڵ��Ĳ���
 *  ����ṹ������
 */
struct os_quadNodeDataT
{
	//! ��������Ʒ�󣬴˽�����߸߶�
	float    m_fMaxHeight;
	//! �˽�����Ʒ����
	WORD     m_wNodeObjNum;
	//! �˽�����Ʒ��ʼ����
	WORD     m_dwObjStartIdx;
};
struct os_quadTreeDataT
{
	//! ���ͼ����Ʒ����Ŀ
	WORD     m_wExceedObjNum;
	//! ���ͼ����Ʒ������
	VEC_word m_vecExceedObj;
	//! �����˿��ͼ����Ʒ�󣬵�ǰTG��bbox
	osVec3D  m_vecMax;
	osVec3D  m_vecMin;

	//! �����Ĳ�������Ʒ������������Ŀ
	WORD     m_wTGObjIdxNum;
	VEC_word  m_vecObjIdx;

	//! �Ĳ���������Ŀ������ȷ��
	DWORD     m_dwNodeNum;
	std::vector<os_quadNodeDataT>  m_vecNodeData;

};

//! ÿһ������Chunk��Ӧ����߶���߶Ⱥ���Ͷ���߶�
struct os_chunkUtmostHeight
{
	float      m_fMaxHeight;
	float      m_fMinHeight;
};

//! �洢�ļ�����
struct  os_tgFileData
{
	//! ��ײ�͸߶���Ϣ.
	os_TileCollisionInfo   m_vecColInfo[128*128];

	//! ÿһ�����ӵ���������
	BYTE                  m_vecAttribute[64*64];

	//! ��Ʒ�����ݿ飬���ļ�ͷ������Ʒ����Ŀ
	os_newTGObject*        m_vecObj;

	//! ����TileGround�õ���ChunkTexture����,��Ŀ���ļ�ͷ��
	os_chunkTexture*       m_vecChunkTex;

	//! TileGround�ڵ�Chunk����
	os_chunkData           m_dwChunkData[CHUNKNUM_PERTG];

	//! ��Ⱦ���õ��ĸ߶���Ϣ����
	float                 m_vecVerHeight[65*65];

	//! ÿһ��TG�ڵ�Chunk���޸߶���Ϣ
	os_chunkUtmostHeight   m_vecUtmostHeight[CHUNKNUM_PERTG];

	//! Alpha���ݺ͹���ͼ���ݵ����ݿ飬��ChunkΪ��λ������֯
	//os_alphaALmpData       m_vecAlphaLmpData[CHUNKNUM_PERTG];

	//! �Ĳ���������ص����ݣ�Ӧ���ɳ������Ĳ����ṹֱ�ӵ��룬�˴�������ʾ��
	os_quadTreeDataT       m_sQuadTree;

	os_tgFileData(){ m_vecObj = NULL;m_vecChunkTex = NULL; }
	~os_tgFileData()
	{
		SAFE_DELETE_ARRAY( m_vecObj );
		SAFE_DELETE_ARRAY( m_vecChunkTex );
	}
	
};

