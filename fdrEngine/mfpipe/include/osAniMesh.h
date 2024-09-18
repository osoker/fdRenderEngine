/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osAniMesh.h
 * 
 *  His:      River created @ 2004-5-12
 *
 *  Desc:     ������ص�mesh�����ݽṹ���Ӵ�6�ĵ�������еõ����ݡ�
 *  
 *  
 *  ��ȡ�����ϣ��������У�ȡ�����У��������¡�
 *   
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "osShader.h"

# include "../../interface/osInterface.h"
# include "../../interface/miskFunc.h"
# include "../../tools/include/fadeMgr.h"
# include "osPolygon.h"


class     osc_middlePipe;


//! aniMesh���õ���vector.
typedef   std::vector<os_bsphere>       VEC_bsphere;
typedef   std::vector<os_objectVertex>  VEC_objectVertex;



//! �洢��������ɫ��Ϣ���൱��ARGB��DWORDֵ.
struct os_colorQuad
{
	unsigned char m_cAlpha;
	unsigned char m_cRed;
	unsigned char m_cGreen;
	unsigned char m_cBlue;
};



/** \brief
 *  �洢��ÿһ��mesh�Ĳ��ʣ���Ϊÿһ����Ⱦ����Ҫ�ı���ʣ�
 *  �������ֵ�п���ȥ����
 *
 *  ��Ⱦ��������˵Ķ���ʱ����Ҫʹ�ò��ʶ�����
 */
struct os_material
{
	os_colorQuad m_dwDiffuse;
	os_colorQuad m_dwAmbient;
	os_colorQuad m_dwSpecular;
	os_colorQuad m_dwEmissive;
	float	     m_fShininess;

};


//! һ����Ķ�Ӧ�Ķ���index. 
struct os_refFace
{
	WORD  v_index[3];
};



//!��meshInstance��������
class     osc_aniMeshIns;


//! ���ʶ�Ӧ����ͼ�ļ����ĳ��ȡ�
#define FILENAME_LEN  64


/** \brief
 *  ����һ��dip�Ľṹ��
 *
 *  ÿһ���������õ����mesh,ÿһ��MeshFace�൱��һ�ֲ��ʣ�
 *  MeshFace���ж�Ӧ�Ĳ��ʺ�����
 */
struct os_aniMFace
{
	BYTE        m_cFlag;   //==0: normal,   ==1:blend
   os_material   m_Material;
   D3DMATERIAL9  m_deviceMat;
     
	char        m_szTexName[FILENAME_LEN];          //ֻ�����ļ����� ��·��
	int         m_nFace;
	os_refFace*   m_ptrFace;

	//@{
	//! ���ʹ�ò��ʶ����Ļ����˴��ж��material 
	int          m_iMaterialNum;              
	os_material    *m_ptrMaterialList; 
	float         *m_ptrfTimekey;
	VEC_float      m_vecAlpha;
	//@} 

	//! ��ǰ�Ĳ����Ƿ���alpha��صĲ��ʡ�
	bool           m_bAlphaMat;

public:
	os_aniMFace( void );
	~os_aniMFace( void );

	//! ���ļ��е���aniMesh����,�������ݣ����ƶ�����ָ�롣
	bool          load_aniMeshFromMem( BYTE*& _ds,bool _bAniMat );

	//! �õ���ǰmaterial���ݡ�
	void          get_currentMaterial( D3DMATERIAL9& _mat,osc_aniMeshIns* _ins );

	//! �ͷŵ�ǰ��aniMFace.
	void          release_aniMFace( void );

};





/** \brief
 *  max�е�AffineParts�ṹ�����ڹ���һ�������ľ���
 *
 *  ������Ҫ֡���ֵ������ʹ�÷����ڲ�ֵ�ľ�����ʽ��
 *  ÿһ����Ա�ĺ�����max sdk��AffineParts�Ķ��塣
 */
struct  os_AffineParts
{
	osVec3D   t;
	osQuat    q;
	osQuat    u;
	osVec3D   k;
	float     f;
};


struct   os_vbufFillData;

/** \brief
 *  ��Ӧmax�������ݽṹ��
 *  
 */
