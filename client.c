#include <rdma/rdma_cma.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT "20079"
#define MSG_SIZE 64

int main() {
    struct rdma_event_channel *ec = rdma_create_event_channel();
    struct rdma_cm_id *conn_id;
    struct rdma_cm_event *event;

    rdma_create_id(ec, &conn_id, NULL, RDMA_PS_TCP);

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(PORT)),
    };
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    rdma_resolve_addr(conn_id, NULL, (struct sockaddr *)&server_addr, 2000);
    rdma_get_cm_event(ec, &event); rdma_ack_cm_event(event);
    rdma_resolve_route(conn_id, 2000);
    rdma_get_cm_event(ec, &event); rdma_ack_cm_event(event);

    struct ibv_pd *pd = ibv_alloc_pd(conn_id->verbs);
    struct ibv_cq *cq = ibv_create_cq(conn_id->verbs, 2, NULL, NULL, 0);

    struct ibv_qp_init_attr qp_attr = {
        .send_cq = cq,
        .recv_cq = cq,
        .qp_type = IBV_QPT_RC,
        .cap = {.max_send_wr = 1, .max_recv_wr = 1, .max_send_sge = 1, .max_recv_sge = 1},
    };

    rdma_create_qp(conn_id, pd, &qp_attr);

    char *recv_buf = malloc(MSG_SIZE);
    memset(recv_buf, 0, MSG_SIZE);

    struct ibv_mr *mr = ibv_reg_mr(pd, recv_buf, MSG_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);

    struct ibv_sge sge = {.addr = (uintptr_t)recv_buf, .length = MSG_SIZE, .lkey = mr->lkey};
    struct ibv_recv_wr wr = {.wr_id = 1, .sg_list = &sge, .num_sge = 1};
    struct ibv_recv_wr *bad_wr;

    ibv_post_recv(conn_id->qp, &wr, &bad_wr);
    rdma_connect(conn_id, NULL);

    rdma_get_cm_event(ec, &event); rdma_ack_cm_event(event); // connection established

    struct ibv_wc wc;
    while (ibv_poll_cq(cq, 1, &wc) < 1);

    printf("Received: %s\n", recv_buf);

    rdma_disconnect(conn_id);
    rdma_destroy_qp(conn_id);
    ibv_dereg_mr(mr);
    free(recv_buf);
    ibv_dealloc_pd(pd);
    ibv_destroy_cq(cq);
    rdma_destroy_id(conn_id);
    rdma_destroy_event_channel(ec);
    return 0;
}
