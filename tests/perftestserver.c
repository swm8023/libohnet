#include <oh.h>


atomic64_t ll = 0;
int sec = 10;
void write_cb(tcp_client* client) {
    log_info("== %d write", client->fd);
}

void read_cb(tcp_client* client) {
    log_info("== %d read", client->fd);
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

void accept_cb(tcp_client* client) {
    log_info("== %d accept", client->fd);
}

void calll_cb(evt_loop* loop, evt_timer* ev) {
    tcp_server_hub *sh = (tcp_server_hub*)ev->data;
    log_warn("ttl => %lfMb/s client => %d %d %d %d", atomic_get(ll)/1024.0/1024.0/sec, 
        atomic_get(sh->servers[0]->conn_cnt),
        atomic_get(sh->servers[1]->conn_cnt),
        atomic_get(sh->servers[2]->conn_cnt),
        atomic_get(sh->servers[3]->conn_cnt));

    atomic_add_get(ll, -ll);
}

int main() {
    set_default_logif_level(LOG_WARN);
    evt_loop *loop = evt_loop_init();

    net_addr addr;
    netaddr_init_v4(&addr, "0.0.0.0", 8887);

    // tcp_server *server = tcp_server_init(&addr, loop, 0);
    // tcp_connection_set_on_write(server, write_cb);
    // tcp_connection_set_on_read(server, read_cb);
    // tcp_connection_set_on_close(server, close_cb);
    // tcp_connection_set_on_accept(server, accept_cb);


    // evt_timer calll;
    // evt_timer_init(&calll, calll_cb, SECOND(sec), SECOND(sec));
    // evt_set_data(&calll, server);
    // evt_timer_start(loop, &calll);

    // tcp_server_start(server);
    // evt_loop_run(loop);


    int i, n = 4;
    evt_pool *pool = evt_pool_init(n);
    tcp_server_hub *serverh = tcp_server_hub_init(&addr, pool, 0);
    tcp_server_hub_set_on_write(serverh, write_cb);
    tcp_server_hub_set_on_read(serverh, read_cb);
    tcp_server_hub_set_on_close(serverh, close_cb);
    tcp_server_hub_set_on_accept(serverh, accept_cb);
    tcp_server_hub_start(serverh);

    evt_timer calll;
    evt_timer_init(&calll, calll_cb, SECOND(sec), SECOND(sec));
    evt_set_data(&calll, serverh);
    evt_timer_start(pool->loops[0], &calll);
    evt_pool_run(pool);

    return 0;
}