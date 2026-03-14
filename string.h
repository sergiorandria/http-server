#ifndef __STRING_H
#define __STRING_H 

#include <linux/types.h> 
#include <string.h>

#include "ptr_allocation.h"
#include "http_server_alias.h"

#define INIT_CAPACITY   16 
#define MAX_CAPACITY    8192
#define INIT_SIZE       64 
#define MAX_SIZE        8192

extern char string_table[];

struct _ExtendedString {
    __u8 id; // The string id on the table 
    int nullval;
    
    union {
        #ifdef __USE_EXTENDED_STRING32 
        // Will be used on older 
        // machine, the maximum size and capacity is 
        // limited at 2^32. 
        struct _ext_string32 { 
            char *data; 
            __u32 size; 
            __u32 capacity; 
        } _ext_string32;
        
        #define _ext_string     _ext_string32 
        #endif 

        #ifdef __USE_EXTENDED_STRING64
        struct _ext_string64 { 
            char *data;
            __u64 size; 
            __u64 capacity; 
        } _ext_string64;

        #define _ext_string     _ext_string64 
        #endif
    };
};

typedef struct _ExtendedString _ExtendedString; 

#define set_str_size(ext_str,sz)        ((ext_str)->_ext_string.size = (sz) ;)
#define set_str_capacity(ext_str,sz)    ((ext_str)->_ext_string.capacity = (sz) ;)
#define init_str_data(ext_str)          (memset((ext_str)->data, 0, (ext_str)->size))

_ExtendedString *new_str(size_t capacity, const char *raw_chr); 
void free_str(_ExtendedString *ext_str); 
void append_str(_ExtendedString *ext_str, const char *raw_chr);
void concat_str(_ExtendedString *ext_str1, _ExtendedString *ext_str2); 
int strcmp_str(_ExtendedString *ext_str1, _ExtendedString *ext_str2);
int strncmp_str(_ExtendedString *ext_str1, _ExtendedString *ext_str2);

#define ExtendedString  _ExtendedString

http_server_alias( string, ExtendedString );
http_server_alias( string, _ExtendedString ); 

#endif // __STRING_H 
