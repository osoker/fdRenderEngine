//Data Block Infrastructure with 1-level Sliding-Window Cache
//          (simulates the behavior of a physical multi-sector disk)
//          All the I/O accesses are made via an internal cache (configurable).
#ifndef _GETFS_DATA_H_
#define _GETFS_DATA_H_
#include <stdio.h>

extern int getfs_data_IOMODE;

#define GETFS_DATA_BLOCK_SIZE      512                         // Basic data block: 1K
#define GETFS_DATA_DEF_CACHE_SIZE  (128 * GETFS_DATA_BLOCK_SIZE) // Default cache size: 128 blocks = 64K
#define GETFS_DATA_SYNC            1                           // Constant for cache's sync state
#define GETFS_DATA_NOT_SYNC        0                           // Constant for cache's unsync state

typedef struct getfs_data_Cache {
    int   size;             /* cache size */
    char* buffer;           /* buffer, see size above */
    int   position;         /* start position in the file */
    int   sync;             /* in sync? 1=true, 0=false(needs flushing) */
} getfs_data_Cache;

typedef struct getfs_data_Data {
    FILE*            file;     /* file where the data (virtual disk) is stored */
    char*            filename; /* name of data file */
    getfs_data_Cache*  cache;    /* cache for this virtual disk */
} getfs_data_Data;


// Open an existing virtual disk
getfs_data_Data*  getfs_data_open( const char* _filename );
// Create a new virtual disk
getfs_data_Data*  getfs_data_create( const char* _filename );
// Read one block of data from block number 'block_no' into the memory referenced by 'to'
int               getfs_data_read( getfs_data_Data* _data, int _block_no, void* _to );
// Write one block of data from 'from' to absolute block number 'block_no'
int               getfs_data_write( getfs_data_Data* _data, int _block_no, void* _from );
// Close (but do not destroy) the given virtual disk
void              getfs_data_close( getfs_data_Data* _data );
// Close and destroy (remove) the given virtual disk
void              getfs_data_destroy( getfs_data_Data* _data );


#endif //_GETFS_DATA_H_