//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSocket.cpp
 *
 *  Desc:     处理引擎中用到的网络接口。
 *
 *  His:      River created @ 2004-3-31
 *
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/fdSocket.h"

# pragma comment( lib,"Ws2_32" )


//! 用于表示数据包大小的数据字节的长度。
# define PACK_SIZELENGTH   2

//! 给外部使用的数据缓冲区
#define CUSE_BUFFER_SIZE      (12*1024)      

#define MAX_CRYPT_DECODE_BUFFER_SIZE 1024 * 10

#ifdef FD_SETSIZE
#undef FD_SETSIZE
#define FD_SETSIZE  RECV_BUFFER_SIZE
#endif

char g_crypt_encode_buffer[MAX_CRYPT_DECODE_BUFFER_SIZE] = {0};
char g_crypt_decode_buffer[MAX_CRYPT_DECODE_BUFFER_SIZE] = {0};

BOOL m_bJustConnectServer = FALSE;


# define SOCKET_RECORD  0

//! 接收数据时数据在缓冲区中的起始位置。
int osc_fdSocket::m_iRecvPos = 0;
//! 
int osc_fdSocket::m_iProcPos = 0;
BYTE* osc_fdSocket::m_pRecvBuf = NULL;
//! 操作接收缓冲区CS
CRITICAL_SECTION osc_fdSocket::m_sRecvBuf;

//!
SOCKET osc_fdSocket::m_sSocket;

HANDLE osc_fdSocket::m_sWsaEvent = NULL;

//! 退出消息接收线程的消息
HANDLE osc_fdSocket::m_sThreadExit = NULL;

//! 跟服务器的连接已经成功
BOOL osc_fdSocket::m_bConnectSuccess = false;


# if SOCKET_RECORD
FILE*   g_ptrReceive = NULL;
FILE*   g_ptrDecode = NULL;
# endif 

osc_fdSocket::osc_fdSocket()
{
	m_pRecvBuf = new BYTE[RECV_BUFFER_SIZE];

	// River @ 2005-8-30:减少发送缓冲大小，节省内存，引擎只公布发送单条消息的接口。
	m_pSendBuf = new BYTE[CUSE_BUFFER_SIZE];
	m_pClientUseDBuf = new BYTE[CUSE_BUFFER_SIZE];

	m_iSendPos = 0;
	m_iRecvPos = 0;
	m_iProcPos = 0;
	m_llRecvDataLen = m_llSendDataLen = 0;

	m_bInit = false;

	m_sWsaEvent = FALSE;
	m_threadHandle = NULL;

	// 创建临界区
	::InitializeCriticalSection( &m_sRecvBuf );

	m_sThreadExit = WSACreateEvent();
	osassertex( WSA_INVALID_EVENT != m_sThreadExit,"创建接收线程退出事件失败...\n" );

	::InitializeCriticalSection(&m_sendOneMsgSection);

#if USE_ENGINE_CRYPT
	m_pJzCrypt = _exGetjzCryptObj();
	osassert(m_pJzCrypt);
	m_bHasRecvKeyValue = FALSE;
	m_wTotalSize = m_wPackIndex = 0;
#endif //USE_ENGINE_CRYPT

# if SOCKET_RECORD
	g_ptrReceive = fopen( "d:\\bintest\\receive.bin","wb" );
	//g_ptrDecode = fopen( "d:\\t2.bin","wb" );
	fclose( g_ptrReceive );
	//fclose( g_ptrDecode );
# endif 

}


osc_fdSocket::~osc_fdSocket()
{
	close_socket();

	SAFE_DELETE_ARRAY( m_pSendBuf );
	SAFE_DELETE_ARRAY( m_pRecvBuf );
	SAFE_DELETE_ARRAY( m_pClientUseDBuf );

	::DeleteCriticalSection( &m_sRecvBuf );

#if USE_ENGINE_CRYPT
	_exDestroyjzCryptObj(m_pJzCrypt);
#endif //USE_ENGINE_CRYPT

	::DeleteCriticalSection(&m_sendOneMsgSection);
}

