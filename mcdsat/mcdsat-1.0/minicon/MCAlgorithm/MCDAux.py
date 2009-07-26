import itertools
import copy
from sets import Set
from MCD import *

class MCDAux:
    ## hc: head homomorfismo (dict)
    ## phic: mapping vars(Q) en vars(V) (dict)
    ## Gc: subobjetivos cubiertos (set) 
    ## cabezaH: cabeza de la vista con hc aplicado a sus argumentos
    def __init__(self, query, vista, phictodo, gi, phic, hc, gc):#, mapSubOb):
        self.vista = vista
        self.query = query
        self.phictodo = phictodo
        self.phic, self.hc = phic, hc
        self.subObsPosibles = gc
        self.esValido = False
        self.clave = " phictodo:"+str(self.phictodo)+" phic:"+str(self.phic)+" hc:"+str(self.hc)+" esValido:"+str(self.esValido)
        self.cumpleProperty1_c1 = True
        self.obtMCDsGcminimal(gi)


    def extender(self, varq, varv, gi):
        phictodo2 = {} 
        for x,y in self.phictodo.iteritems():
            phictodo2[x]=y[:]
        phictodo2.setdefault(varq, []).append(varv)

        if gi and property1_c1(self.query, self.vista, phictodo2):
            phic, hc = calcularPhiH(self.vista, phictodo2)
            gc = calcularGcPosibles(self.query, phic, hc, self.vista, gi)
            return MCDAux(self.query, self.vista, phictodo2, gi, phic, hc, gc)
        else:
            return None


    # OJO CON ESTO PQ EL STRING DE UNA TABLA DE HASH NO ES NECESARIAMENTE EL MISMO SIEMPRE
    def __str__(self):
        return self.clave

    __repr__ = __str__

    def __hash__(self):
        return self.clave.__hash__()

    def __eq__(self, other):
        return self.clave == other.clave

    def asignarValido(self):
        self.esValido = True


    def obtMCDsGcminimal(self, gi):
        self.mcds = Set()
        self.obtMCDsGcminimal1(self.mcds, gi, self.subObsPosibles)
        eliminarNoMinimales(self.mcds)


    def obtMCDsGcminimal1(self, mcds, gc, subObsPosibles):
        if self.property1_c2(gc):
            mcd = MCD(self.vista, self.query, self.phictodo, gc, self.phic, self.hc)            
            if mcd.mapsTodosUtiles: #es un mapping invalido porque trata de unir en h variables no distinguibles
                mcds.add(mcd)
                self.asignarValido()
            return 

        n = len(subObsPosibles)
        for x in xrange(n):
            subob = subObsPosibles[x]
            gc2 = gc.copy()
            gc2.add(subob)
            subObsPosibles2 = subObsPosibles[x+1:]
            self.obtMCDsGcminimal1(mcds, gc2, subObsPosibles2)


    def property1_c2(self, gc):
        subobsvarq = self.query.cuerpo
        n = len(subobsvarq)
        for varq in self.phictodo:
            for varv in self.phictodo[varq]:
                if not self.vista.esVarDisting(varv):
                    for i in xrange(n):
                        g = subobsvarq[i]
                        if varq in g.argumentos:
                            if i not in gc or not property1c2_2(subobsvarq[i], self.vista):
                                return False
        return True


def calcularGcPosibles(query, phic, hc, vista, gi):
    gc = []
    i = 0
    for x in query.cuerpo:
        j = 0
        if i not in gi:
            for y in vista.cuerpo:
                if x.predicado == y.predicado:
                    soniguales = True
                    for (varx, vary) in itertools.izip(x.orden, y.orden):
                        if phic.get(varx, varx) != hc.get(vary,vary):
                            soniguales = False
                            break
                    if soniguales:
                        gc.append(i)
            j = j+1
        i = i+1
    return gc


def calcularPhiH(vista, phictodo, ):
    disting = vista.cabeza.argumentos
    h = {}
    htemp = []
    phi = {}
    error = False
    for var in phictodo:
        rep = phictodo[var][0]
        if len(phictodo[var]) > 1:
            if rep not in disting:
                return {}, {}

            for i in xrange(1,len(phictodo[var])):
                x = phictodo[var][i]
                if x in disting:
                    if x in h:
                        #print "xxx", x, rep, h[x]
                        h[rep]=h[x]
                        rep = h[x]
                else:
                    return {}, {}

            for i in xrange(1,len(phictodo[var])):
                x = phictodo[var][i]
                h[x] = rep

        phi[var] = rep

    clausura(h)

    for q,v in phi.iteritems():
        if v in h:
            phi[q] = h[v]

    return phi, h



def clausura(h):    
    for i in h:
        for x,y in h.iteritems():
            if i == y:
                h[x] = h[i]


def eliminarNoMinimales(mcds):
    listaMcds = list(mcds)
    n = len(listaMcds)
    for x in xrange(n):
        mcdx = listaMcds[x]
        gcmcdx = mcdx.gc
        for y in xrange(x+1,n):
            gcmcdy = listaMcds[y].gc
            if gcmcdy <= gcmcdx:
                mcds.remove(mcdx)
                break            


def property1c2_2(g, vista):
    for subob in vista.cuerpo:
        if g.predicado == subob.predicado:
            return True
    return False


def property1_c1(query, vista, phictodo):
    for varq in phictodo:
        for varv in phictodo[varq]:
            if not vista.esVarDisting(varv) and query.esVarDisting(varq):
                return False
    return True