struct os_aniNode
{
	//! �ǲ��Ǽ������ݽ�㡣
	bool         m_bIsGeoNode;

	// 
	//! ʹ��affineParts�ṹ��ʹ�ö����ṹ����������
	int             m_iAffinePartsNum;
	osMatrix*        m_arrKeyMatrix;
	os_AffineParts*  m_arrAffineParts;
	float*          m_arrFAffTimeKey;


	// 
	//!  ���Ϊ 1�� û�б��ζ����� ���>1, ��
	int                m_iMorphFrames;            
	int                m_iVertex;

	//! ��ͨ�Ķ������ݡ�
	os_objectVertex*    m_arrVertex;  

	//! ���ʹ�õ��ŽǶ�����ʹ����������������ݡ�
	os_objectColVertex*  m_arrColVertex;

	//! ����Ϊ��λ, ÿһ���Ӧ��ʱ��
	float*            m_fTimeKey;   

	//! �����morphFrame,�������ۺϺ��������顣
	VEC_objectVertex    m_vecAPVertex;


	//! ��ǰ����Ӧ��mesh���ݡ�
	int               m_iNumOfMesh;
	os_aniMFace*        m_arrMesh;


	//! ��ǰ�Ľ���Ƿ�ȫ��Ϊ͸����Ʒ����������������ֵ��
	bool              m_bTotalAlphaFace;
	//! ��ǰ�Ľ�����Ƿ���͸����Ʒ��
	bool              m_bExistAlphaFace;


	//! the matrix to trans parent matrix to current matrix...
	osMatrix           m_matNodetm;   

	//! ��ǰani Node��һ֡��Bounding Box.
	os_aabbox            m_bbox;

	//! ��ǰani node��Bounding Sphere.
	os_bsphere         m_bsphere;

	//@{
	// ����Ϊ��ǰnode�õ����豸������ݡ�
	/** \brief
	 *   aniNode�õ��Ķ��㻺������
	 *   
	 *   �˴�Ϊ���㻺������id.���м�ܵ��������id.
	 */
	int               m_iVerBufId;

	/** \brief
	 *  ��ǰbuffer�У�����Ŀ�ʼλ��
	 *
	 *  �ھ�̬�Ķ��㻺�����У���ԶΪ0,�ڶ�̬�Ķ��㻺������ÿһ�ζ����б仯��
	 */
	int               m_iStartVer;


	/** \brief
	 *  aniNode�õ���index Buffer id.���м���������index buffer.
	 *
	 *  ATTENTION TO OPP:����ʹ��һ��index buffer����䵱ǰaniMesh��
	 *  ���е�idx���ݡ�
	 */
	VEC_int            m_vecIdxBufId;

	/** \brief
	 *  ÿһ��nodeMesh�õ���shaderId.
	 */
	VEC_int            m_vecShaderId;

	//@} 

	//! ��Ⱦ��ǰ��nodeʹ�õ�����Ⱦ����Ρ�
	os_polygon         m_sPolygon;


	//! �����ͼ�ж������Ƿ���Ҫ����ͼ����uv������ֵ��
	bool               m_bNotInterpolate;

	//! ��ǰ��������Ƿ�ʹ�õ���ɫ���ݡ�
	bool               m_bColorVertex;

	/** \brief
	 *  ��ǰ���������ɶ�����Ҫ�����ʱ��
	 *
	 *  ����affine Time ��morph Time����ʱ������.
	 */
	float              m_fPlayTotalTime;

	// River @ 2006-7-22: ����ؼ�֡����
	bool               m_bUseKeyMat;

private:
	//! ��䵱ǰaniNode��Ⱦʹ�õĶ���Ρ�
	void              fill_rPoly( void );

	//! �õ���ǰ��poskey��λ�á�
	void              get_posKey( osMatrix& _mat,osc_aniMeshIns* _ins );

	//! �õ���ǰ��rotkey���ݡ�
	void              get_rotKey( osMatrix& _mat,osc_aniMeshIns* _ins );

	//! �õ���ǰ��scaleKey���ݡ�
	void              get_scaleKey( osMatrix& _mat,osc_aniMeshIns* _ins );