/*! \brief 加密算法的Key，由服务器最先发送过来。
*
*	\param _key						: key value
*/
void osc_fdSocket::set_cryptKey(char* _key,int _len)
{
	guard;

#if USE_ENGINE_CRYPT
	
	osassert(m_pJzCrypt);
	m_pJzCrypt->InitData(_key,_len);

	m_bHasRecvKeyValue = TRUE;
#endif //USE_ENGINE_CRYPT

	unguard;
}
/** \brief
*  进行我们自己的初始化工作。
*/
BOOL osc_fdSocket::WSAInitialize ( void )
{
	guard;

   WSADATA WSAData;

   if( WSAStartup(MAKEWORD(1,1), &WSAData) != 0) 
	   return FALSE;

   return TRUE;

	unguard;
}


/** \brief
*  连接到服务器的接口。
*/
bool osc_fdSocket::connect_server( const char* _hostAddr,int _port,int _WSA,HWND _hwnd )
{
	guard;

	SOCKADDR_IN    t_InAddr;

	osassert( _hostAddr );
	osassert( _hwnd );

	this->m_iRecvPos = 0;
	this->m_iSendPos = 0;
	this->m_iProcPos = 0;
	if( m_sSocket ) 
		close_socket();

	t_InAddr.sin_addr.s_addr = inet_addr( _hostAddr );
	t_InAddr.sin_family = AF_INET;
	t_InAddr.sin_port = htons( (WORD)_port );       
	m_sSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( m_sSocket == INVALID_SOCKET) {
		osDebugOut( "socket return Error code %d",::WSAGetLastError() );
		return FALSE;
	}


	// 创建之前选择使用阻塞模式
	m_sWsaEvent = WSACreateEvent();
	if( WSA_INVALID_EVENT == m_sWsaEvent )
	{
		osDebugOut( "创建网络监视事件失败...\n" );
		return false;
	}

	if( SOCKET_ERROR == WSAEventSelect( m_sSocket,m_sWsaEvent,FD_READ | FD_CLOSE | FD_CONNECT ) )
	{
		// 关闭事件
		if( m_sWsaEvent )
		{
			WSACloseEvent( m_sWsaEvent );
			m_sWsaEvent = NULL;
		}

		osDebugOut( "网络连接错误....\n" );
		return false; 
	}

	// Start the thread.
	m_threadHandle = NULL;
	::WSAResetEvent( m_sThreadExit );
	m_threadHandle = CreateThread( NULL,
		0,
		(LPTHREAD_START_ROUTINE)osc_fdSocket::recv_thread,
		this,
		0,
		(LPDWORD)&m_threadID );

	osDebugOut(va("\nconnect time .... socket index = %ld \n",m_sSocket));

	// River: 发消息前确认与服务器连接成功.
	m_bConnectSuccess = false;
	if( connect(m_sSocket,(PSOCKADDR)&t_InAddr,sizeof(t_InAddr)) ==  SOCKET_ERROR )       
	{ 
		if(  WSAEWOULDBLOCK != WSAGetLastError()  )
		{
			osDebugOut( "close_socket: WSAEWOULDBLOCK != WSAGetLastError(), is<%d>\n", WSAGetLastError() );

			// 内部的批处理,需要关闭相应的事件，下次再试
			close_socket();

			return false;
		}
	}

	m_bInit = true;
	osDebugOut( "connect_server:done.\n" );


	m_bJustConnectServer = TRUE;

	return true;

	unguard;
}


//! 当前的Socket是否关闭
bool osc_fdSocket::is_socketClosed( void )
{
	if( m_sSocket == 0 )
		return true;
	else
		return false;
}


