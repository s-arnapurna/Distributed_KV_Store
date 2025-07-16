#!/usr/bin/env bash
N=${1:-3}
for i in $(seq 1 $N); do
  ./scripts/run_node.sh $i &
done
wait
