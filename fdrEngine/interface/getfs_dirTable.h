// Directory Table (DT)
#ifndef _GETFS_DIRTABLE_H_
#define _GETFS_DIRTABLE_H_
#include "getfs_iio.h"

#define GETFS_DT_MAX_SUBNAME_SIZE     256  /* Max size of component part */
#define GETFS_DT_DEF_CHANNEL1_SIZE    1    /* Default channel size, in blocks */
#define GETFS_DT_DEF_CHANNEL2_SIZE    2    /* Default channel size, in blocks */
#define GETFS_DT_MAX_SKN_SIZE       60

#ifdef  GETFS_DT_MAX_SUBNAME_SIZE
#undef  GETFS_DT_MAX_SUBNAME_SIZE
#define GETFS_DT_MAX_SUBNAME_SIZE   4096
#endif

typedef struct getfs_dt_DT {
    getfs_iio_File*   file;                 /* IIO file */
    int             channel1;               /* IIO file channel #1 (Patricia nodes) */
    int             channel2;               /* IIO file channel #2 (keys) */
    int             unallocated;            /* head of free chain (internal) */
    int             unallocated2;           /* head of free chain for channel1 (keynodes) */
} getfs_dt_DT;


// Open a directory table
getfs_dt_DT* getfs_dt_open( getfs_iio_File* _file, int _channel1, int _channel2 );
// Create a new directory table in the given iio file & channel
getfs_dt_DT* getfs_dt_create( getfs_iio_File* _file, int _bpc1, int _bpc2 );
// Lookup file name in dirTable, return index in dirTable
int          getfs_dt_filename_lookup( getfs_dt_DT* _dt, const char* _filename );
// Add new file name, return index in channel
int          getfs_dt_filename_add( getfs_dt_DT* _dt, const char* _filename );
// Get the nodeTable index associated with a given filename (given by its index in the DT)
int          getfs_dt_filename_get_nt_index( getfs_dt_DT* _dt, int _filename_idx );
// Set the NT index associated with a given filename (given by its index in the DT)
int          getfs_dt_filename_set_nt_index( getfs_dt_DT* _dt, int _filename_idx, int _nt_idx );
// Close the directory table
int          getfs_dt_close( getfs_dt_DT* _dt );
// Destroy the directory table
int          getfs_dt_destroy( getfs_dt_DT* _dt );
// Globbing (pattern matching): lookup those names that match the given pattern and
// for each of those names call the given callback */
int          getfs_dt_filename_glob( getfs_dt_DT* _dt,
                         char* _pattern,
                         int _flags,
                         int (*p)(getfs_dt_DT* _dt, char* _filename, int _filename_idx, void* _tmp ),
                         void* _tmp ) ;
// Delete file name given by its index in table
int          getfs_dt_filename_delete( getfs_dt_DT* _dt, char* _filename );
// 设置文件的大小信息
int          getfs_dt_set_size_info( getfs_dt_DT* _dt, char* _filename, int _original_size, int _zip_size );
int          getfs_dt_get_size_info( getfs_dt_DT* _dt, char* _filename, int& _original_size, int& _zip_size );

#endif //_GETFS_DIRTABLE_H_