	//! ����morph Frame,��䴦���Ķ��㡣
	bool              process_morphFrame( osc_aniMeshIns* _ins );

	//! ������objectVertex���в�ֵ��
	void              objver_lerp( os_objectVertex* _res,
		               os_objectVertex* _src,os_objectVertex* _dst,float _factor );


	/** \brief
	 *  �ͷŵ�ǰ��aniNode����
	 */
	void              release_aniNode( void );

	//! �õ���ǰ��㲥����Ҫ��ʱ�䡣
	float             get_playNodeTime( void );

	//! ˮ����ص�״̬����
	void              waterObj_rsSet( osc_aniMeshIns* _ami,osMatrix& _worldMat );



public:
	os_aniNode( void );
	~os_aniNode( void );

	//! ���ļ��е���aniNode����,��ƫ�����ݵ�ָ�롣
	void       load_aniNodeFromMem( BYTE*& _ds,bool _bAniMat );



	//! ������ǰnode���豸������ݡ�
	bool       create_devDepData( osc_middlePipe* _pipe,
		         int _vbId,os_vbufFillData* _vbfill,int& _vbNum );


	//! �ͷŵ�ǰnode���豸��ء����ݡ�
	bool       release_devdepData( osc_middlePipe* _pipe );

	//  TEST CODE:
	//! ȷ��shaderId�Ŀ����ԡ�
	bool       assert_devdepData( bool _assert = true );



	/** \brief
	 *  �õ���ǰ��Bounding Sphere,��Ʒ���꣬��Ҫת��������ռ䡣
	 */
	void       get_bsphere( os_bsphere& _bs );

	/** \brief
	 *  ��Ⱦ��ǰ��node,ʹ��aniMeshInstance.
	 *
	 *  ʹ���м�ܵ���polygon�������????
	 *  ֻ��Ҫʹ��һ��polygon,��Ⱦ��ͬ��matʱ����Ҫ����idx buf id��shader id.
	 */
	bool       render_aniNode( osc_aniMeshIns* _ami,
		         int _idx,bool _alpha,float _faderate = 1.0f,bool _waterObj = false );

	bool       render_FogAniNode( osc_aniMeshIns* _ami,int _idx );

	/** \brief
	 *  �򵥵���Ⱦ��������ȾaniMesh��ͷһ֡��ָ����λ�ú����š�
	 * 
	 *  ֻ������ֻ��һ�����ʵļ�ģ�ͣ����������Զɽ��
	 *
	 *  \param float _alphaV Ҫ��Ⱦ����͸����
	 */
	bool       render_aniNode( osc_middlePipe* _pipe,float _rotAgl,
		            osVec3D& _pos,osVec3D& _scale,float _alphaV );
	//@{
	// Windy mod @ 2005-8-26 12:25:48
	//Desc:Test Code ��ӡ���㣬�������Ⱦ��Ϣ�����ڵ��Է�����.
	void		DumpRenderInfo(osc_middlePipe* _pipe);

	//@}
};



/** \brief
 *  root node struct of the scene...
 */
class os_rootNode
{
private:
	/** \brief
	 *  �õ���ǰ������ڲ��ܹ��������Ŀ��
	 *
	 *  ÿһ��rootNodeʹ��һ����Ķ��㻺������
	 *  ����н���morphFrame��Ŀ����1,���Node
	 *  ��ʹ�ô�Ķ��㻺����
	 */
	int         get_totalVerNum( void );

public:
	//! ��ǰ�����м��ν�����Ŀ��
	int         m_iChildNum;

	//! �����м��νṹ��ָ�롣
	os_aniNode*  m_arrNodelist;

	//! ��ǰAniMesh�õ��Ķ��㻺������������
	int         m_iVBId;

public:
	os_rootNode( void )
	{
		m_arrNodelist = NULL;
		m_iVBId = -1;
	}

	~os_rootNode( void )
	{
		SAFE_DELETE_ARRAY( m_arrNodelist );
	}


	/** \brief
	 *  ����ǰrootNode���豸������ݡ�
	 *
	 *  ���������ÿһ�����Ķ������ݺ��������ݡ�
	 *  
	 */
	bool        create_devdepData( osc_middlePipe* _pipe );


