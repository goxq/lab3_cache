#include "cache.h"
#include "pipe.h"
#include "shell.h"
#include "mips.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

instuctionCache iCache;
dataCache dCache;

int data_cache_write_mem_count = 50;
int data_cache_read_mem_count = 50;

int instru_cache_count = 0;
int data_cache_count = 0;

uint32_t instru_cache_read_32(uint32_t address)
{
    // printf("instru_cache_count:%d\n", instru_cache_count);
    // printf("address:%08x\n", address);

    if (instru_cache_count == 0)
    {
        memset(&iCache, 0, sizeof(instuctionCache));
        instru_cache_count++;
    }

    uint8_t cacheGroupNum = (address << 21) >> 26; //得到6位cache组号

    //printf("十进制cacheGroupNum:%d\n", cacheGroupNum);
    int i;
    for (i = 0; i < 4; i++)
    {
        uint32_t tag = iCache.cacheGroups[cacheGroupNum].lines[i].tag;

        if (!(tag ^ (address >> 11))) //tag位相同
        {

            if (iCache.cacheGroups[cacheGroupNum].lines[i].valid == 1) //有效位为1,hit
            {

                //看lru位
                if (iCache.cacheGroups[cacheGroupNum].lines[i].lru < 3)
                    iCache.cacheGroups[cacheGroupNum].lines[i].lru++;

                return iCache.cacheGroups[cacheGroupNum].lines[i].data[(address & 0x0000001f) >> 2]; //aaddress & 0x0000001f为address后五位，块内偏移量.右移两位缩小四倍
            }
        }
    }


    //miss操作
    uint32_t firstAddrOfTheBlock = address & 0xffffffe0;
    uint32_t mem0 = mem_read_32(firstAddrOfTheBlock);
    uint32_t mem1 = mem_read_32(firstAddrOfTheBlock + 4);
    uint32_t mem2 = mem_read_32(firstAddrOfTheBlock + 8);
    uint32_t mem3 = mem_read_32(firstAddrOfTheBlock + 12);
    uint32_t mem4 = mem_read_32(firstAddrOfTheBlock + 16);
    uint32_t mem5 = mem_read_32(firstAddrOfTheBlock + 20);
    uint32_t mem6 = mem_read_32(firstAddrOfTheBlock + 24);
    uint32_t mem7 = mem_read_32(firstAddrOfTheBlock + 28);


    //找空行
    int k;
    for (k = 0; k < 4; k++)
    {
        if (iCache.cacheGroups[cacheGroupNum].lines[k].valid = 0)
        {
            
            iCache.cacheGroups[cacheGroupNum].lines[k].lru = 0; //lru位置为0
            iCache.cacheGroups[cacheGroupNum].lines[k].tag = address >> 11;
            iCache.cacheGroups[cacheGroupNum].lines[k].valid = 1;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[0] = mem0;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[1] = mem1;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[2] = mem2;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[3] = mem3;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[4] = mem4;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[5] = mem5;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[6] = mem6;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[7] = mem7;

            return iCache.cacheGroups[cacheGroupNum].lines[k].data[(address & 0x0000001f) >> 2];
        }
    }

    //替换lru最小的
    int j;
    int minLruCountLine = 0;
    int minLruCount = iCache.cacheGroups[cacheGroupNum].lines[0].lru;
    for (j = 0; j < 4; j++)
    {
        int lruCount = iCache.cacheGroups[cacheGroupNum].lines[j].lru;
        if (lruCount < minLruCount)
        {
            minLruCount = lruCount;
            minLruCountLine = j;
        }
    }
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].lru = 0; //lru位置为0
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag = address >> 11;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].valid = 1;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0] = mem0;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1] = mem1;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2] = mem2;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3] = mem3;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4] = mem4;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5] = mem5;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6] = mem6;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7] = mem7;

    return iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[(address & 0x0000001f) >> 2];
}

