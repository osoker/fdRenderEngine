//Interlaced I/O module (IIO)
#include "getfs_iio.h"
#include <windows.h> 
#include "getfs_debug.h"
#include <io.h>
#include <assert.h>
#include "getfs.h"

int getfs_iio_IOMODE = 1;
int getfs_iio_BLOCK_SIZEv;

#if (getfs_iio_CC_POLICY == ___LRU___)
int getfs_iio_CLOCK = 0;
#endif


#define getfs_iio_MAGIC ('S' | ('Y' << 8) | ('Q' << 16) | ('0' << 24))

typedef struct {
    int magic;
    int channels;
	
	int ver_main;
	int ver_minor;
	int ver_patch;

	int ver_from_main;
	int ver_from_minor;
	int ver_from_patch;

} getfs_iio_File_Header;

typedef struct {
    int blocks;
    int size;
} getfs_iio_Channel_Header;


// expands the cache and makes it encapsulate the given page
static int cache_expand( getfs_iio_File* _file, int _channel, int _page )
{
    int oldpgs, i;
    int pgs;
    
    if( _file->chn[_channel]->cache != NULL )
        pgs = _file->chn[_channel]->cache->nr_pages;
    else
        pgs = 0;
    oldpgs = pgs;
    // double until page is in cache
    if( pgs == 0 ) pgs++;
    while( (pgs <= _page) || (pgs < getfs_iio_CC_DEF_TABSIZE) ) pgs <<= 1;
    // create cache if nonexistent
    if( _file->chn[_channel]->cache == NULL )
	{
        _file->chn[_channel]->cache = (getfs_iio_Cache*)malloc( sizeof(getfs_iio_Cache) );
        _file->chn[_channel]->cache->pages = NULL;
        _file->chn[_channel]->cache->nr_pages = 0;
        _file->chn[_channel]->cache->active_pages = 0;
    }
    // update the page table
    _file->chn[_channel]->cache->nr_pages = pgs;
    _file->chn[_channel]->cache->pages = (getfs_iio_Cache_Page**)realloc( 
		                      _file->chn[_channel]->cache->pages,
                              pgs * sizeof(getfs_iio_Cache_Page*) 
							  );
    for( i=oldpgs; i<pgs; i++ )
        _file->chn[_channel]->cache->pages[i] = NULL;
    
    return pgs;
}

static void cache_page_set_sync( getfs_iio_Cache_Page* _c, int _s ) 
{
    if( _c == NULL )
        return;
    _c->buffer = (char*)( (int)_c->buffer & getfs_iio_CC_BUF_MASK );
    _c->buffer = (char*)( (int)_c->buffer | ( _s & getfs_iio_CC_SYNC_MASK ) );
}

// chooses the best page to reuse, avoiding the specified page
static int cache_page_choose_best_to_reuse( getfs_iio_File* _file, int _channel, int _page )
{
    int i, k;
    int rpage = 0;
    
#if ( getfs_iio_CC_POLICY == ___LRU___ )
    
    i = 0x7fffffff;
    for( k=0; k<_file->chn[_channel]->cache->nr_pages; k++ )
	{
        if( ( _file->chn[_channel]->cache->pages[k] != NULL ) &&
            ( _file->chn[_channel]->cache->pages[k]->timestamp < i ) &&
            ( k != _page ) )
		{
            i = _file->chn[_channel]->cache->pages[k]->timestamp;
            rpage = k;
        }
	}
        
#endif
        
#if ( getfs_iio_CC_POLICY == ___LFU___ )
       
        i = 0x7fffffff;
        for( k=0; k<_file->chn[_channel]->cache->nr_pages; k++ )
		{
            if( ( _file->chn[_channel]->cache->pages[k] != NULL ) &&
                ( _file->chn[_channel]->cache->pages[k]->hits < i ) &&
                ( k != _page ) )
			{
                i = _file->chn[_channel]->cache->pages[k]->hits;
                rpage = k;
            }
		}
            
#endif
            
#if ( getfs_iio_CC_POLICY == ___NLN___ )
	/* Policy: Next Lower Neighbor */
         
    rpage = _page - 1;
    while( _file->chn[_channel]->cache->pages[rpage] == NULL )
	{
		if( rpage == 0 )
			rpage = _file->chn[_channel]->cache->nr_pages - 1;
        else
            rpage--;
	}
             
#endif

	return rpage;
}

static int cache_page_get_sync( getfs_iio_Cache_Page* _c )
{
    if( _c == NULL )
        return 0;
    return (int)_c->buffer & getfs_iio_CC_SYNC_MASK;
}

