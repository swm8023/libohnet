#include <oh.h>


atomic64_t ll = 0;
int sec = 10;
void write_cb(tcp_client* client) {
    log_info("== %d write", client->fd);
}

void read_cb(tcp_client* client) {
    log_info("==%d read", client->fd);
    // atomic_add_get(ll, buf_used(&client->rbuf));
    // tcp_send_buffer(client, &client->rbuf);
    ohbuffer *rbuf = &client->rbuf;
    char buf[65536];
    int n = buf_readall(rbuf, buf, 65536);
    atomic_add_get(ll, n);
    tcp_send(client, buf, n);
}

void close_cb(tcp_client* client) {
    log_info("== %d close", client->fd);
}

void connect_cb(tcp_client* client) {
    if (client->flag & TCPFLG_CLT_CONNFAIL) {
        log_error("connect error");
        tcp_close(client);
        return;
    }
    log_info("== %d connect", client->fd);
    char buf[65536];
    tcp_send(client, buf, 16 * 4096);
}

void calll_cb(evt_loop* loop, evt_timer* ev) {
    log_warn("ttl => %lfMb/s", atomic_get(ll)/1024.0/1024.0/sec);

    atomic_add_get(ll, -ll);
}

int main() {
    set_default_logif_level(LOG_WARN);
    evt_pool *pool = evt_pool_init(10);

    net_addr addr;
    netaddr_init_v4(&addr, "127.0.0.1", 8887);

    ohbuffer_unit_objpool *upool[10];
    int i;
    for (i = 0; i < 10; i++) {
        upool[i] = (ohbuffer_unit_objpool*)ohmalloc(sizeof(ohbuffer_unit_objpool));
        bufunit_pool_init(upool[i], TCPCLIENT_OBJPOOL_BLOCKSZ, OHBUFFER_UNIT_DEFAULT_SIZE + sizeof(ohbuffer_unit));
    }
    for (i = 0; i < 1000; i++) {
        evt_loop *loop = pool->loops[i%10];
        tcp_client *client = (tcp_client*)ohmalloc(sizeof(tcp_client));
        tcp_client_init(client, &addr, loop, 0, upool[i%10], OHBUFFER_UNITPOOL_NOLOCK);
        tcp_connection_set_on_write(client, write_cb);
        tcp_connection_set_on_read(client, read_cb);
        tcp_connection_set_on_close(client, close_cb);
        tcp_connection_set_on_connect(client, connect_cb);
        if (tcp_connect(client) < 0) {
            log_error("error");
        }
    }
    evt_timer calll;
    evt_timer_init(&calll, calll_cb, SECOND(sec), SECOND(sec));
    evt_timer_start(pool->loops[0], &calll);

    // tcp_server_start(server);

    evt_pool_run(pool);


    // int i, n = 4;
    // evt_pool *pool = evt_pool_init(n);
    // tcp_server_hub *serverh = tcp_server_hub_init(&addr, pool, 0);
    // tcp_server_hub_set_on_write(serverh, write_cb);
    // tcp_server_hub_set_on_read(serverh, read_cb);
    // tcp_server_hub_set_on_close(serverh, close_cb);
    // tcp_server_hub_set_on_accept(serverh, accept_cb);
    // tcp_server_hub_start(serverh);

    // evt_timer calll;
    // evt_timer_init(&calll, calll_cb, SECOND(sec), SECOND(sec));
    // evt_set_data(&calll, serverh);
    // evt_timer_start(pool->loops[0], &calll);
    // evt_pool_run(pool);

    return 0;
}