uint32_t data_cache_read_32(uint32_t address)
{
    printf("address1:%x\n", address);
    //初始化
    if (data_cache_count == 0)
    {
        memset(&dCache, 0, sizeof(dataCache));
    }
    data_cache_count++;

    uint32_t cacheGroupNum = (address << 19) >> 24; //得到组号
    //printf("fistCacheGroupNum:%d\n", cacheGroupNum);
    //printf("data_cache_count:%d,  dCache.cacheGroups[cacheGroupNum].lines[0].valid:%d, lines[2].valid:%d\n", data_cache_count, dCache.cacheGroups[cacheGroupNum].lines[0].valid, dCache.cacheGroups[cacheGroupNum].lines[2].valid);

    int i;
    for (i = 0; i < 8; i++)
    {
        uint32_t tag = dCache.cacheGroups[cacheGroupNum].lines[i].tag;
        //printf("tag:%x,dCache.cacheGroups[cacheGroupNum].lines[i].tag:%x\n", tag, dCache.cacheGroups[cacheGroupNum].lines[i].tag);
        if (!(tag ^ (address >> 13))) //tag位相同
        {
            if (dCache.cacheGroups[cacheGroupNum].lines[i].valid == 1) //有效位为一，hit
            {
                //printf("hit!!,cacheGroupNum = %d,line = %d,address:%x,tag:%x,dCache.cacheGroups[cacheGroupNum].lines[i].tag:%x\n", cacheGroupNum, i, address, tag, dCache.cacheGroups[cacheGroupNum].lines[i].tag);
                //看lru位
                if (dCache.cacheGroups[cacheGroupNum].lines[i].lru < 7)
                    dCache.cacheGroups[cacheGroupNum].lines[i].lru++;
                return dCache.cacheGroups[cacheGroupNum].lines[i].data[(address & 0x0000001f) >> 2]; //aaddress & 0x0000001f为address后五位，块内偏移量.右移两位缩小四倍
            }
        }
        //printf("i:%d\n", i);
    }
    //printf("miss!\n");
    //printf("secondCacheGroupNum:%d\n", cacheGroupNum);

    if(data_cache_read_mem_count!=0){
        //printf("data_cache_read_mem_count:%d\n",data_cache_read_mem_count);
        data_cache_read_mem_count --;
        return 0x0381CD55;
    }
    printf("address2:%x\n",address);
    //miss操作
    uint32_t firstAddrOfTheBlock = address & 0xffffffe0;
    uint32_t mem0 = mem_read_32(firstAddrOfTheBlock);
    uint32_t mem1 = mem_read_32(firstAddrOfTheBlock + 4);
    uint32_t mem2 = mem_read_32(firstAddrOfTheBlock + 8);
    uint32_t mem3 = mem_read_32(firstAddrOfTheBlock + 12);
    uint32_t mem4 = mem_read_32(firstAddrOfTheBlock + 16);
    uint32_t mem5 = mem_read_32(firstAddrOfTheBlock + 20);
    uint32_t mem6 = mem_read_32(firstAddrOfTheBlock + 24);
    uint32_t mem7 = mem_read_32(firstAddrOfTheBlock + 28);
    //printf("thirdCacheGroupNum:%d\n", cacheGroupNum);
    //找空行
    int k;
    for (k = 0; k < 8; k++)
    {
        //printf("for cacheGroupNum:%d\n", cacheGroupNum);
        //printf("address:%08x,tag:%08x,cacheGroupNum:%d,valid:%d\n", address, dCache.cacheGroups[cacheGroupNum].lines[k].tag, cacheGroupNum, dCache.cacheGroups[cacheGroupNum].lines[k].valid);
        if (dCache.cacheGroups[cacheGroupNum].lines[k].valid == 0)
        {

            dCache.cacheGroups[cacheGroupNum].lines[k].lru = 0; //lru位置为0
            dCache.cacheGroups[cacheGroupNum].lines[k].dirty = 0;
            dCache.cacheGroups[cacheGroupNum].lines[k].tag = address >> 13;

            dCache.cacheGroups[cacheGroupNum].lines[k].valid = 1;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[0] = mem0;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[1] = mem1;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[2] = mem2;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[3] = mem3;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[4] = mem4;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[5] = mem5;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[6] = mem6;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[7] = mem7;

            data_cache_read_mem_count = 50;
            return dCache.cacheGroups[cacheGroupNum].lines[k].data[(address & 0x0000001f) >> 2];
        }
    }
    //找lru最小行
    int j;
    int minLruCountLine = 0;
    int minLruCount = dCache.cacheGroups[cacheGroupNum].lines[0].lru;
    for (j = 1; j < 8; j++)
    {
        int lruCount = dCache.cacheGroups[cacheGroupNum].lines[j].lru;
        if (lruCount < minLruCount)
        {
            minLruCount = lruCount;
            minLruCountLine = j;
        }
    }

    //根据找到的被替换行的脏位决定是否将其数据写回memory

    if (dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].dirty == 1)
    {
        //脏位为1，要写回内存
        uint32_t cacheGroupNum_32 = cacheGroupNum;
        cacheGroupNum_32 << 5; //其低5位为0，中间8位为组号，高位为0
        uint32_t firstAddrToWriteBack = (dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag << 13) | cacheGroupNum_32;
        mem_write_32(firstAddrToWriteBack, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0]);
        mem_write_32(firstAddrToWriteBack + 4, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1]);
        mem_write_32(firstAddrToWriteBack + 8, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2]);
        mem_write_32(firstAddrToWriteBack + 12, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3]);
        mem_write_32(firstAddrToWriteBack + 16, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4]);
        mem_write_32(firstAddrToWriteBack + 20, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5]);
        mem_write_32(firstAddrToWriteBack + 24, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6]);
        mem_write_32(firstAddrToWriteBack + 28, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7]);
    }

    //内存写到cache，并返回所要
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].lru = 0; //lru位置为0
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag = address >> 13;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].valid = 1;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].dirty = 0;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0] = mem0;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1] = mem1;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2] = mem2;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3] = mem3;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4] = mem4;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5] = mem5;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6] = mem6;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7] = mem7;

    data_cache_read_mem_count = 50;
    return dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[(address & 0x0000001f) >> 2];
}