int getfs_iio_blocks_per_chunk( getfs_iio_File* _file ) 
{
    int i;
    int blocks = 0;
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    getfs_iio_CLOCK++;
#endif
    
    if( _file == NULL )
        return -1;

    // count the number of blocks for all channels
    for( i=0; i<_file->channels; i++ )
        blocks += _file->chn[i]->blocks;
    
	// return the number of blocks per chunk
    return blocks;
}

static int channel_block_to_absolute_block( getfs_iio_File* _file, int _channel, int _cb )
{
    int chunk_no;
    int block_within_chunk;
    int bpc;
    int i;
    
    if( _file == NULL )
        return -1;
    
    // chunk number ( where the given block resides )
    chunk_no = _cb / _file->chn[_channel]->blocks;
    
    // compute the block index within the chunk
    block_within_chunk = _cb % _file->chn[_channel]->blocks;
    for( i = 0; i < _channel; i++ )
        block_within_chunk += _file->chn[i]->blocks;
    
    // how many blocks per chunk?
    bpc = getfs_iio_blocks_per_chunk( _file );
    
    // compute and return absolute block
    return ( chunk_no * bpc + block_within_chunk );
}

static int channel_pos_to_absolute_block( getfs_iio_File* _file, int _channel, int _pos )
{
    int cb;
    
    // block within channel
    cb = _pos / getfs_iio_BLOCK_SIZEv;
    
    // convert block-within-channel to absolute-block
    return channel_block_to_absolute_block( _file, _channel, cb );
}

static int header_size( getfs_iio_File* _file ) 
{
    int k = sizeof(getfs_iio_File_Header) + _file->channels * sizeof(getfs_iio_Channel_Header);
    if( k > getfs_iio_BLOCK_SIZEv ) 
	{
        debug_out( "internal error: iio overflow. Too many iio channels." );
        exit(-1);
    }
    return getfs_iio_BLOCK_SIZEv;
}

// reads N absolute blocks starting at nr. 'start'    
static int read_absolute_block_n( getfs_iio_File* _file, int _start, int _N, void* _buf ) 
{
    int read_ = 0;
    
#ifdef INSTRUMENT_SYSTEM
    abs_block_reads++;
#endif
    
    if( _file->file == NULL )
        return -1; // handle not valid
    
    if( _buf == NULL )
        return -1; // invalid address
    
    // go to that position and read the block
    fseek( _file->file, header_size(_file) + (_start * getfs_iio_BLOCK_SIZEv), 0 );
    read_ = fread( _buf, 1, getfs_iio_BLOCK_SIZEv * _N, _file->file );
    
    // if an incomplete block was read in, complete it with additional zeroes
    if( read_ < getfs_iio_BLOCK_SIZEv * _N )
        memset( (char*)_buf + read_, 0, getfs_iio_BLOCK_SIZEv * _N - read_ );
    
    return getfs_iio_BLOCK_SIZEv * _N;
}

// writes N contiguous absolute blocks starting at 'start'
static int write_absolute_block_n( getfs_iio_File* _file, int _start, int _N, void* _buf )
{
#ifdef INSTRUMENT_SYSTEM
    abs_block_writes++;
#endif
    
    if( _file->file == NULL )
        return -1; // handle not valid
    
    if( _buf == NULL )
        return -1; // invalid address
    
    // go to that position and write the block
    fseek( _file->file, header_size(_file) + (_start * getfs_iio_BLOCK_SIZEv), 0 );
    fwrite( _buf, 1, getfs_iio_BLOCK_SIZEv * _N, _file->file );
    
    return getfs_iio_BLOCK_SIZEv * _N;
}

static char* cache_page_get_buffer( getfs_iio_Cache_Page* _c )
{
    if( _c == NULL )
        return NULL;
    return (char*)( (int)_c->buffer & getfs_iio_CC_BUF_MASK );
}

// dumps the given cache page immediately in the given channel
static int cache_page_dump( getfs_iio_File* _file, int _channel, int _page )
{
    int pos, ablock;
    
#ifdef INSTRUMENT_SYSTEM
    cache_flushes++;
#endif
    
    // invalid page?
    if( _file->chn[_channel]->cache->pages[_page] == NULL )
        return 0;
    
    pos = _file->chn[_channel]->cache->pages[_page]->position;
    ablock = channel_pos_to_absolute_block( _file, _channel, pos );
    write_absolute_block_n( _file,
        ablock,
        _file->chn[_channel]->blocks,
        cache_page_get_buffer( _file->chn[_channel]->cache->pages[_page] ) );
    
    // cache page is in sync
    cache_page_set_sync( _file->chn[_channel]->cache->pages[_page], 1 );
    
    return 0;
}

