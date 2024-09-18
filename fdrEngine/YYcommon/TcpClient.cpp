#include "stdafx.h"
#include "TcpClient.h"

CTcpClient  g_tcpClient;


CTcpClient::CTcpClient(void)
{
    m_bIsRun = false;
    m_hSocket = NULL;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData); 
}

CTcpClient::~CTcpClient(void)
{
	Close(); 
	WSACleanup(); 
}

bool CTcpClient::Send(const char * buf, DWORD len)
{
	if (m_hSocket == NULL) { return false; }
	return send( m_hSocket, buf, len, 0 ) != SOCKET_ERROR;
}

void CTcpClient::Close(void)
{
	m_bIsRun = false; 
	if (m_hSocket)
	{
		closesocket(m_hSocket); 
		m_hSocket = NULL; 
	}
}


bool CTcpClient::ConnectTo( char* pAddr, u_short Port)
{
	// �ر���ǰ������
	Close(); 

	// �½�һ��SOCKET�������ӷ�����
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hSocket == INVALID_SOCKET) 
	{
		m_hSocket = NULL; 
		return false;
	}

	// ���sockaddr_in�ṹ 
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( pAddr );
	clientService.sin_port = htons( Port );

	// ���ӷ�����
	if( connect( m_hSocket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) 
	{
        Close();
		return false;
	}

	m_bIsRun = true; 
	DWORD dwThreadId; 
	// ����һ���߳����ڶ�ȡ����
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, &dwThreadId); 
	if (hThread == NULL)
	{
        Close();
		return false; 
	}
	CloseHandle(hThread); 
	return true; 
}


DWORD WINAPI CTcpClient::ThreadProc( CTcpClient * lpClient )
{
    #define MAX_LEN 1024
	while (lpClient->m_bIsRun)
	{
		int bytesRecv = SOCKET_ERROR;
		char recvbuf[MAX_LEN];
		bytesRecv = recv( lpClient->m_hSocket, recvbuf, MAX_LEN, 0 ); 
		if (bytesRecv <= 0) break; 

		if(lpClient->m_pfnRecvCallBack)
		{
            lpClient->m_pfnRecvCallBack( recvbuf, bytesRecv ); 
		}

	}
	return 0; 
}


