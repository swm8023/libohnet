#include <string.h>
#include <errno.h>

#include <ohev/log.h>
#include <ohev/tcpsrv.h>
#include <ohev/fdoper.h>

static void tcp_server_accept(evt_loop*, evt_io*);
static void tcp_client_read(evt_loop*, evt_io*);
static void tcp_client_write(evt_loop*, evt_io*);
static void tcp_client_close(tcp_client*);

static void evt_async_start_io_clean(evt_async *ev);
static void evt_async_start_io(evt_loop *loop, evt_async *ev);
static void tcp_server_accept_check(evt_loop *loop, evt_before *ev);

tcp_server* tcp_server_init_anyv4(int port, int flag) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
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
    server->pool_on = NULL;
    memcpy(&server->addr, addr, sizeof(struct sockaddr_in));

    /* client pool */
    server->clt_cnt = 0;
    server->clt_objpool = (tcp_client_objpool*)ohmalloc(sizeof(tcp_client_objpool));
    if (server->clt_objpool == NULL) {
        log_error("tcp server client pool alloc error");
        goto tcp_server_init_failed;
    }
    tcp_clientpool_init(server->clt_objpool,
        TCPCLIENT_OBJPOOL_BLOCKSZ, sizeof(tcp_client_obj));

    /* buffer unit pool */
    server->bufu_objpool = (ohbuffer_unit_objpool*)ohmalloc(sizeof(ohbuffer_unit_objpool));
    if (server->bufu_objpool == NULL) {
        log_error("tcp server buffer unit pool alloc error");
        goto tcp_server_init_failed;
    }
    bufunitpool_init(server->bufu_objpool,
        TCPCLIENT_OBJPOOL_BLOCKSZ, OHBUFFER_UNIT_DEFAULT_SIZE + sizeof(ohbuffer_unit));

    // evt_io_init(&server->accept_ev, tcp_server_accept ,listenfd, EVTIO_READ);
    // evt_set_data(&server->accept_ev, server);

    /* init when bind with loop or pool */
    server->accept_ev = NULL;

    /* callbakcs */
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
            tcp_clientpool_destroy(server->clt_objpool);
            ohfree(server->clt_objpool);
        }
        if (server->bufu_objpool) {
            bufunitpool_destroy(server->bufu_objpool);
            ohfree(server->bufu_objpool);
        }
        ohfree(server);
    }
    return NULL;
}

int tcp_server_bind_loop(tcp_server* server, evt_loop* loop) {
    server->loop_on = loop;
    server->pool_on = NULL;
    server->flag   |= TCPSRV_FLG_LOOP_SINGLE;

    server->accept_ev = (evt_io*)ohmalloc(sizeof(evt_io));
    evt_io_init(server->accept_ev, tcp_server_accept ,server->fd, EVTIO_READ);
    evt_set_data(server->accept_ev, server);
    evt_io_start(loop, server->accept_ev);

    server->accept_checkev = (evt_before*)ohmalloc(sizeof(evt_before));
    evt_before_init(server->accept_checkev, tcp_server_accept_check);
    evt_set_data(server->accept_checkev, server);
    evt_before_start(loop, server->accept_checkev);

    return 0;
}

int tcp_server_bind_pool(tcp_server* server, evt_pool* pool, int flag) {
    int i;
    server->pool_on = pool;
    server->flag   |= flag;

    if (flag == TCPSRV_FLG_POOL_SINGLE) {
        server->loop_on = pool->lb_next_loop(pool);
        server->accept_ev = (evt_io*)ohmalloc(sizeof(evt_io));
        evt_io_init(server->accept_ev, tcp_server_accept ,server->fd, EVTIO_READ);
        evt_set_data(server->accept_ev, server);
        evt_io_start(server->loop_on, server->accept_ev);

        server->accept_checkev = (evt_before*)ohmalloc(sizeof(evt_before));
        evt_before_init(server->accept_checkev, tcp_server_accept_check);
        evt_set_data(server->accept_checkev, server);
        evt_before_start(server->loop_on, server->accept_checkev);
    } else if (flag == TCPSRV_FLG_POOL_EACH) {
        server->loop_on = NULL;
        server->accept_ev = (evt_io*)ohmalloc(sizeof(evt_io) * pool->loops_num);
        for (i = 0; i < pool->loops_num; i++) {
            evt_io_init(&server->accept_ev[i], tcp_server_accept ,server->fd, EVTIO_READ);
            evt_set_data(&server->accept_ev[i], server);
            evt_io_start(pool->loops[i], &server->accept_ev[i]);
        }

        server->accept_checkev = (evt_before*)ohmalloc(sizeof(evt_before) * sizeof(evt_before));
        for (i = 0; i < pool->loops_num; i++) {
            evt_before_init(&server->accept_checkev[i], tcp_server_accept_check);
            evt_set_data(&server->accept_checkev[i], server);
            evt_before_start(pool->loops[i], &server->accept_checkev[i]);
        }

    } else {
        log_fatal("erro server bind flag!");
        return -1;
    }
    return 0;
}

void tcp_server_accept_check(evt_loop* loop, evt_before* ev) {
    int i;
    evt_io *accept_ev = NULL;
    tcp_server *server = (tcp_server*)ev->data;

    if (server->flag & TCPSRV_FLG_POOL_EACH) {
        for (i = 0; i < server->pool_on->loops_num; i++) {
            if (ev == &server->accept_checkev[i]) {
                accept_ev = &server->accept_ev[i];
                break;
            }
        }
    } else {
        accept_ev = server->accept_ev;
    }
    if (accept_ev == NULL || accept_ev->active) {
        return;
    }

    /* restart accept ev */
    if (server->close_after_emfile) {
        evt_io_start(loop, accept_ev);
    }
}

