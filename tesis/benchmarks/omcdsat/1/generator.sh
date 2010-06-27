#!/bin/bash

GEN=./generator.py

for levels in `seq 7`; do
        $GEN $levels > casos/${levels}levels.ontology
done
