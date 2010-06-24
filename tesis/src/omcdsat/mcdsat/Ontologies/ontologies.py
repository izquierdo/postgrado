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

    def __repr__(self):
        return "%s(%s) =[ %s(%s)" % (self.subclass_name, self.subclass_args, self.class_name, self.class_args)

    def join(self, other):
        unified = self._unify(self, other)

        if not unified:
            return None

        subclass_args = unified[0]
        class_args = unified[1]

        joined = Spec(self.subclass_name, subclass_args, other.class_name, class_args)

        return joined

    def _unify(self, a, b):
        if a.class_name != b.subclass_name:
            return None

        if len(a.class_args) != len(b.subclass_args):
            return None

        usets = []
        usets_repr = []

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

                # add uset representative
                if es_const(a.class_args[i]):
                    usets_repr.append(a.class_args[i])
                else:
                    usets_repr.append(b.subclass_args[i])

                # add uset
                usets.append(us)

        # check for incompatible mappings
        for us in usets:
            cus = [x for x in us if es_const(x)]

            if len(cus) > 1:
                return None

        new_subclass_args = a.subclass_args
        new_class_args = b.class_args

        for i in xrange(len(new_subclass_args)):
            for (usi, us) in enumerate(usets):
                if new_subclass_args[i] in us:
                    new_subclass_args[i] = usets_repr[usi]

        print "^^"
        print "result of attempting to unify:"
        print new_subclass_args
        print "TO"
        print new_class_args
        print "USETS ARE"
        print usets
        print "$$"
        return (new_subclass_args, new_class_args)

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

        pre_len = len(tc)
        tc.update(new)
        pos_len = len(tc)

        if pre_len < pos_len:
            break
