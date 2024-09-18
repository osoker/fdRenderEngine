#include "getfs_fat.h"
#include <stdlib.h>

// gets the value of the node given by 'node_idx'
static int node_get_value( getfs_fat_FAT* _fat, int _node_idx )
{
    int value;
    getfs_iio_seek( _fat->file, _fat->channel, _node_idx * sizeof(int) );
    getfs_iio_read( _fat->file, _fat->channel, &value, sizeof(int) );
    return value;
}

// returns the index of the first free node, starting from 'start'.
static int next_free( getfs_fat_FAT* _fat, int _start )
{
    int node = _start;

    // find the first non-zero
    while( ( node <= 0 ) || ( node_get_value( _fat, node ) != 0 ) )
        node++;
    
    // this is the non-allocated cell
    return node;
}

// sets the value of the node given by 'node_idx'
static void node_set_value( getfs_fat_FAT* _fat, int _node_idx, int _value )
{
    int v = _value;
    getfs_iio_seek( _fat->file, _fat->channel, _node_idx * sizeof(int) );
    getfs_iio_write( _fat->file, _fat->channel, &v, sizeof(int) );
}

// recovers the given node (makes it available)
static int node_recover( getfs_fat_FAT* _fat, int _node_idx )
{
    // set node to 0
    if( _node_idx > 0 )
        node_set_value( _fat, _node_idx, 0 );
        /* update the 'unallocated' pointer. Notice that
        node #0 is never used (thus recovered), since
    an index of 0 has a special meaning. */
    if( ( _node_idx < _fat->unallocated ) && ( _node_idx > 0 ) )
        _fat->unallocated = _node_idx;
    return 1; // must return 1 (true)
}

// returns the index of the last node in the chain (value = -1)
static int find_last_in_chain( getfs_fat_FAT* _fat, int _start )
{
    int node      = _start;
    int last_node;
    
    // iterate until the end of the chain
    do 
	{
        last_node = node;
        node = node_get_value( _fat, node );
    } 
	while( node != GETFS_FAT_EOC );
    
    // return last non-EOC node found
    return last_node;
}


// Opens an existing FAT and initializes it with the information found in the given channel
getfs_fat_FAT* getfs_fat_open( getfs_iio_File* _file, int _channel_no )
{
    getfs_fat_FAT* fat;
    
    if( _file == NULL )
        return NULL;
    
    // create and initialize a new fat handle
    fat = (getfs_fat_FAT*)malloc( sizeof(getfs_fat_FAT) );
    fat->channel     = _channel_no;
    fat->file        = _file;
    fat->unallocated = next_free( fat, 0 );

    return fat;
}


// Creates a FAT using a new channel in the given IIO file
getfs_fat_FAT* getfs_fat_create( getfs_iio_File* _file, int _blocks_per_channel )
{
    int           chn;
    getfs_fat_FAT* fat;
    
    if( _file == NULL )
        return NULL;
    
    if( _blocks_per_channel == 0 )
        _blocks_per_channel = GETFS_FAT_DEF_CHANNEL_SIZE;
    
    // allocate new channel
    chn = getfs_iio_allocate_channel( _file, _blocks_per_channel );
    
    // allocate new fat handle and initialize it
    fat = (getfs_fat_FAT*)malloc( sizeof(getfs_fat_FAT) );
    fat->file        = _file;
    fat->channel     = chn;
    fat->unallocated = 1;
    
    return fat;
}

// Allocates a new 1-node chain and returns the index of the first (only) node in the chain
int getfs_fat_create_chain( getfs_fat_FAT* _fat )
{
    int node;
    
    if( ( _fat == NULL) || (_fat->file == NULL ) )
        return -1; // oops
    
    // initializes the node with the index of the available node
    // (fat->unallocated), and update fat->unallocated
    node = _fat->unallocated;
    node_set_value( _fat, node, GETFS_FAT_EOC );
    _fat->unallocated = next_free( _fat, _fat->unallocated );
    
    // return the node
    return node;
    
}

// Get the index of the 'n'-th node in the chain, given the start node
// the "0th" node is always the start node
int getfs_fat_chain_get_nth( getfs_fat_FAT* _fat, int _first_node_index, int _n )
{
    int node = _first_node_index;
    int nth  = 0;
    
    if( ( _fat == NULL ) || ( _fat->file == NULL ) )
        return -1; // oops
    
    // iterate until n-th is found or EOC is encountered
    while( ( nth < _n ) && ( node != GETFS_FAT_EOC ) )
	{
        node = node_get_value( _fat, node );
        nth++;
    }
    
    // return the node
    return node;
}

// Calls a user function for each node in the chain
void getfs_fat_chain_for_each( getfs_fat_FAT* _fat, int _first_node_index, int (*p)(getfs_fat_FAT*, int ) ) 
{
    int node  = _first_node_index;
    int node2 = node;
    int quit  = 0;
    
    if( ( _fat == NULL) || ( _fat->file == NULL ) )
        return; // oops
    
    // iterate until end of chain (-1) is encountered
    do 
	{
        node2 = node_get_value( _fat, node );
        if( !p( _fat, node ) )
            quit = 1;
        else 
		{
            if( node <= 0 )
                quit = 1;
            node = node2;
        }
    } 
	while ( !quit && ( node != GETFS_FAT_EOC ) );
}

// Destroys the chain given by the index of the first node in the chain
int getfs_fat_destroy_chain( getfs_fat_FAT* _fat, int _first_node_index )
{
    if( ( _fat == NULL ) || ( _fat->file == NULL ) || ( _first_node_index <= 0 ) )
        return -1; // oops
    getfs_fat_chain_for_each( _fat, _first_node_index, node_recover );
    return 0;
}

// Truncates the chain starting at the given node
int getfs_fat_chain_truncate( getfs_fat_FAT* _fat, int _first )
{
    int following;
    
    if( ( _fat == NULL ) || ( _fat->file == NULL ) )
        return -1;
    
    // get the following node
    following = node_get_value( _fat, _first );
    
    // truncate at this point
    node_set_value( _fat, _first, GETFS_FAT_EOC );
    
    if( following != GETFS_FAT_EOC )
	{
        // destroy the chain that starts at 'following'
        getfs_fat_destroy_chain( _fat, following );
	}
    
    return 0;
}

// Allocates a new node for the given chain and appends it at the end. Returns the node index.
int getfs_fat_chain_extend( getfs_fat_FAT* _fat, int _first_node_index )
{
    int node, last_node;
    
    if( ( _fat == NULL ) || ( _fat->file == NULL ) )
        return -1; // oops
    
    // get a free node
    node = _fat->unallocated;
    node_set_value( _fat, node, GETFS_FAT_EOC );
    _fat->unallocated = next_free( _fat, _fat->unallocated );
    
    // find last node and link it to this one
    last_node = find_last_in_chain( _fat, _first_node_index );
    node_set_value( _fat, last_node, node );
    
    // return the last allocated node
    return node;
}

// Closes the given FAT
int getfs_fat_close( getfs_fat_FAT* _fat )
{
    if( _fat != NULL )
        free( _fat );
    return 0;
}

