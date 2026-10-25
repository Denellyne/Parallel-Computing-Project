#!/bin/bash

inputs=(../tests/input*)
outputs=(../tests/output*)

make clean && make build
clear

for ((i = 0; i < ${#inputs[@]}; i++)); do
  inputFile=$(basename "${inputs[$i]}")
  output="./out:$inputFile"
  ../bin/foxSeq <"${inputs[$i]}" >"$output" 2>&1
  retVal=$?
  if [[ $retVal -eq 1 ]]; then
    echo "Fail for the input $inputFile" >"$output"
  elif [[ $(diff "$output" "${outputs[$i]}" | wc -l) -ne 2 ]]; then
    echo "Different output than expected, $inputFile"
  fi
done
rm ./out*
