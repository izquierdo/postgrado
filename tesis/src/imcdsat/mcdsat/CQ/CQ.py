from SubObjetivo import *
from Seq import *

CONSTANT_BASE = 1000000000
VAR_BASE = 400000000

def es_const(x):
    #hack para las reescrituras que colocan la x antes
    if type(x) == str:
        x = x.lstrip('_XYZ')

    if type(x) != int:
        x = int(x)

    return x >= CONSTANT_BASE

def es_var(x):
    return not es_const(x)
 
class CQ:
    def __init__(self, cab, cuer, comp):
        self.cabeza = cab
        print "cab es %s" % (cab,)
        if str(cab) == "v1(_0, _1)":
            None()
        self.cuerpo = cuer
        self.comparacion = comp
        self.vars,self.varsExist,self.varsDist = self.obtVariables();
        self.varsExistL = list(self.varsExist)

    def variables(self):
        return self.vars

    def obtVariables(self):
        vars = set()
        varsExist = set()
        varsDist = set()
        for so in self.cuerpo:
            for x in so.orden:
                vars.add(x)
                if self.esVarDisting(x):
                    varsDist.add(x)
                else:
                    varsExist.add(x)
        return vars, varsExist, varsDist

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
        return str(self.cabeza) + " :- " + ", ".join(str(x) for x in self.cuerpo)

    __repr__ = __str__

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
        for order in self.comparacion: 
            comp.append(order.map_variables(psi))#ojo con esto
        return CQ(cab, cuer, comp)

    def esVarDisting(self, var):
        return self.cabeza.argumentos.has_key(var) or es_const(var)

    def todasVarDisting(self):
        vars_cabeza = set([v for v in self.cabeza.argumentos.keys() if es_var(v)])
        vars_cuerpo = set([v for v in self.vars if es_var(v) and self.esVarDisting(v)])

        return vars_cuerpo.issubset(vars_cabeza)
    
    def imprimir(self, dic):
        for x in dic.keys():
            print x, map(str, dic[x])
        
    def obtSubObXVar(self, var):
        res = set([])
        i = 0
        for subOb in self.cuerpo:
            if subOb.argumentos.has_key(var):
                res.add(i)
        return res
                
