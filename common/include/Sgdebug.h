//-----------------------------------------------------------------------------
/** \file
 * Filename:sgdebug.h
 *
 * Desc:    func and macro to help sg engine debug.
 *
 * His:     River created @ 9/20 2001
 */ 
//-----------------------------------------------------------------------------
#ifndef __SGDEBUG_INCLUDE__
#define __SGDEBUG_INCLUDE__


#include "stdio.h"	
#include "assert.h"
#include "iostream"
# include "crtdbg.h"
# include "unknwn.h"
# include "../common.h"

#include <windows.h>
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")



//��Ϊ����ԭ���ԭ��,����new overloadͷ�ļ���Ӧ�ó����л����,
//����Ӧ����Ӧ�ó������Լ�������new overload.
//#include "../include/memorymanager.h"
//
//River @August 2002
//��.cpp�ļ��а�������ļ�.
//


//
// Outstr_tolog()
// ==============
/**
 * ���������Ϣ��LOG�ļ�
 */
COMMON_API void Outstr_tolog( const char* fmt, ... );   

COMMON_API void OutBuf_tolog(char * _pbuf, int nLen);

/**
 * �˺������������Ϣ��VC������
 */
COMMON_API void sgDebugOutFunc( const char * msg, ... );

//! �������Ϣ�������һ����־�����ڻ�е���log
COMMON_API void sgDebugOutFunc_tag( const char* _tag,const char * msg, ... );

//! ֱ�����һ����Ϣ��������vsprintf ����
COMMON_API void sgDebugOutFunc_directly( const char * msg );

/**
 *  sgasser��һ��ʽҪҪInt 3�ж�������assert,�����������Int 3�ж�.
 */
COMMON_API bool CustomAssertFunction( bool, const char*, int,const char* );


//! generate the miniDumpfile...
COMMON_API void CreateMiniDump( LPEXCEPTION_POINTERS _lpExceptionInfo) ;


//! popup the bug commit dialog
COMMON_API int PopupBugCommitDlg(void);

/*! \brief enable the CUP inject the exception of read\write some 
*			memory according specific address
*
*	\param _bEnable				: the switch of exception
*	\param _dwAddress			: the memory specific address
*	\param _bWrite				: rise exception when write this address
*	\param _bRead				: rise exception when read this address
*/
COMMON_API void EnableCPUMemoryException(BOOL _bEnable,DWORD _dwAddress,BOOL _bWrite,BOOL _bRead = FALSE);

//! get some basic information of local machine
COMMON_API void GetWindowsVerDesc(size_t _maxLength,char* _desc);
COMMON_API void GetCpuInfoDesc(size_t _maxLength,char* _desc);
COMMON_API void GetMemSizeInfoDesc(size_t _maxLength,char* _desc);

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday

River: From QuakeIII
============
*/
COMMON_API const char*      va( char *format, ... );


//@{
/** \brief
 *  �˱�������Ϊ���Ժ󣬰����е�osDebugOut�������log�ļ�������ֻ�������ĺ���Stack.
 */
extern COMMON_API bool      g_bLogToFile;

/** \brief
 *  �˱���ȷ�����жϵ����򣬻���ֱ�������쳣����Stack����DebugView�м䡣
 *
 *  Ĭ������£��жϵ������С����Ҫ����������Ŷӣ������ֱ�������쳣��
 *
 *	This variable is presented by IsDebuggerPresent() Funciton
 */
//extern COMMON_API bool      g_bInterruptInProgram;

/** \brief
 *  �����������ǿ��osDebugOut�����log�ļ�������g_bInterruptInProgramΪ��Ϊ��
 *
 *  �˱���Ĭ��Ϊ�٣����Ҫǿ�����,�����ڿͻ�������Ϊ�棬�����ɿͻ�����int 3����
 *  miniDump����ʱ���ֹ��Ĺر�log�ļ���������sgdebug_end
 */
extern COMMON_API bool      g_bForceWriteLogFile;


//! �ڷ���osassert������,�����ջ��Ϣ
extern COMMON_API bool      g_bExportStackInfo;

