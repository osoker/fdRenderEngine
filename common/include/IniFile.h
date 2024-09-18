////////////////////
////////////////////
//----------------------------------------------------------------------------------------------
/** \file
 *  Filename: IniFile.h
 *  Desc:     读入和写入Ini文件的类.
 *  His:      
 *
 *  2003.4.30: 加入了内存的Ini文件操作,从内存块读入ini文件信息.
 *
 */
//----------------------------------------------------------------------------------------------
#ifndef __EMINIFILE_H__
#define __EMINIFILE_H__
////////////////////


////////////////////
#include <stdio.h>		// FILE
////////////////////
# include "../common.h"

#define _MAX_STRING					256



/**
 *  用这个类来做读入和写进Ini文件的工作.
 *  用Ini文件可以做一些比如初始化信息之类的工作.
 *  
 */
class COMMON_API CIniFile
{
private:
	//! 文件指针
	FILE *m_fp;
	//! 是否是读模式
	bool m_bRead;


	//! 如果从内存读入数据,此处的内存数据指针.
	BYTE* m_pData;
	
	//! 内存文件的长度.
	int   m_iSize;

	//! 内存数据读取需要的指针.相当于文件位置的指针.
	int   m_iP;

	//! 是不是从内存读入数据.
	bool  m_bReadMem;

	
public:
	
	CIniFile();
	~CIniFile();
	
	/**
	 * 文件操作
	 * 打开文件读
	 * /param filename	:	文件名
	 * /return bool	:	成功为真
	 */
	bool OpenFileRead( const char *filename );

	/**
	 * 从内存块读入ini信息.
	 *
	 */
	void OpenMemRead( BYTE *_dstart,int _isize );

	/** 
	 * 打开文件写
	 * /param  filename	:	文件名
	 * /return bool	:	成功为真
	 */
	bool OpenFileWrite( const char *filename );
	/**  关闭文件
	 */
	void CloseFile();
	
	/**
	 *  得到文件的指针.
	 */
	FILE *GetFilePointer();
	bool IfRead();
	
	/** 
	 * 读操作
	 * 文件指针跳到某一段，文件指针从当前位置开始查找
	 * /param section	:	段名
	 * /return bool :	找到该段为真
	 */
	bool ReadSection(  char *section );
	/**
	 * 读出域名和取值
	 * /param  name		:	域名，如果为空，则缺省读下一个域
	 * /param  value	:	取值字符串
	 * /param  nSize	:	取值字符串的长度，如果长度小于取值字符串(Value)的真实长度，则会出现致命错误
	 * /return  bool	:	读到该段为真
	 */
	bool ReadLine( const char *name, char *value, int nSize );
	bool ReadLine( const char *name, int *value );
	bool ReadLine( const char *name, float *value );
	bool ReadLine( const char *name, double *value );
	
	/** 
	 * 读入字符。
	 */
	bool ReadLine( const char *name, char* value );
	
	/** 
	 * 跳到文件头
	 */
	void ReadStart();
	
	/**
	 * 写操作,写入某一段
	 * /param section	:	段名
	 */
	void WriteSection( const char * section );
	
	/**
	 * 写入域名和取值
	 * /param name		:	域名
	 * /param value	:	取值
	 */
	void WriteLine( const char *name, const char *value );
	void WriteLine( const char *name, int value );
	void WriteLine( const char *name, float value );
	void WriteLine( const char *name, double value );
	/** 
	 * 写入空行
	 */
	void WriteLineSeperator();
	
};
////////////////////


////////////////////
#endif	// __EMINIFILE_H__