// flushes the given cache page
static void cache_page_flush( getfs_iio_File* _file, int _channel, int _i )
{
    // flush if out-of-sync
    if( cache_page_get_sync( _file->chn[_channel]->cache->pages[_i] ) == 0 )
        cache_page_dump( _file, _channel, _i );
}



// creates the given cache page
static void cache_page_create( getfs_iio_File* _file, int _channel, int _page ) 
{
    if( _file->chn[_channel]->cache->pages[_page] == NULL ) 
	{
    // if the number of active pages is not larger than
    // the maximum number allowed, increase it and allocate
    // another page
        
#ifdef getfs_iio_CC_RESTRICT_GROWTH
        if( _file->chn[_channel]->cache->active_pages < getfs_iio_CC_MAX_ACTIVE_PGS )	{
#endif
            
            // increase the number of active pages
            _file->chn[_channel]->cache->active_pages++;
            
            // allocate page entry
            _file->chn[_channel]->cache->pages[_page] = (getfs_iio_Cache_Page*)malloc( sizeof(getfs_iio_Cache_Page) );
            
            // allocate page buffer
            _file->chn[_channel]->cache->pages[_page]->buffer = (char*)malloc( 
				getfs_iio_BLOCK_SIZEv * _file->chn[_channel]->blocks );

            // allocation errors?
            if( _file->chn[_channel]->cache->pages[_page]->buffer == NULL )
			{
                fprintf( stderr, "Internal error: not enough memory for cache page. Aborting...\n" );
                exit(-1);
            }
            
            // initialize the page
            _file->chn[_channel]->cache->pages[_page]->position = 0;
            
#ifndef getfs_iio_CC_MEMSET_OPT
            memset( _file->chn[_channel]->cache->pages[_page]->buffer,
                0,
                getfs_iio_BLOCK_SIZEv * _file->chn[_channel]->blocks );
#endif
            
            // force the sync flag
            cache_page_set_sync( _file->chn[_channel]->cache->pages[_page], 1 );
            
#ifdef getfs_iio_CC_RESTRICT_GROWTH
        } else {
            
            int rpage;
            
            /* The maximum amount of active pages has been reached, so
            an existing page has to be reused. Choose the best page to
            reuse and "rewire" it to this entry */
            
#ifdef INSTRUMENT_SYSTEM
            page_switches++;
#endif
            
            // choose the best one
            rpage = cache_page_choose_best_to_reuse( _file, _channel, _page );
            
#ifdef __DEBUG__
            printf("[debug] page switch: %d's buffer rewired to %d\n", rpage, page);
#endif
            
            // flush the old page if necessary
            cache_page_flush( _file, _channel, rpage );
            
            // rewire it
            _file->chn[_channel]->cache->pages[_page] = _file->chn[_channel]->cache->pages[rpage];
            _file->chn[_channel]->cache->pages[rpage] = NULL;
            
            // reconfigure the page
            _file->chn[_channel]->cache->pages[_page]->position = 0;
#ifndef getfs_iio_CC_MEMSET_OPT
            memset( cache_page_get_buffer( _file->chn[_channel]->cache->pages[_page]),
                0,
                getfs_iio_BLOCK_SIZEv * _file->chn[_channel]->blocks );
#endif
#if (getfs_iio_CC_POLICY == ___LRU___)
            _file->chn[_channel]->cache->pages[_page]->timestamp = getfs_iio_CLOCK;
#endif
            
#if (getfs_iio_CC_POLICY == ___LFU___)
            _file->chn[_channel]->cache->pages[_page]->hits = 0;
#endif
            
            // force the sync flag
            cache_page_set_sync( _file->chn[_channel]->cache->pages[_page], 1 );
        }
#endif /* nfs_iio_CC_RESTRICT_GROWTH */
    }
}

// refreshes the cache page from the given position
static int cache_page_refresh( getfs_iio_File* _file, int _channel, int _pos )
{
    int page, ablock;
    
    // compute the page number, expand cache if necessary
    page = _pos / (getfs_iio_BLOCK_SIZEv * _file->chn[_channel]->blocks );
    if( ( _file->chn[_channel]->cache == NULL) || (page >= _file->chn[_channel]->cache->nr_pages ) )
        cache_expand( _file, _channel, page );
    
    // is the page already allocated? if not, allocate
    if( _file->chn[_channel]->cache->pages[page] == NULL )
        cache_page_create( _file, _channel, page );
    
    // refresh the page
    ablock = channel_pos_to_absolute_block( _file, _channel, _pos );
    read_absolute_block_n( _file,
                           ablock,
                           _file->chn[_channel]->blocks,
                           cache_page_get_buffer( _file->chn[_channel]->cache->pages[page] ) );
    
    // cache page is in sync
    cache_page_set_sync( _file->chn[_channel]->cache->pages[page], 1 );
    _file->chn[_channel]->cache->pages[page]->position = _pos;
    
    return 0;
}

