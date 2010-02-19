#!/bin/bash

INDIR=casos
OUTFILE=experimento5-allrw.out

echo '########' | tee -a ${OUTFILE}

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat

for nviews in `seq 55 5 80`; do
    for nview_sos in `seq 2 5`; do
        #casos largos
        if [ $nviews = 40 -a $nview_sos = 5 ]; then
            continue
        fi
        if [ $nviews = 45 -a $nview_sos = 4 ]; then
            continue
        fi

        VIEWSFILE=${INDIR}/views-$nviews-$nview_sos-4-10-10-3-0.5.txt
        QUERYFILE=${INDIR}/query-$nviews-$nview_sos-4-10-10-3-0.5.txt

        echo ${nviews} ${nview_sos} 4 10 10 3 0.5 | tee -a ${OUTFILE}

        for i in `seq 1`; do #n times to get average
            ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
        done
    done
done