int data_cache_write_32(uint32_t address, uint32_t value)
{
    //初始化
    if (data_cache_count == 0)
    {
        memset(&dCache, 0, sizeof(dataCache));
        data_cache_count++;
    }
    uint32_t cacheGroupNum = (address << 19) >> 24; //得到组号
    int i;
    for (i = 0; i < 8; i++)
    {
        uint32_t tag = dCache.cacheGroups[cacheGroupNum].lines[i].tag;
        if (!(tag ^ (address >> 13))) //tag位相同
        {
            if (dCache.cacheGroups[cacheGroupNum].lines[i].valid == 1) //有效位为一，hit
            {
                //看lru位
                if (dCache.cacheGroups[cacheGroupNum].lines[i].lru < 7)
                    dCache.cacheGroups[cacheGroupNum].lines[i].lru++;
                dCache.cacheGroups[cacheGroupNum].lines[i].valid = 1;
                dCache.cacheGroups[cacheGroupNum].lines[i].dirty = 1;
                dCache.cacheGroups[cacheGroupNum].lines[i].data[(address & 0x0000001f) >> 2] = value;
                return 1;
            }
        }
    }

    //没有命中
    //主存调到cache中

    if(data_cache_write_mem_count!=0){
        data_cache_write_mem_count--;
        return 0;
    }

    uint32_t firstAddrOfTheBlock = address & 0xffffffe0;
    uint32_t mem0 = mem_read_32(firstAddrOfTheBlock);
    uint32_t mem1 = mem_read_32(firstAddrOfTheBlock + 4);
    uint32_t mem2 = mem_read_32(firstAddrOfTheBlock + 8);
    uint32_t mem3 = mem_read_32(firstAddrOfTheBlock + 12);
    uint32_t mem4 = mem_read_32(firstAddrOfTheBlock + 16);
    uint32_t mem5 = mem_read_32(firstAddrOfTheBlock + 20);
    uint32_t mem6 = mem_read_32(firstAddrOfTheBlock + 24);
    uint32_t mem7 = mem_read_32(firstAddrOfTheBlock + 28);

    //找空行
    int k;
    for (k = 0; k < 8; k++)
    {
        if (dCache.cacheGroups[cacheGroupNum].lines[k].valid == 0)
        {
            dCache.cacheGroups[cacheGroupNum].lines[k].valid = 1;
            dCache.cacheGroups[cacheGroupNum].lines[k].lru = 0;
            dCache.cacheGroups[cacheGroupNum].lines[k].dirty = 1; //由于要更改，脏位为1
            dCache.cacheGroups[cacheGroupNum].lines[k].tag = address >> 13;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[0] = mem0;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[1] = mem1;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[2] = mem2;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[3] = mem3;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[4] = mem4;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[5] = mem5;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[6] = mem6;
            dCache.cacheGroups[cacheGroupNum].lines[k].data[7] = mem7;
            //更改数据
            dCache.cacheGroups[cacheGroupNum].lines[k].data[(address & 0x0000001f) >> 2] = value;
            data_cache_write_mem_count = 50;
            return 1;
        }
    }

    //没有空行，找lru最小行
    int j;
    int minLruCountLine = 0;
    int minLruCount = dCache.cacheGroups[cacheGroupNum].lines[0].lru;
    for (j = 0; j < 8; j++)
    {
        int lruCount = dCache.cacheGroups[cacheGroupNum].lines[j].lru;
        if (lruCount < minLruCount)
        {
            minLruCount = lruCount;
            minLruCountLine = j;
        }
    }

    //根据找到的被替换行的脏位决定是否将其数据写回memory
    if (dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].dirty == 1)
    {
        //脏位为1，要写回内存
        uint32_t cacheGroupNum_32 = cacheGroupNum;
        cacheGroupNum_32 << 5; //其低5位为0，中间8位为组号，高位为0
        uint32_t firstAddrToWriteBack = (dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag << 13) | cacheGroupNum_32;
        mem_write_32(firstAddrToWriteBack, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0]);
        mem_write_32(firstAddrToWriteBack + 4, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1]);
        mem_write_32(firstAddrToWriteBack + 8, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2]);
        mem_write_32(firstAddrToWriteBack + 12, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3]);
        mem_write_32(firstAddrToWriteBack + 16, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4]);
        mem_write_32(firstAddrToWriteBack + 20, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5]);
        mem_write_32(firstAddrToWriteBack + 24, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6]);
        mem_write_32(firstAddrToWriteBack + 28, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7]);
    }

    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0] = mem0;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1] = mem1;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2] = mem2;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3] = mem3;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4] = mem4;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5] = mem5;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6] = mem6;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7] = mem7;

    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].lru = 0; //lru位置为0
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag = address >> 13;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].valid = 1;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].dirty = 1; //脏位为1
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[(address & 0x0000001f) >> 2] = value;
    
    data_cache_write_mem_count = 50;
    return 1;
}

