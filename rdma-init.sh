#!/bin/bash

set -e

# Load RDMA RXE kernel module
if ! modprobe rdma_rxe; then
    echo "Failed to load rdma_rxe module"
    exit 1
fi

# Create RXE interface
if ! rdma link add rxe0 type rxe netdev enp0s3 2>/dev/null; then
    echo "rxe0 already exists or failed to create. Continuing anyway..."
fi

# Show active RDMA devices and links
echo "RDMA devices:"
ibv_devices || echo "ibv_devices not available"

echo "RDMA links:"
rdma link show