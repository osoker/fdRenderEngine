#include "getfs_dirTable.h"
#include <stdlib.h>
#include <string.h>
#include "getfs_pmatch.h"
#include <windows.h>
#include "getfs.h"

typedef struct getfs_dt_TrieNode {
    unsigned short nt_index;               /* index in NT */
    short          bindex;                 /* bit index (for Patricia) */
    int            kindex;                 /* index in Key table (channel 1), and avail bit (msb) */
    int            left;                   /* index of left child */
    int            right;                  /* index of right child */
} getfs_dt_TrieNode;

#define getfs_dt_MAX_SKN_SIZE       60

typedef struct getfs_dt_KeyNode {
    int  next;                             /* next plus availability bit (msb) */
    char name[getfs_dt_MAX_SKN_SIZE];       /* (partial) name */
	int  original_size;          
	int  zip_size;
} getfs_dt_KeyNode;


static int trienode_get( getfs_dt_DT* _dt, int _node, getfs_dt_TrieNode* _n )
{
	::EnterCriticalSection( &g_sTrienode_get );	
    getfs_iio_seek( _dt->file, _dt->channel1, _node * sizeof(getfs_dt_TrieNode) );
	int ret = getfs_iio_read( _dt->file, _dt->channel1, _n, sizeof(getfs_dt_TrieNode) );
	::LeaveCriticalSection( &g_sTrienode_get );
    return ret;
}

// get a key node (one node might contain only a partial key!)
static int keynode_get( getfs_dt_DT* _dt, int _node, getfs_dt_KeyNode* _n )
{
    getfs_iio_seek( _dt->file, _dt->channel2, _node * sizeof(getfs_dt_KeyNode) );
    return getfs_iio_read( _dt->file, _dt->channel2, _n, sizeof(getfs_dt_KeyNode) );
}

// return 1 if the Trie node is free, 0 otherwise
static int trienode_is_free( getfs_dt_DT* _dt, int _node )
{
    getfs_dt_TrieNode ni;
    int k;
    if( _node == 0 )
        return 0; // no! #0 is reserved!
    k = trienode_get( _dt, _node, &ni );
    if( k < 0 )
        return 0;
    return ( ( ni.kindex & 0x80000000 ) == 0 );
}

static int trienode_find_first_free( getfs_dt_DT* _dt, int _start )
{
    int index = ( _start - 1 );
    do { ++index; }
	while (!trienode_is_free( _dt, index ) || ( index <= 0 ) );
    return index;
}

// return 1 if the given Key node is free, 0 otherwise
static int keynode_is_free( getfs_dt_DT* _dt, int _node )
{
    getfs_dt_KeyNode ni;
    int k;
    
    if( _node == 0 )
        return 0; // no! #0 is reserved!
    
    k = keynode_get( _dt, _node, &ni );
    if( k < 0 )
        return 0;
    return (( ni.next & 0x80000000) == 0 );
}

static int keynode_find_first_free( getfs_dt_DT* _dt, int _start )
{
    int index = ( _start - 1 );
    do { ++index; } while (!keynode_is_free( _dt, index ) || (index <= 0) );
    return index;
}

// set the value of the given key node
static int keynode_set( getfs_dt_DT* _dt, int _node, getfs_dt_KeyNode* _n ) 
{
    getfs_iio_seek( _dt->file, _dt->channel2, _node * sizeof(getfs_dt_KeyNode) );
    return getfs_iio_write( _dt->file, _dt->channel2, _n, sizeof(getfs_dt_KeyNode) );
}

// set the value of the given Trie node
static int trienode_set( getfs_dt_DT* _dt, int _node, getfs_dt_TrieNode* _n )
{
    getfs_iio_seek( _dt->file, _dt->channel1, _node * sizeof(getfs_dt_TrieNode) );
    return getfs_iio_write( _dt->file, _dt->channel1, _n, sizeof(getfs_dt_TrieNode) );
}

static int p_get_head( getfs_dt_DT* _dt ) 
{
    return 0; // node at index 0 is always the head (root) of the Patricia tree
}

