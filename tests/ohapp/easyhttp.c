#include <oh.h>


char *resp="\
HTTP/1.1 200 OK\r\n\
Server: Apache/2.4.7 (Ubuntu)\r\n\
Content-Type: text/html\r\n\
Content-Length: 11\r\n\
Keep-Alive: timeout=5, max=100\r\n\
Connection: Keep-Alive\r\n\
\r\n\
hello world\
";
int resplen;

int findrnrn(char *str, int len){
    str[len] = 0;
    char *p = strstr(str, "\r\n\r\n");
    if (p == NULL)
        return -1;
    else
        return p - str + 4;
}

void write_cb(tcp_client* client) {
    log_info("== %d write", client->fd);
}
void read_cb(tcp_client* client) {
    log_info("== %d read", client->fd);
    ohbuffer *rbuf = &client->rbuf;
    char buf[4096];
    int n = buf_peek(rbuf, 4096, buf, 4096);
    int p = -1;
    char *b = buf;
    while (n > 0 && (p = findrnrn(b, n)) != -1) {
        buf_read(rbuf, p, NULL, 4096);
        //log_warn("===\n%s\n===", b);
        tcp_send(client, resp, resplen);

        n -= p;
        b += p;
    }
    //log_warn("break");
}

void close_cb(tcp_client* client) {
//    log_info("== %d close", client->fd);
}

void accept_cb(tcp_client* client) {
//    log_info("== %d accept", client->fd);
}


int main() {
    set_default_logif_level(LOG_WARN);

    resplen = strlen(resp);
    int i, n = 4;
    evt_pool *pool = evt_pool_init(n);
    net_addr addr;
    netaddr_init_v4(&addr, "0.0.0.0", 8484);
    tcp_server_hub *serverh = tcp_server_hub_init(&addr, pool, 0);
    tcp_server_hub_set_on_write(serverh, write_cb);
    tcp_server_hub_set_on_read(serverh, read_cb);
    tcp_server_hub_set_on_close(serverh, close_cb);
    tcp_server_hub_set_on_accept(serverh, accept_cb);
    tcp_server_hub_start(serverh);

    evt_pool_run(pool);

    return 0;
}