javac -classpath ~/yoli/tesis/capjprof/java:$CLASSPATH *.java &&

for m in 3 4 5 6 7 8 9 10
do
  echo "generando exp tipo $1 con m = $m"
  for n in 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 
    do

    java Query $m $n $1 ../experimentos/expTipo$1/expM$m/expM${m}N$n/vistasM${m}N$n ../experimentos/expTipo$1/expM$m/expM${m}N$n/consM$m
    done
done
