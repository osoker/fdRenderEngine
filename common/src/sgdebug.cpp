//-----------------------------------------------------------------------------
//Filename:sgdebug.cpp
//Desc:    func and macro to help sg engine debug.
//His:     River created at 9/20 2001
//-----------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/sgdebug.h"
# include "../include/timecounter.h"
#include "../LogWriter.h"

# include "../include/sgdefine.h"




COMMON_API sg_profile*   debug_profile = NULL;


COMMON_API bool          g_bLogToFile = FALSE;


/** \brief
 *  此变量确认是中断到程序，还是直接引发异常，把Stack导出DebugView中间。
 *
 *  默认情况下，中断到程序中。如果要编译给测试团队，则可以直接引发异常。
 */
COMMON_API bool      g_bInterruptInProgram = false;

/** \brief
 *  用这个变量来强制osDebugOut输出到log文件，不管g_bInterruptInProgram为真为假
 *
 *  此变量默认为假，如果要强制输出,必须在客户端设置为真，并且由客户端在int 3或是
 *  miniDump发生时，手工的关闭log文件，即调用sgdebug_end
 */
COMMON_API bool      g_bForceWriteLogFile = false;

//! 在否在osassert发生后,输出堆栈信息
COMMON_API bool      g_bExportStackInfo = FALSE;


//! force dump the file (include the Macro "osassert" and "osassertex")
COMMON_API bool		g_bForceDumpFile = false;



CLogWriter			g_logWriter;


//! the critical section to make the debug out normal
//! because those use the "static" variables
enum _CriticalSection
{
	Outstr_tolog_const_char_Sec,
	OutBuf_tolog_char_int_Sec,
	sgDebugOutFunc_directly_Sec,
	sgDebugOutFunc_tag_Sec,
	sgDebugOutFunc_Sec,

	va_thread_0_Sec,
	va_thread_1_Sec,
	va_thread_2_Sec,
	va_thread_3_Sec,
	va_thread_4_Sec,

	CriticalSection_Num,
};
static CRITICAL_SECTION sg_criticalSectionSet[CriticalSection_Num] = {0};

//
//
// ----------------------------------------------------------------------------
void Outstr_tolog( const char* fmt, ... )
{	
	if( g_bLogToFile ) {

		CCriticalSec t_criticalSec(&sg_criticalSectionSet[Outstr_tolog_const_char_Sec]);

		va_list		argptr;
		static char msg[8192];

		va_start (argptr,fmt);
		vsprintf (msg,fmt,argptr);
		va_end (argptr);

		strcat( msg,"\n" );
		g_logWriter.PushLogString(msg);

		OutputDebugString( msg );
	}
}


//
const char * Int2Hex(unsigned char c, char *_pbuf)
{
	static const char HexTable[] = "0123456789ABCDEF"; 

	_pbuf[0] = HexTable[ (c & 0xF0) >> 4 ];
	_pbuf[1] = HexTable[ c & 0x0F ];
	_pbuf[2] = '\0';

    return _pbuf;
}

#define  MAXMESSLENGTH 8192

/**brief
 *打印BUF到
 *add by blueboy @ 2005.9.6
 */\
void OutBuf_tolog(char * _pbuf, int nLen)
{
	guard;

	if(g_bLogToFile){

		CCriticalSec t_criticalSec(&sg_criticalSectionSet[OutBuf_tolog_char_int_Sec]);

		char		t_szMsg[MAXMESSLENGTH];
		char		t_szTmp[10];

		osassert(nLen <= 2048);

		t_szMsg[0] = '\0';

		for (int i = 0; i < nLen; ++i, ++_pbuf)
		{
			strcat(t_szMsg, Int2Hex(*_pbuf, t_szTmp));
			strcat(t_szMsg, " ");
		}

		Outstr_tolog(t_szMsg);
	}

	unguard;

}

//! 直接输出一条信息，不进行vsprintf 解析
void sgDebugOutFunc_directly( const char * msg )
{
	// 加入强制输出log文件的条件
	if( g_bLogToFile ) 
	{
		CCriticalSec t_criticalSec(&sg_criticalSectionSet[sgDebugOutFunc_directly_Sec]);

		g_logWriter.PushLogString(msg);
		// 输出字符串
		//
		OutputDebugString( msg );
	}
	
}

