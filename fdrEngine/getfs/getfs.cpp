//---------------------------------------------------------------------------------------
/**  \file
*   Filename: getfs.cpp
*   Desc:     
*   His:      yq_sun created @ 2005-05-16
*/
//---------------------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <fcntl.h>
#include <assert.h>
#include <string>

#include "getfs.h"
#include "getfs_iio.h"
#include "getfs_data.h"
#include "getfs_dirTable.h"
#include "getfs_pmatch.h"

#include "getfs_debug.h"

#include "md5.h"

#include "../interface/zlib.h"

//#ifdef _DEBUG
//#pragma comment( lib, "../lib/zlib.lib" )
//#else
//#pragma comment( lib, "../lib/zlibR.lib" )
//#endif


int getfs_errno = 0;

int g_curTmpBufferSize = 0;

CRITICAL_SECTION  g_sReadFile;
CRITICAL_SECTION  g_sWriteFile;
CRITICAL_SECTION  g_sFileExist;
CRITICAL_SECTION  g_sTrienode_get;
CRITICAL_SECTION  g_sOpenFile;
CRITICAL_SECTION  g_sCloseFile;
CRITICAL_SECTION  g_sSetSize;
CRITICAL_SECTION  g_sIIORead;




unsigned char* get_tmpBuffer( getfs_Handle* _fs, int _size = MAX_TEMPBUFFER_SIZE );

//get temp buffer
unsigned char* get_tmpBuffer( getfs_Handle* _fs, int _size/*=MAX_TEMPBUFFER_SIZE*/ )
{
	assert( _fs );
	if( g_curTmpBufferSize < _size ){
		if( _fs->getfs_buffer )
			free( _fs->getfs_buffer );
		int newsize = _size;
		_fs->getfs_buffer = (unsigned char*) malloc( newsize );
		g_curTmpBufferSize = newsize;
	}
	return _fs->getfs_buffer;
}
// true if the file exists, false otherwise
static int file_exists( const char* _filename ) 
{
	FILE* f = fopen( _filename, "rb" );
	if( f == NULL )
		return 0;
	fclose( f );
	return 1;
}

static int lock_check( const char* _filename, int _mode )
{
#if GETFS_USELOCK

	FILE* f;
	int mm, ret;
	char lock_name[GETFS_NAME_LEN];

	strcpy( lock_name, _filename );
	strcpy( &lock_name[strlen(_filename)], GETFS_LOCK_SUFFIX );
	f = fopen( lock_name, "rb" );
	if( f == NULL )
		return 1; /* lock not found; access granted */

	// read mode
	fseek( f, 0, 0 );
	fread( &mm, 1, sizeof(int), f );

	if( mm & FS_WRITE )
		ret = 0; // no other processes are allowed when FS locked for writing
	else
		if(mm & FS_READ)
			ret = (_mode == FS_READ); /* only readers are allowed */
		else
			ret = 0; /* invalid mode */
	fclose(f);
	return ret;

#else
	return 1;
#endif
}

// allocate a new (empty) file descriptor and update the file system handle to reflect the new state
static int allocate_file_descriptor( getfs_Handle* _fs )
{
    int i = 0;
    getfs_FH* fh = (getfs_FH*)malloc( sizeof(getfs_FH) );
    if( fh == NULL )
        return -1;
    fh->dt_index = 0;
    fh->nt_index = 0;
    fh->chain    = 0;
    fh->fp       = 0;
    
    // check if h-table is already open, and look for an available slot
    if( _fs->handles != NULL ) 
	{
        while( ( i < _fs->ht_size ) && ( _fs->handles[i] != NULL ) )
            i++;
        if( i >= _fs->ht_size )
		{
            // has to be expanded
            _fs->handles = (getfs_FH**)realloc( _fs->handles, sizeof(getfs_FH*) * (_fs->ht_size * 2 ) );
            if( _fs->handles == NULL )
                return -1;
            for( i = _fs->ht_size; i < (_fs->ht_size * 2); _fs->handles[i++] = NULL );
            i = _fs->ht_size; // .. which is obviously empty
            _fs->ht_size <<= 1; // double it!
            _fs->handles[i] = fh;
            return i;
        } 
		else 
		{
            // empty slot
            _fs->handles[i] = fh;
            return i;
        }
    }
    
    // first time a file is open
    _fs->ht_size = GETFS_INITIAL_HT_SIZE;
    _fs->handles = (getfs_FH**)malloc( sizeof(getfs_FH*) * _fs->ht_size );
    if( _fs->handles == NULL )
        return -1;
    for( i = 0; i < _fs->ht_size; _fs->handles[i++] = NULL );
    _fs->handles[0] = fh;
    return 0; // first index chosen
}

// truncate a file
static void file_truncate( getfs_Handle* _fs, getfs_FH* _fh, int _size )
{
    int node_no, node;
    
    // truncate the chain
    node_no = _size / GETFS_DATA_BLOCK_SIZE;
    node    = getfs_fat_chain_get_nth( _fs->fat, _fh->chain, node_no );
    getfs_fat_chain_truncate( _fs->fat, node );
    
    // update the size of data (field in the node in NT)
    getfs_nt_node_set_size( _fs->nt, _fh->nt_index, _size );
}

// Returns nonzero (true) if the given file system is available, false (0) if not
int getfs_exists( const char* _filename )
{
	char lli_name[GETFS_NAME_LEN];
	char llp_name[GETFS_NAME_LEN];
	// form the appropriate names for lli/llp
	strcpy(lli_name, _filename);
	strcpy(&lli_name[strlen(_filename)], GETFS_IIO_SUFFIX );
	strcpy(llp_name, _filename);
	strcpy(&llp_name[strlen(_filename)], GETFS_DAT_SUFFIX );
	// do these files exist? 
	return ( file_exists(lli_name) && file_exists(llp_name) );
}