// creates a channel cache
static int cache_create( getfs_iio_File* _file, int _channel )
{
    // errors?
    if( _file == NULL )
        return -1;
    
    // read in the initial data
    cache_page_refresh( _file, _channel, 0 );
    
    // success!
    return 0;
}

static int read_header( getfs_iio_File* _file ) 
{
    static getfs_iio_File_Header    fh;
    static getfs_iio_Channel_Header ch;
    int i, k;
    
    // errors?
    if( _file->file == NULL )
        return -1;
    
    // read the file header, check magic
    fseek( _file->file, 0, 0 );
    fread( &fh, sizeof(fh), 1, _file->file );
    if( fh.magic != getfs_iio_MAGIC )
        return -1; // invalid IIO file
	
	// set version...
	_file->ver_main = fh.ver_main;
	_file->ver_minor = fh.ver_minor;
	_file->ver_patch = fh.ver_patch;

	_file->ver_from_main = fh.ver_from_main;
	_file->ver_from_minor = fh.ver_from_minor;
	_file->ver_from_patch = fh.ver_from_patch;

    // init channels
    _file->channels = fh.channels;
    _file->chn = (getfs_iio_Channel**)malloc( fh.channels * sizeof(getfs_iio_Channel*) );
    
    // reopen the appropriate channels
    for( i=0; i < fh.channels; i++ ) 
	{
        _file->chn[i] = (getfs_iio_Channel*)malloc(sizeof(getfs_iio_Channel) );
        // read channel header
        fseek( _file->file, sizeof(fh) + (i * sizeof(ch)), 0 );
        k = fread( &ch, 1, sizeof(ch), _file->file );
        _file->chn[i]->size   = ch.size;
        _file->chn[i]->blocks = ch.blocks;
        _file->chn[i]->cp     = 0;
        _file->chn[i]->cache  = NULL;
    }
    
    // initialize the cache buffers
    for( i=0; i < fh.channels; i++ )
        cache_create( _file, i );
    
    return 0;
    
}

static int write_header( getfs_iio_File* _file )
{
    getfs_iio_File_Header    fh;
    getfs_iio_Channel_Header ch;
    int i;
    
    // errors?
    if( _file->file == NULL )
        return -1;
    
    // initialize the file header
    fh.magic    = getfs_iio_MAGIC;
    fh.channels = (int)_file->channels;

	fh.ver_main = _file->ver_main;
	fh.ver_minor = _file->ver_minor;
	fh.ver_patch = _file->ver_patch;

	fh.ver_from_main = _file->ver_from_main;
	fh.ver_from_minor = _file->ver_from_minor;
	fh.ver_from_patch = _file->ver_from_patch;
    
    // write file header
    fseek( _file->file, 0, SEEK_SET );
    fwrite( &fh, 1, sizeof(fh), _file->file );
    
    // initialize the channel headers and dump them in the file
    for( i = 0; i < _file->channels; i++ ) 
	{
        // initialize channel header for channel 'i'
        ch.blocks = _file->chn[i]->blocks;
        ch.size   = _file->chn[i]->size;
        // write channel header 
        fwrite( &ch, 1, sizeof(ch), _file->file );
    }
    
    fflush( _file->file );
    
    return 0;
}

// get the size of the vmem page
static int get_page_size() 
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (int)si.dwPageSize;
}

// flushes the entire cache if necessary
static void cache_flush( getfs_iio_File* _file, int _channel )
{
    int i;
    // flush all pages (if necessary) 
    for( i=0; i<_file->chn[_channel]->cache->nr_pages; i++ )
        cache_page_flush( _file, _channel, i );
}

static void flush_data( getfs_iio_File* _file )
{
    int i;
    
    // flush the channel cache buffers
    for( i=0; i<_file->channels; i++ )
        cache_flush( _file, i );
    
    // i/o flush
    if( _file->file != NULL )
        fflush( _file->file );
}

// Auto-truncate the IIO file if necessary
void auto_truncate( getfs_iio_File* _file )
{
    int i, nc, ns;
    int max_chunks  = 0; /* max chunks in a channel */
    int bps         = 0; /* total blocks per slice */
    
    for( i=0; i < (int)_file->channels; i++ )
	{
        nc = 1 + ( ( 1 + ( _file->chn[i]->size / getfs_iio_BLOCK_SIZEv ) ) / _file->chn[i]->blocks );
        if( nc > max_chunks )
		{
            max_chunks = nc;
        }
        bps += _file->chn[i]->blocks;
    }
    
    if( max_chunks > 0 )
	{
        ns = max_chunks * bps;
        /* ns is now the number of blocks actually used */
        fseek( _file->file, header_size(_file) + ns * getfs_iio_BLOCK_SIZEv, 0 );
#if defined(WIN32) && !defined(CYGWIN) && !defined(CYGNUS)
        chsize( fileno( _file->file), header_size(_file) + ns * getfs_iio_BLOCK_SIZEv );
#else
        ftruncate( fileno( _file->file), header_size(_file) + ns * getfs_iio_BLOCK_SIZEv );
#endif
    }
}

