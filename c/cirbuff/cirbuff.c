/*
 * cirbuff.c - 环形缓冲区的实现
 *
 * 采用记录读写位置的方法实现，没有做线程安全处理。
 */


#include <stdlib.h>

#include "cirbuff.h"

struct cirbuff
{
    uint8_t *buff;
    uint32_t rpos;
    uint32_t wpos;
    uint32_t size;
    uint32_t cnt;
    uint32_t update;
};



// 静态函数声明
static uint32_t cirbuff_cover_write(cirbuff_handle cbuff_h,
        uint8_t *indata, uint32_t inlen);
static uint32_t cirbuff_nocover_write(cirbuff_handle cbuff_h,
        uint8_t *indata, uint32_t inlen);
static uint32_t cirbuff_clear_read(cirbuff_handle cbuff_h,
        uint8_t *outdata, uint32_t outlen);
static uint32_t cirbuff_noclear_read(cirbuff_handle cbuff_h,
        uint8_t *outdata, uint32_t outlen);



// 以下函数为对循环缓冲区的操作


/*
 * cirbuff_creat - 创建一个环形缓冲区
 *
 * 所做的包括分配空间和初始变量赋值
 */
cirbuff_handle cirbuff_creat(uint32_t size)
{
    struct cirbuff *cbuff = NULL;
    int32_t rlt = 0;

    if (size == 0)
        rlt = -1;
    else
        rlt = 1;

    // 为cbuff分配空间
    if (rlt)
    {
        cbuff = (struct cirbuff *)malloc(sizeof(*cbuff));
        if (cbuff == NULL)
            rlt = -2;
    }

    // 为cbuff->buff分配空间
    if (rlt)
    {
        cbuff->buff = (uint8_t *)malloc(size);
        if (cbuff->buff == NULL)
        {
            free(cbuff);
            cbuff = NULL;
            rlt = -2;
        }
    }

    if (rlt)
    {
        cbuff->rpos = 0;
        cbuff->wpos = 0;
        cbuff->size = size;
        cbuff->cnt = 0;
        cbuff->update = 0;
    }

    return (cirbuff_handle)cbuff;
}

/*
 * cirbuff_write - 往环形缓冲区中写入数据
 * @cbuff:
 * @indata:
 * @inlen:
 * @be_cover: 0表示非覆盖，1表示覆盖
 *
 * 覆盖写入时，如果buff满会循环覆盖先前已有的数据，并返回覆盖的数据计数。
 * 非覆盖写入时，如果写入下一个数据时，buff已满，就退出，并返回写入的数据计数。
 */
uint32_t cirbuff_write(cirbuff_handle cbuff_h,
        uint8_t *indata, uint32_t inlen, uint8_t be_cover)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;
    int32_t ret;

    if (be_cover)
        ret = cirbuff_cover_write(cbuff, indata, inlen);
    else
        ret = cirbuff_nocover_write(cbuff, indata, inlen);

    return ret;
}


/*
 * cirbuff_cover_write - 覆盖写入循环缓冲区
 *
 * 返回覆盖的字节数
 */
static uint32_t cirbuff_cover_write(cirbuff_handle cbuff_h,
        uint8_t *indata, uint32_t inlen)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;
    uint32_t i;
    uint32_t cover_cnt = 0;

    for (i = 0; i < inlen; i++)
    {
        cbuff->buff[cbuff->wpos] = indata[i];

        // 更新状态
        cbuff->wpos++;
        if (cbuff->wpos == cbuff->size)
            cbuff->wpos = 0;

        // 是否存储区满
        if (cirbuff_be_full(cbuff))
        {
            cbuff->rpos = cbuff->wpos;
            cover_cnt++;
        }
        else
        {
            cbuff->cnt++;
        }
    }

    if (i)
        cbuff->update = 1;

    return cover_cnt;
}

/*
 * cirbuff_nocover_write - 非覆盖写入循环缓冲区
 *
 * 返回写入的字节数
 */
