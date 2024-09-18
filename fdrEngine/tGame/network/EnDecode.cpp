// EnDecode.cpp : Defines the entry point for the DLL application.
//

# include "endecode.h"



int WINAPI fnEncryptBuf(char *pszScr,char *pszDest,WORD nLen)
{
	nLen +=2;
	memmove(&pszDest[0],&nLen,2);
	memmove(&pszDest[2],pszScr,nLen-2);	
	return nLen;
}

/*
int  WINAPI fnDencryptBuf(char *pszSrc,char *pszDest)
{
	int nLen = GetPaketLen(pszSrc);
	
	memmove(pszDest,&pszSrc[2],nLen-2);
	nLen -= 2;
	pszDest[nLen] = '\0';
	return nLen;
}
*/