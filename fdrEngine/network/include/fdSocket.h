//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSocket.h
 *
 *  Desc:     处理引擎中用到的网络接口。
 *
 *  His:      River created @ 2004-3-31
 *
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once


# include "../../interface/osInterface.h"
# include "../../../TangSvr/TangPub/lib/jzcrypt.h"




//! 接收数据缓冲区的大小。
#define RECV_BUFFER_SIZE      (1024 * 1024 * 4)         
//! 发送数据缓冲区的大小。
#define SEND_BUFFER_SIZE      (1024 * 1024)

//! 最大的消息长度
#define MAX_MESSAGE_SIZE       8192          


//! 不启用引擎处理加密（上层处理加解密增强安全性）
#define USE_ENGINE_CRYPT   1


/** \brief
 *  实现网络接口功能的客户端类。
 *
 *  
 */
class osc_fdSocket : public I_socketMgr
{
private:

	//! 发送和接收缓冲区。
	BYTE*      m_pSendBuf;
	

	//! 给外部使用的数据缓冲区.
	BYTE*      m_pClientUseDBuf;

	//! 发送数据时数据在缓冲区中的超始位置。
	int        m_iSendPos;

	//@{
	//  接收网络数据相关的数据结构
	//! 接收数据时数据在缓冲区中的起始位置。
	static int        m_iRecvPos;
	//! 
	static int        m_iProcPos;
	static BYTE*      m_pRecvBuf;
	//! 操作接收缓冲区CS
	static CRITICAL_SECTION  m_sRecvBuf;
	static UINT_PTR   m_sSocket;
	static HANDLE     m_sWsaEvent;
	
	//! 退出消息接收线程的消息
	static HANDLE     m_sThreadExit;

	//@} 

	//! 当前的socketMgr是否已经初始化。
	bool       m_bInit;


	//@{
	//  接收数据线程相关的句柄
	//! 接收数据线程的handle.
	HANDLE     m_threadHandle;
	//! 线程的ID.
	DWORD       m_threadID;
	//@}

	//! 网络连接是否脱机模式
	BOOL       m_bOffLine;

	//! 跟服务器的连接已经成功
	static BOOL m_bConnectSuccess;


	//@{ 数据统计数据
	//! 接收到的数据统计
	union{

		struct{
			DWORD	m_dwRecvDataLenLow;
			DWORD	m_dwRecvDataLenHight;
		};

		ULONGLONG	m_llRecvDataLen;
	};

	//! 发送的数据统计
	union{

		struct{
			DWORD	m_dwSendDataLenLow;
			DWORD	m_dwSendDataLenHight;
		};

		ULONGLONG	m_llSendDataLen;
	};
	//@}

	//! 上层可能有多个线程调用 sendOneMsg 这个函数
	CRITICAL_SECTION	m_sendOneMsgSection;

#if USE_ENGINE_CRYPT
	//! the crypt object
	IjzCryptInterface* m_pJzCrypt;

	//! has been received the key value
	BOOL				m_bHasRecvKeyValue;

	//! total decode receiving message size
	WORD				m_wTotalSize;

	//! a pack decode message index
	WORD				m_wPackIndex;

	//! get the decode message firstly 
	char*				GetDecodeMessage(WORD& _size);

	//! receive the decode message
	char*				DecodeMessage(char* _encode,WORD& _size);

#endif //USE_ENGINE_CRYPT

public:
	//! 网络关闭后要调用的回调函数
	static 	sockClosed_callbackFunc  m_ptrClosedCallbackFunc;

	

public:
	osc_fdSocket();
	~osc_fdSocket();


	/** \brief
	 *  进行我们自己的初始化工作。
	 */
	BOOL                WSAInitialize ( void );


	/** \brief
	 *  连接到服务器的接口。
	 */
	virtual bool         connect_server( const char* _hostAddr,int _port,int _WSA,HWND _hwnd );
	/** \brief
	 *  关闭当前的连接。
	 */
	virtual bool         close_socket( void );

	//@{
	/** \brief
	 *  接收消息。
	 */
	static bool          receive( osc_fdSocket* _ptrSocket );

	/** \brief
	 *  从消息缓冲区中读入一个消息。
	 */
	virtual char *        read_message( WORD& _msgSize,int* _errorCode,int* _errorType );
	//@}

	//读取消息缓冲区，返回NULL即为读取完毕
	char* read_buffer( WORD& _msgSize );

	//@{

	//! 发送相关的消息。
	virtual bool          add_message( const char* _pmsg,WORD _size );
	virtual bool          send_message( void );

	//! 发送单条的指令，最常用的函数。
	virtual bool          send_oneMsg( const char* _msg,WORD _size );
	//@}

	//! 设置为脱机模式，此模式下，send_oneMsg永远返回true.
	virtual bool          set_offLineState( bool _offLine ){ m_bOffLine = _offLine; return true; }

	//! 注册网络关闭后调用的客户端回调函数 
	virtual bool          register_socketCloseCallback( sockClosed_callbackFunc _func );


	virtual bool          is_socketClosed( void );

	//! 读入数据的thread
	static int            recv_thread( void* _param );

	/*! 获得发送数据的总长度
	*	
	*	\param _dwDataLenHight			: DWORD长度的高位
	*	\param _llDataLen				: 总的长度
	*	\return DWORD					: DWORD长度的低位
	*/
	virtual DWORD		  get_sendDataLen(DWORD* _dwDataLenHight = NULL)const
	{
		return ((_dwDataLenHight)?(*_dwDataLenHight=m_dwSendDataLenHight):((void)0)),m_dwSendDataLenLow;
	}

	/*! 获得接受数据的总长度
	*	
	*	\param _dwDataLenHight			: DWORD长度的高位
	*	\param _llDataLen				: 总的长度
	*	\return DWORD					: DWORD长度的低位
	*/
	virtual DWORD		  get_recvDataLen(DWORD* _dwDataLenHight = NULL)const
	{
		return ((_dwDataLenHight)?(*_dwDataLenHight=m_dwRecvDataLenHight):((void)0)),m_dwRecvDataLenLow;
	}

	/*! \brief 加密算法的Key，由服务器最先发送过来。
	*
	*	\param _key						: key value
	*	\param _len						: key length;
	*/
	virtual void		  set_cryptKey(char* _key,int _len);

};
