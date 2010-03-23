#!/bin/bash

INDIR=casos
OUTFILE=plot2.raw

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly

MCDSAT=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/mcdsat
MINIMAXSAT="/home/daniel/proyectos/postgrado/tesis/src/MiniMaxSat/minimaxsat1.0 -F=2"

echo '################################################################################' | tee -a ${OUTFILE}


nview_sos=3
n_preds=10
n_vars=10
n_varsperpred=3
perc_constants=30

for nquery_sos in `seq 10 1 15`; do
    for nviews in `seq 20 10 40`; do
        VIEWSFILE=casos/views-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants
        QUERYFILE=casos/query-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants
        COSTFILE=../../../src/micosts.txt

        CNF=`basename $VIEWSFILE`.cnf

        echo ${nviews} ${nview_sos} ${nquery_sos} ${n_preds} ${n_vars} ${n_varsperpred} ${perc_constants} | tee -a ${OUTFILE}

        for i in `seq 1`; do #n times to get average
            ${TIME} ${PROGRAM} RW ${VIEWSFILE} ${QUERYFILE} > /dev/null
        done
    done
done