// get the 'left' field of the given Trie node
static int trienode_get_left( getfs_dt_DT* _dt, int _node )
{
    int kk, r;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) +
        /* offset  of 'left' in the structure */
        (2 * sizeof(int)) /* nt_idx, bindex, kindex */
        );
    r = getfs_iio_read( _dt->file, _dt->channel1, &kk, sizeof(int) );
    return ( ( r < sizeof(int)) ? 0 : kk );
}

// get the 'right' field of the given Trie node
static int trienode_get_right( getfs_dt_DT* _dt, int _node )
{
    int kk, r;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) +
        /* offset  of 'right' in the structure */
        (3 * sizeof(int))       /* nt_index + bindex + kindex + left */
        );
    r = getfs_iio_read( _dt->file, _dt->channel1, &kk, sizeof(int) );
    return ( ( r < sizeof(int) ) ? 0 : kk );
}

// get the 'bindex' field of the given Trie node
static int trienode_get_bindex( getfs_dt_DT* _dt, int _node )
{
    short kk, r;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) +
        /* offset  of 'bindex' in the structure */
        (sizeof(short)               /* nt_index */
        ) );
    r = getfs_iio_read( _dt->file, _dt->channel1, &kk, sizeof(short) );
    return ( ( r < sizeof(short) ) ? 0 : ((int)kk) );
}

static int bit_get( const char* _bit_stream, int _n )
{
    int k = ( _n & 0x7 );
    if( _n < 0 ) return 2;
    return ( (*(_bit_stream + (_n >> 3))) >> k) & 0x1;
}

static int p_compare_keys( getfs_dt_DT* _dt, const char* _key, int _node )
{
    getfs_dt_TrieNode tn;
    getfs_dt_KeyNode kn;
    int i, j, s;
    
    // key length
    s = strlen( _key );
    
    // get trie node
    trienode_get( _dt, _node, &tn );
    
    // compare the keys (in 60-byte chunks)
    i = 0;
    j = ( tn.kindex & 0x7fffffff );
    while( i < s )
	{
        keynode_get( _dt, j, &kn );
        if( strncmp( (char*)_key + i, kn.name, GETFS_DT_MAX_SKN_SIZE) != 0 )
            return 0; /* not equal */
        j = kn.next & 0x7fffffff;
        i += GETFS_DT_MAX_SKN_SIZE;
    }
    
    return 1;
    
}

// lookup a given key, return its index in the file (channel). If the key is not found, return -1
// Note: this is actually a Patricia tree item search (compacted binary trie search)
static int p_lookup_key( getfs_dt_DT* _dt, const char* _key ) 
{
    int p, x, px;
    
    // start at the root
    p = p_get_head( _dt );
    x = trienode_get_right( _dt, p );
    
    // navigate down the tree until an upward link is encountered
    px = trienode_get_bindex( _dt, x );
    while( trienode_get_bindex( _dt, p ) < px )
	{
        p = x;
        x = bit_get( _key, px ) ? trienode_get_right( _dt, x ) : trienode_get_left( _dt, x );
        px = trienode_get_bindex( _dt, x );
    }
    
    // full string comparison
    if( !p_compare_keys( _dt, _key, x ) )
	{
		char   t_sz[256];
		sprintf( t_sz,"getfs:search <%s>,NOT FOUND!..\n",_key );
		OutputDebugString( t_sz );
        return -1; // not found!
	}

    // return the node index
    return x;
}

// extract the key from the given fnode
static int fnode_extract_key( getfs_dt_DT* _dt, int _node, char* _key )
{
    register int i, j;
    int k, next;
    getfs_dt_KeyNode kn;
    
    i = 0;
    next = _node;
    while( next > 0 )
	{
        // get keynode
        k = next;
        keynode_get( _dt, k, &kn );
        // copy contents to key - hope this will be
        // properly optimized by the compiler... 
        j = 0;
        while( ( j < GETFS_DT_MAX_SKN_SIZE ) && ( kn.name[j] != 0 ) )
            _key[i++] = kn.name[j++];

        // next node
        next = ( kn.next & 0x7fffffff );
    }
    _key[i] = 0;
    return 0;
}

