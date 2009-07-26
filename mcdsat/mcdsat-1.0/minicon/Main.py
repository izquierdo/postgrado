#!/bin/python
import sys
import pprint
import resource
from CQ import *
from BuckAlgorithm import *
from CQ.Argumento import *
from CQ.Predicado import *
from CQ.SubObjetivo import *
from CQ.CQ import *
from CQ.SOComparacion import *
from BuckAlgorithm.createBuckets import *
from BuckAlgorithm.combinarBuckets import *
from BuckAlgorithm.containment import *
from MCAlgorithm.createMCDs import *
from MCAlgorithm.combinarMCDs import *
from Parser.CQparser import *
from sets import Set
from random import *

def main(archVistas, archCons, exp):
    MCexp(archVistas, archCons, exp)

def MCexp(archVistas, archCons, exp):
    vistas = cargarCQ(archVistas)
#     print "\nVistas: "
#     pprint.pprint(vistas)
    consultas = cargarCQ(archCons)
    #import psyco      # used for improved performance
    #psyco.full()      # used for improved performance
    for q in consultas:
#         print "Q:"
#         pprint.pprint(q)
        tiempoi = resource.getrusage(resource.RUSAGE_SELF)[0]
        bs = createMCDs(vistas, q)
        tiempof1 = resource.getrusage(resource.RUSAGE_SELF)[0]

        if exp == 'RW':
            rs = combinarMCDs(q, bs)
            for r in rs:
                print r
            tiempof2 = resource.getrusage(resource.RUSAGE_SELF)[0]
        elif exp == 'MCD':
            rs = []
            for b in bs:
                print b
            tiempof2=tiempof1
            
        print str(len(bs)) + '\t' + str(len(rs)) + '\t' + str(tiempof1-tiempoi) + '\t' + str(tiempof2-tiempof1)



def comparar(archVistas, archCons, archRes1, archRes2):
    vistas = cargarCQ(archVistas)
    vistas2 = {}
    for v in vistas:
        vistas2[v.cabeza.predicado] = v
        
    consultas = cargarCQ(archCons)
    res1 = cargarCQ(archRes1)
    res2 = cargarCQ(archRes2)

    for r1 in res1:
        contenido = False    
        r1e = r1 
        print r1e
        for r2 in res2:
            r2e = r2 
            if estaContenido(r1e,r2e):
                contenido = True
                break
        if not contenido:
            print "   no esta contenido"
        else:
            print "   contenido", r2e


# crea el mapping entre s y v
def mappingH(g, v):
    psi = {}
    h = {}
    varsY = v.orden
    j = 0
    for x in g.orden:
        if not psi.has_key(x):
            psi[x] = [varsY[j]]
        else:
            psi[x].append(varsY[j])
        j = j + 1
    for var in psi.keys():
        ###############################
        if len(psi[var]) > 1:
            rep = psi[var][0]
            del psi[var][0]
            for x in psi[var]:
                h[x] = rep
            psi[var] = rep
        else:
            psi[var] = psi[var][0]
    return psi, h

def expandirVistas(res, vistas):
    cue = []
    for so in res.cuerpo:
        v = vistas[so.predicado]
        p, h = mappingH(v.cabeza,so)
        vcuer = v.map_variables2(p)
        cue = cue + vcuer.cuerpo
    return CQ(res.cabeza, cue, [])


if __name__ == "__main__":
    main(sys.argv[2], sys.argv[3], sys.argv[1])
