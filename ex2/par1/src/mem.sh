#!/bin/bash

inputs=(../tests/input*)
results="./data.txt"
echo "" >"$results"

make clean && make  build
clear

for ((i = 0; i < ${#inputs[@]}; i++)); do
  inputFile=$(basename "${inputs[$i]}")

  output="./out:$inputFile"
  ../bin/main <"${inputs[$i]}" &

  PID=$!
  echo "timestamp,rss_kb,vsz_kb" > "$output".csv

  while kill -0 $PID 2>/dev/null; do
    STATS=$(ps -o rss=,vsz= -p $PID | awk '{print $1","$2}')
    echo "$(date +%s),$STATS" >> "$output".csv
    sleep 0.5
  done
done


cat "$results"
