//=======================================================================================================
/**  \file
 *   Filename: xmlPreLoader.h
 *   Desc:     ������ϷĿ¼�µĴ���ļ���������ؽڵ��ļ�������Ϊ���Ż�Qtang�������޸ġ�
 //            �µĹ��̿���ȥ�����ļ���
 *
 *   His:      River created @ 2011-2-28
 *
 *   "�û������Ż�û�о�ͷ"
 *  
 */
//=======================================================================================================

#include "stdafx.h"

# include "../interface/fileOperate.h"

# include "packFileOperate.h"
# include "direct.h"


//! dataĿ¼��xml�ļ��ĺϼ�
# define DATAXML_FILELIST   "data\\xml.list"

# include "../interface/osInterface.h"


//! ����hashֵ��Ϊ�˴���"\\"��"/".
struct s_xmlFileHash
{
	SFileBuf   m_sFileBuf;
	BYTE*      m_ptrFileData;

	DWORD      m_dwHash1;
	DWORD      m_dwHash2;

	BOOL       m_bLoaded;

	s_xmlFileHash() 
	{
		m_ptrFileData = NULL;
		m_dwHash1 = 0;
		m_dwHash2 = 0;
		m_bLoaded = FALSE;
	}
};

static BYTE*            g_ptrTmpXmlDataBuf = NULL;
static s_xmlFileHash*   g_ptrXMLSFileBuf = NULL;
static SFileHead        g_sXMLHeader;

bool init_xmlDataFileCatch( void )
{
	guard;

	int  t_iGBufIdx = -1;
	int  t_iFileSize = get_fileSize( DATAXML_FILELIST ) ; 
	g_ptrTmpXmlDataBuf = new BYTE[t_iFileSize];
	osassertex( g_ptrTmpXmlDataBuf,"Initialize memory failed...\n" );

	BYTE* t_fstart = g_ptrTmpXmlDataBuf;
	DWORD t_size;
	t_size = read_fileToBuf( DATAXML_FILELIST,t_fstart,t_iFileSize );
	osassert( t_size>=0 );

	READ_MEM_OFF( &g_sXMLHeader,t_fstart,sizeof( SFileHead ) );

	//! �����������������Ҫ�ĸ�ʽ.
	g_ptrXMLSFileBuf = new s_xmlFileHash[g_sXMLHeader.dwFileNum];
	s_string   t_strfname;
	osDebugOut( "The filesize is<%d>,fileNum is:<%d>...\n",
		t_iFileSize,g_sXMLHeader.dwFileNum );

	for( int t_i = 0;t_i< (int)g_sXMLHeader.dwFileNum;t_i ++ )
	{
		READ_MEM_OFF( &g_ptrXMLSFileBuf[t_i].m_sFileBuf,t_fstart,sizeof( SFileBuf ) );
		g_ptrXMLSFileBuf[t_i].m_ptrFileData = t_fstart;

		osDebugOut( "The <%d><%s> file length is:<%d>...",
			t_i,
			g_ptrXMLSFileBuf[t_i].m_sFileBuf.szFileName,
			g_ptrXMLSFileBuf[t_i].m_sFileBuf.dwBufLenth );
		
		// build hash Value.
		t_strfname = "data\\";
		t_strfname += g_ptrXMLSFileBuf[t_i].m_sFileBuf.szFileName;
		std::transform( t_strfname.begin(),t_strfname.end(),t_strfname.begin(),tolower );
		g_ptrXMLSFileBuf[t_i].m_dwHash1 = string_hash( t_strfname.c_str() );

		t_strfname = "data/";
		t_strfname += g_ptrXMLSFileBuf[t_i].m_sFileBuf.szFileName;
		std::transform( t_strfname.begin(),t_strfname.end(),t_strfname.begin(),tolower );
		g_ptrXMLSFileBuf[t_i].m_dwHash2 = string_hash( t_strfname.c_str() );

		t_fstart += g_ptrXMLSFileBuf[t_i].m_sFileBuf.dwBufLenth;	
	}

	return true;

	unguard;
}

