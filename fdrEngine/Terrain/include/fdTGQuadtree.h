//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTGQuadtree.h
 *
 *  His:      River created @ 2005-11-16
 *
 *  Des:      ÿһ��TG��Ӧ���Ĳ���������ݽṹ,sceneGraph����Щ���ݽṹ����.
 *   
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "fdQuadtreeTemplate.h"

# include "../geoMipTerrain/MapTile.h"

class  osc_newTileGround;
struct os_ViewFrustum;
struct os_chunkUtmostHeight;
struct os_dlInfo;


/** \brief
 *  ʹ��������ݽṹ��Ԥ����TG�ڵ���Ʒ�ɼ�����Ϣ
 *
 *  ���ڿ�64*64��ͼ����Ʒ����洢��������Ʒ�б�ʹ��ȷ�ļ�����Ʒ�Ŀɼ�����Ϣ
 */
struct fd_sceneObjBbData
{
	//! obj��bbox.
	os_bbox    m_sObjBox;

	//! obj����������������
	WORD      m_wObjIdx;

	//! ����.
	fd_sceneObjBbData()
	{
		m_sObjBox.reset_bbox();
		m_wObjIdx = -1;
	}
};
typedef std::vector<fd_sceneObjBbData> VEC_objBbData;


//! ÿһ��quadTree����Ӧ�����ݽṹ
class fd_quadTreeNodeData
{
public:
	//! ��ǰ����aabbox,��������Ʒ�ĸ߶�
	os_aabbox       m_sNodeBox;

	/** \brief
	 *  ������Ʒ��ĸ߶�.
	 *
	 *  һ��QuadtreeNode��������߸߶�����,һ���Ǵ���ر����߸߶�����,
	 *  һ���Ǽ�������Ʒ֮�����߸߶�����.�������ܻ����������Ҫ��һ��
	 *  Node�ڵ���Ʒ������Ⱦ,������Ҫ�����chunk�ĵر������Ⱦ�����,�Ż�
	 *  ��Ⱦ����.
	 */
	float           m_fMaxHeight;

	//! ��������յ�Ҷ��㣬�������Ϊ���Ӧ������mapBlock�ڵ�Chunk����
	WORD            m_wRenderChunkIdx;

	//! ��ǰ����Ӧ�Ŀ���Ⱦ��ƷId����ʼ��������Ŀ��
	WORD            m_wObjStartIdx;
	WORD            m_wObjNum;

	fd_quadTreeNodeData()
	{
		memset( &m_sNodeBox,0,sizeof( os_aabbox ) );
		m_fMaxHeight = 0.0f;
		m_wRenderChunkIdx = 0xffff;
		m_wObjStartIdx = 0xffff;
		m_wObjNum = 0xffff;
	}

};




typedef os_qtNodeTemplate<fd_quadTreeNodeData>*  qtNode_ptr;


//! ÿһ��TG��Ӧ���Ĳ������ݽṹ
class osc_tgQuadtree : public os_quadtreeTemplate<fd_quadTreeNodeData>
{
private:
	//! ��ǰquadtree��Ӧ��TGָ��
	osc_newTileGround*      m_ptrTGPtr;

	//! ȫ�ֵ�vfָ��.
	static os_ViewFrustum*  m_ptrVf;


	//! �����Ĳ����õ�����Ʒ��������
	VEC_word               m_vecObjIdx;


	/** \brief
	 *  ÿһ��64*64�ĵ�ͼ�����ͼ�ڵ���Ʒ�����˵�ͼ��bbox,���ݳ�������Ʒ��bbox,
	 *  �����µĵ�ͼbbox.
	 *  
	 *  �ټ�¼һ��������ͼbbox����Ʒ�б���vf�Ƚϣ������ڲ���Ⱦ�ĵ��ε������£�
	 *  ��Ⱦ��Щ��Ʒ��
	 */
	os_aabbox               m_sTotalBox;

	//! ATTENTION TO OPP:���յ����ݣ��������Ĳ�����ֻ�洢os_bbox��ָ�룬��ʡ�ڴ桡
	VEC_objBbData           m_vecExceedObjIdx;

	//! ��������ѡʱ���ҳ����ܺ������ཻ��Chunk�б�
	int                    m_iPickChunkNum;
	os_tileIdx              m_arrPickChunk[MapTile::CHUNKS*MapTile::CHUNKS];
	osVec2D                 m_arrChunkXZ[MapTile::CHUNKS*MapTile::CHUNKS];
	osVec3D                 m_vec3RayStart,m_vec3RayDir;



	//! River @ 2006-7-13:�Ż�������ر�����ײ�����..�Ƿ���ʹ���߶����ཻ����.
	bool                    m_bPickWithLs;
	osVec3D                 m_vec3LsEnd;

	//! ��̬����ص�����
	int                     m_iAffectDlNum;
	os_bsphere              m_vecDlBSphere[MAX_LIGHTNUM];
	osColor                 m_vecLightColor[MAX_LIGHTNUM];
	int                     m_iDlIdx[MAX_LIGHTNUM];

private:

	//! Ϊ��һ��������BBox.
	void                 allot_aabbox( qtNode_ptr _nodePtr,
		                  const os_chunkUtmostHeight* _ptrHei,os_aabbox& _box );

# ifdef _DEBUG
	//! ��Ⱦÿһ������aabbox.
	void                 draw_nodeAabbox( 
		                   qtNode_ptr _nodePtr,bool _solid = false );
# endif 

