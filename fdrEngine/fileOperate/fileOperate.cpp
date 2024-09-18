//=======================================================================================================
/**  \file
 *   Filename: fileOperate.cpp
 *   Desc:     Osok�����в����ļ��İ�����������.
 *
 *   His:      River created @ 4/27 2003.
 *
 *   "��ס��Ҫ�������µ��˵ĳɳ�����˾�ĳɳ�ȡ�����ǲ���ÿһ���˶��ڳɳ���
 *    �����˾�ڲ�ÿ���˶��ڳɳ�����ô�����˾���ڳɳ���"
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
//! ȫ���Ƿ�ʹ�ô���ļ��Ŀ��ء���ʼ��Ϊ��ʹ�á�
FILEOPERATE_API    BOOL       g_bUsePackSystem = FALSE;

//! �Ƿ����ʹ������ļ���
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



// TEST CODE: ���ļ���Ҫʹ�ü����???
CRITICAL_SECTION    g_sFileReadCS;



/** \brief
 *  ȫ���ԵĻ��������ݱ���.
 *
 *  ���ڵ����ļ�������������;��һ����ʱ������.��СΪ4M.
 *  �ڵ����ļ���ʱ��Ϊ��������������ڴ�,ʹ����֮��ɾȥ
 *  ��Щ�ڴ�,��ʹ���Լ������ɾȥ���ڴ�.
 */
static BYTE*       g_tmpBuffer = NULL;

//! СһЩ��ȫ�������ݻ�����,ֻ�������߳�ʹ��.
static BYTE*       g_tmpLittleBuffer = NULL;

//@{
/**
 *  ʹ����������ʹ��ȫ�ֵ����ݻ�����.
 */
# define   START_GBUFFER      g_tmpBuffer = new BYTE[TMP_BUFSIZE]; \
							  ((DWORD*)g_tmpBuffer)[0] = 0;((DWORD*)g_tmpBuffer)[1]= 0;\
	                        g_tmpLittleBuffer = new BYTE[TMP_BUFSIZE]; \
							  ((DWORD*)g_tmpLittleBuffer)[0] = 0;((DWORD*)g_tmpLittleBuffer)[1]= 0;


# define   END_GBUFFER      SAFE_DELETE_ARRAY( g_tmpBuffer );\
	                       SAFE_DELETE_ARRAY( g_tmpLittleBuffer );
//@}




//! �Ƿ���ʹ��GBufferʱ��������Ϣ��
FILEOPERATE_API BOOL                     g_bExportInfoUseGBuf = FALSE;

//! gbuffer�õ��Ķ��̰߳�ȫ������.
static CRITICAL_SECTION                  g_sGbufSec;
  
//! һ���Ե������е�xml��ʩ��
extern bool init_xmlDataFileCatch( void );

//@{ 
//  �����ʹ��pack�ļ������µ�������������Ҫ������
//! ��ʼ��ȫ�ֵ�Pack�ļ��������ݡ�
bool        init_packinterface(const char * iniFile)
{
	guard;

	g_mapPackInfo			= new MapPackInfo;
	g_mapPackInfo_Update	= new MapPackInfo;

	START_GBUFFER;

	//! ��ʼ��ȫ��ʹ�õ��ٽ���
	::InitializeCriticalSection( &g_sGbufSec );


	::InitializeCriticalSection( &g_sFileReadCS );

	//!�����ʹ�������ǵ��ļ��������ļ�����ʼ������İ��ļ���
	if( g_bUsePackSystem )
		return init_packinterface_pack( iniFile);

# if __PACKFILE_LOAD__
	// River @ 2011-2-28:����һ���Ե������е�xml�ļ�
	init_xmlDataFileCatch();
# endif 


	return true;

	unguard;
}


/** \brief
 *  �ͷ�ȫ�ֵ�pack�ļ�������Դ��
 */
void        release_packinterface( void )
{
	guard;

	::DeleteCriticalSection( &g_sFileReadCS );

	//! �ͷ�ȫ��ʹ�õ��ٽ���.
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
*  �õ��ļ����ȵĺ���,������*.osk�ļ��е��ļ�,Ҳ���Ե������ļ�.
*   
*  ����ļ����ȴ���MAXDWORD,�ͻ����,��Ϊ�����޷�ʹ����ô���ֵ. 
*
*  /return  ����������,����-1,�����ȷ,�����ļ�����,��byteΪ��λ.
*  /param  _filename Ҫ������ļ�������.
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
		osassertex( false, va( "get_fileSize,local,ȱ���ļ�:%s", _filename) );
	}

	//t_sec.LeaveCriticalSec();

	FindClose( hFindHandle );

	// 
	// ����ʧ�ܡ�
	return t_iSize;

	unguard;
}

