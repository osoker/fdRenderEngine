//File Allocation Table (FAT)
#ifndef _GETFS_FAT_H_
#define _GETFS_FAT_H_
#include "getfs_iio.h"

#define GETFS_FAT_DEF_CHANNEL_SIZE    4       /* Default channel size in blocks ("bandwidth") */
#define GETFS_FAT_EOC                 -1      /* End of chain marker */

typedef struct getfs_fat_FAT {
    getfs_iio_File*   file;                /* IIO file */
    int             channel;             /* alloted channel in IIO file */
    int             unallocated;         /* offs of first entry in the chain of unallocated entries */
} getfs_fat_FAT;


// Opens an existing FAT and initializes it with the information found in the given channel
getfs_fat_FAT* getfs_fat_open( getfs_iio_File* _file, int _channel_no );
// Creates a FAT using a new channel in the given IIO file
getfs_fat_FAT* getfs_fat_create( getfs_iio_File* _file, int _blocks_per_channel );
// Allocates a new 1-node chain and returns the index of the first (only) node in the chain
int            getfs_fat_create_chain( getfs_fat_FAT* _fat );
// Get the index of the 'n'-th node in the chain, given the start node
// the "0th" node is always the start node
int            getfs_fat_chain_get_nth( getfs_fat_FAT* _fat, int _first_node_index, int _n );
// Truncates the chain starting at the given node
int            getfs_fat_chain_truncate( getfs_fat_FAT* _fat, int _first );
// Allocates a new node for the given chain and appends it at the end. Returns the node index.
int            getfs_fat_chain_extend( getfs_fat_FAT* _fat, int _first_node_index );
// Closes the given FAT
int            getfs_fat_close( getfs_fat_FAT* _fat );
// Destroys the chain given by the index of the first node in the chain
int            getfs_fat_destroy_chain( getfs_fat_FAT* _fat, int _first_node_index );

#endif //_GETFS_FAT_H_
