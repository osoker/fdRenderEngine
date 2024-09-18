//=======================================================================================================
/**  \file
 *   Filename: fileOperate.h
 *   Desc:     Osok�����в����ļ��İ�����������.
 *
 *   His:      River created @ 4/27 2003.
 *
 *  
 */
//=======================================================================================================
# pragma once

/** 
 *  ��ʹ��common_mfc�汾��common�⣬��memoryManager.h�ĵ�һ�У���
 *   # define TRACKMEM_ALLOCK  0
 *   ���ֵ����Ϊ0, ����������mfc��ʹ�õ�common����Ⱦ�⣬��Ϊ1,��
 *   ȫ�ֵ�����new��delete. 
 */
# include "../../common/com_include.h"
# include "../interface/getfs.h"
 


#ifdef FILEOPERATE_EXPORTS
#define FILEOPERATE_API __declspec(dllexport)
#else
#define FILEOPERATE_API __declspec(dllimport)
#endif


//! ��������ʹ��zlib�Ķ�Ӧ�ļ���
# include "../interface/zconf.h"
# include "../interface/zlib.h"

                                  


//! ��ʼ��ȫ�ֵ�Pack�ļ��������ݡ�
FILEOPERATE_API bool        init_packinterface(const char * iniFile);

//! �ͷ�ȫ�ֵ�pack�ļ�������Դ��
FILEOPERATE_API void        release_packinterface( void );

/** \brief
 *  �鿴һ���ļ��Ƿ����. �����״̬g_bUsePackSystem�й�
 *  
 *  \param bool _forceNPack ǿ�Ʋ�ʹ�ô���ļ�.
 */
FILEOPERATE_API bool        file_exist( const char* _filename,bool _forceNPack = false );


/** 
 *  �õ��ļ����ȵĺ���,������*.osk�ļ��е��ļ�,Ҳ���Ե������ļ�.
 *  
 *  \return  ����������,����-1,�����ȷ,�����ļ�����,��byteΪ��λ.
 *  \param   _filename Ҫ������ļ�������.
 */
FILEOPERATE_API int         get_fileSize( char* _filename );

/** 
 *  ���ļ����뵽һ����������.���ض�����ļ��ĳ��ȡ�
 *
 *  \return ����ɹ�,�����ļ�����,���򷵻�-1.
 *  \param _buf �ļ�Ҫ����Ļ�����,�Ѿ������㹻�ڴ�.
 *  \param _bufsize ����ļ����ȴ���_bufsize,����-1.
 *  \param _forceDisk ǿ�ƶ���Ӳ���ϵ��ļ�
 */
FILEOPERATE_API int         read_fileToBuf( 
	                          char* _filename,BYTE* _buf,int _bufsize,bool _forceDisk = false );


/** 
 *  д�ļ������̣�Ϊֱд�Ͱ�д�Ĺ��ú���
 *
 *  \return ����ɹ�,�����ļ�����,���򷵻�-1.
 *  \param _filename �ļ���  �磺sound\\mysnd.wav
 *  \param _buf      ���ݻ�����
 *  \param _bufsize  ���ݴ�С(byte)
 */
FILEOPERATE_API  int        write_file( char* _filename, BYTE* _buf,int _bufsize );

//�õ����İ汾��
FILEOPERATE_API  bool       get_packVerion( PACKVER& _ver );

//! �Ƿ�ʹ�ô��
extern FILEOPERATE_API    BOOL       g_bUsePackSystem;

//! �Ƿ����ʹ������ļ���
extern FILEOPERATE_API    BOOL       g_bUseDefaultFile;

//! �Ƿ���ʹ��GBufferʱ��������Ϣ��
extern FILEOPERATE_API    BOOL       g_bExportInfoUseGBuf;


/**
 *  ȫ�����ݻ������Ĵ�С.
 */
# define   TMP_BUFSIZE  1024*1024*5

/*
//! ����debug��release�汾�Ĳ�ͬ��ʹ�ò�ͬ�ĺ������ǳ衣
# ifdef _DEBUG  

//! ���ؿ���ʹ�õ�ȫ���ڴ滺����.��СΪTMP_BUFSIZE
extern  FILEOPERATE_API    BYTE*        start_useGBuf( char* _file,int _line );
# define START_USEGBUF               start_useGBuf( __FILE__,__LINE__)

//! ����ʹ��ȫ�ֵ��ڴ滺����.
extern  FILEOPERATE_API    void         end_useGBuf( char* _file,int _line );
# define END_USEGBUF                 end_useGBuf( __FILE__,__LINE__ )             

//! һ����С��ȫ�ֻ�����,ֻ�������̷߳��ʵĻ�����.
extern  FILEOPERATE_API    BYTE*        start_useLittleGBuf( char* _file,int _line );
# define START_USELGBUF                 start_useLittleGBuf( __FILE__,__LINE__ )

extern  FILEOPERATE_API    void         end_useLittleGBuf( char* _file,int _line );
# define END_USELGBUF                 end_useLittleGBuf( __FILE__,__LINE__ )             

# endif 
*/

