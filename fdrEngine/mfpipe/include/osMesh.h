/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osMesh.h
 * 
 *  His:      River created @ 2003-12-27
 *
 *  Desc:     �м�ܵ��ɽ��ܵĶ�������Ⱦ��������.
 *  
 *  
 *  "�����������һ��ǿ�ҵľ�����ʶ,���ֲ�������������˾޴�Ķ������Ժ��С�
 *   �������ֲ�Ը���Լ�����˥��Ķ���֧�������ǡ�"
 *   
 */
/////////////////////////////////////////////////////////////////////////////////////////////////

# pragma once

# include "osShader.h"
# include "../../interface/osInterface.h"
# include "../../exLib/sceneEx.h"
# include "osAniMesh.h"


//! �����Զ�ľ���
# define  FAR_TERRMAXSIZE   350.0f


//! һ����Ļ�����ɴ��ڵ�mesh������Ŀ.
# define  MAXMESHNUM_INSCREEN    230

# define  MAXSUBSET_PERMESH      12 
# define  MAXMESH_FILENAMELEN    64

//! ����ÿһ��64*64��С��ͼ������ʹ��osa�ļ�����Ŀ��
# define  MAX_OSANUM_PERTG       224

//! ����ÿһ��tileGround������ʹ�õ�mehsInstance����Ŀ��
# define  MAXMESHINS_PERSCE      96

//! ���峡���е���Ч������ʹ�õ�osa�ļ�����Ŀ��
# define  MAXEFFECT_OSANUM      1024

//! ����������������ʹ��AniMesh����Ŀ�����е�aniMeshIns��ʹ����������е����á�
# define  INIT_ANIMESHSIZE      384

struct  os_ViewFrustum;
class   osc_middlePipe;



//! �ֹ��õ�staticMesh�Ķ��㣬������������Ȼ���ڵõ�Mesh��attributeTable,Ȼ�����DIP.
# define CUSTOM_MESHDRAW     0


//! Ӱ��һ����Ʒ�Ķ�̬�����ݽṹ
struct os_meshDLight
{
	//! �˴���byteֵ�����˶�Ӧ�ڶ�̬��������ڵĵƹ���Ϣ
	char      m_btDLIdx[MAX_LIGHTNUM];

public:
	os_meshDLight()
	{
		m_btDLIdx[0] = -1; 		
	}

	BOOL         is_usedDl( void )
	{
		if( -1 == m_btDLIdx[0] )
			return FALSE;
		else
			return TRUE;
	}
	void         reset_dl( void )
	{ 
		for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ ) 
			m_btDLIdx[t_i] = -1; 
	} 

};


/// ��Ⱦһ��Mesh��Ҫ���Ľṹ��
struct os_meshRender
{
	int          m_iMeshId;
	float        m_fAngle;
	osVec3D      m_vec3Pos;
	
	//! һ����̬Mesh������һ����̬�ĵ��Դ��Ӱ��,���ڸ���ȷ�ļ��㳡������Ʒ�Ĺ���Ч��
	os_meshDLight m_sDLight;

	//! ��Ⱦ��meshʱ���Ƿ�ʹ���Զ���ĵ���ɫ,���Ϊ�㣬��ʹ�ô���ɫ��
	DWORD         m_dwTerrColor;

	//! ģ�͵�������Ϣ
	osVec3D      m_vec3Scale;


public:
	os_meshRender()
	{
		m_iMeshId = -1;
		m_dwTerrColor = 0;
		m_vec3Scale.x = m_vec3Scale.y = m_vec3Scale.z = 1.0f;
	}
	void reset( void ) 
	{ 
		m_iMeshId = -1;
		m_dwTerrColor = 0;
		m_vec3Scale.x = m_vec3Scale.y = m_vec3Scale.z = 1.0f;
	}
};


typedef std::vector<os_meshDLight>  VEC_meshDLight;


typedef std::vector<os_meshRender>   VEC_meshRender;


