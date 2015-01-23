#include <string.h>
#include <errno.h>

#include <ohev/log.h>
#include <ohev/tcpsrv.h>
#include <ohev/fdoper.h>

static void tcp_cb_accept(evt_loop*, evt_io*);
static void tcp_cb_read(evt_loop*, evt_io*);
static void tcp_cb_write(evt_loop*, evt_io*);
static void tcp_cb_close(tcp_client*);

static void evt_async_start_io_clean(evt_async *ev);
static void evt_async_start_io(evt_loop *loop, evt_async *ev);
static void tcp_server_accept_check(evt_loop *loop, evt_before *ev);

static int tcp_server_init_finish(tcp_server*, int, net_addr *, evt_loop* , int);

int tcp_server_init_finish(tcp_server* server, int fd, net_addr *addr, evt_loop* loop, int flag) {
    /* connection attr */
    server->flag     = flag | TCPFLG_IS_SERVER;
    server->fd       = fd;
    server->loop_on  = loop;
    server->data     = NULL;
    server->on_read  = NULL;
    server->on_write = NULL;
    server->on_close = NULL;
    server->peer     = NULL;
    memcpy(&server->addr, addr, sizeof(net_addr));

    /* client pool */
    server->conn_cnt = 0;
    server->clt_objpool = (tcp_client_objpool*)ohmalloc(sizeof(tcp_client_objpool));
    if (server->clt_objpool == NULL) {
        log_error("tcp server client pool alloc error");
        goto tcp_server_init_finish_failed;
    }
    tcp_client_pool_init(server->clt_objpool,
        TCPCLIENT_OBJPOOL_BLOCKSZ, sizeof(tcp_client));

    /* buffer unit pool */
    server->bufu_objpool = (ohbuffer_unit_objpool*)ohmalloc(sizeof(ohbuffer_unit_objpool));
    if (server->bufu_objpool == NULL) {
        log_error("tcp server buffer unit pool alloc error");
        goto tcp_server_init_finish_failed;
    }
    bufunit_pool_init(server->bufu_objpool,
        TCPCLIENT_OBJPOOL_BLOCKSZ, OHBUFFER_UNIT_DEFAULT_SIZE + sizeof(ohbuffer_unit));

    evt_io_init(&server->accept_ev, tcp_cb_accept ,fd, EVTIO_READ);
    evt_set_data(&server->accept_ev, server);

    server->lb_ctx = NULL;

    return 0;

tcp_server_init_finish_failed:
    if (server != NULL) {
        if (server->clt_objpool) {
            tcp_client_pool_destroy(server->clt_objpool);
            ohfree(server->clt_objpool);
        }
        if (server->bufu_objpool) {
            bufunit_pool_destroy(server->bufu_objpool);
            ohfree(server->bufu_objpool);
        }
    }
    return -1;
}


tcp_server* tcp_server_init(net_addr* addr, evt_loop* loop, int flag) {
    int listenfd = -1;
    tcp_server *server = NULL;

    /* allocate server */
    server = (tcp_server*)ohmalloc(sizeof(tcp_server));
    if (server == NULL) {
        goto tcp_server_init_failed;
    }

    memset(server, 0, sizeof(tcp_server));

    /* socket -> bind  && listen when start  */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        log_error("tcp server bind error");
        goto tcp_server_init_failed;
    }

    fd_reuse(listenfd);
    fd_nonblock(listenfd);

    if (bind(listenfd, netaddr_to_sockaddr(addr), sizeof(struct sockaddr_in)) < 0) {
        log_error("tcp server bind error");
        goto tcp_server_init_failed;
    }

    if (tcp_server_init_finish(server, listenfd, addr, loop, flag)) {
        goto tcp_server_init_failed;
    }

    log_inner("tcp server(%d) init success, listen at %d",
        listenfd, ntohs(((struct sockaddr_in*)netaddr_to_sockaddr(addr))->sin_port));
    return server;

tcp_server_init_failed:
    if (listenfd != -1) {
        close(listenfd);
    }
    if (server) {
        ohfree(server);
    }
    return NULL;
}

