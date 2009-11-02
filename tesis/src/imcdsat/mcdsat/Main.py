#!/bin/python
import pprint
import sys
from CQ import *
from CQ.Argumento import *
from CQ.Predicado import *
from CQ.SubObjetivo import *
from CQ.CQ import *
from CQ.SOComparacion import *
from Parser.CQparser import *
from Traductor.Traductor3 import *
from Traductor.GenerarReescrituras import *
from sets import Set
from random import *
#from  import *




if __name__ == "__main__":
    #import psyco  # only needed for improved performance
    #psyco.full()  # only needed for improved performance
    op = sys.argv[1]
    exp = sys.argv[2]
    archVistas = sys.argv[3]
    archCons = sys.argv[4]
    archVars = sys.argv[5]
    archTiempo = sys.argv[6]
    if op == 'T':
        archSalida = sys.argv[7]
        traducir(exp, archVistas, archCons, archVars, archTiempo, archSalida)
    elif op == 'G':
        generarReescrituras(exp, archVistas, archCons, archVars, archTiempo, sys.stdin)