typedef std::vector<D3DXATTRIBUTERANGE>  VEC_meshAttribute;

/// ������mesh��Ⱦ��������, ��ǰ��Mesh�Ƕ�d3dxMesh�İ�װ.
class osc_Mesh 
{
	friend class osc_meshMgr;
public:
	osc_Mesh( void );
	~osc_Mesh( void );

	/** ���ļ���ʼ��һ��Mesh
	 *
	 *  \param _fname Ҫ�������ļ�������.
	 *  \param _meshMem �����Ϊ��,Ϊ��mesh�ļ���Ӧ��Ex�ļ�
	 */
	bool              create_meshFromFile( const char* _fname,
		                     LPDIRECT3DDEVICE9 _dev, BYTE* _meshMem = NULL );

	/// �ڶ�̬������ͷ�meshʱ������������ᱻ�õ�
	void              release_mesh( void );

	/// ��Ⱦ��ǰ��Mesh
	bool              render_mesh( os_meshRender* _mr,bool _alpha = false  );

	 /// �õ���ǰ��mesh�Ƿ���ʹ����
	bool              is_inUse( void );

	//! ���Ե�ǰMesh����������Ŀ.
	int               get_meshTriNum( void );

	//! �õ���ǰMesh��Ӧ���ļ�����
	const char*       get_meshFName( void )       { return m_strMeshName; }
	DWORD             get_hashVal( void )         { return m_dwHash; } 

	//! �õ���ǰMesh�Ƿ����alpha���֣�������ڣ�����Ⱦʱ����Ҫ����
	bool              has_alphaSubset( void )     { return m_bAlphaMesh; }
	//! ��ǰmesh�Ƿ���ڷ�͸���Ĳ��֣���������ڣ�����Ⱦ��͸����Ʒʱ������Ҫ����
	bool              has_opacitySubset( void )   { return m_bOpacityMesh; } 

	//! �õ���ǰMesh��sphere�뾶��
	float             get_bsRadius( void )        { return m_bsphere.radius;}

	//! �õ�mesh��bounding Box.
	os_aabbox*        get_meshBBox( void )        { return &this->m_bbox; }

	//! ����һ������,���Ƿ��*.x��mesh�ļ��ཻ.
	bool              intersect_mesh( osVec3D& _rayPos,
		                   float _dis,osVec3D& _rayDir,float* _resDis = NULL );

	//! �õ���ǰ����Ʒ�Ƿ��ǹؼ���Ʒ
	bool              is_keyObj( void )           { return m_bVipObj; } 

	//! ��ǰ����Ʒ�Ƿ�������͸�����ֲ���Ҫд��ZBuffer����Ʒ
	bool              is_treeObj( void )          { return m_bTreeObj; }

	//! �Ƿ���������Ʒ.
	BOOL              is_bridgeObj( void )        { return m_bBridgeObj;}
	//! �Ƿ��������Ӱ��
	BOOL              is_AcceptFogObj( void )        { return m_bAcceptFog;}
	//! �Ƿ�����alpha����ײ
	BOOL              is_NoAlphaObj( void )        { return m_bVipObj;}

	//! �õ�faceNormal.
	bool              get_faceNormal( int _faceIdx,osVec3D& _normal );

	//! ���õ�ǰ��meshIn memory.
	void              set_inMemory( void ) { m_bInMemoryForEver = true; } 
	//! �õ���ǰ��mesh in memory״̬��
	BOOL              get_inMemory( void ) { return m_bInMemoryForEver; } 

private:
	//! �����������Ⱦ����.
	LPD3DXMESH        m_ptrPMesh;

	//! ��ǰMesh�õ���ShaderId,�͵�ǰmesh��subset��Ŀ��
	int               m_iShaNum;
	int               m_vecShaderId[MAXSUBSET_PERMESH];


# if CUSTOM_MESHDRAW
	//! �Ի�xMesh��Ҫ����Դ������attributeTable����Ҫ�Ķ��������������ָ��
	VEC_meshAttribute  m_vecMeshAttribute;
	LPDIRECT3DVERTEXBUFFER9  m_ptrMeshVBuf;
	LPDIRECT3DINDEXBUFFER9   m_ptrMeshIBuf;
# endif 
	