getfs_iio_File* getfs_iio_create( const char* _filename ) 
{
    getfs_iio_File* handle;
    
#ifdef INSTRUMENT_SYSTEM
    instrument_init();
#endif
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    getfs_iio_CLOCK++;
#endif
    
    // first, figure out what the vm page size is, and set the block
    // size equal to the page size 
    getfs_iio_BLOCK_SIZEv = get_page_size();
    if( getfs_iio_BLOCK_SIZEv < getfs_iio_BLOCK_SIZE )
        getfs_iio_BLOCK_SIZEv = getfs_iio_BLOCK_SIZE;
    
    // create a new file handle and initialize it
    handle = (getfs_iio_File*)malloc(sizeof(getfs_iio_File));
    if( handle == NULL )
        return NULL; // not enough memory
    handle->channels = 0;
    handle->chn      = NULL;
    handle->filename = (char*)malloc(strlen(_filename) + 8);
    if( handle->filename == NULL )
        return NULL; // out of memory
    strcpy( handle->filename, _filename );
    handle->file     = fopen( _filename, "w+b" );
    if( handle->file == NULL )
        return NULL; // cannot create file - permission problem
    
	handle->ver_main = 0;
	handle->ver_minor = 0;
	handle->ver_patch = 0;

	handle->ver_from_main = 0;
	handle->ver_from_minor = 0;
	handle->ver_from_patch = 0;

    // write the header 
    write_header( handle );
    
    return handle;
}

getfs_iio_File* getfs_iio_open( const char* _filename )
{
	getfs_iio_File* handle;

#ifdef INSTRUMENT_SYSTEM
	//instrument_init();
#endif

#if (getfs_iio_CC_POLICY == ___LRU___)
	getfs_iio_CLOCK++;
#endif

	// first, figure out what the vm page size is, and set the block
	// size equal to the page size
	getfs_iio_BLOCK_SIZEv = get_page_size();
	if( getfs_iio_BLOCK_SIZEv < getfs_iio_BLOCK_SIZE )
		getfs_iio_BLOCK_SIZEv = getfs_iio_BLOCK_SIZE;

	// create a new file handle and initialize it
	handle = (getfs_iio_File*)malloc( sizeof(getfs_iio_File) );
	if( handle == NULL )
		return NULL; // not enough memory
	handle->channels = 0;
	handle->chn      = NULL;

	handle->filename = (char*)malloc( strlen(_filename) + 8 );
	if( handle->filename == NULL )
		return NULL; // out of memory
	strcpy( handle->filename, _filename );

	if( getfs_iio_IOMODE & 2 ) // write
		handle->file     = fopen( _filename, "r+b" );
	else
		handle->file     = fopen( _filename, "rb" );

	if( handle->file == NULL )
		return NULL; // file not found

	// read the file header
	if( read_header( handle ) < 0 )
		return NULL;



	return handle;
}

static int cache_destroy( getfs_iio_Cache* _c )
{
    int i;
    
    if( _c == NULL )
        return -1;
    
    if( _c->pages == NULL )
        return -1;
    
    // free pages
    for( i=0; i<_c->nr_pages; i++ )
	{
        if( _c->pages[i] != NULL )
		{
            free( (char*)( (int)_c->pages[i]->buffer & ~1 ) );
            free( _c->pages[i] );
        }
	}
        
    // free entry table
    free( _c->pages );
        
    // free cache structure
    free( _c );
        
    return 0;
}

// returns non-zero (true) if the given channe block is in the cache
static int is_in_cache( getfs_iio_File* _file, int _channel, int _block ) 
{
    int page = _block / _file->chn[_channel]->blocks;
    return ( _file->chn[_channel]->cache->pages[page] != NULL );
}

// updates the channel cache to include the given position
static void cache_update( getfs_iio_File* _file, int _channel, int _pos )
{
    int page, pgsize;
    
    /* First, the position has to be a multiple of cache page size, otherwise
    the hell breaks loose from this point on */
    pgsize = getfs_iio_BLOCK_SIZEv * _file->chn[_channel]->blocks;
    if( ( _pos % pgsize ) != 0 )
        _pos -= ( _pos % pgsize );
    
    // compute the page number, expand cache if necessary 
    page = _pos / pgsize;
    if( ( _file->chn[_channel]->cache == NULL ) ||
        ( page >= _file->chn[_channel]->cache->nr_pages ) )
        cache_expand( _file, _channel, page );
    
    // is the page already allocated? if not, allocate 
    if( _file->chn[_channel]->cache->pages[page] == NULL )
        cache_page_create( _file, _channel, page );
    
    // if out-of-sync, dump
    cache_page_flush( _file, _channel, page ); // necessary? 
    
    // refresh the page
    cache_page_refresh( _file, _channel, _pos );
}

