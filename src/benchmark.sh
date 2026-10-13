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
process=(1 4 9 16 25)
results="./data.txt"
echo "" > "$results"

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
        totalTime=0
        for ((it = 0; it < 10; it++)); do
          mpirun --oversubscribe -np "${process[$proc]}" ../bin/fox < "${inputs[$i]}" > "$output" 2>&1
          currentTime=$(tail -n 1 "$output" | grep -Eo "[[:digit:]]*[.][[:digit:]]*")
          totalTime=$(echo "scale=9;  $totalTime + $currentTime" | bc -l)
        done
          totalTime=$(echo "scale=9; $totalTime / 10.0" | bc -l)

        echo "$inputFile with ${process[$proc]} threads took an average of $totalTime seconds in 10 tests" >> "$results"
    else
      echo "The number of threads ${process[$proc]} doesn't equally divide the matrix for the input $inputFile" 
    fi
  done
done

cat "$results"
rm ./out*

