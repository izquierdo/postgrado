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
        R = (rel.cabeza.predicado, rel.cabeza.orden)
        P = (rel.cuerpo[0].predicado, rel.cuerpo[0].orden)
        
        tc.add((R, P))

    # transitive closure

    while True:
        new = set()

        for (a, b) in product(tc):
            pass

        if len(new) == 0:
            break

        tc.update(new)

    print pares
