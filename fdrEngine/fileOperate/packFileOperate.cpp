//=======================================================================================================
/**  \file
 *   Filename: packFileOperate.cpp
 *   Desc:     Osok�����в����ļ��İ�����������.
 *
 *   His:      River created @ 4/27 2003.
 *
 *   "����Բ���ʧ��!!!"
 *  
 */
//=======================================================================================================

#include "StdAfx.h"

#include "packFileOperate.h"
#include "md5.h"

//#if PACK_NEW
#include "../interface/getfs.h"

# ifdef _DEBUG
   #pragma comment( lib, "../../lib/getfs.lib" )
   #pragma comment( lib, "../../lib/zlib.lib" )
# else
   #pragma comment( lib, "../../lib/getfsR.lib" )
   #pragma comment( lib, "../../lib/zlibR.lib" )
# endif 

getfs_Handle*       fs = 0;
//#endif
#define  USE_STATIC_STR

#define ERROR_FILEINFO_ORIGINALSIZE "�ڴ˳��������ļ���Ϣ��ԭʼ�ļ���С�洢������ĳ����ǿ���޸ģ���\n����uncompressʱ���������������ɣ�"

//�ļ���С������4M
#define MAX_FILESIZE (0x400000)

static CRITICAL_SECTION  g_csRFToBuf;

// River @ 2010-5-17:md5ת����Ҫ��cs.
static CRITICAL_SECTION  g_csMD5;


//ʹ��hash�ļ���
#define ENGINE_USE_HASH_FILENAME  1

MD5_CTX m_md5;
////  ELF Hash Function 
//unsigned  int  ELFHash( char   * str)
//{
//	unsigned  int  hash  =   0 ;
//	unsigned  int  x     =   0 ;
//
//	while  ( * str)
//	{
//		hash  =  (hash  <<   4 )  +  ( * str ++ );
//		if  ((x  =  hash  &   0xF0000000L )  !=   0 )
//		{
//			hash  ^=  (x  >>   24 );
//			hash  &=   ~ x;
//		} 
//	} 
//
//	return  (hash  &   0x7FFFFFFF );
//}

//�ֽڵ�ʮ�����Ƶ�ASCII��ת��
static std::string byteHEX( unsigned char byte0 )
{
	guard;

    char ac[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
        'A', 'B', 'C', 'D', 'E', 'F'
    };
    char ac1[3];
    ac1[0] = ac[(byte0 >> 4) & 0xf];
    ac1[1] = ac[byte0 & 0xf];
	ac1[2] = 0;

    return std::string(ac1);

	unguard;
}

std::string getMd5Str( const char* _s )
{
	guard;

	std::string hashstr;
	m_md5.MD5Update( (unsigned char*)_s, strlen(_s) );
	unsigned char digest[16] = {0};
	m_md5.MD5Final( digest );
	for( int i=0;i<16;i++ )		{
		hashstr += byteHEX( digest[i] );
	}
	hashstr += "._";
	return hashstr;

	unguard;
}
void getMd5Str( const char* _s,char* _out )
{
	guard;

	std::string hashstr;
	m_md5.MD5Update( (unsigned char*)_s, strlen(_s) );
	unsigned char digest[16] = {0};
	m_md5.MD5Final( digest );
	for( int i=0;i<16;i++ )		{
		hashstr += byteHEX( digest[i] );
	}
	hashstr += "._";
	strcpy(_out,hashstr.c_str());
	return;

	unguard;
}
//------------------------------------------------------------------------





//------------------------------------------------------------------------



//-------------------------------------------------------------------
// vector����׼��
//-------------------------------------------------------------------

// FreeInfo vector ����������С���������׼��
bool lessSortSize( const FreeInfo& f1, const FreeInfo& f2 )
{
	return f1.freeSize < f2.freeSize;
}

// FreeInfo vector ����������ַ���������׼��
bool lessSortAddr( const FreeInfo& f1, const FreeInfo& f2 )
{
	return f1.freeAddr < f2.freeAddr;
}
//-------------------------------------------------------------------



//-------------------------------------------------------------------
// Pack
//-------------------------------------------------------------------
Pack::Pack()
{ 
	ClearUp();	
	m_sPackHeader.SizeType = ST_DOT5K;
	m_pCompressWorkBuffer = new BYTE[COMPRESS_BUFFER_SIZE];
	m_pOriginalWorkBuffer = new BYTE[ORIGINAL_BUFFER_SIZE];
	m_CompressWorkBufferSize = COMPRESS_BUFFER_SIZE;
	m_OriginalWorkBufferSize = ORIGINAL_BUFFER_SIZE;
	m_CurrCount = 0;
	m_TotalCount = 0;
}

//-------------------------------------------------------------------
Pack::~Pack()
{
	delete 	[]	m_pCompressWorkBuffer ;
	m_pCompressWorkBuffer = NULL;
	delete  []  m_pOriginalWorkBuffer ;
	m_pOriginalWorkBuffer = NULL;
}

