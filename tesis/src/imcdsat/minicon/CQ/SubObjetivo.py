# Representa los subobjetivos de un CQ
# los argumentos de un subobjetivo pueden ser variables o cttes
# tipo = 0 (variable)
# tipo = 1 (constante)
from sets import Set
class SubObjetivo:
    argumentos = {}
    orden = [] #Mantiene el orden de los argumentos en la tupla
    
    def __init__(self, pre, param, ord):
        self.predicado = pre
        self.argumentos = param
        self.orden = ord
        self.conjVars = Set(ord)
        self.clave = self.predicado + "(" + (", ".join(self.orden)) + ")"

    def __str__(self):
        return self.clave

    __repr__ = __str__

    def __hash__(self):
        return self.clave.__hash__()

    def __eq__(self, other):
        return self.clave == other.clave

    def esIgual(self, subob):
        return self.predicado == subob.predicado #and str(self.orden) == str(subob.orden)


    def esVarArgumento(self, arg):
        return arg in self.argumentos and self.argumentos[arg] == 0

    def map_variables(self, mapping):
        ord = []
        args = {}
        for var in self.orden:
            if var in mapping:
                psi_var = mapping[var]
                ord.append(psi_var)
                args[psi_var] = 0# self.argumentos[var]
            else:
                ord.append(var)
                args[var] = 0#self.argumentos[var]
        return SubObjetivo(self.predicado, args, ord)

    #igual que map_variables pero si encuentra una variable que no se encuentre en
    #el dominio de psi, la sustituye por una variable nueva
    def map_variables2(self, mapping, seq):
        ord = []
        args = {}
        for var in self.orden:
            if var in mapping:
                psi_var = mapping[var]
                ord.append(psi_var)
                args[psi_var] = self.argumentos[var]
            else:
                nuevaVar = seq.nuevaVar(var)
                ord.append(nuevaVar)
                args[nuevaVar] = 0
        return SubObjetivo(self.predicado, args, ord)



    def map_variables3(self, mapping):
        ord = []
        args = {}
        for var in self.orden:
            if mapping.has_key(var):
                psi_var = mapping[var]
                ord.append(psi_var)
                args[psi_var] = self.argumentos[var]
            else:
                ord.append(var)
                args[var] = self.argumentos[var]
        return SubObjetivo(self.predicado, args, ord)


    def unifica(self, hecho, varsInst):
        instancia = False
        inst = {}
        for i,x in enumerate(self.orden):
            if x in varsInst and varsInst[x] != hecho.orden[i]:
#                #print "no", varsInst, x, hecho
                return "No", []
            if x not in varsInst:
                inst[x] = hecho.orden[i]
                instancia = True
        if instancia:
#            #print "Ins"
            return "Ins", inst
#        #print "yes"
        return "Yes", []
                

            
    
    
