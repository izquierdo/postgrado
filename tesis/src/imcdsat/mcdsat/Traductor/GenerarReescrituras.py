import sys
import timeit
import os
import resource
import pprint
import random
import cPickle  
import pickle  
import logging

from Unification import unificar
from Parser import *
from Parser.CQparser import *
from CQ.Argumento import *
from CQ.Predicado import *
from CQ.SubObjetivo import *
from CQ.CQ import *
from CQ.SOComparacion import *
from TransformarFormula import *
from VariableSat import *
from MCD import *

archVistas = ''
archCons = ''

 
def generarReescrituras(exp, archV, archC, archVars, archTiempo, stdin):
    tiempoi = resource.getrusage(resource.RUSAGE_SELF)[0]
    generarReescrituras1(exp, archV, archC, archVars, stdin)
    tiempof = resource.getrusage(resource.RUSAGE_SELF)[0]
    
    fileobject = open(archTiempo, 'w')
    fileobject.write(str(tiempof-tiempoi))
    fileobject.close()

def generarReescrituras1(exp, archVistas, archCons, archVars, stdin):
    archVistas2 = archVistas.replace('.txt', '')
    print "archVistas2: %s" % (archVistas2,)
    vistas = cargarCQ(archVistas,rename=False)
    print "V: %s" % (vistas,)
    #print archMod
    consultas = cargarCQ(archCons,rename=False)
    print "Q: %s" % (consultas,)
    for q in consultas:
        numeros = leerVars(archVars)
        lenQuery = len(q.cuerpo)
        if exp == 'Sat':
            generarReescMCD(numeros, stdin, vistas)
        if exp == 'SatRW':
            generarReescRW(numeros, stdin, lenQuery,q, vistas)
##        llamarzchaff(transf, archSalida)

def leerVars(archVars):
    fh = open(archVars,'r')
    numeros = pickle.load(fh)
    fh.close()
    return numeros 


def generarReescMCD_old(numeros, archMods, vistas):
    modelos = leerModelos(numeros, archMods)
    nmod = len(modelos)
    print "Modelos = ",nmod
    if nmod > 1:
        for mod in modelos:
            mcd = crearMCD(mod, vistas)
            if mcd != None:
                #print mod
                print mcd
    else:
        print "No hay MCDs"


def generarReescRWold(numeros, archMods, lenQuery, query, vistas):
    modelos = leerModelosRW(numeros, archMods, lenQuery, query, vistas)
    mcdsc = []
    for mod in modelos:
        mcds = []
        for mc in mod:
            mcd = crearMCD(mc, vistas)
            if mcd != None:
                #print mcd
                mcds.append(mcd)
        #print 
        mcdsc.append(mcds)
    rs = crearReescritura(mcdsc, query)
    pprint.pprint(rs)

def crearReescritura(mcdc, query):
    r = []
    seq = Seq()
    ecgeneral = unificar([k.ec for k in mcdc], [nombreReal(var) for var in query.variables()])
    #print "ecgeneral", ecgeneral
    for m in mcdc:
        unif = m.obtUnificacion(ecgeneral)
        vis = m.vistaH.map_variables2(unif, seq)
        r.append(vis)

    cue = [v.cabeza for v in r]
    cab = query.cabeza.map_variables3(ecgeneral)
    res = CQ(cab, cue, [])
    return res    

def nombreReal(var):
    if es_const(var):
        return str(int(var)-CONSTANT_BASE)

    #VARRENAME
    return 'X'+str(int(var)%10000)

def crearMCD(mod, vistas):
    phictodo = {}
    gc=set()
    for var in mod:
        if var[0] == 'v':
            numVista = var[2:len(var)-1]
            if numVista[0] == '-':
                return None
            else:
                vista = vistas[int(numVista)]
        elif var[0] == 't':
            i = var.find(',')
            vq = nombreReal(var[2:i])
            vv = nombreReal(var[i+2:len(var)-1])

            phictodo.setdefault(vq,[]).append(vv)
        elif var[0] == 'g':
            gc.add(var[2:len(var)-1])
    return MCD(phictodo, gc, vista)


def generarReescRW(numeros, stdin, lenQuery, query, vistas):
    infile = stdin
    modelos = []
    n = len(numeros)
    for x in infile:
        l = x.strip().split()
        if l[0] != 'main:':
            print "MODELORW: %s QUERY: %s" % (obtModeloRW(numeros, n, l, lenQuery, vistas), query)
            print crearReescritura(obtModeloRW(numeros, n, l, lenQuery, vistas), query)
    return modelos


def obtModeloRW(numeros, n, lista, lenQuery, vistas):
    modp = []
    mod = []
    initeo = 0
    for var in lista:
        var2 = int(var)
        if var2 > (initeo + n):
            mcd = crearMCD(mod, vistas)
            if mcd != None:
                modp.append(mcd)
            mod = []
            initeo=initeo + n
        mod.append(numeros[var2-initeo])
    mcd = crearMCD(mod, vistas)
    if mcd != None:
        modp.append(mcd)
    return modp


def generarReescMCD(numeros, stdin, vistas):
    infile = stdin
    modelos = []
    count = 0
    x = infile.readline()
    x = infile.readline()
    while 1:
        if not(x):
            break
        x = infile.readline()
        l = x.strip().split()
        n = len(l)
        if n > 1: 
            mod = obtModelo(numeros, l, n)
            mcd = crearMCD(mod, vistas)
            if mcd != None:
                #print mod
                print mcd


def obtModelo(numeros, lista, n):
    mod = []
    for x in xrange(n):
        var = lista[x]
        mod.append(numeros[int(var)])
    return mod