	//  ATTENTION : �ͷ�һ�����ʱ�����ͷ��豸�����Դ���ݣ����ͷ��ڴ����ݡ�
	//! �ͷŵ�ǰ�����ͷ�������ݡ�
	void        release_devdepData( osc_middlePipe* _pipe );

	//! �ͷŵ�ǰ�Ľ��.
	void        release_node( void );

};


/** \brief
 *  ������Ʒ�࣬��ani�ļ��е������Ƕ�����Ʒ��
 *
 *  �������Ҫ�洢�豸��ص����ݣ����м�ܵ��洢������
 *  �豸������ݡ�
 *  
 */ 
class osc_aniMesh : public os_refObj
{
	friend class osc_aniMeshIns;
private:
	//! ��ǰmesh��Ӧ���ļ�����
	s_string         m_szFname;
	DWORD            m_dwNameHash;

	//! ��ǰmesh��Ӧ�����ݡ�
	os_rootNode  m_sRootNode;

	//! ��ǰ��ani Mesh�Ƿ��õ��˲��ʶ�����
	bool         m_bAniMat;


	//! ��ǰ��aniMesh�Ķ�����ʽ�Ƿ���Ҫ����֡����в�ֵ��
	bool         m_bNotInterpolate;

	//! ��ǰ��aniMesh�Ƿ�ʹ���˶�����ɫ����,���������ɫ����ʹ��������ļ���ʽ��
	bool         m_bColorVertex;


	/** \brief
	 *  ��ǰ��aniMesh�ͱ�������Ⱦ��ʽ��
	 *
	 *  Ŀǰ��6����Ⱦ��ʽ��
	 */
	BYTE         m_btRStyle;

	//! �ļ�����ʱȷ���Ƿ��ܻ�����Ӱ�졣
	bool         m_bEffByAmbient;

	//! �Ƿ���Ҫ��Ⱦ�ظ�����
	BOOL         m_bRepeatTexture;

	//! �Ƿ�ʹ�ùؼ�֡����
	BOOL         m_bKeyMatrix;

	//! ��ǰ��mesh�Ƿ���ʹ�á�
	bool         m_bInuse;

	//! �Ƿ�����û����ڴ�.
	bool         m_bCanSwap;

	//! ���ŵ�ǰ��aniMesh��Ҫʱ��.
	float        m_fPlayTotalTime;

	//! ����aniMesh��,���㵱ǰMesh��aabbox
	os_aabbox     m_sAabbox;

private:
	/** \brief
	 *  ������ǰmesh���豸������ݡ�
	 */
	bool        create_devdepRes( osc_middlePipe* _pipe );


	//! ���ļ��е������ǵ�mesh���ݡ�
	bool        load_meshFromfile( const char *_fname );

	//! ����mesh��aabbox.
	void        cal_aabbox( void );

protected:
	/** \brief
	 *  �ͷŵ�ǰ��Դ�Ĵ��麯������release_ref�������á�
	 */
	virtual void release_obj( DWORD _ptr = NULL );


public:
	//! ��ǰmesh��node��ʹ��morphFrame�����ֵ������Ӧ��aniNode.
	int          m_iMaxMorph;
	os_aniNode*   m_ptrMaxMorphNode;

	//! ��ǰmesh��ʹ��affine������Ŀ�����ֵ������Ӧ��aniNode.
	int          m_iMaxAffine;
	os_aniNode*   m_ptrMaxAffineNode;


public:
	osc_aniMesh();


	//! ��Ⱦ��ǰ��mesh,�õ���Ⱦ��ǰ��mesh��Ҫ���ݡ�

	/** \brief
	 *  ���ļ��д������ǵ�mesh���ݡ�
	 *  
	 *  �������ݵ�ʱ�򣬴����豸��ص����ݡ�
	 */
	bool          create_meshFromfile( os_aniMeshCreate& _mc );

	//! ��Ⱦ��ǰ��aniMesh.
	void          render_aniMesh( osc_aniMeshIns* _ami,
		            bool _alpha,float _fadeRate = 1.0f,bool _waterObj = false ,bool _fogObj = false);

