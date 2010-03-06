import timeit
import os
import resource
import pprint
import random
import cPickle

from Parser import *
from Parser.CQparser import *
from CQ.Argumento import *
from CQ.Predicado import *
from CQ.SubObjetivo import *
from CQ.CQ import *
from CQ.SOComparacion import *
from TransformarFormula import *
from VariableSat import *


archVistas = ''
archCons = ''
varsV = {}
varsG = {}
varsT = {}
varsZ = {}
contribVT = {}

def traducir(exp, archV, archC, archVars, archTiempo, archSalida, archCostosSat):
    #tiempo = timeit.Timer('traducir1()', "from __main__ import traducir1; import psyco; psyco.full()").timeit(1)/1
    tiempoi = resource.getrusage(resource.RUSAGE_SELF)[0]
    traducir1(exp, archV, archC, archVars, archSalida, archCostosSat)
    tiempof = resource.getrusage(resource.RUSAGE_SELF)[0]
    
    fileobject = open(archTiempo, 'w')
    fileobject.write(str(tiempof-tiempoi))
    fileobject.close()

def traducirBig(exp, archV, archC, archVars, archTiempo, archSalida, archCostos):
    #tiempo = timeit.Timer('traducir1()', "from __main__ import traducir1; import psyco; psyco.full()").timeit(1)/1
    tiempoi = resource.getrusage(resource.RUSAGE_SELF)[0]
    traducir1Big(exp, archV, archC, archVars, archSalida, archCostos)
    tiempof = resource.getrusage(resource.RUSAGE_SELF)[0]
    
    fileobject = open(archTiempo, 'w')
    fileobject.write(str(tiempof-tiempoi))
    fileobject.close()

def traducir1(exp, archVistas, archCons, archVars, archSalida, archCostosSat): #(archVistas, archCons):
    vistas = cargarCQ(archVistas)
    consultas = cargarCQ(archCons)
    for q in consultas:
        if exp == 'SatRW' or exp == 'SatBestRW':
            transf = traducirConsultaRW(q, vistas, archSalida)
        elif exp == 'Sat':
            transf = traducirConsultaMCD(q, vistas, archSalida)
        guardarVars(transf, archVars)

def traducir1Big(exp, archVistas, archCons, archVars, archSalida, archCostos): #(archVistas, archCons):
    vistas = cargarCQ(archVistas)
    consultas = cargarCQ(archCons)
    for q in consultas:
        traducirConsultaBigBestRW(q, vistas, archSalida, archCostos)

def guardarVars(transf, archVars):
    fh = open(archVars,'w')
    cPickle.dump(transf.numeros, fh)
    fh.close()


def traducirConsultaMCD(q, vistas, archSalida):
    variables, clausulas = generarTeoriaMCD(q, vistas)
    transf = TransformarFormula(variables)
    n = 1
    imprimirCopias(variables, clausulas, [], n, transf, archSalida)
    return transf


def traducirConsultaRW(q, vistas, archSalida):
    variables, clausulas = generarTeoriaMCD(q, vistas)
    transf = TransformarFormula(variables)
    n = len(q.cuerpo)
    clausulas2 = clausulasCombinarMCD(transf, n, q, vistas)
    imprimirCopias(variables, clausulas, clausulas2, n, transf, archSalida)
    return transf

def traducirConsultaBigBestRW(q, vistas, archSalida, archCostos):
    #costos
    costFile = open(archCostos, 'r')
    costStr = costFile.read()
    costFile.close()
    costList = costStr.split()[1:]
    costDict = {}

    for i in range(0, len(costList), 2):
        costDict[costList[i]] = costList[i+1]

    #impresion

    variables, clausulas = generarTeoriaMCD(q, vistas)
    transf = TransformarFormula(variables)
    n = len(q.cuerpo)
    clausulas2 = clausulasCombinarMCD(transf, n, q, vistas)
    imprimirCopiasPeso(variables, clausulas, clausulas2, n, costDict, transf, archSalida)
    return transf

