#!/usr/bin/env python

from itertools import product
from Parser.CQparser import cargarCQ

def load_ontology(filename):
    o = cargarCQ(filename)

    return o

def generate_bichos(filename):
    o = load_ontology(filename)

    tc = set()

    for rel in o:
        tc.add((rel.cabeza, rel.cuerpo[0]))

    # transitive closure

    print tc

    while True:
        new = set()

        for (a, b) in product(tc, repeat=2):
            if a != b:
                print (a, b)

        if len(new) == 0:
            break

        tc.update(new)
