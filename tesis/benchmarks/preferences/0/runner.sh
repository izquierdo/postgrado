#!/bin/bash

INDIR=casos
OUTFILE=preferences0.out

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat

#echo '########' | tee -a ${OUTFILE}

for vistas in `seq 5 5 5`; do
    for pasos in `seq 2 2`; do
            VIEWSFILE=${INDIR}/views-${vistas}-${pasos}.txt
            QUERYFILE=${INDIR}/query-${vistas}-${pasos}.txt

            #echo ${vistas} ${pasos} ${copias} | tee -a ${OUTFILE}

            #${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE}
            ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE}
    done
done