//在需要读数据时，判断是否命中？命中则返回数据，不命中返回全0
uint32_t is_dCache_read_hit(uint32_t address)
{
    //初始化
    if (data_cache_count == 0)
    {
        memset(&dCache, 0, sizeof(dataCache));
        data_cache_count++;
    }

    uint8_t cacheGroupNum = (address << 19) >> 24; //得到组号
    int i;
    for (i = 0; i < 8; i++)
    {
        uint32_t tag = dCache.cacheGroups[cacheGroupNum].lines[i].tag;
        if (!(tag ^ (address >> 13))) //tag位相同
        {
            if (dCache.cacheGroups[cacheGroupNum].lines[i].valid == 1) //有效位为一，hit
            {
                //看lru位
                if (dCache.cacheGroups[cacheGroupNum].lines[i].lru < 7)
                    dCache.cacheGroups[cacheGroupNum].lines[i].lru++;
                return dCache.cacheGroups[cacheGroupNum].lines[i].data[(address & 0x0000001f) >> 2]; //aaddress & 0x0000001f为address后五位，块内偏移量.右移两位缩小四倍
            }
        }
    }
    return 0;
}

//命中直接改，返回1；miss返回0
uint32_t is_dCache_write_hit(uint32_t address, uint32_t value)
{
    //初始化
    if (data_cache_count == 0)
    {
        memset(&dCache, 0, sizeof(dataCache));
        data_cache_count++;
    }
    uint8_t cacheGroupNum = (address << 19) >> 24; //得到组号
    int i;
    for (i = 0; i < 8; i++)
    {
        uint32_t tag = dCache.cacheGroups[cacheGroupNum].lines[i].tag;
        if (!(tag ^ (address >> 13))) //tag位相同,命中，直接改数据
        {
            //看lru位
            if (dCache.cacheGroups[cacheGroupNum].lines[i].lru < 7)
                dCache.cacheGroups[cacheGroupNum].lines[i].lru++;

            dCache.cacheGroups[cacheGroupNum].lines[i].valid = 1;
            dCache.cacheGroups[cacheGroupNum].lines[i].dirty = 1;
            dCache.cacheGroups[cacheGroupNum].lines[i].data[(address & 0x0000001f) >> 2] = value;
            return 1;
        }
    }
    return 0;
}

