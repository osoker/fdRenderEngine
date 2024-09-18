///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacter.h
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     ����os�����е����ﶯ�����á�
 *  
 *  "Ů����ʴ�������������ˬ�����ԣ�����������������E������־ɱ��"
 *  
 */
///////////////////////////////////////////////////////////////////////////////////////////
# ifndef __OSCHARACTER_INCLUDE__
# define __OSCHARACTER_INCLUDE__



# include "../../interface/osInterface.h"
# include "../../tools/include/fadeMgr.h"
# include "osShader.h"
# include "osPolygon.h"
# include "../include/osChrRide.h"

#include "../../Terrain/include/fdFieldMgr.h"
#include "../../fdrEngine/scenePostTrail.h"

//
// ���µĲ�������У�ÿһ�������������������������ֻ������һ���������������������
// �Զ��ӵ�������������Ŀ����ʹ3d�豸���ٵ�ʹ��״̬�л���

//! һ�������������е���ЧԪ����
# define EFFECTNUM_PERWEAPON  3

//! �������������
# define CHAR_AMBISCALE 1.0f

//
// ʹ�ÿ��ļ���������ļ���ʽ��

//! ÿһ��bpm������ʹ�õ�bone��Ŀ.
# define MAX_BONEPERBPM 26

//! һ������ϵͳ������Frame��Ŀ��
# define MAX_FRAMENUM    128

# define SKINMESH_DEBUG   1

//! bodyPartMesh�ļ�������󳤶ȡ�
# define MAX_BPMFNLEN 64


//! ���峡����������ʹ��skinMesh��Ŀ,�ڸ��������64��skinMesh��Ʒ
extern int  MAX_SCENESMNUM; 

//@{
//! ���岿λ�Ķ���
# define PART_DECORATION     0
# define PART_HEAD          1
# define PART_COAT          2
# define PART_PANTS         3
# define PART_FOOT          4
# define PART_HAND          5
# define PART_WEAPON        6
//@}


//! �����ļ����ļ�ͷ����չ����
# define GLOSSY_FHEADER  "gls"


//! ÿһ������������ʹ��32���ֽ���Ϊ������������֡���
# define ACT_NAMELEN        32

//! �任һ��������Ҫ��ʱ�䡣
# define ACT_CHANGESTATETIME 0.10f

//! ÿһ��������������Ĺ�����Ŀ��
# define VERTEX_INFLUENCE    3


//////////////////////////////////////////////////////////
//////! �����ļ����ݵİ汾��
// 
// �����˵��⸽�����ݺ��BodyPartMesh(*.skn)�ļ��汾�š�
# define GLOSSY_BPMFVERSION    102
//! 
# define GLOSSY_BPMFVERSION103    103
//! 104�ĵ������ݼ����˶Ե�����β���ȵı༭
# define GLOSSY_BPMFVERSION104    104
//! 105�ĵ������ݼ����˶�������Ч���ŵ�֧��
# define GLOSSY_BPMFVERSION105    105
//! 106�ĵ������ݼ����˶�4��������Ч�ļ���֧�֣����ڶ��ǿ������
# define GLOSSY_BPMFVERSION106    106
//! 107�ĵ������ݼ����˶��������Чλ�õ�֧��
# define GLOSSY_BPMFVERSION107    107


//! ������ȾIndexedBlend��SkinMesh���ݣ�֧��Geforce���Կ�,���һ�������ȥ��???
#define IDXBLEND_FVF ( D3DFVF_XYZB3|D3DFVF_LASTBETA_UBYTE4|D3DFVF_NORMAL|D3DFVF_TEX1 )


////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                              ���ڹ��������ļ�˵��

����ѡ����ֶ�����ʽ��
   1: ʹ��VS matrix palette������
   2: ʹ��d3d������������ʽ��
   3: ���Բ���ʹ��hlsl�Ķ�����
   4: ע���Ӧ��ͬ�����岿λ���ݣ�������Ҫ��ͬ��
      effect�ļ���
װ����أ�
����װ���Ͷ�����صĽṹ���ϲ����Ϸ������д����˴�ֻ����
  ����򵥵Ķ�����Ϣ��
    

���ڹ�������������Ҫ����Դ��
����
����1: ���㻺������
�������йܵ���������������.
  ���޸��м�ܵ���ʹ�м�ܵ������ͷŶ��㻺������������䶥��
    �����������ڸ�������װ�������ε�ʱ���õõ���

	2: Effect�ļ�,�м��������Ҫ��vertex shader.
	��ȫ�ֵ�shaderMgr������õ���

	3: 3d�豸��
	�ڴ�������Ⱦһ��3d����������Ʒ��ʱ�򣬴���ǰ�˹ܵ���ָ������м�
	�ܵ���ָ�룬ʹ�ÿ�������3d�豸���ڲ�״̬��
    

��������Ӧ��ע������
�����ڵ�ǰ�Ķ����ṹ�У�ֻ��osc_skinMesh�����м��osc_bodyFrame��osc_bpmInstance
  ���������Ļ�ϵ���ʾ��ɫ��Ψһ�Ľṹ�������Ľṹ����Manager����ʹ����Ĺ����ࡣ


*/
////////////////////////////////////////////////////////////////////////////////////////////////////////



//! ��ǰ������skinMesh�õ��Ķ���ṹ��
struct os_characterVer
{
	osVec3D   m_vec3Pos;

	//! ����Ȩ��,�̶�ʹ������������
	float     m_vecWeight[2];
	BYTE      m_vecBoneIdx[4];

	//! skinMesh�����normalֵ��
	osVec3D   m_vec3Normal;

	//! �����uv���꣬�Ժ����ʹ������������������Ч��
	osVec2D   m_vec2TexUv;

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   �����������������������Ķ�������࣬���еĶ������Ӷ����������еõ�.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// ����ÿһ�׹�����Ӧ�����Ĺ���������Ŀ��
#define MAX_ANI_PERBONE	    128


/** \brief
 *  ÿһ�������Ĺؼ�֡����,����quaternion��offset.
 *
 *  ��Щ�����������??????
 */
struct os_bactPose
{
	osQuat      m_actQuat;
	osVec3D     m_vec3Offset;


	//! River @ 2005-3-4: ������FlipDeterminantֵ�Ķ����汾��
	//! �����ֵΪ�棬����Ҫ����ת���ݲ����ľ������-1��Scale.
	BOOL        m_bFlipDeterminant;


	//! �ӵ�ǰ�Ķ����õ�����
	void        get_posMatrix( osMatrix& _mat );

};
typedef std::vector< os_bactPose > VEC_bactPose;

/** \brief
 *  �����Ķ�����
 */
struct os_boneAction
{
	int         m_iBoneNum;
	int         m_iNumFrame;

	//! ��ŵ�ǰ������Ϣ�ľ���
	VEC_bactPose   m_vecMatrix;



	//! ���ŵ�ǰ������Ҫ��ʱ�䡣
	float       m_fTimeToPlay;

	//! ��ǰ�����Ĳ����ٶ�.֡/��
	int         m_iPlaySpeed;

	//! ����õ���ǰ�����ֹͣ����֡������
	int         m_iStopFrameIdx;

	//! ��ǰ������Ӧ������,�ϲ����ʹ�����������Ŷ�����
	char        m_szActName[ACT_NAMELEN];


public:
	os_boneAction();

	/** \brief
	 *  ���ļ��е���һ��������bone�Ķ����ļ���
	 *  
	 */
	bool        load_boneActFromFile( const char* _fname );

	/** \brief
	 *  ���õ�ǰ�����Ĳ����ٶ�.
	 *
	 *  \param _speed ÿ���Ӳ��ŵ�ǰ�������ٴ�.
	 */
	void        set_actPlaySpeed( float _speed );

	/** \brief
	 *  ���õ�ǰ���������֡�
	 */
	void        set_actName( const char *_name );
 
	

};
typedef std::vector< os_boneAction > VEC_boneAction;


/** \brief
 *  ÿһ������Ķ�������Ӧһ�������б������ϲ����ͨ�����������ö�����
 */
struct os_actNameStruct
{

	//! ��ǰ�������ֶ�Ӧ�Ķ�����Ŀ��
	int       m_iActNum;

	//! ��ǰ���������֡�
	char      m_szActName[ACT_NAMELEN];

	//! ��ǰ�������ֶ�Ӧ�Ķ�����������
	int       m_vecActIdx;

	//  River @ 2010-6-22:
	//! ����ǰ�Ķ���ʱ���Ƿ�����������λ.
	BOOL      m_bHideWeapon;

	os_actNameStruct()
	{
		m_iActNum = 1;
		m_szActName[0] = NULL;
		m_vecActIdx = 0;
		m_bHideWeapon = FALSE;
	}
};
typedef  std::vector<os_actNameStruct>  VEC_actName;

/** \brief
 *  ��������������ݵ��ࡣ
 *  
 *  �ǵ�ǰ�����Ķ����б�,�����ܰ�һ���������еĶ������ϲ���һ���ļ���ȥ��
 *  ���뵱ǰ�Ĺ�������ʱ���������еĶ�����
 */
struct os_boneAni
{
	char                m_szAniName[32];
	DWORD               m_dwHash;

	DWORD				m_iNumBones;
	DWORD				m_iNumParts;
	VEC_dword			m_ptrBone;
	DWORD				m_iNumAniTypeCount;

	//! �������Ķ�����Ŀ��
	DWORD				m_iTotalAniFrame;

	//! ��������֡����Ŀ��
	int               m_iBANum;
	//! ��������֡���б�
	VEC_boneAction     m_vecBATable;

	//! ��ǰ�׶���ÿһ��������������Ϣ��
	VEC_actName        m_vecActName;


	/** \brief
	 *  �Ӷ����ļ��еõ��µĶ������ݡ�
	 *  
	 *  \param _id ��Ϊ�����ļ�ʹ��������׼������ֻ��Ҫ
	 *             ����_id�Ϳ����˵õ��µĶ�����
	 */
	bool              load_boneAction( const char* _actName,int _actNum );

	/** \brief
	 *  ���ļ��е��������������Ϣ�ļ�������ÿһ�����������֣�ÿһ�������Ĳ����ٶȵȡ�
	 *
	 *  \param _accIfname  ����Ķ�����Ϣ�ļ�����
	 */
	bool              load_actionInfoFile( const char* _actIfname );

	//! ��һ���������ֵõ�һ��������������
	int               get_actIdxFromName( const char* _actName,int& _nameIdx );

	//! River @ 2010-7-1: ��ǰ�Ķ��������Ƿ���Ҫ��������.
	bool              is_actNameHideWeapon( const char* _actName );


	//! ��һ��actNameIdx����������ڶ������������õ�һ��bone Act Table.
	int               get_actBoneAction( int _nameIdx );


};
typedef std::vector<os_boneAni>  VEC_boneAni;

/** \brief
 *  ����������������
 *
 *  ���ݽ�ɫ�����֣��ж���ǰ�������Ƿ��е�ǰ��ɫ�Ķ��������û�У��ѵ�ǰ��ɫ�����е�
 *  ���������뵽�����������С�
 *  
 */
class osc_boneAniMgr
{
private:
	 VEC_boneAni       m_vecBoneAni;
	 int              m_iBoneAniNum;

public:
	osc_boneAniMgr();
	~osc_boneAniMgr();

	//! �ͷŵ�ǰ��boneAniMgr.
	void               release_boneAniMgr( void );

	/** \brief
	 *  ���ļ��д�������ǰ����ϵͳ��ȫ���Ķ������ݡ�
	 *  
	 *  �����ǰ�Ĺ��������Ѿ�����������ݣ���������ָ�롣
	 *  
	 *  \return os_boneAin* ������ؿգ��򴴽�ʧ�ܡ�
	 */
	os_boneAni*        create_boneAni( const char* _aniName,int _aniNum );

};



//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      ��ʼMesh�������������Ĺ�����������*.SKN�ļ�
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////

/** \brief
 *  ʹ����������ȥ��ʼ��һ��skinMesh���豸������ݲ��֡�
 *  
 */
struct  os_smDevdepDataInit
{
	int     m_iVerNum;
	int     m_iVerSize;
	void*   m_ptrDStart;

	int     m_iIdxNum;
	WORD*   m_ptrIStart;

	//! ��Ӧ���ļ�����
	char*   m_strFname;

	os_smDevdepDataInit()
	{
		m_ptrDStart = NULL;
		m_ptrIStart = NULL;
		m_strFname = NULL;
	}

};

/** \brief
 *  Mesh�Ĵ���������ݡ�
 *  
 *  ����os_refObj������ʹ�ù����ж�̬���ͷŲ���Ҫ���ڴ���Դ��
 *  
 */
struct os_smMeshDevdep : public os_refObj
{
public:
	//! ��ǰ��mesh��Ӧ�Ķ��㻺������
	LPDIRECT3DVERTEXBUFFER9   m_pBuf;

	//! ������Ŀ��
	int                     m_iVerNum;
	int                     m_iVerSize;

	//! ��ǰ��mesh��Ӧ��Idx��������
	LPDIRECT3DINDEXBUFFER9    m_pIdxBuf;
	int                       m_iIdxNum; 

	//! ��ǰ��mesh��Ӧ��Ӳ���ļ���
	char                    m_strMFname[64];
	DWORD                   m_dwHash;

	//! ��ǰ��mesh�Ƿ��ڱ�ռ��״̬��
	bool                    m_bUsed;

protected:
	/** \brief
	 *  �ͷŵ�ǰ��Դ�Ĵ��麯��������ǰ�����ü�����Ϊ0ʱ���Զ��������
	 *  ������
	 */
	virtual void release_obj( DWORD _ptr = NULL );


public:
	os_smMeshDevdep();
	~os_smMeshDevdep();

	//! ʹ���ļ���ʼ����ǰ��dev dep resource.
	bool                     init_smMesh( os_smDevdepDataInit& _vi,LPDIRECT3DDEVICE9 _dev );

	//! restore dev dep resource.
	bool                     restore_smMesh( LPDIRECT3DDEVICE9 _dev );

	//! on deivce lost.
	bool                     onlost_smMesh( void );


# if __GLOSSY_EDITOR__
	//! ѡ��󱸷ݵ���������������
	WORD*               m_wTriIdx;
	int                 m_iBackIdxNum;
	bool                m_bFirstSel;
	/** brief
	 *  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
	 *
	 *  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
	 *  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
	 */
	virtual int        displaytri_bysel( int _triidx[24],int _trinum );
# endif 	

};


//! Devdep ���ݵ�stl vector
typedef std::vector<os_smMeshDevdep>    VEC_smMeshDevdep;

/** \brief
 *  ����skinMesh�㴢�������Դ���ࡣ
 *
 *  �ϲ���Ҫ���ļ��д���һ��mesh��ʱ���Ȳ�ѯ����������������������
 *  �м�������mesh,����Ҫ�������´���mesh.
 *  
 *  ���������м�ܵ�����3d�豸�򽻵���ȥ�����豸��صĶ����index��Դ��
 *  ��һ�����㻺����û��ʹ�����Ŀͻ���ʱ���ͷ�������㻺��������Index buffer.
 *
 *  ����������а����˶������ݺ�shader���ݵĴ�����
 *  
 */
class osc_smDepdevResMgr 
{
private:
	//! ����������ݵ��б�
	VEC_smMeshDevdep       m_vecDevdepRes;

	//! �豸������ݵ�ָ�롣
	LPDIRECT3DDEVICE9       m_pd3dDevice;


private:
	//! 

public:
	osc_smDepdevResMgr();
	~osc_smDepdevResMgr();

	/** \brief
	 *  ��ʼ����ǰ��manager.
	 */
	void           init_smDevdepMgr( LPDIRECT3DDEVICE9 _dev );

	//! �ͷŵ�ǰ��skinMesh Devdep Manager.
	void           release_smDevdepMgr( bool _finalRelease = true );


	/** \brief
	 *  ��Mesh�ļ������豸��ص����ݡ�
	 *  
	 *  \return int -1��ʾ����ʧ�ܡ�
	 */
	int            create_devdepMesh( os_smDevdepDataInit& _vi );


	//! lost&restore devdepMeshMgr.
	void           onlost_devdepMesh( void );
	void           restore_devdepMesh( void );



