#!/bin/bash

INDIR=casos
OUTFILE=plot1.raw

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

echo '################################################################################' | tee -a ${OUTFILE}

for vistas in `seq 90 10 90`; do
    vistas=45
    for pasos in `seq 2 5`; do
        pasos=5
        VIEWSFILE=${INDIR}/views-${vistas}-${pasos}.txt
        QUERYFILE=${INDIR}/query-${vistas}-${pasos}.txt

        echo ${vistas} ${pasos} | tee -a ${OUTFILE}

        for i in `seq 1`; do #n times to get average
            ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
        done
    done
done
