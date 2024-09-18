//=======================================================================================================
/**  \file
 *   Filename: fileOperate.cpp
 *   Desc:     Osok引擎中操作文件的帮助函数和类.
 *
 *   His:      River created @ 4/27 2003.
 *
 *   "记住不要害怕手下的人的成长，公司的成长取决于是不是每一个人都在成长，
 *    如果公司内部每个人都在成长，那么这个公司就在成长。"
 *  
 */
//=======================================================================================================
#include "stdafx.h"

# include "../interface/fileOperate.h"
# include "../interface/osInterface.h"
# include "packFileOperate.h"
# include "direct.h"

#pragma comment (lib,"winmm.lib")
#define PackageRootFolder    "tangrun"
//! 全局是否使用打包文件的开关。初始化为不使用。
FILEOPERATE_API    BOOL       g_bUsePackSystem = FALSE;

//! 是否可以使用替代文件。
FILEOPERATE_API    BOOL       g_bUseDefaultFile = TRUE;

//@{
//tzz added for finding galaxy game's file
//! does record filename?
FILEOPERATE_API    BOOL       g_bRecordFilename = FALSE;

//! initialize ?
BOOL						  g_bInitRecord = FALSE;

//! FILE handle
//FILE*						  g_fileHandle = NULL;

//! current file line
int							  g_iCurrFileIndex = 0;

//! filename arry
char						  g_arrFilename[5][300] = {0};

class CRecordFile
{
public:
	~CRecordFile(void){

		if(g_bInitRecord){
			FILE* t_file =  fopen("usingFile.txt","w");
			for(int i = 0;i< g_iCurrFileIndex;i++){
				fprintf(t_file,"%s\n",g_arrFilename[i]);
			}
			fclose(t_file);
		}
	}
};

CRecordFile					  g_record;

void init_record(void)
{	
	//ZeroMemory(g_arrFilename,sizeof(g_arrFilename));
}

//! the filename
void record_filename(const char* _filename)
{
	if(!g_bInitRecord){
		init_record();
		g_bInitRecord = TRUE;
	}

	if(_filename){
			FILE* t_file =  fopen("usingFile.txt","a");
			
			fprintf( t_file,"%s\n",_filename );
			fclose(t_file);		
	}

}

//@}



// TEST CODE: 读文件需要使用监界区???
CRITICAL_SECTION    g_sFileReadCS;



/** \brief
 *  全局性的缓冲区数据变量.
 *
 *  用于调入文件或是其它的用途的一个临时缓冲区.大小为4M.
 *  在调入文件的时候为这个缓冲区分配内存,使用完之后删去
 *  这些内存,由使用自己分配和删去段内存.
 */
static BYTE*       g_tmpBuffer = NULL;

//! 小一些的全局性数据缓冲区,只能是主线程使用.
static BYTE*       g_tmpLittleBuffer = NULL;

//@{
/**
 *  使用这两个宏使用全局的数据缓冲区.
 */
# define   START_GBUFFER      g_tmpBuffer = new BYTE[TMP_BUFSIZE]; \
							  ((DWORD*)g_tmpBuffer)[0] = 0;((DWORD*)g_tmpBuffer)[1]= 0;\
	                        g_tmpLittleBuffer = new BYTE[TMP_BUFSIZE]; \
							  ((DWORD*)g_tmpLittleBuffer)[0] = 0;((DWORD*)g_tmpLittleBuffer)[1]= 0;


# define   END_GBUFFER      SAFE_DELETE_ARRAY( g_tmpBuffer );\
	                       SAFE_DELETE_ARRAY( g_tmpLittleBuffer );
//@}




//! 是否在使用GBuffer时，导出信息。
FILEOPERATE_API BOOL                     g_bExportInfoUseGBuf = FALSE;

//! gbuffer用到的多线程安全缓冲区.
static CRITICAL_SECTION                  g_sGbufSec;
  
//! 一次性调入所有的xml设施。
extern bool init_xmlDataFileCatch( void );

