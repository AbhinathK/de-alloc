#define M61_DISABLE 1
#include "m61.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

//global structure to initilize stats
struct m61_statistics current_stats = {
    .nactive = 0,        //total number of active allocations
    .active_size = 0,   //size of total active allocations in bytes
    .ntotal = 0,       //total number of allcations
    .total_size = 0,  //size of total allocations in bytes
    .nfail = 0,      //total number of failed allocation attempts
    .fail_size = 0, // size of total faild allocations in bytes
    
    .heap_min = (char *) 0xffffffff ,
    .heap_max = (char *) 0x00
};

//this structure hold meta-data
//related to allocation information, size etc
#define ALIGNMENT 8
struct meta {
    size_t block_size;
};
size_t meta_header_padding = (sizeof(struct meta) % ALIGNMENT) ? (sizeof(struct meta) + ALIGNMENT) - (sizeof(struct meta) % ALIGNMENT) : sizeof(struct meta);




//global variable to keeps track of number of free(ptr).
unsigned long long total_free = 0;

//keeps updating the global current_stats structure, 
//called whenever memory is allocated and deallocated.
void update_active_allocations() {
    //updates the active number of allocations
    current_stats.nactive = current_stats.ntotal - total_free;
}
//keeps track of smallest and largest heap address seen so far, 
//called whenever memory is allocated, 
//make comparision b/w addresses
void update_heap_address(char *input_address, size_t sz) {
    if(input_address < current_stats.heap_min)  
    {
        current_stats.heap_min = input_address;
    }
    if(input_address > current_stats.heap_max)
    {
        current_stats.heap_max = input_address + sz;
    }
}

/// m61_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc may
///    either return NULL or a unique, newly-allocated pointer value.
///    The allocation request was at location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    void *starting_address = base_malloc(sz + meta_header_padding); // also allocate space for meta data.
    struct meta *meta_data_ptr = starting_address; 
    if(starting_address == NULL || sz + meta_header_padding < sz ) // memory allocation failed.
    {
        //updates the total number of failed memory allocation attempts.
        current_stats.nfail += 1;
        //updates the size of total failed allocations
        current_stats.fail_size += sz;
        return NULL; //small memory may allocate due to overflow, 
    }
    else 
    {
       // struct meta *meta_data_ptr = starting_address;
        meta_data_ptr->block_size = sz;
        current_stats.ntotal += 1; // updates every allocation, keeps track of total number of allocations.
        update_active_allocations(); //updates the current_stats, because more memory is allocated.
        current_stats.total_size += sz; // updates total bytes allocated so far. 
        update_heap_address((char*)starting_address + meta_header_padding, sz); // updates heap address space seen so far.
        current_stats.active_size += sz;
    }  
    void *payload = starting_address + meta_header_padding;
    return payload;
}


/// m61_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`, which must have been
///    returned by a previous call to m61_malloc and friends. If
///    `ptr == NULL`, does nothing. The free was called at location
///    `file`:`line`.

void m61_free(void *ptr, const char *file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    void *memory = ptr - meta_header_padding;
    struct meta *meta_data_ptr = (struct meta *) memory; //computing ptr to meta data
    if(!ptr)
        return;
    current_stats.active_size -= meta_data_ptr->block_size; // extracting allocation size from meta data, updating active size
    base_free(memory);
    total_free += 1; //updates the total number of free(ptr) so far. 
    update_active_allocations(); //this changes because memory is being released. 
}


/// m61_realloc(ptr, sz, file, line)
///    Reallocate the dynamic memory pointed to by `ptr` to hold at least
///    `sz` bytes, returning a pointer to the new block. If `ptr` is NULL,
///    behaves like `m61_malloc(sz, file, line)`. If `sz` is 0, behaves
///    like `m61_free(ptr, file, line)`. The allocation request was at
///    location `file`:`line`.

void* m61_realloc(void* ptr, size_t sz, const char* file, int line) {
    void* new_ptr = NULL;
    if (sz)
        new_ptr = m61_malloc(sz, file, line);
    if (ptr != NULL && new_ptr != NULL) {
        // Copy the data from `ptr` into `new_ptr`.
        // To do that, we must figure out the size of allocation `ptr`.
        // Your code here (to fix test012).
        void *memory = ptr - meta_header_padding;
        struct meta *meta_data_ptr = (struct meta *) memory;
        memcpy(new_ptr, ptr, meta_data_ptr->block_size);
    }
    if(ptr)
        m61_free(ptr, file, line);
    return new_ptr;
}


/// m61_calloc(nmemb, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `nmemb` elements of `sz` bytes each. The memory
///    is initialized to zero. If `sz == 0`, then m61_malloc may
///    either return NULL or a unique, newly-allocated pointer value.
///    The allocation request was at location `file`:`line`.

void* m61_calloc(size_t nmemb, size_t sz, const char* file, int line) {
    //avoid multiplication overflow
    int size = nmemb * sz;
    if(size < nmemb) { //failed due to overflow of multiplication
        current_stats.nfail += 1; //updating the failed allocation attemps
        return NULL;
    }
    void* ptr = m61_malloc(size, file, line);
    if (ptr)
        memset(ptr, 0, nmemb * sz);
    return ptr;
}


/// m61_getstatistics(stats)
///    Store the current memory statistics in `*stats`.

void m61_getstatistics(struct m61_statistics* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(struct m61_statistics));
    *stats = current_stats; // initilize global stats pointer to current stats information
}


/// m61_printstatistics()
///    Print the current memory statistics.

void m61_printstatistics(void) {
    struct m61_statistics stats;
    m61_getstatistics(&stats);

    printf("malloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("malloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}


/// m61_printleakreport()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.

void m61_printleakreport(void) {
    // Your code here.
}