	/*** \brief
	 *  �ͷ�һ��body part mesh���豸������ݡ�
	 *  
	 */
	void           release_devdepMesh( int _id );


	/** \brief
	 *  ʹ��id�õ�������Ҫ���豸������ݡ�
	 */
	os_smMeshDevdep* get_smMesh( int _id );


	/** \brief
	 *  �õ���ǰ�������������ռ�õĶ���������������ڴ��С��
	 *
	 *  \return int ������MΪ��λ����Ŀ��
	 */
	int            get_occuMemSize( void );

# if __GLOSSY_EDITOR__
	/** brief
	 *  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
	 *
	 *  \param int _resId  Ҫ�����resourceId
	 *  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
	 *  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
	 */
	virtual int        displaytri_bysel( int _resId,int _triidx[24],int _trinum );
# endif 

};


//! ÿһ��bodyPartmesh��,��������˵�������,��Ϊ���浶�����ݵİ汾��.
# define GLOSSSY_DATAVERSION     0x100

//! ��¼����λ�õ���Ŀ��
# define  MAX_SWORDPOS  18
# define  MIN_SWORDPOS  6

//! ��Ⱦ����ʱ,�ùؼ�֡����ϸ�ֵĸ���.
# define  MAX_RENDERSNUM 192

//! ÿһ��������������8��������Ӧ���⡣
# define  MAX_ACTGLOSSY 8

//! ���嵶������Ⱦ֮֡�����̾���.
# define  MINDIS_INGRLINE  0.18f

/** \brief
 *  ����������ݽṹ��
 *
 *  ��¼�ϼ�֡�����λ����Ϣ����������Щλ����Ϣ��
 *  
 *  
 */
class osc_swordGlossy
{
public:
	//! ��¼����Ŀ�ʼ��ͽ�����.
	osVec3D      m_vec3Start;
	osVec3D      m_vec3End;



	//! ��һ��������Ӧ�������ɫ,Ĭ���Ǵ���ɫ.
	DWORD        m_vecGlossyColor[MAX_ACTGLOSSY];


	//! �����Ӧ�����ĵ㡣
	osVec3D      m_vec3Center;

	//! ����ؼ��߿����ͽ�����ĵĳ���.
	float        m_fSgLength;

	//! ��������Ӧ�Ĺ�����������
	int         m_iMatrixIdx;



	/////////////////////////////////////////////////////////////////////
	// ���µ�����Ϊ����������֮�乲�������,���ڲ�ͬ����ʹ����ͬ����ʱ

	//! ��Чλ��,����0.0f��1.0f֮�䣬��vecStart��vecEnd���ֵ��
	float       m_fEffectPos;

	//! �����osa�ļ�,osa�ļ�������Y����ת,�Ե�����Чģ�͵ķ���.
	float       m_fOsaRotYAngle;


	//! ÿһ����Ч�������ţ���ֵΪ������Ч������ֵ
	float       m_fScaleVal[EFFECTNUM_PERWEAPON];


	//! ��Ӧ����Ķ�����Ŀ������һ��������Ӧ���⡣
	int          m_iActNum;
	//! ���������顣
	int          m_vecActIdx[MAX_ACTGLOSSY];

	//! ������Ӧ����Ч�ļ����ݣ�ÿһ����������Ӧ4����Ч
	I_weaponEffect  m_sEffectName[4];

	/** \brief
	 *  �����Ӧ�������ļ���.
	 *
	 *  ֻ���ļ�������effectTexĿ¼���ļ�.����: "glossy"
	 *  ���Ӧ�ļ�: "effTexture/glossy.hon"�ļ���
	 */
	char        m_szGlossyTex[32];

	//! �������β���ȡ���ֵ�Ĵ�С��6-12
	int         m_iGlossyMoveLength;

	//! River @ 2010-3-5:��Чλ��2��3.
	float       m_fAddEffectPos[2];

public:
	osc_swordGlossy();

# if __GLOSSY_EDITOR__
	//! �ѵ����ص������ĵ��x����ת�����ڵ������⡣
	void        rot_swordGlossyX( float _radian );
	//! �ѵ����ص������ĵ��Y����ת���������⡣
	void        rot_swordGlossyY( float _radian );

# endif 

};



/** \brief
 *  �����ʵ�����ݡ�
 *
 *  
 */
class osc_swordGlossyIns
{
	friend class osc_skinMesh;
private:
	osc_swordGlossy*   m_ptrSwordGlossy;

# if SKINMESH_DEBUG
    osc_skinMesh*      m_ptrmesh;
# endif 

	/** \brief
	 *  ��¼����ؼ�֡�õ��ĵ������ݡ�
	 *
     *  ���һ�����ݾ��ǵ���ؼ�֡��ǰ���ڵ�λ��.
	 */
	osVec3D      m_arrVec3KeyS[MAX_SWORDPOS];
	osVec3D      m_arrVec3KeyE[MAX_SWORDPOS];

	//! ��¼��ǰҪ��¼�ĵ����λ�á�
	int          m_iGlossyPos;

	/** \brief
	 *  ��ǰ�������ɫ.
	 *
	 *  �������DWORD��alpha����Ϊ0,����Ҫ��Ⱦ����.
	 */
	DWORD              m_dwGlossyColor;

	//! ���ӵ���ʱ,�õ��Ĳ�ֵ�ؼ�֡����Ŀ.
	int                m_iGRNum;

	//! �����ֵҪ��ʼ��λ�á�
	int                m_iInterSPos;

	

	//! ��ǰ��¼����λ���Ƿ񳬹������Ŀռ䡣
	bool                m_bPosOverFlow;

	//! �Ƿ������Ⱦ����,���ֵ����0,��ֵΪ��������.
	int                 m_iPRSword;


	//! ת����ĵ���Ŀ�ʼλ�úͽ���λ�á�
	osVec3D      m_vec3TransStart;
	osVec3D      m_vec3TransEnd;


	/** \brief
	 *  ��Ⱦ����ؼ�֡�õ�������.
	 */
	static osVec3D      m_arrVec3Start[MAX_RENDERSNUM];
	static osVec3D      m_arrVec3End[MAX_RENDERSNUM];

	//! ʹ�ù����Ķ������ݣ���Ⱦ��ʽ�Ǹ�alphaAdd�ķ�ʽ��
	static os_billBoardVertex     m_verSG[MAX_RENDERSNUM*2];


	//@{
	//  �豸�������.
	//! ������Ⱦ�����õ��Ķ��������������.
	static int                 m_iIdxBufId;
	static int                 m_iVerBufId;
	static osc_middlePipe*      m_ptrMidPipe;
	static os_polygon          m_sPolygon;
	//@} 

	/* \brief
	 *  �����Ӧ����Ч�豸���id.
	 *
	 *  River @ 2009-12-9:����Ӧ����effect Shader ID.
	 */
	int               m_iEffectId[EFFECTNUM_PERWEAPON];


	// 
	//! ����ʹ�õ������shaderId.
	int               m_iShaderId;

	//! effect������.
	int               m_iEffectType[EFFECTNUM_PERWEAPON];

	//! Ҫ�л���������Ч����
	int               m_iWeaponEffIdx;

	//! ����������Ч������ֵ,Ĭ��Ϊ1.0f
	float             m_fScaleWeaponEffect;

	//  River @ 2011-2-25:
	//! �Ƿ��Ѿ����Ի���ǰ�ĵ���.
	bool              m_bReadyToDraw;

private:

	//! ��䵱ǰ�����õ���indexBuffer���ݡ�
	static void   fill_indexBufferData( WORD* _idxBuf );

	//! ��䵶��Ķ������ݣ���Ҫ���㵶���alpha���ݡ�
	void         fill_vertexBufferData( os_billBoardVertex* _verBuf );

	//! ���նԵ������Ⱦ.
	void         final_renderSG( int _lineNum );

	//! ���ݵ���ĳ��ȣ���ʼ�㣬�����㣬���µ���ؼ��߽������λ�á�
	void         get_sgEndPt( osVec3D _spt,osVec3D& _ept,float _len );


	/** \brief
	 *  ��InterPos�����յĵ���ؼ���֮���ֵ��
	 *
	 *  \param _is,_ie ��ʼ�ؼ��ߺͽ����ؼ���.
	 */
	void        Interplate_keyLine( int _is,int _ie );


	//! ���ݻ������е����ݣ����²��õ�����ص�����
	bool        recal_sgData( void );

	//! ���������Ч�Ĵ���
	void         process_sgEffect( bool _bindPos = true );

	//! ���ļ�����չ���õ���Ч������
	int          get_effTypeFromName( const char* _name );

public:

	//! ��Ⱦ�����õ������ݽṹ����
	osMatrix     m_matBone;

public:
	osc_swordGlossyIns();
	~osc_swordGlossyIns();

	static void init_swordGlossyIns( osc_middlePipe* _mp );

	//! ���õ���ʵ��ָ��ĵ������ݡ�
	void        set_swordGlossyPtr( osc_swordGlossy* _sg,osc_skinMesh* _mesh = NULL );

	
	/** \brief
	 *  ��ʼ��Ⱦ����,�Ե���ʵ���л�����.
	 *
	 *  �������л��ڵ���༭����ģʽ�»ᵼ�µ�����ɫ�ĸı�.ʹ�õ���Ķ�����ʱ
	 *  ��ʾ�������ĵ���.��ʹ�õ���Ķ�����ʱ��ʾ����ɫ�ĵ���.
	 *
	 *  ����������ģʽ��,��ʹ�õ���Ķ�������ʾ����.
	 *  \param _actIdx �¶���������.
	 */
	DWORD        start_swordGlossy( int _idx );

	//! ���赱ǰ�ĵ������ݡ�ÿһ֡��ʼʱ���������������
	void        reset_swordGlossy( void );
	
	//! �洢��ǰ֡�ĵ����λ�á�
	void        add_swordPos( void );

	/** \brief
	 *  �Ե��������Ⱦ
	 *
	 *  \param _ds �����ֵΪfalse,����Ҫ��Ⱦ���⣬��λ��ƷҪ�ô˹��ܡ�
	 *  \param bool _glossyDraw  �Ƿ���󻭵��⣬���ܻ������Ҫ������������Ч�����������������
	 *  \param bool _bindEffPos �Ƿ���������������Ч��λ�ã�������Ч�д��ڳ����ڵĴ���
	 */
	bool        draw_swordGlossy( osVec3D& _spos,bool _glossyDraw,
		                 bool _ds = true,bool _bindEffPos = false );

	//! ��ӵ���Ĺؼ�֡��
	void        add_swordGlossyKey( osMatrix& _rotMat );


	//! �õ���������ĵ㡣
	void        get_glossyCenter( osVec3D& _center );

	/** \brief
	 *  ɾ�����������Ч��
	 *
	 *  \param bool _glossy �����ֵΪ�棬��ɾ��������Ч���豸�������
	 *  \param bool _effect �����ֵΪ�棬��ɾ������������Ч���������
	 */
	void        delete_devdepRes( bool _glossy = true,bool _effect = true );

	//! ����ɾ�����������Ч�Ĵ���������ݡ�
	bool        create_devdepRes( int _idx,bool _glossy = true,bool _effect = true,bool _topLayer = false );

	/** \brief
	 *  �Ƿ���Ҫ�ƽ��������λ��.
	 *  
	 *  ������⸽�ӵ�����Ч,����ʱ�ֲ���ʾ����Ļ�,����Ҫ�ƽ��������λ��.
	 */
	bool        need_pushMultiSPos( void );


	//! �л���������Ч��ָ��������
	bool        change_weaponEffect( int _idx );

	//! �����������ӵ���Ч.
	void        scale_weaponEffect( float _rate );




# if __GLOSSY_EDITOR__

	//! �õ�ʹ�õ���Ķ���������
	void        get_swordGlossyActIdx( int& _actNum,int* _actIdx,
											  DWORD* _color,char* _glossyTex );

	//! ���õ�ǰglossy��Ӧ������
	void        set_glossyTex( const char* _tex );

	//! ����osa��Ч�������ת.
	void        set_osaYRot( float _agl );

	//! ������Ч�����ű�����
	float       set_effScale( int _idx,float _scale );

	//! �õ���Ч��ص���Ϣ.
	void        get_sgEffect( float& _pos,I_weaponEffect& _effFName );

	//! �õ����ӵ�addPos.
	void        get_sgEffectPos( float& _pos,int _idx );

	//! ���ø��ӵ���Чλ��
	void        set_sgEffectPos( float _pos,int _idx );


	//! ���õ�����β���ƶ�����
	void        set_sgMoveLength( int _len );
    

# endif 

	// �õ�������β�ĳ�������
	int         get_sgMoveLength( void ){ return m_ptrSwordGlossy->m_iGlossyMoveLength;} 

};





//! ʹ������ؼ��ַ���ȷ�ϵ�ǰ��bodyPartMesh�Ƿ��������������λ��
# define  WEAPON_PARTCHAR   '7'
# define  WEAPON_PART8CHAR   '8'
# define  WEAPON_PARTID7  6
# define  WEAPON_PARTID8  7
# define  WEAPON_PARTOFFSET  8


/** \brief
 *  ���岿λ��Ӧ�Ķ���Mesh���ݡ�
 *
 *  ��������osc_bodyFrame������Ϊָ�룬��Ⱦʱ��Ҫ��������еõ�
 *  ��skinAnimation��ص���Դ������matrix Palette.
 *  
 *  ÿһ��bodyPartMeshӦ�ö�Ӧ�̶��Ĺ�����frameID,������һ����Ҳ����ʹ��
 *  ��������������ڴ����Ѿ�������һ���֣�ֻ��Ҫ���ڴ�ȡ��ָ�룬�������
 *  �ڣ���Ҫ��Ӳ���ϵ����ļ���
 *  
 *
 *  
 */
class osc_bodyPartMesh :public 	os_refObj
{

	//! bodyFrame��ʹ��bpm��ʹ��IndexedBlend����Ⱦ��ʽ��
	friend class    osc_bodyFrame;
	friend class    osc_skinMesh;

protected:

	/** \brief
	*  �ͷŵ�ǰ��Դ�Ĵ��麯��������ǰ�����ü�����Ϊ0ʱ���Զ��������
	*  ������
	*/
	virtual void release_obj( DWORD _ptr = NULL );


public:
	
	//! ��ǰbpm��Ӧ���ļ�����
	char                  m_strFname[MAX_BPMFNLEN];
	DWORD                 m_dwHash;

	//! ��ǰmesh��Ӧ��FrameId.
	DWORD					m_dwID;
	//! ��ǰmesh��Ӧ�ĸ����id.
	DWORD                 m_dwParentID;

	//! �ж��ٸ�palette�͵�ǰ��mesh����.
	DWORD					m_dwNumPalette;		// 5-17 

	//! ÿһ�������ܼ���������Ӱ�졣
	DWORD                 m_dwNumInfluPV;


	//! ÿһ��������Ӧ��matrix.
	osMatrix 				m_arrBoneMatrix[MAX_BONEPERBPM];		// numPalette

	//! ��Ӧ��ÿһ������id.
	DWORD                  m_arrBoneId[MAX_BONEPERBPM];

	DWORD					m_dwNumFaceInflunce;	// numFaceInflunce =   1,   2,   3,   4 		


	//! ��ǰmesh�����Ӧ��mesh fvf.
	DWORD                 m_dwFVF;
	//! ��ǰmesh�����vertex Size.
	DWORD                 m_dwVertexSize;

	//! ��ǰmesh�ж�Ӧ��indices����Ŀ��
	DWORD                 m_dwNumIndices;
	//! mesh�ж������Ŀ��
	DWORD                 m_dwNumVertex;


	//! ��Ӧ��devdepResMgr�е�mesh resource ID.
	int                     m_iDevdepResId;

	//! ��ǰbodyPartMesh�Ƿ���ʹ��״̬��. 
	bool                    m_bInuse;

	//! �����õ������ݽṹ,������������Ĳ��֣���ֵΪ�ա�
	osc_swordGlossy*       m_ptrSwordGlossy;