	/** \brief
	 *  ��ȾaniMesh�ĵ�һ֡��ָ����λ�á�
	 *
	 *  \param osVec3D _scale Ҫ����aniMesh����ֵ��
	 *  \param float _alphaV Ҫ��ȾaniMesh��͸���� 
	 */
	void          render_aniMesh( osVec3D& _pos,float _rotAgl,osVec3D& _scale,
		           osc_middlePipe* _pipe,float _alphaV = 1.0f );

	//! ��ǰ��mesh�Ƿ���ʹ�á�
	bool          is_aniMeshInUse( void );



	//! ��ǰ��mesh�Ƿ�ʹ�ò��ʶ�����
	bool          is_aniMeshAniMat( void );

	os_rootNode*   get_rootNodePtr( void ) { return &m_sRootNode; } 

	//! �õ���ǰmesh�����֡�
	const char*    get_meshName( void )    { return m_szFname.c_str(); } 
	DWORD         get_nameHash( void )    { return m_dwNameHash;}

	//! �õ���ǰmesh����������Ҫ��ʱ�䡣
	float          get_playTotalTime( void ) { return this->m_fPlayTotalTime; }

	//! �õ���ǰmesh��aabbox.
	const os_aabbox*     get_aabboxPtr( void ) { return &m_sAabbox; }

	// 
	void           set_swap( bool _s )    { this->m_bCanSwap = _s; }
	bool           get_swap( void )       { return this->m_bCanSwap; } 

	// TEST CODE:
	//! ȷ��aniMesh��Ӧ��shaderid��Ч��
	bool           assert_shader( bool _assert = true );

	//! �õ���aniMesh��Ӧ�ĵ�һ��effect�������ϲ�õ�ˮ���effect.
	osc_effect*    get_effect( void );

};

typedef std::vector< osc_aniMesh >    VEC_aniMesh;



/** \brief
 *  ��Ⱦʱʹ�õĶ�����Ʒʵ����
 *  
 *  ����������ݡ�
 *  
 *  MeshInsʹ��lru׼��
 *  
 */ 
class osc_aniMeshIns : public os_createVal
{   
	//! ��Ⱦnode��ʱ�򣬿���ʹ���������ݡ�
	friend struct    os_aniNode;    
	friend struct    os_aniMFace;

	//! TEST CODE:
	friend class     osc_meshMgr;
private:
	//! ��ǰaniMesh��Ӧ��aniMeshָ�롣
	osc_aniMesh*     m_ptrMesh;

	/** \brief
	 *  ��ǰmeshIns��Ⱦʱʹ�õ���ȫ�ֹ��ա�
	 *
	 *  ��̬��Ʒ�Ķ�̬��Ҳ��Ҫʹ�����������ģ�⡣
	 */
	DWORD           m_dwAmbientL;

	//! ��ǰ��Ʒ�Ƿ���ȫ�ֹ��Ӱ�죬Ĭ��Ӱ�졣
	bool            m_bEffectByAL;


	//! ��ǰ��aniMesh���ŵ���ǰ��ȥ��ʱ�䡣
	float           m_fEleTime;

	//! ѭ�����ŵĴ�����
	int             m_iLoopTimes;

	//! ��ǰ��positionFrame��
	int             m_iCurPosFrame;
	float           m_fPTime;
	float           m_fPFactor;


	//! ��ǰ��morphFrame.
	int             m_iCurMorphFrame;
	float           m_fMTime;
	float           m_fMFactor;

	//! ���ʶ�����ص�.
	int             m_iCurMatFrame;
	float           m_fMatTime;
	float           m_fMatFactor;


	//@{
	//! ���ڼ���Ƿ��Ѿ�������һ����Ʒ������
	bool            m_bOverLoop;
	//@} 


	/** \brief
	 *  ����ռ���meshIns��λ�ú���ת����
	 *  
	 *  ��Щ������Ҫ����Ʒ�Ķ����������ʹ�á�
	 */
	osVec3D         m_vec3Pos;
	float           m_fRotAgl;

	//! ��ǰ�����Ĳ������͡�
	ose_amPlayType   m_ePlayType;

