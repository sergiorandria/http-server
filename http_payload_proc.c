#include "http_payload_proc.h" 

char *get_http_header(_HttpRequest *req, char *header) 
{ 
    assert(req != NULL);
    assert(header != NULL); 
 
    for(__u32 i = 0; i < req->headers_len; ++i) 
    { 
        if (strcasecmp(header, req->headers[i].key) == 0) 
            return req->headers[i].value; 
    }

    return NULL; 
}
