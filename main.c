#include "my_options.h"
#include "wrapper.h"

#include <stdio.h>
#include <stdlib.h>

//callback.c
extern void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
extern void on_new_connection(uv_stream_t* server, int status);
extern void on_tcp_connect(uv_connect_t* req, int status);
extern void on_udp_read_start(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
extern void on_udp_recv_start(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);

int main(int argc, char **argv)
{
    Options op = get_options(argc, argv);
    uv_loop_t *loop = uv_default_loop();
    
    //使用管道读写stdin和stdout 
    uv_pipe_init(loop, &stdin_pipe, 0);
    uv_pipe_init(loop, &stdout_pipe, 0);
    uv_pipe_open(&stdin_pipe, 0);
    uv_pipe_open(&stdout_pipe, 1);
    
    if (op.tcp && op.listen) //TCP服务器
    {
        uv_tcp_init(loop, &tcp_server_socket);

        //设置监听地址和端口
        struct sockaddr_in local_addr;        
        if (op.bind && op.local_address && op.local_port)
            uv_ip4_addr(op.local_address_str, atoi(op.local_port_str), &local_addr);
        else if (op.bind && op.local_port)
            uv_ip4_addr("0.0.0.0", atoi(op.local_port_str), &local_addr);
        else 
            uv_ip4_addr("0.0.0.0", 0, &local_addr);
        
        int bind_result;
        if ((bind_result = uv_tcp_bind(&tcp_server_socket, (const struct sockaddr *)&local_addr, UV_UDP_REUSEADDR)) < 0)
            fprintf(stderr, "bind error %s\n", uv_err_name(bind_result));  

        int listen_result;
        if ((listen_result = uv_listen((uv_stream_t *)&tcp_server_socket, DEFAULT_BACKLOG, on_new_connection)))
            fprintf(stderr, "listen error %s\n", uv_err_name(bind_result));  

        uv_run(loop, UV_RUN_DEFAULT);
    }
    else if (op.tcp && op.connect) //TCP客户端
    {
        uv_tcp_init(loop, &tcp_client_socket);
        
        //设置监听地址和端口
        struct sockaddr_in local_addr;        
        if (op.bind && op.local_address && op.local_port)
            uv_ip4_addr(op.local_address_str, atoi(op.local_port_str), &local_addr);
        else if (op.bind && op.local_port)
            uv_ip4_addr("0.0.0.0", atoi(op.local_port_str), &local_addr);
        else 
            uv_ip4_addr("0.0.0.0", 0, &local_addr);
        
        int bind_result;
        if ((bind_result = uv_tcp_bind(&tcp_client_socket, (const struct sockaddr *)&local_addr, UV_UDP_REUSEADDR)) < 0)
            fprintf(stderr, "bind error %s\n", uv_err_name(bind_result));  

        uv_tcp_nodelay(&tcp_client_socket, TCP_NODELAY);

        struct sockaddr_in remote_addr;
        uv_ip4_addr(op.remote_address_str, atoi(op.remote_port_str), &remote_addr);
        int connect_result;
        if ((connect_result = uv_tcp_connect(&tcp_connect, &tcp_client_socket, (const struct sockaddr *)&remote_addr, on_tcp_connect)) < 0)
            fprintf(stderr, "connect error %s\n", uv_err_name(connect_result));  

        uv_run(loop, UV_RUN_DEFAULT);
    }
    else if (op.udp) //UDP客户端&服务器
    {
        uv_udp_init(loop, &udp_listen_socket);
        uv_udp_init(loop, &udp_connect_socket);
        
        //设置监听地址和端口
        struct sockaddr_in local_addr;        
        if (op.bind && op.local_address && op.local_port)
            uv_ip4_addr(op.local_address_str, atoi(op.local_port_str), &local_addr);
        else if (op.bind && op.local_port)
            uv_ip4_addr("0.0.0.0", atoi(op.local_port_str), &local_addr);
        else 
            uv_ip4_addr("0.0.0.0", 0, &local_addr);

        int bind_result;
        if ((bind_result = uv_udp_bind(&udp_listen_socket, (const struct sockaddr *)&local_addr, UV_UDP_REUSEADDR)) < 0)
            fprintf(stderr, "bind error %s\n", uv_err_name(bind_result));  
        
        //从套接字接收数据，输出到stdout
        uv_udp_recv_start(&udp_listen_socket, alloc_buffer, on_udp_recv_start);

        //连接远程套接字
        struct sockaddr_in remote_addr;        
        uv_ip4_addr(op.remote_address_str, atoi(op.remote_port_str), &remote_addr);
        int connect_result;
        if ((connect_result = uv_udp_connect(&udp_connect_socket, (struct sockaddr *)&remote_addr)) < 0)
            fprintf(stderr, "connect error %s\n", uv_err_name(connect_result));  
        
        //从控制台读取数据，通过connect发送出去
        uv_read_start((uv_stream_t *)&stdin_pipe, alloc_buffer, on_udp_read_start);

        uv_run(loop, UV_RUN_DEFAULT);
    }
    else
        err_quit("main error");

    return 0;
}