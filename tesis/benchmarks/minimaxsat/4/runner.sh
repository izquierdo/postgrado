#!/bin/bash

INDIR=casos

OUT_MCD=plot_mcd2.raw
OUT_MMS=plot_mms2.raw

TIME_MCD="/usr/bin/time -apo ${OUT_MCD}"
TIME_MMS="/usr/bin/time -apo ${OUT_MMS}"

PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

MCDSAT=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat
MINIMAXSAT="/home/daniel/proyectos/postgrado/tesis/src/MiniMaxSat/minimaxsat1.0 -F=2"

echo '################################################################################' | tee -a ${OUT_MCD}
echo '################################################################################' | tee -a ${OUT_MMS}

ulimit -t 600
ulimit -d 512000

for nquery_sos in `seq 16 1 20`; do
    #for nviews in `seq 10 10 100`; do
    for nviews in 20 40 60 80 100; do
        VIEWSFILE=casos/views-$nviews-$nquery_sos.in
        QUERYFILE=casos/query-$nviews-$nquery_sos.in
        COSTFILE=../../../src/micosts.txt

        CNF=`basename $VIEWSFILE`.cnf

        echo ${nviews} ${nquery_sos} | tee -a ${OUT_MCD}
        echo ${nviews} ${nquery_sos} | tee -a ${OUT_MMS}

        for i in `seq 1`; do #n times to get average
            ${TIME_MCD} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null

            ${TIME_MMS} ./mmsrunner.sh "$MCDSAT" "$VIEWSFILE" "$QUERYFILE" "$COSTFILE" "$MINIMAXSAT" "$CNF"
        done
    done
done
