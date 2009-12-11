#!/usr/bin/env python

import sys
import generators.flights
import generators.structured

commands = {}
commands['flights.sameairline'] = generators.flights.sameairline
commands['flights.sameairline_manyviews'] = generators.flights.sameairline_manyviews
commands['structured.chainquery'] = generators.structured.chainquery
commands['randomviews'] = generators.structured.randomviews

def writelist(l, filename):
    f = open(filename, 'w')
    f.writelines([str(e) + '\n' for e in l])
    f.close()

def main():
    if len(sys.argv) < 4:
        usage_and_quit()

    viewsfilename = sys.argv[1]
    queryfilename = sys.argv[2]

    command = sys.argv[3]

    generator = commands.get(command)

    if generator is None:
        print_usage()
        sys.exit(1)
    
    query, views = generator(sys.argv[4:])

    writelist(views, viewsfilename)
    writelist([query], queryfilename)

def print_usage():
    print "Usage: %s <views_file> <query_file> <command> [command_arguments...]" % (sys.argv[0])
    print
    print "Available commands:"

    for c in commands.keys():
        print "  %s" % (c)

if __name__ == "__main__":
    main()
