#!/bin/bash
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