//@{ 
//  如果不使用pack文件，以下的两个函数不需要工作。
//! 初始化全局的Pack文件索引数据。
bool        init_packinterface(const char * iniFile)
{
	guard;

	g_mapPackInfo			= new MapPackInfo;
	g_mapPackInfo_Update	= new MapPackInfo;

	START_GBUFFER;

	//! 初始化全局使用的临界区
	::InitializeCriticalSection( &g_sGbufSec );


	::InitializeCriticalSection( &g_sFileReadCS );

	//!　如果使用了我们的文件包索引文件，初始化打包的包文件。
	if( g_bUsePackSystem )
		return init_packinterface_pack( iniFile);

# if __PACKFILE_LOAD__
	// River @ 2011-2-28:用于一次性调入所有的xml文件
	init_xmlDataFileCatch();
# endif 


	return true;

	unguard;
}


/** \brief
 *  释放全局的pack文件索引资源。
 */
void        release_packinterface( void )
{
	guard;

	::DeleteCriticalSection( &g_sFileReadCS );

	//! 释放全局使用的临界区.
	::DeleteCriticalSection( &g_sGbufSec );

	END_GBUFFER;

	SAFE_DELETE(g_mapPackInfo);
	SAFE_DELETE(g_mapPackInfo_Update);

	if(g_bUsePackSystem){
        release_packinterface_pack();
	}

	unguard;
}
//@} 



/** 
*  得到文件长度的函数,可以是*.osk文件中的文件,也可以单独的文件.
*   
*  如果文件长度大于MAXDWORD,就会出错,因为程序无法使用那么大的值. 
*
*  /return  如果处理错误,返回-1,如果正确,返回文件长度,以byte为单位.
*  /param  _filename 要处理的文件的名字.
*/
int         get_fileSize( char* _filename )
{
	guard;// get_fileSize() );

	if( g_bUsePackSystem )
	{
		char FileNamePrefix[256];
		sprintf(FileNamePrefix,"%s\\%s",PackageRootFolder,_filename);
		return get_fileSize_pack( FileNamePrefix );
	}

	HANDLE hFindHandle;
	WIN32_FIND_DATA   t_f;
	int              t_iSize = -1;

	hFindHandle = ::FindFirstFile( _filename,&t_f );
	if( hFindHandle != INVALID_HANDLE_VALUE )
	{
		osassert( !t_f.nFileSizeHigh );

		t_iSize = t_f.nFileSizeLow;
	}
	else
	{
		osassertex( false, va( "get_fileSize,local,缺少文件:%s", _filename) );
	}

	//t_sec.LeaveCriticalSec();

	FindClose( hFindHandle );

	// 
	// 返回失败。
	return t_iSize;

	unguard;
}

/** \brief
 *  调入默认的相关文件。
 *
 *  根据文件的扩展名来调入。对于一些关键性的文件，仍然需要assert并退出。
 *  
 */
int  load_defaut_file( char* _filename,BYTE* _buf,int _bufsize )
{
	guard;

	s_string     t_str = _filename;
	int          t_idx;

	t_idx = (int)t_str.rfind( "." );
	t_str = t_str.substr( t_idx+1,t_str.length() );
 

	//osassert( false );
	osDebugOut( "load_defaut_file...文件<%s>找不到，调入默认文件\n", _filename );
	MessageBox( NULL,_filename,"文件找不到，调入默认文件",MB_OK );

	// 分别调入不同的类型的文件
	if( t_str == "hon" )
	{
		return read_fileToBuf( "scemesh\\default.hon",_buf,_bufsize );
	}
	if( t_str == "ex" )
	{
		return read_fileToBuf( "scemesh\\default.ex",_buf,_bufsize );
	}
	if( t_str == "par" )
	{
		return read_fileToBuf( "particle\\default.par",_buf,_bufsize );
	}
	if( t_str == "osa" )
	{
		return read_fileToBuf( "keyAni\\default.osa",_buf,_bufsize );
	}
	if( t_str == "bb" )
	{
		return read_fileToBuf( "efftexture\\default.bb",_buf,_bufsize );
	}
	if( t_str == "wav" )
	{
		return read_fileToBuf( "sound\\default.wav",_buf,_bufsize );
	}
	if( t_str == "mp3" )
	{
		return read_fileToBuf( "music\\default.mp3",_buf,_bufsize );
	}
	if( t_str == "spe" )
		return read_fileToBuf( "speff\\default.spe",_buf,_bufsize );

	char  t_sz[256];

	::_getcwd( t_sz,256 );

	FILE* t_file = fopen( _filename,"rb" );
	if( t_file )
	{
		osDebugOut( "Open file <%s> success...\n",_filename );
		fclose( t_file );
	}
	osassertex( false,va( "文件<%s>是关键文件，在目录<%s>找不到..ErrorCode<%d>...请重新下载最新的客户端修复错误",
		_filename,t_sz,::GetLastError() ) );

	return -1;

	unguard;
}

