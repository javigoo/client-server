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

# Periodic communication packages
ALIVE = 0x10
ALIVE_REJ = 0x11

def debug(msg):
    if debugger_opt: print(time.strftime("%H:%M:%S") + ": DEBUG -> " + str(msg))

def msg(msg):
    print(time.strftime("%H:%M:%S") + ": MSG   -> " + str(msg))

#################################### SETUP #####################################

def setup():

    global state, socketTCP, socketUDP, configuration, udp_pdu

    state = DISCONNECTED
    msg("State = DISCONNECTED")

    socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    debug("UDP socket initialized")

    socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    debug("TCP socket initialized")

    configuration = read_configuration()
    debug("Configuration data file loaded - Device: {}".format(configuration.id))

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

def register(register_attempts = 1):
    if register_attempts == 1:
        debug("Register on the server initialized")

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
        udp_addr = configuration.server, int(configuration.UDP)
        sent = send_package_udp(REG_REQ, configuration.id, "00000000", "", udp_addr)
        debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_REQ", configuration.id, "00000000", "" ))

        if state == NOT_REGISTERED:
            state = WAIT_ACK_REG
            msg("State = WAIT_ACK_REG")

        # Receive package
        if sent != 0:
            received = receive_package_udp()
            debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, received.pkg, received.id, received.rand, received.data))

            server_id = received.id
            server_rand = received.rand
            server_port = received.data

            if(REG_ACK == received.pkg and WAIT_ACK_REG == state):

                data = configuration.TCP+","+configuration.elements
                addr = configuration.server, int(server_port)
                sent = send_package_udp(REG_INFO, configuration.id, received.rand, data, addr)
                debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_INFO", configuration.id, received.rand, data))

                state = WAIT_ACK_INFO
                msg("State = WAIT_ACK_INFO")

                # Wait for confirmation of INFO_ACK
                i, o, e = select.select([socketUDP], [], [], 2*t)
                if i != []:
                    package_content, addr = i[0].recvfrom(struct.calcsize(udp_pdu))
                    received = struct.unpack(udp_pdu, package_content)
                    received = received_data(received)
                    debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, received.pkg, received.id, received.rand, received.data))
                else:
                    state = NOT_REGISTERED
                    msg("State = NOT_REGISTERED")
                    # Start new register attempt

            if(REG_NACK == received.pkg):
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")
                #go to send REG_REQ
                msg("REG_NACK")
                sys.exit()

            if(REG_REJ == received.pkg):
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")
                break

            if(INFO_ACK == received.pkg and WAIT_ACK_INFO == state):
                # Checking if the server data is correct
                if(server_id == received.id and server_rand == received.rand):
                    state = REGISTERED
                    msg("State = REGISTERED")
                    port_tcp = received.data # Puerto TCP del servidor por el que recibira datos del cliente
                    periodic_communication(state, (server_id, server_rand, server_port)) # Crear clase para server info

            if(INFO_NACK == received.pkg and WAIT_ACK_INFO == state):
                # Checking if the server data is correct
                if(server_id == received.id and server_rand == received.rand):
                    state = NOT_REGISTERED
                    msg("State = NOT_REGISTERED")
                    msg(received.data)
                    #go to send REG_REQ
                    msg("INFO_NACK")
                    sys.exit()

            state = NOT_REGISTERED
            msg("State = NOT_REGISTERED")
            return

        # Set delay between packages
        else:
            if(package_attempt <= p):
                time.sleep(t)
            elif i*t < q*t:
                time.sleep(i*t)
                i += 1
            else:
                time.sleep(q*t)
            continue

    # Control of number of subscription processes
    if register_attempts<r:
        time.sleep(u)
        register_attempts += 1
        register(register_attempts)
    else:
        msg("Number of subscription processes exceeded ({})".format(register_attempts))
        sys.exit()

def send_package_udp(pkg, id, rand, data, addr):
    package_content = struct.pack(udp_pdu, pkg, bytes(id, 'utf-8'), bytes(rand, 'utf-8'), bytes(data, 'utf-8'))
    return socketUDP.sendto(package_content, addr)

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
        # Añadir addr devuelta

    def __str__(self):
        return('pkg = %s\nid = %s\nrand = %s\ndata = %s' % (self.pkg, self.id, self.rand, self.data))

########################### PERIODIC COMMUNICATION #############################

def periodic_communication(state = DISCONNECTED, server_info = None):
    debug("Periodic communication with the server initialized")

    # Timers and thresholds
    v = 2
    r = 2
    s = 3

    if state == REGISTERED:
        while True:
            # Send alive
            udp_addr = configuration.server, int(configuration.UDP)
            sent = send_package_udp(ALIVE, configuration.id, server_info[1], "", udp_addr)
            debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "ALIVE", configuration.id, server_info[1], ""))

            # Wait for confirmation of INFO_ACK
            i, o, e = select.select([socketUDP], [], [], r*v)
            if i != []:
                package_content, addr = i[0].recvfrom(struct.calcsize(udp_pdu))
                received = struct.unpack(udp_pdu, package_content)
                received = received_data(received)
                debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, received.pkg, received.id, received.rand, received.data))

                if(server_info[0] == received.id and server_info[1] == received.rand and configuration.id[:5] == received.data ): # Hacer un parser de verdad
                    if state != SEND_ALIVE:
                        state = SEND_ALIVE
                        msg("State = SEND_ALIVE")

                    # Abrir puerto TCP para recepcion de conexiones del servidor
                    # Leer comandos de terminal
            else:
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")
                return
                #return register() # nou proces de suscripcio

            time.sleep(v)
    else:
        msg("Incorrect client state")
        sys.exit()

################################# SEND DATA ####################################

def send_data():
    debug("Send data to the server initialized")
    pass

############################## WAIT CONNECTIONS ################################

def wait_connections():
    debug("Wait for server tcp connections initialized")
    pass

#################################### MAIN ######################################

def main():
    setup()
    register()
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