	/** \brief
	 *  ��һ��������Ҵ˽�㵱ǰ���ڵ�Quadtree leaf node.
	 *
	 *  \param _useY  _pt��Yֵ�Ƿ������ԣ���������룬��ֻҪx,zֵ�ڽ���ڣ��˶����ڽ����
	 */ 
	qtNode_ptr            find_node( qtNode_ptr _node,osVec3D& _pt,bool _useY = true );

	//! ��һ����㿪ʼ�õ���ǰ�������vf�ڵĵ�ͼ��������.
	void                  get_inVfTile( qtNode_ptr _node,osMatrix& _reflMat );

	//! Ϊÿһ��������Chunk����
	void                 allot_chunkIdx( void );

	//! Check&Add quadtree Obj��ʾ����
	void                 check_nodeObjDis( qtNode_ptr _node,osMatrix& _reflMat );


	//! �ݹ�õ����к������ཻ��Chunk�б�
	void                 get_chunkList( qtNode_ptr _node );

	//! ���ݶ�̬�����ɫ��λ�÷�Χ�������㵱ǰnode�Ķ�̬����ɫ
	void                 cal_nodeObjDLight( qtNode_ptr _node );

	//! �ݹ���ÿһ����㣬�Ƿ�ʹ����sphere�б���ײ
	void                 col_withSphere( qtNode_ptr _node );

public:
	osc_tgQuadtree();
	~osc_tgQuadtree();

	/** \brief
	 *  ���ڴ��е����Ĳ�������
	 *
	 *  �㷨������
	 *  1: ������򵥵��Ĳ������ݣ�ÿһ���Ĳ������ĸ��ӿ�ʼ��ͽ�����
	 *  2: �����Ĳ�����aabbox���ݺ�obj���ݡ�
	 *  3: ���ݸ߶����ݣ�����ÿһ������aabbox����
	 */
	BOOL                 load_quadtreeDataFromMem( 
		                    osc_newTileGround* _ptr,const BYTE* _dstart );

	//! ���㵱ǰ��vf�ڵĽ����Ŀ
	void                 cal_visualTileSet( osMatrix& _reflMat );

	//! TEST CODE:������Ⱦ��ǰ��quadTree. 
	void                 draw_quadTree( void );

	/** \brief
	 *  ���ݴ����λ�ã��õ���ǰλ�����ڵ�quadTree Chunk�ڵ���Ʒ�����б�
	 *
	 *  ���������Ҫ���ڵ�ס�����Ʒ�İ�͸����
	 *  \param _pos �����������������λ�á�
	 *  \param _objList λ�����ڵ�Chunk����Ʒ�������б�
	 *  \param _objNum  ��������������ɵ������Ʒ�����������ڵ�Chunk����Ʒ������Ŀ
	 *  \return int     �����Ѿ��ҵ���λ����Ŀ,��������������ڴ�tg,�򷵻�2,���
	 *                  �����ڣ������㡣
	 */
	int         get_chunkObj( osVec3D& _pos,
		                   int* _objList,int& _objNum,int _maxObjNum );

	//! get_chunkObj�����Ĳ��䣬���ڵõ�����chunk�߽����Ʒ�б�
	int         get_exceedChunkObj( osVec3D& _spos,
		osVec3D& _epos,int* _objList,int& _objNum,int _maxObjNum );


	/** \brief
	 *  ����һ��λ��,�õ���λ������chunk��������Ʒ���б�
	 *
	 *  \param osVec3D& _pos  ������Ҫ�õ�����Ʒ�б��λ��.
	 *  \param int* _objList  ������Ʒ��id�б�.
	 *  \param int _maxObjNum ������Ʒid�б�����ռ�.
	 *  \return int           ���ص���Ʒ����.
	 */
	int         get_posInChunkObj( osVec3D& _pos,int* _objList,int _maxObjNum );

	/** \brief
	 *  ����һ��λ�ã��õ���λ���Ƿ��quadTree����tileGround�İ�Χ���ཻ�����õ�
	 *  ���ĸ���Ʒ�ཻ��
	 *
	 */
	int         get_posExceedQuadObj( osVec3D& _pos,int* _objList,int _maxObjNum );

	
	/** \brief
	 *  ����ѡʱ���Ĳ�������������ཻ����Chunk�б�
	 *  
	 *  \param const os_tileIdx*&  �ϲ㴫���ָ�룬���ڵõ������ཻ��chunk�б�
	 *                             ��������ཻ��chunkΪ�㣬��ֵ�ں����ڸ�ΪNULL
	 *  \param const osVec2D*& _chunkXZStart �ϲ㴫���ָ�룬���ڵõ�ÿһ���ཻChunk
	 *                                       ��x,z�����ϵ���Сֵ,�����ϲ�ļ���
	 *  \param osVec3D* _lineSegEnd �����Ϊ��,��ʹ���߶κ͵�ǰ��chunkList���ཻ����.
	 */
	int                    get_pickChunkList( osVec3D& _rayStart,osVec3D& _rayDir,
		                      const os_tileIdx*& _chunkArr,const osVec2D*& _chunkXZStart,
							  osVec3D* _lineSegEnd = NULL );


	/** \brief
	 *  ����̬��Ӱ����һ����ͼ�ڵ�Chunk.
	 *
	 *  ����ÿһ����̬���Ӱ�췶Χ��ȷ�ϵ�ǰ��Chunk�Ƿ��ܶ�̬��Ӱ��
	 *
	 *  \param _bsphere ����Ҫ��ײ����bsphere���ݣ����4��
	 */
	void                   set_dlAffectChunk( const os_dlInfo* _dlInfo );

	//! ����������Ƿ����ǰ��QuadTree�ཻ
	bool                   ray_intersect( osVec3D& _rayStart,osVec3D& _rayDir );

};