//! force dump the file (include the Macro "osassert" and "osassertex")
extern COMMON_API bool		g_bForceDumpFile;

//! ���ʹ��_SGRELEASE������������ٶȺ���С���ڴ�����,���������κε��쳣�������.
# if 1
  # define _DEBUGRUNTIME 1
# else
  # define _SGRELEASE 1
# endif 

#define FUNCTION_PERF 0


extern COMMON_API char g_ErrorSummary[1024];
COMMON_API char* g_GetErrorSummary();


//! Final Release.
/*
# define _SGRELEASE 1
*/
//@}


/*! \brief crazy debug helper class....
*
*	<<<<<<<<<<<<<<<<��Ҫ˵��>>>>>>>>>>>>>>>>>>
*
*	�ڰ���common��Ĺ����е����������ļ���,C/C++ �������б������һ�� 
*	/EHa���� ����ֹ Compiler Warning (level 3) C4535
*	��������Ǹ�C����ʱ�� _set_se_translator �����õġ���ϸ�������� MSDN
*/
class CSE
{
public:
	//! exception pointers to generate 
	//! the dump file
	EXCEPTION_POINTERS	m_eptr;

	//! to record the stack..
	char				m_func[2048];

	//! to identify the kind of exception
	bool				m_bAssert ;

public: 

	/*! \brief to record the stack function
	*/
	CSE(const char* _funcStr)
	{
		m_bAssert = true;
		strcpy(m_func,_funcStr);
	
		ZeroMemory(&m_eptr,sizeof(EXCEPTION_POINTERS));
	}

	/*! \brief map SEH to the CE...
	*/
	static void MapSEToCE(void)
	{
		if(!::IsDebuggerPresent()){
			_set_se_translator(TranslateSEToCE);
		}
	}

	bool IsAssert(void)
	{
		return (bool)(m_bAssert);
	}

	void PushFunc(const char* _func)
	{
		strcat(m_func,_func);
		strcat(m_func,"\n\n");
	}

private:

	CSE(PEXCEPTION_POINTERS _eptr)
	{
		m_bAssert = false;

		if(!g_bForceDumpFile){
			sprintf(m_func,"Some SE will generate, the Exception Code is <%x> .Please Check the dump file .\n",_eptr->ExceptionRecord->ExceptionCode);
		}else{
			sprintf(m_func,"Force Generate dump file \n");
		}
		strcpy(g_ErrorSummary,va("Some SE will generate, \nthe Exception Code is <%x> .\nPlease Check the dump file .\n",_eptr->ExceptionRecord->ExceptionCode));

		memcpy(&m_eptr,_eptr,sizeof(EXCEPTION_POINTERS));
	}


	//! ....
	static void _cdecl TranslateSEToCE(UINT _uiEC,PEXCEPTION_POINTERS _pep)
	{
		throw CSE(_pep);
	}

};

// ���߳��쳣�Ĵ��������� 
// ����catch com_thread::thread_process osassert �쳣
typedef void (*ExceptionCatchHandler_multi_thread)(CSE& _se);
extern COMMON_API ExceptionCatchHandler_multi_thread      g_pExceptionFuncHandler_multi_thread ;

#  pragma warning( disable : 4800 )  
#  define ALWAYS_IGNORE		10000