// Returns nonzero if the given file exists. Faster than opening the file for reading.
int getfs_file_exists( getfs_Handle* _fs, const char* path )
{
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return 0;
    }
    
    // look for the file name in the DT
    return ( getfs_dt_filename_lookup( _fs->dt, (char*)path ) >= 0 );
}

// Create a file with the given name. Disregards the value of 'mode'.
int getfs_file_create( getfs_Handle* _fs, const char* _path, int )
{
    int          filedes, oflag;
    getfs_FH*     fh;
    int          dt_idx;
    getfs_nt_Node ni;
    int          already = 1;
    
    if( _fs == NULL ) 
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return -1;
    }
    
    // look for the file name in the DT, create a new one if not found 
    dt_idx = getfs_dt_filename_lookup( _fs->dt, (char*)_path );
    if( dt_idx < 0 )
	{
        already = 0;
        dt_idx = getfs_dt_filename_add( _fs->dt, (char*)_path );
        if( dt_idx < 0 )
		{
            getfs_errno = GETFS_ERR_DT_INTERNAL;
            return -1;
        }
    }
    
    // allocate a new (and empty) file descriptor
    filedes = allocate_file_descriptor( _fs );
    if( filedes < 0 )
	{
        getfs_errno = GETFS_ERR_NOMEM_OPEN;
        return -1;
    }
    
    // increment the number of files open 
    _fs->fls_open++;
    
    // initialize descriptor 
    fh = _fs->handles[filedes];
    fh->oflags   = O_CREAT | O_RDWR | O_TRUNC; // default oflags for "creat" 
    oflag = fh->oflags;
    fh->dt_index = dt_idx;  // index in dirTable
    if( already )
        // index of existing node in nodeTable
        fh->nt_index = getfs_dt_filename_get_nt_index( _fs->dt, dt_idx );
    else 
	{
        // index of new node in nodeTable
        fh->nt_index = getfs_nt_allocate_node( _fs->nt );
        getfs_dt_filename_set_nt_index( _fs->dt, dt_idx, fh->nt_index ); // link
    }
    getfs_nt_get_node( _fs->nt, fh->nt_index, &ni );
    if( !already )
	{
        // index of new chain head in FAT
        ni.chain     = getfs_fat_create_chain( _fs->fat );
        getfs_nt_set_node( _fs->nt, fh->nt_index, &ni );
    }
    fh->chain    = ni.chain; // index in FAT
    fh->fp       = 0;
    
    // truncate file if requested
    if ( ((oflag & O_RDWR) || (oflag & O_WRONLY)) &&
        (oflag & O_TRUNC) &&
        already ) 
	{
        file_truncate( _fs, fh, 0 );
    }
    
    // return the descriptor
    return filedes;
}

/* create a lock file, using the given mode (read/write),
and initialize the lock's refcount to 1 */
static int lock_create( const char* _filename, int _mode )
{
#if GETFS_USELOCK
    int refcount;
    FILE* f;
    char lock_name[GETFS_NAME_LEN];
    
    // open lock
    strcpy( lock_name, _filename );
    strcpy( &lock_name[strlen(_filename)], GETFS_LOCK_SUFFIX );
    f = fopen( lock_name, "wb" );
    if( f == NULL )
        return -1; // could not create lock
    fwrite( &_mode, 1, sizeof(int), f );
    refcount = 1;
    fwrite( &refcount, 1, sizeof(int), f );
    fclose( f );
    return 0;
#else
	return 0;
#endif
}

/* "enters" the pool of processes that are allowed to
use the locked file system at the same time. The refcount
is updated to reflect the new number of processes
accessing the lock simultaneously */
static int lock_enter( const char* _filename, int _mode )
{
#if GETFS_USELOCK
    int refcount, r;
    FILE* f;
    char lock_name[GETFS_NAME_LEN];
    strcpy( lock_name, _filename );
    strcpy( &lock_name[strlen(_filename)], GETFS_LOCK_SUFFIX );
    f = fopen( lock_name, "r+b" );
    if( f == NULL )
        return lock_create( _filename, _mode );
    
    // write access mode
    fseek( f, 0, 0 );
    fwrite( &_mode, 1, sizeof(int), f );
    fflush( f );
    
    // read refcount
    fseek( f, sizeof(int), 0 );
    r = fread( &refcount, 1, sizeof(int), f );
    if( r != sizeof(int) )
        refcount = 0;
    
    // increment
    refcount++;
    
    // write new refcount
    fflush( f );
    fseek( f, sizeof(int), 0 );
    fwrite( &refcount, 1, sizeof(int), f );
    
    fclose( f );
    return 0;
#else
	return 0;
#endif
}

// removes a lock file
static int lock_remove( const char* _filename )
{
#if GETFS_USELOCK
    char lock_name[GETFS_NAME_LEN];
    strcpy( lock_name, _filename );
    strcpy( &lock_name[strlen(_filename)], GETFS_LOCK_SUFFIX );
    return remove( lock_name );
#else
	return 0;
#endif
}

