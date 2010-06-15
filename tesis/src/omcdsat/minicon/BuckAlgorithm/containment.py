from CQ.Seq import *
from CQ.SubObjetivo import *

def estaContenido(q1, q2):
    freeze, mapVars = congelarCuerpo(q1)
    ##print
    #print "freeze", freeze
    cabCong, varsInst = obtCabCongelada(q2, freeze, mapVars, q1.cabeza)
    #print "varsInst", varsInst
    ret = derivar(q2, freeze, varsInst)        
    return ret

def derivar(q2, freeze, varsInst):
    #print "q2", q2
    return derivar1(q2, freeze, varsInst, 0, len(q2.cuerpo))


def derivar1(q2, freeze, varsInst, i, n):
    if (i == n):
        return True
            
    subOb = q2.cuerpo[i]
    if subOb.predicado in freeze:
        for hecho in freeze[subOb.predicado]:
            #print "i", i, "hecho", hecho, "subOb", subOb, varsInst 
            res, inst = subOb.unifica(hecho, varsInst)
            b = False
            if res == "Yes":
                #print "Yes"
                b = derivar1(q2, freeze, varsInst, i+1, n)
            elif res == "Ins":
                #print "Ins", inst
                inst.update(varsInst)
                b = derivar1(q2, freeze, inst, i+1, n)
            if b :
                return True
    return False
        
def congelarCuerpo(q):
    freeze = {}
    map = {}
    seq = Seq()
    for subob in q.cuerpo:
        pred = subob.predicado
        args = subob.argumentos
        varsc = []
        for var in subob.orden:
            if args[var] == 0:
                if map.has_key(var):
                    varsc.append(map[var])
                else:
                    ctte = seq.next()
                    varsc.append(ctte)
                    map[var] = ctte
        subobcon = SubObjetivo(pred, {}, varsc)
        if freeze.has_key(pred):
            freeze[pred].append(subobcon)
        else:
            freeze[pred] = [subobcon]
    return (freeze, map)

def obtCabCongelada(q, freeze, mapVars, cabQ1):
    args = q.cabeza.argumentos
    pred = q.cabeza.predicado
    varsc = []
    varsInst = {}
    for i in xrange(len(q.cabeza.orden)):
        var = q.cabeza.orden[i]
        var2 = cabQ1.orden[i]
        if args[var] == 0:
            if mapVars.has_key(var2):
                varsc.append(mapVars[var2])
                varsInst[var] = mapVars[var2]
            else: # no deberia entrar por este caso pq el query deberia ser seguro
                print "no deberia entrar por este caso pq el query deberia ser seguro"
                ctte = seq.next()
                varsc.append(ctte)
                mapVars[var] = ctte
                varsInst[var] = ctte
    cabCong = SubObjetivo(pred, {}, varsc)
    return cabCong, varsInst