static uint32_t cirbuff_nocover_write(cirbuff_handle cbuff_h,
        uint8_t *indata, uint32_t inlen)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;
    uint32_t i = 0;

    while (i < inlen)
    {
        // 是否存储区满
        if (cirbuff_be_full(cbuff))
            break;

        // 更新状态
        cbuff->buff[cbuff->wpos++] = indata[i];
        cbuff->cnt++;
        if (cbuff->wpos == cbuff->size)
            cbuff->wpos = 0;

        i++;
    }

    if (i)
        cbuff->update = 1;

    return i;
}

/*
 * cirbuff_clear_read - 读循环缓冲区的数据，并且清除已读的数据
 *
 * 参数outdata可以为NULL，此时读操作的赋值语句被屏蔽，可用于删除一段数据
 * 返回已读的字节数，和outdata是否为NULL无关
 */
static uint32_t cirbuff_clear_read(cirbuff_handle cbuff_h,
        uint8_t *outdata, uint32_t outlen)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;
    uint32_t i = 0;

    while (i < outlen)
    {
        // 是否存储区空
        if (cirbuff_be_empty(cbuff))
            break;

        if (outdata != NULL)
            outdata[i] = cbuff->buff[cbuff->rpos];

        // 更新状态
        cbuff->cnt--;
        cbuff->rpos++;
        if (cbuff->rpos == cbuff->size)
            cbuff->rpos = 0;

        i++;
    }

    if (i)
        cbuff->update = 0;

    return i;
}

/*
 * cirbuff_noclear_read - 读循环缓冲区的数据，不会清除已读的数据
 *
 * 参数outdata可以为NULL，此时读操作的赋值语句被屏蔽，可用于仅仅置更新状态的操作
 * 返回已读的字节数，和outdata是否为NULL无关
 */
static uint32_t cirbuff_noclear_read(cirbuff_handle cbuff_h,
        uint8_t *outdata, uint32_t outlen)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;
    uint32_t i = 0;
    uint32_t cnt = cbuff->cnt;
    uint32_t rpos = cbuff->rpos;

    while (i < outlen)
    {
        // 是否存储区空
        if (cirbuff_be_empty(cbuff))
            break;

        if (outdata != NULL)
            outdata[i] = cbuff->buff[rpos];

        // 更新状态
        cnt--;
        rpos++;
        if (rpos == cbuff->size)
            rpos = 0;

        i++;
    }

    if (i)
        cbuff->update = 0;

    return i;
}

uint32_t cirbuff_read(cirbuff_handle cbuff_h,
        uint8_t *outdata, uint32_t outlen, uint8_t be_clear)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;
    int32_t ret;

    if (be_clear)
        ret = cirbuff_clear_read(cbuff, outdata, outlen);
    else
        ret = cirbuff_noclear_read(cbuff, outdata, outlen);

    return ret;
}

void cirbuff_revert(cirbuff_handle  cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    cbuff->rpos = 0;
    cbuff->wpos = 0;
    cbuff->cnt = 0;
    cbuff->update = 0;

    return;
}

void cirbuff_destroy(cirbuff_handle cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    if (cbuff != NULL)
    {
        if (cbuff->buff != NULL)
            free(cbuff->buff);

        free(cbuff);
        cbuff = NULL;
    }

    return;
}

// 以下函数为对循环缓冲区的查询

inline uint32_t cirbuff_be_empty(cirbuff_handle cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    return  cbuff->cnt == 0 ? 1 : 0;
}
inline uint32_t cirbuff_be_full(cirbuff_handle cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    return  cbuff->cnt == cbuff->size ? 1 : 0;
}
inline uint32_t cirbuff_data_num(cirbuff_handle cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    return cbuff->cnt;
}
inline uint32_t cirbuff_free_num(cirbuff_handle cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    return cbuff->size - cbuff->cnt;
}
inline uint32_t cirbuff_be_update(cirbuff_handle cbuff_h)
{
    struct cirbuff *cbuff = (struct cirbuff *)cbuff_h;

    return  cbuff->update;
}

