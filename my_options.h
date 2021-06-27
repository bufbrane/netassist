#ifndef MY_OPTIONS_H
#define MY_OPTIONS_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/*
options:

-t                      TCP socket
-u                      UDP socket
-b <[address:]port>     bind
-l                      listen
-c <[address:]port>     connect
-h                      help

*/

typedef struct Options
{
    bool tcp;
    bool udp;
    bool bind;
    bool listen;
    bool connect;
    
    bool local_address;         
    char *local_address_str;
    bool local_port;
    char *local_port_str;

    bool remote_address;
    char *remote_address_str;
    bool remote_port;
    char *remote_port_str;

} Options;

Options get_options(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif