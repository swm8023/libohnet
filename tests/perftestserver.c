#include <oh.h>


atomic64_t ll = 0;
int sec = 10;
void write_cb(tcp_client* client) {
    log_info("== %d write", client->fd);
}

void read_cb(tcp_client* client) {
    // atomic_add_get(ll, n);
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

void accept_cb(tcp_client* client) {
    log_info("== %d accept", client->fd);
}

void calll_cb(evt_loop* loop, evt_timer* ev) {
    tcp_server *server = (tcp_server*)ev->data;
    log_warn("ttl => %lfMb/s client => %d", atomic_get(ll)/1024.0/1024.0/sec, 
        atomic_get(server->clt_cnt));

    atomic_add_get(ll, -ll);
}

int main() {
    set_default_logif_level(LOG_WARN);
    tcp_server *server = tcp_server_init_anyv4(8887, 0);
    tcp_server_set_on_write(server, write_cb);
    tcp_server_set_on_read(server, read_cb);
    tcp_server_set_on_close(server, close_cb);
    tcp_server_set_on_accept(server, accept_cb);

    evt_pool* pool = evt_pool_init(8);

    evt_timer calll;
    evt_timer_init(&calll, calll_cb, SECOND(sec), SECOND(sec));
    evt_set_data(&calll, server);
    evt_timer_start(pool->loops[0], &calll);

    tcp_server_bind_pool(server, pool, TCPSRV_FLG_POOL_EACH);
    evt_pool_run(pool);

    return 0;
}