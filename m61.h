#ifndef M61_H
#define M61_H 1
#include <stdlib.h>
#include <inttypes.h>

//function prototypes
void* m61_malloc(size_t sz, const char* file, int line);
void m61_free(void* ptr, const char* file, int line);
void* m61_realloc(void* ptr, size_t sz, const char* file, int line);
void* m61_calloc(size_t nmemb, size_t sz, const char* file, int line);

//for general memory allocation tracking 
struct m61_statistics {
    unsigned long long nactive;         // # active allocations
    unsigned long long active_size;     // # bytes in active allocations
    unsigned long long ntotal;          // # total allocations
    unsigned long long total_size;      // # bytes in total allocations
    unsigned long long nfail;           // # failed allocation attempts
    unsigned long long fail_size;       // # bytes in failed alloc attempts
    char* heap_min;                     // smallest allocated addr
    char* heap_max;                     // largest allocated addr
};

//for memory allocation profiler (i.e. heavy hitter)
//16-Byte per file-line pair overhead added for each node
struct heavy_hitter_metadata {
    unsigned long long size;
    const char *file;
    int line;
    
    struct heavy_hitter_metadata *next;
};

//prototypes for memory debugger & statistics tracking
void m61_getstatistics(struct m61_statistics* stats);
void m61_printstatistics(void);
void m61_printleakreport(void);

//prototypes for heavy hitter detector
void m61_updateheavyhitter(unsigned long long, const char *, int);
void m61_printheavyhitterreport(void);
void m61_cleanheavyhittermetadata(void);



#if !M61_DISABLE
#define malloc(sz)              m61_malloc((sz), __FILE__, __LINE__)
#define free(ptr)               m61_free((ptr), __FILE__, __LINE__)
#define realloc(ptr, sz)        m61_realloc((ptr), (sz), __FILE__, __LINE__)
#define calloc(nmemb, sz)       m61_calloc((nmemb), (sz), __FILE__, __LINE__)
#endif

void* base_malloc(size_t sz);
void base_free(void* ptr);
void base_disablealloc(int is_disabled);

#endif
