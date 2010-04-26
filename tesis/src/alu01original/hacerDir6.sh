#!/bin/bash
cd ../experimentos
mkdir expTipo$1
for m in 8
  do
  mkdir expTipo$1/expM$m
  for n in 110 120 130 140 150
    do
    mkdir expTipo$1/expM$m/expM${m}N$n
    echo "Hace dirs $1 con m = $m y n = $n"
  done
done
