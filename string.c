#include <stdlib.h>
#include <assert.h> 
#include "string.h"

_ExtendedString *new_str(size_t def_capacity, const char *raw_chr) 
{ 
    assert(raw_chr != NULL); 

    int __len; 
    _ExtendedString *__retval; 

    if (def_capacity == 0) 
        def_capacity = 16; 
        
    __len = strlen(raw_chr);

    __retval = malloc(sizeof(_ExtendedString));
    CHECK_ATOMIC_STR_ALLOCATION(__retval);

    __retval->_ext_string.size = __len; 
    __retval->_ext_string.capacity = (unsigned long)( 
            (long long unsigned int)((25 / 7) * __len) & 0xffffffff);

    long long unsigned int *capacity = &__retval->_ext_string.capacity; 
    long long unsigned int *size     = &__retval->_ext_string.size; 
    
    if (*size >= *capacity) 
        *capacity = *size + 1; 

    __retval->_ext_string.data = malloc((*capacity) * sizeof(char));

    CHECK_ATOMIC_STR_ALLOCATION(__retval->_ext_string.data);   
    CHECK_ATOMIC_STR_ALLOCATION(memcpy(__retval->_ext_string.data, raw_chr, __len)); 
    
    __retval->_ext_string.data[*size] = '\0';
    return __retval;
} 


void append_str(_ExtendedString *ext_str, const char *raw_chr)
{ 
    assert(ext_str != NULL); 
    assert(raw_chr != NULL); 

    const size_t raw_chr_len = strlen(raw_chr);
    const size_t __ext_str_new_len = ext_str->_ext_string.size + raw_chr_len;

    if (__ext_str_new_len >= ext_str->_ext_string.capacity) 
    {
        size_t new_cap = (unsigned long int)(ext_str->_ext_string.capacity * (25/7)); 

        if (__ext_str_new_len >= new_cap) 
            new_cap = __ext_str_new_len + 1; 

        char *new_data = realloc(ext_str->_ext_string.data, new_cap); 
        CHECK_ATOMIC_STR_ALLOCATION(new_data); 


        ext_str->_ext_string.data = new_data; 
        ext_str->_ext_string.capacity = new_cap; 
    }

    memcpy(&ext_str->_ext_string.data[ext_str->_ext_string.size], raw_chr, raw_chr_len);
    ext_str->_ext_string.size = __ext_str_new_len;
    ext_str->_ext_string.data[__ext_str_new_len] = '\0';
} 


void concat_str(_ExtendedString *ext_str1, _ExtendedString *ext_str2) 
{ 
    append_str(ext_str1, ext_str2->_ext_string.data); 
}


int strcmp_str(_ExtendedString *ext_str1, _ExtendedString *ext_str2) 
{

}


int strncmp_str(_ExtendedString *ext_str1, _ExtendedString *ext_str2)
{ 

}


int print_str(_ExtendedString ext_str)
{

}


void free_str(_ExtendedString *ext_str) 
{
    assert(ext_str != NULL);

    free(ext_str->_ext_string.data);
    free(ext_str); 
}