tcp_server* tcp_server_clonefd(tcp_server* fserver, evt_loop* loop, int flag) {
    tcp_server *server = NULL;

    /* allocate server */
    server = (tcp_server*)ohmalloc(sizeof(tcp_server));
    if (server == NULL) {
        goto tcp_server_clonefd_failed;
    }

    memset(server, 0, sizeof(tcp_server));

    if (tcp_server_init_finish(server, fserver->fd, &fserver->addr, loop, 
        flag | TCPFLG_SRV_CLONEFD)) {
        goto tcp_server_clonefd_failed;
    }

    log_inner("tcp server(%d) init success, listen at %d",
        fserver->fd, ntohs(((sockaddr_in*)netaddr_to_sockaddr(&fserver->addr))->sin_port));
    return server;

tcp_server_clonefd_failed:
    if (server) {
        ohfree(server);
    }
    return NULL;
}

int tcp_server_start(tcp_server *server) {
    return tcp_server_start_lb(server, NULL, NULL);
}

int tcp_server_start_lb(tcp_server *server, evt_pool* pool, tcp_lb_ctx* lb_ctx) {
    server->flag &= ~TCPFLG_SRV_MLOOP;

    /* use load balance */
    if (pool != NULL && lb_ctx != NULL) {
        /* error! lb_next_loop must be set */
        if (lb_ctx->next_loop == NULL) {
            log_error("load balance on but no callback to get next loop.");
        } else {
            server->lb_ctx = lb_ctx;
            lb_ctx->pool = pool;
            if (lb_ctx->init) {
                lb_ctx->init(lb_ctx);
            }
            /* fix server flag */
            server->flag |= TCPFLG_SRV_MLOOP;
        }
    }

    /* listen and start accept event */
    if ((server->flag & TCPFLG_SRV_CLONEFD) == 0) {
        if (listen(server->fd, 1024) < 0) {
            log_error("tcp server listen error");
            return -1;
        }
    }
    evt_io_start(server->loop_on, &server->accept_ev);
    return 0;
}

void tcp_server_stop(tcp_server* server) {

}

void tcp_server_destroy(tcp_server* server) {

}

tcp_server_hub* tcp_server_hub_init(net_addr* addr, evt_pool* pool, int flag) {
    int i;
    tcp_server_hub* server_hub = (tcp_server_hub*)ohmalloc(sizeof(tcp_server_hub));
    if (server_hub == NULL) {
        goto tcp_server_hub_init_failed;
    }
    memset(server_hub, 0, sizeof(tcp_server_hub));

    server_hub->servers_num = pool->loops_num;
    server_hub->servers = (tcp_server**)ohmalloc(sizeof(tcp_server*) * pool->loops_num);

    for (i = 0; i < pool->loops_num; i++) {
        server_hub->servers[i] = ( i == 0 ?
            tcp_server_init(addr, pool->loops[i], flag) :
            tcp_server_clonefd(server_hub->servers[0], pool->loops[i], flag));
        if (server_hub->servers[i] == NULL) {
            goto tcp_server_hub_init_failed;
        }
    }

    return server_hub;

tcp_server_hub_init_failed:
    log_error("tcp server hub init failed!");
    if (server_hub != NULL) {
        ohfree(server_hub->servers);
        for (i = 0; i < pool->loops_num; i++) {
            if (server_hub->servers[i]) {
                tcp_server_destroy(server_hub->servers[i]);
            }
        }
    }
    ohfree(server_hub);
    return NULL;
}

int tcp_server_hub_start(tcp_server_hub* server_hub) {
    int i;
    for (i = 0; i < server_hub->servers_num; i++) {
        tcp_server_start(server_hub->servers[i]);
    }
}

void tcp_server_hub_set_on_accept(tcp_server_hub* server_hub, tcp_cb cb) {
    int i;
    for (i = 0; i < server_hub->servers_num; i++) {
        server_hub->servers[i]->on_accept = cb;
    }
}

void tcp_server_hub_set_on_read(tcp_server_hub* server_hub, tcp_cb cb) {
    int i;
    for (i = 0; i < server_hub->servers_num; i++) {
        server_hub->servers[i]->on_read = cb;
    }
}

void tcp_server_hub_set_on_write(tcp_server_hub* server_hub, tcp_cb cb) {
    int i;
    for (i = 0; i < server_hub->servers_num; i++) {
        server_hub->servers[i]->on_write = cb;
    }
}