# if ( _DEBUGRUNTIME )
	#  define osassert( exp ) \
	{\
		if( CustomAssertFunction( (bool)(exp), #exp,__LINE__, __FILE__ ) )  \
		{\
			if( !g_bExportStackInfo ) sgdebug_end(); \
			if( g_bForceDumpFile ) { ::MessageBoxA( NULL,"����ж�ǿ������Dump�ļ�","�ͻ��˳������",MB_OK ); *(char*)0 = 0;}\
			if( ::IsDebuggerPresent() ){ _asm { int 3 };}\
			else\
			{\
				sprintf( g_ErrorSummary,"<#$>\n�������:<%s : %d>\n",__FILE__ , __LINE__ );\
				Outstr_tolog( g_ErrorSummary );\
				throw CSE(g_ErrorSummary); \
			} \
		} \
	}

	#  define osassertex( exp,desc ) \
	{\
		if( (!(bool)(exp))&&CustomAssertFunction( (bool)(exp), desc,__LINE__, __FILE__ ) ) \
		{\
			if( !g_bExportStackInfo ) sgdebug_end();\
			if( g_bForceDumpFile ) { ::MessageBoxA( NULL,"����ж�ǿ������Dump�ļ�","�ͻ��˳������",MB_OK ); *(char*)0 = 0;}\
			if( ::IsDebuggerPresent() ){ _asm { int 3 };}\
			else\
			{\
				sprintf( g_ErrorSummary,"<#$>\n��������:< %s >\n�������:<%s : %d>\n",(desc),__FILE__ , __LINE__ );\
				Outstr_tolog( g_ErrorSummary );\
				throw CSE(g_ErrorSummary); \
			}\
		}\
	}

# elif ( _SGRELEASE )

	#  pragma warning( disable : 4002 )  //��osDebugOut�����ʵ��̫��
	#  define osassert( exp ) ((void)0)
	#  define osassertex( exp, desc ) ((void)0)

# else

#  error "No macro named  '_DEBUGRUNTIME' or '_SGRELEASE' found!";

# endif // end if define osassert() and osassertex()





//! help class to avoid the throw the exception without call the ::LeaveCriticalSection function
class CCriticalSec
{
	LPCRITICAL_SECTION		m_ptrCriticalSec;
	CCriticalSec(void);
public:

	void LeaveCriticalSec(void)
	{
		if(m_ptrCriticalSec){
			::LeaveCriticalSection(m_ptrCriticalSec);
			m_ptrCriticalSec = NULL;
		}
	}

	void AttachCriticalSec(LPCRITICAL_SECTION _sec)
	{
		LeaveCriticalSec();
		m_ptrCriticalSec = _sec;
		::EnterCriticalSection(_sec);
	}

	CCriticalSec(LPCRITICAL_SECTION _sec):m_ptrCriticalSec(_sec)
	{
		::EnterCriticalSection(m_ptrCriticalSec);
	}

	~CCriticalSec(void)
	{ 
		LeaveCriticalSec();
	}
};

//! to reset the event to avoid the throw the exception without call the ::ResetEvent









//
// FormatStr()
// ===========
//
/**
 *  ��ʽ���ַ����ĺ���������һ��string���ʵ����������������Ϳ��Ը�������ַ�
 *  �����������Ӹ�ʽ���Ĺ��ܣ�ͬʱ�����ڶ��������ʹ�ÿɱ������
 *
 *  For example :
 *
 *   void OutputInfo( const char * );
 *   ...
 *   OutputInfo( Formatstr("%s\n",errinfo).c_str() );
 *
 *  ʹ��std::string������ֱ�ӷ���char*�ǳ��ڶ��̰߳�ȫ�ԵĿ���
 *
 *  TODO : �����ɱ�����ĺ���( ��Outstr_tolog()�� )����Ϊ foo( FormatStr(...) ) 
 *        ����ʽ��
 */
# include <string>
COMMON_API std::string FormatStr( const char * fmt, ... );


/** \brief
 *  ��򵥵����������Ϣ�ĺ���.
 */
COMMON_API void        error_msg( const char* fmt, ... );



//
// sgDebugOut
// ==========
//
// ������Ϣ��������
//

//!	�����Ϣ��������
# if _DEBUGRUNTIME
	#  define osDebugOut sgDebugOutFunc
	#  define osDebugOut_tag sgDebugOutFunc_tag
	#  define osDebugOut_directly sgDebugOutFunc_directly
# else
    # define osDebugOut(exp) ((void)0)
	#  define osDebugOut_tag(exp) ((void)0)
	#  define osDebugOut_directly(exp) ((void)0)
# endif 


// Guard macros
//
//  CRT��Խ���⺯�������ڳ������ѭ����ʹ��    
# if ( !_SGRELEASE )
#	if FUNCTION_PERF

#		define guard   { CSE::MapSEToCE(); try { DWORD __function_perf_index__ = ::GetTickCount();

#		define unguard  if(::GetAsyncKeyState(VK_F1) & 0x8000){\
							__function_perf_index__ = ::GetTickCount() - __function_perf_index__;\
							if(__function_perf_index__){osDebugOut(__FUNCTION__##" time<%d>",__function_perf_index__);}\
						}\
					}catch(CSE& _se)\
					{\
						_se.PushFunc(__FUNCTION__);\
						throw _se;\
					}\
				}
#	else

#		define guard   { CSE::MapSEToCE(); try {

#		define unguard  }\
			catch(CSE& _se)\
			{\
				_se.PushFunc(__FUNCTION__);\
				throw _se;\
			}\
		}
