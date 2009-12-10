#!/usr/bin/env python

import sys
import generators.flights

def writelist(l, filename):
    f = open(filename, 'w')
    f.writelines([str(e) + '\n' for e in l])
    f.close()

def main():
    if len(sys.argv) < 4:
        usage_and_quit()

    commands = {}
    commands['sameairline'] = generators.flights.sameairline
    commands['sameairline_manyviews'] = generators.flights.sameairline_manyviews

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

if __name__ == "__main__":
    main()