	//! ����ǰ�ʱ�䲥�ţ����ŵ�ʱ�䡣
	float           m_fPlayTime;

	//! ����ǰ��β��ţ����ŵĴ�����
	int             m_iPlayFrames;


	//! ��ǰ��aniMeshIns�Ƿ���ʹ���С�
	bool            m_bInuse;

	//! ��ǰ��instance������ɺ��Ƿ��Զ�ʹ��ǰ��instance��Ч��
	bool            m_bAutoDelete;

	//! ��ǰMesh�����ź���ת
	float           m_fSizeScale;

	//@{
	//! River mod 2008-7-16:�������Ŷ������೤ʱ�����ɵ�ǰ��sizeScale��׼���ŵ�һ���µĻ�׼
	float           m_fNewScale;
	float           m_fBaseScale;
	float           m_fScaleTime;
	float           m_fScaleEleTime;
	bool            m_bInScaleState;
	//@}

	/** \brief
	 *  ��ǰaniMeshInsÿһ������Bounding Box.
	 *
	 *  �˴�Ϊ��һ֡��bounding sphere,�жϿɼ��Ե�ʱ��Ҳ����һ֡��
	 *  Bounding sphereΪ����,������̫�����
	 */
	VEC_bsphere     m_vecBs;
	//! ��ǰIns��bsphere.
	os_bsphere      m_bsphere;


	//@{
	//  ������ã���ʹ�ô˾������������λ�ã���ʹ��λ�����õ�����
	//! ��ǰins��ת������
	osMatrix        m_matTrans;
	//@} 


	//! ����ʱ�ٶ�ʱ��ѹ���ȣ����������ŵĶ�����ʹ�ô����ݡ�
	float           m_fTimeScale;
	//@{
	//! ��ǰ�Ƿ���fade״̬,0������fade״̬,1 fadeIn.2 fadeOut.
	int            m_iFadeState;
	float          m_fFadeTime;
	float          m_fCurEleFadeTime;
	float          m_fCurFadeRate;

	//! river added @ 2009-10-10:������osa��͸��
	float          m_fMinFade;
	//@}

	//! ��ǰ�Ƿ�����.
	bool           m_bHide;

	//! top layer render
	bool           m_bTopLayer;
	//! River added @ 2010-1-20:
	//! ��ǰ��ins�Ƿ��Ѿ���Ⱦ,�����������ϵ�������Ч��
	//! �����������ڽ�������Ⱦ��ʱ�����
	bool           m_bRenderedInCurFrame;

	//! �Ƿ���ˮ�������Ʒ
	bool           m_bWaterAni;
	//! �Ƿ����������Ʒ
	bool           m_bFogAni;
	int            m_FogShaderId;
	
	//! River @ 2010-6-29:��Ļ�ⲨЧ���õ�.
	bool           m_bIsScreenWave;

private:
	//! �������ǵ�meshIns
	void            reset_meshIns( void );

	//! �õ���ǰmeshIns�õ���������ת��λ�ƾ���
	void            get_transMatrix( osMatrix& _mat );

	//@{ 
	//  ���ӵ�֡���ֵ���㣬���ݲ�ͬ�Ĺ��ã����в�ͬ�Ĵ���

	//! ����posKey���ݡ�
	void            process_posKey( os_aniNode* _ptrNode,float _etime );

	//! ����morphFrame.
	void            process_morphFrame( os_aniNode* _ptrNode,float _etime );

	//! ����matFrame
	void            process_aniMatFrame( os_aniNode* _ptrNode,float _etime );
	//@} 

	/** \brief
	 *  ������������ŵĻ����Ƿ��Ѿ�������
	 *
	 *  \return bool �������true,�򲥷���ɡ�
	 */ 
	bool            finish_playWithFrame( void );

	//! �����ʱ�䲥�ţ��Ƿ��Ѿ�������ɡ�
	bool            finish_playWithTime( float _et );

	//! framemove��ǰ������״̬
	void            scale_frameMove( void );
	//! frameMove��ǰosa��fade״̬.
	void            fade_frameMove( void );

public:
	//! �м�ܵ���ָ�롣
	static osc_middlePipe*  m_ptrMPipe;


public:
	osc_aniMeshIns();
	~osc_aniMeshIns();

