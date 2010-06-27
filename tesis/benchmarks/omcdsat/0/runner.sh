#!/bin/bash

INDIR=casos
OUT=omcdsat_0.raw
TIME="timeout 600 /usr/bin/time -apo ${OUT}"
GENONTOLOGIES=/home/daniel/proyectos/postgrado/tesis/src/omcdsat/mcdsat/GenOntologies.py
OMCDSAT=/home/daniel/proyectos/postgrado/tesis/src/omcdsat/mcdsat/mcdsat

echo '################################################################################' | tee -a ${OUT_MMS}

for levels in `seq 2 3`; do
    ONTOLOGY=casos/${levels}levels.ontology
    VIEWS=casos/vistas.txt
    QUERY=casos/query.txt

    echo ${levels} | tee -a ${OUT}

    for i in `seq 1`; do #n times to get average
        ${TIME} ./orunner.sh "$GENONTOLOGIES" "$OMCDSAT" "$ONTOLOGY" "$VIEWS" "$QUERY"
    done
done
