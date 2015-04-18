#include <oh.h>

#define BUFSZ 65535

typedef struct _tag_hole_client{
    tcp_client *client;
    int id;
    int closeflag;
} hole_client;

int gid = 0;
map_t *seqc;

net_addr serv_addr;
net_addr self_addr;

tcp_client *cc_client = NULL;

void on_cc_connect(tcp_client *client);
void on_cc_read(tcp_client *client);
void on_cc_close(tcp_client *client);

void on_ss_connect(tcp_client *client);
void on_ss_read(tcp_client *client);
void on_ss_close(tcp_client *client);


void on_cc_accept(tcp_client *client) {
    if (client->flag & TCPFLG_CLT_CONNFAIL) {
        tcp_close(client);
        return;
    } else {
        cc_client = client;
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
        if (buf_used(rbuf) < len) {
            return;
        }
        buf_read(rbuf, 4, tmp, BUFSZ);
        buf_read(rbuf, 5, tmp, BUFSZ);
        memcpy(&id, tmp + 1, 4);

        log_debug("%d %x %d", len, tmp[0]&0xFF, id);
        /* get client */
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
                log_error("rclient is null");
            } else {
                tcp_send(rclient, tmp, len);
            }
        /* close command */
        } else if (op == 1){
            if (rclient) {
                hole_client *hc = (hole_client*)rclient->data;
                hc->closeflag = 1;
                tcp_close(rclient);
            }
        }
    }
}

void on_cc_close(tcp_client *client) {
    cc_client = NULL;
}


void on_ss_accept(tcp_client *client) {
    if (client->flag & TCPFLG_CLT_CONNFAIL) {
        tcp_close(client);
        return;
    } else {
        hole_client *hc = (hole_client*)ohmalloc(sizeof(hole_client));
        hc->id = gid++;
        hc->closeflag = 0;
        hc->client = client;
        client->data = hc;
        map_put(seqc, hc->id, client);
        log_info("ss connect success");
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
        if (cc_client) {
            log_debug("read %d %x %d", len, tmp[4]&0xFF, hc->id);
            tcp_send(cc_client, tmp, len);
        }
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
        if (cc_client) {
            log_debug("close %d %x %d", len, tmp[4]&0xFF, hc->id);
            tcp_send(cc_client, tmp, len);
        }
    }
    map_erase_val(seqc, hc->id);
    ohfree(hc);
}

int main() {
   // set_default_logif_level(LOG_WARN);
    evt_loop *loop = evt_loop_init();
    netaddr_init_v4(&self_addr, "0.0.0.0", 8888);
    netaddr_init_v4(&serv_addr, "0.0.0.0", 8880);

    tcp_server *cc_server = tcp_server_init(&self_addr, loop, 0);
    tcp_server *ss_server = tcp_server_init(&serv_addr, loop, 0);


    tcp_connection_set_on_close(cc_server, on_cc_close);
    tcp_connection_set_on_read(cc_server, on_cc_read);
    tcp_connection_set_on_accept(cc_server, on_cc_accept);

    tcp_connection_set_on_close(ss_server, on_ss_close);
    tcp_connection_set_on_read(ss_server, on_ss_read);
    tcp_connection_set_on_accept(ss_server, on_ss_accept);

    seqc = map_new(int, tcp_client*);

    tcp_server_start(cc_server);
    tcp_server_start(ss_server);
    evt_loop_run(loop);
    return 0;
}