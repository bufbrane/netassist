#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <uv.h>

//行缓冲区长度
#define MAXLINE 1024

//listen的第二个参数，backlog参数规定了内核应该为相应套接字排队的最大连接个数
#define DEFAULT_BACKLOG 1024

uv_tcp_t tcp_client_socket, tcp_server_socket;
uv_connect_t tcp_connect;
uv_udp_t udp_listen_socket, udp_connect_socket;
uv_pipe_t stdin_pipe, stdout_pipe;

void err_quit(char *message, ...);

/*自定义request，用于write_data函数传递context*/
typedef struct 
{
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

/*
释放write_req的内存，用于on_stdout_write和on_file_write函数
*/
void free_write_req(uv_write_t *req);

/*
向stream中写入buf的前size字节数据
*/
void write_data(uv_stream_t *dest, size_t size, uv_buf_t buf, uv_write_cb cb) ;

/*
UDP发送数据

handle：已connect的UDP套接字句柄
*/
void udp_send_data(uv_udp_t *handle, size_t size, const uv_buf_t buf, uv_udp_send_cb cb);

#ifdef __cplusplus
}
#endif

#endif