	//! ��ǰ��mesh��shaderId�Ƿ���ʹ��������alpha��mesh.
	bool              m_vecAlphaSubset[MAXSUBSET_PERMESH];

	//! ��ǰ��mesh���Ƿ���͸����Ƭ��
	bool              m_bAlphaMesh;
	//! ��ǰ��Mesh���Ƿ��з�͸������Ƭ
	bool              m_bOpacityMesh;

	//! ��ǰMesh�����֡�
	char              m_strMeshName[MAXMESH_FILENAMELEN];
	DWORD             m_dwHash;

	static            LPDIRECT3DDEVICE9  m_pd3dDevice;


	//@{
	//  River Mod @ 2005-6-3: ��������������,��������Ҫ�ڰ汾Ϊ2��ex�ļ�������
	/** ��ǰ��mesh�Ƿ��ǹؼ���Ʒ.
	 *
	 *  �ؼ���Ʒ�Ķ���Ϊ:ֻҪ����Ʒ���ڵĸ��ӿɼ�,����Ʒ�Ϳɼ�.������ƶ��Ĺ�����,
	 *  ��Ʒ���ᵭ�뵭��.
	 */
	BOOL             m_bVipObj;

	// �Ƿ����������Ʒ����Ⱦ͸������ʱ������Ҫд��Zbuffer
	BOOL             m_bTreeObj;

	// River mod @ 2007-4-11:�������Ʒ,���㾫ȷ����ײ���.
	BOOL             m_bBridgeObj;

	//! ��־��Ʒ�Ƿ��������Ӱ��.
	BOOL             m_bAcceptFog;
	


	/** ��ǰ����Ʒ�Ƿ���ֻ�ܷ�����Ӱ��.
	 *
	 *  ֻ�ܷ����Ӱ�����Ʒ,��ĳ����еķ�����Ϊ��Ʒ�Ļ�������ʹ��,��Ʒû��������.
	 *  ���ڲ�����ʾ�����еĲ�,������Ʒ.
	 */
	BOOL             m_bUseDirLAsAmbient;
	//@} 

	//! River @ 2010-12-30:����Ʒ��פ�ڴ治�ͷţ�Ϊ�˵���󳡾�������
	BOOL             m_bInMemoryForEver;

private:
	//! ������ǰMesh�õ���Shader.

	//! ��ǰmesh��AABB Bounding Box.��ҪԤ����ĵ�Ԫ��
	os_aabbox       m_bbox;

	//! ��ǰmesh��Bounding Sphere.
	os_bsphere      m_bsphere;

	//! River @ 2010-3-30:�����������Ʒ���������Ʒ�������normal.
	osVec3D*        m_vecFaceNormal;

};
typedef std::vector<osc_Mesh> VEC_mesh;


class   osc_meshMgr;
class   osc_effect;

/// ��osaMesh����а�װ��ʹ���Դ�����պ�Զ����
class osc_osaSkyTerr : public ost_Singleton<osc_osaSkyTerr>
{
private:
	//! ʹ���������飬ʹ����ʹ��������պ�Զ�������л�
	osc_aniMesh       m_sSkyMesh[2];

	//{@ windy add 6-20
	osc_aniMesh       m_sSkyMesh1;
	osc_aniMesh       m_sSkyMesh2;
	osc_aniMeshIns	  m_SkymeshIns1;
	osc_aniMeshIns	  m_SkymeshIns2;
	char              m_szSky1[128];
	char              m_szSky2[128];

	osc_aniMesh       m_sBoltMesh;
	osc_aniMeshIns	  m_BoltmeshIns;
	char              m_szBolt[128];
	float			  m_szBoltRate;
	float			  m_fBoltAngle;
	osColor			  m_BoltColor;
	float			  m_BoltHeight;

