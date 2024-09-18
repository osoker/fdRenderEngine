//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSocket.h
 *
 *  Desc:     �����������õ�������ӿڡ�
 *
 *  His:      River created @ 2004-3-31
 *
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once


# include "../../interface/osInterface.h"
# include "../../../TangSvr/TangPub/lib/jzcrypt.h"




//! �������ݻ������Ĵ�С��
#define RECV_BUFFER_SIZE      (1024 * 1024 * 4)         
//! �������ݻ������Ĵ�С��
#define SEND_BUFFER_SIZE      (1024 * 1024)

//! ������Ϣ����
#define MAX_MESSAGE_SIZE       8192          


//! ���������洦����ܣ��ϲ㴦��ӽ�����ǿ��ȫ�ԣ�
#define USE_ENGINE_CRYPT   1


/** \brief
 *  ʵ������ӿڹ��ܵĿͻ����ࡣ
 *
 *  
 */
class osc_fdSocket : public I_socketMgr
{
private:

	//! ���ͺͽ��ջ�������
	BYTE*      m_pSendBuf;
	

	//! ���ⲿʹ�õ����ݻ�����.
	BYTE*      m_pClientUseDBuf;

	//! ��������ʱ�����ڻ������еĳ�ʼλ�á�
	int        m_iSendPos;

	//@{
	//  ��������������ص����ݽṹ
	//! ��������ʱ�����ڻ������е���ʼλ�á�
	static int        m_iRecvPos;
	//! 
	static int        m_iProcPos;
	static BYTE*      m_pRecvBuf;
	//! �������ջ�����CS
	static CRITICAL_SECTION  m_sRecvBuf;
	static UINT_PTR   m_sSocket;
	static HANDLE     m_sWsaEvent;
	
	//! �˳���Ϣ�����̵߳���Ϣ
	static HANDLE     m_sThreadExit;

	//@} 

	//! ��ǰ��socketMgr�Ƿ��Ѿ���ʼ����
	bool       m_bInit;


	//@{
	//  ���������߳���صľ��
	//! ���������̵߳�handle.
	HANDLE     m_threadHandle;
	//! �̵߳�ID.
	DWORD       m_threadID;
	//@}

	//! ���������Ƿ��ѻ�ģʽ
	BOOL       m_bOffLine;

	//! ���������������Ѿ��ɹ�
	static BOOL m_bConnectSuccess;


	//@{ ����ͳ������
	//! ���յ�������ͳ��
	union{

		struct{
			DWORD	m_dwRecvDataLenLow;
			DWORD	m_dwRecvDataLenHight;
		};

		ULONGLONG	m_llRecvDataLen;
	};

	//! ���͵�����ͳ��
	union{

		struct{
			DWORD	m_dwSendDataLenLow;
			DWORD	m_dwSendDataLenHight;
		};

		ULONGLONG	m_llSendDataLen;
	};
	//@}

	//! �ϲ�����ж���̵߳��� sendOneMsg �������
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
	//! ����رպ�Ҫ���õĻص�����
	static 	sockClosed_callbackFunc  m_ptrClosedCallbackFunc;

	

public:
	osc_fdSocket();
	~osc_fdSocket();


	/** \brief
	 *  ���������Լ��ĳ�ʼ��������
	 */
	BOOL                WSAInitialize ( void );


	/** \brief
	 *  ���ӵ��������Ľӿڡ�
	 */
	virtual bool         connect_server( const char* _hostAddr,int _port,int _WSA,HWND _hwnd );
	/** \brief
	 *  �رյ�ǰ�����ӡ�
	 */
	virtual bool         close_socket( void );

	//@{
	/** \brief
	 *  ������Ϣ��
	 */
	static bool          receive( osc_fdSocket* _ptrSocket );

	/** \brief
	 *  ����Ϣ�������ж���һ����Ϣ��
	 */
	virtual char *        read_message( WORD& _msgSize,int* _errorCode,int* _errorType );
	//@}

	//��ȡ��Ϣ������������NULL��Ϊ��ȡ���
	char* read_buffer( WORD& _msgSize );

	//@{

	//! ������ص���Ϣ��
	virtual bool          add_message( const char* _pmsg,WORD _size );
	virtual bool          send_message( void );

	//! ���͵�����ָ���õĺ�����
	virtual bool          send_oneMsg( const char* _msg,WORD _size );
	//@}

	//! ����Ϊ�ѻ�ģʽ����ģʽ�£�send_oneMsg��Զ����true.
	virtual bool          set_offLineState( bool _offLine ){ m_bOffLine = _offLine; return true; }

	//! ע������رպ���õĿͻ��˻ص����� 
	virtual bool          register_socketCloseCallback( sockClosed_callbackFunc _func );


	virtual bool          is_socketClosed( void );

	//! �������ݵ�thread
	static int            recv_thread( void* _param );

	/*! ��÷������ݵ��ܳ���
	*	
	*	\param _dwDataLenHight			: DWORD���ȵĸ�λ
	*	\param _llDataLen				: �ܵĳ���
	*	\return DWORD					: DWORD���ȵĵ�λ
	*/
	virtual DWORD		  get_sendDataLen(DWORD* _dwDataLenHight = NULL)const
	{
		return ((_dwDataLenHight)?(*_dwDataLenHight=m_dwSendDataLenHight):((void)0)),m_dwSendDataLenLow;
	}

	/*! ��ý������ݵ��ܳ���
	*	
	*	\param _dwDataLenHight			: DWORD���ȵĸ�λ
	*	\param _llDataLen				: �ܵĳ���
	*	\return DWORD					: DWORD���ȵĵ�λ
	*/
	virtual DWORD		  get_recvDataLen(DWORD* _dwDataLenHight = NULL)const
	{
		return ((_dwDataLenHight)?(*_dwDataLenHight=m_dwRecvDataLenHight):((void)0)),m_dwRecvDataLenLow;
	}

	/*! \brief �����㷨��Key���ɷ��������ȷ��͹�����
	*
	*	\param _key						: key value
	*	\param _len						: key length;
	*/
	virtual void		  set_cryptKey(char* _key,int _len);

};
