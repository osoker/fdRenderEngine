//=======================================================================================================
/**  \file
 *   Filename: packFileOperate.h
 *   Desc:     Osok�����в����ļ��İ�����������.
 *
 *   His:      River created @ 4/27 2003.
 *
 *   "����Բ���ʧ��!!!"
 *  
 */
//=======================================================================================================
# include "../interface/fileOperate.h"
# include "../interface/getfs.h"
# include "direct.h"

#define PACK_NEW            1

//����ʹ����
#define PACK_USE_IN_ENGINE  1

//! �ļ������Ƿ��ٽ�
# define  FILE_CRITICAL   1


# pragma warning( disable : 4786 )  


# include <windows.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <list>
#include <algorithm>
#include <functional>
#include <numeric>

// һЩ��������

//! ���ݿ�Ĵ�С��byte��
#define BLOCK_SIZE  1024

//! �ļ�ͷorβ��־���� (byte)
#define FLAG_SIZE   40

//! �ļ�ͷ��־��,д��ʱռ��FLAG_SIZE��С
#define HEADER_FLAG   "LandLinePack Header"

//! �ļ�β��־��,д��ʱ��СΪʵ�ʴ�����
#define END_FLAG  "LandLinePack End"

//! ��Ϣ�����ļ�ʹ�õı�־ͷ
#define INFOFILE_HEADER_FLAG  "LandLineInfo Header"

//! ����������-ѹ��������ʹ�õĿռ��ʼ��С
#define COMPRESS_BUFFER_SIZE    1024*1024*2   // 1M

//! ����������-ԭʼ�ļ�ʹ�õĿռ��ʼ��С
#define ORIGINAL_BUFFER_SIZE    1024*1024*2   // 1M

//! ������С�ļ�ѹ������Ч����֤��־
#define SERVER_UPDATE_FILE_HEADER_FLAG     0x327CB4C7

//! ���������ļ�ѹ������Ч����֤��־
#define SERVER_UPDATE_BIG_FILE_HEADER_FLAG   0x5B1C3A8E

//! ������ѹ�������ļ�ʱ��ԭ�ļ��������ӵĺ�׺
#define SERVER_COMPRESS_POSTFIX "c_"

//! ������Ƭ�Ĵ�С����,����������ʱ�������ݿ�ʱ�ã����ļ��Ĵ洢��Ӱ��
enum SIZE_TYPE
{
	ST_DOT5K= 512,   // 512 �ֽ�
};

//! ��Ҫ������Ƭ�����ֵ
#define MAX_CLEARUPFRAGMENT 300
/**\brief
*
*  �汾�ŵĶ���
* 
*/
typedef struct
{
	int master;
	int minor;
} VER;

/**\brief
*
*  �����ļ�ͷ�ṹ
*
*/
typedef struct
{
	//! �ļ����ͱ�־
    char  cFileFlag [ FLAG_SIZE ]; 
	//! ���İ汾��
	VER   ver;
	//! llp �� lliƥ������֤��
	DWORD validateCode;
	//! ���ڵ�ÿƬ���ݴ�С����
	SIZE_TYPE SizeType;  
	//! ������β��ַ,����ļ�β��־��
	DWORD  dwEndAddr;    
	
} PackHeader;


/**\brief
*
*  ��Ϣ�����ļ�ͷ�ṹ
*
*/
typedef struct
{
	//! �ļ���־
	char cFileFlag[FLAG_SIZE];
	//! �ļ�����
	DWORD iInfoCount;
	//! ����������
	DWORD iFreeCount;

	//! llp �� lliƥ������֤��
	DWORD validateCode;

} InfoHeader ;




/**\brief
*
*  �ļ���Ϣ�ṹ
*
*/
typedef struct
{
	//! �ļ���������·���������·��
	char m_chName[MAX_PATH]; 
	//! �ļ��ĵ�һ�����ݿ�ĵ�ַ
	DWORD m_dwFirstDataAdd; 
	//! �ļ�ѹ��ǰ��С
	DWORD m_dwOriginalSize;
	//! �ļ�ѹ�����С
	DWORD m_dwCompressSize;
	//! ������Ƭ������,ʣ���С�����ݲ�������
	UINT m_main_count;
	//! ���һƬ�����ݳ���
	UINT m_remain_len;
		
} FileInfo; 