#		endif //FUNCTION_PERF

# else

#  define guard ((void)0)
#  define unguard    ((void)0)

# endif //( !_SGRELEASE )


#define  ERRORFILE  "crierror.txt" 
/** 
 * ��ͼ�����ĵ��Ժ���������ʹ�á�ʹ�������������harddisk������
 * һ��crierror.txt���ļ�.
 */
COMMON_API void critical_error( const char* str, ... );


/** \brief
 *  ���ڶ��ڴ��assert,�˺�ֻ��debug�汾�������á�
 *
 *  \param _address �ڴ����ʼ��ַ��
 *  \param _size    Ҫassert�ڴ�Ĵ�С��
 *  \param _access  ���Ϊ1��������ڴ��Ƿ�ɶ���д�����Ϊ0,��ֻ���ɶ���
 */
# ifdef _DEBUG
# define assertMem( _address,_size,_access ) \
	     osassertex( _CrtIsValidPointer( _address,_size,_access ),"Memory Not Valid" ); 
# endif 

//! ��Release�汾�£�DoNothing.
# ifdef NDEBUG

# define assertMem( x,y,z )    ;

# endif 



//
// sgdebug_start(), sgdebug_end()
// ===============================
/**
 * ��������ϵͳ,����Ӧ�ð�����������뵽winmain������ȥ.
 *
 * @param logfilename ���debug��Ϣ��*.txt�ļ�������.
 * @see sgdebug_end()
 *
 */ 
COMMON_API bool sgdebug_start( const char* logfilename );

/** \brief
 * ��ֹ����ϵͳ,����Ӧ�ð�����������뵽winmain������ȥ.
 *
 * @see sgdebug_start()
 * \param bool _errorEnd ����Ƿ�������ֹ����(��osassert��ֹ����)��
                         ���ֵ��Ϊtrue.
 */ 
COMMON_API void sgdebug_end( const char* _str = NULL,bool _errorEnd = false );



//! �Ƿ�򿪸���ϸ�����ܲ���
# define DETAIL_PROFILE 0
# define MAX_PROFILE_FRMAE  2048

//---------------------------------------------------------------------------------------------------------
/**
 *
 *  Profiling used func,for test the function performance.
 *
 */
//---------------------------------------------------------------------------------------------------------
struct COMMON_API  sg_funcprofiledata
{
	//! �������֡�
	char      funcname[256];       
	//! ���ô�����
	int       num;
	//! �ܹ�ʹ��ʱ�䡣
	float     totalsec;            
	//! ���ִ��ʱ�䡣
	float     min;    
	//! �ִ��ʱ�䡣 
	float     max;                 

	float     lasttime;
	
	//! ����ִ��profile?
	bool      inprofile;          

	//! ÿһ��func profile ��һ����ʼʱ��.
	LARGE_INTEGER    m_lStart;
	
# if DETAIL_PROFILE
	float     m_fFrameTime[MAX_PROFILE_FRMAE];
# endif 

public:
	sg_funcprofiledata( void );
	
	/** ����profile����.
	 */
	float     end_profilefunc( const char* _funcname );
	/** ��ʼprofile����.
	 */
	void      start_profilefunc( void );
	
};

