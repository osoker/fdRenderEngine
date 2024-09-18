//========================================================
/*\file
 *\Filename:XlsExportFile.h
 *
 *\Desc:	由xls文件导出的文本文件读写类，只实现了顺序读写
 *
 *\His:		snowwin Create ＠ 2005－10－26
 */
//========================================================
/*------------使用举例--------------------
	XlsExportFile xf;
	if( !xf.open( "ex.txt", 10 ) )
	{
		// 错误处理
	}
	std::vector<std::string> v;
	while( !xf.isEOF() )
	{
		if( !xf.readLine( v ) )continue;
		//...执行到这里v中已经放好了这一行的数据
		//..v中放的就是本行中各个字段的内容
	}
	xf.close();
*///--------------------------------------
#ifndef __SNOWWIN_XLS_EXPORT_FILE_H__
#define __SNOWWIN_XLS_EXPORT_FILE_H__

#include <string>
#include <vector>


/// 由xls文件导出的文本文件读写类
class XlsExportFile
{
public:
	XlsExportFile(void);
	~XlsExportFile(void);

	/// 打开文件，colNum为列的个数，列间由tab分开
	bool open( const char* fname, int colNum, bool write = false );

	/// 关闭文件
	void close( void );

	/// 文件是否结束
	bool isEOF( void );

	/// 读一行,返回本行是不是完整行
	bool readLine( std::vector<std::string>& val );

	/// 写一行
	bool writeLine( std::vector<std::string>& val );

	// 打开文件，colNum为列的个数，列间由tab分开
	bool openXls( const char* fname, int colNum );
	// 关闭文件
	void closeXls( void );
	// 文件是否结束
	bool isXlsEOF( void );
	// 读一行,返回本行是不是完整行
	bool readXlsLine( std::vector<std::string>& val );

protected:
	FILE*	file_;		//!< 文件指针

	BYTE*   m_fileBuffer; 
	BYTE*   m_fileCurPos; 
	int		m_filesize;

	int		colCount_;	//!< 列数
	char	line_[1024];
};

#endif //__SNOWWIN_XLS_EXPORT_FILE_H__
