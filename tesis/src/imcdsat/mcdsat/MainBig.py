#!/bin/python
import pprint
import sys
import logging
from CQ import *
from CQ.Argumento import *
from CQ.Predicado import *
from CQ.SubObjetivo import *
from CQ.CQ import *
from CQ.SOComparacion import *
from Parser.CQparser import *
from Traductor.Traductor3 import *
from Traductor.GenerarReescrituras import *
from random import *
#from  import *

if __name__ == "__main__":
    #import psyco  # only needed for improved performance
    #psyco.full()  # only needed for improved performance
    logging.basicConfig(level=logging.INFO)
    op = sys.argv[1]
    exp = sys.argv[2]
    archVistas = sys.argv[3]
    archCons = sys.argv[4]
    archCostosSat = sys.argv[5]
    archVars = sys.argv[6]
    archTiempo = sys.argv[7]

    if op == 'T':
        archSalida = sys.argv[8]

        traducirBig(exp, archVistas, archCons, archVars, archTiempo, archSalida, archCostosSat)
    elif op == 'G':
        if len(sys.argv) > 7:
            archCostos = sys.argv[7]
        else:
            archCostos = None

        generarReescrituras(exp, archVistas, archCons, archVars, archTiempo, archCostos, sys.stdin)