// writes a partial channel block (cached)
static int cache_write_partial_channel_block( getfs_iio_File* _file, int _channel, int _block_no, int _start, int _end, void* _block )
{
    int page;
    
#ifdef INSTRUMENT_SYSTEM
    partial_writes++;
#endif
    
    // update the cache if necessary
    if( !is_in_cache( _file, _channel, _block_no ) )
#ifdef INSTRUMENT_SYSTEM
    {
        cache_misses++;
#endif
        cache_update( _file, _channel, _block_no * getfs_iio_BLOCK_SIZEv );
#ifdef INSTRUMENT_SYSTEM
    }
    else
        cache_hits++;
#endif
    
    // compute the page where this block resides
    page = _block_no / _file->chn[_channel]->blocks;
    
    // copy partial block to cache
    memcpy( cache_page_get_buffer( _file->chn[_channel]->cache->pages[page] ) +
        ( _block_no * getfs_iio_BLOCK_SIZEv - _file->chn[_channel]->cache->pages[page]->position ) +
        _start,
        _block,
        ( _end - _start + 1 ) );
    
    // cache page is out-of-sync!
    cache_page_set_sync( _file->chn[_channel]->cache->pages[page], 0 );
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    _file->chn[_channel]->cache->pages[page]->timestamp = getfs_iio_CLOCK;
#endif
    
#if (getfs_iio_CC_POLICY == ___LFU___)
    _file->chn[_channel]->cache->pages[page]->hits++;
#endif
    
    // return the number of bytes written
    return ( _end - _start + 1 );
}

// writes an entire channel block (cached)
static int cache_write_channel_block( getfs_iio_File* _file, int _channel, int _block_no, void* _block )
{
    int page;
    
#ifdef INSTRUMENT_SYSTEM
    full_writes++;
#endif
    
    // update the cache if necessary
    if( !is_in_cache( _file, _channel, _block_no ) )
#ifdef INSTRUMENT_SYSTEM
    {
        cache_misses++;
#endif
        cache_update( _file, _channel, _block_no * getfs_iio_BLOCK_SIZEv );
#ifdef INSTRUMENT_SYSTEM
    }
    else
        cache_hits++;
#endif
    
    // compute the page where this block resides
    page = _block_no / _file->chn[_channel]->blocks;
    
    // copy block to cache 
    memcpy( cache_page_get_buffer( _file->chn[_channel]->cache->pages[page] ) +
            ( _block_no * getfs_iio_BLOCK_SIZEv - _file->chn[_channel]->cache->pages[page]->position ),
            _block,
            getfs_iio_BLOCK_SIZEv );
    
    // cache page is out-of-sync 
    cache_page_set_sync( _file->chn[_channel]->cache->pages[page], 0 );
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    _file->chn[_channel]->cache->pages[page]->timestamp = getfs_iio_CLOCK;
#endif
    
#if (getfs_iio_CC_POLICY == ___LFU___)
    _file->chn[_channel]->cache->pages[page]->hits++;
#endif
    
    // return the number of bytes written
    return getfs_iio_BLOCK_SIZEv;
}

// reads a partial channel block (cached)
static int cache_read_partial_channel_block( getfs_iio_File* _file, int _channel, int _block_no, int _start, int _end, void* _block )
{
    int page;
    
#ifdef INSTRUMENT_SYSTEM
    partial_reads++;
#endif
    
    // update the cache if necessary
    if( !is_in_cache( _file, _channel, _block_no ) )
#ifdef INSTRUMENT_SYSTEM
    {
        cache_misses++;
#endif
        cache_update( _file, _channel, _block_no * getfs_iio_BLOCK_SIZEv );
#ifdef INSTRUMENT_SYSTEM
    }
    else
        cache_hits++;
#endif
    
    // compute the page where this block resides
    page = _block_no / _file->chn[_channel]->blocks;
    
	if( page != 0 )
	{
		static int aa = 0;
		aa++;
	}

    // copy partial block from cache
    memcpy( _block,
            cache_page_get_buffer( _file->chn[_channel]->cache->pages[page]) +
            ( _block_no * getfs_iio_BLOCK_SIZEv - _file->chn[_channel]->cache->pages[page]->position ) +
            _start,
            ( _end - _start + 1 ) );
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    _file->chn[_channel]->cache->pages[page]->timestamp = getfs_iio_CLOCK;
#endif
    
#if (getfs_iio_CC_POLICY == ___LFU___)
    _file->chn[_channel]->cache->pages[page]->hits++;
#endif
    
    // return the number of bytes read
    return ( _end - _start + 1 );    
}

