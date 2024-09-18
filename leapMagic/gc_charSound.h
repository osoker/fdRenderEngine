///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_charSound.h
 *
 *  Desc:     ħ��demo�ͻ����õ������ﶯ��������ص����װ
 * 
 *  His:      River created @ 2006-7-27
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"


//! ÿһ������������
struct gcs_sound
{
	//! �����ļ�
	char  m_szSound[32];
	//! ��ʼ����ʱ�䣬���ڶ�������ʱ�䲻ȷ�������ʱ����һ����Ա���
	float m_fStartTime;
	//! ���Ÿ���
	float m_fPlayChance;
	gcs_sound()
	{
		m_szSound[0] = 0;
		m_fStartTime = 0.0f;
		m_fPlayChance= 1.0f;
	}
	gcs_sound& operator=( const gcs_sound& _s )
	{
		strcpy( m_szSound, _s.m_szSound );
		m_fStartTime = _s.m_fStartTime;
		m_fPlayChance= _s.m_fPlayChance;
		return *this;
	}
};

// һ����������Ӧ4����Ч
#define MAX_ACTSOUNDS 3
//! ���������Ľṹ��
struct gcs_actSound
{
	//! ��������
	char       m_szActName[32];
	
	//! ��ǰ��������ӵ�е���������
	DWORD      m_dwNumSound;
	//! ��ǰ������Ӧ����Ч��
	gcs_sound  m_vecSounds[MAX_ACTSOUNDS];

	gcs_actSound()
	{
		m_szActName[0] = 0;
		m_dwNumSound   = 0;
	}
    gcs_actSound& operator=( const gcs_actSound& _as )
	{
		strcpy( m_szActName, _as.m_szActName );
		m_dwNumSound = _as.m_dwNumSound;
		for ( int i=0; i<MAX_ACTSOUNDS; i++ )
			m_vecSounds[i] = _as.m_vecSounds[i];

		return *this;
	}
};
struct gcs_ACTSoundData
{
	std::string mActName;
	DWORD		mActNameHash;
	std::vector<gcs_sound>	mSoundItemList;
	gcs_ACTSoundData& operator = (const gcs_actSound& rhs)
	{
		mSoundItemList.clear();
		mActName = rhs.m_szActName;
		mActNameHash = string_hash(rhs.m_szActName);
		for (DWORD i = 0; i < MAX_ACTSOUNDS; i++)
		{
			if (strlen(rhs.m_vecSounds[i].m_szSound))
			{
				mSoundItemList.push_back(rhs.m_vecSounds[i]);
			}
		}
		return *this;
	}
};
typedef std::vector<gcs_actSound>   VEC_actSound;


//! ÿ���������Ч�ṹ
struct gcs_charSound
{
	//! ��������
	char  m_szCharName[32];
	//! ƥ������Ч�Ķ�������
	DWORD m_dwActCount;    
	//! ������Ч�б�
	std::vector<gcs_ACTSoundData> m_vecActSounds;

	gcs_charSound( void )
	{
		m_szCharName[0] = 0;
		m_dwActCount    = 0;
//		m_vecActSounds = NULL;
	}
	gcs_charSound& operator = (const gcs_charSound& rhs)
	{
		memcpy(m_szCharName,rhs.m_szCharName,32);
		m_dwActCount = rhs.m_dwActCount;
		m_vecActSounds = rhs.m_vecActSounds;
		return *this;
	}
	void Clear()
	{
		m_vecActSounds.clear();
	}
	~gcs_charSound()
	{
		Clear();
	}
};
typedef std::vector<gcs_charSound>   VEC_ChaSound;
//====================================================================

#define OSMAKEFOURCC(ch0, ch1, ch2, ch3)           \
	((size_t)(unsigned char)(ch0) | ((size_t)(unsigned char)(ch1) << 8) |       \
	((size_t)(unsigned char)(ch2) << 16) | ((size_t)(unsigned char)(ch3) << 24 ))
