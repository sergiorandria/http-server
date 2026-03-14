#ifndef _GNU_SOURCE 
#define _GNU_SOURCE 
#endif 

#ifndef GNU_SOURCE 
#define GNU_SOURCE 
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>

#include "string.h"
#include "http_server_method.h"
#include "http_payload_proc.h"

#define CHECK_RETVAL(ret, func_name)   \
    do \
{\
    if ((ret) == -1) {\
        perror((func_name)); \
        goto cleanup; \
    }\
} while (0)

static void handle_connection(int conn_fd);
static void *handle_conn_callback(void *arg); 

ExtendedString *filename = NULL;

    int 
main(int argc, const char *argv[]) 
{ 
    int server_fd = -1, reuse_addr = 1; 
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t client_addr_len = sizeof(client_addr); 

    extern ExtendedString *filename; 

    if (argc > 2) { 
        filename = new_str(0,argv[2]);
        __u32 len = strlen(argv[2]);
        if (argv[2][len - 1] != '/') { 
            append_str(filename, "/"); 
        }
    } else { 
        filename = new_str(0, "./public/");
    }

    setbuf(stdout, NULL); 
    setbuf(stderr, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    CHECK_RETVAL(server_fd, "socket"); 
    CHECK_RETVAL(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)), "setsockopt");

    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr = (struct in_addr){ htonl(INADDR_ANY) };
    server_addr.sin_port = htons(4221); 

    CHECK_RETVAL(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)), "bind");
    CHECK_RETVAL(listen(server_fd, 10), "listen"); 

    printf("Waiting for a client to connect ...\n");

    while (true) 
    {
        int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len); 
        CHECK_RETVAL(conn_fd, "accept");

        handle_connection(conn_fd);
    }

    return 0;

cleanup: 
    if (server_fd != -1) {
        close(server_fd);
    }

    if (filename != NULL) {
        free_str(filename);
    }

    return -1; 
} 

void handle_connection(int conn_fd) 
{
    pthread_t new_thread;
    __u16 *conn_fd_ptr = malloc(sizeof(__u16)); 

    CHECK_ATOMIC_STR_ALLOCATION(conn_fd_ptr);

    *conn_fd_ptr = conn_fd; 
    pthread_create(&new_thread, NULL, handle_conn_callback, conn_fd_ptr); 
}

