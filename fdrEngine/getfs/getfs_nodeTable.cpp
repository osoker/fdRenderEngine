#include "getfs_nodeTable.h"
#include <stdlib.h>

static int node_set( getfs_nt_NT* _nt, int _node, getfs_nt_Node* _node_info )
{
    getfs_iio_seek( _nt->file, _nt->channel, _node * sizeof(getfs_nt_Node) );
    return getfs_iio_write( _nt->file, _nt->channel, _node_info, sizeof(getfs_nt_Node) );
}

static int node_get( getfs_nt_NT* _nt, int _node, getfs_nt_Node* _node_info )
{
    getfs_iio_seek( _nt->file, _nt->channel, _node * sizeof(getfs_nt_Node) );
    return getfs_iio_read( _nt->file, _nt->channel, _node_info, sizeof(getfs_nt_Node) );
}


static int find_first_free( getfs_nt_NT* _nt, int _start )
{
    getfs_nt_Node ni;
    int node = _start;
    
    // look for the first free one
    node_get( _nt, node, &ni );
    while( ni.refcount > 0 )
        node_get( _nt, ++node, &ni );
    
    return node;
}

static int node_recover( getfs_nt_NT* _nt, int _node ) 
{
    getfs_nt_Node ni;
    
#ifdef PURIFY
    // Initialize ni to avoid UMR/UMC in purify. 
    memset( &ni, 0, sizeof(ni) );
#endif // PURIFY 
    
    // init an empty node 
    ni.refcount = 0;
    ni.size     = 0;
    ni.chain    = 0;
    
    // delete node 
    node_set( _nt, _node, &ni );
    
    // update chain
    if( _node < _nt->unallocated )
        _nt->unallocated = _node;
    
    return 0;
}

/*
static int node_recover( getfs_nt_NT* _nt, int _node )
{
    getfs_nt_Node ni;
    
#ifdef PURIFY
    // Initialize ni to avoid UMR/UMC in purify.
    memset(&ni, 0, sizeof(ni));
#endif // PURIFY
    
    // init an empty node
    ni.refcount = 0;
    ni.size     = 0;
    ni.chain    = 0;
    
    // delete node
    node_set( _nt, _node, &ni );
    
    // update chain
    if( _node < _nt->unallocated )
        _nt->unallocated = _node;
    
    return 0;
}
*/

// Get the chain associated with a node 
int getfs_nt_node_get_chain( getfs_nt_NT* _nt, int _node )
{
    getfs_nt_Node ni;
    getfs_nt_get_node( _nt, _node, &ni );
    return ni.chain;
}

// Decrement refcount, if refcount=0 delete node and ret true, otherwise return false 
int getfs_nt_refcount_decr( getfs_nt_NT* _nt, int _node )
{
    getfs_nt_Node ni;
    int          must_delete = 0;
    
    // decrement refcount 
    getfs_nt_get_node( _nt, _node, &ni );
    ni.refcount--;
    if( ni.refcount <= 0 )
        must_delete = 1;
    getfs_nt_set_node( _nt, _node, &ni );
    
    // delete node if necessary 
    if( must_delete )
        node_recover( _nt, _node );
    
    // pass to caller 
    return must_delete;
}

// Update the given node with the information from the given nfs_nt_Node
int getfs_nt_set_node( getfs_nt_NT* _nt, int _node, getfs_nt_Node* _node_info )
{
    return node_set( _nt, _node, _node_info );
}

// Get the information in node 'node'
int getfs_nt_get_node( getfs_nt_NT* _nt, int _node, getfs_nt_Node* _node_ptr )
{
    if( _nt == NULL )
        return -1;
    return node_get( _nt, _node, _node_ptr );
}

// Open the nodeTable stored in channel 'channel' of the given IIO file
getfs_nt_NT* getfs_nt_open( getfs_iio_File* _file, int _channel )
{
    getfs_nt_NT* nt;
    
    // allocate new nodeTable handle and initialize it
    nt = (getfs_nt_NT*)malloc( sizeof(getfs_nt_NT) );
    nt->file        = _file;
    nt->channel     = _channel;
    nt->unallocated = find_first_free( nt, 0 );
    
    return nt;
}

// Create a new nodeTable in the given IIO file, using the given nr. of blocks per channel
getfs_nt_NT* getfs_nt_create( getfs_iio_File* _file, int _blocks_per_channel ) 
{
    getfs_nt_NT* nt;
    int         chn;
    
    if( _file == NULL )
        return NULL;
    
    if( _blocks_per_channel == 0 )
        _blocks_per_channel = GETFS_NT_DEF_CHANNEL_SIZE;
    
    // allocate new channel
    chn = getfs_iio_allocate_channel( _file, _blocks_per_channel );
    
    // allocate new nodeTable handle and initialize it
    nt = (getfs_nt_NT*)malloc( sizeof(getfs_nt_NT) );
    nt->file        = _file;
    nt->channel     = chn;
    nt->unallocated = 0;
    
    return nt;
}

// Allocate a new node, return the index
int getfs_nt_allocate_node( getfs_nt_NT* _nt )
{
    int          node;
    getfs_nt_Node ni;
    
    if( _nt == NULL )
        return -1;
    
    // get head of free chain
    node = _nt->unallocated;
    
    // initialize the node
    getfs_nt_get_node( _nt, node, &ni );
    ni.refcount = 1;
    ni.size     = 0;
    ni.chain    = 0;
    getfs_nt_set_node( _nt, node, &ni );
    
    // update the free chain
    _nt->unallocated = find_first_free( _nt, _nt->unallocated );
    
    // return the node index
    return node;
}

// Get the size of the data associated with a node
int getfs_nt_node_get_size( getfs_nt_NT* _nt, int _node )
{
    getfs_nt_Node ni;
    getfs_nt_get_node( _nt, _node, &ni );
    return ni.size;
}

// Set the new size of data associated with a node
void getfs_nt_node_set_size( getfs_nt_NT* _nt, int _node, int _size )
{
    getfs_nt_Node ni;
    getfs_nt_get_node( _nt, _node, &ni );
    ni.size = _size;
    getfs_nt_set_node( _nt, _node, &ni );
}

// Close the nodeTable
void getfs_nt_close( getfs_nt_NT* _nt )
{
    if( _nt != NULL )
        free( _nt );
}

// Destroy the nodeTable
void getfs_nt_destroy( getfs_nt_NT* _nt )
{
    getfs_nt_close( _nt );
    // remove ... 
    // ...
}