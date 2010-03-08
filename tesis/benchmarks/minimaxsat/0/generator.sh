#!/bin/bash

INDIR=casos
OUTFILE=plot1.raw

TIME="/usr/bin/time -apo ${OUTFILE}"
PROGRAM=/home/daniel/proyectos/postgrado/tesis/src/imcdsat/mcdsat/compileonly
QRPGEN=/home/daniel/proyectos/postgrado/tesis/src/qrpgen/qrpgen.py

echo '################################################################################' | tee -a ${OUTFILE}

$QRPGEN vtest qtest randomviews 10 4 3 10 6 2 25

nview_sos=3
n_preds=10
n_vars=10
n_varsperpred=3
perc_constants=25

for nviews in `seq 20 10 100`; do
    for nquery_sos in `seq 10 5 30`; do
        viewsfile=casos/views-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants
        queryfile=casos/query-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants

        $QRPGEN $viewsfile $queryfile randomviews $nviews $nview_sos $nquery_sos $n_preds $n_vars $n_varsperpred $perc_constants
    done
done
