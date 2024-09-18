#include "getfs_data.h"
#include <stdlib.h>
#include <string.h>

int getfs_data_IOMODE = 1;


// create cache
static int cache_create( getfs_data_Data* _data )
{    
    int size = GETFS_DATA_DEF_CACHE_SIZE;
    if( _data == NULL )
        return -1;
    
	// avoid arbitrarily sized cache - round up to the nearest
    // multiple of GETFS_DATA_BLOCK_SIZE
    if( ( size % GETFS_DATA_BLOCK_SIZE) != 0 )
        size += ( GETFS_DATA_BLOCK_SIZE - ( size % GETFS_DATA_BLOCK_SIZE ) );
    
    // allocate
    _data->cache = (getfs_data_Cache*)malloc( sizeof(getfs_data_Cache) );
    if( _data->cache == NULL )
        return -1; // not enough memory
    _data->cache->buffer = (char*)malloc( size );
    if( _data->cache->buffer == NULL )
        return -1; // not enough memory
    
    // initialize
    _data->cache->size     = size;
    _data->cache->sync     = GETFS_DATA_SYNC;
    _data->cache->position = 0;
    
    // read a block of size 'size'
    fseek( _data->file, _data->cache->position, 0 );
    fread( _data->cache->buffer, 1, _data->cache->size, _data->file );
    
    return 0;
}

// return nonzero (true) if a block is currently in cache
static int is_in_cache( getfs_data_Data* _data, int _block ) 
{
    if( ( _data == NULL) || ( _data->cache == NULL ) )
        return 0;
    if( ( _block >= ( _data->cache->position / GETFS_DATA_BLOCK_SIZE ) ) &&
        ( _block <  ( ( _data->cache->position + _data->cache->size ) / GETFS_DATA_BLOCK_SIZE ) ) )
        return 1; // yes!
    return 0; // gotta refresh... :( 
}

// flush the cache
static int cache_flush( getfs_data_Data* _data )
{
    // tests
    if( _data->cache == NULL )
        return -1; // no cache
    if( _data->cache->sync == GETFS_DATA_SYNC )
        return 0;  // flush unnecessary
    
    // data->cache->sync is 0, which means that a process
    // wrote some stuff in the cache and the cache hasn't
    // been flushed
    fseek( _data->file, _data->cache->position, 0 );
    fwrite( _data->cache->buffer, 1, _data->cache->size, _data->file );
    
    // mark the cache as being in sync
    _data->cache->sync = GETFS_DATA_SYNC;
    
    return 0;
}

// slides the cache window
static int cache_slide( getfs_data_Data* _data, int _new_position )
{
    // is the cache is out of sync, flush it first
    if( _data->cache->sync != GETFS_DATA_SYNC )
        cache_flush( _data );
    
    // adjust cache positions that are not block-aligned
    if( ( _new_position % GETFS_DATA_BLOCK_SIZE ) != 0 )
        _new_position = (int)( _new_position / GETFS_DATA_BLOCK_SIZE ) * GETFS_DATA_BLOCK_SIZE;
    
    // update cache based on the new absolute position
    _data->cache->position = _new_position;
    fseek( _data->file, _data->cache->position, 0 );
    fread( _data->cache->buffer, 1, _data->cache->size, _data->file );
    
    return 0;
    
}

// write 'how_much' bytes from 'from' to the cached disk, at absolute position 'abspos'
static int cache_put( getfs_data_Data* _data, int _abspos, int _how_much, void* _from )
{
    int i;
    
    for( i = 0; i < _how_much; i += GETFS_DATA_BLOCK_SIZE )
	{
        // refresh cache only if necessary
        if( !is_in_cache( _data, ( ( _abspos + i) / GETFS_DATA_BLOCK_SIZE ) ) )
            cache_slide( _data, (int)( ( _abspos + i ) / GETFS_DATA_BLOCK_SIZE) * GETFS_DATA_BLOCK_SIZE );

        // get one block of data from cache
        memcpy( (void*)((char*)_data->cache->buffer + ( _abspos - _data->cache->position + i)), (char*)_from + i, GETFS_DATA_BLOCK_SIZE );

        // something has been dumped into the cache, so set the unsync flag
        _data->cache->sync = GETFS_DATA_NOT_SYNC; // needs flushing
    }
    
    return 0;
}

