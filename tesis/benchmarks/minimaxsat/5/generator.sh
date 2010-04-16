#!/bin/bash

QRPGEN=/home/daniel/proyectos/postgrado/tesis/src/qrpgen/qrpgen.py

for nquery_sos in `seq 2 1 20`; do
    for nviews in `seq 20 20 100`; do
        VIEWSFILE=casos/views-$nviews-$nquery_sos.in
        QUERYFILE=casos/query-$nviews-$nquery_sos.in

        $QRPGEN $VIEWSFILE $QUERYFILE flights.sameairline $nviews $nquery_sos
    done
done
