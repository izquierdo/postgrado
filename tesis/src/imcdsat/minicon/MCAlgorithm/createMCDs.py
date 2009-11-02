# define el algoritmo de im
# - conjunto de vistas
# - esquema global

import copy
import pprint
from MCD import *
from MCDAux import *

#Recibe una lista de vistas
#Un query CQ
def createMCDs(vistas, query):
    C = set([])
    subsQ = query.cuerpo
    conQ = query.comparacion
    n = len(subsQ)
    for i in xrange(n):
        g = subsQ[i]
        for V in vistas:
            subsV = V.cuerpo
            maps = obtPosiblesMaps(query, V)
            for v in subsV:
                if v.esIgual(g):
                    phitodo = mappingH(g, v, V)
                    if phitodo:
                        gi = set([i])
                        mcdaux = crearMCDAux(query, V, phitodo, gi)
                        if mcdaux != None:
                            extenderMapping(C, mcdaux, query, V, maps, gi)
        i = i + 1
    eliminarMCDsNoMinimales(C);
    return C


def crearMCDAux(query, vista, phictodo, gi):
    if not property1_c1(query, vista, phictodo):
        return None
    else:
        phic, hc = calcularPhiH(vista, phictodo)
        gc = calcularGcPosibles(query, phic, hc, vista, gi)
        return MCDAux(query, vista, phictodo, gi, phic, hc, gc)


def extenderMapping(C, mcdaux, query, vista, maps, gi):
    if not mcdaux.cumpleProperty1_c1:
        return 

    mapsFaltan = obtMapsFaltantes(maps, mcdaux.phictodo)
    return extenderMapping1(C, mcdaux, query, vista, mapsFaltan, gi)


def extenderMapping1(mcds, mcdaux, query, vista, maps, gi):
    if mcdaux is None:
        return 

    if len(mcdaux.phic) == 0:
        return 

    if mcdaux.esValido:
         mcds |= mcdaux.mcds
         return

    if len(maps) == 0:
        return
    
    n = len(maps)
    for x in xrange(n):
        (varq, varv) = maps[x]
        maps2 = maps[x+1:]
        mcdauxN = agregarMapping(mcds, mcdaux, varq, varv, query, vista, gi)
        extenderMapping1(mcds, mcdauxN, query, vista, maps2, gi)


def obtPosiblesMaps(query, V):
    maps = set([])
    for subobq in query.cuerpo:
        for subobv in V.cuerpo:
            if subobq.predicado == subobv.predicado:
                variablesq = subobq.orden
                variablesv = subobv.orden
                n = len(variablesq)
                for x in xrange(n):
                    maps.add((variablesq[x], variablesv[x]))
    return maps


def agregarMapping(mcds, mcdaux, varq, varv, query, vista, gi):
    return mcdaux.extender(varq, varv, gi)

def mappingH(g, v, V):
    phitodo = {}
    varsY = v.orden
    j = 0
    for x in g.orden:
        phitodo.setdefault(x,set([])).add(varsY[j])
        j = j + 1

    for i in phitodo:
        phitodo[i]=list(phitodo[i])
    return phitodo

def obtMapsFaltantes(maps, phictodo):
    maps2 = maps.copy()
    for varq in phictodo:
        for varv in phictodo[varq]:
            maps2.remove((varq, varv)) 
    return list(maps2)

def eliminarMCDsNoMinimales(mcds):
    listaMcds = list(mcds)
    n = len(listaMcds)
    for x in xrange(n):
        mcdx = listaMcds[x]
        gcmcdx = mcdx.gc
        mapsmcdx = mcdx.conjuntoMaps
        for y in xrange(x+1,n):            
            mcdy = listaMcds[y]
            if x != y and mcdx.vista.cabeza.predicado == mcdy.vista.cabeza.predicado:
                gcmcdy = mcdy.gc
                mapsmcdy = mcdy.conjuntoMaps
                if mapsmcdy <= mapsmcdx and gcmcdy <= gcmcdx:
                    mcds.discard(mcdx)
                    break
                elif mapsmcdy >= mapsmcdx and gcmcdy >= gcmcdx:
                    mcds.discard(mcdy)



