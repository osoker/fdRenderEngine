//------------------------------------------------------------------------------------------------
/** \file
 *  Filename:inputmanager.h
 *
 *  Desc:    dinput manager,manage keyboard and mouse input
 *  
 *  His:     river created @ 4/18 2003
 *         
 *  "英明的专制胜过腐朽的民主".  
 *
 */
//------------------------------------------------------------------------------------------------
# pragma once

#include "..\interface\osInterface.h"
#include <dinput.h>


/** \brief
 *  MouseInput Manager.
 *
 *  封装Dinput接口,得到Mouse产生的信息.
 * 
 */
class osc_mousemanager
{
private:
	LPDIRECTINPUTDEVICE8   demouse;
	HWND                   hwnd;
	
public:
	osc_mousemanager();
	~osc_mousemanager();
	
	bool                   init_mouse( LPDIRECTINPUT8 _lpdi,HWND _hWnd );
	void                   get_input( int _size,LPVOID _pvdata );      //从mouse设备上得到输入数据。             
	
	void                   release_mouse( void );
	bool                   acquire( void );
	bool                   unacquire( void );
	
};


/** \brief
 *  Keyboard input Manager.
 *
 *  封装Keyboard接口,得到keyboard产生的信息.
 *  因为使用键盘太灵敏，所以可能使用buffered keyboard input.
 *  或者是根据时间来决定移动等因素。
 */
class osc_keymanager
{
private:
	LPDIRECTINPUTDEVICE8   dekeyboard;
	HWND                   hwnd;
	
public:
	osc_keymanager();
	~osc_keymanager();
	
	bool                   init_keyboard( LPDIRECTINPUT8 _lpdi,HWND _hWnd );
	void                   get_input( int _size,LPVOID _pvdata );
	void                   releaes_keyboard( void );
	bool                   acquire( void );
	bool                   unacquire( void );
};



/*
	应该放在camera类库里进行camera控制。
	重力系统应该和camera做在一起吗？如果做在一起的话，会增加
	camera系统的工作量。但应该会使用整个程序架构更加清析。

	重力系统由场景碰撞系统来创建,因为场景中所有的Entity都要重力,
	而不光是Camera.
*/
//------------------------------------------------------------------------------------------------
/** \brief
 *  Mouse&Keyboard  Class manager and all other input device.
 *  
 *  
 *  这个类目前封装了Mouse&Keyboard 这两个输入设备,得到这两个输入设备的输入数据,
 *  如果需要更多的输入设备,比如摇杆输入或是其它输入,需要在这个类中加入这个输入
 *  设备的管理器.
 *  
 */
//------------------------------------------------------------------------------------------------
class  osc_inputmanager : public I_inputMgr
{
private:
	LPDIRECTINPUT8      lpdi;
	HWND                hwnd;
	HINSTANCE           hinst;
	

	//@{
	/**
	 *  这些Manager向总的Input Manager负责,可能还会加入更多的InputDevice Manager.
	 */
	osc_mousemanager     mouse;
	osc_keymanager       keyboard;
	//@}
	

	//! 数据区包含了mouse&keyboard数据.
	os_dInputdata       inputdata;


protected:
	/**
	 * 从input device上得到数据。类似于framemove函数。
	 */
	void                get_input( void );                           

public:
	osc_inputmanager();
	~osc_inputmanager();



	//@{
	/**
	 *  初始化和释放Input Manager.
	 */
	virtual bool               init_inputmgr( HINSTANCE _hInst,HWND _hWnd );
	virtual bool               release_inputmgr( void );
    //@}

	/**
	 *  Get inupt data from dinput device.
	 */
	virtual os_dInputdata*     get_inputdata( void );


	//@{
	/**
	 *  使输入设备处于可以接受数据或是不接受数据的状态.
	 */
	virtual bool               acquire( void );
	virtual bool               unacquire( void );
	//@}


};

// 全局变量,控制是否创建Exclusive的Mouse.
extern int       g_iExclusiveMouse;

