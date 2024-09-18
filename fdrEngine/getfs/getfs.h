//---------------------------------------------------------------------------------------
/**  \file
*   Filename: getfs.h
*   Desc:     
*   His:      yq_sun created @ 2005-05-16
*/
//---------------------------------------------------------------------------------------

#ifndef _GETFS_H_
#define _GETFS_H_

#include <fcntl.h>
#include "getfs_iio.h" 
#include "getfs_data.h"
#include "getfs_dirTable.h"
#include "getfs_nodeTable.h"
#include "getfs_fat.h"

#if _MSC_VER > 1310 // VC8
	#define chsize _chsize
	#define fileno _fileno
#endif //_MSC_VER > 1310


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GETFS_USELOCK  0
#define MAX_TEMPBUFFER_SIZE (1024*1024*6)

//使用checksum
#define USE_CHECKSUM 1

// Definitions
#define GETFS_NULL               NULL
#define GETFS_NAME_LEN            256
#define GETFS_IIO_SUFFIX         ".iio"
#define GETFS_DAT_SUFFIX         ".dat"
#define GETFS_LOCK_SUFFIX        ".lock"
#define GETFS_CHANNEL_DT1        0      /* channel for Directory Table (Trie Nodes) */
#define GETFS_CHANNEL_DT2        1      /* channel for Directory Table (Key Nodes) */
#define GETFS_CHANNEL_NT         2      /* channel for Node Table */
#define GETFS_CHANNEL_FAT        3      /* channel for File Allocation Table */

#define GETFS_CHANNEL_DT1_SIZE   4      /* 8k blocks/dt_channel(trie) */
#define GETFS_CHANNEL_DT2_SIZE   8      /* 8k blocks/dt_channel(keys) */
#define GETFS_CHANNEL_NT_SIZE    1      /* 8k blocks/nt_channel */
#define GETFS_CHANNEL_FAT_SIZE   4      /* 8k blocks/fat_channel */

#define GETFS_INITIAL_HT_SIZE    32     /* 32 slots available by default, will auto-increase as needed */

#define FS_READ                1
#define FS_WRITE               2
#define FS_RW                  (FS_READ | FS_WRITE)

#ifndef GLOB_ERR

#define	GLOB_ERR	       (1 << 0)  /* Return on read errors.  */
#define	GLOB_MARK	       (1 << 1)  /* Append a slash to each name.  */
#define	GLOB_NOSORT	       (1 << 2)  /* Don't sort the names.  */
#define	GLOB_DOOFFS	       (1 << 3)  /* Insert PGLOB->gl_offs NULLs.  */
#define	GLOB_DOOFS	       GLOB_DOOFFS
#define	GLOB_NOCHECK       (1 << 4)  /* If nothing matches, return the pattern.  */
#define	GLOB_APPEND	       (1 << 5)  /* Append to results of a previous call.  */
#define	GLOB_NOESCAPE      (1 << 6)  /* Backslashes don't quote metacharacters.  */
#define	GLOB_PERIOD	       (1 << 7)  /* Leading `.' can be matched by metachars.  */
#define GLOB_MAGCHAR       (1 << 8)  /* Set in gl_flags if any metachars seen.  */
#define GLOB_ALTDIRFUNC    (1 << 9)  /* Use gl_opendir et al functions.  */
#define GLOB_BRACE         (1 << 10) /* Expand "{a,b}" to "a" "b".  */
#define GLOB_NOMAGIC       (1 << 11) /* If no magic chars, return the pattern.  */
#define GLOB_TILDE	       (1 << 12) /* Expand ~user and ~ to home directories. */
#define GLOB_ONLYDIR       (1 << 13) /* Match only directories.  */
#define GLOB_TILDE_CHECK   (1 << 14) /* Like GLOB_TILDE but return an error
if the user name is not available.  */
#define GLOB_FIRSTONLY     (1 << 15) /* Stop after the first match. */

#define GLOB_ONLYCURFOLDERFILE (1 << 16) //仅目录下文件

#define __GLOB_FLAGS       (GLOB_ERR|GLOB_MARK|GLOB_NOSORT|GLOB_DOOFFS| \
    GLOB_NOESCAPE|GLOB_NOCHECK|GLOB_APPEND|     \
    GLOB_PERIOD|GLOB_ALTDIRFUNC|GLOB_BRACE|     \
