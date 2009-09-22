#!/usr/bin/env python

import sys
import random

def sample_reps(pop, n):
    assert(n > 0)
    ret = []

    while n > 0:
        n = n - 1

def query(subgoals, varlist):
    print "q(%s) :- " % (",".join(varlist))

def view(views, subgoals, varlist):
    print "view"

def main(views, subgoals, variables, predicates, distinguished):
    varnum = int(variables)

    varlist = ["X%d" % (i) for i in range(varnum)]
    predlist = [("p%d" % (i), random.randint(1,varnum)) for i in range(int(predicates))]

    print varlist
    print predlist

    query(subgoals, varlist)
    view(views, subgoals, variables)

def usage():
    print "Usage: %s <views> <subgoals> <variables> <predicates> <distinguished>" % (sys.argv[0])

if __name__ == '__main__':
    #TODO necesario que views,subgoals,variables,predicates sean ints >= 1, y distinguished porcentaje
    if len(sys.argv) != 6:
        usage()
        sys.exit(1)

    random.seed()
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
