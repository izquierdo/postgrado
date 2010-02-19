#!/bin/bash

INDIR=casos
OUTFILE=experimento7.out

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

echo '########' | tee -a ${OUTFILE}

for vistas in `seq 100 10 100`; do
    for pasos in `seq 2 4`; do
        for copias in 2; do
            VIEWSFILE=${INDIR}/views-${vistas}-${pasos}-${copias}.txt
            QUERYFILE=${INDIR}/query-${vistas}-${pasos}-${copias}.txt

            echo ${vistas} ${pasos} ${copias} | tee -a ${OUTFILE}

            for i in `seq 1`; do #n times to get average
                ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
            done
        done
    done
done
