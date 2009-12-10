from qrp_structures import *

def sameairline(argv):
    airlines = int(argv[0])
    steps = int(argv[1])
    return nsteps(airlines, steps, 'sameairline')

def specificairline(argv):
    airlines = int(argv[0])
    steps = int(argv[1])
    return nsteps(airlines, steps, 'specificairline')

def sameairline_manyviews(argv):
    airlines = int(argv[0])
    steps = int(argv[1])
    views_per_airline = int(argv[2])
    return nsteps(airlines, steps, views_per_airline)

def nsteps(airlines, steps, views_per_airline=1):
    flights_pred = "r1"

    # generate query
    company_var = Argument('var', steps)
    query_pred = "q"
    query_args = [Argument('var', i) for i in range(1, steps)]
    query_table = Table(query_pred, query_args)
    cityA, cityB = (Argument('const', 1), Argument('const', 2))
    query_sos = [Table(flights_pred, [cityA, Argument('var', 1), company_var])]

    for i in range(1, steps-1):
        query_sos.append(Table(flights_pred, [Argument('var', i), Argument('var', i+1), company_var]))

    query_sos.append(Table(flights_pred, [Argument('var', steps-1), cityB, company_var]))

    query = Query(query_table, query_sos)

    # generate views
    view_pred_prefix = 'v'

    def gen_view(num, a):
        varS = Argument('var', 1)
        varT = Argument('var', 2)
        constAirline = Argument('const', a)

        view_table = Table(view_pred_prefix + str(num), [varS, varT])
        view_sos = [Table(flights_pred, [varS, varT, constAirline])]
        view = Query(view_table, view_sos)

        return view

    views_base = 100
    views = []

    for n in range(views_per_airline):
        views += [gen_view(n*airlines+i, i) for i in range(views_base, views_base+airlines)]

    return query, views
