#!/bin/bash
m=${2}
n=$3
rep=$4
echo $'#MCD\t#Rew\tTMCD\tTRew' > ../experimentos/expTipo$1/TiemposMCTipo$1M${m}N$n.txt &&
for ((a=0; a<rep; a++))
do
  (ulimit -t 1800; java -Xmx1024M GoodPlan ../experimentos/expTipo$1/expM${m}/expM${m}N$n/vistasM${m}N${n}_$a.txt ../experimentos/expTipo$1/expM${m}/expM${m}N$n/consM${m}_$a.txt >> ../experimentos/expTipo$1/TiemposMCTipo$1M${m}N$n.txt )  
done &&
echo "fin  ../experimentos/expTipo$1/TiemposMCTipo$1M${m}N$n.txt"
