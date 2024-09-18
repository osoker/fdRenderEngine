
# include "stdafx.h"
#include "../include/sgDebug.h"
#include "../include/IniFile.h"
////////////////////




////////////////////
CIniFile::CIniFile()
{
	// 文件指针
	m_fp = NULL;
	// 是否是读模式
	m_bRead = true;
}
CIniFile::~CIniFile()
{
	if( m_fp != NULL )
	{
		CloseFile();
	}
}
////////////////////


////////////////////
// 属性
FILE *CIniFile::GetFilePointer()
{
	return m_fp;
}
bool CIniFile::IfRead()
{
	return m_bRead;
}
////////////////////


////////////////////
// 文件操作
// 打开文件读
// filename	:	文件名
// returns	:	成功为真
bool CIniFile::OpenFileRead( const char *filename )
{
	guard;//( CIniFile::OpenFileRead() );

	char msg[_MAX_STRING];
	assert( m_fp == NULL );
	
	m_fp = fopen( filename, "rt" );
	
	if( m_fp == NULL )
	{
		sprintf( msg, "CIniFile::OpenFileRead() Error <%d> : Could not open file for read : <%s>\n",::GetLastError(),filename );
		osDebugOut_directly( msg );
		return false;
	}
	m_bRead = true;
	
	m_bReadMem = false;

	return true;

	unguard;
}

/**
* 从内存块读入ini信息.
*
*/
void CIniFile::OpenMemRead( BYTE *_dstart,int _isize )
{
	guard;//( CIniFile::OpenMemRead() );
	osassert( _dstart );
	osassert( _isize>=0 );

	this->m_pData = _dstart;
	this->m_iSize = _isize;

	m_bReadMem = true;
	this->m_iP = 0;

	unguard;
}


// 打开文件写
// filename	:	文件名
// returns	:	成功为真
bool CIniFile::OpenFileWrite( const char *filename )
{
	char msg[_MAX_STRING];
	assert( m_fp == NULL );
	m_fp = fopen( filename, "wt" );
	if( m_fp == NULL )
	{
		sprintf( msg, "CIniFile::OpenFileWrite() Error (0) : Could not open file for write : <%s>\n", filename );
		OutputDebugString( msg );
		return false;
	}
	m_bRead = false;

	m_bReadMem = false;
	
	return true;
}
// 关闭文件
void CIniFile::CloseFile()
{

	if( !this->m_bReadMem )
	{
		assert( m_fp != NULL );

		fclose( m_fp );
		m_fp = NULL;
		m_bRead = true;
	}

	// 如果是内存操作ini文件,Do nothing.

}

