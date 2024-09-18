//=======================================================================================================
/**  \file
 *   Filename: packFileOperate.cpp
 *   Desc:     Osok引擎中操作文件的帮助函数和类.
 *
 *   His:      River created @ 4/27 2003.
 *
 *   "你绝对不能失败!!!"
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

#define ERROR_FILEINFO_ORIGINALSIZE "在此出错，由于文件信息的原始文件大小存储错误（在某处被强制修改），\n导致uncompress时缓冲区不够大而造成！"

//文件大小不超过4M
#define MAX_FILESIZE (0x400000)

static CRITICAL_SECTION  g_csRFToBuf;

// River @ 2010-5-17:md5转化需要的cs.
static CRITICAL_SECTION  g_csMD5;


//使用hash文件名
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

//字节到十六进制的ASCII码转换
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
// vector排序准则
//-------------------------------------------------------------------

// FreeInfo vector 按空闲区大小进行排序的准则。
bool lessSortSize( const FreeInfo& f1, const FreeInfo& f2 )
{
	return f1.freeSize < f2.freeSize;
}

// FreeInfo vector 按空闲区地址进行排序的准则。
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
*	新建包，写入包的头结构。
*
*	\param _chpName  新建包的名称。
*	\return bool     返回true表示创建成功，否则是false。
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
*	将文件的属性设为可读可写
*
* \param   _filename   文件名
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
*	打开包文件。
*
*	\param _chpName       要打开的包文件的名称。
*	\return bool          返回true表示打开成功，否则是false。
*
*	说明：打开文件并检查文件格式标志。
*
*/
bool Pack::OpenPack( char *_chpName )
{
	guard;
	char b[MAX_PATH] = {0};

	if( m_hFile != NULL )
	{
		MessageBox( NULL,"包已经打开，此次操作无效！","提示", MB_OK|MB_ICONWARNING );
		return false;
	}
	m_strPackName = _chpName;

	std::string infoFile( _chpName );
	if( infoFile.length() > 0 )
		*( infoFile.end() - 1 ) = 'i';
	
	//check pack file
	if( !IsFileExist(_chpName) )
	{
		MessageBox(NULL,"包文件不存在","提示",MB_OK);
		return false;
	}

	//check info file
	if( !IsFileExist((char*)infoFile.c_str() ) )
	{
		sprintf(b,"%s 文件不存在",infoFile.c_str() );
		MessageBox(NULL,b,"提示",MB_OK);
		return false;
	}

	//open info file...
	SetFileAttr_ReadWrite( (char*)infoFile.c_str() );
	HANDLE handle =::CreateFile( infoFile.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if( handle == INVALID_HANDLE_VALUE )
	{
		sprintf( b,"打开文件%s失败!",infoFile.c_str() );
		MessageBox(NULL,b,"提示",MB_OK);
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
		sprintf( b,"文件%s不是有效格式!", infoFile.c_str() );
		MessageBox(NULL,b,"提示",MB_OK);
		return false;
	}

	// 打开包文件
	SetFileAttr_ReadWrite( _chpName );
	Sleep( 50 );
	m_hFile=::CreateFile( _chpName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		sprintf( b,"打开文件文件%s失败!", _chpName );
		MessageBox( NULL, b, "提示", MB_OK );
		CloseHandle( handle );
		m_hFile = NULL;
		return false;
	}

	// 检查是否为有效文件
	ReadBound( &m_sPackHeader, 0, sizeof( PackHeader ), m_hFile );
	if( strcmp( m_sPackHeader.cFileFlag, HEADER_FLAG ) != 0 )
	{
		CloseHandle( handle );
		CloseHandle( m_hFile );
		m_hFile = NULL;
		MessageBox( NULL,"打开的不是(*.llp)文件！","提示",MB_OK|MB_ICONWARNING );
		return false;
	}

	// 两文件验证
	if( ih.validateCode != m_sPackHeader.validateCode )
	{
		CloseHandle( handle );
		CloseHandle( m_hFile );
		m_hFile = NULL;
		sprintf( b, "%s, llp与lli文件不匹配!", _chpName );
		MessageBox( NULL, b, "错误", MB_OK | MB_ICONWARNING );
		return false;
	}
	
	//装入信息
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
*	关闭一个包文件。
*
*	关闭并存储 pack file & Info file
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

	//重写文件头
	WriteBound( &m_sPackHeader, 0, sizeof(PackHeader), m_hFile );

	//写尾标志,实际长度
	WriteBound( END_FLAG, GetDataEndAddr(), strlen(END_FLAG), m_hFile );

	SetEndOfFile(m_hFile);

	// 关闭文件。
	CloseHandle(m_hFile);

	m_hFile = NULL;

	ClearUp();

#endif

	unguard;
}

/**\brief
*
*	清空成员变量文件句柄 
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
*	写入文件信息及空闲的信息，名字为pack名字的最后的一个字符改为'i'
*
* \return bool     返回true指示成功，否则为失败
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
		osDebugOut( "创建文件%s失败!\n", s.c_str() );
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
*	向包文件中写入一个文件。
*
*	\param  _filename  要向包内增加的文件名，磁盘的绝对路径
*   \param  _PackPath  包内的相对路径,如为"",则为包根目录
*
*	说明：压缩文件并写入包内.
*
*/
bool Pack::AddFile( char * _PackPath, char * _filename )
{
	guard;

	//要增加的文件是否存在?
	if( !IsFileExist(_filename) )
	{
		MessageBox( NULL,"你要增加的文件不存在！","提示",MB_OK );
		return false;
	}

	//打开指定的文件
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
	MakePath( path, _PackPath, name );//包内路径

	//将目录文件名改为小写
	std::string str( path );
	std::transform( str.begin(), str.end(), str.begin(), tolower );

	//压缩，得到大小, 缓冲区
	DWORD dwOutSize=(DWORD)( (float)filesize * 1.01f + 12 );

	// 分配文件缓冲区
	BYTE * pFileBuffer = new BYTE[filesize];
	memset( pFileBuffer, 0, filesize );

	BYTE * pCompressBuffer = new BYTE[dwOutSize];
	memset( pCompressBuffer,0, dwOutSize );

	// 读入数据
	ReadBound( pFileBuffer, 0, filesize, handle );

	// 压缩数据
	if( Z_MEM_ERROR==compress( pCompressBuffer, &dwOutSize, pFileBuffer, filesize ) )
	{
		::MessageBox(NULL,"AddFile分配输出缓冲区不足！","警告",MB_OK);
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
	//将目录文件名改为小写
	std::string str( _PackFileName );
	std::transform( str.begin(), str.end(), str.begin(), tolower );
	const char * path = str.c_str();

	DWORD dwOutSize = _dwCompressSize;
	DWORD filesize = _dwOriginalSize;
	
	//算文件信息
	DWORD i=0;
	BYTE * p = (BYTE*)_pCompressData;
	DWORD main_count = dwOutSize / m_sPackHeader.SizeType;
	DWORD remain_len = dwOutSize - main_count * m_sPackHeader.SizeType;

	//是否有空闲区可用
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

	//写入
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
				osDebugOut( "pack:AddFile:覆写_新 < 旧.时出错\n" );
				osassert( false );
			}

			m_listFreeInfo.push_back( free );
			it->second.m_dwOriginalSize = filesize;
			it->second.m_dwCompressSize = dwOutSize;
			it->second.m_main_count = main_count;
			it->second.m_remain_len = remain_len;
		}

		//覆写时空间不足
		else if( dwOutSize > size )
		{
			//有可用的空闲区
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
						osDebugOut( "pack:AddFile:覆写_新 > 旧.时出错\n" );
						osassert( false );
					}

					m_listFreeInfo.push_back( free );
				}
				free.freeSize = size;
				free.freeAddr = Addr;
					
				//TEST:
				if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
				{
					osDebugOut( "pack:AddFile:覆写_新 > 旧.保存旧空间时出错\n" );
					osassert( false );
				}

				m_listFreeInfo.push_back( free );

				it->second.m_dwFirstDataAdd = FreeAddr;
				it->second.m_dwOriginalSize = filesize;
				it->second.m_dwCompressSize = dwOutSize;
				it->second.m_main_count = main_count;
				it->second.m_remain_len = remain_len;
			}
			//无可用空闲区
			else
			{
				WriteBound( p, GetDataEndAddr(), dwOutSize, m_hFile );
				free.freeSize = size;
				free.freeAddr = Addr;
				
				//TEST:
				if( free.freeSize > MAX_FILESIZE || free.freeAddr > MAX_FILESIZE )
				{
					osDebugOut( "pack:AddFile:覆写_新 > 旧.无可用空闲空间时出错\n" );
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
	else //无此文件
	{
		std::string s = path;
		memcpy( fi.m_chName, path, strlen( path ) );

		//有空闲
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
					osDebugOut( "pack:AddFile:无此文件:有空闲,时出错\n" );
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
*	设置包的数据的尾地址,尾地址后存放文件尾标志串
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
*	获取包的数据的尾地址,尾地址后存放文件尾标志串
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
*	从包文件中删除一个文件。
*
*	\param  _filename   包内文件名，包内的相对路径
*   \return             成功返1,失败返回-1
*	说明：
*
*/
//int Pack::DeletePackFile( char * _FileName )
//{
//	guard;
//	DWORD srcAddr = 0;
//	FreeInfo free;
//	//查找文件的首地址
//	MapInfo::iterator pos;
//	pos = m_mapInfo.find(_FileName);
//	if( pos != m_mapInfo.end() )
//	{
//		free.freeAddr = pos->second.m_dwFirstDataAdd;
//		free.freeSize = pos->second.m_dwCompressSize;
//		m_listFreeInfo.push_back(free);
//
//		//从文件信息map中删除记录
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
*	用磁盘目录与基目标比对，获取多余的目录串，作为包内目录
*
* \param   _out   输出路径串，长－短
* \param  _big     长路径
* \param  _small 　　短路径
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
*	向包文件中增加文件夹及其子内容。
*
*	\param _PackPath        包内路径，如:pack\
*	\param _DiskPath        要添加目录的磁盘路径,如  c:\tool\test(\...\...) 
*	\param _DiskPathBase    要添加目录的磁盘路径，为_DiskPath的串子集,如  c:\tool\test
*
*	说明：
*
*/
//bool Pack::AddFolder( char * _PackPath , char * _DiskPath, char * _DiskPathBase )
//{
//	guard;
//	if(_DiskPath[0]=='\0' )
//	{
//		::MessageBox(NULL,"你要增加的文件夹路径不能设为空！","提示",MB_OK);
//		return false;
//	}
//	if(_DiskPathBase[0]=='\0' )
//	{
//		::MessageBox(NULL,"目录基（工作路径）不能为空！","提示",MB_OK);
//		return false;
//	}
//
//
//	std::string DiskFolder ;
//	if(0>GetDiskFolder(DiskFolder, std::string(_DiskPath),std::string(_DiskPathBase)) )
//	{
//		::MessageBox(NULL,"基目录设置不正确","提示",MB_OK);
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
//				// 目录，递归调用
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
*	从包文件中删除文件夹及其子内容。
*
*	\param _FolderName  文件目录   如：base\   或  base  将删除所有base文件夹下的文件
*
*	说明：
*
*/
//bool Pack::DeleteFolder( char * _FolderName )
//{
//	guard;
//	//查找文件路径包括 _FolderName　的所有信息块
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
//		MessageBox(NULL,"你要删除的文件夹不存在！","提示",MB_OK);
//	}
//	return true;
//	unguard;
//}

/**\brief
*
*	在_full内是否包含_pathbase且在　位于_full的首部
*
* \param  _full      母路径 
* \param  _pathbase  将与母路径比较的子路径
* \return bool     返回true指示_pathbase属于_full的子集，且子串位于0，否则为不在其内
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
*	从包文件中拷贝单个文件到磁盘。
*
*	\param _disk_dest       目的文件夹路径（绝对路径）
*	\param _pack_src_file   包内文件路径（包内相对路径）
*	\return   bool          返回true表示成功，否则失败。 
*                                           
*	说明：用此函数从包内拷贝单个文件到磁盘。
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
			MessageBox( NULL, ERROR_FILEINFO_ORIGINALSIZE, "错误", MB_OK );
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
*	从包文件中拷贝文件夹及其子内容到磁盘。
*
*	\param _disk_dest   目的文件夹（绝对路径）
*	\param _pack_src　　包内文件夹（包内相对路径）　
*	\return   bool  返回true表示打开成功，否则失败。 
*
*	说明：从包文件中拷贝文件夹及其子内容到磁盘,如果目的路径无指定的文件夹，创建它,拷贝
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
*	写LOG文件，空闲区信息,最小的这，最大的空闲片，空闲片的数量信息，
*              调试用
*
* \param   _logfile   日志文件名或路径名
* \return  bool                操作成功返true,失败返false
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
*	读取包中的一个文件到内存。
*	\param _InPackFileName                 要读取的文件的信息块的地址。
*	\param _pOutBuffer                     内存数据地址
*   \param _dwBufferSize                   内存数据的大小(文件大小)
*
*	说明：循环文件的所有的数据块，读出数据、解压、写入外部文件。
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
		osDebugOut( "Pack缺少文件:%s\n", _InPackFileName );
		osassert( false );
	}

	if( _dwBufferSize < fi.m_dwOriginalSize )
	{
		osDebugOut( "缓冲区太小:%s, size:%d, but,at least:%d\n", _InPackFileName, _dwBufferSize, fi.m_dwOriginalSize );
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
*	合成路径,两个字符串相加
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
*	在包中查找指定的文件。
*	\param _FileName             要查找的文件的名称，在包中的名称。相对路径
*	\return PosInfo              文件的位置及大小信息。
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
*	获得文件的信息。
*	\param _FileName             要查找的文件的名称，在包中的名称。相对路径
*	\return FileInfo              文件的信息。
*
*
*/
FileInfo Pack::FindFileInfo(char * _filename )
{
	guard;

	//将目录文件名改为小写
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
*	检杳指定的磁盘文件是否存在？
*
* \param _FileName  待检查的文件名
* \return bool     返回true指示文件存在，否则为不存在
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
*	读取包文件中某一范围数据。
*
*	\param _pBuffer       数据缓冲区。
*	\param _dwStartAdd   起始位置。
*	\param _dwNum          要读的数据的大小。
*   \param _handle       文件句柄
*
*	说明：先定位文件指针到_dwStartAdd，然后读出_dwNum数量的数据。
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
*	写入数据到包文件中的某一范围。
*
*	\param _pBuffer          数据缓冲区。
*	\param _dwStartAdd   起始位置。
*	\param _dwNum          要写的数据的大小。
*   \param _handle       文件句柄
*
*	说明：先定位文件指针到_dwStartAdd，然后写入_dwNum数量的数据。
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
*	获得文件的个数。
*	\return DWORD   包内文件的个数。
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
*	获得m_mapInfo,用于打包工具向列表添加item用
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
*	文件的改名
*	\param _newName  在包内的新名字
*   \param _oldName  在包内的原来的名字
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
//	//是否已经存在?
//	pos = m_mapInfo.find( _newName );
//	if( pos != m_mapInfo.end() )
//	{
//		char msg[256] = {0};
//		sprintf( msg, "已存在文件：<%s>，改名失败!", _newName );
//		MessageBox( NULL, msg, "失败", MB_OK );
//		return;
//	}
//
//	FileInfo info;
//
//	//移除文件名字信息
//	pos = m_mapInfo.find( _oldName );
//	if( pos != m_mapInfo.end() )
//	{
//		info = pos->second;
//		m_mapInfo.erase( pos );
//	}
//	
//	//重新添加
//	strcpy( info.m_chName, _newName );
//	m_mapInfo.insert( std::make_pair( std::string( _newName ), info ) );
//	unguard;
//}

/**\brief
*
*	清空包内文件的碎片，减少包的大小。使包内文件紧密相连
*   此函数调用时创建了一个临时文件，调用结束自动删除
*
*  \return bool 整理完成成功返true,失败返false;
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
*	服务器端使用此函数进行将单个的文件压缩.输出时将文件名变为
*   
*   \param _DiskFile  　　磁盘文件的名字
*   \param _fileNameInPack    在包内的文件名
*   \return bool          成功返true,失败false
*
*/
//bool Pack::ServerCompressFile( char * _DiskFile,  char * _FileNameInPack )
//{
//	guard;
//	if( !IsFileExist(_DiskFile) )
//	{
//		MessageBox( NULL,"你要增加的文件不存在！","提示",MB_OK );
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
//		::MessageBox(NULL,"ServerCompressFile分配输出缓冲区不足！","警告",MB_OK);
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
*	服务器端使用此函数将 *.*c_ 文件解压回原文件，
*
*   \param  _c_File  　　服务器使用ServerCompressFile函数产生的压缩包
*   \return bool          成功返true,失败false
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
//		::MessageBox(NULL,"你要解压缩的文件不是.lsc文件,或改文件已损坏！","解压失败",MB_OK);
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
// 以下是包的接口函数, 操作步骤：初始化接口，操作，释放接口
//--------------------------------------------------------------------
//! 如果是MFC引用这个工程的话，这两个全局变量在未进入栈时就在堆上
//! 分配了内存，导致MFC的堆检查机制失效，报出内存泄漏的错误
// MapPackInfo   g_mapPackInfo_Update; //初始化更新时用
// MapPackInfo   g_mapPackInfo;        //运行时用　

MapPackInfo*   g_mapPackInfo_Update = NULL; //初始化更新时用
MapPackInfo*   g_mapPackInfo = NULL;        //运行时用　


//　获取文件的路径的首段文件夹，如: c:\tool\test\1.txt ->  tool
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
 *  查看一个文件是否存在.
 *  
 *  \param  _filename  包内路径
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

	// River added @ 2009-11-25:用于处理多线程的安全问题
	::EnterCriticalSection( &g_sFileReadCS );
	int n = getfs_file_exists( fs, t_hashstr );
	::LeaveCriticalSection( &g_sFileReadCS );

	if( n==0 )
	{
		osDebugOut( "h2文件不存在:file_exist_pack:<%s><%s>\n",t_hashstr, t_fileName );
		::LeaveCriticalSection( &g_csRFToBuf );
		return false;
	}


	::LeaveCriticalSection( &g_csRFToBuf );
	return true;


	unguard;
}

/** 
 *  得到文件长度的函数,可以是*.osk文件中的文件,也可以单独的文件.
 *  
 *  \return  如果处理错误,返回-1,如果正确,返回文件长度,以byte为单位.
 *  \param   _filename 要处理的文件的名字.
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

	// River @ 2010-4-1:去掉此段代码，否则内部可能死锁.
	/*
	if( !file_exist_pack( (char*)t_fileName ) )
	{
		osDebugOut( "get_fileSize_pack:文件没找到:<%s>\n", _filename );
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
		osDebugOut( "查找文件<%s>返回值有错....\n",_filename );
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
 *  调入默认的相关文件。
 *
 *  根据文件的扩展名来调入。对于一些关键性的文件，仍然需要assert并退出。
 *  
 */
int  load_defaut_file_pack( char* _filename,BYTE* _buf,int _bufsize )
{
	guard;

	s_string     t_str = _filename;
	int          t_idx;

	t_idx = (int)t_str.rfind( "." );
	t_str = t_str.substr( t_idx+1,t_str.length() );

	MessageBox( NULL,_filename,"文件找不到，调入默认文件",MB_OK );

	// 分别调入不同的类型的文件
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

	osDebugOut( "\n文件<%s>是关键文件，在目录找不到...请重新下载最新的客户端修复错误\n",_filename );
	osassert( false );

	return -1;

	unguard;
}



/** 
 *  把文件读入到一个缓冲区中.返回读入的文件的长度。
 *
 *  \return 如果成功,返回文件长度,输入BUFFER长度不足返回-1.无此文件返回-2
 *  \param _buf 文件要读入的缓冲区,已经分配足够内存.
 *  \param _bufsize 缓冲区的大小，单位：字节.
 */
int read_fileToBuf_pack( char* _filename,BYTE* _buf,int _bufsize )
{
	guard;

	// tzz:
	// 将临界区的进入和离开用一个类管理
	// 避免在 EnterCriticalSection 和 LeaveCriticalSection 之中有osassert 抛出异常的时候
	// 程序被陷入到系统内核
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
	// 将临界区的进入和离开用一个类管理
	// 避免在 EnterCriticalSection 和 LeaveCriticalSection 之中有osassert 抛出异常的时候
	// 程序被陷入到系统内核
	//
	CCriticalSec t_readSec(&g_sFileReadCS);
	// River @ 2009-11-24:加入此处，避免多线程读入文件出错.
	//::EnterCriticalSection( &g_sFileReadCS );

	if( !getfs_file_exists( fs, t_hashstr ) )
	{
		char t_sz[256];
		getfs_perror( fs,t_sz );
		osDebugOut( "h3文件不存在:<%s><%s><%s>getfs_file_exists\n",t_sz,hashstr,t_fileName );
	}
	int f1 = getfs_file_open( fs, t_hashstr, O_RDONLY );
	if( f1 < 0 )
	{
		char t_sz[256];
		getfs_perror( fs,t_sz );
		osDebugOut( "文件出错:<%s><%s>\n",t_sz,t_fileName );
		// 如果可以读入替代文件。
		if( g_bUseDefaultFile )
		{
			// tzz CCriticalSec 的析构函数会自动 LeaveCriticalSection
			//
			// River @ 2009-11-24:加入此处，避免多线程读入文件出错.
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


		MessageBox( NULL,"请在接下来的错误描述输入框，输入您的QQ号,您会获得2000哆币的奖励，请您提交错误后《重起电脑》，即可顺利进行游戏。如果《重起电脑》后仍然进不去游戏，请您保存好当前的客户端，不要着急重新安装，并联系客服，您可以得到10000哆币的奖励。","非常抱歉",MB_OK );
		
		//River @ 2010-9-16: 空指针，用于能够产生dump文件
		(*(char*)0) = NULL;

		osassert( false );
	}

	getfs_file_close( fs, f1 );

	// tzz CCriticalSec 的析构函数会自动 LeaveCriticalSection
	//
	// River @ 2009-11-24:加入此处，避免多线程读入文件出错.
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
 *  客户端 初始化包的接口,在此进行内存的分配
 *  
 *  \param  iniFile  包的配置文件
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
		osDebugOut( "PACK:初始化包<%s>失败!\n", path );
		return false;
	}
	::InitializeCriticalSection( &g_csRFToBuf );
	
	// River @ 2010-5-17:
	::InitializeCriticalSection( &g_csMD5 );

	return true;

	unguard;
}

/** 
 *  写文件到包
 *
 *  \return 如果成功,返回原始文件长度,否则返回-1.
 *  \param _filename 文件名  如：sound\\mysnd.wav
 *  \param _buf      数据缓冲区
 *  \param _bufsize  数据大小(byte)
 */
int write_file_pack( char* _filename, BYTE* _buf,int _bufsize )
{
	guard;
	osassert( false );

#if PACK_USE_IN_ENGINE
	osDebugOut( "pack:引擎不支持写文件操作...\n" );
	return 0;
#else

	std::string s( _filename );
	osassert( s.length() > 2 );
	if( s[0] == '\\' )
		s = s.substr(1,s.length() );
	
	//取得文件夹名
	int pos = s.find_first_of( "\\" );
	if(pos<0){
		char msgstr[256] = {0};
		sprintf( msgstr, "试图将文件%s写入包内，但文件无目录，根目录不存在包。请指定正确的文件路径！", _filename );
		MessageBox( NULL, msgstr, "pack system", MB_OK );
		return -1;
	}
	std::string FolderName = s.substr( 0, pos );

	//取文件名
	int n = s.find_last_of( "\\" );
	std::string sfn = s.substr( n+1, s.length() );

	//写一个临时文件
	char tempName[256]={0};
	sprintf( tempName, "%s", sfn.c_str()  );
	FILE* fp = NULL;
	if( !(fp=fopen( tempName, "wb" )))
		return -1;
	int size = fwrite( _buf, 1,_bufsize,fp );
	fclose(fp);

	//查找是否存在有效的pack对象
	MapPackInfo::iterator it = g_mapPackInfo->find( FolderName );
	if( it != g_mapPackInfo->end() )
	{
		//加入包内
		FolderName += "\\";
		if( ! it->second.pPack->AddFile( (char*)FolderName.c_str(), (char*)sfn.c_str() ) )
		{
			DeleteFile( tempName );
			return -1;
		}
	}

	//删除临时文件
    DeleteFile( tempName );

	return size;

#endif

	unguard;
}



/** \brief
 *  释放包接口使用的内存资源
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
 *  初始化升级包更新系统
 *
 *  \param _iniName 包配置文件名,用此配置文件可以找到游戏中存在那些包
 *  \return bool    如果成功,返回ture,失败返回false
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

//! 释放更新包系统
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
 *  更新包文件，用服务器端的打包数据更新本地包，
 *
 *  \param _c_File 从服务器下载的更新文件路径
 *  \return int     如果成功,返回1,失败返回-1无此文件,不是有效文件返回-2
 *  
 */
//int update_packfile( char * _c_File )
//{
//	guard;
//
//	char str[256] = {0};
//	if( !file_exist( _c_File ) )
//	{
//		sprintf( str, "packFileOperate:update_packfile：文件<%s>不存在!", _c_File );
//		MessageBox( NULL,str,"错误", MB_OK );				
//		osassert( false );
//		return -1;
//	}
//
//	HANDLE handle = NULL;
//	//Pack::SetFileAttr_ReadWrite( (char*)_c_File );
//	handle = ::CreateFile(_c_File,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
//	if( handle == INVALID_HANDLE_VALUE )
//	{
//		sprintf( str, "pack.dat:update_packfile：打开文件<%s>出错!", _c_File );
//		MessageBox( NULL,str,"错误", MB_OK );
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
//		return -2;//不是*.*c文件
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
//				osDebugOut( "pack:更新<%s>时失败, 对应包文件<%s>\n", _c_File, name );
//				delete [] pCompress;
//				osassert( false );
//			}
//			else
//			{
//				bOk = true;
//				osDebugOut( "pack:加入服务器端更新的文件:<%s>\n", name );
//			}
//		}
//	}
//	else
//	{
//		osDebugOut( "从指定的更新文件<%s>,没找到目的包<%s>.\n", _c_File, first );
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


//! 获得磁盘文件大小
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
		sprintf( str, "缺少文件:%s", _filename );
		osassert( false );
	}

	FindClose( hFindHandle );

	return t_iSize;

	unguard;
}

