# pragma once
//===================================================================
/** \file  
*  Filename: Character.h
*  Desc:     �����н�ɫ����Ķ���
*  His:      ���� @ 1/12 2004 16:18
*/
//===================================================================

# include "newAstar.h"

//============================================================================

/**
*	��Ԫ���С
*/
#define	 BASESIZE					TILE_WIDTH/2.0f
/**
*	������״̬�Ķ���
*/
#define  OBJ_STATE_WAIT				0
#define  OBJ_STATE_WALK				1
#define  OBJ_STATE_RUN				2
#define  OBJ_STATE_ACTION			3
#define  OBJ_STATE_ACT				4
#define  OBJ_STATE_BEACT			5
#define  OBJ_STATE_DEATH			6

#define  OBJ_STATE_LEAF				7

//�ȴ���������״̬

/**
*	!ini�ļ�key-value�Ķ���
*/
#define  HERO						"HERO"
#define  NPC						"NPC"
#define  NAME						"����"
#define  DIRTCTION					"Ŀ¼��"
#define  FIELDX						"FIELDX"
#define  FIELDY						"FIELDY"
#define  POSX						"POSX"
#define  POSY						"POSY"
#define  HEIGHT						"HEIGHT"
//!��������ֻ��Ŀǰ��npc��Ч
#define  PATHPOSX1					"·��һX1"
#define  PATHPOSY1					"·��һY1"
#define  PATHPOSX2					"·����X2"
#define  PATHPOSY2					"·����Y2"

#define  ANGLE						"�Ƕ�"

#define  SHADEDIR					"Character\\"

class I_Hero;
class I_Npc;
class CNpcManager;


//extern 	os_skinMeshInit   g_smInitOne;
//! ��4��8λ�ַ������޷���32λ������ֵ��
#define OSMAKEFOURCC(ch0, ch1, ch2, ch3)			\
	((DWORD)(unsigned char)(ch0) | ((DWORD)(unsigned char)(ch1) << 8) |		\
	((DWORD)(unsigned char)(ch2) << 16) | ((DWORD)(unsigned char)(ch3) << 24 ))

#define ChaFileID OSMAKEFOURCC('C','H','A','\0')

struct ChaHeader
{
	ChaHeader():mbUseDirLAsAmbient(FALSE),mAttackedPos(0.0f,0.0f,0.0f)
	{}
	DWORD	mID;		//!< �ļ�ID
	DWORD	mVersion;	//!< �汾��Ϣ

	DWORD	mHeadPicFileNameOfs;	//!< ����ͷ���ļ���ƫ��,Char����
	DWORD	mHeadPicFileNameNr;		

	int		mTotalActTypeNum;
	int		mPartNum;

	osVec3D	mBoxMax;
	osVec3D	mBoxMin;

	osVec3D	mPos;
	float		mRot;

	osVec3D	mDeathBoxMax;
	osVec3D	mDeathBoxMin;

	BOOL		mbUseDirLAsAmbient;

	osVec3D	mAttackedPos;

	//! River added @ 2007-6-4:������﹥���Ŀ�ʼλ��
	osVec3D	mStartPos;


	BOOL		mbIsRide;	//!< �Ƿ�������
	osVec3D	mRidePos;	//!< ����΢����

	DWORD	mEffectOfs;
	DWORD	mEffectNr;
	DWORD	mReserved3;
	DWORD	mReserved4;
};

// ���������Ч 
struct SChaEffectItem
{
	DWORD mNameOfs;	
	DWORD mNameNr;
	osVec3D	mEffectOffset;
};



/**brief
*!�������ӿ�
*/
struct I_Character
{
	//!����
	std::string					m_strName;
	//!��I_fdScene�����Ķ���
	I_skinMeshObj				*m_pObj;
	//�����岿�ֵ�����
	int							m_nPartNum;
	//����ǰ״̬
	UINT						m_nState;
	//!����ֵ
	int							m_nLife;
	//!��ʼλ��
	osVec3D						m_verPos;
	//!��ɫ�����ĸ߶�
	float						m_fHeight;
	//!��ת�ĽǶ�
	float						m_fAngle;
	//!��ǰ�ķ���
	osVec3D						m_vCurDir;
	//!��ɫ���ٶ�
	float						m_fSpeed;
	//!Ѱ·�ҵ���·����
	/*std::list<BaseMesh>*/PATH			m_VECPath;
	//!���ﵱǰ���ڵĸ���
	BaseMesh					m_sBaseMesh;
	//!��ǰ�������ڸ��ӵĸ߶�ֵ
	float						m_fHei;
	//!�������ڵĵ�ǰ��ͼ
	int							m_nFieldX;
	int							m_nFieldY;
	//!װ���б�
	std::map<s_string, os_equipment>   m_MAPequipment;
	//!���õ�ȫ��װ��������
	int							m_nTotalEquipCount;
	//!���õ�ȫ��װ��������
	std::vector<s_string>		m_VECEquipName;
	//����ǰװ��������
	s_string					m_strCurEquipName;
	//!��ǰװ��
	os_equipment			    *m_pCurEquip;
	//!ȫ���Ķ�������
	int							m_nTotalActCount;
	//!��ǰ����
	int						    m_nCurAct;
	//!��ǰ����ִ��һ������Ҫ��ʱ��
	float						m_fFrameTime;
	//!ִ�ж����Ѿ��ù���ʱ��
	float						m_fActTime;
	//!�任����
	osMatrix				    m_matWorld;
};
/**brief
*!���ǽӿڶ���
*/
class I_Hero : public I_Character
{
public:
	I_Hero(void);
	~I_Hero(void);