void tcp_server_hub_set_on_close(tcp_server_hub* server_hub, tcp_cb cb) {
    int i;
    for (i = 0; i < server_hub->servers_num; i++) {
        server_hub->servers[i]->on_close = cb;
    }
}



void evt_async_start_io_clean(evt_async *ev) {
    ohfree(ev);
}

void evt_async_start_io(evt_loop *loop, evt_async *ev) {
    evt_io_start(loop, (evt_io*)ev->data);
}

int tcp_client_init(tcp_client *client, net_addr* addr, evt_loop* loop, int flag, 
    ohbuffer_unit_objpool *bpool, int buflock) {
    int clientfd = -1;

    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        return -1;
    }

    fd_nonblock(clientfd);

    memset(client, 0, sizeof(tcp_client));
    client->flag     = TCPFLG_CLT_PORT | TCPFLG_IS_CLIENT;
    client->fd       = clientfd;
    client->data     = NULL;
    client->on_read  = NULL;
    client->on_write = NULL;
    client->on_close = NULL;
    client->peer     = NULL;
    client->loop_on  = loop;
    memcpy(&client->addr, addr, sizeof(net_addr));

    if (bpool != NULL) {
        buf_init(&client->rbuf, OHBUFFER_UNIT_DEFAULT_SIZE, bpool, buflock);
        buf_init(&client->wbuf, OHBUFFER_UNIT_DEFAULT_SIZE, bpool, buflock);
    } else {
        buf_init(&client->rbuf, OHBUFFER_UNIT_DEFAULT_SIZE, NULL, 0);
        buf_init(&client->wbuf, OHBUFFER_UNIT_DEFAULT_SIZE, NULL, 0);
    }

    evt_io_init(&client->read_ev, tcp_cb_read, clientfd, EVTIO_READ);
    evt_set_data(&client->read_ev, client);

    evt_io_init(&client->write_ev, tcp_cb_write, clientfd, EVTIO_WRITE);
    evt_set_data(&client->write_ev, client);

    log_inner("tcp client(%d) init success, listen at %d", clientfd);
    return 0;
}

int tcp_connect(tcp_client* client) {
    if (client->flag & (TCPFLG_CLT_CONNING | TCPFLG_CLT_CONNED)) {
        return -1;
    }
    int r = -1;
    do {
        r = connect(client->fd, netaddr_to_sockaddr(&client->addr), sizeof(sockaddr_in));
    } while (r == -1 && errno == EINTR);
    /* success */
    if (r >= 0 || (r == -1 && errno == EINPROGRESS)) {
        client->flag |= TCPFLG_CLT_CONNING;
        evt_io_start(client->loop_on, &client->write_ev);
        return 0;
    }
    /* failed */
    return -1;
}

void tcp_cb_accept(evt_loop* loop, evt_io* ev) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);
    int clientfd = -1;

    tcp_client *client = NULL;
    tcp_server *server = (tcp_server*)ev->data;

    /* accept */
    clientfd = accept(server->fd, (struct sockaddr*)&addr, &len);

    if (clientfd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            log_warn("tcp server(%d) accept failed!", server->fd);
            /* no file descriptors, stop service several seconds */
            if (errno == EMFILE) {
                //server->close_after_emfile = 0;
            }
        }
        return;
    } else {
        log_inner("tcp server(%d) accept new client(%d)", server->fd, clientfd);
    }

    fd_nonblock(clientfd);

    /* get client from object pool, don't need lock  */
    if (server->flag & TCPFLG_SRV_MLOOP) {
        client = tcp_client_pool_get(server->clt_objpool, OBJPOOL_LOCK);
    } else {
        client = tcp_client_pool_get(server->clt_objpool, OBJPOOL_NOLOCK);
    }
    if (client == NULL) {
        log_error("tcp server(%d) get client object failed!", server->fd);
        goto tcp_cb_accept_failed;
    }
    memset(client, 0, sizeof(tcp_client));

    /* init connection attr */
    client->flag     = TCPFLG_CLT_PASV | TCPFLG_IS_CLIENT;
    client->fd       = clientfd;
    client->data     = NULL;
    client->on_read  = server->on_read;
    client->on_write = server->on_write;
    client->on_close = server->on_close;
    client->peer     = (tcp_connection*)server;
    memset(&client->addr, 0, sizeof(net_addr));

    /* get the loop which client should run */
    if (server->flag & TCPFLG_SRV_MLOOP) {
        client->loop_on = server->lb_ctx->next_loop(server->lb_ctx);
    } else {
        client->loop_on = loop;
    }

    /* init buffer */
    if (server->flag & TCPFLG_SRV_MLOOP) {
        buf_init(&client->rbuf, OHBUFFER_UNIT_DEFAULT_SIZE,
            server->bufu_objpool, OHBUFFER_UNITPOOL_LOCK);
        buf_init(&client->wbuf, OHBUFFER_UNIT_DEFAULT_SIZE,
            server->bufu_objpool, OHBUFFER_UNITPOOL_LOCK);
    } else {
        buf_init(&client->rbuf, OHBUFFER_UNIT_DEFAULT_SIZE,
            server->bufu_objpool, OHBUFFER_UNITPOOL_NOLOCK);
        buf_init(&client->wbuf, OHBUFFER_UNIT_DEFAULT_SIZE,
            server->bufu_objpool, OHBUFFER_UNITPOOL_NOLOCK);
    }


    /* init client read and write event  */
    evt_io_init(&client->read_ev, tcp_cb_read, clientfd, EVTIO_READ);
    evt_set_data(&client->read_ev, client);

    evt_io_init(&client->write_ev, tcp_cb_write, clientfd, EVTIO_WRITE);
    evt_set_data(&client->write_ev, client);

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

    atomic_increment(server->conn_cnt);

    /* user callback */
    if (server->on_accept) {
        server->on_accept(client);
    }
    return;