static int bit_first_different( const char* bit_stream1, char* bit_stream2 )
{
    int n = 0;
    int d = 0;
    while ( (bit_stream1[n] == bit_stream2[n]) &&
        ( bit_stream1[n] != 0) &&
        ( bit_stream2[n] != 0) )
        n++;
    while( bit_get( &bit_stream1[n], d ) == bit_get(&bit_stream2[n], d ) )
        d++;
    return ( ( n << 3 ) + d );
}

static int p_find_first_different_bit( getfs_dt_DT* _dt, const char* _key, int _node )
{
    getfs_dt_TrieNode tn;
    char tempname[4096];
    
    // get the trie node
    trienode_get( _dt, _node, &tn );
    
    // extract the full key from the fnode
    fnode_extract_key( _dt, tn.kindex & 0x7fffffff, tempname );
    
    return bit_first_different( _key, tempname );
}

// allocates a new chain of keynodes using the given key. Returns the index of the first keynode in chain.
static int fnode_allocate( getfs_dt_DT* _dt, const char* _key )
{
    int keynodes[1024]; /* 60*1024 = 61240 bytes / filename (max) */
    int nr_keynodes;
    int klen, i;
    getfs_dt_KeyNode kn;
    
    klen = strlen( _key );
    nr_keynodes = 1 + ( klen / GETFS_DT_MAX_SKN_SIZE );
    
    // preallocate
    i = 0;
    while( i < nr_keynodes )
	{
        keynodes[i] = _dt->unallocated2;
        _dt->unallocated2 = keynode_find_first_free( _dt, _dt->unallocated2 + 1 );
        i++;
    }
    
    // fill in the actual info
    for( i = 0; i < nr_keynodes; i++ )
	{
        // wire properly 
        if( i == ( nr_keynodes - 1 ) )
            kn.next = 0x80000000;
        else
            kn.next = 0x80000000 | keynodes[i+1];
        // partial key
        strncpy( kn.name, (char*)_key + i * GETFS_DT_MAX_SKN_SIZE, GETFS_DT_MAX_SKN_SIZE );
        // write keynode
        keynode_set( _dt, keynodes[i], &kn );
    }
    
    // return first in chain
    return keynodes[0];
    
}



// returns the index of the trie node, not the key node!
static int node_allocate( getfs_dt_DT* _dt, const char* _filename, int _nt_index, int _bindex )
{
    int keynode, trienode;
    getfs_dt_TrieNode tn;
    
    // first, allocate (and fill in) the keynode chain (fnode)
    keynode = fnode_allocate( _dt, _filename );
    
    // second, allocate (and fill in) a new trienode
    trienode = _dt->unallocated;
    tn.nt_index = _nt_index;
    tn.bindex   = _bindex;
    tn.kindex   = keynode | 0x80000000;
    tn.left     = 0;
    tn.right    = 0;
    trienode_set( _dt, trienode, &tn );
    
    // next in chain
    _dt->unallocated = trienode_find_first_free( _dt, _dt->unallocated );
    
    // return the trie node
    return trienode;
    
}

// set the 'left' field of the given Trie node 
static int trienode_set_left( getfs_dt_DT* _dt, int _node, int _left )
{
    int kk = _left;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) +
        /* offset  of 'left' in the structure */
        (sizeof(int) +              /* nt_index (+bindex) */
        sizeof(int)                /* bindex or kindex */
        ) );
    return getfs_iio_write( _dt->file, _dt->channel1, &kk, sizeof(int) );
}

// set the 'right' field of the given Trie node 
static int trienode_set_right( getfs_dt_DT* _dt, int _node, int _right )
{
    int kk = _right;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) +
        /* offset  of 'right' in the structure */
        (sizeof(int) +              /* nt_index (+bindex) */
        sizeof(int) +              /* bindex or kindex */
        sizeof(int)                /* left */
        ) );
    return getfs_iio_write( _dt->file, _dt->channel1, &kk, sizeof(int) );
}

