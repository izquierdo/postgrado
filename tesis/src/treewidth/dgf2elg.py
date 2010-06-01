#!/usr/bin/env python

import os
import sys
from getopt import getopt

# note: vertices in elg files are 1-indexed
def dgf2elg(filepath):
    """Convert the DGF graph into an ELG graph."""

    # read in the graph
    basename = os.path.basename(filepath)
    if os.path.splitext(basename)[1].lower()!='.dgf' \
            and os.path.splitext(basename)[1].lower()!='.dimacs':
        print 'Error: Input file %s not .CNF file' % basename
        return
    fin = open(filepath, 'r')
    data = [x for x in fin.read().split('\n') if x!='']
    fin.close()
    edata = [x.split() for x in data if x[0]=='e']
    edges = [(int(x[1]), int(x[2])) for x in edata]
    vertices = set()
    vertices.update([x[0] for x in edges])
    vertices.update([x[1] for x in edges])

    # open output file
    graphname = os.path.splitext(basename)[0]
    newfile = graphname + '.elg'
    fout = open(newfile, 'w')

    # output graph data
    fout.write('%i %i\n' % (len(vertices), len(edges)))
    for edge in edges:
        fout.write('%i %i\n' % (edge[0], edge[1]))
    fout.close();

    print "Wrote %s" % newfile

def dgf2elg_dir(dirpath):
    """Convert all DGF graphs in the given directory into ELG graphs."""
    graphs = [g for g in os.listdir(dirpath) if os.path.splitext(g)[1]=='.dgf' \
                  or os.path.splitext(g)[1]=='.dimacs']
    if graphs==[]:
        print "Error: No .DGF files in directory %s" % dirpath
        return
    graphs.sort()
    for g in graphs:
        dgf2elg(os.path.join(dirpath,g))


def usage():
    print "Usage:"
    print "  dgf2elg.py [-d] path"
    print "Default: path = path of .DGF graph file"
    print "Directory mode [-d]: path = path of directory of .DGF files"

if __name__ == "__main__":
    try:
        opts, args = getopt(sys.argv[1:], "hd", ["help"])
    except:
        usage()
        sys.exit(2)

    dirflag = False
    componentflag = False

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
        dgf2elg_dir(args[0])
    else:
        dgf2elg(args[0])
