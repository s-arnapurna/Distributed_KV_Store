#!/bin/bash

# This script starts a 3-node cluster on the local machine.

# Path to the server executable
SERVER_EXE="./build/kv_server"
CLUSTER_CONF="127.0.0.1:8001,127.0.0.1:8002,127.0.0.1:8003"

# --- 1. Check if server executable exists ---
if [ ! -f "$SERVER_EXE" ]; then
    echo "Server executable not found at $SERVER_EXE"
    echo "Please build the project first using CMake."
    exit 1
fi

# --- 2. Clean up previous runs ---
echo "Cleaning up old data and processes..."
pkill -f kv_server
rm -rf ./data1 ./data2 ./data3

# --- 3. Create data directories ---
mkdir -p ./data1 ./data2 ./data3
echo "Created data directories."

# --- 4. Start the 3 nodes in the background ---
echo "Starting nodes..."

./build/kv_server --port=8001 --data_path=./data1 --conf=$CLUSTER_CONF &
PID1=$!
echo "  - Started Node 1 with PID $PID1 on port 8001"

./build/kv_server --port=8002 --data_path=./data2 --conf=$CLUSTER_CONF &
PID2=$!
echo "  - Started Node 2 with PID $PID2 on port 8002"

./build/kv_server --port=8003 --data_path=./data3 --conf=$CLUSTER_CONF &
PID3=$!
echo "  - Started Node 3 with PID $PID3 on port 8003"

echo ""
echo "Cluster started successfully!"
echo "To check status, open a browser to http://127.0.0.1:8001/status"
echo "To stop the cluster, run: pkill -f kv_server"
