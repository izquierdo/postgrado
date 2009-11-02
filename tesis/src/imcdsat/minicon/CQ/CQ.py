from SubObjetivo import *
from Seq import *
class CQ:
    def __init__(self, cab, cuer, comp):
        self.cabeza = cab
        self.cuerpo = cuer
        self.comparacion = comp
        self.clave = str(self.cabeza) + " :- " + ", ".join(str(x) for x in self.cuerpo)

    def variables(self):
        self.vars, self.varsXsubob = self.obtVariables();
        return self.vars

    def obtVariables(self):
        vars = set()
        varsXsubob = {}
        for so in self.cuerpo:
            for x in so.orden:
                vars.add(x)
                varsXsubob.setdefault(x,set([])).add(so)
        return vars, varsXsubob

    def esSeguro(self):
        varsDisting = self.cabeza.argumentos
        for arg in varsDisting.keys():
            if varsDisting[arg] == 0:
                if (not self.estaVarEnCuerpo(arg)):
                    return False
        return True    

    def estaVarEnCuerpo(self, arg_tipo):
        for subObj in self.cuerpo:
            if subObj.esVarArgumento(arg_tipo):
                return True
        return False

    def __str__(self):
        return self.clave #+ map(str,(self.comparacion))

    __repr__ = __str__

    def __hash__(self):
        return self.clave.__hash__()

    def __eq__(self, other):
        return self.clave == other.clave


    def map_variables(self, psi):
        cuer = []
        comp = []
        cab = self.cabeza.map_variables(psi)
        for subob in self.cuerpo:
            cuer.append(subob.map_variables(psi))
        for order in self.comparacion:
            comp.append(order.map_variables(psi))
        return CQ(cab, cuer, comp)

    #igual que map_variables pero si encuentra una variable que no se encuentre en
    #el dominio de psi, la sustituye por una variable nueva
    def map_variables2(self, psi, seq):
        cuer = []
        comp = []
        cab = self.cabeza.map_variables2(psi, seq)
        for subob in self.cuerpo:
            cuer.append(subob.map_variables2(psi, seq))
        return CQ(cab, cuer, comp)


    def esVarDisting(self, var):
        return var in self.cabeza.argumentos
        
    def imprimir(self, dic):
        for x in dic.keys():
            print x, map(str, dic[x])
        
        