	//@}
	osc_aniMesh       m_sTerrMesh;
	osc_aniMeshIns	  m_sTerrMeshIns;
	char              m_szSky0Name[128];
	char              m_szSky1Name[128];

	char              m_szTerrName[128];
	//char              m_szTerr1Name[128];
	//@{
	// Windy mod @ 2005-9-11 9:58:13
	//Desc: 
	//osc_aniMesh     m_sSkyMesh3;
	//char            m_szSkyName3[128];
	//!Զ������������õ����ǲ�ͬ��Projmat
	//D3DXMATRIXA16   m_sky3matProj,
	//				  m_oldmatProj;
	//@}

	osc_meshMgr*      m_ptrMeshMgr;

	//! ��ǰ�������պ�Զɽ
	int               m_iCurActiveSkyIdx;
	int               m_iCurActiveTerrIdx;

	//! ��պ�Զɽ�ۺ�ʱ���ڶ����͸���ȡ�
	float             m_fSkyAlphaVal;
	float             m_fTerrAlphaVal;

	//! �����ת�ĽǶ�
	float             m_fSkyRotAgl;
	float			  m_fSkyRotAgl1;
	float			  m_fSkyRotAgl2;

private:
	//! ���ݲ�ͬ��״̬����Ⱦ��գ�������Ҫ������ջ���Զɽ���ۺϡ�
	void              set_skyRenderState( void );

	//! ����պ�Զ�����ۺ����ݽ��д���
	void              frame_moveSkyTerr( bool _reflect );

	//! ��Ⱦ���.
	void              render_bkSky( osVec3D& _cen,bool _reflect );

	//! ��ȾԶ��.
	void              render_bkTerr( osVec3D& _cen,float _terrScale );

	//! ����������.
	osc_osaSkyTerr();
	~osc_osaSkyTerr();

public:
	//! �õ�һ��osc_osaSkyTerr��Instanceָ��
	static osc_osaSkyTerr*  Instance( void );
	
	//! ɾ��һ��osc_osaSkyTerr��Instanceָ��.
	static void             DInstance( void );

	bool                    init_skyTerr( osc_meshMgr* _meshMgr, const char* _skyName,const char* _terrName);

	//! �����µ����,�����ͬĿǰ����alpha����
	bool                    create_newSky( const char* _skyName,const char* _skyName1,const char* _skyName2 ,const char* _bolt = NULL,float rate = 1.0f,float _angle = 0.0f,osColor _color  = osColor(1.0f,1.0f,1.0f,1.0f),float _boltHeight = 20.0f);

	//! �����µ�Զ��, Զ��ͬĿǰ����alpha���ɡ�
	bool                    create_newTerr( const char* _terrName );


	/** ��Ⱦ�����Զ����
	 *
	 *  \param _terrScale Զ��Ҫ�����Ӿص�Զ���������š�
	 */
	void                    render_skyTerr( osVec3D& _cen,float _fardis,bool _reflect );

	//!  �õ���ǰ����������.
	const char*             get_skyOsaName( void );

	//!  �õ���ǰ�����Զɽ������.
	const char*             get_terrOsaName( void );

	const char*	get_skyOsaName1();
	const char*	get_skyOsaName2();
	const char*	get_Bolt();
	float	get_BoltRate();
};


//! ��ǰ�Ķ���mesh�����6����Ⱦ��ʽ��
# define   MAX_MESHRSTYLE    6

