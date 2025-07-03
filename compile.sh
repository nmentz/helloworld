#!/bin/bash

if ! rdma link add rxe0 type rxe netdev enp0s3 ; then
    exit 1
fi

if ! modprobe rdma_rxe; then
    exit 1
fi

set -e

case "$1" in
    server)
        gcc server.c -g -lrdmacm -libverbs -o server
        ;;
    client)
        gcc client.c -g -lrdmacm -libverbs -o client
        ;;
    *)
        echo "Usage: ./compile [client|server]"
        exit 1
        ;;
esac

ibv_devices
rdma link show
