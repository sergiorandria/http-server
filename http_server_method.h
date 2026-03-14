#ifndef __HTTP_METHOD_H 
#define __HTTP_METHOD_H 

#include "http_server_alias.h"
#include "http_server_method_type.h" 

struct _HttpMethod { 
    const char *str; 
    enum HttpMethodType type; 
}; 

typedef struct _HttpMethod _HttpMethod; 
 
#define HTTP_VERSION        "HTTP/1.1" 

#define HTTP_METHOD_GET      "GET" 
#define HTTP_METHOD_POST     "POST" 
#define HTTP_METHOD_PUT      "PUT" 
#define HTTP_METHOD_PATCH    "PATCH" 
#define HTTP_METHOD_DELETE   "DELETE" 

extern _HttpMethod HTTP_METHODS[]; 
 
#define HTTP_METHODS_LEN    5 

#define HttpMethod _HttpMethod 

http_server_alias(http_method, HttpMethod); 
http_server_alias(http_method, _HttpMethod); 

#endif
