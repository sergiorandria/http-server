#ifndef __PTR_ALLOCATION_H 
#define __PTR_ALLOCATION_H 

#define OBJ_TYPE_STRING     0xff0033


#define CHECK_ATOMIC_STR_ALLOCATION(ptr)   \
    do \
{   \
    if ((ptr) == NULL) \
        exit(OBJ_TYPE_STRING);  \
} while (0)


#endif 
