#ifndef _GETFS_IIO_H_
#define _GETFS_IIO_H_
#include <stdio.h>

extern int getfs_iio_IOMODE;

#define getfs_iio_BLOCK_SIZE         256       /* default: size of a block */
#define getfs_iio_CC_DEF_TABSIZE     1024      /* default: initial size of the cache */
#define getfs_iio_CC_MAX_ACTIVE_PGS  8         /* maximum number of active pages */
#define getfs_iio_CC_SYNC_MASK       1         /* mask for sync flag */
#define getfs_iio_CC_BUF_MASK        (~getfs_iio_CC_SYNC_MASK) /* mask for actual page buffer */

#define getfs_iio_CC_RESTRICT_GROWTH           /* if defined, will restrict the growth of the cache */
#define getfs_iio_CC_MEMSET_OPT                /* tries to avoid useless memsets */

// cache page switching policies
#define ___LRU___                   1         /* least recently used */
#define ___LFU___                   2         /* least frequently used */
#define ___NLN___                   3         /* next lower neighbor */
#define getfs_iio_CC_POLICY           ___LRU___ /* policy for page switching */

typedef struct getfs_iio_Cache_Page {
    char* buffer;                   /* lowest 1 bit: sync flag; rest: cache buffer for this page */
    int   position;                 /* position within channel */
#if (getfs_iio_CC_POLICY == ___LRU___)
    int   timestamp;
#endif
#if (getfs_iio_CC_POLICY == ___LFU___)
    int   hits;
#endif
} getfs_iio_Cache_Page;

typedef struct getfs_iio_Cache {
    getfs_iio_Cache_Page** pages;    /* cache pages */
    int                   nr_pages; /* number of pages currently in the cache */
    int                   active_pages; /* number of active pages */
} getfs_iio_Cache;

typedef struct getfs_iio_Channel {
	int blocks;                     /* number of blocks in this channel */
	int size;                       /* bytes currently in this channel */
	int cp;                         /* channel pointer for this channel */
	getfs_iio_Cache* cache;           /* channel cache */
} getfs_iio_Channel;

typedef struct getfs_iio_File {
	FILE* file;                     /* interlaced I/O file ptr */
	char* filename;                 /* name of .iio file */
	short channels;                 /* number of channels */
	getfs_iio_Channel** chn;        /* array of channels */

	//包版本号
	int                ver_main;    
	int                ver_minor;
	int                ver_patch;

	//用于升级包版本号
	int                ver_from_main;  
	int                ver_from_minor;
	int                ver_from_patch;
} getfs_iio_File;



// Create an Interlaced I/O file
getfs_iio_File*        getfs_iio_create( const char* _filename );

// Close and remove an IIO file
void                 getfs_iio_destroy( getfs_iio_File* _file );

// Open existing IIO file
getfs_iio_File*        getfs_iio_open( const char* _filename );

// Close an IIO file, do not remove it
void                 getfs_iio_close( getfs_iio_File* _file );

// Allocate a new channel for the given IIO file, with a certain number of blocks per channel
int                  getfs_iio_allocate_channel( getfs_iio_File* _file, int _blocks );

// Write 'size' bytes to the given channel from 'data', return nr. of bytes written
int                  getfs_iio_write( getfs_iio_File* _file, int _channel, void* _data, int _size );

// Read 'size' bytes from the given channel into 'data', return nr. of bytes read
int                  getfs_iio_read( getfs_iio_File* _file, int _channel, void* _data, int _size );

// Positions the channel pointer to a certain position in the given channel
int                  getfs_iio_seek( getfs_iio_File* _file, int _channel, int _where );

// Returns the size of a chunk (in blocks) for the given IIO file
int                  getfs_iio_blocks_per_chunk( getfs_iio_File* _file );

// Returns the channel number 'N' from the given IIO file */
getfs_iio_Channel*     getfs_iio_get_channel( getfs_iio_File* _file, int _channel );

// Returns the current size of a given channel (in bytes)
int                  getfs_iio_channel_size( getfs_iio_Channel* _channel );

// Returns the size (in blocks) of the given channel
int                  getfs_iio_channel_blocks( getfs_iio_Channel* _channel );

// Truncates a channel at the current channel position
int                  getfs_iio_channel_truncate( getfs_iio_File* _file, int _channel );

#endif //_GETFS_IIO_H_