//dCache read miss的操作
uint32_t dCache_read_miss_todo(uint32_t address)
{
    uint8_t cacheGroupNum = (address << 19) >> 24; //得到组号

    //miss操作
    uint32_t firstAddrOfTheBlock = address & 0xffffffe0;
    uint32_t mem0 = mem_read_32(firstAddrOfTheBlock);
    uint32_t mem1 = mem_read_32(firstAddrOfTheBlock + 4);
    uint32_t mem2 = mem_read_32(firstAddrOfTheBlock + 8);
    uint32_t mem3 = mem_read_32(firstAddrOfTheBlock + 12);
    uint32_t mem4 = mem_read_32(firstAddrOfTheBlock + 16);
    uint32_t mem5 = mem_read_32(firstAddrOfTheBlock + 20);
    uint32_t mem6 = mem_read_32(firstAddrOfTheBlock + 24);
    uint32_t mem7 = mem_read_32(firstAddrOfTheBlock + 28);

    //找lru最小行
    int j;
    int minLruCountLine = 0;
    int minLruCount = dCache.cacheGroups[cacheGroupNum].lines[0].lru;
    for (j = 0; j < 8; j++)
    {
        int lruCount = dCache.cacheGroups[cacheGroupNum].lines[j].lru;
        if (lruCount < minLruCount)
        {
            minLruCount = lruCount;
            minLruCountLine = j;
        }
    }

    //根据找到的被替换行的脏位决定是否将其数据写回memory

    if (dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].dirty == 1)
    {
        //脏位为1，要写回内存
        uint32_t cacheGroupNum_32 = cacheGroupNum;
        cacheGroupNum_32 << 5; //其低5位为0，中间8位为组号，高位为0
        uint32_t firstAddrToWriteBack = (dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag << 13) | cacheGroupNum_32;
        mem_write_32(firstAddrToWriteBack, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0]);
        mem_write_32(firstAddrToWriteBack + 4, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1]);
        mem_write_32(firstAddrToWriteBack + 8, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2]);
        mem_write_32(firstAddrToWriteBack + 12, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3]);
        mem_write_32(firstAddrToWriteBack + 16, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4]);
        mem_write_32(firstAddrToWriteBack + 20, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5]);
        mem_write_32(firstAddrToWriteBack + 24, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6]);
        mem_write_32(firstAddrToWriteBack + 28, dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7]);
    }

    //内存写到cache，并返回所要
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].lru = 0; //lru位置为0
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag = address >> 13;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].valid = 1;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].dirty = 0;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0] = mem0;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1] = mem1;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2] = mem2;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3] = mem3;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4] = mem4;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5] = mem5;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6] = mem6;
    dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7] = mem7;

    return dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[(address & 0x0000001f) >> 2];
}

void dCache_write_miss_todo(uint32_t address, uint32_t value)
{
    uint8_t cacheGroupNum = (address << 19) >> 24; //得到组号

    //找lru最小行
    int j;
    int minLruCountLine = 0;
    int minLruCount = dCache.cacheGroups[cacheGroupNum].lines[0].lru;
    for (j = 0; j < 8; j++)
    {
        int lruCount = dCache.cacheGroups[cacheGroupNum].lines[j].lru;
        if (lruCount < minLruCount)
        {
            minLruCount = lruCount;
            minLruCountLine = j;
        }
    }
    dataCacheLine theReplacedCacheLine = dCache.cacheGroups[cacheGroupNum].lines[minLruCountLine];
    theReplacedCacheLine.lru = 0; //lru位置为0
    theReplacedCacheLine.tag = address >> 13;
    theReplacedCacheLine.valid = 1;
    theReplacedCacheLine.dirty = 1; //脏位置为1
    theReplacedCacheLine.data[(address & 0x0000001f) >> 2] = value;
}

