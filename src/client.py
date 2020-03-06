#!/usr/bin/env python3.7
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :

"""
SYNOPSIS

    client.py [-h,--help] [--version] [-d, --debug]
        [c, --configuration <file>, default=client.cfg]


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

def debug(msg):
    if debugger_opt: print(time.strftime("%H:%M:%S") + ": DEBUG -> " + str(msg))

def msg(msg):
    print(time.strftime("%H:%M:%S") + ": MSG   -> " + str(msg))

def send_package_udp(pkg, id, rand, data):
    package_content = struct.pack(udp_pdu, pkg, id, rand, data)
    return socketUDP.sendto(package_content, udp_addr)

def receive_package_udp():
    package_content, addr = socketUDP.recvfrom(struct.calcsize(udp_pdu))
    received = struct.unpack(udp_pdu, package_content)
    return received_data(received)

class received_data:
    def __init__(self, received):
        self.pkg = received[0]
        self.id = received[1].decode()
        self.rand = received[2].decode()
        self.data = str(received[3]).strip("b'")[:5] # Hacer un parser de verdad

    def __str__(self):
        return('pkg = %s\nid = %s\nrand = %s\ndata = %s' % (self.pkg, self.id, self.rand, self.data))

#################################### SETUP #####################################

def setup():
    global state, socketTCP, socketUDP, configuration, udp_addr, udp_pdu

    state = DISCONNECTED
    msg("State = DISCONNECTED")

    socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    debug("UDP socket initialized")

    socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    debug("TCP socket initialized")

    configuration = read_configuration()
    debug("Configuration data file loaded - Device: {}".format(configuration.id))

    udp_addr = configuration.server, int(configuration.UDP)
    udp_pdu = "1B 13s 9s 61s"

def read_configuration():
    with open(configuration_opt) as file:
        id = file.readline().strip('Id =\n')
        elements = file.readline().strip('Params =\n')
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
        return('Id = %s\PParams = %s\nLocal-TCP = %s\nServer = %s\nServer-UDP = %s' % (self.id, self.elements, self.TCP, self.server, self.UDP))

################################### REGISTER ###################################

def register_and_periodic_communication(register_attempts = 1):
    # Timers and thresholds
    t = 1
    u = 2
    n = 7
    o = 3
    p = 3
    q = 3

    i = 1 # Increment variable for time
    r = 2 # Max. register attempts

    state = NOT_REGISTERED
    msg("State = NOT_REGISTERED")
    debug("Register attempt = {}".format(register_attempts))

    for package_attempt in range(1, n+1):

        sent = send_package_udp(REG_REQ, bytes(configuration.id, 'utf-8'), b'00000000', b'')
        debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_REQ", configuration.id, "00000000", "" ))

        if state == NOT_REGISTERED:
            state = WAIT_ACK_REG
            msg("State = WAIT_ACK_REG")

        if sent == 0:
            # Set delay between packages
            if(package_attempt <= p):
                time.sleep(t)
            elif t+i < q*t:
                    time.sleep(t+i)
                    i += 1
            else:
                time.sleep(q*t)
            continue
        else:
            # Receive package
            received = receive_package_udp()
            debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, received.pkg, received.id, received.rand, received.data))

            if(received.pkg == REG_ACK and state == WAIT_ACK_REG):
                #server : id, rand, ip
                sent = send_package_udp(REG_INFO, bytes(configuration.id, 'utf-8'), bytes(received.id, 'utf-8'), bytes(configuration.TCP+","+configuration.elements, 'utf-8'))

                state = WAIT_ACK_INFO
                msg("State = WAIT_ACK_INFO")

            elif(received.pkg == REG_NACK):
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")

            elif(received.pkg == REG_REJ):
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")

            elif(received.pkg == INFO_ACK and state == WAIT_ACK_INFO):
                state = REGISTERED
                msg("State = REGISTERED")

            elif(received.pkg == INFO_NACK and state == WAIT_ACK_INFO):
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")

            return

    if register_attempts<r:
        time.sleep(u)
        register_attempts += 1
        register_and_periodic_communication(register_attempts)
    else:
        msg("Number of subscription processes exceeded ({})".format(register_attempts))


########################### PERIODIC COMMUNICATION #############################
"""
    debug("Periodic communication with the server initialized")
    pass
    debug("Periodic communication with the server finished")
"""
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
    debug("Register on the server initialized")
    register_and_periodic_communication()
    debug("Register on the server finished")
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

        (options, args) = parser.parse_args()
        if len(args) > 0: parser.error ('Bad arguments, use --help for help')

        debugger_opt = options.debug
        configuration_opt = options.configuration

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
