#!/usr/bin/env python

from itertools import product
from collections import defaultdict

from Parser.CQparser import cargarCQ

def load_ontology(filename):
    o = cargarCQ(filename)

    return o

class Spec:
    def __init__(self, subclass_name, subclass_args, class_name, class_args):
        self.subclass_name = subclass_name
        self.class_name = class_name

        self.mapping = defaultdict(set)

        for i, arg in enumerate(subclass_args):
            self.mapping[arg].add((0, i))

        for i, arg in enumerate(class_args):
            self.mapping[arg].add((1, i))

    def __repr__(self):
        return "%s [ %s (%s)" % (self.subclass_name, self.class_name, self.mapping)

def generate_bichos(filename):
    o = load_ontology(filename)

    tc = set()

    for rel in o:
        tc.add(Spec(rel.cabeza.predicado, rel.cabeza.orden, rel.cuerpo[0].predicado, rel.cuerpo[0].orden))

    # transitive closure

    while True:
        new = set()

        for (a, b) in product(tc, repeat=2):
            if a != b:
                print (a, b)

        if len(new) == 0:
            break

        tc.update(new)
