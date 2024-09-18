//=======================================================================================================
/**  \file
 *   Filename: packFileOperate.h
 *   Desc:     Osok引擎中操作文件的帮助函数和类.
 *
 *   His:      River created @ 4/27 2003.
 *
 *   "你绝对不能失败!!!"
 *  
 */
//=======================================================================================================
# include "../interface/fileOperate.h"
# include "../interface/getfs.h"
# include "direct.h"

#define PACK_NEW            1

//定义使用者
#define PACK_USE_IN_ENGINE  1

//! 文件读入是否临界
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

// 一些常量定义

//! 数据块的大小（byte）
#define BLOCK_SIZE  1024

//! 文件头or尾标志长度 (byte)
#define FLAG_SIZE   40

//! 文件头标志串,写入时占用FLAG_SIZE大小
#define HEADER_FLAG   "LandLinePack Header"

//! 文件尾标志串,写入时大小为实际串长度
#define END_FLAG  "LandLinePack End"

//! 信息索引文件使用的标志头
#define INFOFILE_HEADER_FLAG  "LandLineInfo Header"

//! 工作缓冲区-压缩缓冲区使用的空间初始大小
#define COMPRESS_BUFFER_SIZE    1024*1024*2   // 1M

//! 工作缓冲区-原始文件使用的空间初始大小
#define ORIGINAL_BUFFER_SIZE    1024*1024*2   // 1M

//! 服务器小文件压缩包有效性验证标志
#define SERVER_UPDATE_FILE_HEADER_FLAG     0x327CB4C7

//! 服务器大文件压缩包有效性验证标志
#define SERVER_UPDATE_BIG_FILE_HEADER_FLAG   0x5B1C3A8E

//! 服务器压缩单个文件时在原文件名后增加的后缀
#define SERVER_COMPRESS_POSTFIX "c_"

//! 数据切片的大小类型,对增量升级时索引数据块时用，对文件的存储无影响
enum SIZE_TYPE
{
	ST_DOT5K= 512,   // 512 字节
};

//! 需要清理碎片的最大值
#define MAX_CLEARUPFRAGMENT 300
/**\brief
*
*  版本号的定义
* 
*/
typedef struct
{
	int master;
	int minor;
} VER;

/**\brief
*
*  包内文件头结构
*
*/
typedef struct
{
	//! 文件类型标志
    char  cFileFlag [ FLAG_SIZE ]; 
	//! 包的版本号
	VER   ver;
	//! llp 与 lli匹配性验证码
	DWORD validateCode;
	//! 包内的每片数据大小类型
	SIZE_TYPE SizeType;  
	//! 数据区尾地址,后接文件尾标志串
	DWORD  dwEndAddr;    
	
} PackHeader;


/**\brief
*
*  信息索引文件头结构
*
*/
typedef struct
{
	//! 文件标志
	char cFileFlag[FLAG_SIZE];
	//! 文件数量
	DWORD iInfoCount;
	//! 空闲区数量
	DWORD iFreeCount;

	//! llp 与 lli匹配性验证码
	DWORD validateCode;

} InfoHeader ;




/**\brief
*
*  文件信息结构
*
*/
typedef struct
{
	//! 文件名，包内路径，是相对路径
	char m_chName[MAX_PATH]; 
	//! 文件的第一个数据块的地址
	DWORD m_dwFirstDataAdd; 
	//! 文件压缩前大小
	DWORD m_dwOriginalSize;
	//! 文件压缩后大小
	DWORD m_dwCompressSize;
	//! 主数据片的数量,剩余的小块数据不算在内
	UINT m_main_count;
	//! 最后一片的数据长度
	UINT m_remain_len;
		
} FileInfo; 

//! 记录文件大小，位置的结构
typedef struct
{
	DWORD freeSize;
	DWORD freeAddr;

} FreeInfo,  PosInfo;

typedef std::map<std::string,FileInfo> MapInfo;
typedef std::vector<FreeInfo> ListFreeInfo;



//! 服务器端使用的更新头
typedef struct
{
	VER ver;        //! 版本号
	int file_count;  //! 需要更新的文件个数

} UpdateHeader_FromServer;

/**\brief
*
*  从服务器传向客户端的压缩文件－头结构,此结构sizeof()之大小与文件中
*  写入的大小不一致，为了节约文件名的空白字符。
* 
*/
typedef struct
{
	DWORD dwFlag;               //! 服务器压缩包的文件有效性验证标志
	int PackFileNameLen;       	//! 包内文件名长度
	char * pPackFileName;  		//! 包内文件名
	DWORD dwOriginalSize;		//! 文件压缩前大小
	DWORD dwCompressSize;		//! 文件压缩后大小

} FileHeader_FromServer;





