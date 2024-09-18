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



//因为不明原因的原因,包含new overload头文件在应该程序中会出错,
//所以应该在应该程序中自己来加入new overload.
//#include "../include/memorymanager.h"
//
//River @August 2002
//在.cpp文件中包含这个文件.
//


//
// Outstr_tolog()
// ==============
/**
 * 输出调试信息到LOG文件
 */
COMMON_API void Outstr_tolog( const char* fmt, ... );   

COMMON_API void OutBuf_tolog(char * _pbuf, int nLen);

/**
 * 此函数输出调试信息到VC调试栏
 */
COMMON_API void sgDebugOutFunc( const char * msg, ... );

//! 在输出消息里面加上一个标志，用于机械这个log
COMMON_API void sgDebugOutFunc_tag( const char* _tag,const char * msg, ... );

//! 直接输出一条信息，不进行vsprintf 解析
COMMON_API void sgDebugOutFunc_directly( const char * msg );

/**
 *  sgasser有一方式要要Int 3中断来产生assert,这个函数用于Int 3中断.
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
 *  此变量设置为真以后，把所有的osDebugOut都输出到log文件。否则，只输出出错的函数Stack.
 */
extern COMMON_API bool      g_bLogToFile;

/** \brief
 *  此变量确认是中断到程序，还是直接引发异常，把Stack导出DebugView中间。
 *
 *  默认情况下，中断到程序中。如果要编译给测试团队，则可以直接引发异常。
 *
 *	This variable is presented by IsDebuggerPresent() Funciton
 */
//extern COMMON_API bool      g_bInterruptInProgram;

/** \brief
 *  用这个变量来强制osDebugOut输出到log文件，不管g_bInterruptInProgram为真为假
 *
 *  此变量默认为假，如果要强制输出,必须在客户端设置为真，并且由客户端在int 3或是
 *  miniDump发生时，手工的关闭log文件，即调用sgdebug_end
 */
extern COMMON_API bool      g_bForceWriteLogFile;


//! 在否在osassert发生后,输出堆栈信息
extern COMMON_API bool      g_bExportStackInfo;

//! force dump the file (include the Macro "osassert" and "osassertex")
extern COMMON_API bool		g_bForceDumpFile;

//! 如果使用_SGRELEASE宏程序将以最快的速度和最小的内存运行,但不会有任何的异常处理机制.
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
*	<<<<<<<<<<<<<<<<重要说明>>>>>>>>>>>>>>>>>>
*
*	在包含common库的工程中的属性配置文件中,C/C++ 的命令行必须加上一个 
*	/EHa开关 来防止 Compiler Warning (level 3) C4535
*	这个开关是给C运行时的 _set_se_translator 函数用的。详细情况请查阅 MSDN
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

// 多线程异常的处理函数定义 
// 用于catch com_thread::thread_process osassert 异常
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
			if( g_bForceDumpFile ) { ::MessageBoxA( NULL,"软件中断强行生成Dump文件","客户端程序出错",MB_OK ); *(char*)0 = 0;}\
			if( ::IsDebuggerPresent() ){ _asm { int 3 };}\
			else\
			{\
				sprintf( g_ErrorSummary,"<#$>\n错误代码:<%s : %d>\n",__FILE__ , __LINE__ );\
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
			if( g_bForceDumpFile ) { ::MessageBoxA( NULL,"软件中断强行生成Dump文件","客户端程序出错",MB_OK ); *(char*)0 = 0;}\
			if( ::IsDebuggerPresent() ){ _asm { int 3 };}\
			else\
			{\
				sprintf( g_ErrorSummary,"<#$>\n错误描述:< %s >\n错误代码:<%s : %d>\n",(desc),__FILE__ , __LINE__ );\
				Outstr_tolog( g_ErrorSummary );\
				throw CSE(g_ErrorSummary); \
			}\
		}\
	}

# elif ( _SGRELEASE )

	#  pragma warning( disable : 4002 )  //“osDebugOut”宏的实参太多
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
 *  格式化字符串的函数，返回一个string类的实例。借助这个函数就可以给任意的字符
 *  串处理函数增加格式化的功能，同时避免在多个函数中使用可变参数。
 *
 *  For example :
 *
 *   void OutputInfo( const char * );
 *   ...
 *   OutputInfo( Formatstr("%s\n",errinfo).c_str() );
 *
 *  使用std::string而不是直接返回char*是出于多线程安全性的考虑
 *
 *  TODO : 将带可变参数的函数( 如Outstr_tolog()等 )更换为 foo( FormatStr(...) ) 
 *        的形式。
 */
# include <string>
COMMON_API std::string FormatStr( const char * fmt, ... );


/** \brief
 *  最简单的输出错误信息的函数.
 */
COMMON_API void        error_msg( const char* fmt, ... );



//
// sgDebugOut
// ==========
//
// 输入信息到调试器
//

//!	输出信息到调试器
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
//  CRT的越界检测函数必须在程序的主循环中使用    
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
 * 最低级错误的调试函数，很少使用。使用这个函数后在harddisk上生成
 * 一个crierror.txt的文件.
 */