void evt_async_start_io_clean(evt_async *ev) {
    ohfree(ev);
}

void evt_async_start_io(evt_loop *loop, evt_async *ev) {
    evt_io_start(loop, (evt_io*)ev->data);
}


void tcp_server_accept(evt_loop* loop, evt_io* ev) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);
    int clientfd = -1;

    tcp_client_obj *client_obj = NULL;
    tcp_client *client = NULL;
    tcp_server *server = (tcp_server*)ev->data;

    /* accept */
    clientfd = accept(server->fd, (struct sockaddr*)&addr, &len);

    if (clientfd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            log_warn("tcp server(%d) accept failed!", server->fd);
            /* no file descriptors, stop service several seconds */
            if (errno == EMFILE) {
                server->close_after_emfile = 0;
                evt_io_stop(loop, ev);
            }
        }
        return;
    } else {
        log_inner("tcp server(%d) accept new client(%d)", server->fd, clientfd);
    }

    fd_nonblock(clientfd);

    /* get client from object pool  */
    client_obj = tcp_clientpool_get_lock(server->clt_objpool);
    if (client_obj == NULL) {
        log_error("tcp server(%d) get client object failed!", server->fd);
        goto tcp_server_accept_failed;
    }
    client = &client_obj->client;
    memset(client, 0, sizeof(tcp_client));

    /* init connection attr */
    client->flag = 0;
    client->fd   = clientfd;
    client->data = NULL;

    /* addr, init when use */

    /* get the loop which client should run */
    if ((server->flag & TCPSRV_FLG_POOL_SINGLE) && server->pool_on) {
        client->loop_on = server->pool_on->lb_next_loop(server->pool_on);
    } else {
        client->loop_on = loop;
    }

    client->server = server;

    /* init buffer */
    buf_init(&client->rbuf, OHBUFFER_UNIT_DEFAULT_SIZE, server->bufu_objpool, OHBUFFER_UNITPOOL_LOCK);
    buf_init(&client->wbuf, OHBUFFER_UNIT_DEFAULT_SIZE, server->bufu_objpool, OHBUFFER_UNITPOOL_LOCK);

    /* init client read and write event  */
    evt_io_init(&client->read_ev, tcp_client_read, clientfd, EVTIO_READ);
    evt_set_data(&client->read_ev, client);

    evt_io_init(&client->write_ev, tcp_client_write, clientfd, EVTIO_WRITE);
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

    atomic_increment(server->clt_cnt);

    /* user callback */
    if (server->on_accept) {
        server->on_accept(client);
    }
    return;

tcp_server_accept_failed:
    if (clientfd != -1) {
        close(clientfd);
    }
}

void tcp_client_read(evt_loop* loop, evt_io* ev) {
    tcp_client *client = (tcp_client*)ev->data;
    tcp_server *server = client->server;

    int len = buf_fd_read(&client->rbuf, client->fd);
    /* read EOF */
    if (len == 0) {
        log_inner("fd(%d) read EOF.", client->fd);
        if (buf_used(&client->wbuf) == 0) {
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_stop(client->loop_on, &client->write_ev);
            tcp_client_close(client);
        /* data not send, delay close */
        } else {
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_start(client->loop_on, &client->write_ev);
            shutdown(client->fd, SHUT_RD);
            client->flag |= TCPCLIENT_FLAG_WAITCLOSE;
        }
    /* read ERROR */
    } else if (len < 0) {
        log_inner("fd(%d) read ERROR.", client->fd);
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_stop(client->loop_on, &client->write_ev);
            tcp_client_close(client);
        }
    /* call user callback */
    } else {
        if (server->on_read) {
            server->on_read(client);
        }
    }
}

void tcp_client_write(evt_loop* loop, evt_io* ev) {
    tcp_client *client = (tcp_client*)ev->data;
    tcp_server *server = client->server;

    int len = buf_fd_write(&client->wbuf, client->fd);
    /* write ERROR */
    if (len < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            log_inner("fd(%d) write ERROR", client->fd);
            evt_io_stop(client->loop_on, &client->read_ev);
            evt_io_stop(client->loop_on, &client->write_ev);
            tcp_client_close(client);
        }
    } else {
        /* user callback */
        if (server->on_write) {
            server->on_write(client);
        }
        /* stop ev and check whether the client need be closed */
        if (buf_used(&client->wbuf) == 0) {
            evt_io_stop(client->loop_on, &client->write_ev);
            if (client->flag & TCPCLIENT_FLAG_WAITCLOSE) {
                tcp_client_close(client);
            }
        }
    }
}

void tcp_client_close(tcp_client* client) {
    tcp_server *server = client->server;

    /* user callback */
    if (server->on_close) {
        server->on_close(client);
    }

    atomic_decrement(server->clt_cnt);
    log_inner("tcp server(%d) close client(%d)", server->fd, client->fd);

    close(client->fd);
    buf_destroy(&client->rbuf);
    buf_destroy(&client->wbuf);

    tcp_client_obj *client_obj = container_of(client, tcp_client_obj, client);
    tcp_clientpool_free_lock(server->clt_objpool, client_obj);

    /* emfile flag */
    server->close_after_emfile++;
}

int tcp_send(tcp_client* client, const char* src, int srcsz) {
    tcp_server *server = client->server;

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

    tcp_client_close(client);
}
