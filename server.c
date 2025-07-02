#include <rdma/rdma_cma.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT "20079"
#define MSG "Hello from server"
#define MSG_SIZE 64

int main() {
    struct rdma_event_channel *ec = rdma_create_event_channel();
    struct rdma_cm_id *listen_id;
    struct rdma_cm_event *event;

    rdma_create_id(ec, &listen_id, NULL, RDMA_PS_TCP);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(PORT)),
        .sin_addr.s_addr = INADDR_ANY,
    };

    rdma_bind_addr(listen_id, (struct sockaddr *)&addr);
    rdma_listen(listen_id, 1);

    printf("Server listening on port %s...\n", PORT);

    rdma_get_cm_event(ec, &event);  // connection request
    struct rdma_cm_id *conn_id = event->id;
    rdma_ack_cm_event(event);

    struct ibv_pd *pd = ibv_alloc_pd(conn_id->verbs);
    struct ibv_cq *cq = ibv_create_cq(conn_id->verbs, 2, NULL, NULL, 0);

    struct ibv_qp_init_attr qp_attr = {
        .send_cq = cq,
        .recv_cq = cq,
        .qp_type = IBV_QPT_RC,
        .cap = {.max_send_wr = 1, .max_recv_wr = 1, .max_send_sge = 1, .max_recv_sge = 1},
    };

    rdma_create_qp(conn_id, pd, &qp_attr);

    char *send_buf = malloc(MSG_SIZE);
    strcpy(send_buf, MSG);

    struct ibv_mr *mr = ibv_reg_mr(pd, send_buf, MSG_SIZE, IBV_ACCESS_LOCAL_WRITE);

    struct ibv_sge sge = {.addr = (uintptr_t)send_buf, .length = MSG_SIZE, .lkey = mr->lkey};
    struct ibv_send_wr wr = {.wr_id = 1, .sg_list = &sge, .num_sge = 1, .opcode = IBV_WR_SEND, .send_flags = IBV_SEND_SIGNALED};
    struct ibv_send_wr *bad_wr;

    rdma_accept(conn_id, NULL);

    ibv_post_send(conn_id->qp, &wr, &bad_wr);

    struct ibv_wc wc;
    while (ibv_poll_cq(cq, 1, &wc) < 1);

    printf("Message sent: %s\n", send_buf);

    sleep(1); // let client receive

    rdma_disconnect(conn_id);
    rdma_destroy_qp(conn_id);
    ibv_dereg_mr(mr);
    free(send_buf);
    ibv_dealloc_pd(pd);
    ibv_destroy_cq(cq);
    rdma_destroy_id(conn_id);
    rdma_destroy_id(listen_id);
    rdma_destroy_event_channel(ec);
    return 0;
}
