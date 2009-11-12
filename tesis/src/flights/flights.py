#!/usr/bin/env python

import sys

class Argument:
    def __init__(self, type, name):
        """
        type may be 'var' or 'const'
        name should be a number
        """

        self.type = type
        self.name = str(name)

    def __str__(self):
        if self.type == 'var':
            return 'X' + self.name

        if self.type == 'const':
            return self.name

class Table:
    def __init__(self, name, args):
        self.name = name
        self.args = args

    def __str__(self):
        arglist = ",".join([str(arg) for arg in self.args])

        return "%s(%s)" % (self.name, arglist)

class Query:
    def __init__(self, qt, sos):
        self.qt = qt
        self.sos = sos

    def __str__(self):
        solist = ",".join([str(so) for so in self.sos])

        return "%s :- %s" % (str(self.qt), solist)

def samecompany(argv):
    if len(argv) != 2:
        usage_and_quit()

    flights_pred = "r0"

    companies = int(argv[0])
    steps = int(argv[1])

    # generate query
    company_var = Argument('var', steps)
    query_pred = "q"
    query_args = [Argument('var', i) for i in range(steps)] + [company_var]
    query_table = Table(query_pred, query_args)
    cityA, cityB = (Argument('const', 1), Argument('const', 2))
    query_sos = [Table(flights_pred, [cityA, Argument('var', 1), Argument('const', 0)])]

    for i in range(1, steps-1):
        query_sos.append(Table(flights_pred, [Argument('var', i), Argument('var', i+1), Argument('const', 0)]))

    query_sos.append(Table(flights_pred, [Argument('var', steps-1), cityB, Argument('const', 0)]))

    query = Query(query_table, query_sos)

    # generate views
    view_pred_prefix = 'v'

    def gen_view(i):
        return 
    views = []

    return query, views

def main():
    if len(sys.argv) < 4:
        usage_and_quit()

    commands = {}
    commands['samecompany'] = samecompany

    viewsfilename = sys.argv[1]
    queryfilename = sys.argv[2]

    command = sys.argv[3]

    generator = commands.get(command)

    if generator is None:
        usage_and_quit()
    
    query, views = generator(sys.argv[4:])

    viewsfile = open(viewsfilename, 'w')
    viewsfile.writelines([str(v) + '\n' for v in views])
    viewsfile.close()

    queryfile = open(queryfilename, 'w')
    queryfile.write(str(query) + '\n')
    queryfile.close()


def usage_and_quit():
    print "Uso: ./buscawiki <archivo> (simple <regex> | relevancia <regex> | grafo <regex>)"
    sys.exit(1)

if __name__ == "__main__":
    main()