def clausulasCombinarMCD(transf, n, q, vistas):
    clausulas2 = []
    for numG in xrange(n):
        varG = VariableSat(False, 'g', [numG])
        varG1 = VariableSat(True, 'g', [numG])
        clsPorLoMenos1 = ''
        for numCopiaX in xrange(n):
            numVarGX = transf.var2NumSimple(varG,numCopiaX)
            clsPorLoMenos1 = clsPorLoMenos1 + transf.var2NumSimple(varG1,numCopiaX) + ' '
            for numCopiaY in xrange(numCopiaX+1, n):
                numVarGY = transf.var2NumSimple(varG,numCopiaY)
                # clausulas C16
                clausulas2.append(numVarGX + ' ' + numVarGY + ' 0\n')
            clausulaSimet= numVarGX+' '
            if numG > 0 and numCopiaX > 0:
                for numGmenor in xrange(numG):
                    varG2 = VariableSat(True, 'g', [numGmenor])                
                    clausulaSimet= clausulaSimet + transf.var2NumSimple(varG2,numCopiaX-1) + ' '
                # clausulas C17
                clausulas2.append(clausulaSimet+'0\n')
            elif numG == 0 and numCopiaX == 0:
                varG2 = VariableSat(True, 'g', [0])                
                clausulas2.append(transf.var2NumSimple(varG2,0) + ' 0\n')                    
        # clausulas C15
        clausulas2.append(clsPorLoMenos1+'0\n')

    varsQ = set([])
    varsVs = set([])

    for so in q.cuerpo:
        varsQ.update(set(so.orden))

    for v in vistas:
        for so in v.cuerpo:
            varsVs.update(set(so.orden))

    for x in varsQ:
        for a in varsVs:
            for b in varsVs:
                if es_const(a) and es_const(b) and (a!=b):
                    for numCopiaT0 in xrange(n):
                        for numCopiaT1 in xrange(n):
                            if numCopiaT0 != numCopiaT1:
                                varT0 = VariableSat(False, 't', [int(x), int(a)])
                                varT1 = VariableSat(False, 't', [int(x), int(b)])

                                try:
                                    numVarT0 = transf.var2NumSimple(varT0, numCopiaT0)
                                    numVarT1 = transf.var2NumSimple(varT1, numCopiaT1)
                                except KeyError:
                                    continue

                                #clausulas para constantes en teoria extendida (S6)
                                clausulas2.append("%s %s 0\n" % (str(numVarT0), str(numVarT1)))


    return clausulas2
                


def imprimirCopias(variables, clausulas, clausulas2, numCopias, transf, archSalida):
    arch = file(archSalida, 'w+')
    numVars = len(variables)*numCopias
    numCl = len(clausulas)*numCopias + len(clausulas2)
    arch.write('p cnf '+ str(numVars) + ' ' + str(numCl) + '\n')


    for cls in clausulas2:
        arch.write(cls)

    for numCopia in xrange(numCopias):
        transf.formula2Num(clausulas, numCopia, arch)

    arch.write('%\n')
    arch.close()

def imprimirCopiasPeso(variables, clausulas, clausulas2, numCopias, pesos, transf, archSalida):
    arch = file(archSalida, 'w+')
    numVars = len(variables)*numCopias
    numCl = len(clausulas)*numCopias + len(clausulas2)
    top = 2**20-1
    arch.write('p wcnf '+ str(numVars) + ' ' + str(numCl) + ' ' + str(top) + '\n')

    for cls in clausulas2:
        arch.write(str(top) + ' ' + cls)

    for numCopia in xrange(numCopias):
        transf.formula2NumWeighted(clausulas, numCopia, arch, pesos, top)

    #arch.write('%\n')
    arch.close()

def generarTeoriaMCD(q, vistas):
    global varsV
    global varsG
    global varsT
    global varsZ
    lv, c1, c2 = variablesV(q, vistas)
    lg, c3 = variablesG(q, lv)
    lt, lz, c6, c7, c8, c9, c14, ltaux = clausulas678(q, vistas)


    # support for constants

    d1, d2, d4, lt_d4d5, d5, d6 = clausulas_d1d2d4d5d6(q, vistas, ltaux, lz)
    d3 = clausulas_d3(q, vistas)

    lt.update(lt_d4d5)

    c10, c11 = clausulas11(lt, lv, ltaux)

    variables = []
    variables = lv+ lg+ list(lt)+ lz

