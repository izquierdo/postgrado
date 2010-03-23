#!/bin/bash

INDIR=casos
OUTFILE=plot1.raw

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

MCDSAT=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat
MINIMAXSAT="minimaxsat1.0 -F=2"

echo '################################################################################' | tee -a ${OUTFILE}


nview_sos=3
n_preds=10
n_vars=10
n_varsperpred=3
perc_constants=25

for nviews in `seq 20 10 100`; do
    for nquery_sos in `seq 10 5 30`; do
        VIEWSFILE=casos/views-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants
        QUERYFILE=casos/query-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants

        CNF=$VIEWSFILE.CNF

        echo ${vistas} ${pasos} | tee -a ${OUTFILE}

        for i in `seq 1`; do #n times to get average
            ${MCDSAT} BIGBESTRW ${VIEWSFILE} ${QUERYFILE} ../../../../src/micosts.txt > /dev/null
            ${TIME} ${MINIMAXSAT} ${CNF} > /dev/null
        done
    done
done
