#ifndef __HTTP_PAYLOAD_PROC 
#define __HTTP_PAYLOAD_PROC 

#define BUFFER_SIZE     1024 
#define MAX_HEADERS     128 

#include <assert.h> 

#include "string.h"
#include "http_server_method_type.h"

struct _HttpHeader { 
    char *key;  
    char *value; 
};

typedef struct _HttpHeader _HttpHeader; 

struct _HttpRequest { 
    enum HttpMethodType method;
    char *path; 

    _HttpHeader *headers; 
    size_t headers_len; 

    char *buffer; 
    size_t buffer_len; 

    ExtendedString *body;
};

typedef struct _HttpRequest _HttpRequest; 

char *get_http_header(_HttpRequest *req, char *header);

#define HttpHeader _HttpHeader 
#define HttpRequest _HttpRequest 

http_server_alias(http_header, HttpHeader); 
http_server_alias(http_header, _HttpHeader); 

http_server_alias(http_request, HttpRequest);
http_server_alias(http_request, _HttpRequest); 

#endif 
