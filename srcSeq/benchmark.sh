#!/bin/bash

inputs=(../tests/input*)
results="./data.txt"
echo "" >"$results"

make clean && make build
clear

for ((i = 0; i < ${#inputs[@]}; i++)); do
  inputFile=$(basename "${inputs[$i]}")
  output="./out:$inputFile"
  totalTime=0
  for ((it = 0; it < 10; it++)); do
    ../bin/foxSeq <"${inputs[$i]}" >"$output" 2>&1
    currentTime=$(tail -n 1 "$output" | grep -Eo "[[:digit:]]*[.][[:digit:]]*")
    totalTime=$(echo "scale=9;  $totalTime + $currentTime" | bc -l)
  done
  totalTime=$(echo "scale=9; $totalTime / 10.0" | bc -l)

  echo "$inputFile took an average of $totalTime seconds in 10 tests" >>"$results"
done

cat "$results"
rm ./out*
