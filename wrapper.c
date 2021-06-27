#include "wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>

void err_quit(char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);

    char buf[MAXLINE + 1];
    bzero(&buf, sizeof(buf));

    //替换%s一类的内容
    int start = 0, count = 0;
    char data_buf[MAXLINE];
    while (start < strlen(msg))
    {
        while (*(msg + start + count) != '\0' && *(msg + start + count) != '%')
            count++;

        strncat(buf, msg + start, count);

        if (*(msg + start + count) == '%')
        {
            switch (*(msg + start + count + 1))
            {
            case 's':
                strcat(buf, va_arg(ap, char*));
                break;
            case 'd':
                sprintf(data_buf, "%d", va_arg(ap, int));
                strcat(buf, data_buf);
                break;
            default:
                fprintf(stderr, "err_quit函数暂不支持除%%s和%%d之外的格式\n");
                break;
            }
        }

        start = start + count + 2;
        count = 0;
    }

    va_end(ap);
    fprintf(stderr, "%s\n", buf);
    exit(EXIT_FAILURE);
}

/*
释放write_req的内存，用于on_stdout_write和on_file_write函数
*/
void free_write_req(uv_write_t *req) 
{
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

/*
向stream中写入buf的前size字节数据
*/
void write_data(uv_stream_t *dest, size_t size, uv_buf_t buf, uv_write_cb cb) 
{   
    write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
    req->buf = uv_buf_init((char *)malloc(size), size);
    memcpy(req->buf.base, buf.base, size);
    uv_write((uv_write_t*)req, (uv_stream_t*)dest, &req->buf, 1, cb);
}

/*
UDP发送数据

handle：已connect的UDP套接字句柄
*/
void udp_send_data(uv_udp_t *handle, size_t size, const uv_buf_t buf, uv_udp_send_cb cb)
{
    uv_udp_send_t *req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
    uv_buf_t sendbuf = uv_buf_init((char*)malloc(size), size);
    req->bufs = &sendbuf;
    memcpy(req->bufs->base, buf.base, size);
    uv_udp_send(req, &udp_connect_socket, &req->bufs[0], 1, NULL, cb);
}