//! ��¼�ļ���С��λ�õĽṹ
typedef struct
{
	DWORD freeSize;
	DWORD freeAddr;

} FreeInfo,  PosInfo;

typedef std::map<std::string,FileInfo> MapInfo;
typedef std::vector<FreeInfo> ListFreeInfo;



//! ��������ʹ�õĸ���ͷ
typedef struct
{
	VER ver;        //! �汾��
	int file_count;  //! ��Ҫ���µ��ļ�����

} UpdateHeader_FromServer;

/**\brief
*
*  �ӷ���������ͻ��˵�ѹ���ļ���ͷ�ṹ,�˽ṹsizeof()֮��С���ļ���
*  д��Ĵ�С��һ�£�Ϊ�˽�Լ�ļ����Ŀհ��ַ���
* 
*/
typedef struct
{
	DWORD dwFlag;               //! ������ѹ�������ļ���Ч����֤��־
	int PackFileNameLen;       	//! �����ļ�������
	char * pPackFileName;  		//! �����ļ���
	DWORD dwOriginalSize;		//! �ļ�ѹ��ǰ��С
	DWORD dwCompressSize;		//! �ļ�ѹ�����С

} FileHeader_FromServer;





class Pack
{
public:
	
	Pack();
	~Pack();

	/**\brief
	*
	*	�½�����д�����ͷ�ṹ��
	*
	*	\param _chpName  �½��������ơ�
	*	\return bool     ����true��ʾ�����ɹ���������false��
	*
	*/
	bool NewPack( char * _chpName );

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
	bool OpenPack( char *_chpName );

	/**\brief
	*
	*	�ر�һ�����ļ���
	*
	*	�رղ��洢 pack file & Info file
	*
	*/
	void ClosePack();

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
	bool AddFile( char * _PackPath, char * _filename );

	/**\brief
	*
	*	����ļ���д��һ��ѹ���������ݡ�
	*
	*	\param  _PackFileName   ����Ƭ�ڰ��ڵ��ļ��������ڵ����·��
	*   \param  _pCompressData  ���ݻ��������洢����ѹ������
	*   \param  _dwCompressSize ��������С
	*
	*	˵��:��������д��ѹ����
	*
	*/
	bool AddFile( char * _PackFileName, VOID * _pCompressData,
				   DWORD _dwCompressSize , DWORD _dwOriginalSize );

	/**\brief
	*
	*	�Ӱ��ļ���ɾ��һ���ļ���
	*
	*	\param  _filename   �����ļ��������ڵ����·��
	*   \return             �ɹ���1,ʧ�ܷ���-1
	*	˵����
	*
	*/
	int DeletePackFile( char * _FileName );

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
	bool AddFolder( char * _PackPath , char * _DiskPath, char * _DiskPathBase );

	/**\brief
	*
	*	�Ӱ��ļ���ɾ���ļ��м��������ݡ�
	*
	*	\param _FolderName  �ļ�Ŀ¼   �磺base\   ��  base  ��ɾ������base�ļ����µ��ļ�
	*
	*	˵����
	*
	*/
	bool DeleteFolder( char * _FolderName );

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
	bool CopyPackFileToDisk( char * _DiskDest, char * _PackSrcFile );

	/**\brief
	*
	*	�Ӱ��ļ��п����ļ��м��������ݵ����̡�
	*
	*	\param _disk_dest   Ŀ���ļ��У�����·����
	*	\param _pack_src���������ļ��У��������·������
	*	\return   bool  ����true��ʾ�򿪳ɹ�������ʧ�ܡ� 
	*
	*	˵�����˺���δ����
	*
	*/
	bool CopyPackFolderToDisk( char * _DiskDest, char * _PackSrc  );

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
	int CopyPackFileToMemory( char * _InPackFileName, LPVOID _pOutBuffer, DWORD _dwBufferSize );	

	/**\brief
	*
	*	�ڰ��в���ָ�����ļ���
	*	\param _FileName             Ҫ���ҵ��ļ������ƣ��ڰ��е����ơ����·��
	*	\return PosInfo              �ļ���λ�ü���С��Ϣ��
	*
	*
	*/
	PosInfo FindFileInPack(char * _FileName );

