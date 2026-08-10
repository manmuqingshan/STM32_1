#!/usr/bin/env python3

import socket
import struct

HOST = "0.0.0.0"
PORT = 502

# ---------------------------------------------------------
# Databases
# ---------------------------------------------------------

COILS = [
    1,0,1,1,0,1,0,0,
    1,1,0,0,1,0,1,0
]

INPUTS = [
    0,1,0,0,1,1,0,1,
    1,0,1,0,0,1,0,1
]

HOLDING_REGS = [
    100,101,102,103,104,
    105,106,107,108,109
]

INPUT_REGS = [
    1000,1001,1002,1003,1004,
    1005,1006,1007,1008,1009
]

# ---------------------------------------------------------
# Exception Codes
# ---------------------------------------------------------

ILLEGAL_FUNCTION = 1
ILLEGAL_ADDRESS  = 2
ILLEGAL_VALUE    = 3

# ---------------------------------------------------------

def print_hex(data):

    print("HEX :", " ".join(f"{b:02X}" for b in data))


# ---------------------------------------------------------

def exception_response(transaction, unit, function, code):

    length = 3

    return struct.pack(
        ">HHHBBB",
        transaction,
        0,
        length,
        unit,
        function | 0x80,
        code
    )


# ---------------------------------------------------------

def read_registers(database,
                   transaction,
                   unit,
                   function,
                   start,
                   quantity):

    if start + quantity > len(database):
        return exception_response(transaction,
                                  unit,
                                  function,
                                  ILLEGAL_ADDRESS)

    byte_count = quantity * 2

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        byte_count + 3,
        unit,
        function,
        byte_count
    )

    for value in database:
        pass

    for value in database[start:start+quantity]:
        response += struct.pack(">H", value)

    return response


# ---------------------------------------------------------

def read_bits(database,
              transaction,
              unit,
              function,
              start,
              quantity):

    if start + quantity > len(database):
        return exception_response(transaction,
                                  unit,
                                  function,
                                  ILLEGAL_ADDRESS)

    byte_count = (quantity + 7) // 8

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        byte_count + 3,
        unit,
        function,
        byte_count
    )

    packed = []

    for b in range(byte_count):

        value = 0

        for bit in range(8):

            index = start + b*8 + bit

            if index >= start + quantity:
                break

            if database[index]:
                value |= (1 << bit)

        packed.append(value)

    response += bytes(packed)

    return response


# ---------------------------------------------------------

print("===================================")
print("Simple Modbus TCP Server")
print("Listening on Port", PORT)
print("===================================\n")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET,
                  socket.SO_REUSEADDR,
                  1)

server.bind((HOST, PORT))
server.listen()

while True:

    client, address = server.accept()

    print("Client Connected :", address)

    while True:

        request = client.recv(260)

        if not request:
            break

        print("\n------------------------------------")
        print("Request")
        print("------------------------------------")

        print_hex(request)

        transaction = struct.unpack(">H", request[0:2])[0]
        protocol    = struct.unpack(">H", request[2:4])[0]
        length      = struct.unpack(">H", request[4:6])[0]
        unit        = request[6]
        function    = request[7]
        start       = struct.unpack(">H", request[8:10])[0]
        quantity    = struct.unpack(">H", request[10:12])[0]

        print("Transaction ID :", transaction)
        print("Unit ID        :", unit)
        print("Function Code  :", function)
        print("Start Address  :", start)
        print("Quantity       :", quantity)

        if quantity == 0:
            response = exception_response(
                transaction,
                unit,
                function,
                ILLEGAL_VALUE
            )

        elif function == 1:

            response = read_bits(
                COILS,
                transaction,
                unit,
                function,
                start,
                quantity
            )

        elif function == 2:

            response = read_bits(
                INPUTS,
                transaction,
                unit,
                function,
                start,
                quantity
            )

        elif function == 3:

            response = read_registers(
                HOLDING_REGS,
                transaction,
                unit,
                function,
                start,
                quantity
            )

        elif function == 4:

            response = read_registers(
                INPUT_REGS,
                transaction,
                unit,
                function,
                start,
                quantity
            )

        else:

            response = exception_response(
                transaction,
                unit,
                function,
                ILLEGAL_FUNCTION
            )

        print("\nResponse")
        print("------------------------------------")
        print_hex(response)

        client.sendall(response)

    client.close()

    print("\nClient Disconnected\n")