// leave a locked system, and update the lock's refcount to reflect the new number of processes accessing the
// locked system simultaneously
static int lock_leave( const char* _filename )
{
#if GETFS_USELOCK
    FILE* f;
    int   mode, refcount = 0;
    char lock_name[GETFS_NAME_LEN];
    
    strcpy( lock_name, _filename );
    strcpy( &lock_name[strlen(_filename)], GETFS_LOCK_SUFFIX );
    
    // open
    f = fopen( lock_name, "r+b" );
    if( f == NULL )
        return -1;
    
    // read mode and refcount
    fseek( f, 0, 0 ); 
    fread( &mode, 1, sizeof(int), f );
    fread( &refcount, 1, sizeof(int), f );
    
    // decrement
    refcount--;
    
    // rewrite refcount
    fflush( f );
    fseek( f, sizeof(int), 0 );
    fwrite( &refcount, 1, sizeof(int), f );
    
    // close file
    fclose( f );
    
    // refcount <= 0 ? then remove lock
    if( refcount <= 0 )
        return lock_remove( _filename );
    
    return 0;
#else
	return 0;
#endif
}

static int blockno( int _fp )
{
    return ( _fp / GETFS_DATA_BLOCK_SIZE );
}

static int nblocks( int _size )
{
    if( _size == 0 )
        return 1;
    return ( ( ( _size - 1 ) / GETFS_DATA_BLOCK_SIZE ) + 1 );
}

static int deallocate_file_descriptor( getfs_Handle* _fs, int _filedes )
{
    if( _fs == NULL )
        return GETFS_ERR_INVALID_FS;
    
    if( _fs->handles[_filedes] == NULL )
        return GETFS_ERR_ALREADY_CLSD;
    
    // deallocate the descriptor
    free( _fs->handles[_filedes] );
    _fs->handles[_filedes] = NULL;
    
    return 0;
}

typedef struct getfs_tmp {
    getfs_glob_t* pglob;
    char*        pattern;
    int          flags;
    int          (*e)(const char*, int);
    int          errnum;
} getfs_tmp;

static int __getfs_glob_cbk( getfs_dt_DT*, char* _filename, int, void* _t ) 
{
    int n;
    getfs_tmp* tmp = (getfs_tmp*)_t;
    int must_extend = 0;
    
    // check if the array needs to be extended
    if( !(tmp->pglob->gl_pathc & 0x1F ) )
        must_extend = 1;
    
    // found one more match!
    tmp->pglob->gl_pathc++;
    
    // how many slots?
    n = tmp->pglob->gl_pathc;
    if( tmp->flags & GLOB_DOOFFS )
        n += tmp->pglob->gl_offs;
    
    // reallocate if necessary
    if( must_extend )
	{
        tmp->pglob->gl_pathv = (char**)realloc( tmp->pglob->gl_pathv, ( ( n-1 ) + 40 ) * sizeof(char*) );
        if( tmp->pglob->gl_pathv == NULL )
		{
            tmp->pglob->gl_pathc--;
            if( tmp->e )
                tmp->e( _filename, GLOB_NOSPACE );
            tmp->errnum = GLOB_NOSPACE; // set the error code
            return 0; // stop
        }
    }
    
    // create a new filename in the next available slot
    tmp->pglob->gl_pathv[n-1] = (char*)malloc( strlen(_filename) + 4 );
    strcpy( tmp->pglob->gl_pathv[n-1], _filename );
    
    // the first pointer after the last pathname must be NULL
    tmp->pglob->gl_pathv[n] = NULL;
    
    if( tmp->flags & GLOB_FIRSTONLY )
        return 0; // stop after the first one
    
    return 1;
}

// Writes the textual error given by (getfs_errno) to the standard output
void getfs_perror( getfs_Handle*, const char* _s )
{
    char* e = "Internal error";
    
    switch((char)getfs_errno )
	{
    case GETFS_ERR_IIO_INIT:
        e = "Could not open/create the IIO file"; break;
    case GETFS_ERR_NO_DATA:
        e = "Could not open/create the data file"; break;
    case GETFS_ERR_NO_FAT:
        e = "Could not initialize the FAT channel"; break;
    case GETFS_ERR_NO_NT:
        e = "Could not initialize the NT channel"; break;
    case GETFS_ERR_NO_DT:
        e = "Could not initialize the DT channel"; break;
    case GETFS_ERR_A_FAT:
        e = "Could not allocate the FAT channel"; break;
    case GETFS_ERR_A_NT:
        e = "Could not allocate the NT channel"; break;
    case GETFS_ERR_A_DT:
        e = "Could not allocate the DT channel"; break;
    case GETFS_ERR_INVALID_FS:
        e = "Invalid file system handle"; break;
    case GETFS_ERR_ALREADY_CLSD:
        e = "File already closed"; break;
    case GETFS_ERR_NOT_FOUND:
        e = "File not found"; break;
    case GETFS_ERR_NO_OPEN:
        e = "No files open"; break;
    case GETFS_ERR_INVALID_FH:
        e = "Invalid file descriptor"; break;
    case GETFS_ERR_INVALID_NT:
        e = "Invalid node in NT"; break;
    case GETFS_ERR_NOMEM_OPEN:
        e = "No memory available for opening files"; break;
    case GETFS_ERR_LOCKED:
        e = "File system is locked by another process"; break;
    case GETFS_ERR_DUPLICATEF:
        e = "The file you are trying to create already exists"; break;
    case GETFS_ERR_DT_INTERNAL:
        e = "Internal error in DT"; break;
    case GETFS_ERR_INVALID_PARMS:
        e = "Invalid parameters"; break;
    default:
        e = "Internal error";
    }
    
    printf( "%s: %s\n", _s, e );
}

