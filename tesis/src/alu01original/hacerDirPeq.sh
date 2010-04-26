#!/bin/bash
cd ../experimentos
mkdir expTipo$1
for m in 3 4 5 6 7 8
  do
  mkdir expTipo$1/expM$m
  for n in 5 10 15 20 25
    do
    mkdir expTipo$1/expM$m/expM${m}N$n
    echo "Hace dirs $1 con m = $m y n = $n"
  done
done
