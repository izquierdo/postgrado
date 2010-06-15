import copy
class MCD:
    ## hc: head homomorfismo (dict)
    ## phic: mapping vars(Q) en vars(V) (dict)
    ## Gc: subobjetivos cubiertos (set) 
    ## cabezaH: cabeza de la vista con hc aplicado a sus argumentos
    def __init__(self, vista, query, phictodo, gc, phic, hc):
        self.vista = vista
        self.phictodo = phictodo
        self.phic = phic
        self.hc = hc
        self.vistaH = vista.map_variables(self.hc)
        self.conjuntoMaps = self.calcularConjuntoMaps()
        mapSubOb = calcularMaps(query, phic, vista, self.vistaH, gc)
        if len(self.phic) > 0:            
            self.gc = gc
            self.ec, self.phi_1 = self.calcularEc(query)
            self.mapsTodosUtiles = self.calcularMapsTodosUtiles(query, mapSubOb)
            if len(self.gc) > 0:
                self.clave = "V:" + vista.cabeza.predicado + " tau:" + str(self.phictodo) + " gc:" + str(self.gc)
            else:
                self.clave = ""
        else:
            self.gc, self.mapsTodosUtiles = set(), False

    def __str__(self):
        return self.clave

    __repr__ = __str__

    def __hash__(self):
        return self.clave.__hash__()

    def __eq__(self, other):
        return self.clave == other.clave

    def calcularMapsTodosUtiles(self, query, mapSubOb):
        if self.gc:
            for x in self.phictodo:
                phicx = self.phictodo[x]
                for y in phicx:
                    if (x,y) not in mapSubOb:
                        return False
            return True
        else:
            return False


    def calcularConjuntoMaps(self):
        conjuntoMap = set()
        for x in self.phictodo:
            for y in self.phictodo[x]:
                conjuntoMap.add((x,y))
        return conjuntoMap
        

    def calcularEc(self, query):
        phi_1 = {}
        for var in self.phic:
            pvar = self.phic[var]
            if pvar in phi_1:
                if query.esVarDisting(var):
                    phi_1[pvar] = [var]+ phi_1[pvar] #colocar las variables distinguibles al principio de la lista 
                else:
                    phi_1[pvar].append(var)
            else:
                phi_1[pvar] = [var]
        ec = {}
        for var in phi_1:
            rep = phi_1[var][0] # como las variables distinguibles se colocaron al inicio, escoje al primero
            if len(phi_1[var]) > 1:
                for x in phi_1[var]:
                    ec[x] = rep
            else:
                ec[rep] = rep
        return ec, phi_1


    def esValido(self):
        return self.phic
                                

    # toda variable distinguible en el query que este en el dominio de phic 
    # tambien es distinguible en vistaH ( h(vista) )
    def propiedad1c1(self, query):
        distingh = self.vistaH.cabeza.argumentos
        for var in query.cabeza.argumentos:
            if var in self.phic:
                if self.phic[var] in self.hc:
                    variable = self.hc[self.phic[var]]
                else:
                    variable = self.phic[var]
                if variable not in distingh:
                    return False                
        ###print "propiedad1c1", True
        return True

    # toda variable phi(x) que no sea distinguible en la vista
    # debe forzar todos los joins a traves del mapping phi.
    def propiedad1c2(self, query):
        distingh = self.vistaH.cabeza.argumentos
        for var in self.phic:
            if self.phic[var] not in distingh:
                for g in query.cuerpo:
                    if var in g.argumentos:
                        if not self.propiedad1c2_1(g) or not self.propiedad1c2_2(g):
                            return False
        return True

    # todas las variables en g deben estar en el dominio de phic
    def propiedad1c2_1(self, g):
        for var in g.argumentos:
            if self.phic[var] not in self.phic:
                return False                
        return True

    # phi(g) \in hv(V)
    def propiedad1c2_2(self, g):
        phi_g = g.map_variables(self.phic)
        s_phi_g = str(phi_g)
        for subOb in self.vistaH.cuerpo:
            if s_phi_g == str(subOb):
                return True
        return False

    def obtUnificacion(self, ecgeneral):
        ret = {}
        for y in self.phi_1:
            x = self.phi_1[y][0]
            if x in ecgeneral:
                x = ecgeneral[x]
            ret[y] = x
        return ret


def calcularMaps(query, phic, vista, vistaH, gi):
    mapSubOb = set()
    i = 0
    for x in query.cuerpo:
        j = 0
        gx = x.map_variables(phic)
        for y in vistaH.cuerpo:
            if str(gx) == str(y):
                agregarMapsXSubOb(query.cuerpo[i],vista.cuerpo[j],mapSubOb)
            j = j+1
        i = i+1
    return mapSubOb

def agregarMapsXSubOb(subobq, subobv, mapSubOb):
    n = len(subobq.orden)
    for i in xrange(n):
        mapSubOb.add((subobq.orden[i],subobv.orden[i]))



######################################################################3
#     def calcularGc(self, query):
#         gc = []
#         i = 0
#         for subobq in query.cuerpo:
#             cubreSubob = True
#             for y in subobq.argumentos:
#                 if y in self.phic:
#                     cubreSubob = True
#                     gc.append(i)
#             if cubreSubob:
#                 gc.append(i)
#             i = i+1
#         return set(gc)

