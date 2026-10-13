#!/bin/bash

sqrt() {
  result=0
  sub=1
  num=$1
  while (( num > 0 )); do
    num=$((num-sub))
    result=$((result + 1))
    sub=$((sub+2))
  done
  return "$result"
}

inputs=(../tests/input*)
outputs=(../tests/output*)
process=(1 4 9 16 25)

make clean && make build
clear

for ((i = 0; i < ${#inputs[@]}; i++)); do
  for ((proc = 0; proc < ${#process[@]}; proc++)); do
    inputFile=$(basename "${inputs[$i]}" )
    numNodes=$(echo "$inputFile" | grep -Eo "[[:digit:]]*")
    sqrt "${process[$proc]}"
    res=$?
    if  [[ $(( numNodes % res )) -eq 0  ]];then
      output="./out:${process[$proc]}:$inputFile"
      mpirun --oversubscribe -np "${process[$proc]}" ../bin/fox < "${inputs[$i]}" > "$output" 2>&1
      retVal=$?
      if [[ $retVal -eq 1 ]]; then
        echo "The number of threads ${process[$proc]} fails for the input $inputFile" > "$output"
        echo "$inputFile failed for ${process[$proc]} processes"
      elif [[ $(diff "$output" "${outputs[$i]}" | wc -l) -ne 2 ]];then
        echo "Different output than expected, $inputFile ${process[$proc]}"
      fi
    else
      echo "The number of threads ${process[$proc]} doesn't equally divide the matrix for the input $inputFile" 
    fi
  done
done
rm ./out*