// inserts the given key in the tree, and return the index of the trie node
static int p_insert_key( getfs_dt_DT* _dt, const char* _key, int _nt_index )
{
    int p, t, x, px, pt;
    int i = 0;
    
    // start at the root
    p = p_get_head( _dt );
    t = trienode_get_right( _dt, p );
    
    // navigate down the tree and look for the key
    pt = trienode_get_bindex( _dt, t );
    while( trienode_get_bindex( _dt, p ) < pt )
	{
        p = t;
        t = bit_get( _key, pt ) ? trienode_get_right( _dt, t ) : trienode_get_left( _dt, t );
        pt = trienode_get_bindex( _dt, t );
    }
    
    // is the key already in the tree?
    if( p_compare_keys( _dt, _key, t ) )
        return -1; // already in the tree!
    
    // find first bit that does not match
    i = p_find_first_different_bit( _dt, _key, t );
    
    // find the appropriate place in the tree where the node has to be inserted
    p  = p_get_head( _dt );
    x  = trienode_get_right( _dt, p );
    px = trienode_get_bindex( _dt, x );
    while( ( trienode_get_bindex( _dt, p ) < px ) && ( px < i ) )
	{
        p = x;
        x = bit_get( _key, px ) ? trienode_get_right( _dt, x ) : trienode_get_left( _dt, x );
        px = trienode_get_bindex( _dt, x );
    }
    
    // allocate a new node and initialize it
    t = node_allocate( _dt, _key, _nt_index, i );
    trienode_set_left( _dt, t, ( bit_get(_key, i) ? x : t ) );
    trienode_set_right( _dt, t, ( bit_get(_key, i) ? t : x ) );
    
    // rewire
    if( bit_get( _key, trienode_get_bindex( _dt, p ) ) )
        trienode_set_right( _dt, p, t );
    else
        trienode_set_left( _dt, p, t );
    
    return t;
}

/* the first node in the channel (#0) will always be reserved for internal
information (the root of the Patricia structure). It cannot be allocated
or deallocated. */
static int p_init_head( getfs_dt_DT* _dt )
{    
    getfs_dt_TrieNode tn;
    getfs_dt_KeyNode  kn;
    
    // init keynode
    kn.next     = 0x80000000;
    kn.name[0]  = '.';
    kn.name[1]  = 0;
    
    // init trienode
    tn.nt_index = 0;
    tn.bindex   = -1;
    tn.kindex   = 0x80000000; /* root */
    tn.left     = 0;
    tn.right    = 0;
    
    keynode_set( _dt, 0, &kn );
    return trienode_set( _dt, 0, &tn );
}

// get the 'nt_index' field of the given Trie node
static int trienode_get_nt( getfs_dt_DT* _dt, int _node )
{
    int r;
    unsigned short kk;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) );
    r = getfs_iio_read( _dt->file, _dt->channel1, &kk, sizeof(unsigned short) );
    return ( ( r < sizeof(unsigned short)) ? 0 : (int)kk );
}

// set the 'nt_index' field of the given Trie node
static int trienode_set_nt( getfs_dt_DT* _dt, int _node, int _nt )
{
    unsigned short kk = (unsigned short)_nt;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof(getfs_dt_TrieNode) );
    return getfs_iio_write( _dt->file, _dt->channel1, &kk, sizeof(unsigned short) );
}

// Set the NT index associated with a given filename (given by its index in the DT)
int getfs_dt_filename_set_nt_index( getfs_dt_DT* _dt, int _filename_idx, int _nt_idx )
{
    return trienode_set_nt( _dt, _filename_idx, _nt_idx );
}

// Add new file name, return index in channel
int getfs_dt_filename_add( getfs_dt_DT* _dt, const char* _filename )
{
    if( _filename == NULL )
        return -1;
    return p_insert_key( _dt, _filename, 0 );
}

