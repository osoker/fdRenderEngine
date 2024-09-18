
#ifndef _FLYDREAM_ENDECODE
#define _FLYDREAM_ENDECODE

# include "windows.h"

#pragma pack(push,1)
typedef struct tag_TDEFAULTMESSAGE			//基本信息结构
{
	WORD	wIdent; 
	int		nRecog;	
	WORD	wParam;
	WORD	wTag;
	WORD	wSeries;
} _TDEFAULTMESSAGE, *_LPTDEFAULTMESSAGE;
#pragma pack(pop,1)

typedef struct tag_TMSGHEADER				//和数据服务器进行通讯的数据结构
{
	int		nCode;
	int		nSocket;
	WORD	wUserGateIndex;
	WORD	wIdent;
	WORD	wUserListIndex;
	WORD	wTemp;
	int		nLength;
} _TMSGHEADER, *_LPTMSGHEADER;

#ifdef _UNICODE
#define fnMakeDefMessage fnMakeDefMessageW
#else
#define fnMakeDefMessage fnMakeDefMessageA
#endif


__inline void WINAPI fnMakeDefMessageW(_LPTDEFAULTMESSAGE lptdm, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries)
	{ lptdm->wIdent	= wIdent; lptdm->nRecog	= nRecog; lptdm->wParam	= wParam; lptdm->wTag = wTag; lptdm->wSeries = wSeries; }


__inline void WINAPI fnMakeDefMessageA(_LPTDEFAULTMESSAGE lptdm, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries)
	{ lptdm->wIdent	= wIdent; lptdm->nRecog	= nRecog; lptdm->wParam	= wParam; lptdm->wTag = wTag; lptdm->wSeries = wSeries; }

//加密解密函数，返回值为长度
int  WINAPI fnEncryptBuf(char *pszScr,char *pszDest,WORD nLen);
int  WINAPI fnDencryptBuf(char *pszSrc,char *pszDest);



#endif //_FLYDREAM_ENDECODE