//! 在输出消息里面加上一个标志，用于解析这个log
void sgDebugOutFunc_tag( const char* _tag,const char * msg, ... )
{
	// 加入强制输出log文件的条件
	if( g_bLogToFile ){

		CCriticalSec t_criticalSec(&sg_criticalSectionSet[sgDebugOutFunc_tag_Sec]);

		va_list argptr;
		static char	  text[8192];

		va_start (argptr,msg);
		vsprintf (text, msg,argptr);
		va_end (argptr);

		time_t t_timeVal = time(NULL);
		char t_timeStr[32];
		itoa((int)t_timeVal,t_timeStr,10);

		std::string t_final = "\n{";
		t_final += _tag;
		t_final += "\n";

		t_final += " Time=\"";
		t_final += t_timeStr;
		t_final += "\" ";

		t_final += "Desc=\"";
		t_final += text;
		t_final += "\" \n";

		t_final += "/}\n";

	
		g_logWriter.PushLogString(t_final.c_str());

		// 输出字符串
		//
		OutputDebugString( t_final.c_str() );
	}
	
	
}
//
// sgDebugOut
// Added by yuanlin 2002/4/10
//-----------------------------------------------------------------------------
void sgDebugOutFunc( const char* msg, ... )
{
	// 加入强制输出log文件的条件
	if( g_bLogToFile ) 
	{

		CCriticalSec t_criticalSec(&sg_criticalSectionSet[sgDebugOutFunc_Sec]);

		va_list argptr;
		static char	  text[8192];

		va_start (argptr,msg);
		vsprintf (text, msg,argptr);
		va_end (argptr);
		strcat( text,"\n" );

		g_logWriter.PushLogString(text);

		// 输出字符串
		//
		OutputDebugString( text );
		OutputDebugString("\n");
	}

}


//
//最低级错误的调试函数，很少使用。
// ----------------------------------------------------------------------------
void     critical_error( const char* str, ... )
{
	FILE*  stream = NULL;
	va_list		argptr;
	char msg[256];
	
	va_start (argptr,str);
	vsprintf (msg,str,argptr);
	va_end (argptr);
	strcat( msg,"\n" );
	
	stream = fopen( ERRORFILE,"at" );
	if( stream )
		fprintf( stream,msg );
	
	fclose( stream );
	stream = NULL;

	
}

/** \brief
*  最简单的输出错误信息的函数.
*/
void        error_msg( const char* fmt, ... )
{
	va_list       argptr;
	char          msg[256];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	MessageBox( NULL,msg,"ERROR",MB_OK );

	return;

}



//
// CustomAssertFunction
// ----------------------------------------------------------------------------
bool CustomAssertFunction( bool exp, const char* desc, int line, const char* file )
{
	//表达式为是，什么都不做
	if( exp )
		return false;
	else//表达式为非
	{

		// 保留20k的缓冲区，使用可以转出大一些的出错信息
		char*  text = new char[1024*20];
		
		sprintf( text, "File:%s Line:%d		%s\n\n", file, line, desc );

		// 输出信息到vc的debug框。
		Outstr_tolog( text );

		SAFE_DELETE_ARRAY( text );

		return true;
	}

}

//-----------------------------------------------------------------------------
//ATTENTION:这种guard的错误检测方式有一个做不到地方:
//         如果在winmain之外调用的函数出错,不可能在
//         我们的winmain  catch中得到错误.比如全局变量
//         的解构函数出现错误.因为是在winmain外解构,所以
//         在文件不能记录错误.最少使用全局变量或者
//         是使用全局指针,在winmain中解构全局变量.
//we should add the following func to winmain func
//-----------------------------------------------------------------------------
bool sgdebug_start(const char* logfilename )
{
	if(!g_logWriter.InitLogWriter(logfilename)){
		MessageBox( NULL,"Initialize debug system failed!!!","ERROR",MB_OK );
		return FALSE;
	}
		
	debug_profile = new sg_profile();
	init_osRandSystem();

	try{
		for(size_t i = 0;i < CriticalSection_Num;i++){
			::InitializeCriticalSection(&sg_criticalSectionSet[i]);
		}
	}catch(...){
		//! no memory (STATUS_NO_MEMORY)
		MessageBox(NULL,"您的系统系统内存过低，不能初始化游戏。","错误!",MB_OK);
		return FALSE;		
	}	
	
	return TRUE;
}

