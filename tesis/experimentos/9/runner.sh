#!/bin/bash

INDIR=casos
OUTFILE=experimento5-allrw.out

echo '########' | tee -a ${OUTFILE}

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

for nviews in `seq 10 5 80`; do
    for nview_sos in `seq 2 5`; do
        VIEWSFILE=${INDIR}/views-$nviews-$nview_sos-6-10-10-3-0.5.txt
        QUERYFILE=${INDIR}/query-$nviews-$nview_sos-6-10-10-3-0.5.txt

        echo ${nviews} ${nview_sos} 6 10 10 3 0.5 | tee -a ${OUTFILE}

        for i in `seq 1`; do #n times to get average
            ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
        done
    done
done
