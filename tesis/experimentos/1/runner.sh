#!/bin/bash

echo '########' | tee -a ${OUTFILE}

INDIR=casos
OUTFILE=datos-grafico1.out

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat

for vistas in `seq 5 5 80`; do
    for pasos in `seq 2 5`; do
        VIEWSFILE=${INDIR}/views-${vistas}-${pasos}.txt
        QUERYFILE=${INDIR}/query-${vistas}-${pasos}.txt

        echo ${vistas} ${pasos} | tee -a ${OUTFILE}

        for i in `seq 6`; do #n times to get average
            ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
        done
    done
done
