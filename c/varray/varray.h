/*
 * varray.h - 动态数组实现的声明
 */

#include <stdint.h>
#include <stdbool.h>


struct varray_inside
{
    uint8_t *buff;    //
    uint32_t size;     //
    uint32_t bsize;    //
};

typedef const struct varray_inside varray_t;



bool varray_init(varray_t *varray, uint32_t bsize);
bool check_varray(varray_t *varray, uint32_t req_size);
void destory_varray(varray_t *varray);
