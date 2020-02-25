#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :

"""
SYNOPSIS

    echotcp-selectserver.py [-h,--help] [--version] [-p,--port <port>, default=1234]

DESCRIPTION

    Creates a ECHO/TCP server, resending all received data back to sender.
    Uses select to process multiple clients
    Default port 1234


EXAMPLES

    echotcp-selectserver.py  --port 4321

AUTHOR

    Carles Mateu <carlesm@carlesm.com>

LICENSE

    This script is published under the Gnu Public License GPL3+

VERSION

    0.0.1 
"""

import sys, os, traceback, optparse
import time, datetime
import socket, select
 

__program__ = "echotcp-selectserver"
__version__ = '0.0.1'
__author__ = 'Carles Mateu <carlesm@carlesm.com>'
__copyright__ = 'Copyright (c) 2012  Carles Mateu '
__license__ = 'GPL3+'
__vcs_id__ = '$Id: echotcp-selectserver.py 557 2012-05-06 09:38:18Z carlesm $'


# Llistes pel select
# Sockets de lectura
insocks=[]
# Sockets d'escriptura
outsocks=[]


#
# Data Buffer (maps databuffers -> sockets)
data = {}

#
# socket->host/port mapping
adrs = {}



def setup():
    global sock, options
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Afegim sock, el socket de servidor, a la llista de lectura
    insocks.append(sock)
    sock.bind(("",options.port))
    sock.listen(5)

def mainloop():
    global sock, options
    try:
        while True:
            i,o,e = select.select(insocks,outsocks,[])
            for x in i:
                if x is sock:
                    #   sock es el de servidor, per tant, nova Connexio
                    newsocket, address = sock.accept()
                    print >> sys.stderr, "Connexio de ", address
                    insocks.append(newsocket)
                    adrs[newsocket]=address
                else:
                    # altra lectura -> dades o desconnexio
                    newdata = x.recv(8192)
                    if newdata:
                        # Noves dades
                        print >> sys.stderr,"%d from %s" % (len(newdata),adrs[x])
                        data[x]=data.get(x,'')+newdata
                        if x not in outsocks:
                            outsocks.append(x)
                    else:
                        # Desconnexio
                        print >> sys.stderr,"desconn: ",adrs[x]
                        del adrs[x]
                        try:
                            outsocks.remove(x)
                        except ValueError:
                            pass
                        x.close()
                        insocks.remove(x)
            for x in o:
                aenviar=data.get(x)
                if aenviar:
                    nsnt = x.send(aenviar)
                    print >>sys.stderr,"%d to -> %s" % (nsnt,adrs[x])
                    aenviar=aenviar[nsnt:]
                if aenviar:
                    data[x]=aenviar
                else:
                    try:
                        del data[x]
                    except KeyError:
                        pass
                    outsocks.remove(x)
    finally:
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