	/**\brief
	*
	*	����ļ�����Ϣ��
	*	\param _FileName             Ҫ���ҵ��ļ������ƣ��ڰ��е����ơ����·��
	*	\return FileInfo              �ļ�����Ϣ��
	*
	*
	*/
	FileInfo FindFileInfo(char * _filename );

	/**\brief
	*
	*	����ļ��ĸ�����
	*	\return DWORD   �����ļ��ĸ�����
	*
	*/
	DWORD GetFileCount();

	/**\brief
	*
	*	���m_mapInfo,���ڴ���������б����item��
	*
	*/
	MapInfo& GetInfoMap();

	//std::string GetBasePath( std::string _s );
	//std::string GetEndPathName( std::string _s );

	/**\brief
	*
	*	�ļ��ĸ���
	*	\param _newName  �ڰ��ڵ�������
	*   \param _oldName  �ڰ��ڵ�ԭ��������
	*
	*/
	void RenameFile( char * _newName, char * _oldName );

	/**\brief
	*
	*	�ô��̵��ļ��滻����ָ���ļ�
	*	\param _PackFile     �ڰ��ڵ�����
	*   \param _DiskSrcFile  �����ļ�������
	*   \return bool   �ɹ���true,ʧ��false
	*
	*/
	bool ReplacePackFileUseDiskFile( char * _PackFile, char * _DiskSrcFile );
	
	/**\brief
	*
	*	�ð��ڵ�ĳ�ļ��滻����ָ���ļ�
	*	\param _OldPackFile     �ڰ��ڵľ��ļ�����
	*   \param _NewPackFile  ���������ļ�������
	*   \return bool   �ɹ���true,ʧ��false
	*
	*/
	bool ReplacePackfileUsePackFile( char * _OldPackFile, char * _NewPackFile );

	
//@{
	/**\brief
	*
	*	��������ʹ�ô˺������н��������ļ�ѹ��.���ʱ���ļ�����Ϊ .lsc
    *   
	*   \param _DiskFile  ���������ļ�������
	*   \param _fileNameInPack    �ڰ��ڵ��ļ���
	*   \return bool          �ɹ���true,ʧ��false
	*
	*   ˵����ѹ���ļ������ԭ�ļ���ͬһ·���ڣ��ļ����ַ������c_,���۴��ļ�������չ��
	*/
	//bool ServerCompressFile( char * _DiskFile,  char * _FileNameInPack );

	/**\brief
	*
	*	��������ʹ�ô˺����� *.*c_ �ļ���ѹ��ԭ�ļ�����ѹ��ͬĿ¼�£��ļ���Ϊԭ���Ƶ���չ��ȥ�����һ��"c"
	*   \param _c_File  ����    ��������ʹ��ServerCompressFileѹ��������.lsc�ļ�
	*   \return bool             �ɹ���true,ʧ��false
	*
	*/
	//bool ServerUnCompressFile( char * _c_File );
//@}



	inline void Pack::SetPackName(char * _NewName )
	{
		m_strPackName = std::string(_NewName);
	}

	/**\brief
	*
	*	��հ����ļ�����Ƭ�����ٰ��Ĵ�С��ʹ�����ļ���������
	*   �˺�������ʱ������һ����ʱ�ļ������ý����Զ�ɾ��
	*
	*  \return bool ������ɳɹ���true,ʧ�ܷ�false;
	*
	*/
	bool ClearUpFragment();

	/**\brief
	*
	*	дLOG�ļ�����������Ϣ,��С���⣬���Ŀ���Ƭ������Ƭ��������Ϣ��
	*              ������
	*
	* \param   _logfile   ��־�ļ�����·����
	* \return  bool                �����ɹ���true,ʧ�ܷ�false
	*
	*/
	void Dump( char * _logfile );

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
	void ReadBound( LPVOID _pBuffer, DWORD _dwStartAdd, DWORD _dwNum , HANDLE _handle );
	
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
	void WriteBound( LPVOID _pBuffer, DWORD _dwStartAdd, DWORD _dwNum , HANDLE _handle);


