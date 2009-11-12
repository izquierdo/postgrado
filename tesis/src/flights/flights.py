#!/usr/bin/env python

import sys
import generators.flights

def main():
    if len(sys.argv) < 4:
        usage_and_quit()

    commands = {}
    commands['samecompany'] = generators.flights.samecompany

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