//
//
// ----------------------------------------------------------------------------
void sgdebug_end( const char* _str/* = NULL*/,bool _errorEnd/* = false*/  )
{
	if(_str){
		OutputDebugString( _str );
		OutputDebugString( "\n\n\n");
	}

	g_logWriter.EndLogWriter(_str);

	if(debug_profile){
		//out put the profile.
		debug_profile->dump_profile();
		SAFE_DELETE(debug_profile);

		debug_profile = NULL;
	}

	// 
	// 全局的释放当前timer的instance.
	sg_timer::DInstance();

	end_osRandSystem();
	
	for(size_t i = 0;i < CriticalSection_Num;i++){
		::DeleteCriticalSection(&sg_criticalSectionSet[i]);
	}
}




//-----------------------------------------------------------------------------
//用于测试函数性能的一组类和函数。
//不很精确，如果需要的话，以后改进。
//-----------------------------------------------------------------------------
static LARGE_INTEGER	t_lCount;	//! 时钟频率

static LARGE_INTEGER    t_lEnd;

sg_funcprofiledata::sg_funcprofiledata( void )
{
	
	memset( funcname,0,sizeof( char )*256 );
	num = 0;
	totalsec = 0;
	min = 10000000;            //要是一个函数执行比这个时间还要长，fuck.
	max = 0;
	inprofile = FALSE;
	lasttime = 0;

	QueryPerformanceFrequency( &t_lCount );


# if DETAIL_PROFILE
	memset( m_fFrameTime,0,sizeof( float )*MAX_PROFILE_FRMAE );
# endif 
	

}

//
//
// ----------------------------------------------------------------------------
void sg_funcprofiledata::start_profilefunc( void )
{
	guard;
	
	inprofile = TRUE;
	QueryPerformanceCounter( &m_lStart );

	return;
	
	unguard;
}

//
//
// ----------------------------------------------------------------------------
float sg_funcprofiledata::end_profilefunc( const char* _funcname )
{
	guard;
	
	float   tmpf;
	
	// 使用精确的处理方法
	QueryPerformanceCounter( &t_lEnd );
	tmpf = float((t_lEnd.QuadPart-m_lStart.QuadPart)/(double)t_lCount.QuadPart);

	//! river mod @ 2007-7-30:在双核cpu的机器上,QueryPerformanceCounter为出现问题，兼容这个错误
	if( tmpf < 0.0f )
		tmpf = 0.0f;
	/*
	osassertex( tmpf >= 0.0f,
		va( "The <%s> function start<%d,%d>..End<%d,%d>..\n",_funcname,
		m_lStart.HighPart,m_lStart.LowPart,t_lEnd.HighPart,t_lEnd.LowPart )  );
	*/

	totalsec += tmpf;
	if( tmpf>max )
		max = tmpf;
	if( tmpf<min )
		min = tmpf;
	
	lasttime = tmpf;
	inprofile = FALSE;
	num++;
	
	// 返回当前的profile时间
	return tmpf;
	
	unguard;
}


//
//
// ----------------------------------------------------------------------------
sg_profile::sg_profile(void)
{
	
	profuncnum = 0;
	totalframetime = 0;
	minframetime = 10000;
	lastframetime = 0;
	maxframetime = 0;
	framenum = 0;

	
}

//
// 这两个函数放在要测函数的开始和结束。
// ----------------------------------------------------------------------------
void sg_profile::profile_begin( char* funcname )
{
	guard;//( sg_profile::profile_begin() );
	
	int   tmpi;
	
	for( tmpi=0;tmpi<profuncnum;tmpi++ )
	{
		//找到要profile的函数。
		if( !strcmp( prodata[tmpi].funcname,funcname )  )
		{
			if( (prodata[tmpi].inprofile) )
			{
				Outstr_tolog( "Profile file func <%s> error",funcname );
				osassert( FALSE );
			}
			prodata[tmpi].start_profilefunc();
			return;
		}
	}
	
	osassertex( profuncnum < MAX_PROFILENUM,"过多的性能测试位置..\n"  );

	//set the new profile func name.
	strcpy( prodata[profuncnum].funcname,funcname );
	prodata[profuncnum].start_profilefunc();
	profuncnum ++;
	
	return;
	
	unguard;
}

