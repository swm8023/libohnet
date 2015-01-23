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
typedef struct sockaddr sockaddr;
typedef struct _tag_net_addr net_addr;
typedef struct _tag_tcp_connection tcp_connection;
typedef struct _tag_tcp_client tcp_client;
typedef struct _tag_tcp_server tcp_server;
typedef struct _tag_tcp_server_hub tcp_server_hub;
typedef struct _tag_tcp_client_objpool tcp_client_objpool;
typedef struct _tag_tcp_lb_ctx tcp_lb_ctx;
typedef struct _tag_tcp_loop_data tcp_loop_data;

typedef void (*tcp_cb)(tcp_client*);



typedef struct _tag_net_addr{
    sockaddr_storage addr;
} net_addr;

sockaddr* netaddr_to_sockaddr(net_addr*);
int netaddr_init_v4(net_addr*, char*, int);

typedef struct _tag_tcp_lb_ctx{
    void *data;
    evt_pool* pool;
    evt_loop* (*next_loop)(tcp_lb_ctx*);
    void (*init)(tcp_lb_ctx*);
    void (*destroy)(tcp_lb_ctx*);
} tcp_lb_ctx;

tcp_lb_ctx* get_default_tcp_lb_ctx();


#define TCP_DEFAULT_LBFUNC  NULL

#define TCPFLG_SRV_DEFAULT  0x0000

#define TCPFLG_IS_SERVER    0x0001
#define TCPFLG_IS_CLIENT    0x0002

#define TCPFLG_CLT_PASV     0x0004  /* remote client */
#define TCPFLG_CLT_PORT     0x0008  /* local  client */
#define TCPFLG_CLT_CONNING  0x0010  /* is connecting */
#define TCPFLG_CLT_CONNED   0x0020
#define TCPFLG_CLT_CONNFAIL 0x0040
#define TCPFLG_CLT_WAITCLS  0x0080  /* wait close */

#define TCPFLG_SRV_CLONEFD  0x0100
#define TCPFLG_SRV_MLOOP    0x0200  /* run in multi loop */
#define TCPFLG_SRV_LB_ON    0x0400  /* load balance on */
#define TCPFLG_SRV_RUNNING  0x0800


#define _TCP_CONNECTION         \
    int flag;                   \
    int fd;                     \
    evt_loop *loop_on;          \
    void *data;                 \
    tcp_cb on_read;             \
    tcp_cb on_write;            \
    tcp_cb on_close;            \
    tcp_connection *peer;       \
    net_addr addr;


typedef struct _tag_tcp_connection {
    _TCP_CONNECTION;
} tcp_connection;

typedef struct _tag_tcp_server {
    _TCP_CONNECTION;

    tcp_client_objpool *clt_objpool;
    ohbuffer_unit_objpool *bufu_objpool;

    atomic32_t conn_cnt;

    tcp_cb on_accept;
    evt_io accept_ev;

    evt_before accept_checkev;

    /* load balance */
    tcp_lb_ctx *lb_ctx;
    tcp_server_hub *hub;
} tcp_server;

typedef struct _tag_tcp_client {
    _TCP_CONNECTION;

    tcp_cb on_connect;

    evt_io read_ev;
    evt_io write_ev;

    ohbuffer rbuf;
    ohbuffer wbuf;

    OBJPOOL_OBJ_BASE(tcp_client)
} tcp_client;

typedef struct _tag_tcp_server_hub {
    int servers_num;
    tcp_server **servers;
} tcp_server_hub;


#define tcp_connection_set_on_accept(conn, cb)   \
    (conn)->on_accept = (cb)
#define tcp_connection_set_on_read(conn, cb)     \
    (conn)->on_read = (cb)
#define tcp_connection_set_on_write(conn, cb)    \
    (conn)->on_write = (cb)
#define tcp_connection_set_on_close(conn, cb)    \
    (conn)->on_close = (cb)
#define tcp_connection_set_on_connect(conn, cb)    \
    (conn)->on_connect = (cb)

tcp_server* tcp_server_init(net_addr*, evt_loop*, int);
tcp_server* tcp_server_clonefd(tcp_server*, evt_loop*, int);
int tcp_server_start(tcp_server*);
int tcp_server_start_lb(tcp_server*, evt_pool*, tcp_lb_ctx*);
void tcp_server_stop(tcp_server*);
void tcp_server_destroy(tcp_server*);

tcp_server_hub* tcp_server_hub_init(net_addr*, evt_pool*, int);
int tcp_server_hub_start(tcp_server_hub*);
void tcp_server_hub_stop(tcp_server_hub*);
void tcp_server_hub_set_on_accept(tcp_server_hub*, tcp_cb);
void tcp_server_hub_set_on_read(tcp_server_hub*, tcp_cb);
void tcp_server_hub_set_on_write(tcp_server_hub*, tcp_cb);
void tcp_server_hub_set_on_close(tcp_server_hub*, tcp_cb);

int tcp_client_init(tcp_client*, net_addr*, evt_loop*, int, ohbuffer_unit_objpool*, int);
int tcp_connect(tcp_client*);

int tcp_send(tcp_client*, const char*, int);
int tcp_send_buffer(tcp_client *, ohbuffer*);
int tcp_send_delay(tcp_client*, const char*, int, int);
void tcp_flush(tcp_client*);
void tcp_close(tcp_client*);


/* client pool */
#define TCPCLIENT_OBJPOOL_BLOCKSZ 256
typedef struct _tag_tcp_client_objpool {
    OBJPOOL_BASE(tcp_client)
} tcp_client_objpool;

#define tcp_client_pool_init(pool, bsz, osz) \
    objpool_init((objpool_base*)(pool), (bsz), (osz))
#define tcp_client_pool_destroy(pool) \
    objpool_destroy((objpool_base*)(pool))
#define tcp_client_pool_get(pool, lock)  \
    (tcp_client*)objpool_get_obj((objpool_base*)(pool), (lock))
#define tcp_client_pool_free(pool, elem, lock)          \
    objpool_free_obj((objpool_base*)(pool), (elem), (lock))



#ifdef __cplusplus
}
#endif
#endif  //OHEV_TCPSRV_H020