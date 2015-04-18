#include <oh.h>

#define BUFSZ 65535

typedef struct _tag_hole_client{
    tcp_client *client;
    int id;
    int closeflag;
    ohbuffer cache;
} hole_client;

map_t *seqc;

net_addr serv_addr;
net_addr self_addr;

tcp_client *cc_client;

void on_cc_connect(tcp_client *client);
void on_cc_read(tcp_client *client);
void on_cc_close(tcp_client *client);

void on_ss_connect(tcp_client *client);
void on_ss_read(tcp_client *client);
void on_ss_close(tcp_client *client);


void on_ss_connect(tcp_client *client) {
    if (client->flag & TCPFLG_CLT_CONNFAIL) {
        tcp_close(client);
        return;
    } else {
        log_info("ss connect success");
    }
    hole_client *hc = (hole_client*)client->data;
    char tmp[BUFSZ];
    while (buf_used(&hc->cache)) {
        int n = buf_readall(&hc->cache, tmp, BUFSZ);
        tcp_send(client, tmp, n);
    }
}

void on_ss_read(tcp_client *client) {
    hole_client *hc = (hole_client*)client->data;
    char tmp[BUFSZ];
    ohbuffer *rbuf = &client->rbuf;
    while (buf_used(rbuf)) {
        int n = buf_readall(rbuf, tmp + 9, BUFSZ - 9);
        int len = n + 9;
        memcpy(tmp, (char*)&len, 4);
        tmp[4] = 0;
        memcpy(tmp + 5, (char*)&hc->id, 4);
        tcp_send(cc_client, tmp, len);
    }
}

void on_ss_close(tcp_client *client) {
    hole_client *hc = (hole_client*)client->data;
    char tmp[BUFSZ];
    int len = 9;
    memcpy(tmp, (char*)&len, 4);
    tmp[4] = 1;
    memcpy(tmp + 5, (char*)&hc->id, 4);
    if (hc->closeflag == 0) {
        tcp_send(cc_client, tmp, len);
    }
    map_erase_val(seqc, hc->id);
    buf_destroy(&hc->cache);
    ohfree(hc);
}


void on_cc_connect(tcp_client *client) {
    if (client->flag & TCPFLG_CLT_CONNFAIL) {
        tcp_close(client);
        return;
    } else {
        log_info("cc connect success");
    }
}

void on_cc_read(tcp_client *client) {
    ohbuffer *rbuf = &client->rbuf;


    while (buf_used(rbuf) >= 5) {
        char tmp[BUFSZ];
        buf_peek(rbuf, 4, tmp, BUFSZ);
        int len = 0, id = 0;
        memcpy(&len, tmp, 4);
        log_debug("%d %d", buf_used(rbuf), len);
        if (buf_used(rbuf) < len) {
            return;
        }
        buf_read(rbuf, 4, tmp, BUFSZ);
        buf_read(rbuf, 5, tmp, BUFSZ);
        memcpy(&id, tmp + 1, 4);


        log_debug("%d %x %d", len, tmp[0]&0xFF, id);
        // /* get client */
        iterator_t it = map_find(seqc, id);
        tcp_client *rclient = NULL;
        if (!iter_equal(it, map_end(seqc))) {
            rclient = *(tcp_client**)map_at(seqc, id);
        }
        /* data */
        char op = tmp[0];
        len -= 9;
        if (op == 0) {
            buf_read(rbuf, len, tmp, BUFSZ);
            /* no client */
            if (rclient == NULL) {
                rclient = (tcp_client*)ohmalloc(sizeof(tcp_client));
                tcp_client_init(rclient, &self_addr, client->loop_on, 0, NULL, 0);
                tcp_connection_set_on_close(rclient, on_ss_close);
                tcp_connection_set_on_read(rclient, on_ss_read);
                tcp_connection_set_on_connect(rclient, on_ss_connect);
                tcp_connect(rclient);
                map_put(seqc, id, rclient);

                hole_client *hc = (hole_client*)ohmalloc(sizeof(tcp_client));
                hc->client = rclient;
                hc->id = id;
                hc->closeflag = 0;
                buf_init(&hc->cache, OHBUFFER_UNIT_DEFAULT_SIZE, NULL, 0);
                rclient->data = (void*)hc;

                buf_write(&hc->cache, tmp, len);
            /* not connect */
            } else if (0 == (rclient->flag & TCPFLG_CLT_CONNED)) {
                hole_client *hc = (hole_client*)rclient->data;
                buf_write(&hc->cache, tmp, len);
            /* connected */
            } else {
                tcp_send(rclient, tmp, len);
            }
        /* close command */
        } else if (op == 1){
            if (rclient) {
                hole_client *hc = (hole_client*)rclient->data;
                hc->closeflag = 1;
                tcp_close(rclient);
            } else {
                log_error("close when no rclient");
            }
        }
    }
}

void on_cc_close(tcp_client *client) {

}

int main() {
    set_default_logif_level(LOG_WARN);
    evt_loop *loop = evt_loop_init();
    netaddr_init_v4(&self_addr, "192.168.0.234", 3389);
    netaddr_init_v4(&serv_addr, "218.244.129.150", 8888);

    cc_client = (tcp_client*)ohmalloc(sizeof(tcp_client));
    tcp_client_init(cc_client, &serv_addr, loop, 0, NULL, 0);

    tcp_connection_set_on_close(cc_client, on_cc_close);
    tcp_connection_set_on_read(cc_client, on_cc_read);
    tcp_connection_set_on_connect(cc_client, on_cc_connect);

    seqc = map_new(int, tcp_client*);

    tcp_connect(cc_client);
    evt_loop_run(loop);
    return 0;
}