	//! �Ƿ���Ҫ��Ⱦ��ǰbpm,���ڶ�λ�ĺ��Ӳ���Ҫ��Ⱦ��
	BOOL                  m_bDisplay;
	//@{
	// Windy mod @ 2005-8-30 19:08:42
	//Desc: ride
	BOOL			 m_IsRideBox;
	//! ����ͷһ���������ݣ����ڸ����������ڵĵڶ����ֹ�����������
	os_characterVer  m_RideVertData;
	osc_ChrRide *	 m_ptrRide;

	//@}

	//  River @ 2010-12-21:
	//! �ͷ�ʱ�ļ����������������ʹ�õ��㷨.
	DWORD            m_dwReleaseCounter;


private:

	//@{
	//  ����IndexedBlend��Ⱦ��ʽ�����ݣ�֧��Geforce������Կ���
	//! ����Ĳ���
	D3DMATERIAL9				  m_sMaterial;
	//@} 

	/** \brief
	 *  ���������ݵĶ��롣
	 *
	 *  ����͵�ǰ��mesh����ͬ��*.gls�ļ����ڣ������ļ�Ϊ���������ļ���
	 */
	void            read_weaponData( void );


public:
	osc_bodyPartMesh();
	~osc_bodyPartMesh();


	/** \brief
	 *  ����skinMesh��ص����ݡ�
	 */
	//@{
	// Windy mod @ 2005-8-30 19:10:23
	//Desc: ride  _IsRide Ϊ���ʾ�ò�λΪ���ﶨλ����
	// River mod @ 2007-7-16:��˵Ĺ������Ԥ�ȵ���Ϊ����˵Ĺ���Ļ����������µ���
	bool        	 LoadMesh( char * _file ,bool _IsRide = false,int _ref = 0 );
	//!����ò��ֵ����ݣ��������ĵ�һ�����λ��
	void			 SaveCenterPos(osVec3D _pos);
	//@}

	/** \brief
	 *  ����*.skn�ļ��Ķ�����������ݡ�
	 *
	 *  �˺������贫���������������ָ���Ѿ��������ݣ����ҿռ��С�㹻��
	 *  ��Щ���������Ӱ��ص����ݡ�
	 *  
	 */
	bool           LoadMeshVNI( char* _file,BYTE* _vbuf,BYTE* _ibuf );


	/** \brief
	 *  �õ��豸������ݵ�id.
	 */
	int             get_devdepResId( void )   { return m_iDevdepResId; }
	
	//! �õ���ǰmesh��fvf.��ʹ���������������ʹ���Լ��Ķ���������
	DWORD           get_meshFVF( void )       { return m_dwFVF; }

	//! ��ǰ��BPM�Ƿ���ʹ���С�
	bool            is_inUsed(  void );


	//! ��ǰ��bpm�Ƿ����������֡�
	bool             is_weaponPart( void );


	//! �õ�������Ч���ַ���.
	const I_weaponEffect*  get_weaponEffect( int _idx );


	//! river added @ 2010-12-21:�����ͷ�
	void             release_bmpRes( void );

///////////////////////////////////////////////////////////////////////////////////////////////////////
	//  
	//  ����Ϊ�༭����ص����ݣ����յ���Ϸ��Ⱦ���治��Ҫ��Щ���ݡ�
# if __GLOSSY_EDITOR__

	//! ���ڴ����������ʱ���ݡ�
	BYTE*           m_ptrVerData;

	//! �洢������ص����ݡ�
	void            save_glossyData( FILE* _file );


	//! �洢��ǰ��bpm���ݣ��������汾�ţ������µĵ������ݡ�
	bool            save_weaponFile( void ) ;

	/** \brief
	*  ���õ�N��������Ч�ļ������4����
	* 
	*  \param int _idx ���õڼ���������Ч�����㿪ʼ�����ֵΪ3.����3�Ļ����ڲ����Ϊ��
	*  \param char* _effName ������Ч���ļ���
	*/
	bool            set_weaponEffect( int _idx,const I_weaponEffect& _effName );

	//! ����ÿһ������ĳ��̡�
	void            adjust_sowrdGlossySize( bool _start,float _size );

	//! ��X������ת���ǵĵ������ݡ�
	void            rot_swordGlossyX( float _radian );
	void            rot_swordGlossyY( float _radian );

	//! ��ǰ�ĵ�����X���Y�����ƶ�һ����������
	void            move_swordGlossy( bool _x,float _dis );

	//! ������ʼ����Sword Glossy.
	void            create_defaultGlossy( void );

	//! ���ô��������ڶ�λ������ʾ��������
	void            set_displayWeapon( bool _dis );

	//! ���赶��ĸ�����ת�Ƕȣ�ʹ��ȫ�ص�����ǰ�ĵ���
	void            reset_swordGlossyPosRot( void );

	/** \brief
	 *  ���õ����Ӧ�Ķ�������, ��������ÿһ��������Ӧ����ɫ���ݡ�
	 *
	 *  \param _actIdx    ��ʾ��һ��������Ҫ��ʾ���⡣
	 *  \param _arrColor  ��ʾ��Ӧ��һ�������������ɫ��
	 */
	void            set_swordGlossyActIdx( int _actNum,int* _actIdx,DWORD* _color );
		                      

	// ���õ����õ�����Ч��
	bool            set_sgEffect( float _pos,const I_weaponEffect& _effFName );

	/** brief
	 *  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
	 *
	 *  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
	 *  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
	 */
	virtual int        displaytri_bysel( int _triidx[24],int _trinum );


# else
	
	//! ����ͷһ���������ݣ����ڸ����������ڹ�����������
	os_characterVer  m_ptrVerData;

# endif 
///////////////////////////////////////////////////////////////////////////////////////////////////////


};

//! skin mesh���ݵ�stl vector.
typedef std::vector<osc_bodyPartMesh>  VEC_bodyPartMesh;
//! bpmָ���stl vector.
typedef std::vector<osc_bodyPartMesh*>  VEC_bpmPtr;


/** \brief
 *  BodyPartMesh�Ĺ�������
 *
 *  ��ΪbodyPartMesh�Ǵ��ļ��д�����ÿһ��bodyPartMesh�ֶ�Ӧ��ͬ�Ĺ�����Frame,
 *  ������������µ����BodyPartMesh���ڴ����Ѿ����ڣ�����Ҫ���룬��ʡȥ��
 *  �͹���������Ĺ��̡�
 *  
 */
class osc_bpmMgr
{
private:
	VEC_bodyPartMesh        m_vecBPM;

	//! �豸������ݵ�ָ�롣
	LPDIRECT3DDEVICE9       m_pd3dDevice;

public:
	osc_bpmMgr();
	~osc_bpmMgr();

	//!  ʹ��3d�豸��ʼ����ǰ��bpmMgr.
	void                  init_bmpMgr( LPDIRECT3DDEVICE9 _dev );

	//!  �ͷŵ�ǰ��bpmMgr
	void                  release_bpmMgr( void );

	//@{
	// Windy mod @ 2005-8-30 19:11:55
	//Desc:ride  ���ļ��е���bpm._IsRide ����ָ���Ƿ������ﲿ�֣�
	//�����Ϊ�棬����������ϲ㴫�룬
	int                   create_bpm( const char* _bpName,bool _IsRide = false );
	//@}

	//!  ��һ��id�еõ�Body part Mesh��ָ�롣
	osc_bodyPartMesh*      get_bpmPtrFromId( int _id );


	//! �ͷ�bpm�����á�
	void                  release_bpm( int _id );


};



//! ÿһ�������ζ�Ӧ���ڽ�������������
struct  os_neighbour
{
	WORD   m_wN[3];
};



//! ����ȫ�ֵ�������ݴ���
class osc_fakeGlowData
{
private:
	//! ��ߵĳ���
	float                      m_fLength;

public:
	//! �洢���е�bmp shadow�ڶ����normal��Ϣ
	std::vector<osVec3D>       m_vec3Normal;
	int                        m_iNormalUseNum;

	//! ��ǰʹ�õ���߿����Ϣ
	float                      m_fCurUseLength;

	//! ��ǰfakeGlow����ɫ
	DWORD                      m_dwColor;

public:
	osc_fakeGlowData();

	osVec3D*       get_unUseNormlDataStart( void );
	void           set_useNormalNum( int _num );

	void           resetNormal( void );

	void           set_fakeGlowData( DWORD _color,float _length );
	void           frame_move( void );

};


//  
//! �����м�ܵ�
class osc_middlePipe;

//! ÿһ��mesh�������еı���Ŀ
# define MAX_EDGE_PERMESH  4096

/** \brief
 *  ��Ӱ��ص����ݽṹ��
 *
 */
class osc_bpmShadow
{
public:
	//! ��ǰbpmʹ�õĶ�������
	os_characterVer*      m_ptrBpmVer;
	int                  m_iVerNum;

	//! River @ 2009-6-23:�����Ż��˴�����һ���õ��ڴ�أ���һ���Գ�ʼ
	//                    �����е���Ӱϵͳ�õ����ڴ�
	//! ÿһ֡����ʹ�ù�������ת����õ��Ķ���,ֻ��Ҫλ�þͿ�����
	osVec3D*             m_ptrBpmVerAT;


	//! ��ǰbpmʹ�õ��������ݡ�
	WORD*                m_ptrBpmIdx;
	int                  m_iIdxNum;

	static osc_fakeGlowData*     m_ptrFakeGlowData;

private:
	//! ÿһ��ģ�͵������ζ�Ӧһ��boolֵ����ʾ���������Ƿ�����ŷ���⡣
	bool*                m_bFront;

	//! ÿһ��������Ӧ�ö�Ӧһ��neightbour�б�.
	os_neighbour*         m_arrNeiList;

	//!  Ҫ��Ⱦ�ıߵ��б�
	WORD*                m_arrEdgeList;
	int                  m_iEdgeNum;


	//! ��ǰ��mesh��Ӧ�Ķ��㻺����,�˶��㻺����ֻ����osVec3D��������.
	LPDIRECT3DVERTEXBUFFER9    m_pShadowVBuf;
	int                        m_iShadowVerNum;
	//! �����������϶��ŷ�����������������ZFAIL��STENCIL������Ⱦ����Ȼɽ��ס����ʱ�����Կ����������Ӱ��
	LPDIRECT3DINDEXBUFFER9     m_pSourceIdxBuf;
	int                        m_iSourceTriNum;

	//! ��̬��3dDeviceָ��
	static LPDIRECT3DDEVICE9        m_pd3dDevice;
	static LPDIRECT3DVERTEXBUFFER9  m_pVBufShadowSqu;


	//@{ ���������ص�����
	//! ȫ�ֵģ��������������ݵ�dynamic vb idx.
	static int                      m_iDynamicVBIdx;
	static osc_middlePipe*          m_ptrMiddlePipe;
	static os_polygon               m_sRenderPolygon;
	//@} 


	//@{
	//! ��Ⱦ��Ӱʱ�õ�����Ⱦ״̬������
	static int                    m_iSHDrawState;
	static int                    m_iSHDrawRestoreState;
	static int                    m_iShadowSquareState;
	static int                    m_iShadowSSRestore;

	/** \brief
	 *  ������Ⱦ״̬��ʹ��Ⱦ��ʱ��ֻ��Ⱦ������Ϣ����zbuffer,����ʾ��
	 * 
	 *  ����Ӱ��������Ҫ����Ⱦ������zbuffer,����ʹ��������û����Ӱ��Ч����
	 */
	static int                    m_iOnlyZbufSB;
	static int                    m_iRestoreOnlyZbufSB;

	//@}

	//! ��Ļ��ȣ���Ļ�߶�
	static float                    m_fScrWidth;
	static float                    m_fScrHeight;


	//! ��ǰ��shadowVolume�Ƿ���¹���
	bool                         m_bUpdated;

	//! �ƹ�ķ���Ĭ��ʹ��ȫ�ֹ�ķ���
	float                m_fShadowProjLength;


	//! ��ǰ��bpm�Ƿ���Ҫ���Ч��
	bool                 m_bFakeGlow;
	osVec3D*             m_vec3Normal;


private:
	//! ���ݴ���ľ�����и���ģ����ÿһ�������λ�á�
	void                update_verPos( osMatrix* _matArr,int _matNum );

	//! �õ�ģ����ÿһ���������뷽���Ĺ�ϵ����Ի��Ǳ��ԣ�
	void                cal_triToLight( osVec3D* _lightDir = NULL );

	//! �����б��м���һ���ߡ�
	void                push_edge( WORD _verIdxs,WORD _verIdxe );

	//! ����õ�������Ӱ��ģ�ͱߡ�
	void                get_shadowEdge( void );


	//! ��ģ���е���Ӱ��Ե����ת��Ϊ�������ݣ�����䵽���㻺�����С�
	void                get_shadowVertexData( void );

	//! fadeGlow��أ�������ı�������䵽���㻺�����ڡ�
	void                get_edgeVertexData( osVec3D& _vpVec );


	//! ÿһ֡��ʼʱ�����赱ǰ��shadowVolume.
	void                reset_shadowVolume( void );

	//! ¼����Ⱦ��Ӱ��Ҫ����Ⱦ״̬�顣
	static void         record_ShadowVolumeStateBlock( void );
	static void         record_ShadowSquareStateBlock( void );

	//! �����Ļ����εļ�������
	static bool         fill_screenSquareVB( void );

public:
	osc_bpmShadow();
	~osc_bpmShadow();

	//! ��ʼ����Ӱ���ݽṹ����
	bool                init_shadowData( const char* _fname );


	//! �ͷŵ�ǰ����Ӱ������ݡ�
	void                release_shadowData( void );

	/** \brief
	 *  ÿһ֡����Ӱ���ݵĸ���
	 *
	 */
	void                update_shadow( osMatrix* _matArr,int _matNum );

	//! ��ʼ��ShadowVolume.
	static void         start_shadowVolumeDraw( void );
	static void         end_shadowVolumeDraw( void );


	//! Shadow��Ⱦ�Ľӿڡ�
	bool                draw_shadowVolume( void );

	/* \brief
	 *  ��fadeGlow�Ľӿڣ����Ч��
	 * \param DWORD _color        Ҫ����fadeGlow����ɫ����
	 * \param DWORD _colorNoise   ��ɫ���ݵ��������ֵ,����ɫ�Ļ�����������������ֵ
	 * \param osVec3D& _vpVec     ��ĳһ��������ߣ�����Ӱ����ʱ���ƹ�ķ���
	 * \param float _length       ������ݵõ���������ɢ�ı�Ե���ݵĳ��ȡ�
	 * \param float _lenthNoise   ��Ե���ݳ��ȵ����ֵ��
	 * \param bool _projParallel  �����ֵΪ�棬��˳��Ͷ�䷽����ǰ��������ݣ�����ֱ����
	 */
	bool                draw_fakeGlow( DWORD _color,DWORD _colorNoise,
		                  osVec3D& _vpVec,float _length,
		                  float _lenthNoise,bool _projParallel );


	//! ����Ļ�ϻ����ǵ�StencilSquare.
	static void          draw_shadowSquare( void );


	//! ȫ�ֵĳ�ʼ�����ǵ���Ӱϵͳ��
	static void          init_shadowSys( osc_middlePipe* _mp );	

	//! ����ȫ�ֵ���Ӱ͸����
	static void          reset_shadowColor( float _alpha );


	//! ȫ�ֵ��ͷ���Ӱϵͳ.
	static void          release_shadowSys( bool _frel );

	//! ����ֻ��ȾzBuffer���豸״̬��
	static void         set_onlyZbufStateBlock( void );

	bool                is_updated( void ) { return m_bUpdated; } 
	void                frame_setShadow( void ) { m_bUpdated = false; } 

	//! ����ShadowӦ��Ͷ�����Ӱ����.
	void                set_shadowProjLength( float _length ) 
	{
		osassertex( _length >= -0.01f,va( "The length is:<%f>...\n",_length ) );
		// �ӳ�2.0f,ȷ����Ӱ���������
		// River @ 2009-5-14:����ӳ��������Ի�ӳ���
		m_fShadowProjLength = _length;
	}

	//! ����fake glow
	void                set_fakeGlow( bool _glow );

