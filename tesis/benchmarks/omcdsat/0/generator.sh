#!/bin/bash

QRPGEN=/home/daniel/proyectos/postgrado/tesis/src/qrpgen/qrpgen.py

for nquery_sos in `seq 2 1 20`; do
    for nviews in `seq 10 10 50`; do
        VIEWSFILE=casos/views-$nviews-$nquery_sos.in
        QUERYFILE=casos/query-$nviews-$nquery_sos.in

        $QRPGEN $VIEWSFILE $QUERYFILE flights.sameairline_manyviews $nviews $nquery_sos 2
    done
done
