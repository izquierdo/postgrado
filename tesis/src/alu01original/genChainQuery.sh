# m = query subgoals
# n = numero de vistas

nquery_sos="$1"
nviews="$2"
VIEWSFILE="$3"
QUERYFILE="$4"

CLASSPATH=~/proyectos/postgrado/tesis/src/alu01original

java -cp "$CLASSPATH" Query $nquery_sos $nviews 1 $VIEWSFILE $QUERYFILE

#for m in 3
#do
  #echo "generando exp tipo $1 con m = $m"
  #for n in 10
    #do
#
    #java Query $m $n $1 ../tmp_output/vistasM${m}N$n ../tmp_output/consM$m
    #done
#done
