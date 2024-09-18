#pragma once
#include "winsock2.h"
#pragma comment(lib, "WS2_32.Lib")


class CTcpClient
{
public:
	typedef void (*PFN_RECVDATA)( const char* buf, DWORD len); 

public:
	CTcpClient(void);
	~CTcpClient(void);

	void Close(void); 
	bool Send(const char * buf, DWORD len); 
	bool ConnectTo(char* pAddr, u_short Port); 
    void SetRecvFunc( void* pfnRecv ) { m_pfnRecvCallBack = (PFN_RECVDATA)pfnRecv; }

private:
	static DWORD WINAPI ThreadProc( CTcpClient * lpParam ); 

private:
	bool m_bIsRun; 
	SOCKET m_hSocket; 
	PFN_RECVDATA m_pfnRecvCallBack; 
};

extern CTcpClient  g_tcpClient;