void release_xmlDataFileCatch( void )
{
	guard;

	SAFE_DELETE_ARRAY( g_ptrTmpXmlDataBuf );
	SAFE_DELETE_ARRAY( g_ptrXMLSFileBuf );

	unguard;
}

static BOOL t_bInFuncRelease = FALSE;


bool file_existInXmlCatch( char* _filename )
{
	guard;

	DWORD       t_dwHash;
	s_string    t_strfname = _filename;

	// 
	// ��ĳЩ����£����µ���������ļ�
	if( g_ptrTmpXmlDataBuf == NULL )
	{
		if( (t_strfname.find( "data\\" ) != -1) ||
			(t_strfname.find( "data/") != -1 ) )
		{
			// ���ж��ļ��Ƿ�����Ӳ���ϡ�
			WIN32_FIND_DATA   t_f;
			bool             t_bRes;
			          
			HANDLE  hFindHandle = ::FindFirstFile( _filename,&t_f );
			if( hFindHandle != INVALID_HANDLE_VALUE )
			{
				osassertex( (!t_f.nFileSizeHigh),_filename );
				t_bRes = true;
			}
			else
				t_bRes = false;
			FindClose( hFindHandle );

			if( !t_bRes )
			{
				init_xmlDataFileCatch();
				t_bInFuncRelease = true;
			}
		}
	}

	// �˴����ܷ���0.
	if( !g_ptrTmpXmlDataBuf )
		return false;

	std::transform( t_strfname.begin(),t_strfname.end(),t_strfname.begin(),tolower );
	t_dwHash = string_hash( t_strfname.c_str() );

	for( int t_i =0;t_i<(int)g_sXMLHeader.dwFileNum;t_i ++ )
	{
		if( (t_dwHash == g_ptrXMLSFileBuf[t_i].m_dwHash1) ||
			(t_dwHash == g_ptrXMLSFileBuf[t_i].m_dwHash2) )
		{
			return true;
		}
	}

	return false;

	unguard;
}

//! �ӻ�����ڴ��ж�ȡxml�ļ���
int  read_catchXmlToBuf( char* _filename,BYTE* _buf,int _bufsize )
{
	guard;

	DWORD       t_dwHash;
	s_string    t_strfname = _filename;

	// 
	// ��ĳЩ����£����µ���������ļ�
	if( g_ptrTmpXmlDataBuf == NULL )
	{
		if( (t_strfname.find( "data\\" ) != -1) ||
			(t_strfname.find( "data/") != -1 ) )
		{
			if( !file_exist( _filename ) )
			{
				init_xmlDataFileCatch();
				t_bInFuncRelease = true;
			}
		}
	}

	// �˴����ܷ���0.
	if( !g_ptrTmpXmlDataBuf )
		return 0;

	std::transform( t_strfname.begin(),t_strfname.end(),t_strfname.begin(),tolower );
	t_dwHash = string_hash( t_strfname.c_str() );

	for( int t_i =0;t_i<(int)g_sXMLHeader.dwFileNum;t_i ++ )
	{
		if( (t_dwHash == g_ptrXMLSFileBuf[t_i].m_dwHash1) ||
			(t_dwHash == g_ptrXMLSFileBuf[t_i].m_dwHash2) )
		{
			osassert( (int)g_ptrXMLSFileBuf[t_i].m_sFileBuf.dwBufLenth < _bufsize );
			
			memcpy( _buf,g_ptrXMLSFileBuf[t_i].m_ptrFileData,
				g_ptrXMLSFileBuf[t_i].m_sFileBuf.dwBufLenth );

			g_ptrXMLSFileBuf[t_i].m_bLoaded = TRUE;

			int   t_iFileSize = g_ptrXMLSFileBuf[t_i].m_sFileBuf.dwBufLenth;
			if( strcmp( g_ptrXMLSFileBuf[t_i].m_sFileBuf.szFileName,"task.xml" ) == 0 )
				release_xmlDataFileCatch();

			if( t_bInFuncRelease )
			{
				t_bInFuncRelease = false; 
				release_xmlDataFileCatch();
			}

			return t_iFileSize;
		}
	}

	if( t_bInFuncRelease )
		release_xmlDataFileCatch();

	return 0;

	unguard;
}
