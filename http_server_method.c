#include "http_server_method.h" 

struct HttpMethod HTTP_METHODS[] = { 
    { .str = HTTP_METHOD_GET, .type = HTTP_GET},
    { .str = HTTP_METHOD_POST, .type = HTTP_POST},
    { .str = HTTP_METHOD_PUT, .type = HTTP_PUT},
    { .str = HTTP_METHOD_PATCH, .type = HTTP_PATCH},
    { .str = HTTP_METHOD_DELETE, .type = HTTP_DELETE},
};