//
//
// ----------------------------------------------------------------------------
void sg_profile::profile_end( char* funcname )
{
	guard;//( sg_profile::profile_end() );
	
	int   tmpi;
	
	for( tmpi=0;tmpi<profuncnum;tmpi++ )
	{
		if( !strcmp( prodata[tmpi].funcname,funcname ) )
		{
			osassert( (prodata[tmpi].inprofile ) );
			float t_fRes = prodata[tmpi].end_profilefunc( funcname );
			

# if DETAIL_PROFILE

			if( MAX_PROFILE_FRMAE > framenum )
				prodata[tmpi].m_fFrameTime[framenum-1] = t_fRes;
# endif 

			return;
		}
	}
	
	Outstr_tolog( "Dismatch profile func <%s>",funcname );
	osassert( FALSE );
	
	return;
	
	unguard;
}

//
// 这两个函数放在程序主循环的开始和结束。
// ----------------------------------------------------------------------------
void sg_profile::frame_movestart(void)
{
	guard;//( sg_profile::frame_movestart() );
	

	framenum++;

	start = timeGetTime();

	// 使用精确的处理方法
	QueryPerformanceCounter( &m_lStart );

	return;
	
	unguard;
}

//
//
// ----------------------------------------------------------------------------
void sg_profile::frame_moveend( void )
{
	guard;//( sg_profile::frame_moveend() );
	
	float       tmpf;

# if 1
	int         end;
	
	end = timeGetTime();
	tmpf = (end-start)/(float)1000.0;

# if DETAIL_PROFILE
	if( framenum < MAX_PROFILE_FRMAE )
		m_fFrameTime[framenum-1] = tmpf;
# endif 

# else

	QueryPerformanceCounter( &t_lEnd );
	tmpf = (t_lEnd.QuadPart-m_lStart.QuadPart)/(float)t_lCount.QuadPart;

# if DETAIL_PROFILE

	if( framenum < MAX_PROFILE_FRMAE )
		m_fFrameTime[framenum-1] = tmpf;

	/*
	// River mod @ 2007-4-28:
	if( (framenum > 2)&&(framenum<(MAX_PROFILE_FRMAE-1)) )
	{
		for( int t_i = 0;t_i<profuncnum;t_i ++ )
		{
			if( (prodata[t_i].m_fFrameTime[framenum-1]/m_fFrameTime[framenum-1])>1.0f )
				osassert( false );
		}
	}
	*/

# endif 

# endif 

	if( tmpf>maxframetime )
		maxframetime = tmpf;
	if( tmpf<minframetime )
		minframetime = tmpf;
	
	totalframetime+=tmpf;
	lastframetime = tmpf;
	
	
	
	unguard;
}

//
//
// ----------------------------------------------------------------------------
double sg_profile::get_firstprofileper( void )
{
	
	return double(prodata[0].lasttime)/double(lastframetime);
	
}

//
//在程序的最后做profile的分析和输出。
// ----------------------------------------------------------------------------
void sg_profile::dump_profile( char* filename )
{
	guard;// sg_profile::dump_profile() );
	
	unguard;
}

//
//
// ----------------------------------------------------------------------------
void sg_profile::profile_analyze_out( int index )
{	
	Outstr_tolog( "--------------------------------Func name<%s>-----------------------------",prodata[index].funcname );
	Outstr_tolog( "Called num----------------------:%d",prodata[index].num );
	Outstr_tolog( "Average func time---------------:%f",prodata[index].totalsec/prodata[index].num );
	Outstr_tolog( "Min func time-------------------:%f",prodata[index].min );
	Outstr_tolog( "Max func time-------------------:%f",prodata[index].max );
	Outstr_tolog( "Percentage of total app time----:%f",prodata[index].totalsec/this->totalframetime );
	
	return;
	
}

