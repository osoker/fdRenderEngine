#include "StdAfx.h"
#include "xlsexportfile.h"

XlsExportFile::XlsExportFile(void) : 
file_(0),
colCount_(0)
{
}

XlsExportFile::~XlsExportFile(void)
{
	guard;
	close();
	unguard;
}

// 打开文件，colNum为列的个数，列间由tab分开
bool XlsExportFile::openXls( const char* fname, int colNum )
{
	guard;

	int tempSize = 1024*1024*2;
	m_fileBuffer = new BYTE[tempSize];

	m_filesize = read_fileToBuf( (char *)fname, m_fileBuffer, tempSize );

	m_fileCurPos = m_fileBuffer;

	osassert( m_filesize<=tempSize );

	colCount_ = colNum;

	return true;

	unguard;
}

// 关闭文件
void XlsExportFile::closeXls( void )
{
	guard;

	if( m_fileBuffer )
		delete[] m_fileBuffer;
	colCount_ = 0;

	unguard;
}

// 文件是否结束
bool XlsExportFile::isXlsEOF( void )
{
	guard;

	if( (m_fileCurPos-m_fileBuffer)>=m_filesize )
		return true;
	return false;

	unguard;
}

// 读一行,返回本行是不是完整行
bool XlsExportFile::readXlsLine( std::vector<std::string>& val )
{
	guard;

	if( isXlsEOF() )
		return false;

	val.clear();

	int i = 0;
	memset( line_, 0, 1024 );
	for( char* p = line_; ; ++p )
	{
		if( isXlsEOF() )
		{
			(*p) = 0;
			break;
		}
		READ_MEM_OFF( p, m_fileCurPos, sizeof( char ) );
		if( (*p) == 13 )
		{
			(*p) = 0;
		}
		if( (*p) == 10 )
		{
			(*p) = 0;
			break;
		}
		i ++;
	}

	if( strlen( line_ ) <= 0 ) 
		return false;

	std::string str = line_;
	int end		= -1;
	for( int i = 0; i < colCount_; ++i )
	{
		if( end+1 >= (int)str.size() )
			val.push_back( "" );
		else
		{
			str = str.substr( end+1, str.size() );

			end = (int)str.find( 9 );  //tab
			if( end == 0 ) 
				val.push_back( "" );
			else
			{
				if( end < 0 )end = (int)str.size();
				val.push_back( str.substr( 0, end ) );
			}
		}
	}

	return true;
	unguard;
}

// 打开文件，colNum为列的个数，列间由tab分开
bool XlsExportFile::open( const char* fname, int colNum, bool write )
{
	guard;
	if( file_ )fclose( file_ );

	if( write )
		file_ = fopen( fname, "wb" );
	else
		file_ = fopen( fname, "rb" );
	if( !file_ ) return false;

	colCount_ = colNum;

	return true;
	unguard;
}


// 关闭文件
void XlsExportFile::close( void )
{
	guard;
	if( file_ )fclose( file_ );
	file_ = 0;
	colCount_ = 0;
	unguard;
}

// 文件是否结束
bool XlsExportFile::isEOF( void )
{
	guard;
	if( !file_ )return true;
	if( feof( file_ ) )return true;

	return false;
	unguard;
}

// 读一行,返回本行是不是完整行
bool XlsExportFile::readLine( std::vector<std::string>& val )
{
	guard;
	if( !file_ )return false;

	val.clear();

	int i = 0;
	memset( line_, 0, 1024 );
	for( char* p = line_; ; ++p )
	{
		if( feof( file_ ) )
		{
			(*p) = 0;
			break;
		}
		fread( p, sizeof( char ), 1, file_ );
		if( (*p) == 13 )
		{
			(*p) = 0;
	//		break;
		}
		if( (*p) == 10 )
		{
			(*p) = 0;
			break;
		}
		i ++;
	}

	if( strlen( line_ ) <= 0 ) 
		return false;

	std::string str = line_;
	int end		= -1;
	for( int i = 0; i < colCount_; ++i )
	{
		if( end+1 >= (int)str.size() )
			val.push_back( "" );
		else
		{
			str = str.substr( end+1, str.size() );

			end = (int)str.find( 9 );
			if( end == 0 ) 
				val.push_back( "" );
			else
			{
				if( end < 0 )end = (int)str.size();
				val.push_back( str.substr( 0, end ) );
			}
		}
	}

	return true;
	unguard;
}

// 写一行
bool XlsExportFile::writeLine( std::vector<std::string>& val )
{
	guard;
	if( !file_ )return false;
	if( val.size() != colCount_ )return false;
	static char buf[256];

	std::vector<std::string>::iterator it, end = val.end();
	for( it = val.begin(); it != end; ++it )
	{
		if( it->length() > 0 )
		{
			fwrite( it->c_str(), it->length(), 1, file_ );
		}
		if( it == end - 1 ) continue; 
		strcpy( buf, "\t" );
		fwrite( buf, strlen(buf), 1, file_ );
	}

	strcpy( buf, "\r\n" );
	fwrite( buf, strlen(buf), 1, file_ );

	return true;
	unguard;
}
