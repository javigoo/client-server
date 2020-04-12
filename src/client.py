#!/usr/bin/env python3.7
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :

"""
SYNOPSIS

    client.py [-h,--help] [--version] [-d, --debug]
        [-c, --configuration <file>, default=client.cfg]


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
import threading

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
WAIT_INFO = 0xa3
WAIT_ACK_INFO = 0xa4
REGISTERED = 0xa5
SEND_ALIVE = 0xa6

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

    global state, socketTCP, socketUDP, configuration, udp_pdu, read_commands_thread

    state = DISCONNECTED
    msg("State = DISCONNECTED")

    socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    debug("UDP socket initialized")

    socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    debug("TCP socket initialized")

    configuration = read_configuration()
    debug("Configuration data file loaded - Device: {}".format(configuration.id))

    udp_pdu = "1B 13s 9s 61s"

    read_commands_thread = threading.Thread(target=read_commands, daemon = True)

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
        self.element_value = {}

        # Initialize the value of the elements
        elements_list = self.elements.split(';')
        for element in elements_list:
            self.element_value[element] = None


    def __str__(self):
        return('Id = %s\nParams = %s\nLocal-TCP = %s\nServer = %s\nServer-UDP = %s' % (self.id, self.elements, self.TCP, self.server, self.UDP))


################################### REGISTER ###################################

def register():
    debug("Register on the server initialized")

    global server_identification_data
    # Timers and thresholds
    t = 1
    u = 2
    n = 7
    o = 3
    p = 3
    q = 3

    max_register_attempts = 1
    timeout = 0

    # Control the number of subscription processes
    for register_attempt in range (1, max_register_attempts+1):

        state = NOT_REGISTERED
        msg("State = NOT_REGISTERED, register attempt = {}".format(register_attempt))

        for package_attempt in range(1, n+1):

            # Sending of the first registration request
            udp_addr = configuration.server, int(configuration.UDP)
            sent = send_package_udp(REG_REQ, configuration.id, "00000000", "", udp_addr)
            debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_REQ", configuration.id, "00000000", ""))

            if state == NOT_REGISTERED:
                state = WAIT_ACK_REG
                msg("State = WAIT_ACK_REG")

            # Check if the sending of the first registration request was correct
            if sent != 0:

                # Set delay between packages
                if(package_attempt <= p):
                    timeout = t
                elif timeout < q*t:
                    timeout =+ t

                # Check for server response
                i, o, e = select.select([socketUDP], [], [], timeout)
                if i != []:
                    package_content, addr = i[0].recvfrom(struct.calcsize(udp_pdu))
                    received_packet = struct.unpack(udp_pdu, package_content)
                    received = received_data(received_packet)

                    # Server identification data is saved
                    server_identification_data = server_data(received.id, received.rand, received.data)

                    if(REG_ACK == received.pkg and WAIT_ACK_REG == state):
                        debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_ACK", received.id, received.rand, received.data))

                        data = configuration.TCP+","+configuration.elements
                        addr = configuration.server, int(received.data)
                        sent = send_package_udp(REG_INFO, configuration.id, received.rand, data, addr)
                        debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_INFO", configuration.id, received.rand, data))

                        state = WAIT_ACK_INFO
                        msg("State = WAIT_ACK_INFO")

                        # Wait for confirmation of INFO_ACK
                        i, o, e = select.select([socketUDP], [], [], 2*t)
                        if i != []:
                            package_content, addr = i[0].recvfrom(struct.calcsize(udp_pdu))
                            received_packet = struct.unpack(udp_pdu, package_content)
                            received = received_data(received_packet)
                            debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, received.pkg, received.id, received.rand, received.data))

                    if(REG_NACK == received.pkg):
                        debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_NACK", received.id, received.rand, received.data))

                        state = NOT_REGISTERED
                        msg("State = NOT_REGISTERED")

                        continue

                    if(REG_REJ == received.pkg):
                        debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "REG_REJ", received.id, received.rand, received.data))

                        state = NOT_REGISTERED
                        msg("State = NOT_REGISTERED")

                    if(INFO_ACK == received.pkg and WAIT_ACK_INFO == state):
                        debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "INFO_ACK", received.id, received.rand, received.data))

                        # Checking if the server data is correct
                        if(server_identification_data.id == received.id and server_identification_data.rand == received.rand):

                            state = REGISTERED
                            msg("State = REGISTERED")

                            return
                            #port_tcp = received.data # Puerto TCP del servidor por el que recibira datos del cliente
                            #periodic_communication(state, (server_id, server_rand, server_port)) # Crear clase para server info

                    if(INFO_NACK == received.pkg and WAIT_ACK_INFO == state):
                        debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "INFO_NACK", received.id, received.rand, received.data))

                        # Checking if the server data is correct
                        if(server_id == received.id and server_rand == received.rand):

                            state = NOT_REGISTERED
                            msg("State = NOT_REGISTERED")

                            # Reason the packet was rejected
                            msg(received.data)

                            continue

                    state = NOT_REGISTERED
                    msg("State = NOT_REGISTERED")

                    # Start new register attempt
                    # ¿ break o register() o return o pass o NADA ?

        # Delay between registration processes
        time.sleep(u)

    msg("Number of subscription processes exceeded")
    sys.exit()

def send_package_udp(pkg, id, rand, data, addr):
    package_content = struct.pack(udp_pdu, pkg, bytes(id, 'utf-8'), bytes(rand, 'utf-8'), bytes(data, 'utf-8'))
    return socketUDP.sendto(package_content, addr)

class received_data:
    def __init__(self, received):
        self.pkg = received[0]
        self.id = received[1].decode()
        self.rand = received[2].decode()
        self.data = ""
        # Añadir addr devuelta

        # Parser byte to ascii (Mejorar)
        for byte in received[3]:
            if byte == 0:
                break
            self.data = self.data + chr(byte)

    def __str__(self):
        return('pkg = %s\nid = %s\nrand = %s\ndata = %s' % (self.pkg, self.id, self.rand, self.data))

class server_data:
    def __init__(self, id, random, port):
        self.id = id
        self.rand = random
        self.port = port

    def __str__(self):
        return('Id = %s\nRandom = %s\nPort= %s' % (self.id, self.rand, self.port))


########################### PERIODIC COMMUNICATION #############################

def periodic_communication():
    debug("Periodic communication with the server initialized")
    state = REGISTERED

    # Timers and thresholds
    v = 2
    r = 2
    s = 3

    alives_not_received = 0

    while True:

        udp_addr = configuration.server, int(configuration.UDP)
        sent = send_package_udp(ALIVE, configuration.id, server_identification_data.rand, "", udp_addr)
        debug("Sent: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "ALIVE", configuration.id, server_identification_data.rand, ""))

        # Wait for confirmation of ALIVE
        i, o, e = select.select([socketUDP], [], [], r*v)

        if i != []:
            package_content, addr = i[0].recvfrom(struct.calcsize(udp_pdu))
            received_packet = struct.unpack(udp_pdu, package_content)
            received = received_data(received_packet)

            if ALIVE == received.pkg:
                debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "ALIVE", received.id, received.rand, received.data))

                if(server_identification_data.id == received.id and server_identification_data.rand == received.rand and configuration.id == received.data ): # Hacer un parser de verdad
                    if state != SEND_ALIVE:
                        state = SEND_ALIVE
                        msg("State = SEND_ALIVE")

                        read_commands_thread.start()

                        # Abrir puerto TCP para recepcion de conexiones del servidor
                else:
                    state = NOT_REGISTERED
                    msg("State = NOT_REGISTERED")
                    return
                    #return register() # nou proces de suscripcio

            if ALIVE_REJ == received.pkg:
                debug("Received: bytes={}, pkg={}, id={}, rand={}, data={}".format(sent, "ALIVE_REJ", received.id, received.rand, received.data))

                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")
                return # Iniciar nou proces de suscripcio

        else:
            if(state == SEND_ALIVE and alives_not_received < s):
                msg("Alive packet {} not received".format(alives_not_received+1))
                alives_not_received += 1

            else:
                state = NOT_REGISTERED
                msg("State = NOT_REGISTERED")
                return
                #return register() # nou proces de suscripcio

        time.sleep(v)

################################## SEND DATA ###################################

def read_commands():
    debug("Send data to the server initialized")

    while True:
        command_line = sys.stdin.readline().strip()
        command = command_line.split()

        if len(command) == 0:
            pass
        elif command[0] == "stat":
            elements_stats()
        elif command[0] == "set":
            if len(command) == 3:
                set_element_value(command[1], command[2])
            else:
                msg("Bad arguments, use:    set <element_identifier> <new_value>")
        elif command[0] == "send":
            if len(command) == 2:
                send_data(command[1])
            else:
                msg("Bad arguments, use:    send <element_identifier>")
        elif command[0] == "quit":
            socketUDP.close()
            socketTCP.close()
            os._exit(1)
        else:
            msg('"{}" is not a valid command'.format(command_line))

def elements_stats():
    print("\nId: {}\n".format(configuration.id))
    for element in configuration.element_value:
        print("{} - {}".format(element, configuration.element_value[element]))
    print("\n")

def set_element_value(element_identifier, new_value):
    if element_identifier in configuration.element_value:
        # Mida maxima de 15 bytes
        configuration.element_value[element_identifier] = new_value
    else:
        msg('"{}" is not part of the device elements'.format(element_identifier))

def send_data(element_identifier):
    print('"send" Not Implemented')

############################## WAIT CONNECTIONS ################################

def wait_connections():
    debug("Wait for server tcp connections initialized")
    pass

#################################### MAIN ######################################

def main():

    setup()
    register()
    periodic_communication()
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