tcp_cb_accept_failed:
    if (clientfd != -1) {
        close(clientfd);
    }
}

void tcp_cb_read(evt_loop* loop, evt_io* ev) {
    tcp_client *client = (tcp_client*)ev->data;

    int len = buf_fd_read(&client->rbuf, client->fd);
    /* read EOF */
    if (len == 0) {
        log_inner("fd(%d) read EOF.", client->fd);
        if (buf_used(&client->wbuf) == 0) {
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_stop(client->loop_on, &client->write_ev);
            tcp_cb_close(client);
        /* data not send, delay close */
        } else {
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_start(client->loop_on, &client->write_ev);
            shutdown(client->fd, SHUT_RD);
            client->flag |= TCPFLG_CLT_WAITCLS;
        }
    /* read ERROR */
    } else if (len < 0) {
        log_inner("fd(%d) read ERROR.", client->fd);
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_stop(client->loop_on, &client->write_ev);
            tcp_cb_close(client);
        }
    /* call user callback */
    } else {
        if (client->on_read) {
            client->on_read(client);
        }
    }
}

void tcp_cb_write(evt_loop* loop, evt_io* ev) {
    tcp_client *client = (tcp_client*)ev->data;

    /* maybe conneting */
    if (client->flag & TCPFLG_CLT_CONNING) {
        int error = 0;
        int errorsz = sizeof(int);
        /* get error */
        getsockopt(client->fd, SOL_SOCKET, SO_ERROR, &error, &errorsz);

        /* still in process */
        if (error == EINPROGRESS) {
            return;
        }

        /* error or no write data */
        if (error || buf_used(&client->wbuf) == 0) {
            evt_io_stop(client->loop_on, &client->write_ev);
        }


        /* fix flag */
        client->flag &= ~TCPFLG_CLT_CONNING;
        client->flag |= (error ? TCPFLG_CLT_CONNFAIL : TCPFLG_CLT_CONNED);

        /* if no error start read */
        if (error == 0) {
            evt_io_start(client->loop_on, &client->read_ev);
        }
        /* user callback */
        if (client->on_connect) {
            client->on_connect(client);
        }
        return;
    }

    int len = buf_fd_write(&client->wbuf, client->fd);
    /* write ERROR */
    if (len < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            log_inner("fd(%d) write ERROR", client->fd);
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_stop(client->loop_on, &client->write_ev);
            tcp_cb_close(client);
        }
    } else {
        /* user callback */
        if (client->on_write) {
            client->on_write(client);
        }
        /* stop ev and check whether the client need be closed */
        if (buf_used(&client->wbuf) == 0) {
            evt_io_stop(client->loop_on, &client->write_ev);
            if (client->flag & TCPFLG_CLT_WAITCLS) {
                tcp_cb_close(client);
            }
        }
    }
}