/** \brief
 *  ����Ĭ�ϵ�����ļ���
 *
 *  �����ļ�����չ�������롣����һЩ�ؼ��Ե��ļ�����Ȼ��Ҫassert���˳���
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
	osDebugOut( "load_defaut_file...�ļ�<%s>�Ҳ���������Ĭ���ļ�\n", _filename );
	MessageBox( NULL,_filename,"�ļ��Ҳ���������Ĭ���ļ�",MB_OK );

	// �ֱ���벻ͬ�����͵��ļ�
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
	osassertex( false,va( "�ļ�<%s>�ǹؼ��ļ�����Ŀ¼<%s>�Ҳ���..ErrorCode<%d>...�������������µĿͻ����޸�����",
		_filename,t_sz,::GetLastError() ) );

	return -1;

	unguard;
}

//! �ӻ�����ڴ��ж�ȡxml�ļ���
extern int read_catchXmlToBuf( char* _filename,BYTE* _buf,int _bufsize );


/** 
*  ���ļ����뵽һ����������.
*
*  \return ����ɹ�,����true.
*  \param _buf �ļ�Ҫ����Ļ�����,�Ѿ������㹻�ڴ�.
*  \param _bufsize ����ļ����ȴ���_bufsize,����false.
*/
int        read_fileToBuf( char* _filename,BYTE* _buf,int _bufsize,bool _forceDisk/* = false*/ )
{
	guard;

	if(g_bRecordFilename){
		record_filename(_filename);
	}

	// River @ 2011-2-28:�ײ㴦��ĳһЩ���͵��ļ���Ҫ�˴���ס���á�
	s_string   t_str = _filename;

# if __PACKFILE_LOAD__
	if( t_str.find( ".xml" ) != -1 )
	{
		int t_iSize = read_catchXmlToBuf( _filename,_buf,_bufsize );
		if( t_iSize > 0 )
			return t_iSize;
		// �Ҳ�����Ӧ��xml,��������
	}
# endif 

	if( g_bUsePackSystem && (!_forceDisk) )
	{
		char FileNamePrefix[256];
		sprintf(FileNamePrefix,"%s\\%s",PackageRootFolder,_filename);
		return read_fileToBuf_pack( FileNamePrefix, _buf, _bufsize );
	}


	DWORD     t_filesize,t_dwResSize;


	// ����ļ������ڣ��������Ĭ�ϵ�����ļ�
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
					The File is :<%s>����С<%d>..\n",_filename,t_filesize );
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
			sprintf( t_str,"Ŀ¼��ȱ���ļ�:<%s\\%s>error<%d>",t_dir,_filename,t_dwError );
			osDebugOut( t_str );

			FILE*  t_file = fopen( t_str,"rb" );
			if( t_file )
			{
				sprintf( t_str,"�ļ��Ҳ��������������Ժ��ٴγ���������Ϸ��FOPEN file success" );
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
 *  �鿴һ���ļ��Ƿ����.
 *  \param bool _forceNPack ǿ�Ʋ�ʹ�ô���ļ�.
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


	// River @ 2011-2-28:�ײ㴦��ĳһЩ���͵��ļ���Ҫ�˴���ס���á�
	s_string   t_str = _filename;
	if( t_str.find( ".xml" ) != -1 )
	{
		bool t_b = file_existInXmlCatch( (char*)_filename );
		if( t_b )
			return true;
		// �Ҳ�����Ӧ��xml,��������
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
		//osDebugOut( "1�ļ�������:<%s>..\n",_filename );
	}

	if( g_bUsePackSystem )
		::LeaveCriticalSection( &g_sFileReadCS );

	// syq
	FindClose( hFindHandle );

	// 
	// ����ʧ�ܡ�
	return t_bRes;

}

/** 
 *  д�ļ������̣�Ϊֱд�Ͱ�д�Ĺ��ú���
 *
 *  \return ����ɹ�,�����ļ�����,���򷵻�-1.
 *  \param _filename �ļ���  �磺sound\\mysnd.wav
 *  \param _buf      ���ݻ�����
 *  \param _bufsize  ���ݴ�С(byte)
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

//! ���ؿ���ʹ�õ�ȫ���ڴ滺����.��СΪTMP_BUFSIZE
FILEOPERATE_API    BYTE*        start_useGBuf( int& _bufIdx )
{
	guard;

	// 
	// River @ 2011-2-26:ʹ��������ȫ�ֵĻ�������Ϊ�˶���߳�ͬʱ��������
	//                   ��ʱ���������һЩ��
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

	// River @ 2011-2-26:������е��ˣ����Ҳ������õ�bug,�ȴ�ֱ���п��õ�buf.
	::Sleep( 1 );
	goto tryagain;

	unguard;
}

//! ����ʹ��ȫ�ֵ��ڴ滺����.
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