void *handle_conn_callback(void *conn_fd_ptr) 
{ 
    assert(conn_fd_ptr != NULL); 

    int conn_fd = *(int *)(conn_fd_ptr), bytes_sent = -1; 
    FILE *fp = NULL; 
    char *method, *content_length_header;
    __u64 content_length = 0; 
    __u64 original_filename_len = filename->_ext_string.size;
    __u64 bytes_read; 

    HttpRequest req = {0}; 

    printf("processing connection with fd %d\n", conn_fd); 

    req.buffer = malloc(BUFFER_SIZE);
    CHECK_ATOMIC_STR_ALLOCATION(req.buffer); 

    bytes_read = recv(conn_fd, req.buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read == -1) {
        printf("error: Recv failed: %s \n", strerror(errno));
        goto cleanup;
    } else if (bytes_read == 0) {
        printf("error: Recv: connection closed by client\n");
        goto cleanup;
    }

    req.buffer[bytes_read] = '\0';
    req.buffer_len = (size_t)bytes_read;

    char *parse_buffer = req.buffer;
    method = strsep(&parse_buffer, " ");
    for (size_t i = 0; i < HTTP_METHODS_LEN; ++i) {
        if (strcmp(method, HTTP_METHODS[i].str) == 0) {
            req.method = HTTP_METHODS[i].type;
            break;
        }
    }

    if (req.method == HTTP_UNKNOWN) {
        printf("error(parse): unknown method %s\n", method);
        goto cleanup;
    }

    req.path = strsep(&parse_buffer, " ");

    strsep(&parse_buffer, "\r");
    char *http_version = strsep(&parse_buffer, "\r");

    parse_buffer++;

    req.headers = malloc(sizeof(struct HttpHeader) * MAX_HEADERS);
    req.headers_len = 0;

    for (size_t i = 0; i < MAX_HEADERS; ++i) {
        char *header_line = strsep(&parse_buffer, "\r");

        parse_buffer++;

        if (header_line[0] == '\0') {
            break;
        }

        char *key = strsep(&header_line, ":");
        header_line++; 
        char *value = strsep(&header_line, "\0");

        req.headers[i].key = key;
        req.headers[i].value = value;

        ++req.headers_len;
    }

    content_length_header = get_http_header(&req, "content-length");
    if (content_length_header != NULL) {
        errno = 0;
        size_t result = strtoul(content_length_header, NULL, 10);
        
        if (errno == 0) {
            content_length = result;
        }
    }

    if (content_length > 0) {
        req.body = new_str(content_length + 1, NULL);
        append_str(req.body, parse_buffer);

        if (req.body->_ext_string.size != content_length) {
        }
    }

    if (strcmp(req.path, "/") == 0) {
        const char res[] = "HTTP/1.1 200 OK\r\n\r\n";
        bytes_sent = send(conn_fd, res, sizeof(res) - 1, 0);
    } else if (strcmp(req.path, "/user-agent") == 0) {
        char *s = get_http_header(&req, "user-agent");
        if (s == NULL) {
            s = "NULL";
        }

        size_t slen = strlen(s);

        char *res = malloc(BUFFER_SIZE);
        sprintf(res,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length:%zu\r\n\r\n"
                "%s",
                slen, s);

        bytes_sent = send(conn_fd, res, strlen(res), 0);
        free(res);
    } else if (strncmp(req.path, "/echo/", 6) == 0) {
        char *s = req.path + 6;
        size_t slen = strlen(s);

        char *res = malloc(BUFFER_SIZE);
        sprintf(res,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length:%zu\r\n\r\n"
                "%s",
                slen, s);

        bytes_sent = send(conn_fd, res, strlen(res), 0);
        free(res);
    } else if (req.method == HTTP_GET && strncmp(req.path, "/files/", 7) == 0) {
        append_str(filename, req.path + 7);

        errno = 0;
        fp = fopen(filename->_ext_string.data, "r");
        if (fp == NULL && errno != ENOENT) {
            perror("error: fopen()");
            goto cleanup;
        }

        ExtendedString *res = new_str(0, HTTP_VERSION);

        if (errno == ENOENT) {
            append_str(res, " 404 Not Found\r\n\r\n");
            bytes_sent = send(conn_fd, res->_ext_string.data, res->_ext_string.size, 0);
            goto cleanup;
        }

        if (fseek(fp, 0, SEEK_END) != 0) {
            perror("error: fseek()");
            goto cleanup;
        }

        long file_size = ftell(fp);
        if (file_size < 0) {
            perror("error: fseek()");
            goto cleanup;
        }

        errno = 0;
        rewind(fp);
        if (errno != 0) {
            perror("error: rewind()");
            goto cleanup;
        }

        char file_size_str[sizeof(file_size) + 1];
        sprintf(file_size_str, "%ld", file_size);

        append_str(res, " 200 OK\r\n");
        append_str(res, "Content-Type: application/octet-stream\r\n");
        append_str(res, "Content-Length: ");
        append_str(res, file_size_str);
        append_str(res, "\r\n\r\n");

        bytes_sent = send(conn_fd, res->_ext_string.data, res->_ext_string.size, 0);
        free_str(res);

        char buffer[BUFFER_SIZE];

        while (file_size > 0) {
            long bytes_to_read = file_size > BUFFER_SIZE ? BUFFER_SIZE : file_size;
            size_t bytes_read = fread(buffer, bytes_to_read, 1, fp);

            if (bytes_read < bytes_to_read && ferror(fp)) {
                puts("error: fread()");
                goto cleanup;
            }

            bytes_sent = send(conn_fd, buffer, bytes_to_read, 0);
            file_size -= bytes_to_read;
        }

    } else if (req.method == HTTP_POST && strncmp(req.path, "/files/", 7) == 0) {
        append_str(filename, req.path + 7);

        errno = 0;
        fp = fopen(filename->_ext_string.data, "r");
        if (fp != NULL) {
            const char res[] = "HTTP/1.1 409 Conflict\r\n\r\n";
            bytes_sent = send(conn_fd, res, sizeof(res) - 1, 0);
            goto cleanup;
        }

        errno = 0;
        fp = fopen(filename->_ext_string.data, "w");
        if (fp == NULL) {
            perror("error: fopen()");
            goto cleanup;
        }

        size_t num_chunks = req.body->_ext_string.size / BUFFER_SIZE;
        if (num_chunks > 0) {
            fwrite(req.body->_ext_string.data, BUFFER_SIZE, num_chunks, fp);
        }

        size_t remaining_bytes = req.body->_ext_string.size % BUFFER_SIZE;
        if (remaining_bytes > 0) {
            fwrite(&req.body->_ext_string.data[req.body->_ext_string.size - remaining_bytes],
                    remaining_bytes, 1, fp);
        }

        const char res[] = "HTTP/1.1 201 Created\r\n\r\n";
        bytes_sent = send(conn_fd, res, sizeof(res) - 1, 0);
    } else {
        const char res[] = "HTTP/1.1 404 Not Found\r\n\r\n";
        bytes_sent = send(conn_fd, res, sizeof(res) - 1, 0);
    }

    if (bytes_sent == -1) {
        perror("error: send()");
    }

cleanup:
    free(conn_fd_ptr);
    free(req.buffer);
    free(req.headers);
    if (req.body != NULL) {
        free_str(req.body);
    }

    close(conn_fd);

    if (fp != NULL) {
        fclose(fp);
    }

    filename->_ext_string.size = original_filename_len;
    filename->_ext_string.data[original_filename_len] = '\0';

    return NULL;
}
