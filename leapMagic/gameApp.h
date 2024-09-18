///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gameApp.h
 *
 *  Desc:     魔法demo客户端最上层的类,负责引擎接口的创建,游戏消息的处理.
 *            场景的渲染等上层的工作.
 * 
 *  His:      River created @ 2006-4-13
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "stdafx.h"
# include "gc_gameScene.h"


class gc_testScene;


//! 最多输出5条调试信息
# define MAX_DEBUGINFO    5

//! 记录客户端的标准消息
struct gc_clientMessage
{
	HWND    m_hWnd;
	UINT    m_sUmsg;
	WPARAM  m_wParam;
	LPARAM  m_lParam;
};
typedef std::vector<gc_clientMessage>  VEC_clientMsg;


/** \brief
 *  魔法demo的基础类，用于整体控制
 *
 */
class gc_gameApp
{
private:
	//! 窗口和实例相关
	HWND         m_hWnd;
	HINSTANCE     m_hIns;

	//! 测试场景的实例
	gc_testScene*  m_ptrTestScene;

	//@{
	//! 要输出的调试信息
	static os_stringDisp      m_arrDebugInfo[MAX_DEBUGINFO];
	static int                m_iInfoNum;
	//@}
	//@{
	//! 场景内画一些示例的DebugBox
	static os_bbox            m_arrDebugBox[MAX_DEBUGINFO];
	static int                m_iDebugBoxNum;
	//@}

	//! 上一帧是否弹出了console窗口
	BOOL                      m_bConsoleLastFrame;

	//! 是否处于激活状态
	BOOL                      m_bActive;

	//! 当前的CursorId.
	int                       m_iCommonCursorId;
	//! 战斗用CursorId.
	int                       m_iAttackCursorId;


private:

	//! 处理全局命令队列内的命令
	void           process_cmdQueue( void );


public:
	//! 设备初始化
    static os_deviceinit  m_sDevInit;

	//! 3d设备的接口指针。
	static I_deviceManager*   m_3dMgr;

	//! Dinput的设置状态
	static I_inputMgr*        m_ptrInputDevice;

	//! gameApp的场景操作指针
	static gc_gameScene*      m_ptrActiveScene;


	//! gameApp场景的声音管理器指针
	static I_soundManager*    m_ptrSndInterface;

	//! 只有在render Loop结束时，才能真正的释放场景内的3d设备。
	static HANDLE             m_sRenderLoopFinish;

private:
	//! 处理键盘按下的消息
	void        process_keyDownMsg( UINT _uMsg, WPARAM _wParam, LPARAM _lParam );

	//! 处理鼠标相关的消息
	void        process_mouseMsg( UINT _uMsg, WPARAM _wParam, LPARAM _lParam );

	//! 在屏幕上渲染输出一些调试信息
	void        render_debugInfo( void );

	//! 在场景内渲染我们加入的bbox盒子，用于一个调试信息的示例
	void        render_debugBox( void );


	//! 处理手工的相机控制
	void        process_customCamControl( const os_dInputdata* _idata );

	//! 处理鼠标控制相机的旋转
	void        process_camRotPitch( const os_dInputdata* _idata );


	//! 处理客户端的命令队列.
	bool        process_msgQueue( void );

public:
	gc_gameApp();
	~gc_gameApp();

	//! 使用这个函数修改玩家的屏幕分辩率。
	static bool change_deskScreenSize();
	//! 重设窗口的大小和样式
	static void resetWindowSizeNStyle( HWND _wnd );

	/** \brief
	 *  对所有的窗口消息进行处理.
	 */
	LRESULT     msg_proc( HWND _hWnd,UINT _uMsg, WPARAM _wParam, LPARAM _lParam );


	//! 初始化当前的gameApp，在这个函数内初始化了3d设备
	bool        init_gameApp( HWND _hwnd,HINSTANCE _hins );

	
	/** \brief
	 *  对当前的app进行渲染的函数.
	 */
	bool        render_gameApp( void );

	//! 推进要渲染的DebugInfo.
	static void push_debugInfo( const char* _info,int _x,int _y,DWORD _color );

	//! 推进要渲染的DebugBBox
	static void push_debugBox( os_bbox& _bbox );

	//! 立即执行一条命令，
	void           do_commandImm( lm_command* _cmd );

	//! 重设光标信息
	void           reset_cursor( const char* _cursorname );

	//! 重设窗口的大小
	void           reset_screen( void );



};

//! 全局的渲染变量。
extern gc_gameApp*  g_ptrGameApp;
