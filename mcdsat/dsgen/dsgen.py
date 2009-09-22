#!/usr/bin/env python

import sys
import random

TEMPSUBGOALSNUMBERERASEME = 0
TEMPDISTINGUISHEDPERCENTAGEERASEME = 0

def sample_reps(population, k):
    assert(k > 0)

    result = [None] * k

    last = len(population)-1

    for i in xrange(k):
        result[i] = population[random.randint(0,last)]

    return result

def argstr(l):
    return ",".join(l)
    
def gen_query(sgs_num, varlist, predlist, dist_pc):
    sgs_predicates = sample_reps(predlist, sgs_num)
    sgs = [(name,sample_reps(varlist,arity)) for (name,arity) in sgs_predicates]
    sgs_strings = ["%s(%s)" % (name, ",".join(l)) for (name,l) in sgs]

    occurring_vars = []

    for (_,l) in sgs:
        for v in l:
            if v not in occurring_vars:
                #I know this is slow
                occurring_vars.append(v)

    dist_vars = random.sample(occurring_vars, int(len(occurring_vars)*(dist_pc/100.0)))

    return "q(%s) :- %s" % (argstr(dist_vars), argstr(sgs_strings))

def gen_view(view_name, vars_list, preds_list):
    global TEMPSUBGOALSNUMBERERASEME
    global TEMPDISTINGUISHEDPERCENTAGEERASEME
    sgs_predicates = sample_reps(preds_list, random.randint(1,TEMPSUBGOALSNUMBERERASEME*2))
    sgs = [(name,sample_reps(vars_list,arity)) for (name,arity) in sgs_predicates]
    sgs_strings = ["%s(%s)" % (name, argstr(l)) for (name,l) in sgs]

    occurring_vars = []

    for (_,l) in sgs:
        for v in l:
            if v not in occurring_vars:
                #I know this is slow
                occurring_vars.append(v)

    dist_vars = random.sample(occurring_vars, int(len(occurring_vars)*(TEMPDISTINGUISHEDPERCENTAGEERASEME/100.0)))

    return "%s(%s) :- %s" % (view_name, argstr(dist_vars), argstr(sgs_strings))

def gen_views(views_num, subgoals, varlist, preds_list):
    return [gen_view("v%d" % i, varlist, preds_list) for i in xrange(views_num)]

def main(views_out, query_out, views, subgoals, variables, predicates, distinguished):
    global TEMPSUBGOALSNUMBERERASEME
    global TEMPDISTINGUISHEDPERCENTAGEERASEME
    sgs_num = int(subgoals)
    vars_num = int(variables)
    views_num = int(views)
    dist_pc = float(distinguished)

    TEMPSUBGOALSNUMBERERASEME = sgs_num
    TEMPDISTINGUISHEDPERCENTAGEERASEME = dist_pc

    vars_list = ["X%d" % (i) for i in range(vars_num)]
    predlist = [("p%d" % (i), random.randint(1,vars_num)) for i in range(int(predicates))]

    vs = gen_views(views_num, sgs_num, vars_list, predlist)
    q = gen_query(sgs_num, vars_list, predlist, dist_pc)

    views_file = open(views_out, "w")
    query_file = open(query_out, "w")

    views_file.writelines(["%s\n" % v for v in vs])
    query_file.writelines(["%s\n" % q])

def usage():
    print "Usage: %s <views_out> <query_out> <views> <subgoals> <variables> <predicates> <distinguished>" % (sys.argv[0])

if __name__ == '__main__':
    #TODO necesario que views,subgoals,variables,predicates sean ints >= 1, y distinguished porcentaje
    if len(sys.argv) != 8:
        usage()
        sys.exit(1)

    random.seed()
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7])