#define CHAEFFECTID OSMAKEFOURCC('E','F','F','\0')

//! �ļ�ͷ,�ļ���¼����������İ���Ч
struct ChaEffectHeader
{
	DWORD	mID;		//!< �ļ�ID
	DWORD	mVersion;	//!< �汾��Ϣ

	DWORD	mChaEffectOfs;	//!
	DWORD	mChaEffectNr;	//! ����Ч���������
};
struct SChaItem
{
	char	mCharName[32];
	//DWORD	mChaNameOfs;	//! ���������ƫ�� �������ּ�¼��"character//nanzhan"
	//DWORD	mChaNameNr;		//!

	DWORD	mActionOfs;		
	DWORD	mActionNr;		//! ��Ӧ��Ч�Ķ�������
};
struct SActItem
{
	char	mActName[32];
	//DWORD	mActNameOfs;	//
	//DWORD	mActNamNr;		//! ��������

	DWORD	mEffectOfs;		//! �������Ӧ����Чƫ��
	DWORD	mEffectNr;		//! �������Ӧ����Ч����
};
struct SEffectItem
{
	char    mEffectName[32];
	//DWORD	mEffectNameOfs;	//! ��Ч������
	//DWORD	mEffectNameNr;	
	osVec3D	mPos;			//! ��Ч��λ��
	float	mPercent;		//! ������Ч�İٷֱ�.
	float	mRate;			//! ���Ÿ���
};


// ��������Ч�ṹ
struct ACTEFFFECT
{
public:
	std::string	mActName;
	DWORD		mActNameHash;
	std::vector<SEffectItem>	mEffectList;
};

// ÿ�������Ӧ�Ķ�����Ч�ṹ
class CChaActEffect
{
public:
	// ��������
	std::string mChaName;
	// ������Ч�б�
	typedef std::vector<ACTEFFFECT> VEC_actEffect;
	VEC_actEffect mActEffectList;
};
typedef std::vector<CChaActEffect>   VEC_ChaEffect;
//======================================================================
//! �������������������Ĺ�����
class gc_charSoundMgr
{
private:
	//! ���е��������������б�

	
public:
	gc_charSoundMgr();

	//! ��ʼ��ȫ�ֵ�����������
	bool    init_charSoundMgr( void );

	//! ע��һ������
	void    register_skinMesh( I_skinMeshObj* _char ,std::string _meshName);

	//! ��ע��һ������
	void    unregister_skinMesh( I_skinMeshObj* _char );

	//! FrameMove����������������������������ݲ���
	void    framemove_charSoundMgr( void );

	// �ɴ��������·������"character\\nanzhan"�õ������ܲ��������б�
	gcs_ACTSoundData* get_CharRunSoundList(std::string _chaPath);
private:
	static	DWORD	sActRunHashName;
	bool LoadActSndFile( const char* _szSndFile );
	VEC_ChaSound	m_vecCharSounds;

	struct CallBackUnit
	{
		I_skinMeshObj* mpSkin;
		gcs_charSound	mActData;
		bool operator == (const I_skinMeshObj* pSkin)
		{
			return (mpSkin == pSkin);
		}
	};
	std::vector<CallBackUnit> mCBListSound;
//=======================================
	bool LoadActEffFile( const char* _szSndFile );
	VEC_ChaEffect	mCharEffectList;

	struct CallBackUnitEffect
	{
		I_skinMeshObj* mpSkin;
		CChaActEffect	mActData;
		bool operator == (const I_skinMeshObj* pSkin)
		{
			return (mpSkin == pSkin);
		}
	};
	std::vector<CallBackUnitEffect> mCBListEffect;
	//! 
	I_fdScene* mptrSceneMgr;
	I_soundManager* m_ptrSoundMgr;
};
extern gc_charSoundMgr* g_pCharSoundMgr;