/** \brief
*  关闭当前的连接。
*/
bool osc_fdSocket::close_socket( void )
{
	guard;
	osDebugOut( "close_socket()\n" );

	::EnterCriticalSection( &m_sRecvBuf );

	m_iRecvPos    = 0;
	m_iProcPos    = 0;

	::LeaveCriticalSection( &m_sRecvBuf );


	// 关闭事件
	if( m_sWsaEvent )
	{
		WSACloseEvent( m_sWsaEvent );
		m_sWsaEvent = NULL;
	}

	// 关闭数据接收线程
	if( m_threadHandle )
	{
		::WSASetEvent( m_sThreadExit );
		DWORD dwRet = WaitForSingleObject( m_threadHandle, 15000 ); // 等待退出
		if( dwRet == WAIT_TIMEOUT)
		{
			osDebugOut( "工作线程无法退出，强行杀死" );
			TerminateThread(m_threadHandle, 1);
		}

		BOOL  t_bRes = ::CloseHandle( m_threadHandle );
		osassert( t_bRes );
		m_threadHandle = NULL;
		osDebugOut( "close_socket:关闭线程\n" );
	}

	m_iSendPos    = 0;
	m_bInit = false;


	if( m_sSocket != 0 ) 
	{

# ifdef _DEBUG
		int t_error = WSAGetLastError();
		osDebugOut( "Close socket,WSA last Error is:%d..\n",t_error );
# endif 

		closesocket( this->m_sSocket );
		m_sSocket = NULL;
		osDebugOut( "close_socket():关闭socket\n" );
	}

	m_sSocket = 0;


#if USE_ENGINE_CRYPT
	m_bHasRecvKeyValue = FALSE;
	m_wTotalSize = m_wPackIndex = 0;
#endif //USE_ENGINE_CRYPT

	return true;

	unguard;
}


//! 网络关闭后要调用的回调函数
sockClosed_callbackFunc osc_fdSocket::m_ptrClosedCallbackFunc = NULL;

//! 注册网络关闭后调用的客户端回调函数 
bool osc_fdSocket::register_socketCloseCallback( sockClosed_callbackFunc _func )
{
	guard;

	//osassertex( _func,"参数为空" );
	m_ptrClosedCallbackFunc = _func;

	return true;

	unguard;
}