/** \brief
 *  ���ڹܵ��м�ܵ����п���Ⱦ��mesh��Դ����.
 *
 *  ��Ⱦ��ʱ��,�ϲ�ʹ����Ʒ���ִ��м�ܵ��õ�һ����Ⱦid,
 *  �Ժ�����Ⱦ��ʱ��,�ϲ㴫���м�ܵ�Ҫ��Ⱦ����Ʒ��id,�м�
 *  �ܵ���������MeshMgr���ܵ���Щ Mesh,�õ�������Ⱦ�Ľṹ,
 *  ��������Ⱦ.
 *
 *  ����PMesh: 
 *  ���Ը���meshռ�õĸ������ݺ�����&�����������Զ�����õ�
 *  ÿһ����ȾʱMesh��Ҫ��Ⱦ������.
 *  
 *  ATTENTION: 
 *  ��meshMgr�У�id< 5000 ��ʾ�Ǿ�̬����Ʒ��ʹ��d3dxmesh,
 *                id> 5000 ��ʾ�Ƕ�̬����Ʒ��ʹ�������Լ����ļ���ʽ��
 *  ����ʹ��������Ʒ���Ա��ּ����ԣ��Ժ��޸ģ���ʹ��d3dxMesh�����ݡ� 
 *  
 *  MeshMgr����Ⱦ��Ʒʱ����Ҫ������Ʒ�����ͣ�d3xMesh or aniMesh������
 *  ��ͬ�����̺����ݡ� 
 */
class osc_meshMgr
{
	//! Զɽ����յ�����ࡣ
	friend class osc_osaSkyTerr;
private:
	//! Manager�е�Mesh.
	CSpaceMgr<osc_Mesh>     m_vecMesh;
	
	//! Manager�б����
	LPDIRECT3DDEVICE9       m_pd3dDevice;

	//! �����м�ܵ���ָ�룬�Զ����ʽ��aniMeshʹ���м�ܵ���
	osc_middlePipe*         m_ptrMPipe;

	//! ��ǰ��Ⱦ�������Ŀ.
	int                     m_iRFaceNum;

	//@{
	// ����mesh��ص����ݡ�
	//! ����mesh�����ݣ���Ӧ��ÿһ��aniMesh�ļ���
	CSpaceMgr<osc_aniMesh>  m_vecAniMesh;

	//! �ڵ�ǰ��Ⱦ֡�У��Ƿ��ǵ�һ�δ���̬�⡣
	bool                    m_bFirstProcessDLight;
	//@} 


	//@{
	//  �����е�aniMesh������ݡ�
	//! ����mesh��ʵ�����ݣ������ڳ����ж���ط���ͬһ��aniMesh,����������ڷ�����Ч��
	CSpaceMgr<osc_aniMeshIns>   m_vecAniMeshIns;

	/** ����������ڷ��ó����е�aniMesh��Ʒ��
	 *  �ֳ��������е�Ŀ�ģ��ǰѶ�����Դʵ����ʹ����Ŀ�����ڿɿصķ�Χ�ڡ�
	 */
	CSpaceMgr<osc_aniMeshIns>   m_vecSceneAMIns;

	//! �Բ�ͬ����Ⱦ���ͽ�������õ�������ָ�롣
	WORD*            m_vecMeshInsPtr[MAX_MESHRSTYLE];	
	int              m_vecMeshInsNum[MAX_MESHRSTYLE];
	int              m_iRAniMeshInsNum;

	//! ��صĶ�����ɫMeshָ��ͳ����ж�����ɫmeshָ�����Ŀ��
	WORD*            m_vecColorMeshPtr;
	int              m_iColorVerMeshNum;
	//@} 


	//@{
	//! ��ȾAniMeshʱ,�õ���RenderStateBlock Id
	int              m_iRSBAniMeshId;
	int              m_iRSBAniMeshAlphaId;
	void             set_animeshRState( bool _alpha );
	void             create_amRSB( bool _alpha );
	//@} 

	//@{
	//! ��ȾaniMesh�еĶ�����ɫ��Ʒʱ,ʹ�õ�RenderStateBlock Id.
	int              m_iRsbCVMeshId;
	int              m_iRsbCVMeshAlphaId;
	void             set_colorVertexRState( bool _alpha );
	void             create_colorVertexRSB( bool _alpha );
	//@} 


private:
	//! ����Ⱦһ��Mesh֮ǰ�Դ�������ݽ�������
	void             mesh_renderSet( os_meshRender* _mr,osMatrix* pOut = NULL );

