///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_stateMagic.h
 *
 *  Desc:     ħ��demo�ͻ����õ�������״̬������ݣ����ڼ�������״̬����ʾ��Ч
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

//״̬ͼ��ṹ
struct STATEDATA
{
	int   m_iStateID;
	char  m_szStateName[STATENAME_LEN];
	char  m_chStateType;
	DWORD m_dwIconID;
	char  m_szStateComment[STATECOMMENT_LEN];
	char  m_chDisplayStyle;                     //��ʾ����
	char  m_szDisplayDesc1[STATECOMMENT_LEN/2]; //��ʾ����1
	char  m_szDisplayDesc2[STATECOMMENT_LEN/2]; //��ʾ����2
	char  m_szStateEffect[40];       //״̬��Ч
	byte  m_btEnableMove;
	bool  m_bDisplayState;           //�Ƿ���ʾ״̬
	char  m_szActName[STATENAME_LEN];

	// ��ʾ�����ĸ���
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

	//! ���ݴ����״̬�������ش�״̬��Ӧ����Ч��
	const char* getStateSpeffName( const char* _sname );

private:
	MAP_STATEDATA m_mapStateData;
	int m_iFadeStateID;
	float m_fFadeStateAlpha;
};