// Open if possible (otherwise create) the given file system, return handle
getfs_Handle* getfs_start( const char* _filename, int _flags ) 
{
	::InitializeCriticalSection( &g_sReadFile );
	::InitializeCriticalSection( &g_sWriteFile );
	::InitializeCriticalSection( &g_sFileExist );
	::InitializeCriticalSection( &g_sTrienode_get );
	::InitializeCriticalSection( &g_sOpenFile );
	::InitializeCriticalSection( &g_sCloseFile );
	::InitializeCriticalSection( &g_sSetSize );
	::InitializeCriticalSection( &g_sIIORead );
	

	
	
	

	char iio_name[GETFS_NAME_LEN];
	char data_name[GETFS_NAME_LEN];
	getfs_Handle* fs = NULL;

	// form the appropriate file names 
	strcpy(iio_name, _filename);
	strcpy(&iio_name[strlen(_filename)], GETFS_IIO_SUFFIX);
	strcpy(data_name, _filename);
	strcpy(&data_name[strlen(_filename)], GETFS_DAT_SUFFIX);

	// allocate new file system handle
	fs = (getfs_Handle*)malloc( sizeof(getfs_Handle) );
	fs->ht_size  = 0;
	fs->handles  = 0;
	getfs_errno   = 0;
	fs->fls_open = 0;

	// set the internal access mode flags 
	getfs_iio_IOMODE  = _flags;
	getfs_data_IOMODE = _flags;

	if( !lock_check( _filename, _flags ) ) 
	{
		// access denied - FS locked
		getfs_errno = GETFS_ERR_LOCKED;
		free( fs );
		return GETFS_NULL;
	}

	// if the files exist, open them, otherwise create them
	if( file_exists(iio_name) && file_exists(data_name) ) 
	{
		// open it
		// open the IIO file
		fs->file = getfs_iio_open( iio_name );
		if( fs->file == NULL )
		{
			getfs_errno = GETFS_ERR_IIO_INIT;
			free( fs );
			return GETFS_NULL;
		}

		// open the data file
		fs->data = getfs_data_open( data_name );
		if( fs->data == NULL )
		{
			getfs_errno = GETFS_ERR_NO_DATA;
			free( fs );
			return GETFS_NULL;
		}

		// Initialize the DT
		fs->dt = getfs_dt_open( fs->file, GETFS_CHANNEL_DT1, GETFS_CHANNEL_DT2 );
		if( fs->dt == NULL )
		{
			getfs_errno = GETFS_ERR_NO_DT;
			free( fs );
			return GETFS_NULL;
		}

		// Initialize the NT
		fs->nt = getfs_nt_open( fs->file, GETFS_CHANNEL_NT );
		if( fs->nt == NULL ) 
		{
			getfs_errno = GETFS_ERR_NO_NT;
			free( fs );
			return GETFS_NULL;
		}

		// Initialize the FAT
		fs->fat = getfs_fat_open( fs->file, GETFS_CHANNEL_FAT );
		if( fs->fat == NULL ) 
		{
			getfs_errno = GETFS_ERR_NO_FAT;
			free( fs );
			return GETFS_NULL;
		}

	} 
	else 
	{
		// The file system does not exist, create it

		/* if the file system does not exist and it's supposed
		to be open for reading => error, the file(s) do(es)
		not exist. */
		if( _flags == FS_READ )
		{
			getfs_errno = GETFS_ERR_IIO_INIT;
			free( fs );
			return GETFS_NULL;
		}

		// create a new IIO file
		fs->file = getfs_iio_create( iio_name );
		if( fs->file == NULL )
		{
			getfs_errno = GETFS_ERR_IIO_INIT;
			free( fs );
			return GETFS_NULL;
		}

		// create a new data file
		fs->data = getfs_data_create( data_name );
		if( fs->data == NULL ) 
		{
			getfs_errno = GETFS_ERR_NO_DATA;
			free( fs );
			return GETFS_NULL;
		}

		// Create the DT
		fs->dt = getfs_dt_create( fs->file, GETFS_CHANNEL_DT1_SIZE, GETFS_CHANNEL_DT2_SIZE );
		if( fs->dt == NULL )
		{
			getfs_errno = GETFS_ERR_A_DT;
			free( fs );
			return GETFS_NULL;
		}

		// Create the NT
		fs->nt = getfs_nt_create( fs->file, GETFS_CHANNEL_NT_SIZE );
		if( fs->nt == NULL )
		{
			getfs_errno = GETFS_ERR_A_NT;
			free( fs );
			return GETFS_NULL;
		}

		/* Create the FAT */
		fs->fat = getfs_fat_create( fs->file, GETFS_CHANNEL_FAT_SIZE );
		if( fs->fat == NULL )
		{
			getfs_errno = GETFS_ERR_A_FAT;
			free( fs );
			return GETFS_NULL;
		}
	}

	fs->filename = (char*)malloc(128);
	strcpy( fs->filename, _filename );

	fs->getfs_buffer = 0;
	fs->getfs_buffer = get_tmpBuffer( fs,MAX_TEMPBUFFER_SIZE );//(unsigned char*) malloc( MAX_TEMPBUFFER_SIZE );

	// create or update the lock
	lock_enter( _filename, _flags );


	

	// return the file system handle
	return fs;
}



