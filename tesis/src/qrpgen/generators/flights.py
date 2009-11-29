from qrp_structures import *

def samecompany(argv):
    companies = int(argv[0])
    steps = int(argv[1])

def specificcompany(argv):
    companies = int(argv[0])
    steps = int(argv[1])

def samecompany(argv):
    if len(argv) != 2:
        #TODO
        sys.exit(1)

    flights_pred = "r0"

    companies = int(argv[0])
    steps = int(argv[1])

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

    def gen_view(i):
        varS = Argument('var', 1)
        varT = Argument('var', 2)
        constCompany = Argument('const', i)

        view_table = Table(view_pred_prefix + str(i), [varS, varT])
        view_sos = [Table(flights_pred, [varS, varT, constCompany])]
        view = Query(view_table, view_sos)

        return view

    views = [gen_view(i) for i in range(100, 100+companies)]

    return query, views
