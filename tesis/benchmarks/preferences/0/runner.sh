#!/bin/bash

INDIR=casos
OUTFILE=preferences0.out

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

echo '########' | tee -a ${OUTFILE}

for vistas in `seq 90 10 100`; do
    for pasos in `seq 2 5`; do
            VIEWSFILE=${INDIR}/views-${vistas}-${pasos}.txt
            QUERYFILE=${INDIR}/query-${vistas}-${pasos}.txt

            echo ${vistas} ${pasos} | tee -a ${OUTFILE}

            ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
    done
done