// Lookup file name in DT, return index in DT
int getfs_dt_filename_lookup( getfs_dt_DT* _dt, const char* _filename )
{
    if( _filename == NULL )
	{
		::LeaveCriticalSection( &g_sFileExist );
		OutputDebugString( "getfs: NULL FILE NAME...\n" );
		return -1;
	}


	::EnterCriticalSection( &g_sFileExist );	
	int key = p_lookup_key( _dt, _filename );
	::LeaveCriticalSection( &g_sFileExist );

    return key;	
}

// Get the nodeTable index associated with a given filename (given by its index in the DT)
int getfs_dt_filename_get_nt_index( getfs_dt_DT* _dt, int _filename_idx )
{
    return trienode_get_nt( _dt, _filename_idx );
}

// Open a directory table
getfs_dt_DT* getfs_dt_open( getfs_iio_File* _file, int _channel1, int _channel2 )
{
    getfs_dt_DT* dt;
    
    // allocate new DT handle and initialize it
    dt = (getfs_dt_DT*)malloc( sizeof(getfs_dt_DT) );
    dt->file        = _file;
    dt->channel1    = _channel1;
    dt->channel2    = _channel2;
    dt->unallocated  = trienode_find_first_free( dt, 1 );
    dt->unallocated2 = keynode_find_first_free( dt, 1 );
    
    return dt;
}

// Create a new directory table in the given iio file & channel
getfs_dt_DT* getfs_dt_create( getfs_iio_File* _file, int _bpc1, int _bpc2 )
{
    getfs_dt_DT*  dt;
    int          chn1, chn2;
    
    if( _file == NULL )
        return NULL;
    
    // init with defaults if necessary
    if( _bpc1 == 0 )
        _bpc1 = GETFS_DT_DEF_CHANNEL1_SIZE;
    
    if( _bpc2 == 0 )
        _bpc2 = GETFS_DT_DEF_CHANNEL2_SIZE;
    
    // allocate two new channels
    chn1 = getfs_iio_allocate_channel( _file, _bpc1 );
    chn2 = getfs_iio_allocate_channel( _file, _bpc2 );
    
    // allocate new DT handle and initialize it
    dt = (getfs_dt_DT*)malloc( sizeof(getfs_dt_DT) );
    dt->file        = _file;
    dt->channel1    = chn1;
    dt->channel2    = chn2;
    dt->unallocated = 1;
    dt->unallocated2 = 1;
    
    // initialize the first node in the channel (Trie&Key nodes #0)
    p_init_head( dt ); /* this node (0) will always be reserved for the root of the tree */
    
    return dt;
}

// Close the directory table
int getfs_dt_close( getfs_dt_DT* _dt )
{
    if( _dt != NULL )
        free( _dt );
    return 0;
}

// Destroy the directory table
int getfs_dt_destroy( getfs_dt_DT* _dt )
{
    // close
    getfs_dt_close( _dt );
    // remove
    // ... 
    return 0;
}

/* the left child or the right child (indices in the DT) are >= 0. An
index of 0 means the first node in the table. Therefore we cannot
have "null" pointers (missing children). This is actually
great, since the Patricia trie does not use null pointers, either. */
static char* find_prefix( char* _s, char* _buffer ) 
{
    int idx = 0;
    while ( ( _s[idx] != 0)   &&
        ( _s[idx] != '*') &&
        ( _s[idx] != '?') &&
        ( _s[idx] != '[') ) 
	{
        _buffer[idx] = _s[idx];
        idx++;
    }
    _buffer[idx] = 0; // asciiz
    return _buffer;
}

