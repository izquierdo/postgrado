#!/bin/bash

OUTFILE=grafico0.out

echo '########' | tee -a ${OUTFILE}

for vistas in `seq 5 5 80`; do
    for pasos in `seq 2 5`; do
        INDIR=./casos-grafico0
        VIEWSFILE=${INDIR}/views-${vistas}-${pasos}.txt
        QUERYFILE=${INDIR}/query-${vistas}-${pasos}.txt

        echo ${vistas} ${pasos} | tee -a ${OUTFILE}

        /usr/bin/time -a -p -o ${OUTFILE} ./imcdsat/mcdsat/compileonly RW ${VIEWSFILE} ${QUERYFILE}

    done
done
