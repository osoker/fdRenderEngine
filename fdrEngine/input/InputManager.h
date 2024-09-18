//------------------------------------------------------------------------------------------------
/** \file
 *  Filename:inputmanager.h
 *
 *  Desc:    dinput manager,manage keyboard and mouse input
 *  
 *  His:     river created @ 4/18 2003
 *         
 *  "Ӣ����ר��ʤ�����������".  
 *
 */
//------------------------------------------------------------------------------------------------
# pragma once

#include "..\interface\osInterface.h"
#include <dinput.h>


/** \brief
 *  MouseInput Manager.
 *
 *  ��װDinput�ӿ�,�õ�Mouse��������Ϣ.
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
	void                   get_input( int _size,LPVOID _pvdata );      //��mouse�豸�ϵõ��������ݡ�             
	
	void                   release_mouse( void );
	bool                   acquire( void );
	bool                   unacquire( void );
	
};


/** \brief
 *  Keyboard input Manager.
 *
 *  ��װKeyboard�ӿ�,�õ�keyboard��������Ϣ.
 *  ��Ϊʹ�ü���̫���������Կ���ʹ��buffered keyboard input.
 *  �����Ǹ���ʱ���������ƶ������ء�
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
	Ӧ�÷���camera��������camera���ơ�
	����ϵͳӦ�ú�camera����һ�����������һ��Ļ���������
	cameraϵͳ�Ĺ���������Ӧ�û�ʹ����������ܹ�����������

	����ϵͳ�ɳ�����ײϵͳ������,��Ϊ���������е�Entity��Ҫ����,
	��������Camera.
*/
//------------------------------------------------------------------------------------------------
/** \brief
 *  Mouse&Keyboard  Class manager and all other input device.
 *  
 *  
 *  �����Ŀǰ��װ��Mouse&Keyboard �����������豸,�õ������������豸����������,
 *  �����Ҫ����������豸,����ҡ�����������������,��Ҫ��������м����������
 *  �豸�Ĺ�����.
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
	 *  ��ЩManager���ܵ�Input Manager����,���ܻ����������InputDevice Manager.
	 */
	osc_mousemanager     mouse;
	osc_keymanager       keyboard;
	//@}
	

	//! ������������mouse&keyboard����.
	os_dInputdata       inputdata;


protected:
	/**
	 * ��input device�ϵõ����ݡ�������framemove������
	 */
	void                get_input( void );                           

public:
	osc_inputmanager();
	~osc_inputmanager();



	//@{
	/**
	 *  ��ʼ�����ͷ�Input Manager.
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
	 *  ʹ�����豸���ڿ��Խ������ݻ��ǲ��������ݵ�״̬.
	 */
	virtual bool               acquire( void );
	virtual bool               unacquire( void );
	//@}


};

// ȫ�ֱ���,�����Ƿ񴴽�Exclusive��Mouse.
extern int       g_iExclusiveMouse;