/* lookup a given key (or prefix), return its index in the file (channel). If the
key is not found, return -1. Restrict the search to the first 'n' bits. 
No final string comparison is performed. */
static int p_lookup_key_n( getfs_dt_DT* _dt, char* _key, int _n ) 
{
    int p, x, px;
    
    // start at the root 
    p = p_get_head( _dt );
    x = trienode_get_right( _dt, p );
    
    // navigate down the tree until an upward link is encountered
    px = trienode_get_bindex( _dt, x );
    while ( ( trienode_get_bindex( _dt, p ) < px ) && ( px < _n ) ) 
	{
        p = x;
        x = bit_get( _key, px ) ? trienode_get_right( _dt, x ) : trienode_get_left( _dt, x );
        px = trienode_get_bindex( _dt, x );
    }
    
    // return the last node that's not discriminating on a bit with index greater than n
    return p;
}

// get the 'kindex' field of the given Trie node
static int trienode_get_kindex( getfs_dt_DT* _dt, int _node ) 
{
    int kk, r;
    getfs_iio_seek( _dt->file, _dt->channel1,
        _node * sizeof( getfs_dt_TrieNode ) +
        // offset of 'kindex' in the structure
        ( sizeof(int) )               // nt_index + bindex
         );
    r = getfs_iio_read( _dt->file, _dt->channel1, &kk, sizeof(int) );
    return ( ( r < sizeof(int)) ? 0 : ( kk & 0x7fffffff ) );
}

static char getfs_glob_key[GETFS_DT_MAX_SUBNAME_SIZE];
static int p_node_iterate( getfs_dt_DT* _dt, int _root, int _from_bidx, char* _pattern, int _flags,
                          int (*p)( getfs_dt_DT* _dt, char* _filename, int _node, void* _tmp ),
                          void* _tmp )
{
    int bidx, tl, tr;

	bidx = trienode_get_bindex( _dt, _root );
    
    if( bidx <= _from_bidx )
	{
        // The root of the Patricia structure cannot be inspected. Continue the search, but cancel the branch
        if( _root <= 0 )
            return 1;
        
        // process the node: does the key match the pattern? If it does, call the user function.
        fnode_extract_key( _dt, trienode_get_kindex( _dt, _root ), getfs_glob_key );
        if( getfs_pmatch( _pattern, getfs_glob_key, _flags ) != FNM_NOMATCH )
            if( p != 0 )
                if( !p( _dt, getfs_glob_key, _root, _tmp ) )
                    return 0;
                
                // after an upward link: stop 
                return 1;
    }
    
    // recursively traverse the left and right subtrees
    tl = trienode_get_left( _dt, _root );
    tr = trienode_get_right( _dt, _root );
    if( !p_node_iterate( _dt, tl, bidx, _pattern, _flags, p, _tmp ) )
        return 0;
    if( !p_node_iterate( _dt, tr, bidx, _pattern, _flags, p, _tmp ) )
        return 0;
    
    return 1;
}

static int keynode_clear( getfs_dt_DT* _dt, int _node )
{
    getfs_dt_KeyNode ni;
    memset( &ni, 0, sizeof(ni) );
    ni.next     = 0;
    ni.name[0]  = 0;
    return keynode_set( _dt, _node, &ni );
}

// deallocates the chain of keynodes given by the keynode index "start" 
static int fnode_free( getfs_dt_DT* _dt, int _start )
{
    int             next = _start;
    int             k;
    getfs_dt_KeyNode kn;
    
    while( next != 0 )
	{
        // get the next block in chain
        k = next;
        keynode_get( _dt, k, &kn );
        next = kn.next & 0x7fffffff;
        // clear this one 
        keynode_clear( _dt, k );
        if( k < _dt->unallocated2 )
            _dt->unallocated2 = k;
    }
    
    return 0;
}

// copy the key and the nt_index from one node to another 
static int node_copy_key( getfs_dt_DT* _dt, int _from, int _to )
{
    getfs_dt_TrieNode tn1, tn2;
    int old_kn2;
    
    trienode_get( _dt, _from, &tn1 );
    trienode_get( _dt, _to,   &tn2 );
    old_kn2 = tn2.kindex & 0x7fffffff;
    
    // rewire from's key to 'to'
    tn2.nt_index = tn1.nt_index;
    tn2.kindex   = tn1.kindex; // this is the actual rewiring 
    
    // save "to" 
    trienode_set( _dt, _to, &tn2 );
    
    // the old key can now be deleted 
    fnode_free( _dt, old_kn2 );
    
    return 0;
}

