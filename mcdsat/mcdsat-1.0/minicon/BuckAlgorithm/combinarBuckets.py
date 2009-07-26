from CQ.CQ import *
from containment import *
import pprint

def combinarBuckets1(query, buckets, i):
    if i == 0:
        reescrituras0 = []
        for x in buckets[0]:
            reescrituras0.append([x])
        return reescrituras0

    reescrituras = []
    ri = combinarBuckets1(query, buckets, i-1)
    for v in buckets[i]:
        for r1 in ri:
            #r = [v] OJO ESTO SE CAMBIO, HAY QUE REVISARLO 20050608
            r = r1 + [v]
            reescrituras.append(r)
    return reescrituras
        

def combinarBuckets(query, buckets):
    reescrituras = combinarBuckets1(query, buckets, len(buckets)-1)
    print "antes"
    pprint.pprint(reescrituras)
    i = 0
    print "Q", query
    final = []
    # Recorro la lista en reverso para poder borrar mientras se itera
    for x in xrange(len(reescrituras)-1, -1, -1):
        r = reescrituras[x]
        e = expandirVistas(r, query)
        if not e.esSeguro() or not estaContenido(e, query):
            del reescrituras[x]
    return reescrituras

def expandirVistas(vistas, query):
    cue = []
    comp = []
    for v in vistas:
        for so in v.cuerpo:
            cue.append(so)
    return CQ(query.cabeza, cue, comp)
            