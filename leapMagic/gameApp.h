///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gameApp.h
 *
 *  Desc:     ħ��demo�ͻ������ϲ����,��������ӿڵĴ���,��Ϸ��Ϣ�Ĵ���.
 *            ��������Ⱦ���ϲ�Ĺ���.
 * 
 *  His:      River created @ 2006-4-13
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "stdafx.h"
# include "gc_gameScene.h"


class gc_testScene;


//! ������5��������Ϣ
# define MAX_DEBUGINFO    5

//! ��¼�ͻ��˵ı�׼��Ϣ
struct gc_clientMessage
{
	HWND    m_hWnd;
	UINT    m_sUmsg;
	WPARAM  m_wParam;
	LPARAM  m_lParam;
};
typedef std::vector<gc_clientMessage>  VEC_clientMsg;


/** \brief
 *  ħ��demo�Ļ����࣬�����������
 *
 */
class gc_gameApp
{
private:
	//! ���ں�ʵ�����
	HWND         m_hWnd;
	HINSTANCE     m_hIns;

	//! ���Գ�����ʵ��
	gc_testScene*  m_ptrTestScene;

	//@{
	//! Ҫ����ĵ�����Ϣ
	static os_stringDisp      m_arrDebugInfo[MAX_DEBUGINFO];
	static int                m_iInfoNum;
	//@}
	//@{
	//! �����ڻ�һЩʾ����DebugBox
	static os_bbox            m_arrDebugBox[MAX_DEBUGINFO];
	static int                m_iDebugBoxNum;
	//@}

	//! ��һ֡�Ƿ񵯳���console����
	BOOL                      m_bConsoleLastFrame;

	//! �Ƿ��ڼ���״̬
	BOOL                      m_bActive;

	//! ��ǰ��CursorId.
	int                       m_iCommonCursorId;
	//! ս����CursorId.
	int                       m_iAttackCursorId;


private:

	//! ����ȫ����������ڵ�����
	void           process_cmdQueue( void );


public:
	//! �豸��ʼ��
    static os_deviceinit  m_sDevInit;

	//! 3d�豸�Ľӿ�ָ�롣
	static I_deviceManager*   m_3dMgr;

	//! Dinput������״̬
	static I_inputMgr*        m_ptrInputDevice;

	//! gameApp�ĳ�������ָ��
	static gc_gameScene*      m_ptrActiveScene;


	//! gameApp����������������ָ��
	static I_soundManager*    m_ptrSndInterface;

	//! ֻ����render Loop����ʱ�������������ͷų����ڵ�3d�豸��
	static HANDLE             m_sRenderLoopFinish;

private:
	//! ������̰��µ���Ϣ
	void        process_keyDownMsg( UINT _uMsg, WPARAM _wParam, LPARAM _lParam );

	//! ���������ص���Ϣ
	void        process_mouseMsg( UINT _uMsg, WPARAM _wParam, LPARAM _lParam );

	//! ����Ļ����Ⱦ���һЩ������Ϣ
	void        render_debugInfo( void );

	//! �ڳ�������Ⱦ���Ǽ����bbox���ӣ�����һ��������Ϣ��ʾ��
	void        render_debugBox( void );


	//! �����ֹ����������
	void        process_customCamControl( const os_dInputdata* _idata );

	//! �����������������ת
	void        process_camRotPitch( const os_dInputdata* _idata );


	//! ����ͻ��˵��������.
	bool        process_msgQueue( void );

public:
	gc_gameApp();
	~gc_gameApp();

	//! ʹ����������޸���ҵ���Ļ�ֱ��ʡ�
	static bool change_deskScreenSize();
	//! ���贰�ڵĴ�С����ʽ
	static void resetWindowSizeNStyle( HWND _wnd );

	/** \brief
	 *  �����еĴ�����Ϣ���д���.
	 */
	LRESULT     msg_proc( HWND _hWnd,UINT _uMsg, WPARAM _wParam, LPARAM _lParam );


	//! ��ʼ����ǰ��gameApp������������ڳ�ʼ����3d�豸
	bool        init_gameApp( HWND _hwnd,HINSTANCE _hins );

	
	/** \brief
	 *  �Ե�ǰ��app������Ⱦ�ĺ���.
	 */
	bool        render_gameApp( void );

	//! �ƽ�Ҫ��Ⱦ��DebugInfo.
	static void push_debugInfo( const char* _info,int _x,int _y,DWORD _color );

	//! �ƽ�Ҫ��Ⱦ��DebugBBox
	static void push_debugBox( os_bbox& _bbox );

	//! ����ִ��һ�����
	void           do_commandImm( lm_command* _cmd );

	//! ��������Ϣ
	void           reset_cursor( const char* _cursorname );

	//! ���贰�ڵĴ�С
	void           reset_screen( void );



};

//! ȫ�ֵ���Ⱦ������
extern gc_gameApp*  g_ptrGameApp;