// 读操作
// 文件指针跳到某一段，文件指针从当前位置开始查找
// section	:	段名
// returns	:	找到该段为真
// 函数效率很有问题,不过这个函数主要用于关卡调入阶段.
bool CIniFile::ReadSection(  char *section )
{
	//char msg[_MAX_STRING];
	char rval;
	int nCount;
	bool bFound;
	bool bBegin;
	char Section[_MAX_STRING];
	
	if( !m_bReadMem )
	{
		assert( m_fp );
		assert( m_bRead == true );

		if ( m_fp == NULL )
			return false;
	}
	

	//
	// 如果是内存操作模式,设备内存指针指针为0,
	// 如果是文件操作模式,设置文件指针到文件开头.
	//
	if( this->m_bReadMem )
	{
		this->m_iP = 0;
	}
	else
	{
		if( fseek( m_fp,0,SEEK_SET ) )
			return false;
	}

	
	bFound = false;
	bBegin = false;
	nCount = 0;
	
	// 如果是从内存中读入Section.
	if( m_bReadMem )
	{

		while( bFound == false )
		{
			if( m_iP == this->m_iSize )
			{
				//OutputDebugString( "Can't Find the Section " );
//				osDebugOut( "%s [%s]\n","Can't Find the Section",section );
				return false;
			}

			rval = this->m_pData[m_iP];

			// Increase the Pointer.
			m_iP++;                     

			// 找到段标志
			if( rval == '[' )
			{
				bBegin = true;

				continue;
			}
			// 找到段结束标志
			if( rval == ']' )
			{
				Section[nCount] = 0;
				nCount = 0;
				bBegin = false;
				//added by no3eye。1。12。2001
				//如果Section是空的话就自动找到一个段并返回
				if ( stricmp(section,"")==0)
				{
					strcpy( section,Section);
					bFound = true;
				}
				else
				{
					if( strcmp( Section, section ) == 0 )
					{	// 找到需要的段
						bFound = true;
					}
				}
				continue;
			}
			// 段内容
			// 如果长度过长，则不加了
			if( bBegin == true && nCount < _MAX_STRING )
			{
				Section[nCount] = rval;
				nCount++;
			}


		}

		return true;
	}
	

	// 从文件中读入Section.
	while( bFound == false )
	{
		// 到达文件尾
		if( (rval = (char)fgetc(m_fp)) == EOF )
		{
			//sprintf( msg, "CIniFile::ReadSection() Error(0) : Reach the end of the file in reading section <%s>\n", section );
			//OutputDebugString( msg );
			return false;
		}
		
		// 找到段标志
		if( rval == '[' )
		{
			bBegin = true;
			continue;
		}
		// 找到段结束标志
		if( rval == ']' )
		{
			Section[nCount] = 0;
			nCount = 0;
			bBegin = false;
			//added by no3eye。1。12。2001
			//如果Section是空的话就自动找到一个段并返回
			if ( stricmp(section,"")==0)
			{
				strcpy( section,Section);
				bFound = true;
			}
			else
			{
				if( stricmp( Section, section ) == 0 )
				{	// 找到需要的段
					bFound = true;
				}
			}
			continue;
		}
		// 段内容
		// 如果长度过长，则不加了
		if( bBegin == true && nCount < _MAX_STRING )
		{
			Section[nCount] = rval;
			nCount++;
		}
	}
	
	return true;
}
// 读出域名和取值
// name		:	域名，如果为空，则缺省读下一个域
// value	:	取值字符串
// nSize	:	取值字符串的长度，如果长度小于取值字符串(Value)的真实长度，则会出现致命错误
// returns	:	读到该段为真
bool CIniFile::ReadLine( const char *name, char *value, int nSize )
{
	int rval;
	int nCount;
	bool bFound;
	char Name[_MAX_STRING];
	
	if( !this->m_bReadMem )
	{
		assert( m_fp );
		assert( m_bRead == true );
	}
	
	// 找到域名
	nCount = 0;
	bFound = false;
	bool bno=false;



	// 如果是从内存谋取数据.
	if( this->m_bReadMem )
	{
		// River @ 2006-8-23:为了可以乱序读入ini文件,记录指针位置
		const int  t_iPBack = m_iP;

		// 如果是从文件谋取数据.
		while( bFound == false )
		{
			// 如果到达文件结束.
			if( m_iP == this->m_iSize )
			{
				//OutputDebugString( "Reach the end of file " );
				m_iP = t_iPBack;
				return false;
			}

			rval = this->m_pData[m_iP];

			// Increase the Pointer.
			m_iP++;                     

			if(rval == '['){
				m_iP = t_iPBack;
				return false;
			}

			// 找到域结束
			if( rval == '=' )
			{
				Name[nCount] = 0;
				nCount = 0;
				if( strcmp( Name, name ) == 0 )
				{
					bFound = true;
				}
				//added by no3eye 1.29.2001
				else
				{
					nCount = 0;
					bno=true;
				}
				continue;
			}
			// 跳过行结束符13.
			if( rval == 13 )
			{
				continue;
			}
			// 跳过空行
			if( rval == '\n' )
			{
				if (bno == true)
					nCount=0;
				continue;
			}

			//'#' added by tzz @ 2008-5-26 for noting
			//';' added by tzz @ 2010-05-29 for noting
			if( rval == '#' || rval == ';'){
				while(m_pData[m_iP] != '\n' && m_pData[m_iP] != '\r' && m_iP < m_iSize){
					m_iP++;
				}
				if (bno == true){
					nCount = 0;
				}
				continue;

			}
			// 域内容
			// 如果长度过长，则不加了
			if( nCount < _MAX_STRING )
			{
				Name[nCount] = (char)rval;
				nCount++;
			}
		}

		// 读取值
		nCount=0;
		bFound = false;
		while( bFound == false )
		{
			rval = this->m_pData[m_iP];
			m_iP++;

			// 如果使用"rb"模式读取文件,则每一行产生字符13.
			//'#' added by tzz @ 2008-5-26 for noting
			if( rval == EOF || rval == '\n' || rval == 13 || rval == '#' || rval == ';')
			{
				value[nCount] = 0;
				bFound = true;
				continue;
			}
			// 取值内容
			// 如果长度过长，则不加了
			if( nCount < nSize-1 )
			{
				value[nCount] = (char)rval;
				nCount++;
			}
		}

		m_iP = t_iPBack;

		return true;

	}



	// 如果是从文件谋取数据.
	// River @ 2006-8-23:为了可以乱序读入ini文件,记录指针位置
	const int   t_iFileOffset = ftell( m_fp );

	while( bFound == false )
	{
		if( (rval = fgetc(m_fp)) == EOF )
		{
		//	sprintf( msg, 
		//		"CIniFile::ReadLine() Error(0) : Reach the end of the file in reading named <%s>\n", name );
		//	OutputDebugString( msg );

			// River @ 2006-8-23:为了可以乱序读入相应的ini字段
			fseek( m_fp,t_iFileOffset,SEEK_SET );
			return false;
		}

		if(rval == '['){
			fseek( m_fp,t_iFileOffset,SEEK_SET );
			return false;
		}

		// 找到域结束
		if( rval == '=' )
		{
			Name[nCount] = 0;
			nCount = 0;
			if( stricmp( Name, name ) == 0 )
			{
				bFound = true;
			}
			//added by no3eye 1.29.2001
			else
			{
				nCount = 0;
				bno=true;
			}
			continue;
		}

		// 跳过空行
		if( rval == '\n' )
		{
			if (bno == true)
				nCount=0;
			continue;
		}
		//'#' added by tzz @ 2008-5-26 for noting
		if( rval == '#' ||  rval == ';'){
			rval = fgetc(m_fp);
			while(rval != EOF && rval != '\n' && rval != '\r' ){
				rval = fgetc(m_fp);
			}
			if (bno == true){
				nCount = 0;
			}
			continue;

		}
		// 域内容
		// 如果长度过长，则不加了
		if( nCount < _MAX_STRING )
		{
			Name[nCount] = (char)rval;
			nCount++;
		}
	}
	
	// 读取值
	nCount=0;
	bFound = false;
	while( bFound == false )
	{
		rval = fgetc(m_fp);
		//'#' added by tzz @ 2008-5-26 for noting
		if( rval == EOF || rval == '\n' || rval == '#' || rval == ';')
		{
			value[nCount] = 0;
			bFound = true;
			continue;
		}
		// 取值内容
		// 如果长度过长，则不加了
		if( nCount < nSize-1 )
		{
			value[nCount] = (char)rval;
			nCount++;
		}
	}
	
	// River @ 2006-8-23:为了可以乱序读入相应的ini字段
	fseek( m_fp,t_iFileOffset,SEEK_SET );

	return true;
}