#    print "V",varsV,lv
#    print "G",varsG,lg
#    print "T",len(varsT),len(lt)
#    print "Z",len(varsZ),len(lz)
    c12 = clausulas12(vistas, lv, lg)
    c4 = clausulas4(q,vistas,ltaux)
    
    c5, c13 = clausulas513(q,vistas,ltaux)
    c15 = clausulas15(q, vistas)
    #print "clausulas C1  \/ vi (por lo menos uno)"
    #pprint.pprint(c1) 
    #print "clausulas C2  -vi \/ -vj (maximo uno)"
    #pprint.pprint(c2) 
    #print "clausulas C3 y C4 \/ gk (por lo menos uno)"
    #pprint.pprint(c3) 
    #print "clausulas C9 Vm /\ tij => -tik y property 1 C2 "
    #pprint.pprint(c4) 
    #print "clausulas C10 vm => -tij (i Dist y j exist) "
    #pprint.pprint(c5)
    #print "clausulas C7 (=>) gi /\ vm => \/ zir (r subob de Vm)"
    #pprint.pprint(c6) 
    #print "clausulas C12 zir => tir"
    #pprint.pprint(c7) 
    #print "clausulas C7 (<=) gi /\ vm <= \/ zir (r subob de Vm)"
    #pprint.pprint(c8)
    #print "clausulas C5 maximo una z por vm, gi"
    #pprint.pprint(c9)
    #print "clausulas C15? t explicito"
    #pprint.pprint(c10)
    #print "clausulas C8  tik => \/ vm (si tik entonces alguna vm)"
    #pprint.pprint(c11)
    #print "clausulas C13 v_m & g_j => -g_k "
    #pprint.pprint(c12)
    #print "clausulas C11  t_ij => -t_kj"
    #pprint.pprint(c13)
    #print "clausulas C6  v_i => -gk cuando los preds son diff"
    #pprint.pprint(c14)

    #print "clausulas S1  t_{x,A} => -t_{x,B}"
    #pprint.pprint(d1)
    #print "clausulas S2  t_{A,x} => -t_{B,x}"
    #pprint.pprint(d2)
    #print "clausulas S3  -t_{A,B}"
    #pprint.pprint(d3)
    #print "clausulas S4  v_i /\\ t_{A,y} /\\ t_{x,y} /\\ t_{x,z} => t_{A,z}"
    #pprint.pprint(d4)
    #print "clausulas S5  v_i /\\ t_{y,A} /\\ t_{y,x} /\\ t_{z,x} => t_{z,A}"
    #pprint.pprint(d5)
    #print "clausulas d6  vi => -t_{x,A} si A no aparece en cuerpo de vi"
    #pprint.pprint(d6)

    #c10=[]
    clausulas =  c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 + c11 + c12 + c13 + c14 + c15
    clausulas = clausulas + d1 + d2 + d3 + d4 + d5# + d6
    #clausulas = clausulas + d1 + d2 + d3

    return variables, clausulas

def clausulas15(query, vistas):
    global varsZ
    n= len(query.cuerpo)
    c15 = []
    z = 0

    for v in vistas:
        ns = len(v.cuerpo)
        for y in xrange(ns):
            for x1 in xrange(n):
                for x2 in xrange(x1+1, n):
                    varz1 = varsZ.get((x1,y,z))
                    varz2 = varsZ.get((x2,y,z))
                    if varz1 != None and varz2 != None:
                        c15.append([varz1.negarVar(), varz2.negarVar()])                        
        z = z+1
    return c15
            


