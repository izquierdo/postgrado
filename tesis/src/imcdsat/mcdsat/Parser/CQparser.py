 #!/usr/bin/env python

import logging
import tpg
from CQ.CQ import *
from CQ.SubObjetivo import *

class CQparser(tpg.Parser):
    r"""

        separator space '\s+';

        token var      '[A-Z][A-Za-z0-9]*';
        token ctte       '[0-9_]+';
        token nomb      '[a-z][a-z0-9]*';
        token imp       ':[\-]';

        START/e -> EXPERIMENTO/e ;

        EXPERIMENTO/l ->  
                            $ l = []
            CQ/a            $ l.append(a)  
            ( CQ/a      $ l.append(a)  
            )*  
        ;

        CQ/e -> SO/h
                imp
                CUERPO/c    $ e = crearCQ(h, c)
                ;
        
        CUERPO/l ->  
                            $ l = []
            SO/a            $ l.append(a)  
            ( ',' SO/a      $ l.append(a)  
            )*  
        ;

        SO/p -> nomb/n      
                LIST/l      $ p = crearSO(n, l)
                ;

        LIST/l ->  
        '\('                
                            $ l = [] 
            ITEM/a          $ l.append(a)  
            ( ',' ITEM/a    $ l.append(a)  
            )*  
        '\)'  
        ;

        ITEM/a ->
                    var/v   $ a = (v[1:],0)
                |   ctte/c  $ a = (c,1)
                ;

    """

cq_actual = 0
renameArguments = True

def getConstantName(n):
    global renameArguments

    if renameArguments:
        return str(int(n)+1000000000)

    return n

def getVariableName(x):
    global renameArguments

    if renameArguments:
        return str(VAR_BASE + (10000*cq_actual) + int(x))

    return x

def getRealConstantName(n):
    global renameArguments

    if renameArguments:
        return str(int(n)-1000000000)

    return n

def getRealVariableName(x):
    global renameArguments

    if renameArguments:
        return 'X'+str(int(x) % 10000)

    return x

def crearSO(pred, lista):
    ord = []
    arg = {}
    for (x,y) in lista:
        if y == 1:
            nx = getConstantName(x)
        else:
            nx = getVariableName(x)

        ord.append(nx)
        arg[nx]=y
    return SubObjetivo(pred, arg, ord)

# falta colocarle los predicados de orden
def crearCQ(cabeza, cuerpo):
    global cq_actual
    cq_actual += 1

    return CQ(cabeza, cuerpo, [])

def cargarCQ(nomArch,rename=True):
    global renameArguments
    global cq_actual

    cq_actual = 0

    logging.debug("cargarCQ(nomArch=%s,rename=%s)" % (nomArch, rename))
    renameArguments = rename
    parser = CQparser()
    in_file = open(nomArch,"r")
    text = in_file.read()
    result = parser(text)
    in_file.close()
    return result

##
##while 1:
##        expr = raw_input('cq: ')
##        if not expr: break
##        #try:
##        print expr
##        result = parser(expr)
##        print expr, "=", result
##        #except Exception, e:
##        #        print expr, ":", e
##        print

