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
    configuration = read_configuration()
    debug("Configuration data file initialized")
    print(configuration)
    authorized = read_authorized()
    debug("Authorized devices file initialized")
    #print(authorized)

def debug(msg):
    if debugger_opt: print(time.strftime("%H:%M:%S") + ": DEBUG -> " + str(msg))

def read_configuration():
    with open(configuration_opt) as file:
        id = file.readline().strip('Id=,\n')
        elements = file.readline().strip('Elements=,\n').split(';')
        TCP = file.readline().strip('Local-TCP=,\n')
        server = file.readline().strip('Server=,\n')
        UDP = file.readline().strip('Server-UDP=,\n')

    return configuration_data(id, elements, TCP, server, UDP)

class configuration_data:
    def __init__(self, id, elements, TCP, server, UDP):
        self.id = id
        self.elements = elements
        self.TCP = TCP
        self.server = server
        self.UDP = UDP

    def __str__(self):
        return('Id = %s\nElements = %s\nTCP = %s\nServer = %s\nUDP = %s' % (self.id, self.elements, self.TCP, self.server, self.UDP))

def read_authorized():
    authorized=[]
    with open(authorized_opt) as file:
        for line in file:
            authorized += line.split()

    return authorized

def register():
    debug("Register on the server initialized")
    pass
    debug("Register on the server finished")

def periodic_communication():
    debug("Periodic communication with the server initialized")
    pass
    debug("Periodic communication with the server finished")

def send_data():
    debug("Send data to the server initialized")
    pass
    debug("Send data to the server finished")

def wait_connections():
    debug("Wait for server tcp connections initialized")
    pass
    debug("Wait for server tcp connections finished")

def main():
    # pdu_udp & pdu_tcp
    setup()
    register()
    periodic_communication()
    send_data()
    wait_connections()

if __name__ == '__main__':
    try:
        # Global variables
        global debugger_opt, configuration_opt, authorized_opt

        # Parser options
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()["__doc__"],version=__version__)
        parser.add_option('-d', '--debug', action = 'store_true', default = False, help = 'Show information for each significant event')
        parser.add_option ('-c', '--configuration', action='store', type='string', default='client.cfg', help='Specify configuration data file, default client.cfg')
        parser.add_option ('-u', '--authorized', action='store', type='string', default='bbdd_dev.dat', help='Specify authorized devices file, default bbdd_dev.dat')
        (options, args) = parser.parse_args()
        if len(args) > 0: parser.error ('Bad arguments, use --help for help')

        debugger_opt = options.debug
        configuration_opt = options.configuration
        authorized_opt = options.authorized

        main()

        # Exception handling
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