/**\brief
*
*	�½�����д�����ͷ�ṹ��
*
*	\param _chpName  �½��������ơ�
*	\return bool     ����true��ʾ�����ɹ���������false��
*
*/
bool Pack::NewPack( char * _chpName ) 
{
	guard;
	m_strPackName = _chpName;

	m_hFile = ::CreateFile(_chpName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	if( m_hFile == INVALID_HANDLE_VALUE )
	{
	    return false;
	}

	// write header
	memcpy( m_sPackHeader.cFileFlag, HEADER_FLAG, strlen(HEADER_FLAG) );

	m_sPackHeader.validateCode = timeGetTime();//syq

	WriteBound(&m_sPackHeader,0,sizeof(PackHeader),m_hFile);
	SetDataEndAddr(sizeof(PackHeader));

	return true;
	unguard;
}

/**\brief
*
*	���ļ���������Ϊ�ɶ���д
*
* \param   _filename   �ļ���
*
*/
void Pack::SetFileAttr_ReadWrite( char * _filename )
{
	guard;
	DWORD FLAG = GetFileAttributes( _filename );
	FLAG &= ~FILE_ATTRIBUTE_READONLY;
	FLAG &= ~FILE_ATTRIBUTE_HIDDEN;
	SetFileAttributes(_filename,FLAG );
	unguard;
}


/**\brief
*
*	�򿪰��ļ���
*
*	\param _chpName       Ҫ�򿪵İ��ļ������ơ�
*	\return bool          ����true��ʾ�򿪳ɹ���������false��
*
*	˵�������ļ�������ļ���ʽ��־��
*
*/
bool Pack::OpenPack( char *_chpName )
{
	guard;
	char b[MAX_PATH] = {0};

	if( m_hFile != NULL )
	{
		MessageBox( NULL,"���Ѿ��򿪣��˴β�����Ч��","��ʾ", MB_OK|MB_ICONWARNING );
		return false;
	}
	m_strPackName = _chpName;

	std::string infoFile( _chpName );
	if( infoFile.length() > 0 )
		*( infoFile.end() - 1 ) = 'i';
	
	//check pack file
	if( !IsFileExist(_chpName) )
	{
		MessageBox(NULL,"���ļ�������","��ʾ",MB_OK);
		return false;
	}

	//check info file
	if( !IsFileExist((char*)infoFile.c_str() ) )
	{
		sprintf(b,"%s �ļ�������",infoFile.c_str() );
		MessageBox(NULL,b,"��ʾ",MB_OK);
		return false;
	}

	//open info file...
	SetFileAttr_ReadWrite( (char*)infoFile.c_str() );
	HANDLE handle =::CreateFile( infoFile.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if( handle == INVALID_HANDLE_VALUE )
	{
		sprintf( b,"���ļ�%sʧ��!",infoFile.c_str() );
		MessageBox(NULL,b,"��ʾ",MB_OK);
		return false;
	}

	int filesize = get_diskFileSize( (char*)infoFile.c_str() );
	osassert( filesize > 0 );

	DWORD curAddr = 0;
	InfoHeader ih;
	ReadBound( &ih, curAddr, sizeof( InfoHeader ), handle );
	curAddr += sizeof(InfoHeader);
	if( strcmp( ih.cFileFlag, INFOFILE_HEADER_FLAG ) != 0 )
	{
		CloseHandle( handle );
		sprintf( b,"�ļ�%s������Ч��ʽ!", infoFile.c_str() );
		MessageBox(NULL,b,"��ʾ",MB_OK);
		return false;
	}

	// �򿪰��ļ�
	SetFileAttr_ReadWrite( _chpName );
	Sleep( 50 );
	m_hFile=::CreateFile( _chpName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		sprintf( b,"���ļ��ļ�%sʧ��!", _chpName );
		MessageBox( NULL, b, "��ʾ", MB_OK );
		CloseHandle( handle );
		m_hFile = NULL;
		return false;
	}

	// ����Ƿ�Ϊ��Ч�ļ�
	ReadBound( &m_sPackHeader, 0, sizeof( PackHeader ), m_hFile );
	if( strcmp( m_sPackHeader.cFileFlag, HEADER_FLAG ) != 0 )
	{
		CloseHandle( handle );
		CloseHandle( m_hFile );
		m_hFile = NULL;
		MessageBox( NULL,"�򿪵Ĳ���(*.llp)�ļ���","��ʾ",MB_OK|MB_ICONWARNING );
		return false;
	}

	// ���ļ���֤
	if( ih.validateCode != m_sPackHeader.validateCode )
	{
		CloseHandle( handle );
		CloseHandle( m_hFile );
		m_hFile = NULL;
		sprintf( b, "%s, llp��lli�ļ���ƥ��!", _chpName );
		MessageBox( NULL, b, "����", MB_OK | MB_ICONWARNING );
		return false;
	}
	
	//װ����Ϣ
	DWORD i;
	FileInfo fi;
	std::string s;
	for( i=0; i<ih.iInfoCount;++i )
	{
		ReadBound( &fi, curAddr, sizeof( FileInfo ), handle );
		s = fi.m_chName;
		m_mapInfo.insert( std::make_pair( s, fi ) );
		curAddr += sizeof( FileInfo );
		osassert( curAddr <= filesize );
	}
	FreeInfo free;
	for( i=0;i<ih.iFreeCount;++i )
	{
		ReadBound( &free, curAddr, sizeof( FreeInfo ),  handle );
		m_listFreeInfo.push_back( free );
		curAddr += sizeof( FreeInfo );
		osassert( curAddr <= filesize );
	}
	CloseHandle( handle );
	return true;
	unguard;
}

/**\brief
*
*	�ر�һ�����ļ���
*
*	�رղ��洢 pack file & Info file
*
*/
void Pack::ClosePack()
{
	guard;

#if PACK_USE_IN_ENGINE

	osDebugOut( "pack:closePack:PACK_USE_IN_ENGINE\n" );
	CloseHandle( m_hFile );

#else

	osDebugOut( "pack:closePack: packtool write... \n" );
	WriteFileInfoAndFreeInfoToFile();

	//��д�ļ�ͷ
	WriteBound( &m_sPackHeader, 0, sizeof(PackHeader), m_hFile );

	//дβ��־,ʵ�ʳ���
	WriteBound( END_FLAG, GetDataEndAddr(), strlen(END_FLAG), m_hFile );

	SetEndOfFile(m_hFile);

	// �ر��ļ���
	CloseHandle(m_hFile);

	m_hFile = NULL;

	ClearUp();

#endif

	unguard;
}

/**\brief
*
*	��ճ�Ա�����ļ���� 
*
*/
void Pack::ClearUp()
{
	guard;
	ZeroMemory( &m_sPackHeader,sizeof(PackHeader) ); 
	m_sPackHeader.SizeType = ST_DOT5K;
	m_listFreeInfo.clear();
	m_mapInfo.clear();
	m_strPackName = "";
	m_CompressWorkBufferSize = COMPRESS_BUFFER_SIZE;
	m_OriginalWorkBufferSize = ORIGINAL_BUFFER_SIZE;
	m_hFile = NULL;
	unguard;
}

/**\brief
*
*	д���ļ���Ϣ�����е���Ϣ������Ϊpack���ֵ�����һ���ַ���Ϊ'i'
*
* \return bool     ����trueָʾ�ɹ�������Ϊʧ��
*
*/	
bool Pack::WriteFileInfoAndFreeInfoToFile()
{
	guard;
	std::string s = m_strPackName;
	*(s.end()-1) = 'i';

	HANDLE handle=::CreateFile( s.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
	if( handle == INVALID_HANDLE_VALUE )
	{
		osDebugOut( "�����ļ�%sʧ��!\n", s.c_str() );
		osassert( false );
	    return false;
	}
	
	DWORD curAddr=0;
	InfoHeader ih;
	memset( &ih,0,sizeof( InfoHeader ) );
	memcpy( ih.cFileFlag, INFOFILE_HEADER_FLAG, FLAG_SIZE );
	ih.iInfoCount = m_mapInfo.size();
	ih.iFreeCount = m_listFreeInfo.size();
	
	ih.validateCode = m_sPackHeader.validateCode; //syq

	WriteBound( &ih, 0, sizeof(InfoHeader), handle );
	curAddr = sizeof( InfoHeader );

	FileInfo fi;
	MapInfo::iterator pos;
	for( pos=m_mapInfo.begin(); pos!=m_mapInfo.end(); ++pos )
	{	
		fi = pos->second;
		WriteBound( &fi, curAddr, sizeof(FileInfo), handle );
		curAddr += sizeof( FileInfo );
	}

	// combine neighbor FreeInfo
	FreeInfo d;
    ListFreeInfo v2;
	ListFreeInfo::iterator it,it2;
	std::sort(m_listFreeInfo.begin(),m_listFreeInfo.end(),lessSortAddr);
	for( it=m_listFreeInfo.begin(); it!=m_listFreeInfo.end(); ++it )
	{
		it2 = it;
		it2 ++;
		d.freeSize = it->freeSize;
		d.freeAddr = it->freeAddr;
		for(;;)
		{
			if( it->freeAddr+it->freeSize == it2->freeAddr )
			{
				d.freeSize += it2->freeSize;
				it ++;
				it2 ++;
				continue;
			}
			else
			{
				break;
			}
		}
		v2.push_back(d);
	}

	// write to file
	for( it=v2.begin(); it!=v2.end(); ++it )
	{
		d=*it;
		WriteBound( &d, curAddr, sizeof(FreeInfo), handle );
		curAddr += sizeof( FreeInfo );
	}

	// rewrite header
	memset( &ih,0,sizeof( InfoHeader ) );
	memcpy( ih.cFileFlag, INFOFILE_HEADER_FLAG, FLAG_SIZE );
	ih.iInfoCount = m_mapInfo.size();
	ih.iFreeCount = v2.size();

	ih.validateCode = m_sPackHeader.validateCode; //syq

	WriteBound( &ih, 0, sizeof(InfoHeader), handle );
	

	m_mapInfo.clear();
	m_listFreeInfo.clear();
	v2.clear();

	CloseHandle( handle );
	return true;
	unguard;
}

/**\brief
*
*	����ļ���д��һ���ļ���
*
*	\param  _filename  Ҫ��������ӵ��ļ��������̵ľ���·��
*   \param  _PackPath  ���ڵ����·��,��Ϊ"",��Ϊ����Ŀ¼
*
*	˵����ѹ���ļ���д�����.
*
*/
bool Pack::AddFile( char * _PackPath, char * _filename )
{
	guard;

	//Ҫ���ӵ��ļ��Ƿ����?
	if( !IsFileExist(_filename) )
	{
		MessageBox( NULL,"��Ҫ���ӵ��ļ������ڣ�","��ʾ",MB_OK );
		return false;
	}

	//��ָ�����ļ�
	HANDLE handle = NULL;
	SetFileAttr_ReadWrite( (char*)_filename );
	handle=::CreateFile(_filename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
	if( handle == INVALID_HANDLE_VALUE )
		return false;
	
	char* name;
	DWORD filesize= 0;
	char path[MAX_PATH]={0};
	char buffer[MAX_PATH]={0};
	filesize = GetFileSize(handle,NULL);
	::GetFullPathName(_filename, MAX_PATH, buffer, &name );
	MakePath( path, _PackPath, name );//����·��

	//��Ŀ¼�ļ�����ΪСд
	std::string str( path );
	std::transform( str.begin(), str.end(), str.begin(), tolower );

	//ѹ�����õ���С, ������
	DWORD dwOutSize=(DWORD)( (float)filesize * 1.01f + 12 );

	// �����ļ�������
	BYTE * pFileBuffer = new BYTE[filesize];
	memset( pFileBuffer, 0, filesize );

	BYTE * pCompressBuffer = new BYTE[dwOutSize];
	memset( pCompressBuffer,0, dwOutSize );

	// ��������
	ReadBound( pFileBuffer, 0, filesize, handle );

	// ѹ������
	if( Z_MEM_ERROR==compress( pCompressBuffer, &dwOutSize, pFileBuffer, filesize ) )
	{
		::MessageBox(NULL,"AddFile����������������㣡","����",MB_OK);
		return false;
	}

    AddFile( (char*)str.c_str(), pCompressBuffer, dwOutSize , filesize );

	//free
	delete [] pCompressBuffer;
    delete [] pFileBuffer;
	CloseHandle(handle);

	return true;
	unguard;
}

bool Pack::AddFile( char * _PackFileName, VOID * _pCompressData,
				   DWORD _dwCompressSize , DWORD _dwOriginalSize )
{
	guard;
	//char * path = _PackFileName;

	// syq @ 05.3.7
	//��Ŀ¼�ļ�����ΪСд
	std::string str( _PackFileName );
	std::transform( str.begin(), str.end(), str.begin(), tolower );
	const char * path = str.c_str();

	DWORD dwOutSize = _dwCompressSize;
	DWORD filesize = _dwOriginalSize;
	
	//���ļ���Ϣ
	DWORD i=0;
	BYTE * p = (BYTE*)_pCompressData;
	DWORD main_count = dwOutSize / m_sPackHeader.SizeType;
	DWORD remain_len = dwOutSize - main_count * m_sPackHeader.SizeType;

	//�Ƿ��п���������
	DWORD FreeAddr = 0,FreeSize = 0;
	ListFreeInfo::iterator pos;
	if( !m_listFreeInfo.empty() )
	{
		std::sort( m_listFreeInfo.begin(), m_listFreeInfo.end(), lessSortSize );
		for( pos = m_listFreeInfo.begin(); pos != m_listFreeInfo.end(); ++pos )
		{
			if( pos->freeSize >= dwOutSize )
			{
				FreeAddr = pos->freeAddr;
				FreeSize = pos->freeSize;
				break;
			}
		}
	}

	//д��
	FileInfo fi;
	memset( &fi, 0, sizeof( FileInfo ) );
	DWORD size=0,Addr=0;
    FreeInfo free;
	MapInfo::iterator it = m_mapInfo.find( path );
	if( it != m_mapInfo.end() )
	{
	    Addr = it->second.m_dwFirstDataAdd;
		size = it->second.m_dwCompressSize;
		if( dwOutSize < size )
		{
			WriteBound( p, Addr, dwOutSize, m_hFile );
			free.freeSize = size - dwOutSize;
			free.freeAddr = Addr + dwOutSize;

			//TEST:
			if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
			{
				osDebugOut( "pack:AddFile:��д_�� < ��.ʱ����\n" );
				osassert( false );
			}

			m_listFreeInfo.push_back( free );
			it->second.m_dwOriginalSize = filesize;
			it->second.m_dwCompressSize = dwOutSize;
			it->second.m_main_count = main_count;
			it->second.m_remain_len = remain_len;
		}

		//��дʱ�ռ䲻��
		else if( dwOutSize > size )
		{
			//�п��õĿ�����
			if( FreeAddr != 0 )
			{
				osassert( FreeSize - dwOutSize >= 0 );
				osassert( pos != m_listFreeInfo.end() );

				m_listFreeInfo.erase( pos );
				WriteBound( p, FreeAddr, dwOutSize, m_hFile );

				if( FreeSize - dwOutSize > 0 )
				{
					free.freeSize = FreeSize - dwOutSize;
					free.freeAddr = FreeAddr + dwOutSize;

					//TEST:
					if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
					{
						osDebugOut( "pack:AddFile:��д_�� > ��.ʱ����\n" );
						osassert( false );
					}

					m_listFreeInfo.push_back( free );
				}
				free.freeSize = size;
				free.freeAddr = Addr;
					
				//TEST:
				if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
				{
					osDebugOut( "pack:AddFile:��д_�� > ��.����ɿռ�ʱ����\n" );
					osassert( false );
				}

				m_listFreeInfo.push_back( free );

				it->second.m_dwFirstDataAdd = FreeAddr;
				it->second.m_dwOriginalSize = filesize;
				it->second.m_dwCompressSize = dwOutSize;
				it->second.m_main_count = main_count;
				it->second.m_remain_len = remain_len;
			}
			//�޿��ÿ�����
			else
			{
				WriteBound( p, GetDataEndAddr(), dwOutSize, m_hFile );
				free.freeSize = size;
				free.freeAddr = Addr;
				
				//TEST:
				if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
				{
					osDebugOut( "pack:AddFile:��д_�� > ��.�޿��ÿ��пռ�ʱ����\n" );
					osassert( false );
				}

				m_listFreeInfo.push_back( free );
				it->second.m_dwFirstDataAdd = GetDataEndAddr();
				it->second.m_dwOriginalSize = filesize;
				it->second.m_dwCompressSize = dwOutSize;
				it->second.m_main_count = main_count;
				it->second.m_remain_len = remain_len;
				SetDataEndAddr( GetDataEndAddr() + dwOutSize );
			}
		}
		else if( dwOutSize == size )
		{
			WriteBound( p, Addr, dwOutSize, m_hFile );
		}
	}
	else //�޴��ļ�
	{
		std::string s = path;
		memcpy( fi.m_chName, path, strlen( path ) );

		//�п���
		if( FreeAddr != 0 )
		{
			osassert( pos != m_listFreeInfo.end() );
			osassert( FreeSize > dwOutSize );

			m_listFreeInfo.erase( pos );

			if( FreeSize > dwOutSize )
			{
				WriteBound( p,FreeAddr, dwOutSize, m_hFile );
				fi.m_dwCompressSize = dwOutSize;
				fi.m_dwFirstDataAdd = FreeAddr;
				fi.m_dwOriginalSize = filesize;
				fi.m_main_count = main_count;
				fi.m_remain_len = remain_len;
				free.freeSize = FreeSize - dwOutSize;
				free.freeAddr = FreeAddr + dwOutSize;

				//TEST:
				if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
				{
					osDebugOut( "pack:AddFile:�޴��ļ�:�п���,ʱ����\n" );
					osassert( false );
				}

				m_listFreeInfo.push_back( free );

				m_mapInfo.insert( std::make_pair( s, fi ) );
			}
			else if( FreeSize == dwOutSize )
			{
				WriteBound( p, FreeAddr, dwOutSize, m_hFile );
				fi.m_dwCompressSize = dwOutSize;
				fi.m_dwFirstDataAdd = FreeAddr;
				fi.m_dwOriginalSize = filesize;
				fi.m_main_count = main_count;
				fi.m_remain_len = remain_len;

				m_mapInfo.insert( std::make_pair( s, fi ) );
			}
		}
		else
		{
			WriteBound( p, GetDataEndAddr(), dwOutSize, m_hFile );
			fi.m_dwCompressSize = dwOutSize;
			fi.m_dwFirstDataAdd = GetDataEndAddr();
			fi.m_dwOriginalSize = filesize;
			fi.m_main_count = main_count;
			fi.m_remain_len = remain_len;
			m_mapInfo.insert( std::make_pair( s, fi ) );
			SetDataEndAddr( GetDataEndAddr() + dwOutSize );
		}
	}

	return true;
	unguard;
}


/**\brief
*
*	���ð������ݵ�β��ַ,β��ַ�����ļ�β��־��
*   
*/
void Pack::SetDataEndAddr( DWORD _addr )
{
	guard;
	m_sPackHeader.dwEndAddr = _addr;
	unguard;
}

/**\brief
*
*	��ȡ�������ݵ�β��ַ,β��ַ�����ļ�β��־��
*   
*/
DWORD Pack::GetDataEndAddr()
{
	guard;
	return m_sPackHeader.dwEndAddr;
	unguard;
}

/**\brief
*
*	�Ӱ��ļ���ɾ��һ���ļ���
*
*	\param  _filename   �����ļ��������ڵ����·��
*   \return             �ɹ���1,ʧ�ܷ���-1
*	˵����
*
*/
//int Pack::DeletePackFile( char * _FileName )
//{
//	guard;
//	DWORD srcAddr = 0;
//	FreeInfo free;
//	//�����ļ����׵�ַ
//	MapInfo::iterator pos;
//	pos = m_mapInfo.find(_FileName);
//	if( pos != m_mapInfo.end() )
//	{
//		free.freeAddr = pos->second.m_dwFirstDataAdd;
//		free.freeSize = pos->second.m_dwCompressSize;
//		m_listFreeInfo.push_back(free);
//
//		//���ļ���Ϣmap��ɾ����¼
//		m_mapInfo.erase( pos );
//		return 1;
//	}
//	else return -1;
//	unguard; 
//}
/*
std::string Pack::GetBasePath( std::string _s )
{
	std::string rs=_s;
	int pos=rs.rfind('\\');
	if(pos!=-1){
		return rs.substr(0,pos+1);
	}
	return rs;
}
*/


/**\brief
*
*	�ô���Ŀ¼���Ŀ��ȶԣ���ȡ�����Ŀ¼������Ϊ����Ŀ¼
*
* \param   _out   ���·������������
* \param  _big     ��·��
* \param  _small ������·��
*
*/
int Pack::GetDiskFolder( std::string& _out, std::string _big, std::string _small )
{
	guard;
	std::string rs;
	int pos= _big.find(_small);
	if(pos!=0)
		return -1;
	if(_big.compare(_small)==0){
		_out="";
		return 1;
	}
	if(*(_small.end()-1)=='\\')
		*(_small.end()-1) = '\0';
	int len = strlen(_small.c_str());
	_out = _big.substr( len+1, _big.length() );
	return 1;
	unguard;
}

/*
std::string Pack::GetEndPathName(std::string _s )
{
	std::string rs = _s;
	int pos = rs.rfind('\\');
	if(pos!=-1){
		return rs.substr(pos+1,_s.length() );
	}
	return rs;
}
*/


/**\brief
*
*	����ļ��������ļ��м��������ݡ�
*
*	\param _PackPath        ����·������:pack\
*	\param _DiskPath        Ҫ���Ŀ¼�Ĵ���·��,��  c:\tool\test(\...\...) 
*	\param _DiskPathBase    Ҫ���Ŀ¼�Ĵ���·����Ϊ_DiskPath�Ĵ��Ӽ�,��  c:\tool\test
*
*	˵����
*
*/
//bool Pack::AddFolder( char * _PackPath , char * _DiskPath, char * _DiskPathBase )
//{
//	guard;
//	if(_DiskPath[0]=='\0' )
//	{
//		::MessageBox(NULL,"��Ҫ���ӵ��ļ���·��������Ϊ�գ�","��ʾ",MB_OK);
//		return false;
//	}
//	if(_DiskPathBase[0]=='\0' )
//	{
//		::MessageBox(NULL,"Ŀ¼��������·��������Ϊ�գ�","��ʾ",MB_OK);
//		return false;
//	}
//
//
//	std::string DiskFolder ;
//	if(0>GetDiskFolder(DiskFolder, std::string(_DiskPath),std::string(_DiskPathBase)) )
//	{
//		::MessageBox(NULL,"��Ŀ¼���ò���ȷ","��ʾ",MB_OK);
//		return false;
//	}
//	std::string PackFolder;
//	std::string str(_DiskPath);
//
//	BOOL bFound = TRUE;
//	HANDLE hFind;
//	WIN32_FIND_DATA findData;
//
//	SetCurrentDirectory(_DiskPath);
//	hFind = FindFirstFile("*.*",&findData);
//	if(hFind == INVALID_HANDLE_VALUE)
//	{
//		return false;
//	}
//
//	while(bFound)
//	{
//		if(!((strcmp(findData.cFileName,".")==0) || (strcmp(findData.cFileName,"..")==0)))
//		{
//			//std::string s = GetEndPathName(_DiskPath);
//			std::string strFileName = _DiskPath ;
//			strFileName += "\\" ;
//			strFileName += findData.cFileName;
//
//			PackFolder = "";
//			PackFolder += DiskFolder;
//			if(PackFolder!="")
//				PackFolder += "\\";
//			PackFolder += findData.cFileName;
//
//			if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
//			{
//				// Ŀ¼���ݹ����
//				//PackFolder = std::string(_PackPath) + DiskFolder ;
//				//PackFolder = PackFolder + "\\" +   GetEndPathName(strFileName);
//				AddFolder( (char*)PackFolder.c_str(), (char*)strFileName.c_str(), _DiskPathBase );
//			}
//			else
//			{
//				char* filename;
//				char buffer[MAX_PATH]={0};
//				::GetFullPathName(strFileName.c_str(), MAX_PATH, buffer, &filename );
//
//				std::string b;
//				if(DiskFolder!="")
//					b = std::string(DiskFolder)  + "\\";
//				else b = DiskFolder;
//					
//				AddFile((char*) b.c_str(),(char*) strFileName.c_str() );
//				m_CurrCount++;
//			}
//		}
//		bFound = FindNextFile(hFind,&findData);
//	}
//	FindClose(hFind);
//	return true;
//	unguard;
//}

/**\brief
*
*	�Ӱ��ļ���ɾ���ļ��м��������ݡ�
*
*	\param _FolderName  �ļ�Ŀ¼   �磺base\   ��  base  ��ɾ������base�ļ����µ��ļ�
*
*	˵����
*
*/
//bool Pack::DeleteFolder( char * _FolderName )
//{
//	guard;
//	//�����ļ�·������ _FolderName����������Ϣ��
//	bool bIsFind = false;
//	MapInfo::iterator pos;
//	pos=m_mapInfo.begin();
//	while(pos!=m_mapInfo.end())//pos!=m_mapInfo.end(); )
//	{
//		if( IsInclude( pos->first, _FolderName ) ) 
//		{
//			bIsFind = true;
//			DeletePackFile( (char*)(pos->first).c_str() );
//			pos = m_mapInfo.begin();
//		}
//		else
//		{
//			++pos;
//		}
//	}
//	if(!bIsFind)
//	{
//		MessageBox(NULL,"��Ҫɾ�����ļ��в����ڣ�","��ʾ",MB_OK);
//	}
//	return true;
//	unguard;
//}

/**\brief
*
*	��_full���Ƿ����_pathbase���ڡ�λ��_full���ײ�
*
* \param  _full      ĸ·�� 
* \param  _pathbase  ����ĸ·���Ƚϵ���·��
* \return bool     ����trueָʾ_pathbase����_full���Ӽ������Ӵ�λ��0������Ϊ��������
*
*/	
bool Pack::IsInclude( std::string _full, std::string _pathbase )
{
	guard;
	int pos = _full.find(_pathbase);
	char prec = _full.at( _pathbase.length()-1 );
	char c = _full.at( _pathbase.length() );
	if(pos==0)
	{
		if(prec=='\\' || c == '\\')
			return true;
		else 
			return false;
	}
	else return false;
	unguard;
}

/**\brief
*
*	�Ӱ��ļ��п��������ļ������̡�
*
*	\param _disk_dest       Ŀ���ļ���·��������·����
*	\param _pack_src_file   �����ļ�·�����������·����
*	\return   bool          ����true��ʾ�ɹ�������ʧ�ܡ� 
*                                           
*	˵�����ô˺����Ӱ��ڿ��������ļ������̡�
*
*/
bool Pack::CopyPackFileToDisk( char * _DiskDest, char * _PackSrcFile )
{
	guard;

	HANDLE hFile=::CreateFile(_DiskDest,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		return false;

	BYTE * pBuffer = NULL;
	BYTE * pOutBuffer = NULL;
	DWORD dwOutSize=0;
	FileInfo fi = FindFileInfo(_PackSrcFile);
	if(fi.m_dwFirstDataAdd!=0 && fi.m_dwCompressSize!=0)
	{
		if(fi.m_dwCompressSize> (DWORD)m_CompressWorkBufferSize)
		{
			delete [] m_pCompressWorkBuffer;
			m_pCompressWorkBuffer = new BYTE[fi.m_dwCompressSize];
			m_CompressWorkBufferSize = fi.m_dwCompressSize;
		}
		if(fi.m_dwOriginalSize> (DWORD)m_OriginalWorkBufferSize)
		{
			delete [] m_pOriginalWorkBuffer;
			m_pOriginalWorkBuffer= new BYTE[fi.m_dwOriginalSize];
			m_OriginalWorkBufferSize = fi.m_dwOriginalSize;
		}

		ReadBound( m_pCompressWorkBuffer, fi.m_dwFirstDataAdd, fi.m_dwCompressSize, m_hFile );
		dwOutSize = fi.m_dwOriginalSize;
		//test
		//dwOutSize += 500;
		if( Z_BUF_ERROR==uncompress( m_pOriginalWorkBuffer, &dwOutSize,  m_pCompressWorkBuffer, fi.m_dwCompressSize ) )
		{
			MessageBox( NULL, ERROR_FILEINFO_ORIGINALSIZE, "����", MB_OK );
			osassert(false);
		}

		WriteBound( m_pOriginalWorkBuffer, 0, dwOutSize, hFile );
	}
	SetEndOfFile( hFile );
	CloseHandle( hFile );
	return true;
	unguard;
}

/**\brief
*
*	�Ӱ��ļ��п����ļ��м��������ݵ����̡�
*
*	\param _disk_dest   Ŀ���ļ��У�����·����
*	\param _pack_src���������ļ��У��������·������
*	\return   bool  ����true��ʾ�򿪳ɹ�������ʧ�ܡ� 
*
*	˵�����Ӱ��ļ��п����ļ��м��������ݵ�����,���Ŀ��·����ָ�����ļ��У�������,����
*
*/
bool Pack::CopyPackFolderToDisk( char * _DiskDest, char * _PackSrc  )
{
	guard ;
	int len = strlen(_DiskDest);
	if(len<=0) return false;
	if(_DiskDest[len-1]!='\\'){
		_DiskDest[len]='\\';
		_DiskDest[len+1]='\0';
	}
	std::string filename;
	MapInfo::iterator pos;
	for(pos=m_mapInfo.begin();pos!=m_mapInfo.end();++pos )
	{
		if( IsInclude( pos->first, _PackSrc ) ) 
		{
			char path[MAX_PATH]={0};
			//MakePath( path, _DiskDest, _PackSrc );
			strcpy(path,_DiskDest);
			if(path[strlen(path)-1]!='\\')
				strcat(path,"\\");
			strcat(path,pos->first.c_str() );
			
			// is folder exist? if no, then create it.
			IfNoFolderThenCreate( path );
			
			CopyPackFileToDisk( path, (char*)(pos->first).c_str() );
		}
	}
	return true;
	unguard;
}

bool Pack::IfNoFolderThenCreate( char * fileOrFolderName )
{
	guard;
	std::string s(fileOrFolderName);
	std::string p;
	int pos1 = 0; int pos2 = 0;
	for(;;)
	{
		pos2=s.find("\\",pos1);
		if(pos2>0)
		{
			p=s.substr(0,pos2);
			WIN32_FIND_DATA fdData;
			HANDLE hFind=::FindFirstFile(p.c_str(),&fdData);
			if (hFind == INVALID_HANDLE_VALUE)
			{ 
				::CreateDirectory(p.c_str(),NULL);
			}
			::FindClose(hFind);
		}else 
			break;
		pos1=pos2+1;
	}
	return true;
	unguard;
}

/**\brief
*
*	дLOG�ļ�����������Ϣ,��С���⣬���Ŀ���Ƭ������Ƭ��������Ϣ��
*              ������
*
* \param   _logfile   ��־�ļ�����·����
* \return  bool                �����ɹ���true,ʧ�ܷ�false
*
*/
void Pack::Dump( char * _logfile )
{
	guard;
	HANDLE hFile=::CreateFile(_logfile,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		return;
	int currAddr = 0;
	char buffer[1024]={0};
	char *pStart = "start dump Free Info in pack...\r\n---------------------------------------------\r\n";
	WriteBound(pStart,currAddr,strlen(pStart),hFile);
	currAddr += strlen(pStart);
	ListFreeInfo::iterator it;
	std::sort(m_listFreeInfo.begin(),m_listFreeInfo.end(),lessSortAddr);
	for(it=m_listFreeInfo.begin();it!=m_listFreeInfo.end();++it)
	{
		sprintf(buffer,"addr: %d\t size: %d\r\n",it->freeAddr,it->freeSize );
		WriteBound(buffer,currAddr,strlen(buffer),hFile);
		currAddr += strlen(buffer);
	}
	char *pEnd = "---------------------------------------------\r\nend\r\n\r\n";
	WriteBound(pEnd,currAddr,strlen(pEnd),hFile);
	currAddr+=strlen(pEnd);

	int size = m_mapInfo.size();
	//write compress(%)
	DWORD a=0,b=0,totalCompressSize=0,totalOriginalSize=0;
	MapInfo::iterator pos;
	for(pos=m_mapInfo.begin();pos!=m_mapInfo.end();++pos)
	{
		a+=pos->second.m_dwCompressSize;
		b+=pos->second.m_dwOriginalSize;
	}
	sprintf(buffer,"file count: %d\r\ntotalCompressSize: %d\r\ntotalOriginalSize: %d\r\ncompress rate: %.2f%%\r\n", 
		m_mapInfo.size(),a,b, (float)a/(float)b*100.f );
	WriteBound(buffer,currAddr, strlen(buffer),hFile);
	currAddr+=strlen(buffer);

	//output all filename...
	int i=0;
	for(pos=m_mapInfo.begin();pos!=m_mapInfo.end();++pos)
	{
		char buf[256];
		sprintf( buf,"%d:%s\r\n",i,pos->first.c_str() );
		WriteBound( buf, currAddr, strlen(buf), hFile );
		currAddr+=strlen(buf);
		i++;
	}

	CloseHandle(hFile);
	unguard;
}

/**\brief
*
*	��ȡ���е�һ���ļ����ڴ档
*	\param _InPackFileName                 Ҫ��ȡ���ļ�����Ϣ��ĵ�ַ��
*	\param _pOutBuffer                     �ڴ����ݵ�ַ
*   \param _dwBufferSize                   �ڴ����ݵĴ�С(�ļ���С)
*
*	˵����ѭ���ļ������е����ݿ飬�������ݡ���ѹ��д���ⲿ�ļ���
*
*/
int Pack::CopyPackFileToMemory( char * _InPackFileName, LPVOID _pOutBuffer, DWORD _dwBufferSize )
{
	guard;
	BYTE * pBuffer = NULL;
	BYTE * pOutBuffer = NULL;
	DWORD dwOutSize=0;
	FileInfo fi = FindFileInfo(_InPackFileName);
	if( fi.m_dwCompressSize == 0 && fi.m_dwFirstDataAdd == 0 &&
		fi.m_dwOriginalSize == 0 && fi.m_main_count == 0 && fi.m_remain_len == 0 )
	{
		osDebugOut( "Packȱ���ļ�:%s\n", _InPackFileName );
		osassert( false );
	}

	if( _dwBufferSize < fi.m_dwOriginalSize )
	{
		osDebugOut( "������̫С:%s, size:%d, but,at least:%d\n", _InPackFileName, _dwBufferSize, fi.m_dwOriginalSize );
		osassert( false );
	}
	DWORD BufferSize;
	if(fi.m_dwFirstDataAdd!=0 && fi.m_dwOriginalSize!=0)
	{
		if(fi.m_dwCompressSize> COMPRESS_BUFFER_SIZE)
		{
			delete [] m_pCompressWorkBuffer;
			m_pCompressWorkBuffer = new BYTE[fi.m_dwCompressSize];
		}
		ReadBound(m_pCompressWorkBuffer,fi.m_dwFirstDataAdd,fi.m_dwCompressSize,m_hFile);
		BufferSize = fi.m_dwOriginalSize;
		uncompress( (BYTE*)_pOutBuffer, &BufferSize,  m_pCompressWorkBuffer, fi.m_dwCompressSize );
		if( BufferSize == 0 )
		{
			osDebugOut( "ErrorFile:%s", _InPackFileName );
			osassert( false );
		}
		if( BufferSize != fi.m_dwOriginalSize )
		{
			osDebugOut( "ErrorFile:%s", _InPackFileName );
			osassert( BufferSize==fi.m_dwOriginalSize );	
		}
	}
	return BufferSize;
	unguard;
}

/**\brief
*
*	�ϳ�·��,�����ַ������
*
*/
void Pack::MakePath( char * _pOut, char * _p1, char * _p2 )
{
	guard;
	strcpy( _pOut, _p1 );
	strcat( _pOut, _p2 );
	unguard;
}

/**\brief
*
*	�ڰ��в���ָ�����ļ���
*	\param _FileName             Ҫ���ҵ��ļ������ƣ��ڰ��е����ơ����·��
*	\return PosInfo              �ļ���λ�ü���С��Ϣ��
*
*
*/
PosInfo Pack::FindFileInPack(char * _FileName )
{
	guard;
	PosInfo pi;
	memset(&pi,0,sizeof(PosInfo));

	if( m_mapInfo.empty() ) return pi;

	MapInfo::iterator pos;
	pos = m_mapInfo.find(_FileName);

	if(pos!=m_mapInfo.end())
	{
		pi.freeAddr = pos->second.m_dwFirstDataAdd;
		pi.freeSize = pos->second.m_dwCompressSize;
	}
	return pi;
	unguard; 
}

/**\brief
*
*	����ļ�����Ϣ��
*	\param _FileName             Ҫ���ҵ��ļ������ƣ��ڰ��е����ơ����·��
*	\return FileInfo              �ļ�����Ϣ��
*
*
*/
FileInfo Pack::FindFileInfo(char * _filename )
{
	guard;

	//��Ŀ¼�ļ�����ΪСд
	std::string str(_filename);
	std::transform(str.begin(), str.end(), str.begin(), tolower);

	FileInfo fi;
	memset( &fi,0,sizeof(FileInfo));
	if( m_mapInfo.empty() ) return fi;
	MapInfo::iterator pos;
	pos = m_mapInfo.find(str.c_str());

	if(pos!=m_mapInfo.end())
	{
		fi.m_dwCompressSize = pos->second.m_dwCompressSize;
		fi.m_dwFirstDataAdd = pos->second.m_dwFirstDataAdd;
		fi.m_dwOriginalSize = pos->second.m_dwOriginalSize;
		fi.m_main_count = pos->second.m_main_count;
		fi.m_remain_len = pos->second.m_remain_len;
	}
	return fi;
	unguard;
}

/**\brief
*
*	����ָ���Ĵ����ļ��Ƿ���ڣ�
*
* \param _FileName  �������ļ���
* \return bool     ����trueָʾ�ļ����ڣ�����Ϊ������
*
*/
bool Pack::IsFileExist( char * _FileName)
{
	guard;
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile( _FileName, &fd );
	if( hFind == INVALID_HANDLE_VALUE )
		return false;
    FindClose( hFind );
	return true;
	unguard; 
}

/**\brief
*
*	��ȡ���ļ���ĳһ��Χ���ݡ�
*
*	\param _pBuffer       ���ݻ�������
*	\param _dwStartAdd   ��ʼλ�á�
*	\param _dwNum          Ҫ�������ݵĴ�С��
*   \param _handle       �ļ����
*
*	˵�����ȶ�λ�ļ�ָ�뵽_dwStartAdd��Ȼ�����_dwNum���������ݡ�
*
*/
void Pack::ReadBound( LPVOID _pBuffer, DWORD _dwStartAdd, DWORD _dwNum, HANDLE _handle )
{
	guard;
	osassert( _handle != 0 );
	SetFilePointer( _handle, _dwStartAdd, NULL, FILE_BEGIN );
	ReadFile( _handle, _pBuffer, _dwNum, &_dwNum, NULL );
	unguard; 
}

/**\brief
*
*	д�����ݵ����ļ��е�ĳһ��Χ��
*
*	\param _pBuffer          ���ݻ�������
*	\param _dwStartAdd   ��ʼλ�á�
*	\param _dwNum          Ҫд�����ݵĴ�С��
*   \param _handle       �ļ����
*
*	˵�����ȶ�λ�ļ�ָ�뵽_dwStartAdd��Ȼ��д��_dwNum���������ݡ�
*
*/
void Pack::WriteBound( LPVOID _pBuffer, DWORD _dwStartAdd, DWORD _dwNum, HANDLE _handle )
{
	guard;
	osDebugOut( "WriteBound:start:%d,size:%d\n", _dwStartAdd, _dwNum );
	osassert( _handle != 0 );
	SetFilePointer( _handle, _dwStartAdd, NULL, FILE_BEGIN );
	WriteFile( _handle, _pBuffer, _dwNum, &_dwNum, NULL );
	unguard;
}

/**\brief
*
*	����ļ��ĸ�����
*	\return DWORD   �����ļ��ĸ�����
*
*/
DWORD Pack::GetFileCount()
{
	guard;
	return m_mapInfo.size();
	unguard;
}

/**\brief
*
*	���m_mapInfo,���ڴ���������б����item��
*
*/
MapInfo& Pack::GetInfoMap()
{
	guard;
	return m_mapInfo;
	unguard;
}

/**\brief
*
*	�ļ��ĸ���
*	\param _newName  �ڰ��ڵ�������
*   \param _oldName  �ڰ��ڵ�ԭ��������
*
*/
//void Pack::RenameFile( char * _newName, char * _oldName )
//{
//	guard;
//	if( _newName[0] == '\0'|| _oldName[0] == '\0' )
//		return;
//
//	MapInfo::iterator pos ;
//
//	//�Ƿ��Ѿ�����?
//	pos = m_mapInfo.find( _newName );
//	if( pos != m_mapInfo.end() )
//	{
//		char msg[256] = {0};
//		sprintf( msg, "�Ѵ����ļ���<%s>������ʧ��!", _newName );
//		MessageBox( NULL, msg, "ʧ��", MB_OK );
//		return;
//	}
//
//	FileInfo info;
//
//	//�Ƴ��ļ�������Ϣ
//	pos = m_mapInfo.find( _oldName );
//	if( pos != m_mapInfo.end() )
//	{
//		info = pos->second;
//		m_mapInfo.erase( pos );
//	}
//	
//	//�������
//	strcpy( info.m_chName, _newName );
//	m_mapInfo.insert( std::make_pair( std::string( _newName ), info ) );
//	unguard;
//}

/**\brief
*
*	��հ����ļ�����Ƭ�����ٰ��Ĵ�С��ʹ�����ļ���������
*   �˺�������ʱ������һ����ʱ�ļ������ý����Զ�ɾ��
*
*  \return bool ������ɳɹ���true,ʧ�ܷ�false;
*
*/
bool Pack::ClearUpFragment()
{
	guard;
	m_CurrCount = 0;

	char * tempFileName = "tmpFilelandlind";
	HANDLE handle=::CreateFile( tempFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
	if( handle == INVALID_HANDLE_VALUE )
	{
	    return false;
	}
	
	//write to temp file.
	DWORD curAddr = 0;
	BYTE * p = NULL;
	FileInfo fi;
	MapInfo::iterator pos;
	for( pos=m_mapInfo.begin(); pos!=m_mapInfo.end(); ++pos )	
	{	
		fi = pos->second;
		
		p = new BYTE[ fi.m_dwCompressSize ];
		ReadBound( p, fi.m_dwFirstDataAdd, fi.m_dwCompressSize, m_hFile );
		WriteBound( p, curAddr, fi.m_dwCompressSize, handle );

		pos->second.m_dwFirstDataAdd = curAddr;
		curAddr += fi.m_dwCompressSize;
		
		m_CurrCount ++;
		delete [] p;
	}

	m_CurrCount=0;

	//Write to file
	curAddr = sizeof( PackHeader );
	for( pos=m_mapInfo.begin(); pos!=m_mapInfo.end(); ++pos )
	{
		fi = pos->second;
		p = new BYTE[ fi.m_dwCompressSize ];
		ReadBound( p, fi.m_dwFirstDataAdd, fi.m_dwCompressSize, handle );
		WriteBound( p, curAddr, fi.m_dwCompressSize, m_hFile );

		pos->second.m_dwFirstDataAdd = curAddr;
		curAddr += fi.m_dwCompressSize;

		m_CurrCount ++;
		delete [] p;
	}
	SetDataEndAddr( curAddr );

	CloseHandle( handle );
	DeleteFile( tempFileName );

	//clear FreeInfo vector
	m_listFreeInfo.clear();

	return true;
	unguard;
}

/**\brief
*
*	��������ʹ�ô˺������н��������ļ�ѹ��.���ʱ���ļ�����Ϊ
*   
*   \param _DiskFile  ���������ļ�������
*   \param _fileNameInPack    �ڰ��ڵ��ļ���
*   \return bool          �ɹ���true,ʧ��false
*
*/
//bool Pack::ServerCompressFile( char * _DiskFile,  char * _FileNameInPack )
//{
//	guard;
//	if( !IsFileExist(_DiskFile) )
//	{
//		MessageBox( NULL,"��Ҫ���ӵ��ļ������ڣ�","��ʾ",MB_OK );
//		return false;
//	}
//
//	HANDLE handle = NULL;
//	SetFileAttr_ReadWrite( (char*)_DiskFile );
//	handle=::CreateFile( _DiskFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
//	if( handle == INVALID_HANDLE_VALUE )
//		return false;
//
//	//read
//	int filesize = GetFileSize( handle, NULL );
//	BYTE * pFileBuffer = new BYTE[ filesize ];
//	DWORD dwOutSize=(DWORD)((float)filesize * 1.01f + 12 );
//	BYTE * pCompressBuffer = new BYTE[ dwOutSize ];
//
//	ReadBound( pFileBuffer, 0, filesize, handle );
//	if(Z_MEM_ERROR==compress( pCompressBuffer, &dwOutSize, pFileBuffer, filesize ) )
//	{
//		::MessageBox(NULL,"ServerCompressFile����������������㣡","����",MB_OK);
//		return false;
//	}
//	CloseHandle( handle );
//
//
//	// change name
//	std::string lsc( _DiskFile );
//	lsc.append( SERVER_COMPRESS_POSTFIX );
//
//	// write
//	handle=::CreateFile( lsc.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
//	if( m_hFile == INVALID_HANDLE_VALUE )
//	{
//	    return false;
//	}
//
//	FileHeader_FromServer ff;
//	memset( &ff, 0, sizeof( FileHeader_FromServer ) );
//	int currAddr = 0;
//	ff.dwFlag = SERVER_UPDATE_FILE_HEADER_FLAG;
//	WriteBound( &ff.dwFlag, currAddr, sizeof( ff.dwFlag ), handle );
//	currAddr+=sizeof(ff.dwFlag);
//	ff.PackFileNameLen = strlen( _FileNameInPack ) + 1;
//	WriteBound( &ff.PackFileNameLen, currAddr, sizeof(ff.PackFileNameLen),handle);
//	currAddr+=sizeof(ff.PackFileNameLen);
//	WriteBound( _FileNameInPack, currAddr, ff.PackFileNameLen, handle);
//	currAddr+=ff.PackFileNameLen;
//	ff.dwOriginalSize = filesize;
//	WriteBound( &ff.dwOriginalSize, currAddr, sizeof(ff.dwOriginalSize), handle );
//	currAddr+=sizeof(ff.dwOriginalSize);
//	ff.dwCompressSize = dwOutSize;
//	WriteBound( &ff.dwCompressSize, currAddr, sizeof(ff.dwCompressSize),handle);
//	currAddr+=sizeof(ff.dwCompressSize);
//	WriteBound( pCompressBuffer, currAddr, dwOutSize, handle);
//
//	CloseHandle(handle);
//
//	delete [] pFileBuffer;
//	delete [] pCompressBuffer;
//
//	return true;
//	unguard;
//}

/**\brief
*
*	��������ʹ�ô˺����� *.*c_ �ļ���ѹ��ԭ�ļ���
*
*   \param  _c_File  ����������ʹ��ServerCompressFile����������ѹ����
*   \return bool          �ɹ���true,ʧ��false
*
*/
//bool Pack::ServerUnCompressFile( char * _c_File )
//{
//	guard;
//	HANDLE handle = NULL;
//	SetFileAttr_ReadWrite( (char*)_c_File );
//	handle=::CreateFile(_c_File,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
//	if( handle == INVALID_HANDLE_VALUE )
//		return false;
//	FileHeader_FromServer ff;
//	memset(&ff,0,sizeof(FileHeader_FromServer));
//	int currAddr=0;
//	char name[MAX_PATH] = {0};
//	ReadBound(&ff.dwFlag,currAddr,sizeof(ff.dwFlag),handle);
//	if(ff.dwFlag!=SERVER_UPDATE_FILE_HEADER_FLAG)
//	{
//		::MessageBox(NULL,"��Ҫ��ѹ�����ļ�����.lsc�ļ�,����ļ����𻵣�","��ѹʧ��",MB_OK);
//		return false;
//	}
//	currAddr+=sizeof(ff.dwFlag);
//	ReadBound(&ff.PackFileNameLen,currAddr,sizeof(ff.PackFileNameLen),handle);
//	currAddr+=sizeof(ff.PackFileNameLen);
//	ReadBound(name,currAddr,ff.PackFileNameLen,handle);
//	currAddr=currAddr+ff.PackFileNameLen;
//	ReadBound( &ff.dwOriginalSize, currAddr,sizeof(ff.dwOriginalSize),handle);
//	currAddr+=sizeof(ff.dwCompressSize);
//	ReadBound( &ff.dwCompressSize, currAddr,sizeof(ff.dwCompressSize),handle);
//	currAddr+=sizeof(ff.dwOriginalSize);
//
//	BYTE* pOriginal = new BYTE[ff.dwOriginalSize];
//	BYTE* pCompress = new BYTE[ff.dwCompressSize];
//	ReadBound( pCompress, currAddr, ff.dwCompressSize, handle);
//	DWORD dwOutSize = ff.dwOriginalSize;
//	if(Z_BUF_ERROR==uncompress( pOriginal, &dwOutSize,  pCompress, ff.dwCompressSize ))
//	{
//		osassert(false);
//	}
//	CloseHandle(handle);
//
//
//	std::string lsc (_c_File);
//	std::string out = lsc.substr(0,lsc.length()-strlen(SERVER_COMPRESS_POSTFIX));
//
//	
//	// write
//	handle=::CreateFile(out.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
//	if( handle == INVALID_HANDLE_VALUE )
//	{
//	    return false;
//	}
//	WriteBound(pOriginal,0,ff.dwOriginalSize,handle);
//	CloseHandle(handle);
//
//	delete [] pOriginal;
//	delete [] pCompress;
//	return true;
//	unguard;
//}


int Pack::GetFolderFileCount(char*_folder)
{
	guard;
	static int count = 0;
	BOOL bFound = TRUE;
	HANDLE hFind;
	WIN32_FIND_DATA findData;

	SetCurrentDirectory(_folder);
	hFind = FindFirstFile("*.*",&findData);
	if(hFind == INVALID_HANDLE_VALUE)
		return false;

	while(bFound)
	{
		if(!((strcmp(findData.cFileName,".")==0) || (strcmp(findData.cFileName,"..")==0)))
		{
			if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				std::string strFileName = _folder ;
				strFileName += "\\" ;
				strFileName += findData.cFileName;
				GetFolderFileCount((char*)strFileName.c_str());		
			}
			else
			{
				m_TotalCount++;
				count = m_TotalCount;
			}
		}
		bFound = FindNextFile(hFind,&findData);
	}

	FindClose(hFind);
	return count;
	unguard;
}


//--------------------------------------------------------------------
// �����ǰ��Ľӿں���, �������裺��ʼ���ӿڣ��������ͷŽӿ�
//--------------------------------------------------------------------
//! �����MFC����������̵Ļ���������ȫ�ֱ�����δ����ջʱ���ڶ���
//! �������ڴ棬����MFC�ĶѼ�����ʧЧ�������ڴ�й©�Ĵ���
// MapPackInfo   g_mapPackInfo_Update; //��ʼ������ʱ��
// MapPackInfo   g_mapPackInfo;        //����ʱ�á�

MapPackInfo*   g_mapPackInfo_Update = NULL; //��ʼ������ʱ��
MapPackInfo*   g_mapPackInfo = NULL;        //����ʱ�á�


//����ȡ�ļ���·�����׶��ļ��У���: c:\tool\test\1.txt ->  tool
void GetFirstFolder( char * _filename, char * _outFirst )
{
	guard;
	std::string s = _filename;
	std::basic_string <char>::size_type first;
	std::basic_string <char>::size_type second = -1;
	if(s.at(0)=='\\')
	{
		first =s.find_first_of('\\',0);
		second = s.find_first_of('\\',first);
	}
	else
	{
		first=0;
		second = s.find_first_of('\\',0);
	}

	std::string s2 = s.substr(first,second);
	strcpy(_outFirst,s2.c_str());
	unguard;
}

/** \brief
 *  �鿴һ���ļ��Ƿ����.
 *  
 *  \param  _filename  ����·��
 */
bool file_exist_pack( const char* _filename )
{
	guard;

	::EnterCriticalSection( &g_csRFToBuf );	

	osassert( fs );
	const char* t_fileName;
	const char* t_hashstr;

	char filePath[MAX_PATH*2];
	t_fileName = filePath;
	strcpy(filePath,_filename);
	int StrLen = strlen(filePath);
	for (int i =0; i < StrLen;i++)
	{
		filePath[i] = tolower(filePath[i]);
		if (filePath[i] == '/')
		{
			filePath[i]='\\';
		}
	}

	char hashstr[256];
	::EnterCriticalSection( &g_csMD5 );
	getMd5Str( filePath,hashstr);
	::LeaveCriticalSection( &g_csMD5 );
	t_hashstr = hashstr;

	// River added @ 2009-11-25:���ڴ�����̵߳İ�ȫ����
	::EnterCriticalSection( &g_sFileReadCS );
	int n = getfs_file_exists( fs, t_hashstr );
	::LeaveCriticalSection( &g_sFileReadCS );

	if( n==0 )
	{
		osDebugOut( "h2�ļ�������:file_exist_pack:<%s><%s>\n",t_hashstr, t_fileName );
		::LeaveCriticalSection( &g_csRFToBuf );
		return false;
	}


	::LeaveCriticalSection( &g_csRFToBuf );
	return true;


	unguard;
}

/** 
 *  �õ��ļ����ȵĺ���,������*.osk�ļ��е��ļ�,Ҳ���Ե������ļ�.
 *  
 *  \return  ����������,����-1,�����ȷ,�����ļ�����,��byteΪ��λ.
 *  \param   _filename Ҫ������ļ�������.
 */
int get_fileSize_pack( char* _filename )
{
	guard;

	osassert( fs );
	const char* t_fileName;
	const char* t_hashstr;

	char filePath[MAX_PATH*2];
	t_fileName = filePath;
	strcpy(filePath,_filename);
	int StrLen = strlen(filePath);
	for (int i =0; i < StrLen;i++)
	{
		filePath[i] = tolower(filePath[i]);
		if (filePath[i] == '/')
		{
			filePath[i]='\\';
		}
	}

	::EnterCriticalSection( &g_sFileReadCS );

	// River @ 2010-4-1:ȥ���˶δ��룬�����ڲ���������.
	/*
	if( !file_exist_pack( (char*)t_fileName ) )
	{
		osDebugOut( "get_fileSize_pack:�ļ�û�ҵ�:<%s>\n", _filename );
		::LeaveCriticalSection( &g_sFileReadCS );
		return -1;
	}
	*/

	int original_size=0, zip_size=0;

	char hashstr[256];
	::EnterCriticalSection( &g_csMD5 );
	getMd5Str( filePath,hashstr);
	::LeaveCriticalSection( &g_csMD5 );
	t_hashstr = hashstr;

	int  t_iRes = 0;
	t_iRes = getfs_file_get_size( fs, t_hashstr, original_size, zip_size );
	if( t_iRes < 0 )
	{
		::LeaveCriticalSection( &g_sFileReadCS );
		osDebugOut( "�����ļ�<%s>����ֵ�д�....\n",_filename );
		return -1;
	}
	::LeaveCriticalSection( &g_sFileReadCS );

	osassertex( original_size > 0 ,
		va( "The File name is:<%s>.size<%d,%d>...\n",t_fileName,original_size, zip_size ) );
	osassertex( original_size > 0 ,
		va( "The File name is:<%s>.size<%d,%d>...\n",t_fileName,original_size, zip_size ) );

	return original_size;

	unguard;
}

/** \brief
 *  ����Ĭ�ϵ�����ļ���
 *
 *  �����ļ�����չ�������롣����һЩ�ؼ��Ե��ļ�����Ȼ��Ҫassert���˳���
 *  
 */
int  load_defaut_file_pack( char* _filename,BYTE* _buf,int _bufsize )
{
	guard;

	s_string     t_str = _filename;
	int          t_idx;

	t_idx = (int)t_str.rfind( "." );
	t_str = t_str.substr( t_idx+1,t_str.length() );

	MessageBox( NULL,_filename,"�ļ��Ҳ���������Ĭ���ļ�",MB_OK );

	// �ֱ���벻ͬ�����͵��ļ�
	if( t_str == "hon" )
	{
		return read_fileToBuf_pack( "scemesh\\default.hon",_buf,_bufsize );
	}
	if( t_str == "ex" )
	{
		return read_fileToBuf_pack( "scemesh\\default.ex",_buf,_bufsize );
	}
	if( t_str == "fdp" )
	{
		return read_fileToBuf_pack( "particle\\default.fdp",_buf,_bufsize );
	}
	if( t_str == "osa" )
	{
		return read_fileToBuf_pack( "keyAni\\default.osa",_buf,_bufsize );
	}
	if( t_str == "bb" )
	{
		return read_fileToBuf_pack( "efftexture\\default.bb",_buf,_bufsize );
	}
	if( t_str == "wav" )
	{
		return read_fileToBuf_pack( "sound\\default.wav",_buf,_bufsize );
	}

	osDebugOut( "\n�ļ�<%s>�ǹؼ��ļ�����Ŀ¼�Ҳ���...�������������µĿͻ����޸�����\n",_filename );
	osassert( false );

	return -1;

	unguard;
}



/** 
 *  ���ļ����뵽һ����������.���ض�����ļ��ĳ��ȡ�
 *
 *  \return ����ɹ�,�����ļ�����,����BUFFER���Ȳ��㷵��-1.�޴��ļ�����-2
 *  \param _buf �ļ�Ҫ����Ļ�����,�Ѿ������㹻�ڴ�.
 *  \param _bufsize �������Ĵ�С����λ���ֽ�.
 */
int read_fileToBuf_pack( char* _filename,BYTE* _buf,int _bufsize )
{
	guard;

	// tzz:
	// ���ٽ����Ľ�����뿪��һ�������
	// ������ EnterCriticalSection �� LeaveCriticalSection ֮����osassert �׳��쳣��ʱ��
	// �������뵽ϵͳ�ں�
	//
	CCriticalSec t_sec(&g_csRFToBuf);
	//::EnterCriticalSection( &g_csRFToBuf );	

	osassert( fs );
	const char* t_fileName;
	const char* t_hashstr;

	char filePath[MAX_PATH*2];
	t_fileName = filePath;
	strcpy(filePath,_filename);
	int StrLen = strlen(filePath);
	for (int i =0; i < StrLen;i++)
	{
		filePath[i] = tolower(filePath[i]);
		if (filePath[i] == '/')
		{
			filePath[i]='\\';
		}
	}

	char hashstr[256];
	::EnterCriticalSection( &g_csMD5 );
	getMd5Str( filePath,hashstr);
	::LeaveCriticalSection( &g_csMD5 );
	t_hashstr = hashstr;

	// tzz:
	// ���ٽ����Ľ�����뿪��һ�������
	// ������ EnterCriticalSection �� LeaveCriticalSection ֮����osassert �׳��쳣��ʱ��
	// �������뵽ϵͳ�ں�
	//
	CCriticalSec t_readSec(&g_sFileReadCS);
	// River @ 2009-11-24:����˴���������̶߳����ļ�����.
	//::EnterCriticalSection( &g_sFileReadCS );

	if( !getfs_file_exists( fs, t_hashstr ) )
	{
		char t_sz[256];
		getfs_perror( fs,t_sz );
		osDebugOut( "h3�ļ�������:<%s><%s><%s>getfs_file_exists\n",t_sz,hashstr,t_fileName );
	}
	int f1 = getfs_file_open( fs, t_hashstr, O_RDONLY );
	if( f1 < 0 )
	{
		char t_sz[256];
		getfs_perror( fs,t_sz );
		osDebugOut( "�ļ�����:<%s><%s>\n",t_sz,t_fileName );
		// ������Զ�������ļ���
		if( g_bUseDefaultFile )
		{
			// tzz CCriticalSec �������������Զ� LeaveCriticalSection
			//
			// River @ 2009-11-24:����˴���������̶߳����ļ�����.
			//::LeaveCriticalSection( &g_sFileReadCS );
			//::LeaveCriticalSection( &g_csRFToBuf );

			int default_size = load_defaut_file_pack( (char*)t_fileName,_buf,_bufsize );

			return default_size;
		}
		
		osDebugOut( "<%s><%s> File not Found!!!!!read_fileToBuf_pack\n", t_hashstr, t_fileName );
		osassertex( false,t_fileName );
	}

	int original_size = getfs_file_read_uncompress( fs,f1,_buf,_bufsize );
	if( original_size < 0 )
	{
		osDebugOut( 
			"getfs_file_read_uncompress:_filename:<%s><%s>, f1:<%d>, _buf:<%d>, _bufsize:<%d>original_size<%d>\n", 
			t_hashstr,_filename,f1,_buf,_bufsize,original_size );	

		osDebugOut( "Error code<%d>ErrorSize<%d>...\n",
			int(*((int*)_buf)),((int*)_buf)[1] );

		FILE*   t_file = fopen( "test.txt","wt" );
		BYTE*  t_ptrByte = (BYTE*)&(((int*)_buf)[2]);
		for( int t_i = 0;t_i<((int*)_buf)[1];t_i ++ )
		{
			//osDebugOut( "%d,",t_ptrByte[t_i]);
			fprintf( t_file,"%d,",t_ptrByte[t_i] );
		}
		fclose( t_file );

		osDebugOut( "\n" );


		MessageBox( NULL,"���ڽ������Ĵ��������������������QQ��,������2000�߱ҵĽ����������ύ�����������ԡ�������˳��������Ϸ�������������ԡ�����Ȼ����ȥ��Ϸ����������õ�ǰ�Ŀͻ��ˣ���Ҫ�ż����°�װ������ϵ�ͷ��������Եõ�10000�߱ҵĽ�����","�ǳ���Ǹ",MB_OK );
		
		//River @ 2010-9-16: ��ָ�룬�����ܹ�����dump�ļ�
		(*(char*)0) = NULL;

		osassert( false );
	}

	getfs_file_close( fs, f1 );

	// tzz CCriticalSec �������������Զ� LeaveCriticalSection
	//
	// River @ 2009-11-24:����˴���������̶߳����ļ�����.
	//::LeaveCriticalSection( &g_sFileReadCS );
	//::LeaveCriticalSection( &g_csRFToBuf );

	return original_size;


	unguard;
}

bool get_packVerion( PACKVER& _ver )
{
	if( !fs )
	{
		_ver.main = 0;
		_ver.minor = 0;
		_ver.patch = 0;
		return false;
	}
	_ver = getfs_get_version( fs );
	return true;

}

/** \brief
 *  �ͻ��� ��ʼ�����Ľӿ�,�ڴ˽����ڴ�ķ���
 *  
 *  \param  iniFile  ���������ļ�
 */
bool init_packinterface_pack( const char * iniFile )
{
	guard;

	char path[MAX_PATH];
	CIni ini;
	if( !ini.Open( iniFile ) )
		osassert( false );
	ini.SetSection( "NEW_PACKDAT" );
	ini.ReadLine( "filename", path, MAX_PATH );
	fs = getfs_start( path, FS_RW );
	if( !fs )
	{
		osDebugOut( "PACK:��ʼ����<%s>ʧ��!\n", path );
		return false;
	}
	::InitializeCriticalSection( &g_csRFToBuf );
	
	// River @ 2010-5-17:
	::InitializeCriticalSection( &g_csMD5 );

	return true;

	unguard;
}

/** 
 *  д�ļ�����
 *
 *  \return ����ɹ�,����ԭʼ�ļ�����,���򷵻�-1.
 *  \param _filename �ļ���  �磺sound\\mysnd.wav
 *  \param _buf      ���ݻ�����
 *  \param _bufsize  ���ݴ�С(byte)
 */
int write_file_pack( char* _filename, BYTE* _buf,int _bufsize )
{
	guard;
	osassert( false );

#if PACK_USE_IN_ENGINE
	osDebugOut( "pack:���治֧��д�ļ�����...\n" );
	return 0;
#else

	std::string s( _filename );
	osassert( s.length() > 2 );
	if( s[0] == '\\' )
		s = s.substr(1,s.length() );
	
	//ȡ���ļ�����
	int pos = s.find_first_of( "\\" );
	if(pos<0){
		char msgstr[256] = {0};
		sprintf( msgstr, "��ͼ���ļ�%sд����ڣ����ļ���Ŀ¼����Ŀ¼�����ڰ�����ָ����ȷ���ļ�·����", _filename );
		MessageBox( NULL, msgstr, "pack system", MB_OK );
		return -1;
	}
	std::string FolderName = s.substr( 0, pos );

	//ȡ�ļ���
	int n = s.find_last_of( "\\" );
	std::string sfn = s.substr( n+1, s.length() );

	//дһ����ʱ�ļ�
	char tempName[256]={0};
	sprintf( tempName, "%s", sfn.c_str()  );
	FILE* fp = NULL;
	if( !(fp=fopen( tempName, "wb" )))
		return -1;
	int size = fwrite( _buf, 1,_bufsize,fp );
	fclose(fp);

	//�����Ƿ������Ч��pack����
	MapPackInfo::iterator it = g_mapPackInfo->find( FolderName );
	if( it != g_mapPackInfo->end() )
	{
		//�������
		FolderName += "\\";
		if( ! it->second.pPack->AddFile( (char*)FolderName.c_str(), (char*)sfn.c_str() ) )
		{
			DeleteFile( tempName );
			return -1;
		}
	}

	//ɾ����ʱ�ļ�
    DeleteFile( tempName );

	return size;

#endif

	unguard;
}



/** \brief
 *  �ͷŰ��ӿ�ʹ�õ��ڴ���Դ
 *  
 */
void release_packinterface_pack()
{
	guard;

	if( fs )
	{
		getfs_end( fs, 0 );
		fs = 0;
	}

	DeleteCriticalSection( &g_csRFToBuf );
	DeleteCriticalSection( &g_csMD5 );


	unguard;
}

//----------------------------------------------------------------------------------
/** 
 *  ��ʼ������������ϵͳ
 *
 *  \param _iniName �������ļ���,�ô������ļ������ҵ���Ϸ�д�����Щ��
 *  \return bool    ����ɹ�,����ture,ʧ�ܷ���false
 *  
 */
//bool init_updatePackSystem( char * _iniName )
//{
//	guard;
//
//	CIni inifile;
//	if( !inifile.Open( _iniName ) )
//	{
//		char buff[256] = {0};
//		sprintf( buff, "open ini file <%s> failed", _iniName );
//		MessageBox( NULL,buff,"Msg",MB_OK );
//		return false;
//	}
//	inifile.SetSection( "PACKINFO" );
//	char name[40] = {0};
//	char base_value1[MAX_PATH] = {0};
//	char path_value2[MAX_PATH] = {0};
//	PackInfo pi;
//	int i=0;
//	char currPath[MAX_PATH]= {0};
//	char s[MAX_PATH] ={0};
//	GetCurrentDirectory(MAX_PATH, currPath);
//	for(;;)
//	{
//		i++;
//		sprintf( name, "flag%d", i);
//		if(false==inifile.ReadLine( name, base_value1, MAX_PATH ) )
//		{
//			break;
//		}
//		sprintf( name, "path%d", i);
//		if(false==inifile.ReadLine( name, path_value2, MAX_PATH ) )
//		{
//			break;
//		}
//		memset(&pi,0,sizeof(pi));
//		strcpy(pi.basePath, base_value1);
//		strcat(pi.packFile, currPath);
//		strcat(pi.packFile, path_value2);
//		Pack * p = new Pack;
//		if(!p->OpenPack( pi.packFile ))
//		{
//			delete p;
//			p=NULL;
//		}
//		pi.pPack = p;
//		m_mapPackInfo_Update.insert(std::make_pair(pi.basePath,pi));
//	}
//	
//	return true;
//	unguard;
//}

//! �ͷŸ��°�ϵͳ
//bool release_updatePackSystem()
//{
//	guard;
//	MapPackInfo::iterator it;
//	for(it=m_mapPackInfo_Update.begin();it!=m_mapPackInfo_Update.end();++it)
//	{
//		if(it->second.pPack!=NULL)
//		{
//			it->second.pPack->ClosePack();
//			delete it->second.pPack;
//		}
//	}
//	m_mapPackInfo_Update.clear();
//
//	return true;
//
//	unguard;
//}

/** 
 *  ���°��ļ����÷������˵Ĵ�����ݸ��±��ذ���
 *
 *  \param _c_File �ӷ��������صĸ����ļ�·��
 *  \return int     ����ɹ�,����1,ʧ�ܷ���-1�޴��ļ�,������Ч�ļ�����-2
 *  
 */
//int update_packfile( char * _c_File )
//{
//	guard;
//
//	char str[256] = {0};
//	if( !file_exist( _c_File ) )
//	{
//		sprintf( str, "packFileOperate:update_packfile���ļ�<%s>������!", _c_File );
//		MessageBox( NULL,str,"����", MB_OK );				
//		osassert( false );
//		return -1;
//	}
//
//	HANDLE handle = NULL;
//	//Pack::SetFileAttr_ReadWrite( (char*)_c_File );
//	handle = ::CreateFile(_c_File,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
//	if( handle == INVALID_HANDLE_VALUE )
//	{
//		sprintf( str, "pack.dat:update_packfile�����ļ�<%s>����!", _c_File );
//		MessageBox( NULL,str,"����", MB_OK );
//		osassert( false );
//		return -1;
//	}
//	FileHeader_FromServer ff;
//	memset(&ff,0,sizeof(FileHeader_FromServer));
//	int currAddr=0;
//	char name[MAX_PATH] = {0};
//	Pack p;
//	p.ReadBound(&ff.dwFlag,currAddr,sizeof(ff.dwFlag),handle);
//	if(ff.dwFlag!=SERVER_UPDATE_FILE_HEADER_FLAG)
//	{
//		return -2;//����*.*c�ļ�
//	}
//	currAddr+=sizeof(ff.dwFlag);
//	p.ReadBound(&ff.PackFileNameLen,currAddr,sizeof(ff.PackFileNameLen),handle);
//	currAddr+=sizeof(ff.PackFileNameLen);
//	p.ReadBound(name,currAddr,ff.PackFileNameLen,handle);
//	currAddr=currAddr+ff.PackFileNameLen;
//	p.ReadBound( &ff.dwOriginalSize, currAddr,sizeof(ff.dwOriginalSize),handle);
//	currAddr+=sizeof(ff.dwCompressSize);
//	p.ReadBound( &ff.dwCompressSize, currAddr,sizeof(ff.dwCompressSize),handle);
//	currAddr+=sizeof(ff.dwOriginalSize);
//
//	BYTE* pCompress = new BYTE[ff.dwCompressSize];
//	p.ReadBound( pCompress, currAddr, ff.dwCompressSize, handle);
//	CloseHandle(handle);
//
//	bool bOk = false;
//	char first[MAX_PATH] = {0};
//	GetFirstFolder( name, first );
//	osassert( strcmp( name, "" ) != 0 );
//	osassert( strcmp( first, "" ) != 0 );
//	MapPackInfo::iterator it;
//	it = g_mapPackInfo->find( first );
//	if( it != g_mapPackInfo->end() )
//	{
//		if( it->second.pPack )
//		{
//			if( !it->second.pPack->AddFile( name, pCompress, ff.dwCompressSize, ff.dwOriginalSize ) )
//			{
//				bOk = false;
//				osDebugOut( "pack:����<%s>ʱʧ��, ��Ӧ���ļ�<%s>\n", _c_File, name );
//				delete [] pCompress;
//				osassert( false );
//			}
//			else
//			{
//				bOk = true;
//				osDebugOut( "pack:����������˸��µ��ļ�:<%s>\n", name );
//			}
//		}
//	}
//	else
//	{
//		osDebugOut( "��ָ���ĸ����ļ�<%s>,û�ҵ�Ŀ�İ�<%s>.\n", _c_File, first );
//		delete [] pCompress;
//		osassert( false );
//	}
//
//	delete [] pCompress;
//
//	if( bOk )
//		return 1;
//	else
//		return -1;
//
//	unguard;
//\}


//! ��ô����ļ���С
int  get_diskFileSize( char* _filename )
{
	guard;

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
		char str[256] = {0};
		sprintf( str, "ȱ���ļ�:%s", _filename );
		osassert( false );
	}

	FindClose( hFindHandle );

	return t_iSize;

	unguard;
}

