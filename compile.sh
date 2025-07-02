#!/bin/bash
gcc client.c -lrdmacm -libverbs -o client
gcc server.c -lrdmacm -libverbs -o server
