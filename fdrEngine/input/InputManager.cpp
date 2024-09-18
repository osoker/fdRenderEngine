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
#include "stdafx.h"
#include "InputManager.h"

# pragma comment( lib,"dinput8" )

/** \brief
 *  控制是否创建Exclusive mouse的变量.
 */
int       g_iExclusiveMouse = 0;


//------------------------------------------------------------------------------------------------
//mouse releated function.
osc_mousemanager::osc_mousemanager()
{
	this->demouse = NULL;
	this->hwnd = NULL;
}

osc_mousemanager::~osc_mousemanager()
{
	
}


//从mouse设备上得到输入数据。             
void osc_mousemanager::get_input( int _size,LPVOID _pvdata )          
{
	guard;
	
	HRESULT    hr;

	try
	{
		hr = demouse->GetDeviceState( _size,_pvdata );
		if( FAILED( hr ) )
		{
			hr = demouse->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = demouse->Acquire();
		}
	}catch (CSE& e){
		osDebugOut( "mouseInput异常<%s>...\n",e.m_func );
		return;
	}

	// ATTENTION TO FIX:
	// 此处应该是 E_ACCESSDENIED 错误，不需要处理???
	//osassertex( !FAILED( hr ),DXGetErrorDescription9(hr ) );	
	
	unguard;
}

