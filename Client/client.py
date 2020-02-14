#!/usr/bin/env python3.7
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :

"""
SYNOPSIS

    client.py [-h,--help] [--version] [-d, --debug]
        [c, --configuration <file>, default=client.cfg]
        [u, --authorized <file>, default=bbdd_dev.dat]


DESCRIPTION

    Creates a client, register with the server, maintain periodic
    communication with the server, send data to the server and wait
    for TCP connections from the server to receive or send information.
    Default configuration data file client.cfg and default authorized
    device file bbdd_dev.dat


EXAMPLES

    client.py  --configuration client1.cfg

AUTHOR

    Javier Roig <javierroiggregorio@gmail.com>

LICENSE

    This script is published under the Gnu Public License GPL3+

VERSION

    0.0.1
"""

import sys, os, traceback, optparse
import time, datetime
import socket, select

__program__ = "client"
__version__ = '0.0.1'
__author__ = 'Javier Roig <javierroiggregorio@gmail.com>'
__copyright__ = 'Copyright (c) 2020  Javier Roig'
__license__ = 'GPL3+'
__vcs_id__ = '$Id$'


def setup():
    global socketTCP, socketUDP
    socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    debug("UDP socket initialized")
    socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    debug("TCP socket initialized")

    #configuration_data_file = read_configuration_data_file(configuration)
    #authorized_devices_file = read_authorized_devices_file(authorized)

def debug(msg):
    if dbg == True:
        print(time.strftime("%H:%M:%S") + ": DEBUG -> " + str(msg))

def main():
    setup()

if __name__ == '__main__':
    try:
        global dbg, configuration, authorized
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()["__doc__"],version=__version__)
        parser.add_option('-d', '--debug', action = 'store_true', default = False, help = 'Show information for each significant event')
        parser.add_option ('-c', '--configuration', action='store', type='string', default='client.cfg', help='Specify configuration data file, default client.cfg')
        parser.add_option ('-u', '--authorized', action='store', type='string', default='bbdd_dev.dat', help='Specify authorized devices file, default bbdd_dev.dat')
        (options, args) = parser.parse_args()
        if len(args) > 0: parser.error ('Bad arguments, use --help for help')

        dbg = options.debug
        configuration = options.configuration
        authorized = options.authorized

        main()

    except KeyboardInterrupt: # Ctrl-C
        print ('Keyboard Interruption')
        raise
    except SystemExit: # sys.exit()
        print ('System Exit')
        raise
    except Exception:
        print ('Unexpected Exception')
        traceback.print_exc()
        os._exit(1)
