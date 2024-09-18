///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_charSound.h
 *
 *  Desc:     魔法demo客户端用到的人物动作声音相关的类封装
 * 
 *  His:      River created @ 2006-7-27
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"


//! 每一个声音的属性
struct gcs_sound
{
	//! 声音文件
	char  m_szSound[32];
	//! 开始播放时间，由于动作播放时间不确定，这个时间是一个相对比例
	float m_fStartTime;
	//! 播放概率
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

// 一个动作最多对应4个音效
#define MAX_ACTSOUNDS 3
//! 动作声音的结构体
struct gcs_actSound
{
	//! 动作名字
	char       m_szActName[32];
	
	//! 当前动作名字拥有的声音个数
	DWORD      m_dwNumSound;
	//! 当前动作对应的音效组
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


//! 每个人物的音效结构
struct gcs_charSound
{
	//! 人物名称
	char  m_szCharName[32];
	//! 匹配了音效的动作个数
	DWORD m_dwActCount;    
	//! 动作音效列表
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

//! 文件头,文件记录了所有人物的绑定特效
struct ChaEffectHeader
{
	DWORD	mID;		//!< 文件ID
	DWORD	mVersion;	//!< 版本信息

	DWORD	mChaEffectOfs;	//!
	DWORD	mChaEffectNr;	//! 绑定特效的人物个数
};
struct SChaItem
{
	char	mCharName[32];
	//DWORD	mChaNameOfs;	//! 人物的名字偏移 人物名字记录如"character//nanzhan"
	//DWORD	mChaNameNr;		//!

	DWORD	mActionOfs;		
	DWORD	mActionNr;		//! 对应特效的动作个数
};
struct SActItem
{
	char	mActName[32];
	//DWORD	mActNameOfs;	//
	//DWORD	mActNamNr;		//! 动作名字

	DWORD	mEffectOfs;		//! 该人物对应的特效偏移
	DWORD	mEffectNr;		//! 该人物对应的特效个数
};
struct SEffectItem
{
	char    mEffectName[32];
	//DWORD	mEffectNameOfs;	//! 特效的名字
	//DWORD	mEffectNameNr;	
	osVec3D	mPos;			//! 特效的位置
	float	mPercent;		//! 播放特效的百分比.
	float	mRate;			//! 播放概率
};


// 动作绑定特效结构
struct ACTEFFFECT
{
public:
	std::string	mActName;
	DWORD		mActNameHash;
	std::vector<SEffectItem>	mEffectList;
};

// 每个人物对应的动作特效结构
class CChaActEffect
{
public:
	// 人物名称
	std::string mChaName;
	// 动作特效列表
	typedef std::vector<ACTEFFFECT> VEC_actEffect;
	VEC_actEffect mActEffectList;
};
typedef std::vector<CChaActEffect>   VEC_ChaEffect;
//======================================================================
//! 场景内所有人物声音的管理器
class gc_charSoundMgr
{
private:
	//! 所有的人物声音资料列表。

	
public:
	gc_charSoundMgr();

	//! 初始化全局的声音管理器
	bool    init_charSoundMgr( void );

	//! 注册一个人物
	void    register_skinMesh( I_skinMeshObj* _char ,std::string _meshName);

	//! 反注册一个人物
	void    unregister_skinMesh( I_skinMeshObj* _char );

	//! FrameMove声音管理器，用于人物的声音数据播放
	void    framemove_charSoundMgr( void );

	// 由传入的人物路径名字"character\\nanzhan"得到人物跑步的声音列表
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