	//! �豸��reset��غ���.
	void                shadow_onLostDevice( void );
	void                shadow_onResetDevice( void );

};
//!������Ӱ�����FX�ļ� 
#define HDRFX_FILENAME      "effect\\shadow.fx"




//! ����ڶ�����Ч����ʹ�õ����ݽṹ��
struct os_chaSecEffect					
{
	//! �ڶ�����ͼ��shader��ʽ��
	/*  �����µ�EffectType:
    5:  ��������.
	6:  ��������.
    7:  �Է�������.
	8:  �����뻷����������
	9:  �������Է�����������.
	10: �������Է�����������.
	*/
	WORD      m_wEffectType;

	//@{
	//! �ڶ�����ͼ�����õ������ݡ�
	float     m_fRotSpeed;
	osVec3D   m_vec3RotAxis;
	float     m_fUMoveSpeed;
	float     m_fVMoveSpeed;
	//@} 


	//! �ڶ�����ͼ��id.
	int       m_iSecTexId;

	//! ����shader��id.
	int       m_iShaderId;

	//@{
	//! �����й�������Ҫ�����ݡ�
	osMatrix  m_matFinal;

	/** \brief
	 *  ����ӿ�ʼ�ƶ������ڹ���ʱ�䡣
	 */
	float     m_fTexmoveTime;
	//@} 


	//! �ڶ����������ͼ����
	char      m_szSecTexName[64];

	int		m_iThdId;
	char    m_szThdTexName[64];

	// River @ 2010-3-11:
	int     m_iFourthId;
	char    m_szFourthTexName[64];
	


# if __CHARACTER_EFFEDITOR__

	//! �洢��ǰ��effect����,�ļ���Ҫ�Զ����
	void      save_chaSecEffToFile( const char* _cname );
# endif 

public:
	os_chaSecEffect();
	~os_chaSecEffect();

	//! ÿһ֡���㵱ǰ��ͼ��Ч���ڲ����ݡ�
	void      frame_moveSecEff( void );

	//! ɾ���豸��ص�����
	void      delete_devdepRes( void );

};


//! ����ڶ���������ͼ��Ԥ���룬������Ԥ���룬ֻ�����ǵ���֮���ڴ滺����һ�ݡ�
struct os_secEffPreLoad
{
	char   m_szSecFile[48];
	DWORD  m_dwHash;

	os_chaSecEffect  m_sCharSecEffect;
};

typedef std::vector<os_secEffPreLoad>  VEC_secEffProLoad;


//! ������ͼshader�ļ��Ļ���.���shaderRefΪ��ʱ����ɾ��������˽ṹ��
struct os_chrShaderBuf
{
	// ��Ϊ���ݽṹ�Ĺ�ϵ������¼shader���֡�
	//char   m_szShaderName[64];
	DWORD  m_dwHashId;

	int    m_iShaderId;
	DWORD  m_dwReleaseCount;

	os_chrShaderBuf() 
	{
		//m_szShaderName[0] = NULL;
		m_dwHashId = 0xffffffff;
		m_iShaderId = -1;
		m_dwReleaseCount = 0xffffffff;
	}

	//! �ͷ�shader.
	void   reset( void );

};

typedef std::vector<os_chrShaderBuf>  VEC_chrShaderBuf;


/** \brief
 *  bodyPartMeshInstance��ʵ����ÿ�ε���һ��bodypartMeshʱ��skinMesh�ṹ�õ�����һ��ʵ����
 *
 *  �����洢��ÿһ�����岿λ�����Ķ������ݣ�ʹ������ṹ��ʹ�������͹���һ�����ݳ�Ϊ���ܣ�
 *  ���ǿ��ܴ��ڲ�ͬ�Ķ����׶Ρ�����ÿһ��Instanceʹ��һ�׶����Ķ������ݡ�
 *  
 *  shader����Ч��ص����ݣ����������ڲ�����
 */
class osc_bpmInstance
{

	friend class             osc_skinMesh;
private:

	//! ��ǰʵ��ָ���bodyPartMesh���ݵ�ָ�롣
	osc_bodyPartMesh*         m_ptrBodyPartMesh;

	//! River @ 2010-12-28:��ǰbpm ins�õ������hash ID.
	DWORD                     m_dwHashId;

	//! ��ǰbodyPartMesh������mgr�е�id.
	int                      m_iBpmId;

	//! ��ǰ��bodyPartMesh�Ƿ��Ѿ��͹������������ˡ�
	bool                     m_bLinkBones;
	
	//! �洢ÿһ����������offsetMatrix.
	osMatrix*			    	m_ptrBoneOffset[MAX_BONEPERBPM];	

	osVec3D*					m_ptrBonePosOffset[MAX_BONEPERBPM];
	osQuat*						m_ptrBoneRotOffset[MAX_BONEPERBPM];

	//@{ 
	//  ATTENTION:
	//  �ڴ˴洢����Ⱦ��Ӱ��ص�����,bodyFrameʹ�õ������ʵ����ָ�룬
	//  ������ȾbodyFrameʱ���Եõ��˴�����Ӱ����

	//! �����ֵ��Ϊ�գ�����Ⱦʱ����Ҫ��Ⱦ��Ӱ��ص����ݡ�
	osc_bpmShadow*            m_ptrShadow;

	// 
	//@}

	/** \brief
	 *  ��ǰ��Instance��Ӧ�Ƿ�����������Ⱦ.
	 *  
	 *  �����ֵ��Ϊ�գ���ָ����һ�������Ⱦ�����ݽṹ��
	 */
	os_chaSecEffect*          m_ptrSecEffect;

	//! bpm������ص�shaderId��
	int                      m_iShaderID;
	//! ��ǰ��bodyPartMeshInstance�Ƿ���͸����shader.
	bool                     m_bAlpha;


	//! River @ 2010-7-1:�Ƿ�������״̬.
	bool                     m_bHide;


	//@{
	//! shaderBuf������ء�
	int                      get_freeBuf( void );
	void                     release_insShader( int _shaderId,bool _finalRelease = false );
	static int               get_bufShaderId( DWORD _hashId );
	//@} 


private:

	//! �õ�һ��ʵ������ָ��.
	osc_swordGlossyIns*                   get_glossyInsPtr( void );

	int                                   m_iNodeIdx;

	//! �õ�һ�������ʵ��������
	osc_ChrRideInst*                      get_chrRidePtr( void );
	int                                   m_iCRNodeIdx;


public:
	//! river added @ 2010-12-22: ����һ���Ե��ڴ����
	static CSpaceMgr<osc_swordGlossyIns>  m_vecDGlossyIns;

	//! River @ 2010-12-24:Ԥ����������ص�����ָ�롣
	static CSpaceMgr<osc_ChrRideInst>     m_vecChrRideIns;

	//! River @ 2010-12-28:shader�Ļ������� 
	static VEC_chrShaderBuf               m_vecChrShaderBuf;


	//! ����е��⣬����������ص�ʵ�����ݡ�
	osc_swordGlossyIns*        m_ptrSGIns;
	//@{
	// Windy mod @ 2005-8-30 19:12:57
	//Desc: ride����ʵ������
	osc_ChrRideInst *				m_ptrRideIns;
	//@}


	//! ����geforce���Կ����Ż���ÿһ��bpmInstanceӦ�е�����shaderId��texture
	LPDIRECT3DTEXTURE9		   m_ptrBpmTexture;


# if __GLOSSY_EDITOR__
	//! �ڵ�����������ʱ,��ʼ��bpsInstance��ص����ݡ�
	void                   init_swordGlossy( void );

	/** brief
	 *  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
	 *
	 *  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
	 *  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
	 */
	virtual int        displaytri_bysel( int _triidx[24],int _trinum );

# endif

	//! ���õڶ��������id.
	void                   set_secTexid( int _id );
	//! ��������������������ڶ�������ͨ��
	void					set_thdTexid( int _id );

public:
	osc_bpmInstance();
	~osc_bpmInstance();


	/** \brief
	 *  ʹ�������������bodyPartMesh,osc_skinMesh��ʹ������ӿ�������bodyPartMesh��صĲ�����
	 *
	 *  ʹ��bpm��Manager���õ�bodypartMesh��ָ�롣
	 *  \param _playSg �Ƿ񲥷ŵ���
	 */
	bool        	        loadBpmIns( char *  _file,bool _playSg,bool _playWEff,
		                         bool _shadowSupport = false,bool _IsRide = false,
								 bool _topLayer = false,osc_skinMesh* _mesh = NULL );

	/** \brief
	 *  �ͷŵ�ǰ��bpmInstance.
	 */
	void                   release_bpmIns( void );


	osc_bodyPartMesh*       get_bpmPtr( void )       {  return m_ptrBodyPartMesh; }
	osMatrix**              get_bpmBoneOffset( void ) {  return m_ptrBoneOffset;  }
	osVec3D**				get_bpmBonePosOffset(){return m_ptrBonePosOffset;}
	osQuat**				get_bpmBoneRotOffset(){return m_ptrBoneRotOffset;}

	//! ����bpmInstance������.
	int                    get_faceNum( void );

	//! �����Ƿ���ʾ���⡣
	DWORD                  set_disSGlossy( int _idx );

	//! ��ǰ���������Ƿ��Ƕ�Ӧ����Ķ�����
	bool                   is_glossyAction( int _id );

	//! �õ���ǰ��������ĵ㡣
	void                   get_glossyCenter( osVec3D& _center );

	//! �õ�shadow��ص�ָ�롣
	osc_bpmShadow*          get_shadowPtr( void ) { return m_ptrShadow; } 


	//! ��ǰBodyPartMeshʵ���Ƿ�ʹ�õ��⡣
	bool                   is_glossySM( void );

	//@{
	//! �õ������õڶ�����ͼ��id.
	int                    get_secTexid( void );
	int                    get_shaderId( void );
	bool                   has_secondEffect( void );
	bool                   is_rottexEffect( void );
	bool                   is_shineTexEffect( void );

	//! river added @ 2010-3-11:�����������ͼ.
	bool                   is_rotEnvEffect( void );
	bool                   is_rotShineEffect( void );
	bool                   is_envShineEffect( void );


	osMatrix*               get_texTransMat( void ); 

	int						get_ThdTexid(void);

	//! River @ 2010-3-11:װ���ڵ����������ID.
	int                     get_fourthTexid( void );
	//@} 

	//! ��ǰbody part mesh�Ƿ���alpha Channel
	bool            has_alphaChannel( void )   { return m_bAlpha; }


	//!  Device reset���.
	void            bpmins_onLostDevice( void );
	void            bpmins_onResetDevice( void );



	//! ������������Ч����
	void            scale_weaponEffect( float _rate );

	//! �Ƿ�����.
	bool            get_hideState( void ) { return m_bHide; }


};

typedef std::vector< osc_bpmInstance*>    VEC_bpmInsPtr;




//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                     ���������Ĳ㼶��ϵ�����
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class osc_skinMesh;


/** \brief
 *  ��ȾbodyFrame�õ��Ľṹ.
 */
struct os_rsBodyFrame
{
	LPDIRECT3DDEVICE9   m_pd3dDevice;
	BOOL               m_bAlpha;
	osc_bodyPartMesh*   m_ptrPM;
	osc_bpmInstance*    m_ptrPMIns;
	os_smMeshDevdep*    m_ptrRes;
	float              m_fAlphaVal;

	//! ����IndexedBlend��Ⱦ��
	osMatrix*          m_ptrBoneMatrix;
};



/** \brief
 *  ��������㼶��ϵ���ࡣ
 *
 *  �������Ҫ��*.bfs�ļ��еõ���ʼ����Ϣ��Ȼ���ٺ�
 *  *.act��������ݺϲ������յĹ����������ݡ�
 *  
 */
class osc_bodyFrame
{
	friend class osc_skinMesh;
	friend class osc_bodyFrame;

private:

	//! ��ǰFrame��Ӧ��id.
	DWORD              m_dwID;             

	//! ��ǰ��������ľ���
	osMatrix           m_matRot;
	//! ��ǰframe�����վ���
	osMatrix           m_matCombined;	

	//!  ���ֵܽ��.
	osc_bodyFrame *     m_pSibling;
	//!  ��һ�ӽ��
	osc_bodyFrame *     m_pFirstChild;

	//! skinMesh��Ӧ�Ķ������ݡ�
   VEC_bpmInsPtr       m_vecPtrBpmIns;

   //! ��ǰbodyFrame��Ӧ�ļ��ν��(BODY PART MESH)����Ŀ
   int                m_iBMNum;

	//! ��ǰbodyFrame��Ӧ��parent skin.
	osc_skinMesh*       m_pParentSkin;

	//! River added @ 2009-5-31:���ڴ����ϲ�õ��Ĺ���λ�����ݡ�
	osMatrix           m_matBonePos;

private:
	//! ����effect Shader��constant���ݡ�
	void               set_shaderConstant( LPD3DXEFFECT _eff );


public:
	osc_bodyFrame(DWORD _id = 0);
	~osc_bodyFrame();

	
	void          set_id( DWORD _id )   { this->m_dwID = _id; }

	osc_bodyFrame *FindFrame (DWORD _dwID);
	HRESULT       InvalidateDeviceObjects();
	
	//
	// �Ե�ǰ��Frame������һ���㼶��
	void          AddFrame(osc_bodyFrame* _pframe);

	/** \brief
	 *  ������������Ⱦ������
	 *
	 *  ��������������ĵ���dip,��Ⱦ����������
	 *
	 *  \param _alpha �Ƿ��ǻ�͸����alphaƬ.
	 *  \param _fade  �Ƿ��ڵ��뵭��״̬����Ⱦ������״̬�£���alpha�����Ҫ��
	 *                alpha���һ���õ���Ⱦ��
	 */
	bool	      Render( LPDIRECT3DDEVICE9 _dev,
		           bool _alpha,bool _fade = false,float _alphaVal = 1.0f );

	//! ��Ⱦ�����Ϣ
	bool	      RenderDepth( LPDIRECT3DDEVICE9 _dev );
	

	/** \brief
	 *  �Ե������Ⱦ��ʹ�õ�����һ��������
	 *
	 *  \param bool _gdraw �Ƿ���Ⱦ���⣬���ֻ��Ⱦ������Ч������Ⱦ���������
	 */
	bool         render_swordGlossy( LPDIRECT3DDEVICE9 _dev,bool _gdraw );

	/** \brief
	 *  ���뵶��Ĺؼ�֡.
	 *
	 *  \brief  ���ص���ins��β�ĳ���
	 */
	int          add_swordGlossyKey( int& _addNum );


	/** \brief
	 *  ʹ��d3dIndexedBlend��ʱ��,ʹ�����������skinMesh������Ⱦ.
	 *
	 *  �Ժ��������������Ⱦ��ʽ,�Ը��Ӻ��������Ӳ������.
	 */
	bool          RenderIndexBlend( os_rsBodyFrame& _rbf );


	/** \brief
	 *  ʹ���豸������ݵ�id�����µ��豸������ݡ�
	 */
	void          AddMesh( const osc_bpmInstance* _pm );

	/** \brief
	 *  ��Frame�ж�Ӧ��meshʹ���µ�bpMesh�滻��
	 *
	 *  \param  _npm Ҫ�滻�ɵ��µ�part mesh Instance.
	 */
	void          change_Mesh( osc_bpmInstance* _pm,
		                      const osc_bpmInstance* _npm );


	void          LinkBones(osc_bodyFrame * _root);

	/** \brief
	 *  ʹ�þ���Ĳ㼶��ϵȥ���µ�ǰ��Frame.
	 */
	void          UpdateFrames(osMatrix & _matCur,bool _catchBoneMat = false );

	/** \brief
	 *  ���õ�ǰ��frameΪ��ʼ״̬���ͷ�ռ�õ���Դ��
	 */
	void          Reset_Frame( void );
	//@{
	// Windy mod @ 2005-8-30 19:13:35
	//Desc: ride ���ڼ���ڶ����ְ�е�λ�ã���λ�����ڷ��������ʵ������
	void		  UpdateRideBoxPos();
	//@}

	//! TEST CODE:���ڱ����㼶.
	void        through( int& _hier,os_boneSys* _boneSys,bool _sibling = true );



};

typedef osc_bodyFrame*    osc_bodyFramefPtr;



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                          �ϲ�ɲ����Ľӿں�skinMesh�Ĺ�������
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class    osc_skinMeshMgr;

//! ����ÿһ֡�������ת�������������ϸ��Χ��λ��,���ڴ�������ĵ��Χ�е�����.
struct osc_smOffsetAdjBBox
{
	//! ����İ�Χ���൱��ԭ���λ��
	osVec2D    m_vec2Offset;

