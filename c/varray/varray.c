/*
 * varray.c - 动态数组的实现
 *
 */
#include <malloc.h>
#include <string.h>

#include "varray.h"

// #define CLEAR_FREE_MEM




bool varray_init(varray_t *varray, uint32_t bsize)
{
    struct varray_inside *va_in = (struct varray_inside *)varray;
    va_in->bsize = bsize;

    if ((va_in->buff = (uint8_t *)malloc(va_in->bsize)) == NULL)
    {
        return 0;
    }

    va_in->size = va_in->bsize;

#ifdef CLEAR_FREE_MEM
    memset(va_in->buff, 0, va_in->size);
#endif /* CLEAR_FREE_MEM */

    return 1;
}


bool check_varray(varray_t *varray, uint32_t req_size)
{
    uint8_t *new_buff = NULL;
    uint32_t new_size = 0;
    struct varray_inside *va_in = (struct varray_inside *)varray;

    if (req_size <= va_in->size)
        return 1;

    // 确定要分配空间的大小
    new_size = (req_size % va_in->bsize == 0) ?
        req_size : (req_size / va_in->bsize + 1) * va_in->bsize;
    if ((new_buff = (uint8_t *)realloc(va_in->buff, new_size)) == NULL)
        return 0;

    if (va_in->buff != new_buff)
        free(va_in->buff);


    va_in->buff = new_buff;
#ifdef CLEAR_FREE_MEM
    memset(&va_in->buff[va_in->size], 0, new_size - va_in->size);
#endif /* CLEAR_FREE_MEM */
    va_in->size = new_size;

    return 1;
}

void destory_varray(varray_t *varray)
{
    struct varray_inside *va_in = (struct varray_inside *)varray;

    if (va_in->buff != NULL)
    {
        free(va_in->buff);
        va_in->buff = NULL;
    }
    va_in->size = 0;
    va_in->bsize = 0;
    return;
}