// Open the given file. O_CREAT is ignored.
int getfs_file_open( getfs_Handle* _fs, const char* _path, int _oflag )
{
	::EnterCriticalSection( &g_sOpenFile );	

    int          filedes;
    getfs_FH*     fh;
    int          dt_idx;
    getfs_nt_Node ni;
    
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
		::LeaveCriticalSection( &g_sOpenFile );
        return -1;
    }
    
    // look for the file name in the DT, error if not found 
    dt_idx = getfs_dt_filename_lookup( _fs->dt, (char*)_path );
    if( dt_idx < 0 )
	{
        getfs_errno = GETFS_ERR_NOT_FOUND;
		::LeaveCriticalSection( &g_sOpenFile );
        return -1;
    }
    
    // allocate a new (and empty) file descriptor
    filedes = allocate_file_descriptor( _fs );
    if( filedes < 0 )
	{
        getfs_errno = GETFS_ERR_NOMEM_OPEN;
		::LeaveCriticalSection( &g_sOpenFile );
        return -1;
    }
    
    // increment the number of files open 
    _fs->fls_open++;
    
    // initialize descriptor 
    fh = _fs->handles[filedes];
    fh->oflags   = _oflag;   // oflags 
    fh->dt_index = dt_idx;  // index in DT
    fh->nt_index = getfs_dt_filename_get_nt_index( _fs->dt, dt_idx ); // index in NT 
    getfs_nt_get_node( _fs->nt, fh->nt_index, &ni );
    fh->chain    = ni.chain; // index in FAT 
    fh->fp       = 0;
    
    // truncate file if requested 
    if( ( ( _oflag & O_RDWR) || ( _oflag & O_WRONLY ) ) &&
        ( _oflag & O_TRUNC ) ) 
	{
        file_truncate( _fs, fh, 0 );
    }

	::LeaveCriticalSection( &g_sOpenFile );

    // return the descriptor 
    return filedes;
}

// Position the file pointer
int getfs_file_lseek( getfs_Handle* _fs, int _filedes, int _offset, int _whence )
{
    int fp, size;
    
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return -1;
    }
    if( _fs->handles == NULL) 
	{
        getfs_errno = GETFS_ERR_NO_OPEN;
        return -1;
    }
    if( _fs->handles[_filedes] == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FH;
        return -1;
    }
    
    // get file size
    size = getfs_nt_node_get_size( _fs->nt, _fs->handles[_filedes]->nt_index );
    
    // whence? 
    if( _whence == (int)SEEK_SET )
	{
        // relative to the beginning
        fp = _offset;
    } 
	else if( _whence == (int)SEEK_CUR )
	{
        // relative to the current position
        fp = _fs->handles[_filedes]->fp + _offset;
    }
	else if( _whence == (int)SEEK_END )
	{
        // relative to the end of the file
        fp = size + _offset;
    } 
	else 
	{
		// To avoid compiler warnings. If whence doesn't have a valid value, use SEEK_SET.
        fp = _offset;
    }
    
    // change the file pointer
    _fs->handles[_filedes]->fp = fp;
    
    // update the FAT chain and size (in NT) if necessary
    if( blockno( fp ) >= nblocks( size ) )
	{
        int node = _fs->handles[_filedes]->chain;
        int sz   = size;
        while( blockno( fp ) >= nblocks( sz ) )
		{
            node = getfs_fat_chain_extend( _fs->fat, node );
            sz  += GETFS_DATA_BLOCK_SIZE;
        }
    }
    
    // update the size (in NT), if necessary
    if( size < fp )
        getfs_nt_node_set_size( _fs->nt, _fs->handles[_filedes]->nt_index, ( fp + 1 ) );
    
    return 0;
}

// Read data from the given file
int getfs_file_read( getfs_Handle* _fs, int _filedes, void* _buf, int _nbyte )
{
    int fp, size;
    int startb, endb;
    int start_in_block, end_in_block;
    int node, idx;
    int sidx, eidx, kk, pp;
    
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return -1;
    }
    if( _fs->handles == NULL )
	{
        getfs_errno = GETFS_ERR_NO_OPEN;
        return -1;
    }
    if( _fs->handles[_filedes] == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FH;
        return -1;
    }
    
    if( _nbyte == 0 )
        return 0;
    
    // get file size and pointer
    fp   = _fs->handles[_filedes]->fp;
    size = getfs_nt_node_get_size( _fs->nt, _fs->handles[_filedes]->nt_index );
    
    /* adjust nbyte if necessary - it may not be possible to read
    that many bytes, if the file pointer is approaching the
    end of file */
    if( fp + _nbyte > size )
        _nbyte = size - fp;
    
    if( _nbyte <= 0 )
        return 0;
    
    // compute the range of nodes needed
    startb         = ( fp / GETFS_DATA_BLOCK_SIZE );
    endb           = ( ( fp + _nbyte - 1 ) / GETFS_DATA_BLOCK_SIZE );
    start_in_block = ( fp % GETFS_DATA_BLOCK_SIZE );
    end_in_block   = ( ( fp + _nbyte - 1 ) % GETFS_DATA_BLOCK_SIZE );
    
    // loop
    node = getfs_fat_chain_get_nth( _fs->fat, _fs->handles[_filedes]->chain, startb );
    idx  = startb;
    kk   = 0;
    while( idx <= endb )
	{
        // sidx, eidx
        if( idx == startb )
            sidx = start_in_block;
        else
            sidx = 0;
        if( idx == endb )
            eidx = end_in_block;
        else
            eidx = ( GETFS_DATA_BLOCK_SIZE - 1 );

        // read (conditional split for efficiency)
        if( ( sidx == 0 ) && ( eidx == ( GETFS_DATA_BLOCK_SIZE - 1 ) ) )
            // one full block, fast
            pp = getfs_data_read( _fs->data, node, (char*)_buf + kk );
        else {
            char tmpbuf[GETFS_DATA_BLOCK_SIZE];
            // a portion of a block (slower)
            pp = getfs_data_read( _fs->data, node, tmpbuf );
            memcpy( (char*)_buf + kk, &tmpbuf[sidx], (eidx - sidx + 1) );
        }
        // next node in chain
        node = getfs_fat_chain_get_nth( _fs->fat, node, 1 );
        idx++;
        if( pp == 0 )
            kk += (eidx - sidx + 1);
    }
    
    // update the file pointer with the number of bytes actually read
	// 不需要记录每次读入的字节数
    //_fs->handles[_filedes]->fp = fp + _nbyte;  
    
    // return the amount of data that was actually read
    return _nbyte;
}