GLOB_NOMAGIC|GLOB_TILDE|GLOB_ONLYDIR|GLOB_TILDE_CHECK)

/* Error returns from `glob'.  */
#define	GLOB_NOSPACE       1	 /* Ran out of memory.  */
#define	GLOB_ABORTED       2	 /* Read error.  */
#define	GLOB_NOMATCH       3	 /* No matches found.  */
#define GLOB_NOSYS         4	 /* Not implemented.  */

/* GNU compatibility */ 
#define GLOB_ABEND         GLOB_ABORTED

#endif


// Errors 
#define GETFS_ERR_IIO_INIT      1      /* Could not open/create the IIO file  */
#define GETFS_ERR_NO_DATA       2      /* Could not open/create the data file */
#define GETFS_ERR_NO_FAT        3      /* Could not initialize the FAT channel */
#define GETFS_ERR_NO_NT         4      /* Could not initialize the NT channel */
#define GETFS_ERR_NO_DT         5      /* Could not initialize the DT channel */
#define GETFS_ERR_A_FAT         6      /* Could not allocate the FAT channel */
#define GETFS_ERR_A_NT          7      /* Could not allocate the NT channel */
#define GETFS_ERR_A_DT          8      /* Could not allocate the DT channel */
#define GETFS_ERR_INVALID_FS    9      /* Invalid file system handle */
#define GETFS_ERR_ALREADY_CLSD  10     /* File already closed */
#define GETFS_ERR_NOT_FOUND     11     /* File not found */
#define GETFS_ERR_NO_OPEN       12     /* No files open */
#define GETFS_ERR_INVALID_FH    13     /* Invalid file descriptor */
#define GETFS_ERR_INVALID_NT    14     /* Invalid node in NT */
#define GETFS_ERR_NOMEM_OPEN    15     /* No memory available for opening files */
#define GETFS_ERR_LOCKED        16     /* File system is locked by another process */
#define GETFS_ERR_DUPLICATEF    17     /* File already exists */
#define GETFS_ERR_DT_INTERNAL   18     /* Internal error in DT */
#define GETFS_ERR_INVALID_PARMS 19     /* Invalid parameters */

extern CRITICAL_SECTION  g_sReadFile;
extern CRITICAL_SECTION  g_sFileExist;
extern CRITICAL_SECTION  g_sTrienode_get;
extern CRITICAL_SECTION  g_sWriteFile;
extern CRITICAL_SECTION  g_sOpenFile;
extern CRITICAL_SECTION  g_sCloseFile;
extern CRITICAL_SECTION  g_sSetSize;
extern CRITICAL_SECTION  g_sIIORead;

//版本号
struct PACKVER 
{
	int main;
	int minor;
	int patch;

	bool operator==( const PACKVER& _ver )
	{
		return ( main==_ver.main && minor==_ver.minor && patch==_ver.patch );
	}
	bool operator != ( const PACKVER& _ver )
	{
		return ( main!=_ver.main || minor!=_ver.minor || patch!=_ver.patch );
	}
	bool operator<( const PACKVER& _ver )
	{
		if( main < _ver.main )
			return true;
		else if( main > _ver.main )
			return false;
		else
		{
			if( minor < _ver.minor )
				return true;
			else if( minor > _ver.minor )
				return false;
			else
			{
				if( patch < _ver.patch )
					return true;
				else
					return false;
			}
		}
		//assert( false );
		return true; //nerver can run here!
	}
	bool operator>( const PACKVER& _ver )
	{
		if( main > _ver.main )
			return true;
		else if( main < _ver.main )
			return false;
		else
		{
			if( minor > _ver.minor )
				return true;
			else if( minor < _ver.minor )
				return false;
			else
			{
				if( patch > _ver.patch )
					return true;
				else
					return false;
			}
		}
//		assert( false );
		return true; //nerver can run here!
	}
};

typedef struct getfs_FH {
    int dt_index;                /* index in Directory Table */
    int nt_index;                /* index in Node Table */
    int chain;                   /* index in FAT (chain start) */
    int fp;                      /* current file pointer */
    int oflags;                  /* current oflags (specified when the file is open) */
} getfs_FH;

