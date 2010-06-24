#!/usr/bin/env python

from itertools import product
from collections import defaultdict

from Parser.CQparser import cargarCQ, es_const

def load_ontology(filename):
    o = cargarCQ(filename)

    return o

class Spec:
    def __init__(self, subclass_name, subclass_args, class_name, class_args):
        self.subclass_name = subclass_name
        self.subclass_args = subclass_args

        self.class_name = class_name
        self.class_args = class_args

        self.mapping = defaultdict(set)

        for i, arg in enumerate(subclass_args):
            self.mapping[arg].add((0, i))

        for i, arg in enumerate(class_args):
            self.mapping[arg].add((1, i))

    def __repr__(self):
        return "%s [ %s (%s)" % (self.subclass_name, self.class_name, self.mapping)

    def join(self, other):
        unified = self._unify(self, other)

        if not unified:
            return None

        joined = Spec(self.subclass_name, other.class_name, [], [])
        joined.mapping = unified

        return joined

    def _unify(self, a, b):
        if a.class_name != b.subclass_name:
            return None

        if len(a.class_args) != len(b.subclass_args):
            return None

        usets = []

        for i in xrange(len(a.class_args)):
            found = False

            for us in usets:
                if (a.class_args[i] in us) or (b.subclass_args[i] in us):
                    found = True
                    us.add(a.class_args[i])
                    us.add(b.subclass_args[i])
                    break

            if not found:
                us = set()
                us.add(a.class_args[i])
                us.add(b.subclass_args[i])
                usets.append(us)

        # check for incompatible mappings
        for us in usets:
            cus = [x for x in us if es_const(x)]

            if len(cus) > 1:
                return None


        print "^^"
        print "attempting to unify"
        print a.mapping
        print "WITH"
        print b.mapping
        print "USETS ARE"
        print usets
        print "$$"
        return None

def generate_specs(filename):
    o = load_ontology(filename)

    tc = set()

    for rel in o:
        tc.add(Spec(rel.cabeza.predicado, rel.cabeza.orden, rel.cuerpo[0].predicado, rel.cuerpo[0].orden))

    # transitive closure

    while True:
        new = set()

        for (a, b) in product(tc, repeat=2):
            if a == b:
                continue

            c = a.join(b)

            if c is not None:
                print "adding %s" % (c,)
                new.add(c)

        if len(new) == 0:
            break

        tc.update(new)