int getfs_file_read_uncompress( getfs_Handle* _fs, int _filedes, void* _buf, int _nbyte )
{
	::EnterCriticalSection( &g_sReadFile );	

	unsigned char* buf = (unsigned char*)get_tmpBuffer(_fs);
	int read_size = getfs_file_read( _fs, _filedes, buf, g_curTmpBufferSize );
	if( read_size <= 0 ){
		debug_out( "getfs_file_read_uncompress: getfs_file_read: read_size<%d>\n", read_size );
		::LeaveCriticalSection( &g_sReadFile );
		return read_size;
	}

	//uncompress
	int k = getfs_uncompress( (unsigned char*)_buf, (unsigned long*)&_nbyte, (unsigned char*)get_tmpBuffer(_fs), read_size );
	if( k != Z_OK ){
		debug_out( "getfs_file_read_uncompress: getfs_uncompress: kill<%d>\n", k );
		
		//! river @ 2010-9-17:返回上层errorCode.
		((int*)_buf)[0] = k;

		::LeaveCriticalSection( &g_sReadFile );
		return -2;
	}
	
	::LeaveCriticalSection( &g_sReadFile );

	return _nbyte;
}

// Write data to the given file. If the file was open with
// O_APPEND, any previous lseek() is ignored and the block
// is appended to the end of the file
int getfs_file_write( getfs_Handle* _fs, int _filedes, const void* _buf, int _nbyte )
{
	::EnterCriticalSection( &g_sWriteFile );

    int fp, size;
    int startb, endb;
    int start_in_block, end_in_block;
    int node, idx;
    int sidx, eidx, kk;
    
    // dumb-proof
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
		::LeaveCriticalSection( &g_sWriteFile );
        return -1;
    }
    if( _fs->handles == NULL ) 
	{
        getfs_errno = GETFS_ERR_NO_OPEN;
		::LeaveCriticalSection( &g_sWriteFile );
        return -1;
    }
    if( _fs->handles[_filedes] == NULL)
	{
        getfs_errno = GETFS_ERR_INVALID_FH;
		::LeaveCriticalSection( &g_sWriteFile );
        return -1;
    }
    
    // if nbyte is zero, "write()" returns immediately - so do we.
    if( _nbyte <= 0 )
	{
		::LeaveCriticalSection( &g_sWriteFile );
        return 0;
	}
    
    // get file size and pointer
    fp   = _fs->handles[_filedes]->fp;
    size = getfs_nt_node_get_size( _fs->nt, _fs->handles[_filedes]->nt_index );
    
    // if the file was open for appending (O_APPEND), write() will ignore any previous lseek() - so do we
    if( ( _fs->handles[_filedes]->oflags & O_APPEND) && ( fp < size ) )
	{
        getfs_file_lseek( _fs, _filedes, size, (int)SEEK_SET );
        size = getfs_nt_node_get_size( _fs->nt, _fs->handles[_filedes]->nt_index );
        fp   = _fs->handles[_filedes]->fp;
    }
    
    //. compute the range of nodes needed
    startb         = ( fp / GETFS_DATA_BLOCK_SIZE );
    endb           = ( ( fp + _nbyte - 1 ) / GETFS_DATA_BLOCK_SIZE );
    start_in_block = ( fp % GETFS_DATA_BLOCK_SIZE );
    end_in_block   = ( ( fp + _nbyte - 1 ) % GETFS_DATA_BLOCK_SIZE );
    
    // extend the FAT chain beforehand, as needed (faster this way)
    getfs_file_lseek( _fs, _filedes, fp + _nbyte - 1, (int)SEEK_SET );
    getfs_file_lseek( _fs, _filedes, fp,              (int)SEEK_SET );
    
    // loop
    node = getfs_fat_chain_get_nth( _fs->fat, _fs->handles[_filedes]->chain, startb );
    idx  = startb;
    kk   = 0;
    while( idx <= endb )
	{
        // sidx, eidx
        if( idx == startb )
            sidx = start_in_block;
        else
            sidx = 0;
        if( idx == endb )
            eidx = end_in_block;
        else
            eidx = ( GETFS_DATA_BLOCK_SIZE - 1 );
        // write (conditional split for efficiency)
        if( ( sidx == 0) && ( eidx == ( GETFS_DATA_BLOCK_SIZE - 1 ) ) )
            // one full block, fast
            getfs_data_write( _fs->data, node, (char*)_buf + kk );
        else 
		{
			char tmpbuf[GETFS_DATA_BLOCK_SIZE]={0};
            // a portion of a block (slower)
            getfs_data_read( _fs->data, node, tmpbuf );
            memcpy( &tmpbuf[sidx], (char*)_buf + kk, ( eidx - sidx + 1 ) );
            getfs_data_write( _fs->data, node, tmpbuf );
        }
        // next node in chain
        node = getfs_fat_chain_get_nth( _fs->fat, node, 1 );
        idx++;
        kk += ( eidx - sidx + 1 );
    }
    
    // update the file pointer with the number of bytes actually written
    getfs_file_lseek( _fs, _filedes, fp + _nbyte - 1, (int)SEEK_SET );
    _fs->handles[_filedes]->fp = fp + _nbyte;
    
	::LeaveCriticalSection( &g_sWriteFile );

    // return the amount of data that was actually written
    return _nbyte;    
}