// File system handle
typedef struct getfs_Handle {
	getfs_iio_File*      file;     /* interlaced i/o file */
	getfs_dt_DT*         dt;       /* directory table */
	getfs_nt_NT*         nt;       /* node table */
	getfs_fat_FAT*       fat;      /* file allocation table */
	getfs_data_Data*     data;     /* virtual disk */
	getfs_FH**           handles;  /* handles for open files */
	int                ht_size;  /* size of handle table (in entries) */
	int                fls_open; /* nr of open files */
	int                mode;     /* mode - FS_READ, FS_WRITE, or both */
	char*              filename;
	unsigned char*     getfs_buffer; // file system buffer

} getfs_Handle;

typedef struct getfs_glob_t {
    int    gl_pathc;    /* Count of paths matched so far  */
    char** gl_pathv;    /* List of matched pathnames.  */
    int    gl_offs;     /* Slots to reserve in `gl_pathv'.  */
} getfs_glob_t;

// Returns nonzero (true) if the given file system is available, false (0) if not
int           getfs_exists( const char* _filename );

	// Open if possible (otherwise create) the given file system, return handle
	getfs_Handle* getfs_start( const char* _filename, int _flags );
	// Close the given file system, remove (physically) if 'remove_fs' is true (1)
	void          getfs_end( getfs_Handle* _fs, int _remove_fs );
	// Returns nonzero if the given file exists. Faster than opening the file for reading.
	int           getfs_file_exists( getfs_Handle* _fs, const char* path );
	// Create a file with the given name. Disregards the value of 'mode'.
	int           getfs_file_create( getfs_Handle* _fs, const char* _path, int );
	// Open the given file. Ignores any O_CREAT flag in 'oflag'.
	int           getfs_file_open( getfs_Handle* _fs, const char* _path, int _oflag );
	// Read data from the given file ( zip format )
	int           getfs_file_read( getfs_Handle* _fs, int _filedes, void* _buf, int _nbyte );
	// Read data from the given file ( original format )
	int           getfs_file_read_uncompress( getfs_Handle* _fs, int _filedes, void* _buf, int _nbyte );
	// Position the file pointer
	int           getfs_file_lseek( getfs_Handle* _fs, int _filedes, int _offset, int _whence );
	// Write data to the given file. If the file was open with
	// O_APPEND, any previous lseek() is ignored and the block
	// is appended to the end of the file
	int           getfs_file_write( getfs_Handle* _fs, int _filedes, const void* _buf, int _nbyte );
	int           getfs_file_set_size( getfs_Handle* _fs, const char* _path, int _original_size, int _zip_size );
	int           getfs_file_get_size( getfs_Handle* _fs, const char* _path, int& _original_size, int& _zip_size );
	// Close the file given by its file descriptor
	int           getfs_file_close( getfs_Handle* _fs, int _filedes );
	// Destroy a hard link or remove a file 
	int           getfs_file_unlink( getfs_Handle* _fs, const char* _path );
	// Search for file names matching the given pattern
	int           getfs_glob( getfs_Handle* _fs, const char* _pattern, int _flags, 
							int (*errfunc)(const char*, int), getfs_glob_t* _pglob );
	// Frees the memory allocated by a previous getfs_glob
	void          getfs_glob_free( getfs_Handle*, getfs_glob_t* _pglob );
	// filesystem version
	PACKVER       getfs_get_version( getfs_Handle* _fs );
	void          getfs_set_version( getfs_Handle* _fs, PACKVER& _ver );
	PACKVER       getfs_get_from_version( getfs_Handle* _fs );
	void          getfs_set_from_version( getfs_Handle* _fs, PACKVER& _ver );
	// Writes the textual error given by (fs->errno) to the standard output
	void          getfs_perror( getfs_Handle* _fs, const char* _s );
	//------------------------------------------------------------------------------
	void          getfs_compress( unsigned char* _dest, unsigned long* _destLen, const unsigned char* _source, unsigned long _sourceLen );
	int           getfs_uncompress( unsigned char* _dest, unsigned long* _destLen, const unsigned char* _source, unsigned long _sourceLen );
	//---------------------------------------------------------------------
	short int get_checkSum( unsigned char* temp, int Length );
	void WriteCheckSum( unsigned char* temp, short int checksum );
	short int SwapByte( short int _checksum );

	const char* getMd5Str( const char* _s );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_GETFS_H_