void tcp_cb_close(tcp_client* client) {
    /* user callback */
    if (client->on_close) {
        client->on_close(client);
    }

    close(client->fd);
    buf_destroy(&client->rbuf);
    buf_destroy(&client->wbuf);

    /* if it is a remote client, then peer is server */
    if (client->flag & TCPFLG_CLT_PASV) {
        tcp_server *server = (tcp_server*)client->peer;
        atomic_decrement(server->conn_cnt);
        log_inner("tcp server(%d) close client(%d)", server->fd, client->fd);
        if (server->flag & TCPFLG_SRV_MLOOP) {
            tcp_client_pool_free(server->clt_objpool, client, OBJPOOL_LOCK);
        } else {
            tcp_client_pool_free(server->clt_objpool, client, OBJPOOL_NOLOCK);
        }
    }
}

int tcp_send(tcp_client* client, const char* src, int srcsz) {
    /* try directly send first if no data in send buffer */
    int len = 0;
    if (buf_used(&client->wbuf) == 0) {
        len = write(client->fd, src, srcsz);
        if (len == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                return -1;
            }
            len = tcp_send_delay(client, src, srcsz, 0);
        } else if (len < srcsz) {
            len += tcp_send_delay(client, src + len, srcsz - len, 0);
        }
    /* use buffer */
    } else {
        len = tcp_send_delay(client, src, srcsz, 0);
    }
    return len;
}

int tcp_send_delay(tcp_client* client, const char* src, int srcsz, int lowmask) {
    /* lowmask >=  0 -> send if buf_used > lowmask
     * lowmask == -1 -> don't send
     */
    int sz = buf_write(&client->wbuf, src, srcsz);
    if (lowmask >= 0 && buf_used(&client->wbuf) > lowmask) {
        evt_io_start(client->loop_on, &client->write_ev);
    }
    return sz;
}

int tcp_send_buffer(tcp_client *client, ohbuffer* buf) {
    return buf_fd_write(buf, client->fd);
}

void tcp_flush(tcp_client* client) {
    evt_io_start(client->loop_on, &client->write_ev);
}

void tcp_close(tcp_client* client) {
    evt_io_stop(client->loop_on, &client->read_ev);
    evt_io_stop(client->loop_on, &client->write_ev);

    tcp_cb_close(client);
}

sockaddr* netaddr_to_sockaddr(net_addr* addr) {
    return (sockaddr*)&addr->addr;
}

int netaddr_init_v4(net_addr* naddr, char* ip, int port) {
    sockaddr_in *addr = (sockaddr_in*)&naddr->addr;
    memset(addr, 0, sizeof(sockaddr_in));

    addr->sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr->sin_addr);
    addr->sin_port = htons(port);

    return 0;
}

/* round-robin way to get next loop int pool */
static void lb_rr_next_loop_init(tcp_lb_ctx*);
static void lb_rr_next_loop_destroy(tcp_lb_ctx*);
static evt_loop* lb_rr_next_loop(tcp_lb_ctx*);

void lb_rr_init(tcp_lb_ctx* lb_ctx) {
    lb_ctx->data = ohmalloc(sizeof(int));
    *(int*)(lb_ctx->data) = lb_ctx->pool->loops_num - 1;;
}

void lb_rr_destroy(tcp_lb_ctx* lb_ctx) {
    ohfree(lb_ctx->data);
}

evt_loop* lb_rr_next_loop(tcp_lb_ctx* lb_ctx) {
    int pre_loop_ind = *(int*)(lb_ctx->data);
    *(int*)(lb_ctx->data) = (pre_loop_ind + 1) % lb_ctx->pool->loops_num;
    return lb_ctx->pool->loops[*(int*)(lb_ctx->data)];
}

tcp_lb_ctx* get_default_tcp_lb_ctx() {
    tcp_lb_ctx *lb_ctx = (tcp_lb_ctx*)ohmalloc(sizeof(tcp_lb_ctx));
    lb_ctx->data      = NULL;
    lb_ctx->pool      = NULL;
    lb_ctx->init      = lb_rr_init;
    lb_ctx->destroy   = lb_rr_destroy;
    lb_ctx->next_loop = lb_rr_next_loop;

    return lb_ctx;
}