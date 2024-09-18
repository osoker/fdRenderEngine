//========================================================
/*\file
 *\Filename:XlsExportFile.h
 *
 *\Desc:	��xls�ļ��������ı��ļ���д�ֻ࣬ʵ����˳���д
 *
 *\His:		snowwin Create �� 2005��10��26
 */
//========================================================
/*------------ʹ�þ���--------------------
	XlsExportFile xf;
	if( !xf.open( "ex.txt", 10 ) )
	{
		// ������
	}
	std::vector<std::string> v;
	while( !xf.isEOF() )
	{
		if( !xf.readLine( v ) )continue;
		//...ִ�е�����v���Ѿ��ź�����һ�е�����
		//..v�зŵľ��Ǳ����и����ֶε�����
	}
	xf.close();
*///--------------------------------------
#ifndef __SNOWWIN_XLS_EXPORT_FILE_H__
#define __SNOWWIN_XLS_EXPORT_FILE_H__

#include <string>
#include <vector>


/// ��xls�ļ��������ı��ļ���д��
class XlsExportFile
{
public:
	XlsExportFile(void);
	~XlsExportFile(void);

	/// ���ļ���colNumΪ�еĸ������м���tab�ֿ�
	bool open( const char* fname, int colNum, bool write = false );

	/// �ر��ļ�
	void close( void );

	/// �ļ��Ƿ����
	bool isEOF( void );

	/// ��һ��,���ر����ǲ���������
	bool readLine( std::vector<std::string>& val );

	/// дһ��
	bool writeLine( std::vector<std::string>& val );

	// ���ļ���colNumΪ�еĸ������м���tab�ֿ�
	bool openXls( const char* fname, int colNum );
	// �ر��ļ�
	void closeXls( void );
	// �ļ��Ƿ����
	bool isXlsEOF( void );
	// ��һ��,���ر����ǲ���������
	bool readXlsLine( std::vector<std::string>& val );

protected:
	FILE*	file_;		//!< �ļ�ָ��

	BYTE*   m_fileBuffer; 
	BYTE*   m_fileCurPos; 
	int		m_filesize;

	int		colCount_;	//!< ����
	char	line_[1024];
};

#endif //__SNOWWIN_XLS_EXPORT_FILE_H__