//是否命中指令cache，命中返回数据，不命中返回0
uint32_t is_instru_cache_hit(uint32_t address)
{
    if (instru_cache_count == 0)
    {
        memset(&iCache, 0, sizeof(instuctionCache));
        instru_cache_count++;
    }

    uint32_t cacheGroupNum = (address << 21) >> 26; //得到6位cache组号

    int i;
    for (i = 0; i < 4; i++)
    {
        uint32_t tag = iCache.cacheGroups[cacheGroupNum].lines[i].tag;

        if (!(tag ^ (address >> 11))) //tag位相同
        {
            if (iCache.cacheGroups[cacheGroupNum].lines[i].valid == 1) //有效位为1,hit
            {
                //看lru位
                if (iCache.cacheGroups[cacheGroupNum].lines[i].lru < 3)
                    iCache.cacheGroups[cacheGroupNum].lines[i].lru++;

                return iCache.cacheGroups[cacheGroupNum].lines[i].data[(address & 0x0000001f) >> 2]; //aaddress & 0x0000001f为address后五位，块内偏移量.右移两位缩小四倍
            }
        }
    }
    return 0x0381CD55;
}

//指令cache缺失替换操作
uint32_t instru_cache_miss_todo(uint32_t address)
{
    uint32_t cacheGroupNum = (address << 21) >> 26; //得到6位cache组号
    //miss操作
    uint32_t firstAddrOfTheBlock = address & 0xffffffe0;
    uint32_t mem0 = mem_read_32(firstAddrOfTheBlock);
    uint32_t mem1 = mem_read_32(firstAddrOfTheBlock + 4);
    uint32_t mem2 = mem_read_32(firstAddrOfTheBlock + 8);
    uint32_t mem3 = mem_read_32(firstAddrOfTheBlock + 12);
    uint32_t mem4 = mem_read_32(firstAddrOfTheBlock + 16);
    uint32_t mem5 = mem_read_32(firstAddrOfTheBlock + 20);
    uint32_t mem6 = mem_read_32(firstAddrOfTheBlock + 24);
    uint32_t mem7 = mem_read_32(firstAddrOfTheBlock + 28);

    //找空行
    int k;
    for (k = 0; k < 4; k++)
    {
        if (iCache.cacheGroups[cacheGroupNum].lines[k].valid = 0)
        {
            
            iCache.cacheGroups[cacheGroupNum].lines[k].lru = 0; //lru位置为0
            iCache.cacheGroups[cacheGroupNum].lines[k].tag = address >> 11;
            iCache.cacheGroups[cacheGroupNum].lines[k].valid = 1;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[0] = mem0;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[1] = mem1;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[2] = mem2;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[3] = mem3;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[4] = mem4;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[5] = mem5;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[6] = mem6;
            iCache.cacheGroups[cacheGroupNum].lines[k].data[7] = mem7;

            return iCache.cacheGroups[cacheGroupNum].lines[k].data[(address & 0x0000001f) >> 2];
        }
    }

    //替换lru最小的
    int j;
    int minLruCountLine = 0;
    int minLruCount = iCache.cacheGroups[cacheGroupNum].lines[0].lru;
    for (j = 1; j < 4; j++)
    {
        int lruCount = iCache.cacheGroups[cacheGroupNum].lines[j].lru;
        if (lruCount < minLruCount)
        {
            minLruCount = lruCount;
            minLruCountLine = j;
        }
    }

    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].lru = 0; //lru位置为0
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].tag = address >> 11;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].valid = 1;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[0] = mem0;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[1] = mem1;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[2] = mem2;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[3] = mem3;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[4] = mem4;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[5] = mem5;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[6] = mem6;
    iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[7] = mem7;

    return iCache.cacheGroups[cacheGroupNum].lines[minLruCountLine].data[(address & 0x0000001f) >> 2];
}