//! 读入数据的thread
int osc_fdSocket::recv_thread( void* _param )
{
	guard;

	osassert( _param );
	osc_fdSocket*   t_ptrSock = (osc_fdSocket*)_param;

	enum
	{ 
		EVENT_EXIT, EVENT_PROCESS, MAX_EVENT
	};
	HANDLE   t_sEventArray[MAX_EVENT] = { osc_fdSocket::m_sThreadExit,osc_fdSocket::m_sWsaEvent };


	while( true )
	{
		DWORD t_dwRes = WSAWaitForMultipleEvents( MAX_EVENT,t_sEventArray,false,WSA_INFINITE,false );

		if( t_dwRes == WSA_WAIT_FAILED )
		{
			osDebugOut( "t_dwRes == WSA_WAIT_FAILED\n" );
			break;
		}

		// 线程退出事件发生
		if( t_dwRes == EVENT_EXIT )
		{
			osDebugOut( "t_dwRes == EVENT_EXIT\n" );
			return 0;
		}

		WSANETWORKEVENTS t_netevent;
		memset( &t_netevent, 0, sizeof(t_netevent));

		if( WSAEnumNetworkEvents( t_ptrSock->m_sSocket,
			t_ptrSock->m_sWsaEvent, &t_netevent) == SOCKET_ERROR)
		{
			osDebugOut( "WSAEnumNetworkEvents == SOCKET_ERROR\n" );
			break;
		}

		// 连接成功或是失败的事件
		if ((t_netevent.lNetworkEvents & FD_CONNECT) == FD_CONNECT)
		{
			if (t_netevent.iErrorCode[FD_CONNECT_BIT] > 0)
			{
				osDebugOut( "跟服务器的连接失败..\n" );
				break;
			}
			else
			{
				osDebugOut( "连接成功.....\n" );

				// River: 发消息前确认与服务器连接成功.
				m_bConnectSuccess = true;
			}
		}

		// 关闭网络...
		if( (t_netevent.lNetworkEvents & FD_CLOSE )== FD_CLOSE )
		{
			int t_error = WSAGetLastError();
			osDebugOut( "接收到服务器要关闭网络的消息，关闭网络...WSAGetLastError<%d>, NetErrorCode:<%d>\n", 
				t_error, t_netevent.iErrorCode[FD_CLOSE_BIT] );

			// 
			// River @ 2009-1-5: 调用上层的网络关闭回调函数
			if( m_ptrClosedCallbackFunc )
			{
				m_ptrClosedCallbackFunc( NULL );
				m_ptrClosedCallbackFunc = NULL;
			}

			break;
		}

		// 读入消息
		if( (t_netevent.lNetworkEvents & FD_READ) == FD_READ )
		{
			if( !osc_fdSocket::receive(t_ptrSock) )	{		
				osDebugOut( "线程读取消息失败!\n" );
				break;
			}
		}
	}

	// 
	// 仅关闭socket,至于其它关闭socket的处理，由主线程来调用处理
	if( t_ptrSock->m_sSocket != 0 ) 
	{

# ifdef _DEBUG
		int t_error = WSAGetLastError();
		osDebugOut( "Close socket,WSA last Error is:%d..\n",t_error );
# endif 

		closesocket( t_ptrSock->m_sSocket );
		t_ptrSock->m_sSocket = NULL;

		osDebugOut( "线程closesocket!\n" );
	}

	osDebugOut( "线程返回!\n" );
	return 0;

	unguard;
}


//@{
/** \brief
*  接收消息。
*/
bool osc_fdSocket::receive( osc_fdSocket* _ptrSocket )
{
	guard;

	static FILE*   t_file = NULL;

	// River @ 2006-8-30: 如果缓冲区内空间太小，则等待另外的线程处理，
	// 直到缓冲区内有足够的空间,因为此处为读取数据，所以不进入临界区
	int Rest = RECV_BUFFER_SIZE - m_iRecvPos;
	while( Rest < 8192 )
	{
		::Sleep( 5 );
		osDebugOut( "Socket buffer over....\n" );
		Rest = RECV_BUFFER_SIZE - m_iRecvPos;
	}

	::EnterCriticalSection( &m_sRecvBuf );


	int tReceiveSize = recv( osc_fdSocket::m_sSocket,(char*)( m_pRecvBuf + m_iRecvPos ), Rest, 0);
	if( tReceiveSize == SOCKET_ERROR ) 
	{
		::LeaveCriticalSection( &m_sRecvBuf );
		osassertex( false ,va( "Receive Message Error:%d..\n",WSAGetLastError() )  );
		return false;
	}

	if( tReceiveSize == 0 )
	{
		::LeaveCriticalSection( &m_sRecvBuf );
		osDebugOut( "The socket is closed ....\n" );
		return false;
	}
	
	//osDebugOut( "receive byte<%d>\n", tReceiveSize );

	//
	// 接收缓冲区空间太小,必须使用收到的数据长度小于接收缓冲区的空间大小.
	// ATTENTION TO FIX:不应该溢出，使用其它的办法
	if( tReceiveSize > Rest ) 
	{
		::LeaveCriticalSection( &m_sRecvBuf );
		osassertex( false,"接收信息出错，接收缓冲区溢出..\n" );
		return false;
	}

# if SOCKET_RECORD
	g_ptrReceive = fopen( "d:\\bintest\\receive.bin","ab" );
	fseek( g_ptrReceive,0,SEEK_END ); 
	fwrite( (char*)( m_pRecvBuf + m_iRecvPos ),tReceiveSize,sizeof( char ),g_ptrReceive );
	fclose( g_ptrReceive );
# endif 

	m_iRecvPos = m_iRecvPos + tReceiveSize;
	_ptrSocket->m_llRecvDataLen += tReceiveSize;

	::LeaveCriticalSection( &m_sRecvBuf );


	//osDebugOut( "receive end , m_iRecvPos<%d>, tReceiveSize<%d>\n", m_iRecvPos, tReceiveSize );
	return true;

	unguard;
}