//
//输出到默认的文件，就是exception的log文件。
// ----------------------------------------------------------------------------
void sg_profile::dump_profile( void )       
{
	guard;//( sg_profile::dump_profile() );
	
	if(debug_profile){

		int     tmpi;
		
		//输出堆栈结束符号用于bugManage解析
		Outstr_tolog("<$#>");

		//general infomation:
		Outstr_tolog( "//----------------------------General profile infomation--------------------------------------//" );
		//输出总的流逝时间。
		Outstr_tolog( "Total frame time is:----------------------%f",totalframetime );
		
		//输出帧数。
		Outstr_tolog( "Total frame num is:-----------------------%d",framenum );
		
		//输出平均帧数
		Outstr_tolog( "Average frame num is:---------------------%f",framenum/totalframetime );
		
		//最短帧时间。
		Outstr_tolog( "The min frame time is:--------------------%f",minframetime );
		
		//最长帧时间。
		Outstr_tolog( "The max frame time is:--------------------%f",maxframetime );
		
		Outstr_tolog( "//---------------------------Per function infomation-----------------------------------------//" );
		
		
		//per func infomation:
		for( tmpi=0;tmpi<this->profuncnum;tmpi++ )
			profile_analyze_out( tmpi );
		
		// 输出细节profile的处理
		int  t_iMax = framenum;
		if( t_iMax > MAX_PROFILE_FRMAE )
			t_iMax = MAX_PROFILE_FRMAE;

	# if DETAIL_PROFILE
		s_string   t_szStr;
		char       t_sz[256];
		for( tmpi=0;tmpi<t_iMax;tmpi ++ )
		{
			sprintf( t_sz,"PF<%f><%f>",
				1.0f/m_fFrameTime[tmpi],m_fFrameTime[tmpi] );
			t_szStr = t_sz;
			for( int t_k=0;t_k<profuncnum;t_k ++ )
			{
				sprintf( t_sz,"<%s><%f><%f>",prodata[t_k].funcname,
					(prodata[t_k].m_fFrameTime[tmpi] / m_fFrameTime[tmpi])*100.0f,
					prodata[t_k].m_fFrameTime[tmpi] );
				t_szStr += t_sz;
			}
			t_szStr += "\n";

			osDebugOut( t_szStr.c_str() );
		}
	# endif 

	}

	unguard;	
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday

River: From QuakeIII
跟FormatStr核心功能一致，一个返回std::string，一个返回const char* 
============
*/
static const int scg_va_maxStringNum = va_thread_4_Sec + 1;
static char		sg_va_string[scg_va_maxStringNum][1024 * 20];	// in case va is called by nested(multi-thread) functions
static int		sg_va_tindex = va_thread_0_Sec;

COMMON_API const char* va( char *format, ... )
{
	CCriticalSec t_criticalSec(&sg_criticalSectionSet[sg_va_tindex]);

	char* t_buf = sg_va_string[sg_va_tindex++];
	if(sg_va_tindex >= scg_va_maxStringNum){
		sg_va_tindex = va_thread_0_Sec;
	}	

	va_list		argptr;
	va_start (argptr, format);
	vsprintf (t_buf, format,argptr);
	va_end (argptr);

	return t_buf;
}




////////////////////////////////////////////////////////////////////////////////////////////
//
// 使用我们自己的rand系统。因为particle系统中用到了太多的rand,使用这个rand要快一些。
# include "../include/sgdefine.h"
# define MAX_RANDIDX  0xfff
static    VEC_word  t_vecRand;
void       init_osRandSystem( void )
{
	srand( timeGetTime() );
	t_vecRand.resize( MAX_RANDIDX );
	for( int t_i=0;t_i<MAX_RANDIDX;t_i ++ )
		t_vecRand[t_i] = rand();
}

void       end_osRandSystem( void )
{
	t_vecRand.clear();
}

static   int   t_iRandIdx = 0;
int      osRand( void )
{

	t_iRandIdx &= MAX_RANDIDX;
	return  t_vecRand[t_iRandIdx++];
}
////////////////////////////////////////////////////////////////////////////////////////////

# include <direct.h>
void get_dumpfileName(char* _lf)
{
	//gFontManager.RegisterFont(EFT_Font2,"黑体",16);
	SYSTEMTIME   t_systime;
	
	GetLocalTime( &t_systime );

	mkdir("dump");

	sprintf( _lf,"dump\\Dumpfile_%d年_%d月_%d日_%d时_%d分.dmp",t_systime.wYear,
		t_systime.wMonth,t_systime.wDay,t_systime.wHour,t_systime.wMinute );

}
//! this function is used to generate miniDump file 
void CreateMiniDump( LPEXCEPTION_POINTERS _lpExceptionInfo) 
{

	// get the log name
	char t_logName[MAX_PATH];
	get_dumpfileName(t_logName);

	// Open a file
	HANDLE hFile = CreateFileA(t_logName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ( hFile != NULL &&  hFile != INVALID_HANDLE_VALUE ) {

		// Create the minidump 
		MINIDUMP_EXCEPTION_INFORMATION t_mdei;
		t_mdei.ThreadId          = GetCurrentThreadId();
		t_mdei.ExceptionPointers = _lpExceptionInfo;
		t_mdei.ClientPointers    = FALSE;

		MINIDUMP_TYPE t_mdt      = MiniDumpFilterMemory;
		BOOL t_ret = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(),
								hFile, t_mdt, ( _lpExceptionInfo != 0 ) ? & t_mdei : 0, 0, 0);

		if ( !t_ret ) {
			OutputDebugString( va("MiniDumpWriteDump failed. Error: %u \n", GetLastError() )); 
		}else{ 
			OutputDebugString( "Minidump created.\n" ); 
		}

		// Close the file
		CloseHandle( hFile );

	} else {
		OutputDebugString( va("Create Dump File failed. Error: %u \n", GetLastError() )); 
	}

}

//! the CUP exception enabled flag
static BOOL sg_bCUPExceptionEnabled = FALSE;
static DWORD sg_dwReadWriteMemory = 0;
static BOOL sg_bReadException = FALSE;
static BOOL sg_bWriteException = FALSE;

/*! \brief enable the CUP inject the exception of read\write some 
*			memory according specific address
*
*	\param _bEnable				: the switch of exception
*	\param _dwAddress			: the memory specific address
*	\param _bWrite				: rise exception when write this address
*	\param _bRead				: rise exception when read this address
*/
COMMON_API void EnableCPUMemoryException(BOOL _bEnable,DWORD _dwAddress,BOOL _bWrite,BOOL _bRead /*=FALSE*/)
{
	guard;
	
	// Example: to monitor the 'val' variable to be wrote
	// 
	// EnableCPUMemoryException(TRUE,(DWORD)val,TRUE);
	//
	//	function(const int& val);
	//
	// EnableCPUMemoryException(FALSE,(DWORD)val,TRUE);
	//
	//

	// cannot work without debugger
	//
	if(!::IsDebuggerPresent())	return;

	if(_bEnable == sg_bCUPExceptionEnabled 
		&& _dwAddress == sg_dwReadWriteMemory
		&& _bWrite == sg_bWriteException
		&& _bRead == sg_bReadException) 
		return ;

	if(!_bWrite && !_bRead && _bEnable) return;

	// test the address space
	//
	if(_dwAddress <= 0x00010000 || _dwAddress >= 0x7FFEFFFF){
		osassert("address is illegal for user address space!");
		return;
	}
	
	sg_dwReadWriteMemory		= _dwAddress;
	sg_bCUPExceptionEnabled		= _bEnable;
	sg_bReadException			= _bRead;
	sg_bWriteException			= _bWrite;

	CONTEXT t_context;
	HANDLE  t_thread = ::GetCurrentThread();

	t_context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	if(!::GetThreadContext(t_thread,&t_context)){
		osassert("Cannot get the thread context!");
		return;
	}

	if(_bEnable){

		t_context.Dr0 = _dwAddress;

		if(_bWrite){
			t_context.Dr7 = 0xD0001;
		}

		if(_bRead && _bWrite){
			t_context.Dr7 = 0xF0001;
		}

	}else{
		t_context.Dr7 = 0;
	}

	if(!::SetThreadContext(t_thread,&t_context)){
		osassert("Cannot set the thread context");
		return;
	}

	unguard;
}

COMMON_API char g_ErrorSummary[1024];
COMMON_API char* g_GetErrorSummary()
{
	return g_ErrorSummary;
}

static size_t MaxStrCpy(char* _szDest,const char* _szSrc,size_t _maxLen)
{
	guard;

	osassert(_szDest && _szSrc);

	size_t i = 0;
	while(i < _maxLen && _szSrc[i] != 0){

		if(((unsigned char) _szSrc[i]) > 127){

			if(i >= (_maxLen - 2) ){
				break;
			}

			_szDest[i] = _szSrc[i];
			_szDest[i+1] = _szSrc[i+1];

			i += 2;
		}else{
			_szDest[i] = _szSrc[i];
			i++;
		}

	}

	_szDest[(i < _maxLen)?i:(_maxLen-1)] = 0;

	return i;

	unguard;
}

COMMON_API void GetWindowsVerDesc(size_t _maxLength,char* _desc)
{
	guard;

	osassert(_desc);

	const size_t BUFSIZE = 120;

	OSVERSIONINFOEX t_info;
	ZeroMemory(&t_info,sizeof(t_info));
	t_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	BOOL t_bCallExSucc;

	std::string t_result = "未知版本";

	if( !(t_bCallExSucc = ::GetVersionEx((LPOSVERSIONINFO)&t_info))){
		t_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if(!::GetVersionEx((LPOSVERSIONINFO)&t_info)){
			goto GetWindowsVerDesc_return ;
		}
	}

	switch (t_info.dwPlatformId)
	{
		// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		// Test for the specific product.
		if(t_info.dwMajorVersion == 6 ){
			t_result = "Microsoft Windows Vista";
		}else if( t_info.dwMajorVersion == 5 && t_info.dwMinorVersion == 2 ){
			t_result = "Microsoft Windows Server 2003, ";
		}else if(t_info.dwMajorVersion == 5 && t_info.dwMinorVersion == 1){
			t_result = "Microsoft Windows XP ,";
		}else if(t_info.dwMajorVersion == 5 && t_info.dwMinorVersion == 0){
			t_result = "Microsoft Windows 2000 ,";
		}else if(t_info.dwMajorVersion <= 4){
			t_result = "Microsoft Windows NT ,";
		}else{
			t_result = "Unknow Window System (Maybe Window 7)";
		}

		// Test for specific product on Windows NT 4.0 SP6 and later.
		if( t_bCallExSucc )
		{
			// Test for the workstation type.
			if ( t_info.wProductType == VER_NT_WORKSTATION )
			{
				if( t_info.dwMajorVersion == 4 )
					t_result +=  "Workstation 4.0 " ;
				else if( t_info.wSuiteMask & VER_SUITE_PERSONAL )
					t_result +=  "Home Edition " ;
				else
					t_result += "Professional " ;
			}

			// Test for the server type.
			else if ( t_info.wProductType == VER_NT_SERVER || 
				t_info.wProductType == VER_NT_DOMAIN_CONTROLLER )
			{
				if( t_info.dwMajorVersion == 5 && t_info.dwMinorVersion == 2 )
				{
					if( t_info.wSuiteMask & VER_SUITE_DATACENTER )
						t_result +=  "Datacenter Edition ";
					else if( t_info.wSuiteMask & VER_SUITE_ENTERPRISE )
						t_result +=  "Enterprise Edition " ;
					else if ( t_info.wSuiteMask == VER_SUITE_BLADE )
						t_result +=  "Web Edition " ;
					else
						t_result += "Standard Edition " ;
				}

				else if( t_info.dwMajorVersion == 5 && t_info.dwMinorVersion == 0 )
				{
					if( t_info.wSuiteMask & VER_SUITE_DATACENTER )
						t_result +=  "Datacenter Server " ;
					else if( t_info.wSuiteMask & VER_SUITE_ENTERPRISE )
						t_result += "Advanced Server " ;
					else
						t_result += "Server " ;
				}

				else  // Windows NT 4.0 
				{
					if( t_info.wSuiteMask & VER_SUITE_ENTERPRISE )
						t_result += "Server 4.0, Enterprise Edition " ;
					else
						t_result += "Server 4.0 " ;
				}
			}
		}
		else  // Test for specific product on Windows NT 4.0 SP5 and earlier
		{
			HKEY hKey;
			char szProductType[BUFSIZE];
			DWORD dwBufLen=BUFSIZE;
			LONG lRet;

			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey );

			if( lRet != ERROR_SUCCESS )
				goto GetWindowsVerDesc_return ;

			lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
				(LPBYTE) szProductType, &dwBufLen);

			if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
				goto GetWindowsVerDesc_return ;

			RegCloseKey( hKey );

			if ( lstrcmpi( "WINNT", szProductType) == 0 )
				t_result += "Workstation " ;
			if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
				t_result += "Server " ;
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				t_result += "Advanced Server " ;

			sprintf(szProductType, "%d.%d ", t_info.dwMajorVersion, t_info.dwMinorVersion );
			t_result += szProductType;
		}

		// Display service pack (if any) and build number.

		if( t_info.dwMajorVersion == 4 && 
			lstrcmpi( t_info.szCSDVersion, "Service Pack 6" ) == 0 )
		{
			HKEY hKey;
			LONG lRet;
			char t_tmpBuffer[sizeof(t_info.szCSDVersion) + BUFSIZE];

			// Test for SP6 versus SP6a.
			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
				0, KEY_QUERY_VALUE, &hKey );

			if( lRet == ERROR_SUCCESS ){
				sprintf(t_tmpBuffer,"Service Pack 6a (Build %d)", t_info.dwBuildNumber & 0xFFFF );         
				t_result += t_tmpBuffer;
			}else // Windows NT 4.0 prior to SP6a
			{
				sprintf(t_tmpBuffer, "%s (Build %d)",
					t_info.szCSDVersion,
					t_info.dwBuildNumber & 0xFFFF);

				t_result += t_tmpBuffer;
			}

			RegCloseKey( hKey );
		}
		else // not Windows NT 4.0 
		{
			char t_tmpBuffer[sizeof(t_info.szCSDVersion) + BUFSIZE];
			sprintf(t_tmpBuffer, "%s (Build %d)",
				t_info.szCSDVersion,
				t_info.dwBuildNumber & 0xFFFF);

			t_result += t_tmpBuffer;
		}


		break;

		// Test for the Windows Me/98/95.
	case VER_PLATFORM_WIN32_WINDOWS:

		if (t_info.dwMajorVersion == 4 && t_info.dwMinorVersion == 0)
		{
			t_result = "Microsoft Windows 95 ";
			if ( t_info.szCSDVersion[1] == 'C' || t_info.szCSDVersion[1] == 'B' )
				t_result += "OSR2 ";
		} 

		if (t_info.dwMajorVersion == 4 && t_info.dwMinorVersion == 10)
		{
			t_result = "Microsoft Windows 98 ";
			if ( t_info.szCSDVersion[1] == 'A' )
				t_result += "SE ";
		} 

		if (t_info.dwMajorVersion == 4 && t_info.dwMinorVersion == 90)
		{
			t_result += "Microsoft Windows Millennium Edition";
		} 
		break;

	case VER_PLATFORM_WIN32s:

		t_result += "Microsoft Win32s";
		break;
	}