COMMON_API void critical_error( const char* str, ... );


/** \brief
 *  用于对内存的assert,此宏只有debug版本中起作用。
 *
 *  \param _address 内存的起始地址。
 *  \param _size    要assert内存的大小。
 *  \param _access  如果为1，则检测此内存是否可读可写，如果为0,则只检测可读性
 */
# ifdef _DEBUG
# define assertMem( _address,_size,_access ) \
	     osassertex( _CrtIsValidPointer( _address,_size,_access ),"Memory Not Valid" ); 
# endif 

//! 在Release版本下，DoNothing.
# ifdef NDEBUG

# define assertMem( x,y,z )    ;

# endif 



//
// sgdebug_start(), sgdebug_end()
// ===============================
/**
 * 启动调试系统,我们应该把这个函数加入到winmain函数中去.
 *
 * @param logfilename 输出debug信息的*.txt文件的名字.
 * @see sgdebug_end()
 *
 */ 
COMMON_API bool sgdebug_start( const char* logfilename );

/** \brief
 * 终止调试系统,我们应该把这个函数加入到winmain函数中去.
 *
 * @see sgdebug_start()
 * \param bool _errorEnd 如果是非正常中止程序(由osassert中止程序)，
                         则此值设为true.
 */ 
COMMON_API void sgdebug_end( const char* _str = NULL,bool _errorEnd = false );



//! 是否打开更详细的性能测试
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
	//! 函数名字。
	char      funcname[256];       
	//! 调用次数。
	int       num;
	//! 总共使用时间。
	float     totalsec;            
	//! 最短执行时间。
	float     min;    
	//! 最长执行时间。 
	float     max;                 

	float     lasttime;
	
	//! 正在执年profile?
	bool      inprofile;          

	//! 每一个func profile 存一个开始时间.
	LARGE_INTEGER    m_lStart;
	
# if DETAIL_PROFILE
	float     m_fFrameTime[MAX_PROFILE_FRMAE];
# endif 

public:
	sg_funcprofiledata( void );
	
	/** 结束profile功能.
	 */
	float     end_profilefunc( const char* _funcname );
	/** 开始profile功能.
	 */
	void      start_profilefunc( void );
	
};

#define       MAX_PROFILENUM  32



class COMMON_API sg_profile
{
private:
	//用于整体的性能分析。
	int                    profuncnum;                  //测试函数的数目。
	float                  totalframetime;              //总共使用的时间。
	float                  minframetime;                    
	float                  maxframetime;
	float                  lastframetime;
	int                    framenum;
	
	//for
	int                    start;

	//! 每一帧 profile 存一个开始时间.
	LARGE_INTEGER          m_lStart;

protected:
	void      profile_analyze_out( int index );
	
private:
	sg_funcprofiledata     prodata[MAX_PROFILENUM];     //profile 的数据。
	
# if DETAIL_PROFILE
	float                  m_fFrameTime[MAX_PROFILE_FRMAE];
# endif 

public:
	sg_profile(void);
	~sg_profile( void ){};
	
	/** 这两个函数放在要测函数的开始和结束。
	 */
	void      profile_begin( char* funcname );
	void      profile_end( char* funcname );
	
	/** 这两个函数放在程序主循环的开始和结束。
	 */
	void      frame_movestart(void);
	void      frame_moveend( void );
	
	/** 在程序的最后做profile的分析和输出。
	 */
	void      dump_profile( char* filename );
	
	/** 输出到默认的文件，就是exception的log文件。
	 */
	void      dump_profile( void );                     
	
	
	//
	double    get_firstprofileper( void );
	
};

extern COMMON_API sg_profile*   debug_profile;




//此处打开OR关闭profile.
#define PROFILEENABLE 1      

#if PROFILEENABLE 
	/** 
	*  profile很是浪费时间,在win98下比在win2000下浪费的时间要多很多.
	*  开始使用Profile的宠,写在要测试的代码之前.
	*/
	#define PSTART(func)   { debug_profile->profile_begin(TEXT(#func)); }
	/**
	* 结束使用profile.写在要测试的代码之后.
	*/
	#define PEND(func)     { debug_profile->profile_end(TEXT(#func)); }

	/**
	* 在每一帧的开始加入这个宏,如果不加入的话,profile的值是一些随机的值.
	*/
	#define PROFILEMOVESTART     { debug_profile->frame_movestart(); }
	/**
	* 在每一帧的结束加入这个宏.
	*/
	#define PROFILEMOVEEND       { debug_profile->frame_moveend(); }
#else

	#define PSTART(func)
	#define PEND(func)  
	#define PROFILEMOVESTART  
	#define PROFILEMOVEEND    

#endif


//@{
//! 定义打开和关闭文件的宏,使保证打开一个文件后,在同一个函数中可以关闭掉这个文件.

//@}


////////////////////////////////////////////////////////////////////////////////////////////
//
// 使用我们自己的rand系统。
extern void       init_osRandSystem( void );
extern void       end_osRandSystem( void );
COMMON_API int        osRand( void );

//! 程序使用使用RAND宏来使用引擎自己的Rand函数。
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