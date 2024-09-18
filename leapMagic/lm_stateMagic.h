///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_stateMagic.h
 *
 *  Desc:     魔法demo客户端用到的人物状态相关数据，用于加入人物状态后显示特效
 * 
 *  His:      River created @ 2006-8-7
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"


#define STATENAME_LEN		64
#define STATECOMMENT_LEN	256
#define STATECONFILENAME	"ui\\g_state_fd.sta"
#define STATEFILE_FLAG		"sta"
#define STATEFILE_VER		0x00

#pragma pack(push,1)

#define  STATE_CANNOTMOVE     0X01
#define  STATE_CANNOTSPELL    0X02
#define  STATE_CANNOTATTACK   0X04

//状态图标结构
struct STATEDATA
{
	int   m_iStateID;
	char  m_szStateName[STATENAME_LEN];
	char  m_chStateType;
	DWORD m_dwIconID;
	char  m_szStateComment[STATECOMMENT_LEN];
	char  m_chDisplayStyle;                     //显示类型
	char  m_szDisplayDesc1[STATECOMMENT_LEN/2]; //显示描述1
	char  m_szDisplayDesc2[STATECOMMENT_LEN/2]; //显示描述2
	char  m_szStateEffect[40];       //状态特效
	byte  m_btEnableMove;
	bool  m_bDisplayState;           //是否显示状态
	char  m_szActName[STATENAME_LEN];

	// 显示描述的个数
	DWORD m_dwDescCount;
	char** m_pDispDesc;
};

typedef std::map< int, STATEDATA > MAP_STATEDATA;

class CStateIconMgr
{
public:
	CStateIconMgr();
	~CStateIconMgr();
	bool         init();

	//! 根据传入的状态名，返回此状态对应的特效名
	const char* getStateSpeffName( const char* _sname );

private:
	MAP_STATEDATA m_mapStateData;
	int m_iFadeStateID;
	float m_fFadeStateAlpha;
};