GetWindowsVerDesc_return:

	MaxStrCpy(_desc,t_result.c_str(),_maxLength);

	unguard;
}


COMMON_API void GetCpuInfoDesc(size_t _maxLength,char* _desc)
{
	guard;

	osassert(_desc);

	int t_maxFuncRequery;
	char t_s0[5] = {0};
	char t_s1[5] = {0};
	char t_s2[5] = {0};
	char t_s3[5] = {0};


	std::string t_result = "";

	// get the cpu's id string
	__asm
	{
		xor eax,eax
		cpuid
		mov t_maxFuncRequery,eax
		mov dword ptr[t_s1],ebx
		mov dword ptr[t_s2],edx
		mov dword ptr[t_s3],ecx
	}

	if(!t_maxFuncRequery){ // if not support the cpuid ins
		goto GetCpuInfoDesc_return;
	}

	// get the cpu desc string
	//
	for(volatile size_t i = 0x80000002 ; i < 0x80000005 ;i++){

		__asm
		{
			mov eax,i
			cpuid
			mov dword ptr[t_s0],eax
			mov dword ptr[t_s1],ebx
			mov dword ptr[t_s2],ecx
			mov dword ptr[t_s3],edx
		}

		t_result += t_s0;
		t_result += t_s1;
		t_result += t_s2;
		t_result += t_s3;
	}


GetCpuInfoDesc_return:

	MaxStrCpy(_desc,t_result.c_str(),_maxLength);

	unguard;
}

COMMON_API void GetMemSizeInfoDesc(size_t _maxLength,char* _desc)
{
	guard;

	osassert(_desc);

	const size_t DIV		= 1024;
	const size_t WIDTH		= 7;

	char t_tmpString[128];
	
	std::string t_result = "";

	MEMORYSTATUSEX t_statex;
	t_statex.dwLength = sizeof (t_statex);

	GlobalMemoryStatusEx (&t_statex);

	sprintf (t_tmpString,"%*I64d MB 系统内存   ",
		WIDTH, t_statex.ullTotalPhys/(1024 *1024));

	t_result += t_tmpString;

	sprintf (t_tmpString,"%*I64d MB 空闲系统内存   ",
		WIDTH, t_statex.ullAvailPhys/(1024*1024));

	t_result += t_tmpString;

	MaxStrCpy(_desc,t_result.c_str(),_maxLength);

	unguard;
}
