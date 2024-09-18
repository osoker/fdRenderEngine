////////////////////
////////////////////
//----------------------------------------------------------------------------------------------
/** \file
 *  Filename: IniFile.h
 *  Desc:     �����д��Ini�ļ�����.
 *  His:      
 *
 *  2003.4.30: �������ڴ��Ini�ļ�����,���ڴ�����ini�ļ���Ϣ.
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
 *  ����������������д��Ini�ļ��Ĺ���.
 *  ��Ini�ļ�������һЩ�����ʼ����Ϣ֮��Ĺ���.
 *  
 */
class COMMON_API CIniFile
{
private:
	//! �ļ�ָ��
	FILE *m_fp;
	//! �Ƿ��Ƕ�ģʽ
	bool m_bRead;


	//! ������ڴ��������,�˴����ڴ�����ָ��.
	BYTE* m_pData;
	
	//! �ڴ��ļ��ĳ���.
	int   m_iSize;

	//! �ڴ����ݶ�ȡ��Ҫ��ָ��.�൱���ļ�λ�õ�ָ��.
	int   m_iP;

	//! �ǲ��Ǵ��ڴ��������.
	bool  m_bReadMem;

	
public:
	
	CIniFile();
	~CIniFile();
	
	/**
	 * �ļ�����
	 * ���ļ���
	 * /param filename	:	�ļ���
	 * /return bool	:	�ɹ�Ϊ��
	 */
	bool OpenFileRead( const char *filename );

	/**
	 * ���ڴ�����ini��Ϣ.
	 *
	 */
	void OpenMemRead( BYTE *_dstart,int _isize );

	/** 
	 * ���ļ�д
	 * /param  filename	:	�ļ���
	 * /return bool	:	�ɹ�Ϊ��
	 */
	bool OpenFileWrite( const char *filename );
	/**  �ر��ļ�
	 */
	void CloseFile();
	
	/**
	 *  �õ��ļ���ָ��.
	 */
	FILE *GetFilePointer();
	bool IfRead();
	
	/** 
	 * ������
	 * �ļ�ָ������ĳһ�Σ��ļ�ָ��ӵ�ǰλ�ÿ�ʼ����
	 * /param section	:	����
	 * /return bool :	�ҵ��ö�Ϊ��
	 */
	bool ReadSection(  char *section );
	/**
	 * ����������ȡֵ
	 * /param  name		:	���������Ϊ�գ���ȱʡ����һ����
	 * /param  value	:	ȡֵ�ַ���
	 * /param  nSize	:	ȡֵ�ַ����ĳ��ȣ��������С��ȡֵ�ַ���(Value)����ʵ���ȣ���������������
	 * /return  bool	:	�����ö�Ϊ��
	 */
	bool ReadLine( const char *name, char *value, int nSize );
	bool ReadLine( const char *name, int *value );
	bool ReadLine( const char *name, float *value );
	bool ReadLine( const char *name, double *value );
	
	/** 
	 * �����ַ���
	 */
	bool ReadLine( const char *name, char* value );
	
	/** 
	 * �����ļ�ͷ
	 */
	void ReadStart();
	
	/**
	 * д����,д��ĳһ��
	 * /param section	:	����
	 */
	void WriteSection( const char * section );
	
	/**
	 * д��������ȡֵ
	 * /param name		:	����
	 * /param value	:	ȡֵ
	 */
	void WriteLine( const char *name, const char *value );
	void WriteLine( const char *name, int value );
	void WriteLine( const char *name, float value );
	void WriteLine( const char *name, double value );
	/** 
	 * д�����
	 */
	void WriteLineSeperator();
	
};
////////////////////


////////////////////
#endif	// __EMINIFILE_H__