//! 从缓冲的内存中读取xml文件。
extern int read_catchXmlToBuf( char* _filename,BYTE* _buf,int _bufsize );


/** 
*  把文件读入到一个缓冲区中.
*
*  \return 如果成功,返回true.
*  \param _buf 文件要读入的缓冲区,已经分配足够内存.
*  \param _bufsize 如果文件长度大于_bufsize,返回false.
*/
int        read_fileToBuf( char* _filename,BYTE* _buf,int _bufsize,bool _forceDisk/* = false*/ )
{
	guard;

	if(g_bRecordFilename){
		record_filename(_filename);
	}

	// River @ 2011-2-28:底层处理，某一些类型的文件需要此处勾住调用。
	s_string   t_str = _filename;

# if __PACKFILE_LOAD__
	if( t_str.find( ".xml" ) != -1 )
	{
		int t_iSize = read_catchXmlToBuf( _filename,_buf,_bufsize );
		if( t_iSize > 0 )
			return t_iSize;
		// 找不到相应的xml,继续查找
	}
# endif 

	if( g_bUsePackSystem && (!_forceDisk) )
	{
		char FileNamePrefix[256];
		sprintf(FileNamePrefix,"%s\\%s",PackageRootFolder,_filename);
		return read_fileToBuf_pack( FileNamePrefix, _buf, _bufsize );
	}


	DWORD     t_filesize,t_dwResSize;


	// 如果文件不存在，则处理调入默认的相关文件
	if( (!file_exist( _filename )) && g_bUseDefaultFile )
	{
		return load_defaut_file( _filename,_buf,_bufsize );
	}


	t_filesize = (int)get_fileSize( _filename );
	osassert(!(t_filesize & 0x80000000));

	osassert(_bufsize > 0);
	if( ((DWORD)_bufsize< t_filesize)||(t_filesize<=0) )
	{
		osDebugOut( "File Size is Too BIG!!!!   \
					The File is :<%s>，大小<%d>..\n",_filename,t_filesize );
		osassertex( false,_filename );
		t_dwResSize = -1;
	}
	else
	{	

		HANDLE hFile = CreateFile(_filename,    // file to open
								GENERIC_READ,          // open for reading
								FILE_SHARE_READ,       // share for reading
								NULL,                  // default security
								OPEN_EXISTING,         // existing file only
								FILE_ATTRIBUTE_NORMAL, // normal file
								NULL);                 // no attr. template

		if( hFile == INVALID_HANDLE_VALUE )
		{
			char    t_str[256];
			char    t_dir[256];
			DWORD t_dwError = GetLastError();
			_getcwd( t_dir,256 );
			sprintf( t_str,"目录下缺少文件:<%s\\%s>error<%d>",t_dir,_filename,t_dwError );
			osDebugOut( t_str );

			FILE*  t_file = fopen( t_str,"rb" );
			if( t_file )
			{
				sprintf( t_str,"文件找不到，请重启电脑后再次尝试运行游戏。FOPEN file success" );
				fclose( t_file );
			}

			MessageBox( NULL,t_str,"ERROR",MB_OK );
			osassertex( false,t_str );

			t_dwResSize = -1;
		}
		else
		{

			ReadFile(hFile,_buf,t_filesize,&t_dwResSize,NULL);

		}
		::CloseHandle(hFile);
	}

	osassert(!(t_dwResSize & 0x80000000));
	return (int)t_dwResSize;

	unguard;
}

//! 
extern bool file_existInXmlCatch( char* _filename );

/** \brief
 *  查看一个文件是否存在.
 *  \param bool _forceNPack 强制不使用打包文件.
 */
