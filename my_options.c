#include "my_options.h"
#include "wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>

static char *optstring = "tub:lc:h";

static void help();
static void options_check(Options *op);
static bool is_a_valid_port(const char *str);
void show_options(Options *op);

//仅照抄命令行，不检查数据的有效性
Options get_options(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "error: missing argument.\n");
        help();
        exit(EXIT_FAILURE);
    }

    Options op;
    bzero(&op, sizeof(op));

    int opt;
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 't':
            op.tcp = true;
            break;
        case 'u':
            op.udp = true;
            break;
        case 'b': //bind允许只给出端口号，或是同时给出IP地址和端口号
            op.bind = true;
            if (argv[optind] && argv[optind][0] == '-') 
            {   //仅给出端口号，且不是最后一个参数
                op.local_address = true;
                op.local_address_str = "0.0.0.0";
                op.local_port = true;
                op.local_port_str = optarg;
            }
            else if (!argv[optind])
            {   //仅给出端口号，且为最后一个参数
                op.local_address = true;
                op.local_address_str = "0.0.0.0";
                op.local_port = true;
                op.local_port_str = optarg;
            }
            else //同时给出IP地址和端口号
            {
                op.local_address = true;
                op.local_address_str = optarg;
                op.local_port = true;
                op.local_port_str = argv[optind];
            }
            break;
        case 'l':
            op.listen = true;
            break;
        case 'c':
            op.connect = true;
            op.remote_address = true;
            op.remote_address_str = optarg;
            op.remote_port = true;
            op.remote_port_str = argv[optind];
            break;
        case 'h':
            help(argc, argv);
            exit(EXIT_FAILURE);
        case '?':
            exit(EXIT_FAILURE);
        case ':':
            fprintf(stderr, "error: missing argument.\n\n");
            help(argc, argv);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "error: argument error.\n\n");
            help(argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    options_check(&op);
    //show_options(&op);
    
    return op;
}

//可视化显示Options内容
void show_options(Options *op)
{
    fprintf(stderr, "show options: ");
    if (op->tcp)
        fprintf(stderr, "TCP ");
    if (op->udp)
        fprintf(stderr, "UDP ");
    if (op->bind)
        fprintf(stderr, "bind ");
    if (op->local_address)
        fprintf(stderr, "local address: %s ", op->local_address_str);
    if (op->local_port)
        fprintf(stderr, "local port: %s ", op->local_port_str);
    if (op->listen)
        fprintf(stderr, "listen ");
    if (op->connect)
        fprintf(stderr, "connect ");
    if (op->remote_address)
        fprintf(stderr, "remote address: %s ", op->remote_address_str);
    if (op->remote_port)
        fprintf(stderr, "remote port: %s ", op->remote_port_str);
    
    fprintf(stderr, "\n");
}

//检查选项的正确性
static void options_check(Options *op)
{
    if (!(op->tcp ^ op->udp)) 
    {   //tcp和udp必须且只能选一个
        fprintf(stderr, "error: '-t' or '-u' (not both) was needed.\n");
        exit(EXIT_FAILURE);
    }
    else if (op->tcp && !(op->listen ^ op->connect))
    {   //对于TCP，listen和connect必须且只能选一个
        fprintf(stderr, "error: '-l' or '-c' (not both) was needed.\n");
        exit(EXIT_FAILURE);
    }
    else if (op->connect && !is_a_valid_port(op->remote_port_str)) 
    {   //判断连接的端口号是否合法
        fprintf(stderr, "error: invalid conncet port.\n");
        exit(EXIT_FAILURE);
    }
    else if (op->bind && !is_a_valid_port(op->local_port_str)) 
    {   //判断绑定端口号是否合法
        fprintf(stderr, "error: invalid bind port.\n");
        exit(EXIT_FAILURE);
    }
    else if (op->listen && !op->bind) 
    {   //TCP服务器必须绑定端口
        fprintf(stderr, "error: server must bind a port.\n");
        exit(EXIT_FAILURE);
    }
    else if (op->bind && (op->local_address_str == NULL || op->local_port_str == NULL))
    {   //绑定必须指出绑定的IP地址和端口号
        fprintf(stderr, "error: remote address and port was needed.\n");
        exit(EXIT_FAILURE);
    }
    else if (op->connect && (op->remote_address_str == NULL || op->remote_port_str == NULL))
    {   //客户端必须指出要连接的IP地址和端口号
        fprintf(stderr, "error: remote address and port was needed.\n");
        exit(EXIT_FAILURE);
    }

}

//打印帮助信息
static void help()
{
    char *help_message = "                          \n\
usage: netassist <options>                          \n\
    \n\
options:                                            \n\
    -t                      TCP socket              \n\
    -u                      UDP socket              \n\
    -b [address] <port>     bind                    \n\
    -l                      listen (TCP only)       \n\
    -c <address> <port>     connect                 \n\
    -h                      help                    \n\
    \n\
for example: lunch a TCP server \
and listen on port 8888 by:                         \n\
$ ./netassist -tb 8888 -l                           \n\
                                                    \n\
, or lunch a UDP server on 127.0.0.1:8888 \
and connect to 127.0.0.1:8889 by:                   \n\
$ ./netassist -ub 127.0.0.1 8888 -c 127.0.0.1 8889  \n\
                                                    \n";

    fprintf(stderr, "%s", help_message);
}

//判断是否为合法端口号
static bool is_a_valid_port(const char *str)
{
    int len = strlen(str);

    if (len <= 0 && len > 5) //字符串长度在1~5之间
        return false;
    
    for (int i = 0; i < len; i++) //字符串必须由数字字符构成
        if (*(str + i) < '0' || *(str + i) > '9')
            return false;

    if (atoi(str) > 65535) //不能超过上限
        return false;

    return true;
}