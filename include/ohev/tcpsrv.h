#ifndef OHEV_TCPSRV_H
#define OHEV_TCPSRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ohutil/util.h>
#include <ohev/evt.h>
#include <ohev/buffer.h>
#include <ohev/evtpool.h>

typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr_in sockaddr_in;

#define _TCP_CONNECTION         \
    int flag;                   \
    int fd;                     \
    sockaddr_storage addr;      \
    evt_loop *loop_on;          \
    void *data;


typedef struct _tag_tcp_connection {
    _TCP_CONNECTION;
} tcp_connection;


typedef struct _tag_tcp_client tcp_client;
typedef struct _tag_tcp_client_objpool tcp_client_objpool;

#define TCPSERVER_FLAG_POOL 0x01


typedef void (*tcp_server_cb)(tcp_client*);

typedef struct _tag_tcp_server {
    _TCP_CONNECTION;

    tcp_client_objpool *clt_objpool;
    ohbuffer_objpool *buf_objpool;

    atomic32_t clt_cnt;

    evt_io accept_ev;
    tcp_server_cb on_accept;
    tcp_server_cb on_read;
    tcp_server_cb on_write;
    tcp_server_cb on_close;
} tcp_server;

tcp_server* tcp_server_init_anyv4(int, int);
tcp_server* tcp_server_init_strv4(const char* , int, int);
tcp_server* tcp_server_init_addrv4(struct sockaddr*, int);


#define TCPCLIENT_OBJPOOL_BLOCKSZ 32

typedef struct _tag_tcp_client {
    _TCP_CONNECTION;

    evt_io read_ev;
    evt_io write_ev;
} tcp_client;

/* client pool */
typedef struct _tag_tcp_client_obj {
    OBJPOOL_OBJ_BASE(struct _tag_tcp_client_obj);
    tcp_client client;
} tcp_client_obj;

typedef struct _tag_tcp_client_objpool {
    OBJPOOL_BASE(tcp_client_obj)
} tcp_client_objpool;


#define tcp_clientpool_init(pool, bsz, osz) \
    objpool_init((objpool_base*)(pool), (bsz), (osz))
#define tcp_clientpool_get(pool)  \
    (tcp_client_obj*)objpool_get_obj((objpool_base*)(pool))
#define tcp_clientpool_free(pool, elem)     \
    objpool_free_obj((objpool_base*)(pool), elem)



#ifdef __cplusplus
}
#endif
#endif  //OHEV_TCPSRV_H020