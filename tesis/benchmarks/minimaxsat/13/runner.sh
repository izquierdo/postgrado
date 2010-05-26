#!/bin/bash

# 8 objetivos, 80 vistas, costos con variaciones 0-10, 0-100, 0-1000, 0-10000

INDIR=casos

OUT_MCD=plot_mcd13.raw
OUT_MMS=plot_mms13.raw

TIME_MCD="timeout 600 /usr/bin/time -apo ${OUT_MCD}"
TIME_MMS="timeout 600 /usr/bin/time -apo ${OUT_MMS}"

PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

MCDSAT=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat
MINIMAXSAT="/home/daniel/proyectos/postgrado/tesis/src/MiniMaxSat/minimaxsat1.0 -F=2"

echo '################################################################################' | tee -a ${OUT_MCD}
echo '################################################################################' | tee -a ${OUT_MMS}

nquery_sos=8
nviews=80

VIEWSFILE=casos/views-${nviews}-${nquery_sos}.in_0.txt
QUERYFILE=casos/query-${nviews}-${nquery_sos}.in_0.txt

for costrange in 0-10 0-100 0-1000 0-10000; do
    COSTFILE=../../../src/costs-range_${costrange}.txt

    CNF=`basename $VIEWSFILE`.cnf

    echo ${costrange} | tee -a ${OUT_MCD}
    echo ${costrange} | tee -a ${OUT_MMS}

    for i in `seq 1`; do #n times to get average
        ${TIME_MCD} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null

        ${TIME_MMS} ./mmsrunner.sh "$MCDSAT" "$VIEWSFILE" "$QUERYFILE" "$COSTFILE" "$MINIMAXSAT" "$CNF"
    done
done