class Pack
{
public:
	
	Pack();
	~Pack();

	/**\brief
	*
	*	新建包，写入包的头结构。
	*
	*	\param _chpName  新建包的名称。
	*	\return bool     返回true表示创建成功，否则是false。
	*
	*/
	bool NewPack( char * _chpName );

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
	bool OpenPack( char *_chpName );

	/**\brief
	*
	*	关闭一个包文件。
	*
	*	关闭并存储 pack file & Info file
	*
	*/
	void ClosePack();

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
	bool AddFile( char * _PackPath, char * _filename );

	/**\brief
	*
	*	向包文件中写入一块压缩过的数据。
	*
	*	\param  _PackFileName   数据片在包内的文件名，包内的相对路径
	*   \param  _pCompressData  数据缓冲区，存储的是压缩数据
	*   \param  _dwCompressSize 缓冲区大小
	*
	*	说明:将数据区写入压缩包
	*
	*/
	bool AddFile( char * _PackFileName, VOID * _pCompressData,
				   DWORD _dwCompressSize , DWORD _dwOriginalSize );

	/**\brief
	*
	*	从包文件中删除一个文件。
	*
	*	\param  _filename   包内文件名，包内的相对路径
	*   \return             成功返1,失败返回-1
	*	说明：
	*
	*/
	int DeletePackFile( char * _FileName );

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
	bool AddFolder( char * _PackPath , char * _DiskPath, char * _DiskPathBase );

	/**\brief
	*
	*	从包文件中删除文件夹及其子内容。
	*
	*	\param _FolderName  文件目录   如：base\   或  base  将删除所有base文件夹下的文件
	*
	*	说明：
	*
	*/
	bool DeleteFolder( char * _FolderName );

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
	bool CopyPackFileToDisk( char * _DiskDest, char * _PackSrcFile );

	/**\brief
	*
	*	从包文件中拷贝文件夹及其子内容到磁盘。
	*
	*	\param _disk_dest   目的文件夹（绝对路径）
	*	\param _pack_src　　包内文件夹（包内相对路径）　
	*	\return   bool  返回true表示打开成功，否则失败。 
	*
	*	说明：此函数未测试
	*
	*/
	bool CopyPackFolderToDisk( char * _DiskDest, char * _PackSrc  );

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
	int CopyPackFileToMemory( char * _InPackFileName, LPVOID _pOutBuffer, DWORD _dwBufferSize );	

	/**\brief
	*
	*	在包中查找指定的文件。
	*	\param _FileName             要查找的文件的名称，在包中的名称。相对路径
	*	\return PosInfo              文件的位置及大小信息。
	*
	*
	*/
	PosInfo FindFileInPack(char * _FileName );

	/**\brief
	*
	*	获得文件的信息。
	*	\param _FileName             要查找的文件的名称，在包中的名称。相对路径
	*	\return FileInfo              文件的信息。
	*
	*
	*/
	FileInfo FindFileInfo(char * _filename );

	/**\brief
	*
	*	获得文件的个数。
	*	\return DWORD   包内文件的个数。
	*
	*/
	DWORD GetFileCount();

	/**\brief
	*
	*	获得m_mapInfo,用于打包工具向列表添加item用
	*
	*/
	MapInfo& GetInfoMap();

	//std::string GetBasePath( std::string _s );
	//std::string GetEndPathName( std::string _s );

	/**\brief
	*
	*	文件的改名
	*	\param _newName  在包内的新名字
	*   \param _oldName  在包内的原来的名字
	*
	*/
	void RenameFile( char * _newName, char * _oldName );

	/**\brief
	*
	*	用磁盘的文件替换包内指定文件
	*	\param _PackFile     在包内的名字
	*   \param _DiskSrcFile  磁盘文件的名字
	*   \return bool   成功返true,失败false
	*
	*/
	bool ReplacePackFileUseDiskFile( char * _PackFile, char * _DiskSrcFile );
	