	/** \brief
	 *  ���ļ��д���osc_aniMesh����ʵ��.
	 *
	 *  ��Ҫ�����Ĳ���������λ�ã���ת���򣬲������͵���Ϣ��
	 */
	WORD            create_aniMeshIns( osc_aniMesh* _mptr,os_aniMeshCreate& _mc,bool _topLayer = false );

	/** \brief
	 *   ��Ⱦ��ǰ�Ķ�̬meshʵ����
	 */
	bool            render_aniMeshIns( float _etime,bool _alpha ,bool _depth=false);

	/** \brief
	 *  ���¿�ʼ��ǰ�Ķ�����
	 */
	void            restart_aniMeshIns( void );

	/** \brief
	 *  ��ʼ��ǰosaAni��fade����.
	 */
	void           start_fade( float _time,bool _fade = true,float _minFade = 0.0f );


	/** \brief
	 *  �õ���ǰmesh Ins��Bonding sphere.
	 */
	os_bsphere*      get_bsphere( void );

	//! �õ�os_bbox
	void            get_bbox( os_bbox* _box );

	//@{
	//! ���ú͵õ���ǰmeshIns�Ĺ���ϵ����
	void            set_light( DWORD _light );
	DWORD           get_light( void );
	//@} 

	/** \brief
	 *  ��ǰ��aniMeshIns�Ƿ���ʹ���У�
	 */
	bool            is_amInsInuse( void );


	//! ��ǰ��meshIns��Ⱦ�ǲ���ʹ�úͱ�����ӵ���Ⱦ��
	int             get_RenderStyle( void );

	//! ��ǰ��Mesh�Ƿ��ǵ���ɫ��mesh
	bool            is_colorVertexMesh( void );

	/** \brief
	 *  ɾ�����ǵ�ǰ��meshIns.
	 */
	void            delete_curIns( void )      { 	reset_meshIns(); } 


	//! ����aniMesh��λ�á�
	void            set_aniMeshPos( osVec3D& _pos );
	void			 set_aniMeshRot(float _RotAgl);
	//! ���ŵ�ǰ��osa����
	void             scale_aniMesh( float _rate,float _time );


	/** \brief
	 *  ����һ���������������
	 *
	 *  ����ӿ���Ҫ����һЩ��Ч�Ĳ��ţ���Ҫ����Ϸ���������������ݡ�
	 */
	void          set_aniMeshTrans( osMatrix& _transMat );



	//! �õ���ǰ��ʵ��������������mesh��ref��Ŀ��
	osc_aniMesh*     get_meshPtr( void );

	//! ����osa�����Ĳ���ʱ��.
	void            set_osaPlayTime( float _timeScale );
	//! �õ�osa��������������ʱ�䡣
	float           get_osaPlayTime( void );

	//! ������Ч�Ķ�����Ϣ
	bool            update_osaAnimation( float _etime );

	//! ���ú͵õ�����״̬��
	void            set_hide( bool _hide ) { m_bHide = _hide; }
	bool            get_hide( void ) { return m_bHide; }

	//! �õ��Ƿ�top Layer render.
	bool            get_topLayer( void ) { return this->m_bTopLayer; } 

	//! �Ƿ���ˮ����صĶ���.
	bool            is_waterAni( void ) { return this->m_bWaterAni; } 
	//! �Ƿ�������صĶ���.
	bool            is_FogAni( void ) { return this->m_bFogAni; } 



	// ��Ч�༭����صĺ�������
# if __EFFECT_EDITOR__
	//! �Ƿ�ֹͣ��ǰ�Ķ������š�
	bool            m_bStopPlay;
	/** \brief
	 *  ֹͣ��ǰ�����Ĳ��š�
	 */
	void            stop_aniMeshIns( void );

	/** \brief
	 *  ����ĳһ��������صĲ���.
	 *
	 *  ���ݴ���Ĳ���������animationMesh�ڲ������ݡ�
	 */
	bool            set_animationMesh( os_aniMeshCreate* _mc );

# endif 

};


//! ����inline��������ļ���
# include "osAniMesh.inl"