// get 'how_much' bytes from the cached disk, write bytes to address referenced by 'to'
static int cache_get( getfs_data_Data* _data, int _abspos, int _how_much, void* _to )
{
    int i;
    for( i=0; i<_how_much; i+=GETFS_DATA_BLOCK_SIZE )
	{
        // refresh cache only if necessary
        if( !is_in_cache( _data, ( ( _abspos + i ) / GETFS_DATA_BLOCK_SIZE ) ) )
            cache_slide( _data, (int)( ( _abspos + i ) / GETFS_DATA_BLOCK_SIZE ) * GETFS_DATA_BLOCK_SIZE );
        // get one block of data from cache
        memcpy( (char*)_to + i,
            (void*)((char*)_data->cache->buffer + ( _abspos - _data->cache->position + i ) ),
            GETFS_DATA_BLOCK_SIZE );
    }
    return 0;
}

// destroys the cache
static int cache_destroy( getfs_data_Data* _data ) 
{
    // flush the cache
    cache_flush( _data );
    
    // deallocate it
    if( _data->cache->buffer != NULL )
        free( _data->cache->buffer );
    if( _data->cache != NULL )
        free( _data->cache );
    _data->cache = NULL; // mark it
    
    return 0;
}
// Open an existing virtual disk
getfs_data_Data* getfs_data_open( const char* _filename )
{
    getfs_data_Data*  data;
    
    // allocate a new data handle corresponding to the virtual disk
    data = (getfs_data_Data*)malloc( sizeof(getfs_data_Data) );
    if( data == NULL )
        return NULL; // not enough memory
    
    // open the file
    data->filename = (char*)malloc( strlen(_filename) + 8 );
    if( !data->filename )
        return NULL;
    strcpy( data->filename, _filename );
    if( getfs_data_IOMODE & 2 ) // write
        data->file = fopen( _filename, "r+b" );
    else
        data->file = fopen( _filename, "rb" );
    
    if( data->file == NULL )
        return NULL; // could not open file for R/W
    
    // create the cache. data->file must be initialized.
    if( cache_create( data ) < 0 )
        return NULL; // could not create cache
    
    return data;
}


// Create a new virtual disk
getfs_data_Data* getfs_data_create( const char* _filename )
{
    getfs_data_Data*  data;
    
    // allocate a new data handle corresponding to the virtual disk
    data = (getfs_data_Data*)malloc( sizeof(getfs_data_Data) );
    if( data == NULL )
        return NULL;
    
    // create the file
    data->filename = (char*)malloc( strlen(_filename) + 8 );
    if( !data->filename )
        return NULL;
    strcpy( data->filename, _filename );
    data->file = fopen( _filename, "w+b" );
    if( data->file == NULL )
        return NULL;
    
    // create the cache. data->file must be initialized.
    if( cache_create(data) < 0 )
        return NULL;
    
    return data;
}

// Read one block of data from block number 'block_no' into the memory referenced by 'to'
int getfs_data_read( getfs_data_Data* _data, int _block_no, void* _to )
{
    if( _block_no < 0 )
        return -1;
    return cache_get( _data, _block_no * GETFS_DATA_BLOCK_SIZE, GETFS_DATA_BLOCK_SIZE, _to );
}

// Write one block of data from 'from' to absolute block number 'block_no'
int getfs_data_write( getfs_data_Data* _data, int _block_no, void* _from )
{
    if( _block_no < 0 )
        return -1;
    return cache_put( _data, _block_no * GETFS_DATA_BLOCK_SIZE, GETFS_DATA_BLOCK_SIZE, _from );
}

// Close (but do not destroy) the given virtual disk
void getfs_data_close( getfs_data_Data* _data )
{
	if( !_data )
		return;

    // close the cache
    cache_destroy( _data );
    
    // close the file
    fflush( _data->file );
    fclose( _data->file );
    
	if( _data->filename )
		free( _data->filename );

    // destroy the handle 
    free( _data );
}

// Close and destroy (remove) the given virtual disk
void getfs_data_destroy( getfs_data_Data* _data )
{
    char* tempfn = (char*)malloc( strlen( _data->filename) + 8 );
    if( tempfn )
        strcpy( tempfn, _data->filename );
    // close and destroy 
    getfs_data_close( _data );
    // remove file
#ifdef WIN32
    if( tempfn )
        remove( tempfn );
#else
    if( tempfn )
        unlink( tempfn );
#endif
    if( tempfn )
        free( tempfn );
}

