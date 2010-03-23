import random

from qrp_structures import *

def chainquery(argv):
    #TODO
    return [], []

def randomviews(argv):
    nviews = int(argv[0])
    nview_sos = int(argv[1])

    nquery_sos = int(argv[2])

    n_preds = int(argv[3])
    n_vars = int(argv[4])

    n_varsperpred = int(argv[5])

    perc_constants = float(argv[6])/100.0

    def rand_constvar(i):
        r = random.random()

        if r < perc_constants:
            return Argument('const', i)

        return Argument('var', i)

    predicates = ["r%d" % i for i in xrange(1, n_preds+1)]

    query_table = Table("q", [Argument('var', i+1) for i in xrange(n_vars)])
    query_sos = [Table(pred, [rand_constvar(i+1) for i in random.sample(range(n_vars)*n_vars, n_varsperpred)]) for pred in random.sample(predicates * nquery_sos, nquery_sos)]
    query = Query(query_table, query_sos)

    views = []

    for n in xrange(1, nviews+1):
        view_table = Table("v%s" % n, [Argument('var', i+1) for i in xrange(n_vars)])
        view_sos = [Table(pred, [rand_constvar(i+1) for i in random.sample(range(n_vars)*n_vars, n_varsperpred)]) for pred in random.sample(predicates * n_preds, nview_sos)]
        view = Query(view_table, view_sos)
        views.append(view)

    return query, views