	//! �ӵ�ǰ��aniMeshArray�еõ����Ǵ���һ��osc_aniMesh.
	osc_aniMesh*     get_aniMeshFromArr( os_aniMeshCreate* _mc,BOOL& _replace );

	/** ������Ani����Ⱦ���У�
	 *
	 *  �ȵõ�meshIns�Ƿ��ڿɼ��ķ�Χ�ڣ�Ȼ��
	 *  �൱�ڸ���ͬ�Ķ���ani��Ⱦ��ʽ��������,�Լ�����Ⱦ״̬�ĸı������
	 *  �������ֻ����������Ч��ص�aniMeshIns.
	 */
	void             sort_aniMeshPtr( os_ViewFrustum* _vf );

	//! ��Ⱦ���ǵõ���aniMeshInstance�Ķ��С�
	bool             render_aniMeshInsArr( bool _alpha,bool _alphaTestOpatity,
		               os_FightSceneRType _rtype = OS_COMMONRENDER );


	//! ��Ⱦ����ɫ��ص�Mesh.
	void             render_colorVertexMesh( bool _alpha );

public: // River @ 2008-12-15:ˮ�����
	//! ˮ�����
	static osc_effect*    m_ptrWaterEffect;

public:
	osc_meshMgr();
	~osc_meshMgr();

	//! ��ʼ����ǰMeshManager.
	bool             init_meshMgr( LPDIRECT3DDEVICE9 _dev,osc_middlePipe* _pipe );

	//! �ͷŵ�ǰ��meshMgr.
	void             release_meshMgr( bool _finalRelease = true );

	/** ���ļ�����һ��mesh.
	 *
	 *  \param _meshMem �����ǰ���ڴ�����mesh��Ӧ������,����ڴ��д���.
	 */
	int              creat_meshFromFile( const char* _filename,BOOL& _replace );
	//! ����һ��mesh������
	void             release_mesh( int _id );

	//! ȫ���ͷž�̬��mesh.
	void             reset_staticMesh( void );

	//! ȫ���ͷŶ�̬��mesh.
	void             reset_osaMesh( void );

	/** �Ӵ����ṹ�д������ǵ�aniMesh.
	 *
	 *  \param _sceC �Ƿ��Ǵӳ����ڴ�����aniMesh.
	 */
	int              create_aniMeshFromFile( os_aniMeshCreate* _mc, 
		                      BOOL& _replace,bool _sceC = false,bool _topLayer = false );

	//! ��meshMgr��aniMeshIns��Ⱦ�����м��볡����Ҫ��Ⱦ��aniMeshIns.
	void             add_sceAniMeshIns( int _idx );

	//! ��ָ����λ������Ⱦ��ǰ��Mesh
	bool             render_mesh( os_meshRender* _mr,bool _alpha = false );
	//! ��Ⱦһ��Mesh
	bool             render_meshVector( VEC_meshRender* _vec, int _mshCount, bool _alpha = false );

	//! ��Ⱦһ��Mesh,�����ò�����Ϣ
	bool             render_meshVectorNoMaterial( VEC_meshRender* _vec, int _mshCount,ID3DXEffect* _pEffect,osMatrix& _ViewProjMat);

	//! �õ�mesh�İ뾶��
	float            get_meshRadius( int _idx );

	//! ��Ⱦ�������ж�̬��mesh
	bool             render_aniMesh( os_ViewFrustum* _vf,
		               bool _alpha,os_FightSceneRType _rtype = OS_COMMONRENDER );

	//! ��Ⱦˮ����ص�����
	bool             render_waterAni( void );

	//! ��Ⱦ�������ص�����
	bool             render_FogAni( int ShaderId ,int dwType);
	


	/** ����aniMesh�Ķ�̬�����⡣��Ҫ�Գ����е�ÿһ����̬������������
	 *  ��̬��ȫ����ȫ�ֹ�����ģ�⡣
	 */
	bool             process_aniMeshDlight( os_dLightData& _dlight );


