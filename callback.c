#include "wrapper.h"

#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
回调函数，由uv_udp_recv_start和uv_read_start调用
为buf分配内存并初始化
*/
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

/*
回调函数，由write_data和uv_write调用
用于清理自定义request的内存
*/
void on_stdout_write(uv_write_t *req, int status) 
{
    free_write_req(req);
}

/*暂未使用
void on_file_write(uv_write_t *req, int status) 
{
    free_write_req(req);
}
*/

//-----------------------------------------TCP server--------------------------------------

/*
回调函数，由write_data调用
TCP发送成功后
*/
void on_tcp_send(uv_write_t* req, int status)
{
    if (status < 0)
        fprintf(stderr, "send error: %s\n", uv_err_name(status));

    free_write_req(req);
}

/*
回调函数，用于uv_read_start
从stream读入数据并通过TCP发送
*/
void on_tcp_server_read_start(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) 
{
    if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            uv_read_stop(stream);
            free(stream);

            struct sockaddr_in remote_addr;
            int addr_len = sizeof(struct sockaddr_in);
            char remote_addr_str[INET_ADDRSTRLEN];

            uv_tcp_getpeername((uv_tcp_t*)stream, (struct sockaddr *)&remote_addr, &addr_len);
            uv_ip4_name(&remote_addr, remote_addr_str, INET_ADDRSTRLEN);

            fprintf(stderr, "connection closed from %s:%d\n",remote_addr_str, ntohs(remote_addr.sin_port));

        }
    } 
    else if (nread > 0) 
    {
        //控制台输出
        write_data((uv_stream_t *)&stdout_pipe, nread, *buf, on_stdout_write);

        //echo服务器
        //write_data((uv_stream_t *)stream, nread, *buf, on_tcp_send);
    }

    if (buf->base)
        free(buf->base);
}

/*
回调函数，由uv_listen调用
用于当listen成功后创建accept
*/
void on_new_connection(uv_stream_t* server, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "new connection error %s\n", uv_err_name(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);
    if (uv_accept((uv_stream_t *)&tcp_server_socket, (uv_stream_t *)client) == 0)
    {
        struct sockaddr_in remote_addr;
        int addr_len = sizeof(struct sockaddr_in);
        char remote_addr_str[INET_ADDRSTRLEN];

        uv_tcp_getpeername(client, (struct sockaddr *)&remote_addr, &addr_len);
        uv_ip4_name(&remote_addr, remote_addr_str, INET_ADDRSTRLEN);

        fprintf(stderr, "connection accepted from %s:%d\n",remote_addr_str, ntohs(remote_addr.sin_port));

        //从套接字读取数据，并打印到控制台
        uv_read_start((uv_stream_t *)client, alloc_buffer, on_tcp_server_read_start);
    }

}

//-----------------------------------------TCP client--------------------------------------

/*
回调函数，用于uv_read_start函数
从TCP接收缓冲区读出数据，并写入标准输出
*/
void on_tcp_recv_start(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    if (nread < 0)
    {
        fprintf(stderr, "read error %s\n", uv_err_name(nread));
        free(buf->base);
    }
    else if (nread > 0)
    {
        write_data((uv_stream_t *)&stdout_pipe, nread, *buf, on_stdout_write);
    }

    if (buf->base)
        free(buf->base);
}

/*
回调函数，用于uv_read_start
从stream读入数据并通过TCP发送
*/
void on_tcp_read_start(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) 
{
    if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            uv_read_stop(stream);
            uv_close((uv_handle_t *)&stdin_pipe, NULL);
            uv_close((uv_handle_t *)&stdout_pipe, NULL);
            uv_close((uv_handle_t *)&tcp_client_socket, NULL);
        }
    } 
    else if (nread > 0) 
    {
        //控制台输出
        //write_data((uv_stream_t *)&stdout_pipe, nread, *buf, on_stdout_write);

        write_data((uv_stream_t *)&tcp_client_socket, nread, *buf, on_tcp_send);
    }

    if (buf->base)
        free(buf->base);
}

/*
回调函数，由uv_tcp_connect调用
*/
void on_tcp_connect(uv_connect_t* req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "tcp connect failed %s\n", uv_err_name(status));
        err_quit("exit.");
    }

    int send_buffer_size = 10;
    uv_send_buffer_size((uv_handle_t*)&stdin_pipe, &send_buffer_size);

    //从套接字读取数据，并打印到控制台
    uv_read_start((uv_stream_t *)&tcp_client_socket, alloc_buffer, on_tcp_recv_start);
    
    //从控制台读取数据，通过connect发送出去
    uv_read_start((uv_stream_t *)&stdin_pipe, alloc_buffer, on_tcp_read_start);
}

//-----------------------------------UDP server & client--------------------------------------

/*
回调函数，用于udp_send_data和uv_udp_send函数
当数据发送完成后检查发送是否成功
*/
void on_udp_send(uv_udp_send_t *req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "send error: %s\n", uv_err_name(status));
    }

    free(req->bufs);
}

/*
回调函数，用于uv_udp_recv_start函数
从UDP接收缓冲区读出数据，并写入标准输出
*/
void on_udp_recv_start(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    if (nread < 0)
    {
        fprintf(stderr, "read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)req, NULL);
        free(buf->base);
    }
    else if (nread > 0)
    {
        write_data((uv_stream_t *)&stdout_pipe, nread, *buf, on_stdout_write);
    }

    if (buf->base)
        free(buf->base);
}

/*
回调函数，用于uv_read_start
从stream读入数据并通过UDP发送
*/
void on_udp_read_start(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) 
{
    if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            uv_read_stop(stream);
            uv_udp_connect(&udp_connect_socket, NULL); //隐式断开connect
            uv_close((uv_handle_t *)&stdin_pipe, NULL);
            uv_close((uv_handle_t *)&stdout_pipe, NULL);
            uv_close((uv_handle_t *)&udp_listen_socket, NULL);
            uv_close((uv_handle_t *)&udp_connect_socket, NULL);
        }
    } 
    else if (nread > 0) 
    {
        //控制台输出
        //write_data((uv_stream_t *)&stdout_pipe, nread, *buf, on_stdout_write);

        udp_send_data(&udp_connect_socket, nread, *buf, on_udp_send);
    }

    if (buf->base)
        free(buf->base);
}

