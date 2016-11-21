/*
 * cirbuff.h - 环形缓冲区的实现的外部声明
 *
 * 采用记录读写位置的方法实现，没有做线程安全处理。
 */

#include <stdint.h>

typedef void *  cirbuff_handle;



// 对循环缓冲区的操作
cirbuff_handle cirbuff_creat(uint32_t size);
uint32_t cirbuff_write(cirbuff_handle cbuff_h,
        uint8_t *indata, uint32_t inlen, uint8_t be_cover);
uint32_t cirbuff_read(cirbuff_handle cbuff_h,
        uint8_t *outdata, uint32_t outlen, uint8_t beclear);
void cirbuff_revert(cirbuff_handle cbuff_h);
void cirbuff_destroy(cirbuff_handle cbuff_h);


// 对循环缓冲区的查询
uint32_t cirbuff_be_empty(cirbuff_handle cbuff_h);
uint32_t cirbuff_be_full(cirbuff_handle cbuff_h);
uint32_t cirbuff_data_num(cirbuff_handle cbuff_h);
uint32_t cirbuff_free_num(cirbuff_handle cbuff_h);
uint32_t cirbuff_be_update(cirbuff_handle cbuff_h);