def clausulas11(lt, lv, ltaux):
    c11 = []
    for t in lt:
        c11temp = [t.negarVar()]
        for v in lv:
            if (t, v) in ltaux:
                c11temp.append(v)
        c11.append(c11temp)
    c10 = []
    for ((t, v), varzs) in ltaux.iteritems():
        c10.append([t.negarVar(), v.negarVar()] + list(varzs))
    return c10, c11

def clausulas4(q, vistas, lt):
    c4 = []
    nv = 0
    for v in vistas:
        varstodas = v.variables()
        varsexist = v.varsExist
        for varq in q.variables():
            for var in varsexist:
                vista = vistas[nv]
                clausula4(q, varq, nv, var, varstodas, lt, c4)
        nv = nv + 1
    return c4

def clausula4(q, varq, vis, varj, varO, lt, c4temp):
    global varsV
    global varsT
    varv = varsV[vis]
    varqi = int(varq)
    varji = int(varj)
    variableij = varsT.get((varqi, varji))

    if (variableij, varv) in lt:
        clausulag4(q, varq, varv, variableij, c4temp)
        for vark in varO:
            varki = int(vark)
            if vark != varj:
                variableik = varsT.get((varqi, varki))
                if (variableik, varv) in lt:
                    c4temp.append([varv.negarVar(),
                                   variableij.negarVar(),
                                   variableik.negarVar()
                                   ])


def clausulag4(q, varq, varv, variableij,c4temp):
    global varsG
    g = 0
    for subOb in q.cuerpo:
        if varq in subOb.argumentos:
            c4temp.append([varv.negarVar(),
                           variableij.negarVar(),
                           varsG[g]
                           ])
        g = g + 1

def clausulas513(q, vistas, lt):
    global varsV
    global varsT
    c5 = []
    nv = 0
    c13 = []
    varsE = q.varsExistL
    n = len(varsE)
    for v in vistas:
        varvt = varsV[nv]
        for varv in v.variables():
            if not v.esVarDisting(varv):
                for varq in q.varsDist:
                    vart = varsT.get((int(varq), int(varv)))
                    if (vart, varvt) in lt:
                        c5.append([varvt.negarVar(),
                                   vart.negarVar()
                                   ])

            
            for x in xrange(n):
                vartx = varsT.get((int(varsE[x]), int(varv)))
                if (vartx, varvt) in lt :
                    for y in xrange(x+1,n):
                        varty = varsT.get((int(varsE[y]), int(varv)))
                        if (varty, varvt) in lt:
                            continue #TODO disabled
                            c13.append([vartx.negarVar(),
                                        varty.negarVar(),
                                        varvt.negarVar()
                                        ])
            
        nv = nv + 1
    return c5, c13

def variablesV(q, vistas):
    global varsV
    vnula = VariableSat(True, 'v', [-1])
    lv =[vnula]
    varsV[-1] = vnula
    c2 = []
    numVista = 0
    for v in vistas:
        var = VariableSat(True, 'v', [numVista])
        lv.append(var)
        varsV[numVista]=var
        numVista = numVista + 1
    n = len(lv)
    for x in xrange(n):
        for y in xrange(x+1,n):
            c2.append([lv[x].negarVar(), lv[y].negarVar()])
    return lv, [lv], c2


def variablesG(q, lv):
    global varsG
    lg =[]
    c3 = []
    numSO = 0
    for v in q.cuerpo:
        var = VariableSat(True, 'g', [numSO])
        lg.append(var)
        varsG[numSO]=var
        numSO = numSO + 1
    for g in lg:
        c3temp = [lv[0].negarVar(), g.negarVar()]
        c3.append(c3temp)                
    for v in lv[1:]:
        c3temp=[v.negarVar()] +lg
        c3.append(c3temp)        
    return lg, c3

#v_m & g_j => -g_k
def clausulas12(vistas, lv, lg):
    c12 = []
    n = len(lg)
    for i in xrange(1, len(lv)):
        vista = vistas[lv[i].indices[0]]
        if vista.todasVarDisting():
            for x in xrange(n):
                for y in xrange(x+1,n):
                    #pass
                    c12.append([lv[i].negarVar(), lg[x].negarVar(), lg[y].negarVar()])                    
    return c12

