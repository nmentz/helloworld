#!/bin/bash

set -e

case "$1" in
    server)
        gcc server.c -g -Wall -o server -lrdmacm -libverbs
        echo "Built: server"
        ;;
    client)
        gcc client.c -g -Wall -o client -lrdmacm -libverbs
        echo "Built: client"
        ;;
    *)
        echo "Usage: ./compile.sh [client|server]"
        exit 1
        ;;
esac