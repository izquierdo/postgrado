#!/usr/bin/env python

import os
import sys
from getopt import getopt


# the primal graph for a cnf formula has a vertex for every variable
# and an edge between any two variables that appear together in a
# clause
def primal_cnf(cnf):
    vset = set()
    eset = set()
    for clause in cnf:
        # get variable in clause
        vs = [abs(x) for x in clause]
        vs.sort()
        # update vertex set with variables
        vset.update(vs)
        # add an edge for each pairs of variables in the clause
        es = [(x,y) for i,x in enumerate(vs) for y in vs[i+1:] if x!=y]
        eset.update(es)
    return (vset,eset)

# note: vertices in elg files are 1-indexed
def cnf2elg(filepath, graphtype):

    # read in the graph
    basename = os.path.basename(filepath)
    if os.path.splitext(basename)[1].lower()!='.cnf':
        print 'Error: Input file %s not .CNF file' % basename
        return
    fin = open(filepath, 'r')
    data = [x for x in fin.read().split('\n') if x!='']
    fin.close()

    cdata = [x for x in data if x[0]=='c']
    pdata = [x for x in data if x[0]=='p']
    cnfdata = [x for x in data if x[0]!='c' and x[0]!='p']
    cnf = [[int(x) for x in clause.split() if x!='0' and x!='%'] for clause in cnfdata]

    if (graphtype=='primal'):
        (vset,eset) = primal_cnf(cnf)
    else:
        print 'Error: Invalid graph type %s' % graphtype
        return

    # open output file
    graphname = os.path.splitext(basename)[0]
    newfile = graphname + '.' + graphtype + '.elg'
    fout = open(newfile, 'w')

    # output graph data
    fout.write('%i %i\n' % (len(vset), len(eset)))
    for e in eset:
        fout.write('%i %i\n' % (e[0], e[1]))
    fout.close()

    print "Wrote %s" % newfile

def cnf2elg_dir(dirpath, graphtype):

    graphs = [g for g in os.listdir(dirpath) if os.path.splitext(g)[1]=='.cnf']
    if graphs==[]:
        print "Error: No .CNF files in directory %s" % dirpath
        return
    graphs.sort()
    for g in graphs:
        cnf2elg(os.path.join(dirpath,g),graphtype)

def usage():
    print "Usage:"
    print "  cnf2elg.py [-d] path type"
    print "Default: path = path of .CNF graph file"
    print "Directory mode [-d]: path = path of directory of .CNF files"
    print "Required: type = type of graph to generate from CNF formula, options:"
    print "  primal"
    print "  dual [NOT IMPLEMENTED YET]"


if __name__ == "__main__":
    try:
        opts, args = getopt(sys.argv[1:], "hd", ["help"])
    except:
        usage()
        sys.exit(2)

    dirflag = False

    for opt, arg in opts:
        if opt in ("-h","--help"):
            usage()
            sys.exit()
        elif opt == '-d':
            dirflag = True

    if len(args)<1:
        print "Missing required argument."
        usage()
        sys.exit(2)

    if dirflag:
        cnf2elg_dir(args[0], args[1])
    else:
        cnf2elg(args[0], args[1])