// Close the file given by its file descriptor
int getfs_file_close( getfs_Handle* _fs, int _filedes )
{

	::EnterCriticalSection( &g_sCloseFile );
	
    int ret;
    if( _fs == GETFS_NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
		::LeaveCriticalSection( &g_sCloseFile );
        return -1;
    }

	if( _filedes < 0 )
	{
        getfs_errno = GETFS_ERR_NO_OPEN; //syq
		::LeaveCriticalSection( &g_sCloseFile );
        return -1;
	}
    
    // deallocate the file descriptor - this is enough
    ret = deallocate_file_descriptor( _fs, _filedes );
    
    // decrement the number of files open
    if( ret == 0 )
        _fs->fls_open--;
    
    // simulate the behavior of "close()"
    if( ret == 0 )
	{
		::LeaveCriticalSection( &g_sCloseFile );
        return 0;
	}
    
	::LeaveCriticalSection( &g_sCloseFile );
    return -1;
}

// Destroy a hard link or remove a file 
int getfs_file_unlink( getfs_Handle* _fs, const char* _path )
{
    int dt_idx, nt_idx, chain;
    int must_go, ret;
    
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return -1;
    }
    
    // look for the file name in the DT, error if not found 
    dt_idx = getfs_dt_filename_lookup( _fs->dt, (char*) _path );
    if( dt_idx < 0 )
	{
        getfs_errno = GETFS_ERR_NOT_FOUND;
        return -1;
    }
    
    // find index in NT 
    nt_idx = getfs_dt_filename_get_nt_index( _fs->dt, dt_idx );
    if( nt_idx < 0 )
	{
        getfs_errno = GETFS_ERR_INVALID_NT;
        return -1;
    }
    
    // get the chain 
    chain = getfs_nt_node_get_chain( _fs->nt, nt_idx );
    if( chain <= 0 ) // node #0 is reserved 
        return -1;
    
    // decrement refcount by 1, record result in 'must_go' 
    must_go = getfs_nt_refcount_decr( _fs->nt, nt_idx );
    
    // the filename goes away 
    ret = getfs_dt_filename_delete( _fs->dt, (char*)_path );
    if( ret < 0 )
        return ret;
    
    // if 'must_go' is true, the FAT chain has to be erased, too
    if( must_go )
        ret = getfs_fat_destroy_chain( _fs->fat, chain );
    
    return ret;
}

int getfs_file_set_size( getfs_Handle* _fs, const char* _path, int _original_size, int _zip_size )
{
	return getfs_dt_set_size_info( _fs->dt, (char*)_path, _original_size, _zip_size );
}

int getfs_file_get_size( getfs_Handle* _fs, const char* _path, int& _original_size, int& _zip_size )
{
	return getfs_dt_get_size_info( _fs->dt, (char*)_path,  _original_size, _zip_size );
}

// Close the given file system, remove (physically) if 'remove_fs' is true (1)
void getfs_end( getfs_Handle* _fs, int _remove_fs )
{
    int i;
    if( _fs == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return;
    }
    
    if( _remove_fs )
	{
        // the file system has to be closed and removed physically
        getfs_dt_destroy( _fs->dt );
        getfs_nt_destroy( _fs->nt );
        getfs_fat_close( _fs->fat );
        getfs_data_destroy( _fs->data );
        getfs_iio_destroy( _fs->file );
    } 
	else 
	{
        // just close it
        getfs_dt_close( _fs->dt );
        getfs_nt_close( _fs->nt );
        getfs_fat_close( _fs->fat );
        getfs_data_close( _fs->data );
        getfs_iio_close( _fs->file );
    }
    
    // deallocate all file descriptors
    if( _fs->handles != NULL )
	{
        for( i = 0; i < _fs->ht_size; i++ )
            deallocate_file_descriptor( _fs, i );
        free( _fs->handles );
    }
    
    // leaves the pool of processes accessing the lock simultaneously
    lock_leave( _fs->filename );
    
    if( _fs->filename != NULL )
        free( _fs->filename );

	// free
	if( _fs->getfs_buffer != NULL )
		free( _fs->getfs_buffer );
    
    // deallocate the file system handle - invalid from this point on 
    free( _fs );

	::DeleteCriticalSection( &g_sReadFile );
	::DeleteCriticalSection( &g_sWriteFile );
	::DeleteCriticalSection( &g_sFileExist );
	::DeleteCriticalSection( &g_sTrienode_get );
	::DeleteCriticalSection( &g_sOpenFile );
	::DeleteCriticalSection( &g_sCloseFile );
	::DeleteCriticalSection( &g_sSetSize );
	::DeleteCriticalSection( &g_sIIORead );
	


}

// cbk for quicksort
int __getfs_glob_sort( const void* e1, const void* e2 ) 
{
    return strcmp( *(char**)e1, *(char**)e2 );
}