	/**\brief
	*
	*	���ã���ȡ���İ汾��
	*   
	*/
	//@{
	inline void SetVer( VER _v ){	m_sPackHeader.ver = _v;	}
	inline VER  GetVer() { return m_sPackHeader.ver; }
	//@}

	/**\brief
	*
	*	��ȡһ���ļ��ڵ��ļ�����
	*   
	*/
	int GetFolderFileCount(char*_folder);

	/**\brief
	*
	*	�ô���Ŀ¼���Ŀ��ȶԣ���ȡ�����Ŀ¼������Ϊ����Ŀ¼
	*
	* \param   _out   ���·������������
	* \param  _big     ��·��
	* \param  _small ������·��
	*
	*/
	int GetDiskFolder( std::string& _out, std::string _big, std::string _small );
	
	/**\brief
	*
	*	���ļ���������Ϊ�ɶ���д
	*
	* \param   _filename   �ļ���
	*
	*/
	static void SetFileAttr_ReadWrite( char * _filename );

	/**\brief
	*
	*	����ļ�װ����̵��������������ʹ��
	*   
	*/
	//@{
	//! ��ǰ��ȡ�ļ�������
	int m_CurrCount;
	//! �ļ�����
	int m_TotalCount;
	//@}

protected:

	/**\brief
	*
	*	��ȡ�������ݵ�β��ַ,β��ַ�����ļ�β��־��
	*   
	*/
	inline DWORD GetDataEndAddr();

	/**\brief
	*
	*	���ð������ݵ�β��ַ,β��ַ�����ļ�β��־��
	*   
	*/
	inline void SetDataEndAddr( DWORD _addr );


	/**\brief
	*
	*	����ָ���Ĵ����ļ��Ƿ���ڣ� �鿴���ļ��Ƿ����ʱʹ��file_exist()
	*
	* \param _FileName  �������ļ���
	* \return bool     ����trueָʾ�ļ����ڣ�����Ϊ������
	*
	*/
	bool	IsFileExist( char * _FileName);
	
	/**\brief
	*
	*	��_full���Ƿ����_pathbase���ڡ�λ��_full���ײ�
	*
	* \param  _full      ĸ·�� 
	* \param  _pathbase  ����ĸ·���Ƚϵ���·��
	* \return bool     ����trueָʾ_pathbase����_full���Ӽ������Ӵ�λ��0������Ϊ��������
	*
	*/	
	bool IsInclude( std::string _full, std::string _pathbase );

	/**\brief
	*
	*	д���ļ���Ϣ�����е���Ϣ������Ϊpack���ֵ�����һ���ַ���Ϊ'i'
	*
	* \return bool     ����trueָʾ�ɹ�������Ϊʧ��
	*
	*/	
	bool WriteFileInfoAndFreeInfoToFile();

	/**\brief
	*
	*	��ճ�Ա�����ļ���� 
	*
	*/
	void ClearUp();

	/**\brief
	*
	*	�ϳ�·��,�����ַ������
	*
	*/
	void MakePath( char * _pOut, char * _p1, char * _p2 );

	
	/**\brief
	*
	*	���һ���ļ�����·������û���򴴽���
	*
	* \param   _fileOrFolderName   �ļ�����·����
	* \return  bool                �����ɹ���true,ʧ�ܷ�false
	*
	*/
	bool IfNoFolderThenCreate( char * _fileOrFolderName );



private:

	//! ���ļ����
	HANDLE m_hFile;
	//! ��ͷ
	PackHeader m_sPackHeader;
	//! ��ǰ�����İ�����
	std::string m_strPackName;

	//! �洢�ļ���Ϣ��map�ṹ
    MapInfo m_mapInfo;

	//! �洢���п���������Ϣ
	ListFreeInfo m_listFreeInfo;
//@{
	//! ʵ�ʵ�ѹ������Ĵ�С
	int m_CompressWorkBufferSize;
	//! ѹ���������ĵ�ָ��
	BYTE * m_pCompressWorkBuffer;

	//! ʵ�ʵ�ԭʼ����Ĵ�С
	int m_OriginalWorkBufferSize;
	//! ԭʼ�Ļ�������ָ��
	BYTE * m_pOriginalWorkBuffer;
//@}
};