	int							m_LightID;
public:
	//!����·��
	bool FindPath(int _nX, int _nY, int _X, int _Y);

	//!�ƶ�����
	void Move(float _fTime);

	//!�任װ��
	void ChangeEquipment();

	//!�任����
	void ChangeAction();
	//!�任����2
	void ChangeAction(int _nCurAct);

	//!������Χ�İ˸�����
	void FindMesh(CNpcManager *_pNpcManager, float _fTime);


	//! ����Ӣ�۵�ʱ�����Ӣ�۵����еĶ���,������������л�Ӣ�۵Ķ�������һ������.
	void change_nextAction( void );


	//! ����ǰCharacter��С���ƶ�������ƽ�����˶������λ�á�
	void move_heroX( float _xdis );
	void move_heroZ( float _zdis );
	void move_heroY( float _ydis );

private:
	//!�õ���ǰλ����������ͼ�е�����,��������һ�����ӵ����ĵ�
	void  GetCurMeshIndex();


public:
    //����ǰ���ڹ����Ķ���
	I_Npc*     m_pcNpc;

	//! ֻ����·�Ķ�����������.
	bool       m_bSoundPlay;

	//! һ��������ʱ�䳤��.
	float      m_fActTime;
	//! ����ʱ��ı�����ʼ��������.
	float      m_fS1,m_fS2;
	bool       m_bS1,m_bS2;

	//! �������ﶯ��������frameMovel
	void       frame_moveSnd( void );


};
/**brief
*!NPC�ӿڶ���
*/
class I_Npc : public I_Character
{
public:
	I_Npc(void);
	~I_Npc(void);

public:
	//!Ԥ���趨�õ������ڵ�
	std::vector<BaseMesh>		m_VECNode;
	
public:
	//!����·��
	bool FindPath();
	//!�ƶ�����
	void Move(float _fTime);
	//!�任����
	void ChangeAction();
	//!�任����2
	void ChangeAction(int _nCurAct);

	BOOL operator == ( CONST I_Npc& t_Npc )
	{
		return (m_strName == t_Npc.m_strName);
	}
	BOOL operator == ( CONST s_string& _strName )
	{
		return (m_strName == _strName);
	}
private:
	//!�õ���ǰλ����������ͼ�е�����,��������һ�����ӵ����ĵ�
	void  GetCurMeshIndex();
};

/**brief
* !NPC�����ඨ��
*/
class CNpcManager
{
public:
	CNpcManager(void);
	~CNpcManager(void);

public:
	//!��һ��Ŀ¼�¶�ȡȫ����Npc��ini�ļ�
	bool Create(char *_pchDirName);

	void Reset();

	//!����ȫ��npc��λ��
	void UpdatePos(float _fTime);

	//!�������ֵõ�npc
	I_Npc*	GetNpcFromName(s_string& _strName);

	//!��������ɾ��һ��npc
	void	RemoveNpcFromName(s_string& _strName);

	//!�õ�ȫ����npc�б�
	std::vector<I_Npc*>* GetTotalNpcList(){
		return &m_VECNpcs;
	}
	std::vector<I_Npc*>		m_VECNpcs;
};
/**brief
* !��һ��ini�ļ�װ��һ��hero
*/
bool LoadHeroFromIni(char *_chPath, I_Hero *_pHero);




/**brief
*!����ӵ�ͼ���ӵ����������������λ��
*/
osVec2D FromIndexToVertex(int _nX, int _nY, int _X, int _Y);

/**brief
*!����������ͼ���ӵ����������������λ��
*/
osVec2D FromWorldIndexToVertex(int _X, int _Y);

/**brief
*!����ӱ��ص�ͼ���ӵ�������������ӵ�����
*/
void FromLocalToWorld(int _nX, int _nY, int _X, int _Y, BaseMesh& _BaseMesh);

/**brief
*!����������ͼ���ӵ����������ظ��ӵ�����
*/
void FromWorldToLocal(int _X, int _Y, int& _nX, int& _nY, BaseMesh& _BaseMesh);

bool   gfEqual(  float _1, float _2  );