#if USE_ENGINE_CRYPT
	//! get the first decode message
	char* osc_fdSocket::GetDecodeMessage(WORD& _size)
	{
		guard;

		if(m_wTotalSize > m_wPackIndex){
			osassert(m_bHasRecvKeyValue);
			const WORD t_orgIndex = m_wPackIndex;

			_size = *(WORD*)(g_crypt_decode_buffer + m_wPackIndex);

			osassert(_size <= 8192);

			m_wPackIndex += _size;

			return g_crypt_decode_buffer + t_orgIndex ;
		}

		return NULL;
		unguard;
	}

	//! decode the message
	char* osc_fdSocket::DecodeMessage(char* _encode,WORD& _size)
	{
		guard;

		if(m_bHasRecvKeyValue){
			m_wTotalSize = m_pJzCrypt->decode((char*)m_pClientUseDBuf,_size,g_crypt_decode_buffer);
			_size = *((WORD*)g_crypt_decode_buffer);

			osassert(_size <= 8192);

			m_wPackIndex = _size;

			return g_crypt_decode_buffer;
		}else{
			return (char*)_encode;
		}

		unguard;
	}
#endif //USE_ENGINE_CRYPT

/** \brief
*  从消息缓冲区中读入一个消息。
*
*  ATTENTION TO FIX:
*  如果底层网络引擎和客户程序使用同一个数据缓冲，有可能破坏数据。
*  
*/
char* osc_fdSocket::read_message( WORD& _msgSize,int* _errorCode,int* _errorType )
{
	guard;
	char*      t_ptrMsg = NULL;
	osassert( _errorCode );
	osassert( _errorType );
	*_errorCode = 0;

	CCriticalSec t_recvSec(&m_sRecvBuf);

#if USE_ENGINE_CRYPT
	if(t_ptrMsg = GetDecodeMessage(_msgSize)){
		return t_ptrMsg;
	}
#endif //USE_ENGINE_CRYPT

	// 如果没有足够的空间接收我的数据。
	if( m_iProcPos >= m_iRecvPos )
	{   
		_msgSize = 0;
		m_iRecvPos = 0;
		m_iProcPos = 0;
		return NULL;
	}

	//
	// 如果当前的Socket没有初始化.
	if( !m_bInit ){
		return NULL;
	}

	if( ( m_iRecvPos - m_iProcPos) < PACK_SIZELENGTH ) 
	{
		return NULL;
	}

	WORD*  t_ptrWord;
	t_ptrWord = (WORD*)(m_pRecvBuf + m_iProcPos );
	_msgSize = *t_ptrWord;

	osassertex(_msgSize <= 8192,"服务器确认这个包的内存数据！！");

	//
	// 如果当前数据包的长度小于我们已经接收到的数据的长度，
	// 等待下一个数据包，TCP可能会把一个大的数据包分开传送。
	if( _msgSize > (m_iRecvPos-m_iProcPos) )
	{
		// River @ 2006-8-25:
		// 处理缓冲区接收不够的情形，即size大于64k的包,
		// 如果此时出现某一个消息没有接收完，则把此消息考到缓冲区的开头。
		// 下次的接收处理则从头开始
		if( m_iRecvPos == RECV_BUFFER_SIZE )
		{
			memcpy( m_pRecvBuf,(const char*)(m_pRecvBuf + m_iProcPos),
				( m_iRecvPos-m_iProcPos ) );
			m_iRecvPos = m_iRecvPos - m_iProcPos;
			m_iProcPos = 0;
		}

		return NULL;
	}

	/*t_ptrMsg = (char*)(m_pRecvBuf + m_iProcPos + 2);
	m_iProcPos += _msgSize ;
	_msgSize -= 2;*/

	// reserve the length of message
	//
	t_ptrMsg = (char*)(m_pRecvBuf + m_iProcPos + 0);
	m_iProcPos += _msgSize ;

	// 
	// ATTENTION TO FIX: 这个缓冲区上层会修改
	// River 2005-8-5: 加入新的数据缓冲区,确保底层的接收消息缓冲区不会出错.
	osassertex( _msgSize<=CUSE_BUFFER_SIZE,"Packet Overrun at rec Buffer...\n" );
	memcpy( m_pClientUseDBuf,t_ptrMsg,_msgSize );


# if SOCKET_RECORD
	static int t_iTimess = 0;
	g_ptrDecode = fopen( va( "d:\\bintest\\de%d.bin",t_iTimess++ ),"ab" );
	fwrite( (char*)m_pClientUseDBuf,_msgSize,sizeof( char ),g_ptrDecode );
	fclose( g_ptrDecode );
# endif 


#if USE_ENGINE_CRYPT

	return DecodeMessage((char*)m_pClientUseDBuf,_msgSize);

#else //USE_ENGINE_CRYPT

	return (char*)m_pClientUseDBuf;

#endif //USE_ENGINE_CRYPT

	
	unguard;
}