	//! ��ʼ����ʱ��,����һЩsmBBox,Ȼ��ÿһ֡�����������ת,���õ��µ�bboxMin��max.
	//! �ٵõ��ƶ���ԭ���MinX��maxX
	void       set_smOffsetAdjBBox( osVec3D& _min,osVec3D& _max,osVec3D& _minOut,osVec3D& _maxOut );


	void       get_smOffsetAdjBBox( osVec3D& _vecOut,float _rotAngl );

};


//! ������Ӧ�����岿λ��
# define WEAPON_BPMID       6

/** \brief
 *  ���ϲ�Ŀ����࣬���������������Ĺ���������ʾ��
 *
 *  ����������ϲ��ṩ����ʹ�ù�������Ķ�����
 *  ������ǰ������֡��λ����Ϣ�ȡ�
 *  �������²��manager�еõ������õ���Ϣ��
 *  
 */
class osc_skinMesh : public I_skinMeshObj
{

	friend   class     osc_skinMeshMgr;
	friend   class     osc_bodyFrame;

# if __ALLCHR_SIZEADJ__
public:
	static osVec3D     m_vec3ScaleAll;
# endif 

private:

	//! Mgr�ڹ���skinMesh���ڴ������ָ�룬Ϊ��ʹ����release_character
	//! �ӿڵ�ʱ�򣬿��ԶԹ�������������
	static CSpaceMgr<osc_skinMesh>*  m_ptrVecDadded;
	static CSpaceMgr<osc_skinMesh>*  m_ptrSceneSM;

	char               m_strSMDir[MAX_BPMFNLEN];

	//! ATTENTION:�Ż��˴�Ϊ��̬������ڴ档
	osc_bodyFrame*	    m_pRoot;

	//! ��¼�������ṹ��frameָ�룬�ڴ���frame�㼶��ϵʱ��¼��
	osc_bodyFramefPtr	m_pframeToAnimate[MAX_FRAMENUM];


	//! ��ǰskinMesh��������ݡ�
	os_bodyLook			m_Look;

	//! ��ǰskinMesh�õ���boneϵͳ��
	os_boneSys*        m_ptrBoneSys;

	//! ��ǰskinMesh�õ���bone�������ݡ�
	os_boneAni*         m_ptrBoneAni;


	//! ��ǰ����������ӦbodyPartMesh��ʵ������,����ʹ���������.
	osc_bpmInstance     m_vecBpmIns[MAX_SKINPART];	

	//! ��ǰ����������mesh��Ŀ��
	int                m_iNumParts;

	//! ��ǰ��skinMesh�Ƿ���ʹ����.
	bool               m_bInUse;


	//! ��ǰ��skinMesh�Ƿ�ɼ���
	bool               m_bInView;

	/** \brief
	 *  ��ǰskinMesh��Ⱦ�˵���
	 *  
	 *  ��ΪskinMesh��Ⱦ������Ҫ��θ�������Ķ�����ʹ�������������¼�Ƿ�
	 *  ��Ⱦ���⣬���������Ӱ��Ⱦ�У����¼�������Ķ�����
	 * 
	 *  ���������֤����һ֡��,ֻ��Ⱦһ��������Ӧ�ĵ���.
	 *  
	 */
	bool               m_bRenderGlossy;



	//! ��ǰ��skinMesh���Ƿ���AlphaPart,����еĻ�����Ҫ�ڶ��ν�����Ⱦ��
	bool              m_bHasAlphaPart;


	//! ����ǰ��skinMesh��͸�����ֵ�ʱ��,�Ƿ���ҪдZbuffer
	bool              m_bWriteZbuf;



	//=================================================================================
	// �������ﶯ������ʱ��Ҫ�ͱ��ֵ����ݡ�
	// ATTENTION TO FIX: ����Щ���ݷֳ�һ���������࣬�ڴ˴����붯���л�������
	//                   ����Ѷ����ֿ��ˣ����԰�skinMeshҲ�����ǿ��Թ�����Դ���ࣿ����
	//                   
	//@{
	//! ��ǰ������id.
	int               m_iAniId;

	//! ��ǰ�������ֵ�����,һ���������ֿ��ܶ�Ӧ�����������
	int               m_iAniNameIdx;


	//! �任����ʱ�����ݵ���һ�ζ�����id.
	int               m_iLastActId;


	/** \brief
	 *  ��һ֡�������ʱ���µ�ʱ�䡣
	 *  
	 *  �ڵ�����һ֮֡ǰ�����ֵһֱ������������һ֡��ʱ��
	 *����ֵ���㣬Ȼ�������¼��㡣
	 */
	float				m_fLastUsedTime;
    //! ��ǰ�������ڵ�֡��,���ڸı䶯����ʱ������ֵ
	WORD				m_wAniIndex;
	//! ���������������ʹ�õ�ǰ�������ڵ�֡���¶����ĵ���֡���в�ֵ�������ݱ��浱ǰ�Ķ�����
	WORD              m_wLastAniIndex;


	//@{
	//! ��"������"ʱ���õ������Ͳ�ֵ���ݡ�
	float             m_fNegLastUsedTime;
	int               m_iNegAniIndex;
	//@} 



	/** \brief
	 *   ��ǰ�Ƿ�����ת����״̬��
	 *
	 *   �������״̬ʱ��������Ҫ�ӵ�ǰ������Ҫת���Ķ����ĵ�һ֡������ֵ��
	 *   Ŀǰʹ�þ�������Բ�ֵ����Ч��Ч����ͨ�����״̬������ʱ��Ϊ
	 *   ACT_CHANGESTATETIME,�ڶԶ�������FrameMove����ʱ������������ڵ�ǰ
	 *   ��״̬������Ҫʹ������ĺ������������⴦��
	 */
	bool              m_bChangeActState;




	//!  ���һ�ζ����л��Ľṹ��ÿһ�׶����������ʱ��Ҫ����
	//!  ����ṹ������Ӧ�Ķ�����
	os_newAction       m_actChange;


	//! �Ƿ񶨸�ǰ������
	bool              m_bFreezeAct;

	//! river added @ 2009-8-26: �Ƿ�����ʵʱ��������Ӱ.
	bool              m_bHideRealShadow;

	//@{
	//
	//  ����skinMesh
	//! ��ǰ���Ŷ���ʹ�õ�֡����
	DWORD				m_dwFPS;
	//! ÿһ֡�õ���ʱ�䡣
	float              m_fTPF;
	//! �ϲ�仯����ʱ,ʹ�õĶ����ٶ���Ϣ.
	float              m_fNewActSpeed;
	//
	//@}

	/** \brief
	 *  ��ǰ��������������Ĳ�ֵ������
	 *
	 *  �������ı��ʱ��ʹ�����ֵ������ʹ�����ı�ĸ������.
	 */
	float             m_fActLerpVal;

	/** \brief
	 *  �����л�ʱ���Ƿ�ʹ�����һ֡�͵�һ֡���в�ֵ��
	 *
	 *  Ĭ������£�����Ҫʹ�ò�ֵ��
	 *
	 *  ����ֵ����Ϊһ�����������������һ֡�͵�һ֡��λ�ò��ܴ󣬱������Ǵ���������
	 *  �������,�������Ժ����Ͽ�ʼ�������һ����������������£����ǲ������������ϵ�
	 *  ��һ֡���������µ����һ֡����������ٺ���һ�������ĵ�һ֡���в�ֵ��
	 *  
	 */
	bool              m_bLerpLastFAndFirstF;


	//@}
	//=================================================================================


	//! �����Ƿ���ֹͣ״̬,�任����ʱ���������µĶ������е�ֹͣ֡����ʱ��ֹͣ�������š�
	bool              m_bActStopStatus;
	//! �����Ƿ񵽴ﵱǰ�����Ĺؼ�ֹͣ֡��
	bool              m_bArriveStopFrame;

	/** \brief
	 *  �ֲ���λ��λ�û��������ϵ���ؼ����λ�ã����ڷ����ֲ�����������λ����Ч��
	 *
	 *  ֻҪ�����е��⣬�˶���λ�þ������á�
	 */
	osVec3D           m_vec3HandPos;

	//! ʹ�ö��������ͼ�����岿λ����
	int               m_iMlFlagIdx;

	//! 2009-6-7: ��ǰ�Ƿ�û�е���
	bool              m_bLoadLock;

	//! post trail effect
	static CPostTrailEff	  sm_postTrailEff;

	//! former render target(non-postTrail surface)
	static LPDIRECT3DSURFACE9 sm_formerSurBeforePostTrail;

	//! is former render target?
	static BOOL				  sm_bIsFormerSurBeforePostTrail;

	//! is post trail?
	bool						m_bPostTrailState;

	//! exclude the light effect...
	bool						m_bExcludeLightState;

	//! ��ǰ�����Ƿ����ڽ�����?
	bool                        m_bUseInInterface;

	//! River @ 2010-1-7:��������������Ч������
	float                       m_fBaseSkinScale;

	//! River @ 2010-7-1:���ﶯ������ʱ���Ƿ���ʾ����.
	bool                        m_bDisWeaponActionOver;

	//! River @ 2011-2-25:��ǰ�����Ƿ���ɾ��״̬�����ڶ��̲߳�����.
	bool                        m_bToBeRelease;
	//! River @ 2011-2-25:��ǰ�����Ƿ��ڵ����д��ڱ�־����״̬��
	int                         m_iVipMTLoadLevel;

	//! �Ƿ���һ֡�������.���̴߳���������Ч���п��ܵ��ⴴ�����ʱ��
	//! ����ҲframeMove���ˣ���Զû�л����ٽ�����Ұ�ˡ�
	bool                         m_bMustUpdateNextFrame;
public:

	//! enable post trail state
	virtual void			enable_postTrail(bool _enabled = true);

	//! exclude light effect
	virtual void			exclude_light(bool _exclude = true);

	//! is exclude light effect 
	virtual bool			is_excludeLight(void);


	//!��Ⱦ�����Post trail 
	static void					start_renderPostTrail(osc_skinMesh* _ptrSkinMesh);
	static void					end_renderPostTrail(bool _alpha);
	static void					clear_postTrail(void);
	static void					init_postTrail(void);
	static void					restore_postTrail(void);
	static void					release_postTrail(void);

	//! is post trail enabled?
	bool					    is_postTrailEnabled(void)const{return m_bPostTrailState;}

	//! �õ���ǰ��char�Ƿ�Ҫrelease.
	bool                        get_toBeRelease( void ) { return m_bToBeRelease; } 

	//! �õ���ǰchar��vip load level.
	int                         get_mtLoadLevel( void ) { return m_iVipMTLoadLevel; } 

public:
	//! ����flipDeterminant��Scale����.
	static osMatrix   m_sFDMatrix;

	//!  IdentityMatrix.
	static osMatrix   m_smatIdentity;

	//! �����ǰ����������Ĺ�,�����ڵ�ǰ��skinMesh.
	osColor           m_sCurSkinMeshAmbi;
	//! River @ 2007-7-7:
	//! ��ǰ������Ƿ����ù������Ĺ⣬���������л�ʱ������ĸ������Ա��档
	bool              m_bSetHighLight;

	//! ����ĵڶ�����ͼָ���޸�Ϊ�ڴ��ģʽ
	static CSpaceMgr<os_chaSecEffect> m_vecSecEffect;

	os_chaSecEffect*  get_sceEffectPtr( void );
	static void       release_secEffectPtr( os_chaSecEffect* _ptr );

	//! ����ĵڶ�����ͼ����Ҳʹ��Ԥ�������ʽ
	static VEC_secEffProLoad  m_vecSecEffPreLoad;
	static int                m_iSecEffPLNum;

private:

	/** 
	 *  ���ﵱǰ��λ��
	 */
	osVec3D            m_vec3SMPos;

	/** \brief
	 *  ��ǰ�����scaleVector.
	 */
	osVec3D 			m_vScale;



	//@{
	//  River added @ 2004-4-22: ������תʱ�������ֵ��
	float             m_fLastRotAgl;
	float             m_fInterpolationTime;
	bool              m_bRotInterpolateStatus;
	//@}
	//! ���ﵱǰ����ת����,��λ�ǻ��ȡ�
	float              m_fRotAgl;

	//! ��Ⱦʱ�������ת����Ϊ�����������ת����
	float              m_fFinalRotAgl;


	//! ��Ⱦʱ�Ƿ���Ⱦ��ǰ�����
	bool               m_bHideSm;


	//! ��ǰ����Ļ�׼Bounding Box.
	os_bbox              m_sBBox;
	//! ����Χ�в���ԭ���ʱ��,��Ҫʹ�����������Χ��������һЩ��Χ�е�λ��
	osc_smOffsetAdjBBox  m_sAdjBBox;

	//! ��ǰ�����Bounding Sphere�뾶��
	float              m_fBSRadius;

	//! ��ǰ�����΢��λ�á�
	osVec3D            m_vec3AdjPos;

	//! ��ǰ�����΢����ת��
	float              m_fAdjRgl;

	//! ��ɫ��ǰ�Ƿ���Բ��ŵ���,Ĭ�����ŵ���.
	bool               m_bPlaySGlossy;
	//! Ҫ���ŵ������ɫ
	DWORD              m_dwFinalGlossyColor;
	//! ��ǰ��ɫ�Ƿ񲥷��������ӵ���Ч
	bool               m_bPlayWeaponEffect;

	//! �Ƿ����ڶ��̴߳�������.
	int                m_iMT_PlayWeaponEffect;

	//@{
	//! �����������ʱ��,��������ĵ����������Ч��ʾЧ��
	bool               m_bPlaySGlossyBack;
	bool               m_bPlayWeaponEffectBack;
	//@}


	//! ��ǰ����Ķ����Ƿ�������Suspend״̬��
	bool                    m_bSuspendState;


# if __GLOSSY_EDITOR__

	//! ��������������bodyPartMesh.
	osc_bodyPartMesh*       m_ptrWeaponBpm;

	//! �����ʵ�����ݡ�
	osc_swordGlossyIns*     m_ptrSGIns; 


	//@{
	// Windy mod @ 2005-10-10 12:04:21
	//Desc: ���������8����֧�֣������༭����֧��
	//! �������������ĵ�8����bodyPartMesh.
	osc_bodyPartMesh*       m_ptrWeaponBpm8;
	//! ��8���ֵ����ʵ�����ݡ�
	osc_swordGlossyIns*     m_ptrSGIns8; 


# endif 

	//@}


	/** \brief
	 *  ��Ӧ��������BodyPartMesh��ʵ��.
	 *
	 *  ���ʵ�����е����ʵ�����ݡ������Ϊ�գ���û�е��⡣
	 *
	 *  River @ 2005-8-16: 
	 *  ����ڰ˸����岿λ����ʹ�õ��⣬����Ҫ�޸Ķദ������صı���.
	 *  �ڰ˸����岿λ������뵶�⣬��Ҫ����Ĺ����������Կ�ʼ����ֻ
	 *  �������ڼ���ڰ˸����岿λ���������ű༭�ڰ˸����岿λ�ĵ��⡣
	 *  ������֧�ֶ����Ӱ�ĸ��Ӷ�Ҫ�ߡ�
	 */
	osc_bpmInstance*        m_ptrSGBpmIns;
	//��8���ֵ����ʵ������
	osc_bpmInstance*        m_ptrSGBpmIns8;
	


	//! �����ǰ��skinMesh����Fade״̬����ֵ��ʾ��sm��alpha��Ϣ��
	float                  m_fAlphaVal;
	//! ��ǰ�Ƿ���fade״̬,Ŀǰ�������͸���Ժ󣬻ᵲס���ݵ���Ⱦ
	bool                   m_bFadeState;

	/** \brief
	 *  ��ǰ��skinMesh��Ʒ�Ƿ���Ҫ��Ⱦ��Ӱ
	 *
	 *  Ŀǰ����Ӱ��Ⱦ������ʹ��cpu������Կ�����ʣ�����ֻ����������
	 *  ����ʹ����Ӱ��ʹ��shadowSupport��skinMesh������Ҫ������ڴ���
	 *  �洢��ص����ݡ�
	 */
	bool                   m_bShadowSupport;