def clausulas_d1d2d4d5d6(q, vistas, ltaux, lz):
    global varsT

    variables_query = []
    constantes_query = []

    variables_vistas = []
    constantes_vistas = []

    # obtener listas de variables y constantes para consulta

    for so in q.cuerpo:
        for v in so.orden:
            if es_var(v):
                variables_query.append(int(v))
            else:
                constantes_query.append(int(v))

    # obtener listas de variables y constantes para vistas

    for vista in vistas:
        for so in vista.cuerpo:
            for v in so.orden:
                if es_var(v):
                    variables_vistas.append(int(v))
                else:
                    constantes_vistas.append(int(v))

    variables_query = list(set(variables_query))
    variables_vistas = list(set(variables_vistas))
    constantes_query = list(set(constantes_query))
    constantes_vistas = list(set(constantes_vistas))

    d1 = clausulas_d1(variables_query, constantes_query, variables_vistas, constantes_vistas)
    d2 = clausulas_d2(variables_query, constantes_query, variables_vistas, constantes_vistas)
    d4, d5, lt_d4d5 = clausulas_d4d5(q, vistas, variables_query, constantes_query, ltaux, lz)

    d6 = clausulas_d6(vistas, variables_query, constantes_vistas)

    return d1, d2, d4, list(set(lt_d4d5)), d5, d6

def clausulas_d1(variables_query, constantes_query, variables_vistas, constantes_vistas):
    global varsT

    d1 = []

    for x in variables_query:
        for a in constantes_vistas:
            t_xa = varsT.get((x, a))

            if t_xa:
                for b in constantes_vistas:
                    if a != b:
                        t_xb = varsT.get((x, b))

                        if t_xb:
                            d1.append([t_xa.negarVar(), t_xb.negarVar()])

    return d1

def clausulas_d2(variables_query, constantes_query, variables_vistas, constantes_vistas):
    global varsT

    d2 = []

    for x in variables_vistas:
        for a in constantes_query:
            t_ax = varsT.get((a, x))

            if t_ax:
                for b in constantes_query:
                    if a != b:
                        t_bx = varsT.get((b, x))

                        if t_bx:
                            d2.append([t_ax.negarVar(), t_bx.negarVar()])

    return d2

def clausulas_d3(q, vistas):
    global varsT

    clausulas = set()

    for soq in q.cuerpo:
        for v in vistas:
            for sov in v.cuerpo:
                for vq in soq.orden:
                    vq = int(vq)
                    if es_const(vq):
                        for vv in sov.orden:
                            vv = int(vv)
                            if es_const(vv):
                                if vq != vv:
                                    vt = varsT.get((vq, vv))

                                    if vt:
                                        clausulas.add(vt.negarVar())

    return [[vt] for vt in clausulas]

def clausulas678(q, vistas):
    global varsT
    global varsZ
    global varsG
    global varsV
    c6 = []
    c7 = []
    c8 = []
    c9 = []
    c10 = []
    lz = []
    lt = set([])
    ltaux = {}
    c6temp = []
    c14temp = {}
    i = 0
    for subOb in q.cuerpo:
        pred = subOb.predicado
        m = 0
        varg = varsG[i]
        for v in vistas:
            j = 0
            varm = varsV[m]
            c6temp = [varg.negarVar(), varm.negarVar()]
            c9temp = []
            subObCubre = False
            for subObtemp in v.cuerpo:
                predtemp = subObtemp.predicado
                if pred == predtemp:
                    varz = VariableSat(True, 'z', [i,j,m])
                    lz.append(varz)
                    varsZ[(i,j,m)]=varz
                    c6temp.append(varz)
                    c9temp.append(varz)
                    lttemp = clausula78a(varz, varg, varm, subOb, subObtemp, m, ltaux, c7, c8, v)
                    lt |= lttemp
                    subObCubre = True
                j = j + 1
            c9 = c9 + clausula9(c9temp)
            m = m + 1
            if len(c6temp) > 2:
                c6.append(c6temp)
            if not subObCubre == True:
                c14temp.setdefault(varm.negarVar(),[]).append(varg.negarVar())
        i = i+1
    return lt, lz, c6, c7, c8, c9, clausula14(c14temp), ltaux


