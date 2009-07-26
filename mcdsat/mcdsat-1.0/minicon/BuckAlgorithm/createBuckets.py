# define el algoritmo de im
# - conjunto de vistas
# - esquema global
from CQ.Seq import *
import pprint

#Recibe una lista de vistas
#Un query CQ
def createBuckets(vistas, query):
    buckets = []
    subsQ = query.cuerpo
    conQ = query.comparacion
    print "Mappings: "
    for r in subsQ:
        bucketi = []
        for v in vistas:
            subsV = v.cuerpo
#            print v.cabeza, "    ",  r
            for s in subsV:
 #               print s
                if r.esIgual(s):
                    psi, psi_1 = mapping(r, s, v)
                    # faltan los predicados de orden
                    #if not esSeguro(query, v, psi_1):
                    v_map = v.map_variables(psi)
                    bucketi.append(v_map)
                    #break OJO REVISAR ESTO LUEGO 20050308
        buckets.append(bucketi)
    return buckets        

def mapping(r, s, v):
    mapping = {}
    mapping_1 = {}
    seq = Seq()
    cabezaY = v.cabeza.argumentos
    varsX = r.orden
    j = 0
    for y in s.orden:
        if cabezaY.has_key(y):
            mapping[y] = varsX[j]
            mapping_1[varsX[j]] = y
        else:
            mapping[y] = seq.nuevaVar(y)
        j = j + 1
    return mapping, mapping_1


#def map_variables(vista, mapping):

# esto viene del paper de minicom
# toda var de la cabeza de q debe ser mapeado a 
# una variable que este en la cabeza de la vista 
def esSeguro(query, vista, mapping):
    varsDisting = query.cabeza.argumentos
    for arg in varsDisting.keys():
        print mapping
        if not mapping.has_key(arg):
            print 'retorne false', arg
            return False
        argM = mapping[arg]
        if varsDisting[arg] == 0:
            if (not vista.estaVarEnCuerpo(argM)):
                print "hora", arg
                return False
    return True    




        
