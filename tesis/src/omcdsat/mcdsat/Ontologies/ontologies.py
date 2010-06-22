#!/usr/bin/env python

from Parser.CQparser import cargarCQ

def load_ontology(filename):
    o = cargarCQ(filename)

def generate_bichos(filename):
    o = load_ontology(filename)

    pares = []

    for rel in o:
        # R [ P
        R = (rel.cabeza.nombre, rel.cabeza.argumentos)
        P = (rel.cuerpo[0].nombre, rel.cuerpo[0].argumentos)
        
        pares.append(R, P)

    print pares
