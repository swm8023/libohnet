#include <oh.h>

#define ON  1
#define OFF 0

typedef struct _tag_conn_peer{
    tcp_client *conn[2];
    int status[2];
} conn_peer;

net_addr serv_addr;
net_addr self_addr;

void on_connect(tcp_client *client);
void on_accept(tcp_client *client);
void on_read(tcp_client *client);
void on_close(tcp_client *client);

int get_peer_ind(conn_peer *peer, tcp_client *client) {
    int ind =  peer->conn[0] == client ? 0 : (peer->conn[1] == client ? 1 : -1);
    return ind;
}

void on_connect(tcp_client *client) {
    if (client->flag & TCPFLG_CLT_CONNFAIL) {
        tcp_close(client);
        return;
    }
    conn_peer *peer = (conn_peer*)client->data;
    int ind = get_peer_ind(peer, client);
    peer->status[ind] = ON;
    on_read(peer->conn[ind^1]);
}

void on_accept(tcp_client *client) {
    conn_peer *cpeer = (conn_peer*)ohmalloc(sizeof(conn_peer));

    tcp_client *rclient = (tcp_client*)ohmalloc(sizeof(tcp_client));
    tcp_client_init(rclient, &serv_addr, client->loop_on, 0, NULL, 0);
    tcp_connection_set_on_connect(rclient, on_connect);
    tcp_connection_set_on_close(rclient, on_close);
    tcp_connection_set_on_read(rclient, on_read);
    tcp_connect(rclient);

    client->data = (void*)cpeer;
    rclient->data = (void*)cpeer;
    cpeer->conn[0] = client;
    cpeer->conn[1] = rclient;
    cpeer->status[0] = ON;
    cpeer->status[1] = OFF;
}

void on_read(tcp_client *client) {
    conn_peer *peer = (conn_peer*)client->data;
    int ind = get_peer_ind(peer, client);
    tcp_client *rclient = peer->conn[ind^1];
    if (peer->status[ind^1] == OFF) {
        return;
    }
    char tmp[65536];
    while(buf_used(&client->rbuf)) {
        int n = buf_readall(&client->rbuf, tmp, 65536);
        tcp_send(rclient, tmp, n);
    }
}

void on_close(tcp_client *client) {
    conn_peer *peer = (conn_peer*)client->data;
    int ind = get_peer_ind(peer, client);
    peer->status[ind] = OFF;
    if (peer->status[ind^1] == ON) {
        tcp_close(peer->conn[ind^1]);
        ohfree(peer);
    }
}

int main() {
    set_default_logif_level(LOG_WARN);
    evt_loop *loop = evt_loop_init();
    netaddr_init_v4(&self_addr, "0.0.0.0", 8322);
    netaddr_init_v4(&serv_addr, "107.170.203.188", 8002);

    tcp_server *server = tcp_server_init(&self_addr, loop, 0);

    tcp_connection_set_on_accept(server, on_accept);
    tcp_connection_set_on_close(server, on_close);
    tcp_connection_set_on_read(server, on_read);

    tcp_server_start(server);
    evt_loop_run(loop);
    return 0;
}