	/** \brief
	 *  ��ǰ��skinMesh�Ƿ�ѷ������Ϊ������
	 *  
	 *  ��Ҫ���ڳ����еĲݵ�͸����Ʒ�������������档
	 */
	bool                   m_bUseDirLAsAmbient;

	//! ��skinMesh����һ֡�Ƿ���������,��Ҫ���ڵ���͵�����Ч���µĴ���
	bool                   m_bInViewLastFrame;

	//! �豸������ݵ�ָ�롣
	static LPDIRECT3DDEVICE9       m_pd3dDevice;

	//char	mszspeFileName[MAX_PATH];
	s_string	mszSpeFileName;

	//! River mod @ 2008-11-21:����skinMesh��Χ���ӵ�λ���ƶ�����ת,������λ�õİ�Χ��,��ʹ�ô˴���
	//  ����,�������Ե�Bug����
	osVec2D                        m_vec2BBoxOffset;
	void                           process_bboxUpdata( void );

	//! River @ 2009-5-7:���ڻ�charScrRect���ܡ�
	DWORD                   m_dwLastRenderTime;


	/*
	 * River @ 2009-6-23:�����������ݺ���������
	 * ��������ÿһ���岿λ������DrawFadeGlow.fadeGlowʹ������ķ���
	 * 
	 * �ȹ��ܣ����Ż�
	 */
	bool                    m_bFakeGlowDraw;
	DWORD                   m_dwFadeGlowColor;
	/*
	bool                    m_bDrawTail;
	float                   m_fTailLength;
	DWORD                   m_dwTailColor;
	//! ����move�ķ�����ٶȣ�����Tail.
	osVec3D                 m_vec3MoveDir;
	*/

private:

	// ��ǰskinMesh�õ���bodyframe�����ݣ�m_pRoot����֮�µ�ָ�붼��������������ݡ�
	osc_bodyFrame       m_arrBodyFrame[MAX_FRAMENUM];
	//! ��¼��ǰʹ�õ���bodyFrame����Ŀ��
	int                m_iBFNum;
	//! �õ�һ������ʹ�õ�bodyFrame�ռ䡣
	osc_bodyFrame*      get_freeBF( void );

	//! ��������ʱ,�õ���ʼ��������id.
	void               get_defaultAct( os_skinMeshInit* _smi );

	//! �ͷ�����ĳһ���ֶ�Ӧ��skinShader.
	void               release_skinShader( int _idx );

	//! ���ڼ�����������Ƿ���Բ�ֵ
	bool               can_lerpInDiffAction( int _actIdx1,int _actIdx2 );

	//! �õ�����ʱ�õ���bis�ļ�����
	void               get_bisFileName( s_string& _res,s_string& _sdir );


	//@{
	// TEST CODE: 
	//! ����ȷ��Frame�����岿��Ŀ��Ϊ��ĺ���.�ڴ���������л�װ��ǰ�����.
	void              assert_frameBMNum( void );
	//@}


	//!  �����ʱ�������ڵ�ǰ���������Ĳ��Űٷֱ�
	float             cal_posePercent( void );

	//! �Ӷ��������У��õ�����Ҫ������һ�����������ܴ������������������
	int               get_aniNameIdx( void );

# if GALAXY_VERSION
	BOOL              m_bProjectShadow;
	//! ������ǰͶӰ��Ӱ�õ��ľ���
	void              construct_shadowMat( osMatrix& _shaMat );
	
# endif

	//! ��¼��ǰskinMeshFrame��mat����


private:

	/** \brief
	 *  ����ǰskinMesh��Frame Hierarchy����
	 */
	void               process_frameHierarchy( void );


	/** \brief
	 *  �滻ĳһ���岿λ��mesh.
	 *
	 *  \param _bidx    ���岿λ��id.
	 *  \param _midx    Ҫ�滻��Mesh��id.
	 */
	bool               change_bodyMesh( int _bidx,int _midx );

	/** \brief
	 *  �滻ĳһ���岿λ��skin.
	 *
	 *  \param _bidx    ���岿λ��id.
	 *  \param _midx    Ҫ�滻��Mesh��id.
     *  \param _sidx    Ҫ�滻��skin��id.
	 */
	bool               change_bodySkin( int _bidx,int _midx,int _sidx );


	/** \brief
	 *  ���ö������ŵ�ʱ����ٶ������Ϣ��
	 */
	void               set_actSpeedInfo( float _actTime );

	/** \brief
	 *  �����ǰ���ڶ����任״̬����Ҫ�������������û�������
	 *
	 */
	void               frame_moveActChange( float _ftime );

	//! ���ռ��㵱ǰ���Ƶľ�����������������ڶ����仯״̬��
	void               cal_finalPose( void );

	/** \brief
	 *  �Ե�ǰ�Ĺ�������frame move
	 */
	void               frame_moveAction( float _ftime );

	/** \brief
	 *  ����"��ʱ��"��skinMesh�������㡣
	 *
	 *  ���ڵ���ĵ��ƣ���Ӱ�ļ���ȵ�Ч����
	 *  ������������Ƕ����ĸı䣬�ڵ���֡��ʱ�򣬲��ͽ���֡����ֵ��
	 *  ���֡�����˿�ʼ֡������һֱ֡�Ӵӽ���֡��λ�ÿ�ʼ��
	 *  
	 *  �ڿ�ʼ�������֮ǰ��
	 *  \param float _ftime ��ֵ����С�ڵ����㣬�������д˺��������塣
	 */
	void               negative_frameMoveAction( float _ftime );

	//! �õ���ǰm_iNegAniIndex�ؼ�֡��Ӧ�ĵĶ�����
	void               get_keyFramePose( void );


	/** \brief
	 *  ÿ������һ�׶���ʱ�����ݵ�ǰ��actChange�ṹ�Զ������д���
	 */
	os_boneAction*               process_ActionOver( void );

	/** \brief
	 *  ����ǰskinmesh��ÿһ��frame�ĸ��¡�
	 */
	void               update_frame( bool _catchBoneMat = false );

	/** \brief
	 *  ��ʼ����ǰ��skin Mesh.
	 */
	void               reset_skinMesh( void );

	/** \brief
	 *  �������������в��������ֵ������ľ���
	 */
	void               lerp_actionPose( osMatrix& _mat,
		                  os_bactPose* _pa,os_bactPose* _pb,float _lf );

	//  River Added @ 2007-4-20:�����µĶ�����ֵ��ʽ��
	//! ������������ֵ������һ���µ�pose,ʹ�����poseȥ���µĶ����ٲ�ֵ��
	void               lerp_actionPose( os_bactPose* _res,
		                  os_bactPose* _pa,os_bactPose* _pb,float _lf );


	//! ����ڶ�����Ч��������ݡ�
	bool                read_secondTexEff( os_chaSecEffect* _seceff,char* _sname );

	//! ������ͨ�ĵ�������������ȾЧ����
	bool                create_singleTexShader( int _idx,char* _sname );

	//! ����������Ч��˫������������ȾЧ����
	bool                create_doubleTexShader( int _idx,char* _sname );

	//! ���������������ͼ����Shader
	bool                create_mlFlagShader( int _idx,char* _sname );

	//! ������ǰskinMeshSystem��shader.
	bool                create_smShader( os_skinMeshInit* _smi );

	/** \brief
	 *  ��Ⱦ��ǰ�����ĵ���
	 *
	 *  ��ʱ����ǰ�ƣ��������ǰ��֡�����λ�á��������Ӧ��Ҳ����ʵ�ֳ���������Ч������
	 *  �˵Ĳ�Ӱ�ȵȣ��Ժ���չ��
	 */
	bool                   render_smSwordGlossy( LPDIRECT3DDEVICE9 _dev );


# if __GLOSSY_EDITOR__
	//! �Ե������ݽ��д�������༭��ר�ú�����
	void                weapon_process( int _idx,osc_bodyPartMesh* _bpm );
# endif 

public:
	/** \brief
	 *  �õ���ǰskinMesh��λ�ö��㡣
	 */
	void                   get_smPos( osVec3D& _vec )         { _vec = m_vec3SMPos; }
	const osVec3D&		   get_smPos(void)const               {return m_vec3SMPos;}
	osVec3D				   get_smPos(void)	                  {return m_vec3SMPos;}
	//! �õ���skinMesh��max�������ƫ�ƶ���
	void                   get_adjPos( osVec3D& _vec )        { _vec = m_vec3AdjPos; }

	/** \brief
	 *  �õ����岿λʹ�õ�shaderId.
	 */
	int                    get_partShaderID( int _idx )        { return m_vecBpmIns[_idx].m_iShaderID; }


	/** \brief
	 *  �õ���ǰ��skinMesh�Ƿ���alphaPart.
	 */
	bool                   whether_hasAlphaPart( void )        { return m_bHasAlphaPart ; }


	//! �����ֲ�λ�ľ���
	void                   set_handPos( osVec3D& _pos )       { this->m_vec3HandPos = _pos; } 

	//! �õ���ǰskinMesh��Ӧ��Ŀ¼��
	void                   get_smDir( s_string& _str )         { _str = m_strSMDir; } 
	//@{
	// Windy mod @ 2005-8-30 19:14:28
	//Desc: ride ���õڶ����������λ��
	bool			  m_bIsRide;
	osVec3D           m_vec3RidePos;
	osVec3D			  m_vec3RideOffset;
	//!��¼�������������ָ��.
	osc_skinMesh * 	  m_HumObj;
	//!��¼�������������ָ��.
	osc_skinMesh *	  m_RideObj;

	DWORD	mEffectId;
	osVec3D mEffectOffset;

	//! ���ڸ������������Ч��λ�úͷ���
	void                    update_followEffect( void );

	//!�������ö�λ�е�λ��
	void					set_ridePos(osVec3D _pos)			{ this->m_vec3RidePos = _pos; }
	//!����
	void					LinkHumObj(osc_skinMesh * _humobj) { osassert(_humobj); m_HumObj = _humobj; }
	//!
	void					DelLinkHumObj(){ m_HumObj = NULL; }
	//!
	void					HideLastPart(bool _IsHide);
	//!ʱʵ�õ����ﶨλ�е�λ�ã�
	void					get_RideBoxPos( osVec3D& _pos );

	//! ��������߳�������changeEquipment������
	bool                   change_equipmentThread( const os_equipment& _equi );
	//! ��������߳����ͷ�����.
	bool                   release_characterThread( BOOL _sceneSm );

	//! �����ǰ������õ������������ָ�룬�п��ܷ��ؿա�
	virtual I_skinMeshObj* get_humanObjPtr( void  ); 
	virtual bool           is_rideObj( void ) { return m_bIsRide; }
 
public:
	//!����ʹ�õĶ����ӿ�


	/** \brief
	* ��������󶨺�����ｫ������һ���ƶ���
	* �ϲ�Ҫ������Ϊ����������ơ���������Ӧ�Ķ���
	*/
	void					SetRide(I_skinMeshObj * _rideobj) ;
	//!ȡ����������ͷ��κ���Դ
	void					DelRide()  ;

	//! �õ�����󶨵����������
	bool osc_skinMesh::GetRidePos( osVec3D& _vec3 );

# if __RIDE_EDITOR__
	//!�õ����һ���ֵ�λ��
	void			GetLastPartPos( osVec3D & _v3d );
	//!�������һ���ֵ�λ��
	void			SetLastPartPos( osVec3D & _v3d);
	//!�������һ���ֵ�λ�õ��ļ���
	void			SaveLastPart();
# endif
	//@}

	//@{ windy add 2006-04-13
	void	get_Default_ActName(std::string& _ActName);
	void	get_Act_List(std::vector<std::string>& _ActNameList);
	//@}



public:
	osc_skinMesh();
	~osc_skinMesh();

	/** \brief
	 *  �Ե�ǰ��skinMesh������Ⱦ��
	 *
	 *  \param ��ǰ��Ⱦ��Ҫ����ȾskinMesh�е�alpha���ֻ��Ƿ�alpha���֡�
	 *  
	 */
	bool                   render_skinMesh( LPDIRECT3DDEVICE9 _dev,bool _alpha = false );

	//!��Ⱦ���������Ϣ
	bool				   render_skinDepth(LPDIRECT3DDEVICE9 _dev);

	/** \brief
	 *  ��Ŀ¼�ļ�����һ������ϵͳ��
	 */
	bool                   load_skinMeshSystem( os_skinMeshInit* _smi );

	//! River added @ 2009-6-7:��ǰ��skinMesh�ӿڲ����Ƿ�ɲ���.���̵߳���skinMesh
	virtual bool           legal_operatePtr( void ){ return !m_bLoadLock; }

	//! River added @ 2009-8-26: ��������ļ�ʱ��Ӱ.
	virtual bool             hide_realShadow( bool _hide = true );


	/** \brief
	 *  �ı䵱ǰskinMeshObject�Ķ���.
	 */
	virtual float           change_skinMeshAction( os_newAction&  _act );
   

	//! ʹֹͣ״̬�Ķ����������š�
	virtual void             start_moveAction( void );


	/** \brief
	 *  ���ﶯ���Ƿ񵽴ﵱǰ�����Ĺؼ�֡
	 *
	 *  ����ؼ�֮֡ǰ����ֵΪ�٣�����֮�󷵻�ֵΪ�档
	 *  
	 */
	virtual bool             arrive_stopFrameOrNot( void );
	//! �õ��Ӷ�����ʼ������ֹͣ֡���õ�ʱ�䡣
	virtual float            get_timeFromStartToSF( void );




	/** \brief
	 *  �滻skinMesh��װ��.
	 */
	virtual bool            change_equipment(const  os_equipment& _equi );


	/** \brief
	 *  �ӵ�ǰ��character�õ�һ��boundingBox.
	 */
	virtual os_bbox*         get_smBBox( void );
	virtual const os_bbox*        get_smBBox( void )const ;

	virtual float            get_smBSRadius( void );

	//! Set VIP Load�����̵߳��룬��ǰskinObj���ڶ��е���ǰ�棬������õ�char���ȼ������.
	virtual void            set_vipMTLoad( void );


	//! ��ɫ����ʹ�õ���.
	virtual void            playSwordGlossy( bool _glossyEnable,bool _weaponEffEnable = true );

	//! ���̴߳���������ص�����.
	void                    playSwordGlossyThread( void );

	//! �ڲ��ú���: ��ɫ����ʹ�õ���,���Ҳ���¼����״̬��
	void                    playSwordGlossyInner( bool _glossyEnable,bool _weaponEffEnable );


	/** \brief
	 *  �л������������Ч
	 *
	 *  /param int _idx           �л�����_idx����Ч�����������Ļ�����ʹ�������Ч��
	 *  /param WEAPON_ID _weapid  ʹ�õ��߻��ǵڰ˸�������λ��
	 */
	virtual void            change_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 );


	//! �õ���ǰ֡��������ĵ㡣
	virtual void            get_swordGlossyCenter( osVec3D& _pos );

	/** \brief
	 *  �õ���ǰskinMesh������ռ��е�λ�ú���ת�Ƕ�.
	 * 
	 *  \param _pos    ����skinMesh������ռ��λ��.
	 *  \param _fangle ����skinMesh������ռ����ת�Ƕ�.
	 */
	virtual void            get_posAndAgl( osVec3D& _pos,float& _fangle );
	
	//! �õ��ֲ��ľ���
	virtual void            get_handPos( osVec3D& _pos )    { _pos = m_vec3HandPos; }

	//! �õ���ǰskinMesh����ת����
	virtual float           get_smAgl( void );

	//! �ϲ������Чʱ�õ�����ת�����������Ⱦ�õ�����������
	virtual float           get_renderSmAgl( void );


	/** \brief
	 *  ���µ�ǰ��skinmeshObj��λ�á�
	 *
	 *  �˺����������޸�skinMesh�����λ�ú���ת�Ƕ�.
	 *  �����ڲ���skinObjFramemove������skinMesh��������
	 *  
	 */
	virtual float           frame_move( osVec3D& _npos,float _nangle );

	//! ��������������λ�õĸ���
	virtual float           frame_move( osVec3D& _npos,
		                         float _nangle,bool _rideSta );