// reads an entire channel block (cached)
static int cache_read_channel_block( getfs_iio_File* _file, int _channel, int _block_no, void* _block )
{
    int page;
    
#ifdef INSTRUMENT_SYSTEM
    full_reads++;
#endif
    
    // update the cache if necessary
    if( !is_in_cache( _file, _channel, _block_no ) )
#ifdef INSTRUMENT_SYSTEM
    {
        cache_misses++;
#endif
        cache_update( _file, _channel, _block_no * getfs_iio_BLOCK_SIZEv );
#ifdef INSTRUMENT_SYSTEM
    }
    else
        cache_hits++;
#endif
    
    // compute the index of the page where this block resides
    page = _block_no / _file->chn[_channel]->blocks;
    
    // copy block from cache
    memcpy( _block,
        cache_page_get_buffer( _file->chn[_channel]->cache->pages[page] ) +
        ( _block_no * getfs_iio_BLOCK_SIZEv - _file->chn[_channel]->cache->pages[page]->position ),
        getfs_iio_BLOCK_SIZEv );
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    _file->chn[_channel]->cache->pages[page]->timestamp = getfs_iio_CLOCK;
#endif
    
#if (getfs_iio_CC_POLICY == ___LFU___)
    _file->chn[_channel]->cache->pages[page]->hits++;
#endif
    
    // return the number of bytes read
    return getfs_iio_BLOCK_SIZEv;
}

void getfs_iio_close( getfs_iio_File* _file )
{
    int i;
    
#if ( getfs_iio_CC_POLICY == ___LRU___ )
    getfs_iio_CLOCK++;
#endif
    
    // flush the remaining data
    flush_data( _file );
    
    // write the header 
    write_header( _file );
    
    // auto-truncate file if necessary
    if( getfs_iio_IOMODE & 2 )
        auto_truncate( _file );
    
    // close the file, mark the handle as being closed 
    fclose( _file->file );
    _file->file = NULL; // handle no longer valid
    
    // deallocate channels
    for( i=0; i<_file->channels; i++ )
        if( _file->chn[i] != NULL )
		{
            if( _file->chn[i]->cache != NULL )
                cache_destroy( _file->chn[i]->cache );
            free( _file->chn[i] );
        }
        
        // deallocate the channel list
        free( _file->chn );
        
        // deallocate the file name
        if( _file->filename )
            free( _file->filename );
        
        // deallocate the handle 
        free( _file );
        
#ifdef INSTRUMENT_SYSTEM
        instrument_done();
#endif
        
}

// Read 'size' bytes from the given channel into 'data', return nr. of bytes read
int getfs_iio_read( getfs_iio_File* _file, int _channel, void* _data, int _size )
{
	::EnterCriticalSection( &g_sIIORead );
	
    int cblock, cp;
    int start, end, k;
    
#ifdef INSTRUMENT_SYSTEM
    iio_read_requests++;
#endif
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    getfs_iio_CLOCK++;
#endif
    
	if( _file->file == NULL ){
		::LeaveCriticalSection( &g_sIIORead );
        return -1;
	}
    
    k = 0;
    cp = _file->chn[_channel]->cp;
    
    for( cblock = ( cp / getfs_iio_BLOCK_SIZEv );
         cblock <= ( ( cp + _size - 1 ) / getfs_iio_BLOCK_SIZEv );
         cblock++ ) 
	{
#if ( getfs_iio_CC_POLICY == ___LRU___)
        getfs_iio_CLOCK++;
#endif
        
        // first block in sequence?
        if( cblock == ( cp / getfs_iio_BLOCK_SIZEv ) )
            start = cp % getfs_iio_BLOCK_SIZEv;
        else
            start = 0;
        
        // last block in sequence? 
        if( cblock == ( ( cp + _size - 1 ) / getfs_iio_BLOCK_SIZEv ) )
            end   = ( cp + _size - 1 ) % getfs_iio_BLOCK_SIZEv;
        else
            end   = getfs_iio_BLOCK_SIZEv - 1;
        
        // read cached channel block 
        if( ( start > 0) || ( end < ( getfs_iio_BLOCK_SIZEv - 1 ) ) )  
            cache_read_partial_channel_block( _file, _channel, cblock, start, end, (char*)_data + k );
        else
            cache_read_channel_block( _file, _channel, cblock, (char*)_data + k );
        
        k += ( end - start + 1 );
    }
    
    // update the channel pointer at this point, 'k' should be equal to 'size'
    _file->chn[_channel]->cp += k;
    
	::LeaveCriticalSection( &g_sIIORead );

    return k;
    
}