//读入字符。
bool CIniFile::ReadLine( const char *name, char* value )
{
	char Value[_MAX_STRING];
	if( ReadLine( name, Value, _MAX_STRING ) == false )
		return false;
	*value = Value[0];
	return true;
}


bool CIniFile::ReadLine( const char *name, int *value )
{
	char Value[_MAX_STRING];
	if( ReadLine( name, Value, _MAX_STRING ) == false )
		return false;
	*value = atoi( Value );
	return true;
}
bool CIniFile::ReadLine( const char *name, float *value )
{
	char Value[_MAX_STRING];
	if( ReadLine( name, Value, _MAX_STRING ) == false )
		return false;
	*value = (float)atof( Value );
	return true;
}
bool CIniFile::ReadLine( const char *name, double *value )
{
	char Value[_MAX_STRING];
	if( ReadLine( name, Value, _MAX_STRING ) == false )
		return false;
	*value = atof( Value );
	return true;
}
// 跳到文件头
void CIniFile::ReadStart()
{
	assert( m_fp );
	assert( m_bRead == false );
	fseek( m_fp, 0, SEEK_SET );
}
////////////////////


////////////////////
// 写操作
// 写入某一段
// section	:	段名
void CIniFile::WriteSection( const char * section )
{
	assert( m_fp );
	assert( m_bRead == false );
	fprintf( m_fp, "[%s]\n", section );
}

// 写入域名和取值
// name		:	域名
// value	:	取值
void CIniFile::WriteLine( const char *name, const char *value )
{
	assert( m_fp );
	assert( m_bRead == false );
	fprintf( m_fp, "%s=%s\n", name, value );
}
void CIniFile::WriteLine( const char *name, int value )
{
	assert( m_fp );
	assert( m_bRead == false );
	fprintf( m_fp, "%s=%d\n", name, value );
}
void CIniFile::WriteLine( const char *name, float value )
{
	assert( m_fp );
	assert( m_bRead == false );
	fprintf( m_fp, "%s=%f\n", name, value );
}
void CIniFile::WriteLine( const char *name, double value )
{
	assert( m_fp );
	assert( m_bRead == false );
	fprintf( m_fp, "%s=%e\n", name, value );
}
// 写入空行
void CIniFile::WriteLineSeperator()
{
	assert( m_fp );
	assert( m_bRead == false );
	fprintf( m_fp, "\n" );
}
////////////////////