	//! �õ���ǰ������������ɵı���,���������������40%���򷵻�0.4
	virtual float           get_actionPlayPercent( void ){ return cal_posePercent(); } 

	//! �õ���ǰ���ڲ��ŵĶ������ֺ�����
	virtual int             get_curPlayActIdx( char _name[32] )const;

	//! �õ�ĳ�������¼���İٷֱȣ������ܴ���Ч��Ҫ���ܴ������Ĳ��Ŷ���ʱ���š�
	virtual float           get_actionTriggerPoint( char _name[32],int _idx = -1 );

	/** \brief
	 *  �ͷŵ�ǰCharacter��ռ�õ���Դ��
	 *
	 *  ������Ϸ�����п�����Ҫ������ͷŴ�������Դ�����������������ʵ�֡�
	 *  
	 */
	virtual void            release_character( BOOL _sceneC = false );


	/** \brief
	 *  ��ǰ�����ܹ��ж����׶������Բ��š�
	 */
	virtual int             get_totalActNum( void );

	/** \brief
	 *  �Ӷ��������ֵõ����ŵ�ǰ������Ҫ��ʱ�䡣
	 */
	virtual float           get_actUsedTime( const char* _actName,int& _actIdx );

	/** \brief
	*  �Ӷ��������ֵõ����ŵ�ǰ������Ҫ��ʱ�䡣
	*/
	virtual float           get_actUsedTime( const char* _actName );

	/** \brief
	 *  ���ص�ǰskinMesh����ʾ��
	 *
	 */
	virtual void            set_hideSkinMesh( bool _hide );

	//! is hide state?
	virtual bool			is_hideState(void) const;

	//! ���ص�ǰ�������ϰ󶨵���Ч
	virtual void            hide_followEffect( bool _hide );


	/** \brief
	 *  �Ӷ�����id�õ����ŵ�ǰ�Ķ�����Ҫ��ʱ��.
	 */
	virtual float           get_actUsedTime( int _id );

	/** \brief
	 *  �õ���ǰskinMesh������
	 */
	int                    get_smFaceNum( void );


	//! ����fade״̬��
	virtual void            set_fadeState( bool _fadeS,float _alphaV = 1.0f );


	//! ����һ��������ֵ��ϵͳ�͵�ǰ�ĳ���������ֵ���Ƚϣ�ʹ�ø����Ĺ�������ǰ��ɫ��
	virtual void            set_maxBright( osColor& _bcolor );

	//! Device reset���.
	void                   sm_onLostDevice( void );
	void                   sm_onResetDevice( void );

	/** \brief
	 *  ��ͣ��ǰ����Ķ�����
	 *
	 *  \param bool _suspend ���Ϊtrue,����ͣ���������Ϊfalse,�����������
	 */
	virtual void            suspend_curAction( bool _suspend );


	//! �����������õ���ǰweapon��Ч���ļ�����
	virtual const I_weaponEffect* get_weaponEffect( int _idx,WEAPON_ID _weapid = WEAPON7 );

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// ����Ϊ�༭����ص����ݣ������յ���Ϸ��Ⱦ�����޹ء�
# if __GLOSSY_EDITOR__
	/** brief
	 *  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
	 *
	 *  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
	 *  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
	 */
	virtual int        displaytri_bysel( int _paridx,int _triidx[24],int _trinum );


	//! �������ﵶ����β�ĳ��ȡ���Ч������6-12��������ڻ���С�ڴ�ֵ��ʹ��Ĭ�ϵ���β����
	virtual void        set_sgMoveLength( int _len );

	// �õ�������β�ĳ�������
	virtual int         get_sgMoveLength( void );



	//  ���º���ֻ�ڱ༭����ı༭����ʹ�á�
	/** \brief
	 *  �ѵ�ǰsm�е������ļ����뵽�µ��ļ��С�
	 */
	virtual bool            save_weaponFile( WEAPON_ID _weapid = WEAPON7);

	/** \brief
	 *  ���õ�N��������Ч�ļ������4����
	 * 
	 *  \param int _idx ���õڼ���������Ч�����㿪ʼ�����ֵΪ3.����3�Ļ����ڲ����Ϊ��
	 *  \param char* _effName ������Ч���ļ���
	 *  \param WEAPON_ID _weapid ���õ��߸����ǵڰ˸�������λ����Ч��
	 */
	virtual bool            set_weaponEffect( int _idx,
		                     const I_weaponEffect& _effName,WEAPON_ID _weapid = WEAPON7 ); 



	//! ������ǰ�������е���Ĵ�С��
	virtual bool            adjust_swordGlossySize( bool _start,float _size, WEAPON_ID _weapid = WEAPON7 );

	//! ��X��������ת���⡣
	virtual void            rot_swordGlossyX( float _radian , WEAPON_ID _weapid = WEAPON7) ;
	//! ��Y��������ת���⡣
	virtual void            rot_swordGlossyY( float _radian , WEAPON_ID _weapid = WEAPON7);

	//! ��X�����Y�����ƶ����⡣
	virtual void            move_swordGlossy( bool _x,float _dis, WEAPON_ID _weapid = WEAPON7 );

	//! ���ô��������ڶ�λ������ʾ��������
	virtual void            set_displayWeapon( bool _dis, WEAPON_ID _weapid = WEAPON7 );    

	//! ���赶��ĸ�����ת�Ƕȣ�ʹ��ȫ�ص�����ǰ�ĵ���
	virtual void            reset_swordGlossyPosRot( WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  ���õ����Ӧ�Ķ�������, ��������ÿһ��������Ӧ����ɫ���ݡ�
	 *
	 *  \param _actIdx    ��ʾ��һ��������Ҫ��ʾ���⡣
	 *  \param _arrColor  ��ʾ��Ӧ��һ�������������ɫ��
	 */
	virtual void            set_swordGlossyActIdx( int _actNum,int* _actIdx,
		                      DWORD* _color,const char* _glossyTex, WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  �õ�����ĵ�ǰ������Ӧ�������ݡ�
	 *  
	 *  �����λ����Ϣ����Ҫ΢����
	 *  \param int& _actNum �����ж��ٸ�����ʹ���˵���.
	 *  \param int* _actIdx ����һ���������飬����������ʹ������12Ԫ�أ�������ʹ�õ����
	 *                      �Ķ���������
	 *  \param char* _glossyTex �ϲ㴫����ִ���������һ��64�ֽڵ�char�����飬�����˵�ǰ
	 *                          ����ʹ�õĵ����������֡�
	 */
	virtual void            get_swordGlossyActIdx( int& _actNum,int* _actIdx,
		                            DWORD* _color,char* _glossyTex, WEAPON_ID _weapid = WEAPON7 );


	//! ����skinMesh��ǰ�������ӵĵ�����Ч��
	virtual void            set_sgEffect( float _pos,
		                       const I_weaponEffect& _effFName,WEAPON_ID _weapid = WEAPON7);

	//! river @ 2010-3-5:����effectPos.
	virtual void            set_sgEffectPos( float _pos,int _idx,WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  �õ�����������Ч��ص���Ϣ.
	 *
	 *  \param _effFName �ⲿ�������Ч�����ִ�,����Ӧ����128���ֽڳ�.
	 */
	virtual void            get_sgEffect( float& _pos,
		                            I_weaponEffect& _effFName, WEAPON_ID _weapid = WEAPON7 );
	//! �õ����ӵ�addPos.
	virtual void            get_sgEffectPos( float& _pos,int _idx,WEAPON_ID _weapid = WEAPON7 );


	/** \brief
	 *  ����������������תosa��Ч,ʹ��osa�ļ��������Ķ�λ���Ӿ�ȷ.
	 *
	 *  �����ǰ������Ӧ����Ч���Ͳ���osa,��˺���û��Ч��.
	 */
	virtual void            rotate_osaEffect( float _agl, WEAPON_ID _weapid = WEAPON7 );

	/** \brief
	 *  ���������ϵ���ЧЧ��
	 *
	 *  \param _scale �����1.0���򱣳�ԭ��Ч��С���䣬2.0������Ϊԭ���Ķ���.0.2����СΪԭ����5��֮1
	 *                �����������ű���С���㣬���ڲ��ı������䣬�����ڲ���ǰ��Ч�����������
	 *  \return float �����˵�ǰ��Ч�����ű�����
	 */
	virtual float           scale_weaponEffect( int _idx,float _scale,WEAPON_ID _weapin = WEAPON7 );

	//@{
	// ����Ķ�����װ���༭��ء�
	

	/** \brief 
	 *  ��ʾ�������һ������֡���ơ�
	 * 
	 *  ����Ķ������봦��suspend״̬����������������á�
	 *
	 *  \param bool _next ���Ϊtrue,��ʹ����֡�����ﶯ�������Ϊfalse,��ʹ����һ֡�����ơ�
	 */
	virtual void            next_actFramePose( bool _next );

	/** \brief
	 *  �õ�ĳһ�������ܹ��ж���֡��
	 *
	 *  \param _actName Ĭ�������ʹ�ö����������õ�֡����Frame��Ŀ��
	 *  \param _actIdx  �����ֵ���ڵ��㣬�򷵻���Ӧ������������Frame��Ŀ
	 */
	virtual int             get_actFrameNum( const char* _actName,int _actIdx = -1  );


	//! �õ���ǰ�������ڲ��Ŷ�����֡������
	virtual int             get_curActFrameIdx( void );

	//! �ĵ����skinMesh�Ĺ���ϵͳ
	//! �ĵ����skinMesh�Ĺ���ϵͳ
	virtual os_boneSys* 			get_boneSys(void) {return m_ptrBoneSys;}
	
	/** \brief
	 *  �õ����岿λ�������������ǰ���岿λ�����ڣ�����0
	 *
	 *  \param int _idx ���岿λ���������㿪ʼ��
	 */
	virtual int             get_bodyTriNum( int _idx );

	//@}


# endif
	///////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// �������岿λ�ڶ�����ͼ����Ч�༭��غ���
# if __CHARACTER_EFFEDITOR__
	/** \brief
	 *  ��������ĳһ��λ��������ͼЧ����
	 *
	 *  ������岿λԭ��û�еڶ������������Ϊʹ�õڶ�������
	 * 
	 *  \param _seceff ��������ֵΪ�գ����ԭ�������岿λʹ����Ч�������Ϊ��ʹ����Ч��
	 *  
	 */
	virtual void           update_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx );

	//! �õ���ǰ���岿λ��chaSecEffect���ݡ�
	virtual bool           get_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx );

	//! �洢�����_idx��λ�ĵڶ���������ͼЧ����
	virtual bool           save_chaSecEffect( int _idx );
# endif 


	/** \brief
	 *  ������������š�
	 *
	 *  \param osVec3D _svec ��ʾ������ά�е�������������Ҫ���ŵı�����
	 */
	virtual bool           scale_skinMesh( osVec3D& _svec );

	// Rriver @ 2010-1-15: TEST CODE:�ڲ�ʹ�ýӿڡ�
	bool                   scale_skinMesh( float _x,float _y,float _z );


# if __ALLCHR_SIZEADJ__
	//! ʹ��ȫ�ֵ�����������Ϣ���ô�������.���ڴ�������ʱ,ʹ��ȫ�ֵ�scale.
	void                  createScale_withAllChrScale( void );
# endif 

	//! �Ƿ��÷������Ϊ��������Ⱦ��
	bool                  is_useDlAsAmbi( void ) { return m_bUseDirLAsAmbient; }


# ifdef _DEBUG
	//! ������ǰskinMesh�ڵ����ݡ�
	void                  dump_debugStr( void );
# endif 

	/** \brief
	*  ����Ļһ����ά��������Ⱦ��ǰ������,�����Ⱦʹ��OrthogonalͶӰ����
	*
	*  \param _rect  Ҫ���ĸ���Ļ��������Ⱦ���
	*  \param _zoom  ���ڿ������������Զ��������,�ڲ��������Χ�е����ĵ����osVec3D( 0.0f,0.0f,0.0f)��
	*                λ���ϣ��������osVec3D( 0.0f,0.0f,-_zoom)��λ���ϡ��ϲ���Ե�����һ�����ʵ�ֵ����
	*                ʵʱ����Ⱦ��һֱʹ�����ֵ��
	*  \param float _rotAgl ���ڶ�ά�������������ת����
	*  \param _newChar �Ƿ����´��������������´���������ӿڣ����������Ķ�������ά�����ڵ�����û��
	*                  �κεĹ�ϵ
	*	\param _camera ������������Ч���Ͳ�����ȫ�ֵ����ָ�뾪����Ⱦ
	*/
	virtual void            render_charInScrRect( RECT& _rect,float _rotAgl,
							float _zoom,BOOL _newChar = FALSE ,I_camera* _camera = NULL);


	//! ���ڵ�skin���������ڵ�ʱ�򣬲�����������ʾ���һ֡�Ķ���
	void                    play_lastFrameStopAction( void );

	//! �л����ĵ���ͼ
	virtual bool            change_mlFlagTexture( const char* _tex1,
		                        const char* _tex2,const char* _tex3 );

	//! ���Եõ�ĳһ��������λ��.
	virtual void            get_bonePos( const char* _boneName,osVec3D& _pos );

	//! ���õ�������ɹ�
	void                    set_loadLock( bool _lock ) { m_bLoadLock = _lock; } 

	//! ����������Ⱦfade glow.
	virtual void            set_fakeGlow( bool _glow,DWORD _color );

	//! River @ 2010-7-1: �����Ƿ�����������������أ�����Ⱦ��������
	void                    hide_weapon( bool _hide = true );



# if GALAXY_VERSION
	//! ����ǰskinMesh��Ӧ��projectShadow.
	void                    draw_projectShadow( void );
# endif 
};

//! skinMesh���stl vector. 
typedef std::vector< osc_skinMesh >   VEC_skinMesh;
typedef std::vector< osc_skinMesh*>   VEC_skinMeshPtr;

# if 1


struct os_charLoadData
{
	//! Ҫ��������ݽṹ
	os_skinMeshInit   m_sMeshInit;

	//! Ҫ����skinMesh����
	osc_skinMesh*     m_ptrSkinPtr;

	//  
	//! �Ƿ��л�װ��
	os_equipment      m_sEquipent;
	bool              m_bChangeEquipent;
	
	//! �Ƿ��ͷ�character.
	bool              m_bReleaseChar;
	//! �Ƿ񳡾���sm.
	bool              m_bSceneSm;

	//! �Ƿ�playSwordGlossy.
	bool              m_bPlaySwordGlossy;

public:
	os_charLoadData() 
	{
		m_ptrSkinPtr = NULL;
		m_sMeshInit.m_szspeFileName[0] = NULL;
		m_sMeshInit.m_strSMDir[0] = NULL;

		//! �����ֵΪ�棬����Ҫ����skinMesh,ֱ���л������װ����
		m_bChangeEquipent = false;
		m_bReleaseChar = false;
		m_bSceneSm = false;

		//��River @ 2011-2-25: �����ֵΪ�棬��������ĵ��ⲥ���������
		m_bPlaySwordGlossy = false;

	}
};

typedef std::list<os_charLoadData>   LIST_charLoadData;

/** \brief
 *  ����Ķ��̵߳�����
 *
 *  ���ݵ�ͼ�Ķ��̵߳������޸Ķ���
 */
class osc_charLoadThread : public com_thread
{
private:
	//! River @ 2011-2-12:ʹ��list������.
	LIST_charLoadData   m_listCharLoadData;
	

	//! ��ǰ���ڴ��������ָ�롣
	int    m_iProcessPtr;
	//! �����ƽ�������ָ��
	int    m_iDataPushPtr;

	//! ��ǰ���ڵ����char.
	os_charLoadData     m_sCurLoadChar;

	//! ��Ҫ�ȴ���ɵ����װ���л������ݽṹ
	os_charLoadData     m_sVipChar;
	//! �Ƿ����ڴ���vip�ĵ��룬ֻ�ڵ����߳���ʹ�õı���
	bool                m_bVipProcess;

private:
	//! ���ƽ��Ͷ���Ҫ�������������ʱ,ʹ�õ��ٽ���.
	static CRITICAL_SECTION   m_sCharNameCS;