	//@{
	//   ��Ʒ������ص����ݡ�
	//! ��ʼ����
	void             start_animationMesh( int _id );
	//! fade����fadeIn��ǰ��osa����.
	void             start_fadeOsa( int _id,float _fadeTime,bool _fadeOut = true,float _minFade = 0.0f );

	//! ���ص�ǰid��osa�ļ�
	void             set_osaHide( int _id,bool _hide );


	//! ���ö�����λ�á�
	void             set_aniMeshPos( int _id,osVec3D& _pos );
	//! ���ö�������ת
	void             set_aniMeshRot( int _id,float _angle );
	//! ���ŵ�ǰ��osa����
	void             scale_aniMesh( int _id,float _rate,float _time );


	//! ��ʱ����osa��λ��
	void             update_osaPosNRot( int _id );


	//! �õ�aniMeshIns�Ĳ���ʱ�䡣
	float            get_osaPlayTime( int _id );
	//! ����osaMeshIns�Ĳ���ʱ�䡣
	void             set_osaPlayTime( int _id,float _timeScale );


	/** ����һ���������������
	 *  ����ӿ���Ҫ����һЩ��Ч�Ĳ��ţ���Ҫ����Ϸ���������������ݡ�
	 */
	void             set_aniMeshTrans( int _id,osMatrix& _transMat );

	//! ��֤����id����Ч�ԡ�
	bool             validate_aniMeshId( int _id );

# if __EFFECT_EDITOR__
	//! ǿ�ƽ���һ������.
	void             stop_animationMesh( int _id );

	//! ����ĳһ��������صĲ���.
	bool             set_animationMesh( int _id,os_aniMeshCreate* _mc );
# endif 

	//! ɾ��һ������.
	void             delete_animationMesh( int _id,bool _sceC = false );
	//@} 

	const char*           get_aniNameFromId( int _id );


	/** ÿһ֡�е������������������û����Ⱦ����mesh��lruֵ
	 *  ÿ���������ͷ�һ�γ����кܳ�ʱ�䲻ʹ�õ�
	 */
	void             frame_moveLru( void );

	//! ��һ��meshId�õ����mesh�Ƿ���alphaMesh
	bool             is_alphaMesh( int _id );

	//! ��ǰMesh�Ƿ���OpacitySubse
	bool             is_opacityMesh( int _id );

	//! �õ���Ⱦ�������Ŀ
	int              get_rFaceNum( void );

	//! �õ���̬mesh��BoundingBox.
	os_aabbox*       get_meshBBox( int _idx );

	//! �õ�һ��osaMesh��BoundingSphere.
	os_bsphere*      get_osaMeshBsphere( int _idx,bool _sce = false );
	//! �õ�һ��osaMesh��os_bbox
	void             get_osaMeshBbox( int _idx,os_bbox* _bbox,bool _sce = false );

	//! �Ƿ��ǹؼ���Ʒ
	bool             is_keyObj( int _idx );
	//! �Ƿ�������Ʒ.
	BOOL             is_bridgeObj( int _idx );
	//! �Ƿ����������Ӱ����Ʒ
	BOOL             is_AcceptFogObj( int _idx );
	

	//! �Ƿ�������Ʒ
	BOOL             is_treeObj( int _idx );

	BOOL			is_NoAlphaObj( int _idx );



	//! ����һ������,���Ƿ��*.x��mesh�ļ��ཻ.
	bool             intersect_mesh( int _idx,osVec3D& _rayPos,
		               float _dis,osVec3D& _rayDir,float* _resDis = NULL );

	//! �����л�ʱ������meshMgr
	void             sceneChange_reset( void );

	//! ����ĳһ��topRenderLayer��meshIns����������ĳһ֡�õ���Ⱦ��
	void             reset_osaMeshInsRender( int _id );



};


//! ������ǰ�ļ��õ���inline������
# include "osMesh.inl"