// ������Ϣ�ṹ
typedef struct PackInfo
{
	// ����·�������ļ����ļ��б�־���������ְ�������
	char basePath[MAX_PATH];
	// �����ļ�λ��,���·��
	char packFile[MAX_PATH];
	// ��ָ��
	Pack * pPack;

} PackInfo;


// �洢������Ϣ�ṹ�б�����ִ�в��Ҷ�λָ�����ļ����ĸ����ڡ�
typedef std::map<std::string, PackInfo> MapPackInfo; 

extern MapPackInfo*   g_mapPackInfo_Update; //��ʼ������ʱ��
extern MapPackInfo*   g_mapPackInfo;        //����ʱ�á�


//--------------------------------------------------------------------
// ����������������Ľӿں���, �������裺��ʼ���ӿڣ��������ͷŽӿ�
//--------------------------------------------------------------------

/** \brief
 *  ��ʼ�����Ľӿ�,�ڴ˽����ڴ�ķ���
 *  
 *  \param  iniFile  ���������ļ������������ļ���������ͬһ���ļ��¡�
 *                   �����ļ���ʽ˵��: ���ܲ��ҵ�·���״���PackBase��һ�µļ�¼����:���ڴ洢��pic1\..., PackBase����Ϊpic,�������Ҳ�����
 *                       ����[PACKINFO]
 *                          PackBase1  :  ���ı�־����Ҫͬ��ʵ��·���״���ͬ,��ִ���ļ����ĸ����Ĳ��Ҷ���ʱ��ͨ���˱�־��λ�������ơ� (��1��ʼ)
 *                          PackPath1  :  ����·����
 *                           ...
 *                          PackBaseN
 *                          PackPathN
 *                            
 */
extern bool init_packinterface_pack(const char * iniFile);



/** \brief
 *  �ͷŰ��ӿ�ʹ�õ��ڴ���Դ
 *  
 */
extern  void release_packinterface_pack();

/** \brief
 *  �鿴һ���ļ��Ƿ����.
 *  
 *  \param  _filename  ����·��
 */
extern bool file_exist_pack( const char* _filename );

/** 
 *  �õ��ļ����ȵĺ���,������*.osk�ļ��е��ļ�,Ҳ���Ե������ļ�.
 *  
 *  \return  ����������,����-1,�����ȷ,�����ļ�����,��byteΪ��λ.
 *  \param   _filename Ҫ������ļ�������.
 */
extern int get_fileSize_pack( char* _filename );

/** 
 *  ���ļ����뵽һ����������.���ض�����ļ��ĳ��ȡ�
 *
 *  \return ����ɹ�,�����ļ�����,����BUFFER���Ȳ��㷵��-1.�޴��ļ�����-2
 *  \param _filename  �����ļ���
 *  \param _buf �ļ�Ҫ����Ļ�����,�Ѿ������㹻�ڴ�.
 *  \param _bufsize ����ļ����ȴ���_bufsize,����-1.
 */
extern int read_fileToBuf_pack( char* _filename, unsigned char* _buf,int _bufsize );

/** 
 *  д�ļ�����
 *
 *  \return ����ɹ�,�����ļ�����,���򷵻�-1.
 *  \param _filename �ļ���  �磺sound\\mysnd.wav
 *  \param _buf      ���ݻ�����
 *  \param _bufsize  ���ݴ�С(byte)
 */
extern int write_file_pack( char* _filename, BYTE* _buf,int _bufsize );

//------------------------------------------------------------------------------------
/** 
 *  ���°��ļ����÷������˵Ĵ�����ݸ��±��ذ���
 *
 *  \param _c_File �ӷ��������صĸ����ļ�·��
 *  \return int     ����ɹ�,����1,ʧ�ܷ���-1,������Ч�ļ�����-2
 *  
 */
//extern int update_packfile( char * _c_File );


//! ��ô����ļ���С
int  get_diskFileSize( char* _filename );

# if FILE_CRITICAL
// TEST CODE: ���ļ���Ҫʹ�ü����???
extern CRITICAL_SECTION    g_sFileReadCS;
# endif 