//clear the node
static int trienode_clear( getfs_dt_DT* _dt, int _node ) 
{
    getfs_dt_TrieNode ni;
    ni.nt_index = 0;
    ni.bindex   = 0;
    ni.kindex   = 0;
    ni.left     = _node;
    ni.right    = _node;
    return trienode_set( _dt, _node, &ni );
}


// recover a trienode
static int trienode_recover( getfs_dt_DT* _dt, int _node_idx )
{
    // clear
    trienode_clear( _dt, _node_idx );
    
    // update chains
    if( _node_idx < _dt->unallocated )
        _dt->unallocated = _node_idx;
    
    return 0;
}

// remove a key from the Patricia structure 
static int p_remove_key( getfs_dt_DT* _dt, char* _k ) 
{
    int p, t, x, pp, lp, pt, px;
    int bp, bl, br;
    char key[GETFS_DT_MAX_SUBNAME_SIZE + 1];
    
    // start at the root 
    pp = -1;
    p  = p_get_head( _dt );
    t  = trienode_get_right( _dt, p );
    
    // navigate down the tree and look for the key 
    pt = trienode_get_bindex( _dt, t );
    while( trienode_get_bindex( _dt, p ) < pt )
	{
        pp = p;
        p  = t;
        t  = bit_get( _k, pt ) ? trienode_get_right( _dt, t ) : trienode_get_left( _dt, t );
        pt = trienode_get_bindex( _dt, t );
    }
    
    // is the key in the tree? If not, get out!
    if( !p_compare_keys( _dt, _k, t ) )
        return -1; // key could not be found! 
    
    // copy p's key to t 
    if( t != p )
        node_copy_key( _dt, p, t );
    
    // is p a leaf? 
    bp = trienode_get_bindex( _dt, p );
    bl = trienode_get_bindex( _dt, trienode_get_left( _dt, p ) );
    br = trienode_get_bindex( _dt, trienode_get_right( _dt, p ) );
    
    if( ( bl > bp ) || ( br > bp ) ) 
	{
        // at least one downwards edge
        if( p != t )
		{
            // look for new (intermediary) key 
            fnode_extract_key( _dt, trienode_get_kindex( _dt, p ), key );
            lp = p;
            x  = bit_get( key, trienode_get_bindex( _dt, p ) ) ? trienode_get_right( _dt, p ) : trienode_get_left( _dt, p );
            px = trienode_get_bindex( _dt, x );
            while (trienode_get_bindex( _dt, lp ) < px )
			{
                lp = x;
                x  = bit_get( key, px ) ? trienode_get_right( _dt, x ) : trienode_get_left( _dt, x );
                px = trienode_get_bindex( _dt, x );
            }
            // If the intermediary key was not found, we have a problem.. 
            if( !p_compare_keys( _dt, key, x ) )
                return -1; // key could not be found! 
            // rewire leaf (lp) to point to t 
            if( bit_get( key, trienode_get_bindex( _dt, lp ) ) )
                trienode_set_right( _dt, lp, t );
            else
                trienode_set_left( _dt, lp, t );
        }
        // rewire parent to point to the real child of p 
        if( pp != p )
		{
            int ch;
            ch = bit_get( _k, trienode_get_bindex( _dt, p ) ) ? trienode_get_left( _dt, p ) : trienode_get_right( _dt, p ); /*!*/
            if( bit_get( _k, trienode_get_bindex( _dt, pp ) ) )
                trienode_set_right( _dt, pp, ch );
            else
                trienode_set_left( _dt, pp, ch );
        }
        
    }
	else 
	{
        // both links pointing upwards or to the node (self-edges) 
        
        // rewire the parent 
        if( pp != p )
		{
            bl = trienode_get_left( _dt, p );
            br = trienode_get_right( _dt, p );
            if( bit_get( _k, trienode_get_bindex( _dt, pp ) ) )
                trienode_set_right( _dt, pp, ( ( ( bl == br ) && ( bl == p ) ) ? pp : ( ( bl == p ) ? br : bl ) ) );
            else
                trienode_set_left( _dt, pp, ( ( ( bl == br ) && ( bl == p ) ) ? pp : ( ( bl == p ) ? br : bl ) ) );
        }
        
    }
    
    // deallocate p - no longer needed 
    trienode_recover( _dt, p ); // keynodes were removed earlier, see above
    
    return 0;
}