	//! �Ƿ�����������̵߳�event.
	static HANDLE             m_sEventLCStartMt;

	//! ��������߳��Ƿ��ڿ���״̬���ڵȴ��̱߳����ѵ���Ϣ
	static HANDLE             m_sEventLCThreadFree;

	//���˳��̵߳�event
	static HANDLE             m_sEventLCExitThread;

	//! ��ǰ�������Ƿ�������.
	static HANDLE             m_sEventCLLoadFinish;

private:
	//! �õ����Ե����character,���������û�����񣬷���false.
	bool         get_frontChar( os_charLoadData& _cl );

	
public:
	osc_charLoadThread();
	~osc_charLoadThread();

	//  
	//! ������Ӧ��ָ��.
	void                    init_mtLoadCharMgr( void );


	//! ÿһ֡�Զ��̵߳�ͼ������������д���
	void                    frame_moveCLMgr( void );

	/** \brief
	 *  ��������ָ��һ��Ҫ����ĵ�ͼ��
	 *
	 *  \param _mapName Ҫ����ͼ������.
	 *  \param _mapIdx �����ͼʱ,ʹ�õĵ�ͼ����.ATTENTION TO FIX:
	 *  \param _waitForFinish �Ƿ�ȴ�������߳���ɵ�ͼ�ĵ���,
	 *                  �˻��������������ʽ�ĵ�ͼ����.
	 */
	void                    push_loadChar( os_charLoadData* _ldata,
		                                   BOOL _waitForFinish = FALSE );

	//! ��չ�����Ҫ���������,���ͷ�������ͼʱ��������������߳����ڵ����������
	void                    reset_loadCharMgr( void );

	//! ��ǰ�Ƿ�������״̬,���ȴ����߳��ƽ�Ҫ���������
	bool                    is_freeStatus( void );

	/**
	 *  ÿһ�������඼�������صĺ���.
	 *
	 *  ������������:
	 *  
	 */
	virtual DWORD            ThreadProc();
	virtual void			 ThreadExceptionProc(); //�쳣����


	//! �����̵߳�ִ��
	void                     end_thread( void );


	//! River @ 2011-2-25:ͬ��Ҫɾ��������ָ���ٽ���.
	static CRITICAL_SECTION  m_sCharReleaseCS;

};
# endif 

//�� River @ 2009-5-10:����汾�����е����ﶼ���Լ�����Ӱ
# if 1 

# define MAX_RTSHADOWNUM  128 

# else

//! ������֧�����ļ�ʱ��Ӱ��Ŀ,Ŀǰ��Ҫ���֧������,��ѡ�˳�����Ҫ����.
# define MAX_RTSHADOWNUM  2 

# endif 

/*
   ����Character�Ŀɼ����ж�

   ÿһ�ζ�character �����µ�λ��ʱ�����õ�ǰcharacter���ڵĵ�ͼ�͸��ӡ�
   ����ԭ���ĵ�ͼ�͸��ӵ�character����ֵ��Ϊ-1.�������õ�ͼ���������µ�characterΪ
   ��ǰcharacter��id������

   ��Ⱦ��ͼ��ʱ�򣬶Ե�ǰ��ͼ�����п��Ӹ��Ӷ�Ӧ��character���ÿɼ��ԡ�

   //
   // 
   // �����FADE��ز���:
   ��ÿһ��I_skinMeshObj*��һ��osc_fadeMgr��ϳ�һ�����ݽṹ��Ȼ����ÿһ֡�ڽ���FrameMove,
   ��������fade,��ɾ���˽ṹ������������Ҫ�Դ�skinMesh���ݴ�����Ⱦ�ɰ�͸�������
   
*/


//! �������������Լ����Ժ����������
# define MAX_BEHINDWALLCHAR   3

//! ����Ķ��п��Ը���һЩ���Է����ڲ����á�
# define MAX_RIDENUM   168
//! River mod @ 20109-28:����ϵͳ�����Զ��̵߳���.
struct os_ridePtr
{
	osc_skinMesh*   m_ptrHuman;
	osc_skinMesh*   m_ptrRide;

	os_ridePtr() { m_ptrHuman = NULL;m_ptrRide = NULL; } 

};


/** \brief
 *  �����������е�skinMesh�Ĺ�������
 *  
 *  �����˵��볡��ʱ�����ļ����˹��������������
 *  �������й����д�����character�����������ݡ�
 *  
 *  ���ÿ�ŵ�ͼʹ�����64��������Ʒ�Ļ���ϵͳ��Լ��Ҫ���10M���ҵ��ڴ�����Ⱦ��Щ��Ʒ��
 *  ��Լ5.5M���ڴ洢skinMesh���顣����ټ���������صļ������أ�����cpu��gpu�ȵȣ��⽫�Ǻܴ����Դ����
 *  
 *  ATTENTION TO OPP:����Gefoce�����Կ���IndexedBlend��SkinMesh������ʽ��
 *  
 */
class osc_skinMeshMgr : public ost_Singleton<osc_skinMeshMgr>
{
	friend class osc_skinMesh;
private:

	//! �ϲ�洢���е��������������ָ��.
	os_ridePtr           m_vecRidePtrArray[MAX_RIDENUM];

	/** \brief
	 *  ����Ķ���������豸�����Դ�Ĺ�������
	 *
	 *  ʹ�����������������ͬһ��mesh��˵���������ڴ���ֻ��Ҫ��
	 *  ��һ�ݾͿ����ˡ�
	 */
	osc_smDepdevResMgr   m_devdepResMgr;

	/** \brief
	 *  ����ϵͳ�Ĺ�������
	 */
	osc_boneSysMgr       m_boneSysMgr;

	/** \brief
	 *  ������������ϵͳ�Ĺ�������
	 */
	osc_boneAniMgr       m_boneAniMgr;


	/** \brief
	 *  ���岿λ���ݵ�manager,����������ݡ�������岿�ֵ�����Ϊ0,�ͷ��ڴ档
	 */
	osc_bpmMgr           m_bpmMgr;


	/** \brief
	 *   ����Щ��Դ�ṩ�ϲ�Ŀɲ�������ָ�롣
	 *
	 *  
	 */
	CSpaceMgr<osc_skinMesh> m_vecDadded;

	//! �����ж�����Ʒʹ�õ�skinMesh�б�
	CSpaceMgr<osc_skinMesh> m_vecSceneSM;



	//! �豸������ݵ�ָ�롣
	static LPDIRECT3DDEVICE9       m_pd3dDevice;

	//! ��ǰ�����ڵ�skinMesh �������б�
	VEC_skinMeshPtr          m_vecInViewSM;
	//! ��ǰ֡���ڳ����е�skinMesh����Ŀ��
	int                    m_iInviewSMNum;

	//! ��ǰmgr�ڣ���ǽ��Ҫ��Ⱦ�������б�
	VEC_skinMeshPtr          m_vecRCBehindWall;


	//! ��ǰ��skinMeshMgr��Ⱦ�����������.
	int                    m_iRFaceNum;

	//! ��ǰ֡Ҫ��Ⱦ�ļ�ʱ��Ӱ������Ŀ.
	int                    m_iRTShadowNum;

private:
	//! ʹ����Ӱ��skinMeshָ�롣//syq
	static osc_skinMesh*           m_ptrShadowSkinPtr[MAX_RTSHADOWNUM];



private:

	//! d3dIndexedBlend��Ⱦʱ�õ�����Ⱦ״̬�顣
	static int              m_iIndexedBlendSB;

	//! ��ȾsmMgrǰ����Ⱦ״̬�ı䡣
	static int              m_iSetsmSB;
	//! ����smMgr��Ⱦʱ��״̬�ı䡣
	static int              m_iEndSetsmSB;


	//! ���̵߳�������.
	osc_charLoadThread      m_sCharLoadThread;

private:
	
	/** \brief
  	 *  �õ��������ڵ�Character���б�
	 *  
	 */
	void                   get_inViewChrList( void );

	/** \brief
	 *  �ڳ�ʼ��skinMeshMgr��ʱ��,��ʼ�����ǵ�skinMethod.
	 */
	void                   init_skinMethod( void );

	/** \brief
	 *  ����ȾskinMeshMgr֮ǰ�����豸��ͬ,������Ⱦ״̬.
	 */
	void                   set_smMgrRState( void );

	//! ����smMgrRState����Ⱦ�顣
	void                   create_smMgrrsSB( void );
	//! ��������smMgrRState����Ⱦ�顣
	void                   create_smMgrEndrsSB( void );


	/** \brief
	 *  ����indexed Blend��Ⱦʱʹ�õ���Ⱦ״̬�顣
	 */
	static void            create_IndexedBlendSB( void );

	/** \brief
	 *  �ڽ���skinMeshMgr��Ⱦ��ʱ��,���õ���Ⱦ״̬.
	 */
	void                   endset_smMgrRState( void );

	//! ��ȾskinMesh������Ӱ��һ��
	void                   render_shadowSkin( void );

	//! �ƽ�Ҫ��Ⱦ���м�ʱ��Ӱ��skinMeshָ��.
	void                   push_rtShadowSmPtr( osc_skinMesh* _smPtr );

	//! ��������������еļ�ʱ��Ӱ����,ʹ�´���Ⱦ��ʱ��,������Ӱ.
	void                   reset_rtShadow( void );

	/** \brief
	 *  ���������λ�úͷ�������������ȷ���Ƿ���Ҫ���������Ӱ
	 *
	 *  ��������λ�����������Ӱ���ڣ�����Ҫ����Ӱ���������ִ������Ӱ��
	 */
	BOOL                   is_cameraInShadowVolume( void );
	//! ����Ľ��������Ƿ���е������Ͷ����Ӱ�壬���¼��
	BOOL                   is_nearCamPlaneClipSV( float _length,int _shaIdx );

	//! ���λ�úͽ���������Ƿ񱻵��ε�ס
	BOOL                   is_camFocusWardByTerr( void );

	//! ʹ��ZPass��Ӱʱ�õ��Ĳ��Ժ���
	void                   zpass_process( void );

	//! �ҵ��봫��λ����Զ��character����,���ڳ������������ʱ,ֻ��ʾ����������.
	int                    get_farthestChrIdx( osVec3D& _pos,float& _resDis );



	//! ����ʵʱ��Ӱ����Ⱦ
	void                   shadow_draw( BOOL _meshWithoutShadow );


private:
	osc_skinMeshMgr();
	~osc_skinMeshMgr();

	//! River @ 2009-6-23:��ȾfakeGlowЧ��
	void                  render_fakeGlow( void );



# if GALAXY_VERSION
	//! Ͷ����Ӱ��״̬��
	static int             m_iProjectShadowSB;

	//! ����Ͷ����Ӱ��״̬��
	void                   create_smMgrProjectShadowSB( void );

public:
	//! ͶӰ��Ӱ�õ���ƽ��
	static osMatrix                m_matShadow;

# endif 

public:

	//! ÿһʱ��mgrֻ�ܴ���һ��mesh.
	static CRITICAL_SECTION  m_sChrCreateCS;

	/** �õ�һ��skinMeshMgr��Instanceָ��.
	 */
	static osc_skinMeshMgr* Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void         DInstance( void );


	//! ��ʼ����ǰ��skinMeshManager.
	virtual bool                init_smManager( osc_middlePipe* _mp );


	//! �ͷŵ�ǰskinMeshManager.
	void                        release_smManager( bool _finalRelease = false );


	/** \brief
	 *  ���ڴ����Ͳ����ϲ���Ҫ��SkinMeshObject.
	 *
	 *  �ɳ�����������������ϲ��ṩ����ʹ�õ�skinMeshָ�롣
	 *
	 *  \param _smi      ���ڳ�ʼ���´�����skinMeshObject.
	 *  \param _sceneSm  �Ƿ񴴽������еĶ�����Ʒ�������еĶ�����������ﶯ���ֿ��洢��
	 *  \return     �������ʧ�ܣ�����NULL.
	 */
	virtual I_skinMeshObj*       create_character( os_skinMeshInit* _smi,
		                            bool _sceneSm = false,bool _waitForFinish = false );   


	/** \brief
	 *  ����Mgr��ĳһ��skin Mesh�Ŀɼ��ԡ�
	 */
	void                set_inViewChr( int _id );

	//! �������ⷢ���仯ʱ����skinMeshMgr���д���
	void                reset_ambient( void );

	
	//! Restore ��ǰ��skinMeshManager.
	void               smMgr_onLostDevice( void );
	void               smMgr_onResetDevice( void );


	virtual bool       render_smDepthMgr();
	//
	/** \brief
	 *   ��Ⱦ��ǰ��skinMeshManager.
	 *
	 *   \param _ralpha ��ǰ��Ⱦ�Ƿ���Ⱦ��alphaͨ����ani���֡�
	 *	 \param _meshWithoutShadow ����� TRUE ��ʱ����Ⱦ ģ�� , FALSE ��ʱ����Ⱦģ�� + ��Ӱ
	 *   \param _rtype             ��ͨ��Ⱦ���Ƿֲ���Ⱦ,�ֲ���Ⱦ��Ҫ�Ѹ����sm��������Ⱦ
	 */
	virtual bool                render_smMgr( os_FightSceneRType _rtype = OS_COMMONRENDER,
		                            BOOL _ralpha = false ,BOOL _meshWithoutShadow = FALSE );

	//! frameMove��ǰ��skin mesh Mgr.
	virtual bool                framemove_smMgr( void );


	//@{
	//��  River @ 2010-1-29: ��Ⱦǽ�۵��������
	virtual bool                renderBehindWallChar( void );
	virtual void                push_behindWallChar( osc_skinMesh* _ptrMesh );
	virtual void                delete_behindWallChar( osc_skinMesh* _ptrMesh );
	//@} 

# if GALAXY_VERSION
	//! ������������skinMesh��projectShadow
	void                        draw_projectShadow( void );
# endif 

	//! ��ĳһ��Character���е�����Ⱦ����Ҫ���ڰ���ά������Ⱦ����ά����Ļ������
	bool                        render_character( osc_skinMesh* _ptrMesh );

	//! ÿһ֡��skinmeshMGR���á�
	virtual void                frame_setSmMgr( void );

	/** \brief
	 *  �õ�ĳһ��skin Mesh����������λ�á�
	 */
	void                       get_smPos( int _id ,osVec3D& _pos );

	/** \brief
	 *  �õ���ǰ��smMgr�������Ŀ.
	 */
	virtual int                 get_smMgrFaceNum( void )   { return m_iRFaceNum; } 


	//! ���������м���ɼ���skinMesh��Ʒ��
	virtual void                add_sceneInviewSm( osc_skinMesh* _ptr );



# if __ALLCHR_SIZEADJ__
	//! ���赱ǰsmMgr��ÿһ������sm��scaleֵ��
	void                        reset_smMgrScale( void );
# endif 


};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                      ����ȫ�ֱ���ֻ�ڹ�������������ļ���ʹ�á�
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! ������������������豸�����Դ��
extern osc_smDepdevResMgr*       g_smDevdepMgr;

//! �����������еĹ���ϵͳ�Ĺ�����ָ�롣
extern osc_boneSysMgr*           g_boneSysMgr;

//! �����������еĹ����������ݵĹ�����ָ�롣
extern osc_boneAniMgr*           g_boneAniMgr;

//! �����������岿�ֵĶ������ݵĹ�����ָ�롣
extern osc_bpmMgr*               g_bpmMgr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//��                   �ļ���ʽ��صĶ���
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! ��ͬ�ļ�����չ��.
# define BIS_EXTENT  ".bis"
# define ACT_EXTENT  ".act"
# define SKN_EXTENT  ".skn"
# define ACS_EXTENT  ".acs"

//! act�ļ��Ŀ�ͷ�ֽ�.
# define ACT_FILEMAGIC "act"
//! ������Ϣ�ļ��Ŀ�ͷ�ֽ�."acf"
# define ACTINFO_FILEMAGIC  "acs"

//! skn�ļ��Ŀ�ͷ�ֽ�.
# define SKN_FILEMAGIC "skn"

//! �ڶ�����ͼ���ݵ���չ��.
# define  CHASEC_EXTENT       "cse"




# include "osCharacter.inl"




# endif  // # define __OSCHARACTER_INCLUDE__

