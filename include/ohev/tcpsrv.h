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

#define TCPSRV_FLG_POOL_SINGLE  0x01
#define TCPSRV_FLG_LOOP_SINGLE  0x02
#define TCPSRV_FLG_POOL_EACH    0x04

#define TCPCLIENT_FLAG_WAITCLOSE 0x01


typedef void (*tcp_server_cb)(tcp_client*);

typedef struct _tag_tcp_server {
    _TCP_CONNECTION;

    evt_pool *pool_on;
    tcp_client_objpool *clt_objpool;
    ohbuffer_unit_objpool *bufu_objpool;

    atomic32_t clt_cnt;

    evt_io *accept_ev;
    evt_before *accept_checkev;
    int close_after_emfile;

    tcp_server_cb on_accept;
    tcp_server_cb on_read;
    tcp_server_cb on_write;
    tcp_server_cb on_close;
} tcp_server;

tcp_server* tcp_server_init_anyv4(int, int);
tcp_server* tcp_server_init_strv4(const char* , int, int);
tcp_server* tcp_server_init_addrv4(struct sockaddr*, int);

int tcp_server_bind_loop(tcp_server*, evt_loop*);
int tcp_server_bind_pool(tcp_server*, evt_pool*, int);

#define tcp_server_set_on_accept(server, cb)   \
    (server)->on_accept = (cb)
#define tcp_server_set_on_read(server, cb)     \
    (server)->on_read = (cb)
#define tcp_server_set_on_write(server, cb)    \
    (server)->on_write = (cb)
#define tcp_server_set_on_close(server, cb)    \
    (server)->on_close = (cb)

int tcp_send(tcp_client*, const char*, int);
int tcp_send_buffer(tcp_client *, ohbuffer*);
int tcp_send_delay(tcp_client*, const char*, int, int);
void tcp_flush(tcp_client*);
void tcp_close(tcp_client*);

#define TCPCLIENT_OBJPOOL_BLOCKSZ 256

typedef struct _tag_tcp_client {
    _TCP_CONNECTION;

    evt_io read_ev;
    evt_io write_ev;

    tcp_server* server;
    ohbuffer rbuf;
    ohbuffer wbuf;
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
#define tcp_clientpool_destroy(pool) \
    objpool_destroy((objpool_base*)(pool))
#define tcp_clientpool_get(pool)  \
    (tcp_client_obj*)objpool_get_obj((objpool_base*)(pool))
#define tcp_clientpool_free(pool, elem)     \
    objpool_free_obj((objpool_base*)(pool), elem)
#define tcp_clientpool_get_lock(pool)  \
    (tcp_client_obj*)objpool_get_obj_lock((objpool_base*)(pool))
#define tcp_clientpool_free_lock(pool, elem)     \
    objpool_free_obj_lock((objpool_base*)(pool), elem)


#ifdef __cplusplus
}
#endif
#endif  //OHEV_TCPSRV_H020