def clausula14(c14temp):
    c14 = []
    for (varm, listvarg) in c14temp.iteritems():
        for varg in listvarg:
            c14.append([varg, varm])
    return c14

def clausula9(c9temp):
    c9 = []
    n = len(c9temp)
    for x in xrange(n):
        for y in xrange(x+1,n):
           c9.append([c9temp[x].negarVar(), c9temp[y].negarVar()])
    return c9

def clausula78a(varz, varg, varm, subObQ, subObV, vis, ltaux, c7, c8, vista):
    global varsT
    global contribVT
    c8temp1 = [varz, varm.negarVar()]
    c8temp2 = [varm]
    lt = set([])
    i = 0
    for x in subObQ.orden:
        y = subObV.orden[i]
        varT = VariableSat(True, 't', [int(x), int(y)])
        lt.add(varT)
        varsT[(int(x), int(y))]=varT
        contribVT.setdefault(vista, set([])).add((int(x), int(y)))
        c7temp = [varm.negarVar(), varz.negarVar(), varT]
        c7.append(c7temp)
        c8temp1.append(varT.negarVar())
        c8temp2.append(varT.negarVar())
        i = i + 1
        ltaux.setdefault((varT,varm),set([])).add(varz)    
    c8.append([varz.negarVar(), varg])
    c8.append([varz.negarVar(), varm])
    return lt

