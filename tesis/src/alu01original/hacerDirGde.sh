#!/bin/bash
cd ../experimentos
mkdir expTipo$1
for m in 3 4 5 6 7 8 9 10 
  do
  mkdir expTipo$1/expM$m
  for n in 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150
    do
    mkdir expTipo$1/expM$m/expM${m}N$n
    echo "Hace dirs $1 con m = $m y n = $n"
  done
done
