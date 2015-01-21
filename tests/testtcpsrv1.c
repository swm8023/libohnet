#include <oh.h>

void write_cb(tcp_client* client) {
    log_info("== %d write", client->fd);
}

void read_cb(tcp_client* client) {
    ohbuffer *rbuf = &client->rbuf;
    char buf[65536];
    int n = buf_readall(rbuf, buf, 65536);
    tcp_send(client, buf, n);
}

void close_cb(tcp_client* client) {
    log_info("== %d close", client->fd);
}

void accept_cb(tcp_client* client) {
    log_info("== %d accept", client->fd);
}

void timer_cb(evt_loop* loop, evt_timer* ev) {
    evt_timer_stop(loop, ev);
}
int main() {
    // tcp_server *server = tcp_server_init_anyv4(8888, 0);
    // tcp_server_set_on_write(server, write_cb);
    // tcp_server_set_on_read(server, read_cb);
    // tcp_server_set_on_close(server, close_cb);
    // tcp_server_set_on_accept(server, accept_cb);
    // evt_pool* pool = evt_pool_init(1);
    // tcp_server_bind_pool(server, pool, TCPSRV_FLG_POOL_EACH);
    // evt_pool_run(pool);
    evt_loop* loop = evt_loop_init();
    evt_timer tm;
    evt_timer_init(&tm, timer_cb, SECOND(1), 0);
    evt_timer_start(loop, &tm);

    evt_loop_run(loop);
    return 0;
}