//# ifdef NDEBUG

//! ���ؿ���ʹ�õ�ȫ���ڴ滺����.��СΪTMP_BUFSIZE
extern  FILEOPERATE_API    BYTE*        start_useGBuf( int& _bufIdx );
# define START_USEGBUF               start_useGBuf

//! ����ʹ��ȫ�ֵ��ڴ滺����.
extern  FILEOPERATE_API    void         end_useGBuf( int& _bufIdx );
# define END_USEGBUF                 end_useGBuf  

//! һ����С��ȫ�ֻ�����,ֻ�������̷߳��ʵĻ�����.
//extern  FILEOPERATE_API    BYTE*        start_useLittleGBuf();
//# define START_USELGBUF              start_useLittleGBuf()

//extern  FILEOPERATE_API    void         end_useLittleGBuf();
//# define END_USELGBUF                end_useLittleGBuf()   

//# endif 



//@{
//!  ������Զ����ڴ�ʱ�ƶ�ָ��λ�õĺ�.Read_memory_andOffsetPointer.
# define   READ_MEM_OFF( dst,src,size )    { {memcpy( dst,src,size ); src += size;} }
# define   READ_MEM( dst,src,size )    { memcpy( dst,src,size );  }

//@}

# define   WRITE_MEM_OFF( dst, src, size ) { memcpy( dst,src,size); dst+=size;  }


//��  ���������ݽӿڣ��ϲ������һ���򵥵����ݴ���
//�� tzz added for finding galaxy game's file
extern FILEOPERATE_API    BOOL       g_bRecordFilename;


//! the class to help read file
class CReadFile
{
	int		m_fileHandle;

	unsigned char* m_fileBuffer;

	int		m_filePos;

	int     t_iGBufIdx;
public:	//constructor and destructor

	//! read from the file(pack)
	CReadFile(const char* _filename):m_fileHandle(-1),
		m_filePos(0),
		m_fileBuffer(NULL)
	{
		t_iGBufIdx = -1;
		OpenFile(_filename);
	}

	//!default function
	CReadFile(void):m_fileHandle(-1),
		m_filePos(0),
		m_fileBuffer(NULL){
		t_iGBufIdx = -1;
		}

		~CReadFile(void){CloseFile();}

public:

	//! is Opening?
	BOOL IsOpen(void){return m_fileHandle != -1;}

	//! open file to read
	BOOL OpenFile(const char* _filename)
	{
		if(m_fileHandle != -1){
			// close first
			//
			CloseFile();
		}

		BOOL t_result = FALSE;

		if(file_exist((char*)_filename) ){
			// this file is exist
			//
			assert(m_fileHandle == -1);

			m_fileBuffer = START_USEGBUF( t_iGBufIdx );
			m_fileHandle = read_fileToBuf( (char*)_filename, m_fileBuffer, TMP_BUFSIZE );

			assert(m_fileHandle > 0 && m_fileBuffer);

			t_result = TRUE;

		}

		return t_result;
	}

	//! get the full buffer
	unsigned char* GetFullBuffer(void)
	{
		return m_fileBuffer;
	}

	//! read byte
	int CReadFile::ReadBuffer(void* _buffer,int _len)
	{

		if(_buffer	
		&& m_fileHandle > 0
		&& m_filePos < m_fileHandle){

			if(m_filePos + _len <= m_fileHandle){
				CopyMemory(_buffer,(void*)(m_fileBuffer + m_filePos ),_len);

				m_filePos += _len;

			}else{
				_len = (m_filePos + _len) - m_fileHandle;
				CopyMemory(_buffer,(void*)(m_fileBuffer + m_filePos ),_len);

				m_filePos = m_fileHandle;
			}
		}else{

			_len = 0;
		}

		return _len;

	}

	//! close file
	void CloseFile(void)
	{

		if(m_fileHandle > 0){

			END_USEGBUF( t_iGBufIdx );
			
			m_fileHandle = -1;
			m_fileBuffer = NULL;
			m_filePos = 0;
		}

	}

	//! get the file size
	int GetFileSize()
	{
		return m_fileHandle;
	}

	//! is opend?
	operator BOOL(void)
	{
		return m_fileHandle > 0;
	}

	//! seek begin
	void SeekBegin(void)
	{
		m_filePos = 0;
	}
	void SeekEnd(void)
	{
		m_filePos = m_fileHandle;
	}
};