// Write 'size' bytes to the given channel from 'data', return  nr. of bytes written
int getfs_iio_write( getfs_iio_File* _file, int _channel, void* _data, int _size )
{
    int cblock, cp;
    int start, end, k;
    
#ifdef INSTRUMENT_SYSTEM
    iio_write_requests++;
#endif
    
#if ( getfs_iio_CC_POLICY == ___LRU___ )
    getfs_iio_CLOCK++;
#endif
    
    if( _file->file == NULL )
        return -1;
    
    k  = 0;
    cp = _file->chn[_channel]->cp;
    
    for( cblock = ( cp / getfs_iio_BLOCK_SIZEv );
         cblock <= ( ( cp + _size - 1 ) / getfs_iio_BLOCK_SIZEv );
         cblock++ ) 
	{
        
#if (getfs_iio_CC_POLICY == ___LRU___)
        getfs_iio_CLOCK++;
#endif
        
        // first block in sequence? 
        if( cblock == ( cp / getfs_iio_BLOCK_SIZEv ) )
            start = cp % getfs_iio_BLOCK_SIZEv;
        else
            start = 0;
        
        // last block in sequence? 
        if( cblock == ( ( cp + _size - 1) / getfs_iio_BLOCK_SIZEv ) )
            end   = ( cp + _size - 1) % getfs_iio_BLOCK_SIZEv;
        else
            end   = getfs_iio_BLOCK_SIZEv - 1;
        
        // write (cached) channel block
        if( ( start > 0) || ( end < ( getfs_iio_BLOCK_SIZEv - 1 ) ) )
            cache_write_partial_channel_block( _file, _channel, cblock, start, end, (char*)_data + k );
        else
            cache_write_channel_block( _file, _channel, cblock, (char*)_data + k );
        
        k += ( end - start + 1 );
    }
    
    // update the channel pointer at this point, 'k' should be equal to 'size' 
    _file->chn[_channel]->cp += k;
    
    // update the size of the channel
    if( _file->chn[_channel]->size <= _file->chn[_channel]->cp )
        _file->chn[_channel]->size = _file->chn[_channel]->cp;
    
    return k;
}

int getfs_iio_seek( getfs_iio_File* _file, int _channel, int _where )
{
#ifdef INSTRUMENT_SYSTEM
    iio_seek_requests++;
#endif
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    getfs_iio_CLOCK++;
#endif
    
    // check for illegal arguments 
    if( _file == NULL )
        return -1;
    
    if( ( _channel < 0) || ( _channel >= (int)_file->channels ) )
        return -1;
    
    // position the channel pointer
    _file->chn[_channel]->cp = _where;
    
    return _where;
}

// Allocate a new channel for the given IIO file, with a certain number of blocks per channel
int getfs_iio_allocate_channel( getfs_iio_File* _file, int _blocks ) 
{
    int               chn_no;
    getfs_iio_Channel* chn_ptr;
    
#if (getfs_iio_CC_POLICY == ___LRU___)
    getfs_iio_CLOCK++;
#endif
    
    // check for illegal arguments
    if( _file == NULL )
        return -1;
    
    // create new channel & initialize it 
    chn_ptr = (getfs_iio_Channel*)malloc( sizeof(getfs_iio_Channel) );
    chn_ptr->blocks = _blocks;
    chn_ptr->size   = 0;
    chn_ptr->cp     = 0;
    chn_ptr->cache  = NULL;
    
    // allocate new channel in the given IIO file
    chn_no  = (int)_file->channels++;
    _file->chn = (getfs_iio_Channel**)realloc( _file->chn, sizeof(getfs_iio_Channel*) * ( _file->channels ) );
    
    // error test
    if( _file->chn == NULL )
        return -1; // not enough memory
    
    // place new channel in its allocated slot, and return the index
    _file->chn[chn_no] = chn_ptr;
    
    // create a cache for this channel
    cache_create( _file, chn_no );
    
    return chn_no;
}

// Close and remove an IIO file
void getfs_iio_destroy( getfs_iio_File* _file )
{
    char* tempfn = (char*)malloc( strlen(_file->filename) + 8 );
    if( tempfn )
        strcpy( tempfn, _file->filename );
    // close the file
    getfs_iio_close( _file );
    // remove the file!
#ifdef WIN32
    if( tempfn )
        remove( tempfn );
#else
    if( tempfn )
        unlink( tempfn );
#endif
    // deallocate memory 
    if( tempfn )
        free( tempfn );
}