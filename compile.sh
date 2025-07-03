#!/bin/bash

if ! modprobe rdma_rxe; then
    echo "failed to load rdma_rxe module"
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