// set size info
static int p_set_key_size( getfs_dt_DT* _dt, char* _k, int _original_size, int _zip_size )
{
    getfs_dt_TrieNode tn;
    getfs_dt_KeyNode kn;
    int j,x;

	// River added @ 2010-5-7:加入cs,确保单线程执行。
	// find key
	::EnterCriticalSection( &g_sFileExist );	
	x = p_lookup_key( _dt, _k );
	::LeaveCriticalSection( &g_sFileExist );

	if( x < 0 )
		return -1;

    // get trie node
    trienode_get( _dt, x, &tn );
    
    j = ( tn.kindex & 0x7fffffff );
    keynode_get( _dt, j, &kn );
	kn.original_size = _original_size;
	kn.zip_size = _zip_size;
	keynode_set( _dt, j, &kn );

	return 0;
}

static int p_get_key_size( getfs_dt_DT* _dt, char* _k, int& _original_size, int& _zip_size )
{
    getfs_dt_TrieNode tn;
    getfs_dt_KeyNode kn;
    int j, x;
    
	// find key
	// River added @ 2010-5-7:加入cs,确保单线程执行。
	// find key
	::EnterCriticalSection( &g_sFileExist );	
	x = p_lookup_key( _dt, _k );
	::LeaveCriticalSection( &g_sFileExist );
	if( x < 0 )
		return -1;

    // get trie node
    trienode_get( _dt, x, &tn );
    
    j = ( tn.kindex & 0x7fffffff );
    keynode_get( _dt, j, &kn );
	_original_size = kn.original_size;
	_zip_size = kn.zip_size;

	return 0;
}

// Globbing (pattern matching): lookup those names that match the given pattern and
// for each of those names call the given callback */
int getfs_dt_filename_glob( getfs_dt_DT* _dt,
                         char* _pattern,
                         int _flags,
                         int (*p)(getfs_dt_DT* _dt, char* _filename, int _filename_idx, void* _tmp ),
                         void* _tmp ) 
{
    char prefix[256];
    int  start;
    
    // find commmon prefix (longest possible prefix in the
    // given pattern that does not contain wildcards)
    find_prefix( _pattern, prefix );
    
    // find the node corresponding to the longest
    // substring of the computed prefix
    start = p_lookup_key_n( _dt, prefix, strlen(prefix) * 8 );
    
    // all the keys in the subtree starting at the node
    // found above is a potential match. Get all these
    // keys, and perform a pattern matching on each one.
    p_node_iterate( _dt, start, trienode_get_bindex( _dt, start) - 1, _pattern, _flags, p, _tmp );
    
    return 0;
}

// Delete file name given by its index in table
int getfs_dt_filename_delete( getfs_dt_DT* _dt, char* _filename )
{
    if( _filename == NULL )
        return -1;
    return p_remove_key( _dt, _filename );
}

// 设置文件的大小信息
int getfs_dt_set_size_info( getfs_dt_DT* _dt, char* _filename, int _original_size, int _zip_size )
{
	if( _filename == NULL )
		return -1;
	::EnterCriticalSection( &g_sSetSize );
	int ret = p_set_key_size( _dt, _filename, _original_size, _zip_size );
	::LeaveCriticalSection( &g_sSetSize );
	return ret;
}

int getfs_dt_get_size_info( getfs_dt_DT* _dt, char* _filename, int& _original_size, int& _zip_size )
{
	if( _filename == NULL )
		return -1;
	return p_get_key_size( _dt, _filename, _original_size, _zip_size );
}