char* osc_fdSocket::read_buffer( WORD& _msgSize )
{
	osassertex(false,"this function is not suggested");

	//char*      t_ptrMsg;
	::EnterCriticalSection( &m_sRecvBuf );

	if( m_iProcPos >= m_iRecvPos )
	{   
		_msgSize = 0;
		m_iRecvPos = 0;
		m_iProcPos = 0;
		::LeaveCriticalSection( &m_sRecvBuf );
		return NULL;
	}
	if( m_iRecvPos > 0 && m_iProcPos < m_iRecvPos )
	{
		if( m_iRecvPos - m_iProcPos >= CUSE_BUFFER_SIZE )
		{
			memcpy( m_pClientUseDBuf, m_pRecvBuf + m_iProcPos, CUSE_BUFFER_SIZE );
			_msgSize = CUSE_BUFFER_SIZE;
			m_iProcPos += CUSE_BUFFER_SIZE;
			::LeaveCriticalSection( &m_sRecvBuf );
			return (char*)m_pClientUseDBuf;
		}
		else
		{
			memcpy( m_pClientUseDBuf, m_pRecvBuf + m_iProcPos, m_iRecvPos- m_iProcPos );
			_msgSize = m_iRecvPos- m_iProcPos;
			m_iRecvPos = 0;
			m_iProcPos = 0;
			::LeaveCriticalSection( &m_sRecvBuf );
			return (char*)m_pClientUseDBuf;
		}
	}


	::LeaveCriticalSection( &m_sRecvBuf );

	//can not run here!!!
	return NULL;
}


//@}


//@{
//! 发送相关的消息。
bool osc_fdSocket::add_message( const char* _pmsg,WORD _size )
{
	guard;

	WORD*  t_ptrWord;

	osassert( _size <= MAX_MESSAGE_SIZE );

	// 如果发送数据过大
	if( ( m_iSendPos + _size + PACK_SIZELENGTH )>= SEND_BUFFER_SIZE ) 
		return false;

	//
	// 加入数据大小的数据头，并且拷贝数据到我们的缓冲区。
	t_ptrWord = (WORD*)(m_pSendBuf+m_iSendPos);


	// 没有压缩
	//*t_ptrWord = _size + 2;
	//memcpy( (t_ptrWord + 1), _pmsg , _size );
	//*(t_ptrWord + 1) = _size;	
	//m_iSendPos += ( _size + 2 );

	memcpy( t_ptrWord, _pmsg , _size );
	m_iSendPos += _size;

	return true;

	unguard;
}

