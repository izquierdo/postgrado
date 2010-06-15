from BuckAlgorithm.containment import estaContenido
from CQ.CQ import *
from Unification import unificar
import pprint

# para combinar los MCDs se procede recursivamente
# cada mcd se combina con todos los demas para ver
# si se puede encontrar una combinacion que tenga 
# todos los subobjetivos del query
def combinarMCDs1(mcds, n, i, cubiertos):
    if i == n:
        return []
    mcd = mcds[i]
    s = mcd.gc # s es un set de python
    if not s <= cubiertos:
        return []
    
    cubiertos = cubiertos - s
    if len(cubiertos) == 0:
        return [[mcd]]

    combinaciones = []
    for m in xrange(i+1,n+1):
        combs = combinarMCDs1(mcds, n, m, cubiertos)
        for x in combs:
            x.append(mcd)
            combinaciones.append(x)
    return combinaciones

def combinarMCDs(query, mcds):
    numSOQ = len(query.cuerpo)
    cubiertos = set(range(numSOQ))
    numMCD = len(mcds)
    mcdsc = []
    reescrituras = []
    mcdslist = list(mcds)
    for m in xrange(len(mcds)):
        mcdsc = mcdsc + combinarMCDs1(mcdslist, numMCD, m, cubiertos)
        
    for mcdc in mcdsc:
        reescrituras.append(crearReescritura(mcdc, query))
    return reescrituras



def crearReescritura(mcdc, query):
    r = []
    seq = Seq()
    ecgeneral = unificar([k.ec for k in mcdc], query.variables())
    for m in mcdc:
        unif = m.obtUnificacion(ecgeneral)
        vis = m.vistaH.map_variables2(unif, seq)
        r.append(vis)

    cue = [v.cabeza for v in r]
    cab = query.cabeza.map_variables3(ecgeneral)
    res = CQ(cab, cue, [])
    return res    



def expandirVistas(vistas, query, ec):
    cue = []
    comp = []
    cab = query.cabeza.map_variables(ec)
    for v in vistas:
        for so in v.cuerpo:
            cue.append(so)
    return CQ(cab, cue, comp)


