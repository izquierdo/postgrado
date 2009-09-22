#!/usr/bin/env python

import sys
import random

def sample_reps(population, k):
    assert(k > 0)

    result = [None] * k

    last = len(population)-1

    for i in xrange(k):
        result[i] = population[random.randint(0,last)]

    return result

def gen_query(sgs_num, varlist, predlist, dist_pc):
    sgs_predicates = sample_reps(predlist, sgs_num)
    sgs = [(name,sample_reps(varlist,arity)) for (name,arity) in sgs_predicates]
    sgs_strings = ["%s(%s)" % (name,",".join(list)) for (name,list) in sgs]

    occurring_vars = []

    for (_,l) in sgs:
        for v in l:
            if v not in occurring_vars:
                #I know this is slow
                occurring_vars.append(v)

    dist_vars = random.sample(occurring_vars, int(len(occurring_vars)*(dist_pc/100.0)))

    return "q(%s) :- %s" % (",".join(dist_vars), ",".join(sgs_strings))

def gen_view(view_name):
    pass

def gen_views(views_num, subgoals, varlist):
    return [gen_view(i) for i in xrange(views_num)]

def main(views, subgoals, variables, predicates, distinguished):
    sgs_num = int(subgoals)
    vars_num = int(variables)
    views_num = int(views)
    dist_pc = float(distinguished)

    varlist = ["X%d" % (i) for i in range(vars_num)]
    predlist = [("p%d" % (i), random.randint(1,vars_num)) for i in range(int(predicates))]

    q = gen_query(sgs_num, varlist, predlist, dist_pc)
    vs = gen_views(views_num, sgs_num, variables)

def usage():
    print "Usage: %s <views> <subgoals> <variables> <predicates> <distinguished>" % (sys.argv[0])

if __name__ == '__main__':
    #TODO necesario que views,subgoals,variables,predicates sean ints >= 1, y distinguished porcentaje
    if len(sys.argv) != 6:
        usage()
        sys.exit(1)

    random.seed()
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
