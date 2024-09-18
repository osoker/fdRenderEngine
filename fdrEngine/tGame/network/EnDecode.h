
#ifndef _FLYDREAM_ENDECODE
#define _FLYDREAM_ENDECODE

# include "windows.h"

#pragma pack(push,1)
typedef struct tag_TDEFAULTMESSAGE			//������Ϣ�ṹ
{
	WORD	wIdent; 
	int		nRecog;	
	WORD	wParam;
	WORD	wTag;
	WORD	wSeries;
} _TDEFAULTMESSAGE, *_LPTDEFAULTMESSAGE;
#pragma pack(pop,1)

typedef struct tag_TMSGHEADER				//�����ݷ���������ͨѶ�����ݽṹ
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

//���ܽ��ܺ���������ֵΪ����
int  WINAPI fnEncryptBuf(char *pszScr,char *pszDest,WORD nLen);
int  WINAPI fnDencryptBuf(char *pszSrc,char *pszDest);



#endif //_FLYDREAM_ENDECODE