	/**\brief
	*
	*	用包内的某文件替换包内指定文件
	*	\param _OldPackFile     在包内的旧文件名字
	*   \param _NewPackFile  　　磁盘文件的名字
	*   \return bool   成功返true,失败false
	*
	*/
	bool ReplacePackfileUsePackFile( char * _OldPackFile, char * _NewPackFile );

	
//@{
	/**\brief
	*
	*	服务器端使用此函数进行将单个的文件压缩.输出时将文件名变为 .lsc
    *   
	*   \param _DiskFile  　　磁盘文件的名字
	*   \param _fileNameInPack    在包内的文件名
	*   \return bool          成功返true,失败false
	*
	*   说明：压缩文件存放在原文件的同一路径在，文件名字符串后加c_,无论此文件有无扩展名
	*/
	//bool ServerCompressFile( char * _DiskFile,  char * _FileNameInPack );

	/**\brief
	*
	*	服务器端使用此函数将 *.*c_ 文件解压回原文件，解压到同目录下，文件名为原名称的扩展名去掉最后一个"c"
	*   \param _c_File  　　    服务器端使用ServerCompressFile压缩产生的.lsc文件
	*   \return bool             成功返true,失败false
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
	*	清空包内文件的碎片，减少包的大小。使包内文件紧密相连
	*   此函数调用时创建了一个临时文件，调用结束自动删除
	*
	*  \return bool 整理完成成功返true,失败返false;
	*
	*/
	bool ClearUpFragment();

	/**\brief
	*
	*	写LOG文件，空闲区信息,最小的这，最大的空闲片，空闲片的数量信息，
	*              调试用
	*
	* \param   _logfile   日志文件名或路径名
	* \return  bool                操作成功返true,失败返false
	*
	*/
	void Dump( char * _logfile );

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
	void ReadBound( LPVOID _pBuffer, DWORD _dwStartAdd, DWORD _dwNum , HANDLE _handle );
	
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
	void WriteBound( LPVOID _pBuffer, DWORD _dwStartAdd, DWORD _dwNum , HANDLE _handle);


	/**\brief
	*
	*	设置，获取包的版本号
	*   
	*/
	//@{
	inline void SetVer( VER _v ){	m_sPackHeader.ver = _v;	}
	inline VER  GetVer() { return m_sPackHeader.ver; }
	//@}

	/**\brief
	*
	*	获取一个文件内的文件数量
	*   
	*/
	int GetFolderFileCount(char*_folder);

	/**\brief
	*
	*	用磁盘目录与基目标比对，获取多余的目录串，作为包内目录
	*
	* \param   _out   输出路径串，长－短
	* \param  _big     长路径
	* \param  _small 　　短路径
	*
	*/
	int GetDiskFolder( std::string& _out, std::string _big, std::string _small );
	
	/**\brief
	*
	*	将文件的属性设为可读可写
	*
	* \param   _filename   文件名
	*
	*/
	static void SetFileAttr_ReadWrite( char * _filename );

	/**\brief
	*
	*	标记文件装入进程的数量，打包工具使用
	*   
	*/
	//@{
	//! 当前读取文件的数量
	int m_CurrCount;
	//! 文件总数
	int m_TotalCount;
	//@}

protected:

	/**\brief
	*
	*	获取包的数据的尾地址,尾地址后存放文件尾标志串
	*   
	*/
	inline DWORD GetDataEndAddr();

	/**\brief
	*
	*	设置包的数据的尾地址,尾地址后存放文件尾标志串
	*   
	*/
	inline void SetDataEndAddr( DWORD _addr );


	/**\brief
	*
	*	检杳指定的磁盘文件是否存在？ 查看包文件是否存在时使用file_exist()
	*
	* \param _FileName  待检查的文件名
	* \return bool     返回true指示文件存在，否则为不存在
	*
	*/
	bool	IsFileExist( char * _FileName);
	
	/**\brief
	*
	*	在_full内是否包含_pathbase且在　位于_full的首部
	*
	* \param  _full      母路径 
	* \param  _pathbase  将与母路径比较的子路径
	* \return bool     返回true指示_pathbase属于_full的子集，且子串位于0，否则为不在其内
	*
	*/	
	bool IsInclude( std::string _full, std::string _pathbase );

	/**\brief
	*
	*	写入文件信息及空闲的信息，名字为pack名字的最后的一个字符改为'i'
	*
	* \return bool     返回true指示成功，否则为失败
	*
	*/	
	bool WriteFileInfoAndFreeInfoToFile();

	/**\brief
	*
	*	清空成员变量文件句柄 
	*
	*/
	void ClearUp();

	/**\brief
	*
	*	合成路径,两个字符串相加
	*
	*/
	void MakePath( char * _pOut, char * _p1, char * _p2 );

	
	/**\brief
	*
	*	检查一个文件名或路径名，没有则创建它
	*
	* \param   _fileOrFolderName   文件名或路径名
	* \return  bool                操作成功返true,失败返false
	*
	*/
	bool IfNoFolderThenCreate( char * _fileOrFolderName );



private:

	//! 包文件句柄
	HANDLE m_hFile;
	//! 包头
	PackHeader m_sPackHeader;
	//! 当前操作的包名字
	std::string m_strPackName;

	//! 存储文件信息的map结构
    MapInfo m_mapInfo;

	//! 存储空闲可用区的信息
	ListFreeInfo m_listFreeInfo;
//@{
	//! 实际的压缩缓冲的大小
	int m_CompressWorkBufferSize;
	//! 压缩缓冲区的的指针
	BYTE * m_pCompressWorkBuffer;

	//! 实际的原始缓冲的大小
	int m_OriginalWorkBufferSize;
	//! 原始的缓冲区的指针
	BYTE * m_pOriginalWorkBuffer;
//@}
};

// 包的信息结构
typedef struct PackInfo
{
	// 基本路径，包文件的文件夹标志，用以区分包的类型
	char basePath[MAX_PATH];
	// 包的文件位置,相对路径
	char packFile[MAX_PATH];
	// 包指针
	Pack * pPack;

} PackInfo;


// 存储包的信息结构列表，用于执行查找定位指定的文件在哪个包内。
typedef std::map<std::string, PackInfo> MapPackInfo; 

extern MapPackInfo*   g_mapPackInfo_Update; //初始化更新时用
extern MapPackInfo*   g_mapPackInfo;        //运行时用　


//--------------------------------------------------------------------
// 以下是引擎操作包的接口函数, 操作步骤：初始化接口，操作，释放接口
//--------------------------------------------------------------------

/** \brief
 *  初始化包的接口,在此进行内存的分配
 *  
 *  \param  iniFile  包的配置文件，包的所有文件都必须在同一个文件下。
 *                   配置文件格式说明: 不能查找到路径首串与PackBase不一致的记录，如:包内存储的pic1\..., PackBase配置为pic,这样查找不到，
 *                       区段[PACKINFO]
 *                          PackBase1  :  包的标志，需要同真实的路径首串相同,在执行文件在哪个包的查找动作时，通过此标志定位包的名称。 (由1开始)
 *                          PackPath1  :  包的路径。
 *                           ...
 *                          PackBaseN
 *                          PackPathN
 *                            
 */
extern bool init_packinterface_pack(const char * iniFile);



/** \brief
 *  释放包接口使用的内存资源
 *  
 */
extern  void release_packinterface_pack();

/** \brief
 *  查看一个文件是否存在.
 *  
 *  \param  _filename  包内路径
 */
extern bool file_exist_pack( const char* _filename );

/** 
 *  得到文件长度的函数,可以是*.osk文件中的文件,也可以单独的文件.
 *  
 *  \return  如果处理错误,返回-1,如果正确,返回文件长度,以byte为单位.
 *  \param   _filename 要处理的文件的名字.
 */
extern int get_fileSize_pack( char* _filename );

/** 
 *  把文件读入到一个缓冲区中.返回读入的文件的长度。
 *
 *  \return 如果成功,返回文件长度,输入BUFFER长度不足返回-1.无此文件返回-2
 *  \param _filename  包内文件名
 *  \param _buf 文件要读入的缓冲区,已经分配足够内存.
 *  \param _bufsize 如果文件长度大于_bufsize,返回-1.
 */
extern int read_fileToBuf_pack( char* _filename, unsigned char* _buf,int _bufsize );

/** 
 *  写文件到包
 *
 *  \return 如果成功,返回文件长度,否则返回-1.
 *  \param _filename 文件名  如：sound\\mysnd.wav
 *  \param _buf      数据缓冲区
 *  \param _bufsize  数据大小(byte)
 */
extern int write_file_pack( char* _filename, BYTE* _buf,int _bufsize );

//------------------------------------------------------------------------------------
/** 
 *  更新包文件，用服务器端的打包数据更新本地包，
 *
 *  \param _c_File 从服务器下载的更新文件路径
 *  \return int     如果成功,返回1,失败返回-1,不是有效文件返回-2
 *  
 */
//extern int update_packfile( char * _c_File );


//! 获得磁盘文件大小
int  get_diskFileSize( char* _filename );

# if FILE_CRITICAL
// TEST CODE: 读文件需要使用监界区???
extern CRITICAL_SECTION    g_sFileReadCS;
# endif 
