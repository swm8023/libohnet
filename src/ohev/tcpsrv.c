#include <string.h>

#include <ohev/log.h>
#include <ohev/tcpsrv.h>
#include <ohev/fdoper.h>

static void tcp_server_accept(evt_loop*, evt_io*);
static void tcp_client_read(evt_loop*, evt_io*);
static void tcp_client_write(evt_loop*, evt_io*);
static void tcp_client_close(tcp_client*);

static void evt_async_start_io_clean(evt_async *ev);
static void evt_async_start_io(evt_loop *loop, evt_async *ev);

tcp_server* tcp_server_init_anyv4(int port, int flag) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htonl(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return tcp_server_init_addrv4((struct sockaddr*)&addr, flag);
}

tcp_server* tcp_server_init_strv4(const char* addr_str, int port, int flag) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (1 != inet_pton(AF_INET, addr_str, &addr.sin_addr)) {
        log_error("inet_pton error, can't init tcp server");
        return NULL;
    }
    return tcp_server_init_addrv4((struct sockaddr*)&addr, flag);
}

tcp_server* tcp_server_init_addrv4(struct sockaddr *addr, int flag) {
    int listenfd = -1;
    tcp_server *server = NULL;

    /* socket -> bind -> listen */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        log_error("tcp server bind error");
        goto tcp_server_init_failed;
    }

    fd_reuse(listenfd);
    fd_nonblock(listenfd);

    if (bind(listenfd, addr, sizeof(struct sockaddr_in)) < 0) {
        log_error("tcp server bind error");
        goto tcp_server_init_failed;
    }

    if (listen(listenfd, 1024) < 0) {
        log_error("tcp server listen error");
        goto tcp_server_init_failed;
    }

    /* allocate server */
    server = (tcp_server*)ohmalloc(sizeof(tcp_server));
    memset(server, 0, sizeof(tcp_server));

    /* connection attr */
    server->flag = flag;
    server->fd   = listenfd;
    server->data = NULL;
    server->loop_on = NULL;
    memcpy(&server->addr, addr, sizeof(struct sockaddr_in));

    /* tcp server attr */
    server->clt_cnt = 0;
    server->clt_objpool = (tcp_client_objpool*)ohmalloc(sizeof(tcp_client_objpool));
    if (server->clt_objpool == NULL) {
        log_error("tcp server client pool init error");
        goto tcp_server_init_failed;
    }
    tcp_clientpool_init(server->clt_objpool,
        TCPCLIENT_OBJPOOL_BLOCKSZ, sizeof(tcp_client_obj));

    evt_io_init(&server->accept_ev, tcp_server_accept ,listenfd, EVTIO_READ);
    evt_set_data(&server->accept_ev, server);

    server->on_accept = NULL;
    server->on_read   = NULL;
    server->on_write  = NULL;
    server->on_close  = NULL;

    log_inner("tcp server(%d) init success, listen at %d",
        listenfd, ntohs(((struct sockaddr_in*)addr)->sin_port));
    return server;

tcp_server_init_failed:
    if (listenfd != -1) {
        close(listenfd);
    }
    if (server != NULL) {
        if (server->clt_objpool) {
            ohfree(server->clt_objpool);
        }
        ohfree(server);
    }
    return NULL;
}

void evt_async_start_io_clean(evt_async *ev) {
    ohfree(ev);
}

void evt_async_start_io(evt_loop *loop, evt_async *ev) {
    evt_io_start(loop, (evt_io*)ev->data);
}

void tcp_server_accept(evt_loop* lopp, evt_io* ev) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);
    int clientfd = -1;

    tcp_client_obj *client_obj = NULL;
    tcp_client *client = NULL;
    tcp_server *server = (tcp_server*)ev->data;

    /* accept */
    clientfd = accept(server->fd, (struct sockaddr*)&addr, &len);
    if (clientfd < 0) {
        log_warn("tcp server(%d) accept failed!");
        return;
    } else {
        log_inner("tcp server(%d) accept new client(%d)", server->fd, clientfd);
    }

    fd_nonblock(clientfd);

    /* get client from object pool  */
    client_obj = tcp_clientpool_get(server->clt_objpool);
    if (client_obj == NULL) {
        log_error("tcp server(%d) get client object failed", server->fd);
        goto tcp_server_accept_failed;
    }
    client = &client_obj->client;
    memset(client, 0, sizeof(tcp_client));

    /* init connection attr */
    client->flag = 0;
    client->fd   = clientfd;
    /* addr */
    if ((server->flag & TCPSERVER_FLAG_POOL)) {

    } else {
        client->loop_on = server->loop_on;
    }

    /* init client attr */
    evt_io_init(&client->read_ev, tcp_client_read, clientfd, EVTIO_READ);
    evt_set_data(&client->read_ev, client);

    evt_io_init(&client->read_ev, tcp_client_write, clientfd, EVTIO_WRITE);
    evt_set_data(&client->write_ev, client);

    /* get default buffer */

    /* only start read event */
    if (client->loop_on->owner_tid == 0 ||
        client->loop_on->owner_tid == thread_tid()) {
        evt_io_start(client->loop_on, &client->read_ev);
    } else {
        evt_async *ev = ohmalloc(sizeof(evt_async));
        evt_async_init(ev, evt_async_start_io, evt_async_start_io_clean);
        evt_set_data(ev, &client->read_ev);
        evt_async_start(client->loop_on, ev);
        evt_loop_wakeup(client->loop_on);
    }

    atomic_increment(server->clt_cnt);
    return;

tcp_server_accept_failed:
    if (clientfd != -1) {
        close(clientfd);
    }
}

void tcp_client_read(evt_loop* loop, evt_io* ev) {

}

void tcp_client_write(evt_loop* loop, evt_io* ev) {

}

void tcp_client_close(tcp_client* clt) {

}