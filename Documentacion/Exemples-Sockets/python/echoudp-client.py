#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :

"""
SYNOPSIS

    echoudp-client.py [-h,--help] [--version] [p, --port <port>, default=1234]
        [-d, --destination <address>, default=127.0.0.1]

DESCRIPTION

    Creates a ECHO/UDP client, connects to server, sends data and
    prints the returned data.
    Default port 1234, default address=127.0.0.1


EXAMPLES

    echoudp-client.py  --port 4321

AUTHOR

    Carles Mateu <carlesm@carlesm.com>

LICENSE

    This script is published under the Gnu Public License GPL3+

VERSION

    0.0.1
"""

import sys, os, traceback, optparse
import time, datetime
import socket


__program__ = "echoudp-client"
__version__ = '0.0.1'
__author__ = 'Carles Mateu <carlesm@carlesm.com>'
__copyright__ = 'Copyright (c) 2012  Carles Mateu '
__license__ = 'GPL3+'
__vcs_id__ = '$Id: echoudp-client.py 554 2012-05-06 08:07:51Z carlesm $'


dades = """ Un paragraf per provar
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas ut facilisis
odio. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per
inceptos himenaeos. Integer eget porta diam. Mauris interdum euismod lacus id
vulputate. Etiam eleifend condimentum erat, nec auctor sem adipiscing ut.
Integer id nunc vel elit ullamcorper."""

def setup():
    global sock, options
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def mainloop():
    global sock, options
    for line in dades.splitlines():
        sock.sendto(line,(options.destination, options.port))
        print >>sys.stderr,"ENVIAT->",line
        resposta = sock.recv(8192)
        print >>sys.stderr,"REBUT ->",resposta
    sock.close()

def main():
    global options, args
    setup()
    mainloop()

if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()["__doc__"],version=__version__)
        parser.add_option ('-v', '--verbose', action='store_true', default=False, help='verbose output')
        parser.add_option ('-p', '--port', action='store', type='int', default=1234, help='Listening port, default 1234')
        parser.add_option ('-d', '--destination', action='store', default="127.0.0.1", help='Listening port, default 1234')
        (options, args) = parser.parse_args()
        if len(args) > 0: parser.error ('bad args, use --help for help')

        if options.verbose: print time.asctime()

        main()

        now_time = time.time()
        if options.verbose: print time.asctime()
        if options.verbose: print 'TOTAL TIME:', (now_time - start_time), "(seconds)"
        if options.verbose: print '          :', datetime.timedelta(seconds=(now_time - start_time))
        sys.exit(0)
    except KeyboardInterrupt, e: # Ctrl-C
        raise e
    except SystemExit, e: # sys.exit()
        raise e
    except Exception, e:
        print 'ERROR, UNEXPECTED EXCEPTION'
        print str(e)
        traceback.print_exc()
        os._exit(1)
