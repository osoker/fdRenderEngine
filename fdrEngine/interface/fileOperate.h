//=======================================================================================================
/**  \file
 *   Filename: fileOperate.h
 *   Desc:     Osok引擎中操作文件的帮助函数和类.
 *
 *   His:      River created @ 4/27 2003.
 *
 *  
 */
//=======================================================================================================
# pragma once

/** 
 *  不使用common_mfc版本的common库，在memoryManager.h的第一行，把
 *   # define TRACKMEM_ALLOCK  0
 *   宏的值定义为0, 则编译可以在mfc下使用的common和渲染库，设为1,则
 *   全局的重载new和delete. 
 */
# include "../../common/com_include.h"
# include "../interface/getfs.h"
 


#ifdef FILEOPERATE_EXPORTS
#define FILEOPERATE_API __declspec(dllexport)
#else
#define FILEOPERATE_API __declspec(dllimport)
#endif


//! 包含我们使用zlib的对应文件。
# include "../interface/zconf.h"
# include "../interface/zlib.h"

                                  


//! 初始化全局的Pack文件索引数据。
FILEOPERATE_API bool        init_packinterface(const char * iniFile);

//! 释放全局的pack文件索引资源。
FILEOPERATE_API void        release_packinterface( void );

/** \brief
 *  查看一个文件是否存在. 与包的状态g_bUsePackSystem有关
 *  
 *  \param bool _forceNPack 强制不使用打包文件.
 */
FILEOPERATE_API bool        file_exist( const char* _filename,bool _forceNPack = false );


/** 
 *  得到文件长度的函数,可以是*.osk文件中的文件,也可以单独的文件.
 *  
 *  \return  如果处理错误,返回-1,如果正确,返回文件长度,以byte为单位.
 *  \param   _filename 要处理的文件的名字.
 */
FILEOPERATE_API int         get_fileSize( char* _filename );

/** 
 *  把文件读入到一个缓冲区中.返回读入的文件的长度。
 *
 *  \return 如果成功,返回文件长度,否则返回-1.
 *  \param _buf 文件要读入的缓冲区,已经分配足够内存.
 *  \param _bufsize 如果文件长度大于_bufsize,返回-1.
 *  \param _forceDisk 强制读入硬盘上的文件
 */
FILEOPERATE_API int         read_fileToBuf( 
	                          char* _filename,BYTE* _buf,int _bufsize,bool _forceDisk = false );


/** 
 *  写文件到磁盘，为直写和包写的共用函数
 *
 *  \return 如果成功,返回文件长度,否则返回-1.
 *  \param _filename 文件名  如：sound\\mysnd.wav
 *  \param _buf      数据缓冲区
 *  \param _bufsize  数据大小(byte)
 */
FILEOPERATE_API  int        write_file( char* _filename, BYTE* _buf,int _bufsize );

//得到包的版本号
FILEOPERATE_API  bool       get_packVerion( PACKVER& _ver );

//! 是否使用打包
extern FILEOPERATE_API    BOOL       g_bUsePackSystem;

//! 是否可以使用替代文件。
extern FILEOPERATE_API    BOOL       g_bUseDefaultFile;

//! 是否在使用GBuffer时，导出信息。
extern FILEOPERATE_API    BOOL       g_bExportInfoUseGBuf;


/**
 *  全局数据缓冲区的大小.
 */
# define   TMP_BUFSIZE  1024*1024*5

/*
//! 根据debug和release版本的不同，使用不同的函数或是宠。
# ifdef _DEBUG  

//! 返回可以使用的全局内存缓冲区.大小为TMP_BUFSIZE
extern  FILEOPERATE_API    BYTE*        start_useGBuf( char* _file,int _line );
# define START_USEGBUF               start_useGBuf( __FILE__,__LINE__)

//! 结束使用全局的内存缓冲区.
extern  FILEOPERATE_API    void         end_useGBuf( char* _file,int _line );
# define END_USEGBUF                 end_useGBuf( __FILE__,__LINE__ )             

//! 一个更小的全局缓冲区,只能是主线程访问的缓冲眍.
extern  FILEOPERATE_API    BYTE*        start_useLittleGBuf( char* _file,int _line );
# define START_USELGBUF                 start_useLittleGBuf( __FILE__,__LINE__ )

extern  FILEOPERATE_API    void         end_useLittleGBuf( char* _file,int _line );
# define END_USELGBUF                 end_useLittleGBuf( __FILE__,__LINE__ )             

# endif 
*/

//# ifdef NDEBUG

//! 返回可以使用的全局内存缓冲区.大小为TMP_BUFSIZE
extern  FILEOPERATE_API    BYTE*        start_useGBuf( int& _bufIdx );
# define START_USEGBUF               start_useGBuf

//! 结束使用全局的内存缓冲区.
extern  FILEOPERATE_API    void         end_useGBuf( int& _bufIdx );
# define END_USEGBUF                 end_useGBuf  

//! 一个更小的全局缓冲区,只能是主线程访问的缓冲眍.
//extern  FILEOPERATE_API    BYTE*        start_useLittleGBuf();
//# define START_USELGBUF              start_useLittleGBuf()

//extern  FILEOPERATE_API    void         end_useLittleGBuf();
//# define END_USELGBUF                end_useLittleGBuf()   

//# endif 



//@{
//!  定义可以读入内存时移动指针位置的宏.Read_memory_andOffsetPointer.
# define   READ_MEM_OFF( dst,src,size )    { {memcpy( dst,src,size ); src += size;} }
# define   READ_MEM( dst,src,size )    { memcpy( dst,src,size );  }

//@}

# define   WRITE_MEM_OFF( dst, src, size ) { memcpy( dst,src,size); dst+=size;  }


//！  公开此数据接口，上层可以做一个简单的数据处理
//！ tzz added for finding galaxy game's file
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