def clausulas_d4d5(q, vistas, variables_query, constantes_query, ltaux, lz):
    global varsT
    global varsV

    lt_d4d5 = []

    #return compatible mappings in a, b, c
    #TODO more efficient implementation
    def compatibles(a, b, c):
        ret = set()

        for (aq, av) in a:
            for (bq, bv) in b:
                for (cq, cv) in c:
                    if (aq == bq and av == bv) or (aq != bq and av != bv):
                        if (aq == cq and av == cv) or (aq != cq and av != cv):
                            if (bq == cq and bv == cv) or (bq != cq and bv != cv):
                                ret.add((aq, av))
                                ret.add((bq, bv))
                                ret.add((cq, cv))

        return ret

    mappings = set()
    newmappings = set()

    viewmappings = {}

    createdmappings = set()

    for (v, numVista) in zip(vistas, range(len(vistas))):
        varV = VariableSat(True, 'v', [numVista])
        currentmappings = set()
        newmappings = set()

        providers = {}
        soqn = 0

        for soq in q.cuerpo:
            sovn = 0

            for sov in v.cuerpo:
                if soq.predicado == sov.predicado:
                    for (x, y) in zip(soq.orden, sov.orden):
                        newmappings.add((x, y))
                        providers.setdefault((x, y), set()).add((soqn, sovn))

                sovn += 1

            soqn += 1

        updated = True

        while updated:
            currentmappings.update(newmappings)
            newmappings = set()
            updated = False

            # PARA D4
            for (a, y0) in currentmappings:
                if es_var(a) or es_const(y0):
                    continue

                for (x1, y1) in currentmappings:
                    if es_const(x1) or (y0 != y1) or (x1 == y1):
                        continue

                    for (x2, z) in currentmappings:
                        if (x1 != x2) or (x2 == z):
                            continue

                        cs = compatibles(providers[(a,y0)], providers[(x1,y1)], providers[(x2,z)])

                        if (a, z) not in currentmappings:
                            if len(cs) > 0:
                                updated = True
                                newmappings.add((a, z))
                                providers[(a,z)] = set()

                                #create the mapping var
                                varT = varsT.get((int(a),int(z)))

                                if varT is None:
                                    varT = VariableSat(True, 't', [int(a), int(z)])
                                    varsT[(int(a), int(z))]=varT
                                    lt_d4d5.append(varT)
                                    createdmappings.add(varT)

                        varT = varsT.get((int(a),int(z)))

                        if varT:
                            providers.setdefault((a,z), set()).update(cs)

                            for (soqn, sovn) in providers[(a,z)]:
                                varZ = varsZ.get((soqn,sovn,numVista))
                                ltaux.setdefault((varT,varV),set([])).add(varZ)    

            # PARA D5
            for (y0, a) in currentmappings:
                if es_var(a) or es_const(y0):
                    continue

                for (y1, x1) in currentmappings:
                    if es_const(x1) or (y0 != y1) or (x1 == y1):
                        continue

                    for (z, x2) in currentmappings:
                        if (x1 != x2) or (x2 == z):
                            continue

                        cs = compatibles(providers[(y0,a)], providers[(y1,x1)], providers[(z,x2)])

                        if (z, a) not in currentmappings:
                            if len(cs) > 0:
                                updated = True
                                newmappings.add((z, a))
                                providers[(z,a)] = set()

                                #create the mapping var
                                varT = varsT.get((int(z),int(a)))

                                if varT is None:
                                    varT = VariableSat(True, 't', [int(z), int(a)])
                                    varsT[(int(z), int(a))]=varT
                                    lt_d4d5.append(varT)
                                    createdmappings.add(varT)

                        varT = varsT.get((int(z),int(a)))

                        if varT:
                            providers.setdefault((z,a), set()).update(cs)

                            for (soqn, sovn) in providers[(z,a)]:
                                varZ = varsZ.get((soqn,sovn,numVista))
                                ltaux.setdefault((varT,varV),set([])).add(varZ)    

    d4set = set([])
    d5set = set([])

    numVista = 0

    for v in vistas:
        for so in v.cuerpo:
            for a in constantes_query:
                for x in variables_query:
                    for y in so.orden:
                        for z in so.orden:
                            if y == z:
                                continue

                            if es_const(y):
                                continue

                            d4set.add((varsV[numVista],a,y,x,z))

        numVista += 1

    numVista = 0

    for v in vistas:
        for so in v.cuerpo:
            for y in variables_query:
                for z in variables_query:
                    for a in so.orden:
                        for x in so.orden:
                            if y == z:
                                continue

                            if es_var(a) or es_const(x):
                                continue

                            d5set.add((varsV[numVista],a,y,x,z))

        numVista += 1

    d4 = []

    for (vv,a,y,x,z) in d4set:
        vtay = varsT.get((int(a),int(y)))
        vtxy = varsT.get((int(x),int(y)))
        vtxz = varsT.get((int(x),int(z)))
        vtaz = varsT.get((int(a),int(z)))

        if vtaz and vtay and vtxy and vtxz:
            #add d4-type clause
            d4.append([vv.negarVar(), vtay.negarVar(), vtxy.negarVar(), vtxz.negarVar(), vtaz])

    d5 = []

    for (vv,a,y,x,z) in d5set:
        vtya = varsT.get((int(y),int(a)))
        vtyx = varsT.get((int(y),int(x)))
        vtzx = varsT.get((int(z),int(x)))
        vtza = varsT.get((int(z),int(a)))
        
        if vtza and vtya and vtyx and vtzx:
            #add d5-type clause
            d5.append([vv.negarVar(), vtya.negarVar(), vtyx.negarVar(), vtzx.negarVar(), vtza])

    return d4, d5, lt_d4d5

def clausulas_d6(vistas, variables_query, constantes_vistas):
    global varsT
    global varsV

    prohibidas = set([])
    d6 = []

    m = 0

    for vista in vistas:
        constantes_esta_vista = set([])

        for so in vista.cuerpo:
            for v in so.orden:
                if es_const(v):
                    constantes_esta_vista.add(int(v))

        for constante in constantes_vistas:
            if constante not in constantes_esta_vista:
                prohibidas.add((varsV[m], constante))

        m = m + 1

    for variable in variables_query:
        for (varV, constante) in prohibidas:
            varT = varsT.get((int(variable), constante))

            if varT:
                d6.append([varV.negarVar(), varT.negarVar()])

    return d6
