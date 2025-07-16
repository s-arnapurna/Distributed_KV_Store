#!/usr/bin/env bash
ID=$1
./kv_node --id ${ID} --config ./config/cluster.json
