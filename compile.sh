#!/bin/bash

echo "run ip link to find device name then pass in to this command e.g. device name is enp0s3 'sudo rdma link add rxe0 type rxe netdev enp0s3'"


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
