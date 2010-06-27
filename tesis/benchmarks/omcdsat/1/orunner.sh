#!/bin/bash

GENONTOLOGIES="$1"
OMCDSAT="$2"
ONTOLOGY="$3"
VIEWS="$4"
QUERY="$5"

${GENONTOLOGIES} $ONTOLOGY && \
${OMCDSAT} BESTRW ${VIEWS} ${QUERY} casos/costs.txt ${ONTOLOGY}.ontology > /dev/null
true