bool        file_exist( const char* _filename,bool _forceNPack/* = false*/ )
{
	if(_filename == NULL){
		return FALSE;
	}

	if( g_bUsePackSystem && (!_forceNPack) )
	{
		char FileNamePrefix[256];
		sprintf(FileNamePrefix,"%s\\%s",PackageRootFolder,_filename);
		return file_exist_pack( FileNamePrefix );
	}


	// River @ 2011-2-28:底层处理，某一些类型的文件需要此处勾住调用。
	s_string   t_str = _filename;
	if( t_str.find( ".xml" ) != -1 )
	{
		bool t_b = file_existInXmlCatch( (char*)_filename );
		if( t_b )
			return true;
		// 找不到相应的xml,继续查找
	}


	HANDLE hFindHandle;
	WIN32_FIND_DATA   t_f;
	bool             t_bRes;

	if( g_bUsePackSystem )
		::EnterCriticalSection( &g_sFileReadCS );

	hFindHandle = ::FindFirstFile( _filename,&t_f );

	if( hFindHandle != INVALID_HANDLE_VALUE )
	{
		osassertex( (!t_f.nFileSizeHigh),_filename );
		t_bRes = true;
	}
	else
	{
		t_bRes = false;
		osassert( strcmp( _filename,"" ) != 0 );
		//osDebugOut( "1文件不存在:<%s>..\n",_filename );
	}

	if( g_bUsePackSystem )
		::LeaveCriticalSection( &g_sFileReadCS );

	// syq
	FindClose( hFindHandle );

	// 
	// 返回失败。
	return t_bRes;

}

/** 
 *  写文件到磁盘，为直写和包写的共用函数
 *
 *  \return 如果成功,返回文件长度,否则返回-1.
 *  \param _filename 文件名  如：sound\\mysnd.wav
 *  \param _buf      数据缓冲区
 *  \param _bufsize  数据大小(byte)
 */
int        write_file( char* _filename, BYTE* _buf,int _bufsize )
{
	if( g_bUsePackSystem )
		return write_file_pack( _filename, _buf, _bufsize );

	FILE * fp = NULL;
	if( !(fp=fopen( _filename, "wb" ) ) )
		return -1;
	int size = (int)fwrite( _buf,  1,_bufsize, fp );
	fclose(fp);
	return size;
}


static bool g_bUseGBuf = false;
static bool g_bUseGLBuf = false;

//! 返回可以使用的全局内存缓冲区.大小为TMP_BUFSIZE
FILEOPERATE_API    BYTE*        start_useGBuf( int& _bufIdx )
{
	guard;

	// 
	// River @ 2011-2-26:使用了两个全局的缓冲区，为了多个线程同时调入数据
	//                   的时候更加流畅一些。
tryagain:

	::EnterCriticalSection( &g_sGbufSec );
	if( !g_bUseGBuf )
	{
		osassert( ((DWORD*)g_tmpBuffer)[0] == NULL );
		g_bUseGBuf = true;
		_bufIdx = 0;
		::LeaveCriticalSection( &g_sGbufSec );	
		return g_tmpBuffer;
	}

	if( !g_bUseGLBuf )
	{
		osassert( ((DWORD*)g_tmpLittleBuffer)[0] == 0 );
		g_bUseGLBuf = true;
		_bufIdx = 1;
		::LeaveCriticalSection( &g_sGbufSec );	
		return g_tmpLittleBuffer;
	}
	::LeaveCriticalSection( &g_sGbufSec );

	// River @ 2011-2-26:如果运行到此，则找不到可用的bug,等待直到有可用的buf.
	::Sleep( 1 );
	goto tryagain;

	unguard;
}

//! 结束使用全局的内存缓冲区.
FILEOPERATE_API    void         end_useGBuf( int& _bufIdx )
{
	if( _bufIdx < 0 )
		return;

	::EnterCriticalSection( &g_sGbufSec );
	if( _bufIdx == 0 )
	{
		((DWORD*)g_tmpBuffer)[0] = 0;
		g_bUseGBuf = false;
		_bufIdx = -1;
		::LeaveCriticalSection( &g_sGbufSec );	
		return;
	}
	((DWORD*)g_tmpLittleBuffer)[0] = 0;
	g_bUseGLBuf = false;
	_bufIdx = -1;
	::LeaveCriticalSection( &g_sGbufSec );
	return;

}