// Search for file names matching the given pattern
int getfs_glob( getfs_Handle* _fs,
             const char* _pattern,
             int _flags,
             int (*errfunc)(const char*, int),
             getfs_glob_t* _pglob )
{ 
    getfs_tmp tmp;
    int      lastc;
    
    if( _fs == GETFS_NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_FS;
        return -1;
    }
    
    if( ( _pattern == NULL ) || ( _pglob == NULL ) ) 
	{
        getfs_errno = GETFS_ERR_INVALID_PARMS;
        return -1;
    }
    
    // init temp structure 
    tmp.pglob   = _pglob;
    tmp.pattern = (char*)_pattern;
    tmp.flags   = _flags;
    tmp.e       = errfunc;
    
    // if GLOB_APPEND is not specified, reinitialize glob structure 
    if( !( _flags & GLOB_APPEND ) ) 
	{
        _pglob->gl_pathv = NULL;
        _pglob->gl_pathc = 0;
    }
    
    // use offset? 
    if( !( _flags & GLOB_DOOFFS ) )
        _pglob->gl_offs  = 0;
    
    // save counter
    lastc = _pglob->gl_pathc;
    
    // do it
	int flags = __FNM_FLAGS;
	if( _flags & GLOB_ONLYCURFOLDERFILE )
		flags |= FNM_ONLYCURFOLDERFILE;
    getfs_dt_filename_glob( _fs->dt, (char*)_pattern, flags, __getfs_glob_cbk, &tmp );
    
    // GLOB_NOCHECK? then return the pattern if no match
    if( ( _flags & GLOB_NOCHECK) && ( _pglob->gl_pathc == lastc ) )
        __getfs_glob_cbk( _fs->dt, (char*)_pattern, 0, &tmp );
    
    // sort if necessary
    if( !( _flags & GLOB_NOSORT ) ) 
	{
        if( _pglob->gl_pathc > 0 )
            qsort( &( _pglob->gl_pathv[_pglob->gl_offs] ), _pglob->gl_pathc, sizeof(char*), __getfs_glob_sort );
    }
    
    return 0;
}

// Frees the memory allocated by a previous getfs_glob
void getfs_glob_free( getfs_Handle*, getfs_glob_t* _pglob )
{
    int i;
    
    if( _pglob == NULL )
	{
        getfs_errno = GETFS_ERR_INVALID_PARMS;
        return;
    }
    
    // free the space allocated to the filenames
    if( _pglob->gl_pathv != NULL )
	{
        for( i = _pglob->gl_offs; i < ( _pglob->gl_offs + _pglob->gl_pathc ); i++ )
		{
            if( _pglob->gl_pathv[i] != NULL )
                free( _pglob->gl_pathv[i] );
        }
        free( _pglob->gl_pathv );
    }
    
    _pglob->gl_pathc = 0;
    _pglob->gl_pathv = NULL;
}

// get filesystem version
PACKVER getfs_get_version( getfs_Handle* _fs )
{
	PACKVER ver;
	ver.main = _fs->file->ver_main;
	ver.minor = _fs->file->ver_minor;
	ver.patch = _fs->file->ver_patch;
	return ver;
}

// set filesystem version
void getfs_set_version( getfs_Handle* _fs, PACKVER& _ver )
{
	_fs->file->ver_main = _ver.main;
	_fs->file->ver_minor = _ver.minor;
	_fs->file->ver_patch = _ver.patch;
}

PACKVER getfs_get_from_version( getfs_Handle* _fs )
{
	PACKVER ver;
	ver.main = _fs->file->ver_from_main;
	ver.minor = _fs->file->ver_from_minor;
	ver.patch = _fs->file->ver_from_patch;
	return ver;
}

void getfs_set_from_version( getfs_Handle* _fs, PACKVER& _ver )
{
	_fs->file->ver_from_main = _ver.main;
	_fs->file->ver_from_minor = _ver.minor;
	_fs->file->ver_from_patch = _ver.patch;
}

void getfs_compress( unsigned char* _dest, unsigned long* _destLen, const unsigned char* _source, unsigned long _sourceLen )
{
	compress( _dest, _destLen, _source, _sourceLen );
}

int getfs_uncompress( unsigned char* _dest, unsigned long* _destLen, const unsigned char* _source, unsigned long _sourceLen )
{
	return uncompress( _dest, _destLen, _source, _sourceLen );
}

short int get_checkSum( unsigned char* temp, int Length )
{   
	unsigned   long   t;   
	unsigned   char   x;   
	int   i,j;   
	short int checksum = 0;

	*(((unsigned   char   *)&t)+1)=*temp;   
	*(((unsigned   char   *)&t)+2)=*(temp+1);   
	for(j=2;j<Length;j++){   
		t=t&0x00ffff00L;   
		*(((unsigned   char   *)&t)+3)=*(temp+j);   
		for(i=0;i<8;i++){   
			t=t>>1;   
			x=(unsigned   char)((t&0x00000080L)>>7);   
			if(x==1)   
				t=t^0x00A00100L;   
		}   
	}   

	checksum |= ((unsigned char)((t&0x0000ff00)>>8) <<8);
	checksum |= (unsigned char)((t&0x00ff0000L)>>16);
	
	//*(temp+Length)=(unsigned   char)((t&0x0000ff00)>>8);   
	//*(temp+Length+1)=(unsigned   char)((t&0x00ff0000L)>>16);  
	
	return checksum;
}

void WriteCheckSum( unsigned char* temp, short int checksum )
{
	unsigned char c1 = checksum >> 8;
	unsigned char c2 = checksum & 0xff;
	temp[0] = c1;
	temp[1] = c2;
}

short int SwapByte( short int _checksum )
{
	return ((_checksum >> 8)&0xff) | ( ( _checksum & 0xff ) << 8 );
}

//字节到十六进制的ASCII码转换
static std::string byteHEX( unsigned char byte0 )
{
	char ac[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
			'A', 'B', 'C', 'D', 'E', 'F'
	};
	char ac1[3];
	ac1[0] = ac[(byte0 >> 4) & 0xf];
	ac1[1] = ac[byte0 & 0xf];
	ac1[2] = 0;

	return std::string(ac1);
}

const char* getMd5Str( const char* _s )
{
	static MD5_CTX m_md5;
	static std::string hashstr;
	hashstr.assign("");
	m_md5.MD5Update( (unsigned char*)_s, strlen(_s) );
	unsigned char digest[16] = {0};
	m_md5.MD5Final( digest );
	for( int i=0;i<16;i++ )		{
		hashstr += byteHEX( digest[i] );
	}
	hashstr += "._";
	return hashstr.c_str();
}