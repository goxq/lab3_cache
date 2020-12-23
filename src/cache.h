#include "shell.h"
#include <string.h>



typedef struct 
{
    uint32_t tag;//低21位为tag
    int valid;
    int lru;//lru计数
    uint32_t data[8];//数据区域，数组中每个四个字节，共八个
} instructionCacheLine;

typedef struct 
{
    instructionCacheLine lines[4];
} iCacheGroup;

typedef struct {
    iCacheGroup cacheGroups[64];
    
} instuctionCache;



typedef struct 
{
    uint32_t tag;//低19位为tag
    int valid;
    int lru;//lru计数
    int dirty;
    uint32_t data[8];//数据区域，数组中每个四个字节，共八个
} dataCacheLine;

typedef struct 
{
    dataCacheLine lines[8];
} dCacheGroup;

typedef struct {
    dCacheGroup cacheGroups[256];
} dataCache;



//uint32_t instru_cache_read_32(uint32_t address);

// uint32_t dCache_read_miss_todo(uint32_t address);
// uint32_t is_dCache_read_hit(uint32_t address);
// void dCache_write_miss_todo(uint32_t address, uint32_t value);
// uint32_t is_dCache_write_hit(uint32_t address, uint32_t value);

uint32_t is_instru_cache_hit(uint32_t address);
uint32_t instru_cache_miss_todo(uint32_t address);
uint32_t data_cache_read_32(uint32_t address);
int data_cache_write_32(uint32_t address, uint32_t value);