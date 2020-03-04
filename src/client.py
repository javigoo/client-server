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

import sys, os, traceback, optparse, time, random, struct
import socket, select

__program__ = "client"
__version__ = '0.0.1'
__author__ = 'Javier Roig <javierroiggregorio@gmail.com>'
__copyright__ = 'Copyright (c) 2020  Javier Roig'
__license__ = 'GPL3+'
__vcs_id__ = '$Id$'

# Client states
DISCONNECTED = 0xa0
NOT_REGISTERED = 0xa1
WAIT_ACK_REG = 0xa2
WAI_INFO = 0xa3
WAIT_ACK_INFO = 0xa4
REGISTERED = 0xa5
SEND_ALIVE =  0xa6

# Register packages
REG_REQ = 0x00
REG_INFO = 0x01
REG_ACK = 0x02
INFO_ACK = 0x03
REG_NACK = 0x04
INFO_NACK = 0x05
REG_REJ =  0x06

class pdu_udp():
    """Protocol Data Unit (PDU) for User Data Protocol (UDP)"""
    def __init__(self, pkg, id, rand, data):
        self.pkg = pkg
        self.id = id
        self.rand = rand
        self.data = data

    def __str__(self):
        return(' pkg = %s\n id = %s\n rand = %s\n data = %s' % (self.pkg, self.id, self.rand, self.data))

#################################### SETUP #####################################

def setup():
    global state, socketTCP, socketUDP, configuration, authorized

    state = DISCONNECTED
    debug("State = DISCONNECTED")

    socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    debug("UDP socket initialized")
    socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    debug("TCP socket initialized")

    configuration = read_configuration()
    debug("Configuration data file loaded - Device: {}".format(configuration.id))
    #print(configuration)
    authorized = read_authorized()
    debug("Authorized devices file loaded")
    #print(authorized)

def debug(msg):
    if debugger_opt: print(time.strftime("%H:%M:%S") + ": DEBUG -> " + str(msg))

def read_configuration():
    with open(configuration_opt) as file:
        id = file.readline().strip('Id =\n')
        elements = file.readline().strip('Params =\n').split(';')
        TCP = file.readline().strip('Local-TCP =\n')
        server = file.readline().strip('Server =\n')
        UDP = file.readline().strip('Server-UDP =\n')

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

################################### REGISTER ###################################

def register():
    debug("Register on the server initialized")
    # Timers and thresholds
    t = 1
    u = 2
    n = 7
    o = 3
    p = 3
    q = 3

    state = NOT_REGISTERED
    debug("State = NOT_REGISTERED")

    # Refactorizar esta basura pls
    pdu = struct.pack('1B 13s 9s 61s', REG_REQ, bytes(configuration.id, 'utf-8'), b'00000000', b'')
    addr = (configuration.server, int(configuration.UDP))
    sent = socketUDP.sendto(pdu, addr)
    debug("Send: bytes={}, Packet={}, id={}, rand={}, data={}".format(sent, "REG_REQ", configuration.id, "00000000", "" ))

    state = WAIT_ACK_REG
    debug("State = WAIT_ACK_REG")

    pdu, addr = socketUDP.recvfrom(struct.calcsize('1B 13s 9s 61s'))
    received = struct.unpack('1B 13s 9s 61s', pdu)
    data = packet_data_parser(received)
    debug("Received: bytes={}, Packet={}, id={}, rand={}, data={}".format(0,data[0],data[1],data[2],data[3]))

    if data[0] == REG_ACK:
        # Introducir datos pdu correctoss
        config_elem=""
        for element in configuration.elements:
            config_elem += element+';'
        dades = str(configuration.TCP)+','+config_elem[:-1]
        print(dades)
        pdu = struct.pack('1B 13s 9s 61s', REG_INFO, bytes(configuration.id, 'utf-8'), bytes(data[2], 'utf-8'), bytes(dades, 'utf-8'))
        addr = (configuration.server, int(configuration.UDP))
        sent = socketUDP.sendto(pdu, addr)
        debug("Send: bytes={}, Packet={}, id={}, rand={}, data={}".format(sent, "REG_INFO", configuration.id, data[2], dades))

def read_authorized():
    authorized=[]
    with open(authorized_opt) as file:
        for line in file:
            authorized += line.split()

    return authorized

    #current_t = time.time()
    #while current_t < t:
    #    pass

    debug("Register on the server finished")

# Hacer un parser de verdad
def packet_data_parser(packet):
    return packet[0], packet[1].decode(), packet[2].decode(), str(packet[3]).strip("b'")[:5]

########################### PERIODIC COMMUNICATION #############################

def periodic_communication():
    debug("Periodic communication with the server initialized")
    pass
    debug("Periodic communication with the server finished")

################################# SEND DATA ####################################

def send_data():
    debug("Send data to the server initialized")
    pass
    debug("Send data to the server finished")

############################## WAIT CONNECTIONS ################################

def wait_connections():
    debug("Wait for server tcp connections initialized")
    pass
    debug("Wait for server tcp connections finished")

#################################### MAIN ######################################

def main():
    setup()
    register()
    #periodic_communication()
    #send_data()
    #wait_connections()

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
        print ("Keyboard Interruption")
        raise
    except SystemExit: # sys.exit()
        print ("System Exit")
        raise
    except Exception:
        print ("Unexpected Exception")
        traceback.print_exc()
        os._exit(1)
