javac -classpath ~/yoli/tesis/capjprof/java:$CLASSPATH *.java &&

for m in 8
do
  echo "generando exp tipo $1 con m = $m"
  for n in 110 120 130 140 150
    do

    java Query $m $n $1 ../experimentos/expTipo$1/expM$m/expM${m}N$n/vistasM${m}N$n ../experimentos/expTipo$1/expM$m/expM${m}N$n/consM$m
    done
done
