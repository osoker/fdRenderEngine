// Node Table (NT)
#ifndef _GETFS_NODETABLE_H_
#define _GETFS_NODETABLE_H_
#include "getfs_iio.h"

#define GETFS_NT_DEF_CHANNEL_SIZE    1 /* default channel size, in blocks */

typedef struct getfs_nt_Node {
    int       refcount;                 /* ref-count, used for hard links */
    int       size;                     /* data size */
    int       chain;                    /* index of first node in FAT chain */
    int       unused;                   /* unused (padding for alignment) */
} getfs_nt_Node;

typedef struct getfs_nt_NT {
    getfs_iio_File*  file;                /* IIO file */
    int            channel;             /* channel within the IIO file */
    int            unallocated;         /* index of first node in free chain */
} getfs_nt_NT;



// Open the nodeTable stored in channel 'channel' of the given IIO file
getfs_nt_NT* getfs_nt_open( getfs_iio_File* _file, int _channel );
// Create a new nodeTable in the given IIO file, using the given nr. of blocks per channel
getfs_nt_NT* getfs_nt_create( getfs_iio_File* _file, int _blocks_per_channel );
// Allocate a new node, return the index
int          getfs_nt_allocate_node( getfs_nt_NT* _nt );
// Get the information in node 'node'
int          getfs_nt_get_node( getfs_nt_NT* _nt, int _node, getfs_nt_Node* _node_ptr );
// Update the given node with the information from the given nfs_nt_Node
int          getfs_nt_set_node( getfs_nt_NT* _nt, int _node, getfs_nt_Node* _node_info );
// Set the new size of data associated with a node
void         getfs_nt_node_set_size( getfs_nt_NT* _nt, int _node, int _size );
// Get the size of the data associated with a node
int          getfs_nt_node_get_size( getfs_nt_NT* _nt, int _node );
// Close the nodeTable
void         getfs_nt_close( getfs_nt_NT* _nt );
// Destroy the nodeTable
void         getfs_nt_destroy( getfs_nt_NT* _nt );
// Get the chain associated with a node 
int          getfs_nt_node_get_chain( getfs_nt_NT* _nt, int _node );
// Decrement refcount, if refcount=0 delete node and ret true, otherwise return false 
int          getfs_nt_refcount_decr( getfs_nt_NT* _nt, int _node );

#endif //_GETFS_NODETABLE_H_