bool osc_fdSocket::send_message( void )
{
	guard;

	if( (m_iSendPos > SEND_BUFFER_SIZE) || 
		(m_iSendPos < 0 ) || (m_sSocket == 0)  )
	{  
		osDebugOut( "Error with Send Pos m_iSendPos[%d]m_sSocket[%d]...\n",m_iSendPos,m_sSocket );
		m_iSendPos = 0;
		return false;
	}

	int SendCount = 0;
	int SendTotalBufLen = 0;	//!<　实际发送长度
	bool bSendBufError = false;	//!< 发送时出错
	do 
	{

#if USE_ENGINE_CRYPT

		m_pJzCrypt->simpledealdata((char*)(m_pSendBuf+SendTotalBufLen),m_iSendPos-SendTotalBufLen);
		int t_iSend = send( m_sSocket,(const char*)m_pSendBuf+SendTotalBufLen, m_iSendPos-SendTotalBufLen, 0 );

#else
		int t_iSend = send( m_sSocket,(const char*)m_pSendBuf+SendTotalBufLen, m_iSendPos-SendTotalBufLen, 0 );

#endif //USE_ENGINE_CRYPT
		
		if (t_iSend==SOCKET_ERROR)
		{
			if( SendCount > 10 )
			{
				bSendBufError = true;
				osDebugOut( "!!!(1)osc_fdSocket::send(), m_sSocket:<%d>, SendTotalBufLen:<%d>, m_iSendPos:<%d>\n", 
					m_sSocket, SendTotalBufLen, m_iSendPos );
				break;
			}

			osDebugOut( "!!!(2)osc_fdSocket::send(), m_sSocket:<%d>, SendTotalBufLen:<%d>, m_iSendPos:<%d>\n", 
				m_sSocket, SendTotalBufLen, m_iSendPos );
			Sleep( 100 );
			SendCount ++;

			continue;
			//break;
		}
		SendTotalBufLen+=t_iSend;
	} while(SendTotalBufLen!=m_iSendPos);


	if (bSendBufError)
	{
		int t_error = WSAGetLastError();
		osDebugOut( "Send msg failed,WSA last Error is:%d..\n",t_error );
		return false;
	}

	/*int t_iSend = send( m_sSocket,(const char*)m_pSendBuf,m_iSendPos, 0 );
	if( t_iSend == SOCKET_ERROR || t_iSend != m_iSendPos )
	{  
		m_iSendPos = 0;

		int t_error = WSAGetLastError();
		osDebugOut( "Send msg failed,WSA last Error is:%d..\n",t_error );

		return false;
	}*/

	m_iSendPos = 0;
	return true; 

	unguard;
}


//! 发送单条的指令，最常用的函数。
bool osc_fdSocket::send_oneMsg( const char* _msg,WORD _size )
{
	guard;

	osassert( _msg );
	osassert( _size >= 0 );
	osassert( _size <= MAX_MESSAGE_SIZE );
	
	CCriticalSec t_sec(&m_sendOneMsgSection);

	if(is_socketClosed()){return true;}
		
	// 
	// River mod @ 2008-11-26: 确认发消息时，服务器连接成功
	if( !m_bConnectSuccess )
	{
		int   t_iWaitTimes = 0;
		while( !m_bConnectSuccess )
		{
			::Sleep( 20 );
			t_iWaitTimes ++;
			if( t_iWaitTimes > 50 )
				return false;
		}
	}

	if( !add_message( _msg,_size ) )
	{
		MessageBox( NULL, "add_message()数据累计超出最大buffer!", "socket error", MB_OK );
		return false;
	}

	if( !send_message() )
	{
		osDebugOut( "send_message()出错!,last error is:<%d>..\n",WSAGetLastError());
		return true;
	}

	m_llSendDataLen += _size;

	return true;

	unguard;
}
//@}
