#!/bin/bash

INDIR=casos
OUTFILE=plot1.raw

TIME="/usr/bin/time -apo ${OUTFILE}"
QRPGEN=/home/daniel/proyectos/postgrado/tesis/src/qrpgen/qrpgen.py

echo '################################################################################' | tee -a ${OUTFILE}

nview_sos=3
n_preds=10
n_vars=10
n_varsperpred=3
perc_constants=30

for nquery_sos in `seq 10 1 15`; do
    for nviews in `seq 20 10 100`; do
        viewsfile=casos/views-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants
        queryfile=casos/query-$nviews-$nview_sos-$nquery_sos-$n_preds-$n_vars-$n_varsperpred-$perc_constants

        $QRPGEN $viewsfile $queryfile randomviews $nviews $nview_sos $nquery_sos $n_preds $n_vars $n_varsperpred $perc_constants
    done
done