#define       MAX_PROFILENUM  32



class COMMON_API sg_profile
{
private:
	//������������ܷ�����
	int                    profuncnum;                  //���Ժ�������Ŀ��
	float                  totalframetime;              //�ܹ�ʹ�õ�ʱ�䡣
	float                  minframetime;                    
	float                  maxframetime;
	float                  lastframetime;
	int                    framenum;
	
	//for
	int                    start;

	//! ÿһ֡ profile ��һ����ʼʱ��.
	LARGE_INTEGER          m_lStart;

protected:
	void      profile_analyze_out( int index );
	
private:
	sg_funcprofiledata     prodata[MAX_PROFILENUM];     //profile �����ݡ�
	
# if DETAIL_PROFILE
	float                  m_fFrameTime[MAX_PROFILE_FRMAE];
# endif 

public:
	sg_profile(void);
	~sg_profile( void ){};
	
	/** ��������������Ҫ�⺯���Ŀ�ʼ�ͽ�����
	 */
	void      profile_begin( char* funcname );
	void      profile_end( char* funcname );
	
	/** �������������ڳ�����ѭ���Ŀ�ʼ�ͽ�����
	 */
	void      frame_movestart(void);
	void      frame_moveend( void );
	
	/** �ڳ���������profile�ķ����������
	 */
	void      dump_profile( char* filename );
	
	/** �����Ĭ�ϵ��ļ�������exception��log�ļ���
	 */
	void      dump_profile( void );                     
	
	
	//
	double    get_firstprofileper( void );
	
};

extern COMMON_API sg_profile*   debug_profile;




//�˴���OR�ر�profile.
#define PROFILEENABLE 1      

#if PROFILEENABLE 
	/** 
	*  profile�����˷�ʱ��,��win98�±���win2000���˷ѵ�ʱ��Ҫ��ܶ�.
	*  ��ʼʹ��Profile�ĳ�,д��Ҫ���ԵĴ���֮ǰ.
	*/
	#define PSTART(func)   { debug_profile->profile_begin(TEXT(#func)); }
	/**
	* ����ʹ��profile.д��Ҫ���ԵĴ���֮��.
	*/
	#define PEND(func)     { debug_profile->profile_end(TEXT(#func)); }

	/**
	* ��ÿһ֡�Ŀ�ʼ���������,���������Ļ�,profile��ֵ��һЩ�����ֵ.
	*/
	#define PROFILEMOVESTART     { debug_profile->frame_movestart(); }
	/**
	* ��ÿһ֡�Ľ������������.
	*/
	#define PROFILEMOVEEND       { debug_profile->frame_moveend(); }
#else

	#define PSTART(func)
	#define PEND(func)  
	#define PROFILEMOVESTART  
	#define PROFILEMOVEEND    

#endif


//@{
//! ����򿪺͹ر��ļ��ĺ�,ʹ��֤��һ���ļ���,��ͬһ�������п��Թرյ�����ļ�.

//@}


////////////////////////////////////////////////////////////////////////////////////////////
//
// ʹ�������Լ���randϵͳ��
extern void       init_osRandSystem( void );
extern void       end_osRandSystem( void );
COMMON_API int        osRand( void );

//! ����ʹ��ʹ��RAND����ʹ�������Լ���Rand������
# define    OSRAND      osRand()


//
// SAFE_DELETE(), SAFE_DELETE_ARRAY(), SAFE_FREE(), SAFE_RELEASE()
// ===============================================================
//
# define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
# define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
# define SAFE_FREE(p)         { if(p) { free(p);        (p)=NULL; } }
# ifdef SAFE_RELEASE
	#  undef SAFE_RELEASE
# endif
#  define SAFE_RELEASE(p)     { if(p) { (p)->Release(); (p)=NULL; } }

inline int GetRefCount(LPUNKNOWN pCom)
{
	int Result = -1;
	if (pCom)
	{
		pCom->AddRef();
		Result = (int)pCom->Release();
	}
	return Result;
}

#endif