bool osc_mousemanager::init_mouse( LPDIRECTINPUT8 _lpdi,HWND _hWnd )
{
	guard;
	
	HRESULT    hr;
	
	osassert( _lpdi );
	
	hwnd = _hWnd;
	hr = _lpdi->CreateDevice( GUID_SysMouse, &this->demouse, NULL);
	if( FAILED( hr ) )
	{
		osDebugOut( "Create mouse device failed" );
		osassert( hr );
		return FALSE;
	}
	
	hr = demouse->SetDataFormat( &c_dfDIMouse2);
	if( FAILED( hr ) )
	{
		osassert( FALSE );
		return FALSE;
	}

	if( g_iExclusiveMouse )
	{
		hr = demouse->SetCooperativeLevel( hwnd,
		       DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}
	else
	{
		
		hr = demouse->SetCooperativeLevel( hwnd,
		       DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		if( FAILED( hr ) )
		{
			/*
			// River @ 2011-3-23:多次尝试，看最后是否会成功
			int  t_time = 0;
			while( t_time < 10 )
			{
				t_time ++;
				::Sleep( 1 );
				hr = demouse->SetCooperativeLevel( hwnd,
					DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
				if( !FAILED( hr ) )
					break;
			}
			*/
			osDebugOut( "设置失败，继续....\n" );
		}
	}

	// 
	/*
	if( FAILED( hr ) )
	{
		osassertex( FALSE,osn_mathFunc::get_errorStr( hr ) );
		return FALSE;
	}
	*/
	
    // Acquire the newly created device
    hr = demouse->Acquire();
	if( hr == DIERR_INPUTLOST )
		hr = demouse->Acquire();
	
	
	return TRUE;
	
	unguard;
}


bool osc_mousemanager::acquire( void )
{
	guard;
	
	HRESULT   hr;
	
	hr = demouse->Acquire();
	if( hr == DIERR_INPUTLOST )
		hr = demouse->Acquire();

	return TRUE;
	
	unguard;
}


bool osc_mousemanager::unacquire( void )
{
	guard;
	
	HRESULT   hr;
	
	hr = demouse->Unacquire();
	if( hr == DIERR_INPUTLOST )
		hr = demouse->Unacquire();
	if( FAILED( hr ) )
	{
		osDebugOut( DXGetErrorDescription9( hr ) );
		osassert( FALSE );
		return FALSE;
	}
	return TRUE;
	
	unguard;
}


void osc_mousemanager::release_mouse( void )
{
	guard;// osc_mousemanager::release_mouse() );
	
	if( this->demouse )
	{
		demouse->Unacquire(); 
		SAFE_RELEASE( demouse );
	}
	
	unguard;
}

//------------------------------------------------------------------------------------------------
//keyboard releated function.
osc_keymanager::osc_keymanager()
{
	this->dekeyboard = NULL;
	this->hwnd = NULL;
	
}

osc_keymanager::~osc_keymanager()
{
	
}


void osc_keymanager::get_input( int _size,LPVOID _pvdata )
{
	guard;
	

	try {
		HRESULT    hr;
		memset( _pvdata,0,_size );
		hr = dekeyboard->GetDeviceState( _size,_pvdata );
		if( FAILED( hr ) )
		{
			hr = dekeyboard->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = dekeyboard->Acquire();
		}
	}catch (CSE& e){
		osDebugOut( "keyboardInput异常<%s>...\n",e.m_func );
		return;
	}

	//osassertex( !FAILED( hr ),DXGetErrorDescription9(hr ) );		
	
	unguard;
}

bool osc_keymanager::init_keyboard( LPDIRECTINPUT8 _lpdi,HWND _hWnd )
{
	guard;// osc_keymanager::init_keyboard() );
	
	HRESULT     hr;
	
	osassert( _lpdi );
	
	hwnd = _hWnd;
	hr = _lpdi->CreateDevice(GUID_SysKeyboard, &this->dekeyboard, NULL); 
	if( FAILED( hr ) )
	{
		osDebugOut( "Create keyboard device failed" );
		osassert( FALSE );
		return FALSE;
	}
	
	hr = dekeyboard->SetDataFormat(&c_dfDIKeyboard); 
	if( FAILED( hr ) )
	{
		osDebugOut( "Set data format failed" );
		osassert( FALSE );
		return FALSE;
	}
	
	hr = dekeyboard->SetCooperativeLevel( hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
	if( FAILED( hr ) )
	{
		osassert( FALSE );
		return FALSE;
	}
	
	hr = dekeyboard->Acquire();
	if( hr == DIERR_INPUTLOST )
	{
		hr = dekeyboard->Acquire();
	}
		
	return TRUE;
	
	unguard;
}

bool osc_keymanager::acquire( void )
{
	guard;// osc_keymanager::acquire() );
	
	HRESULT    hr;
	hr = dekeyboard->Acquire();
	if( hr == DIERR_INPUTLOST )
	{
		hr = dekeyboard->Acquire();
	}
		
	return TRUE;
	
	unguard;
}



bool osc_keymanager::unacquire( void )
{
	guard;// osc_keymanager::unacquire() );
	
	HRESULT    hr;
	hr = dekeyboard->Unacquire();
	if( hr == DIERR_INPUTLOST )
	{
		hr = dekeyboard->Unacquire();
	}
	
	if( FAILED( hr ) )
	{
		osDebugOut( DXGetErrorDescription9( hr ) );
		osassert( FALSE );
		return FALSE;
	}
	
	return TRUE;
	
	unguard;
}


void osc_keymanager::releaes_keyboard( void )
{
	guard;// osc_keymanager::releaes_keyboard() );
	
	if( dekeyboard )
	{
		dekeyboard->Unacquire();
		SAFE_RELEASE( dekeyboard );
	}
	
	unguard;
}


//------------------------------------------------------------------------------------------------
//input manager class.
osc_inputmanager::osc_inputmanager()
{
	this->lpdi = NULL;
	this->hwnd = NULL;
	this->hinst = NULL;
	
	//set input data chunk as zero.
	memset( &inputdata,0,sizeof( os_dInputdata ) );

}

osc_inputmanager::~osc_inputmanager()
{
	guard;// osc_inputmanager::~osc_inputmanager() );
	
	this->release_inputmgr();
	
	unguard;
}


//从input device上得到数据。
void osc_inputmanager::get_input( void )                           
{
	guard;
	
	memset( &inputdata,0,sizeof( os_dInputdata ) );
	keyboard.get_input( sizeof( char )*256,(LPVOID)inputdata.keydata );	
	mouse.get_input( sizeof( DIMOUSESTATE2 ),(void*)&inputdata.mousedata );
	
	unguard;
}

bool osc_inputmanager::init_inputmgr( HINSTANCE _hInst,HWND _hWnd )
{
	guard;// osc_inputmanager::init_input() );
	
	HRESULT    hr;
	bool       tmpb;
	
	osassert( _hInst );
	osassert( _hWnd  );
	
	hinst = _hInst;
	hwnd = _hWnd;
	
	hr = DirectInput8Create( hinst, DIRECTINPUT_VERSION, 
		IID_IDirectInput8, (void**)&lpdi, NULL); 
	if( FAILED(hr) )
	{ 
		osDebugOut( "Create direct input failed" );
		osassert( false );
		return false;
	} 
	
	tmpb = this->mouse.init_mouse( lpdi,hwnd );
	if( !tmpb )
	{
		release_inputmgr();
		return tmpb;
	}
	
	tmpb = this->keyboard.init_keyboard( lpdi,hwnd );
	if( !tmpb )
	{
		release_inputmgr();
		return tmpb;
	}
	
	return true;
	
	unguard;
}



bool osc_inputmanager::release_inputmgr( void )
{
	guard;// osc_inputmanager::release_input() );
	
	this->mouse.release_mouse();
	this->keyboard.releaes_keyboard();
	
	if( this->lpdi )
	{
		SAFE_RELEASE( lpdi );
	}

	return true;
	
	unguard;
}


//
//get inupt data from dinput device.
//
os_dInputdata* osc_inputmanager::get_inputdata( void )
{
	guard;// osc_inputmanager::get_inputdata());

	this->get_input();
	return &inputdata;

	unguard;
}



bool osc_inputmanager::acquire( void )
{
	guard;// osc_inputmanager::acquire() );
	
	bool    tmpb;
	
	tmpb = keyboard.acquire();
	if( !tmpb )
		return tmpb;
	
	tmpb = mouse.acquire();
	
	return tmpb;
	
	unguard;
}

bool osc_inputmanager::unacquire( void )
{
	guard;// osc_inputmanager::unacquire() );

	bool    tmpb;
	
	tmpb = keyboard.unacquire();
	if( !tmpb )
		return tmpb;
	
	tmpb = mouse.unacquire();
	
	return tmpb;
	
	unguard;
}