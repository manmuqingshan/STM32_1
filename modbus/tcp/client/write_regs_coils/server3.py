#!/usr/bin/env python3

import socket
import struct


HOST = "0.0.0.0"
PORT = 502


# =========================================================
# Modbus Data
# =========================================================

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


# =========================================================
# Exception Codes
# =========================================================

ILLEGAL_FUNCTION = 0x01
ILLEGAL_ADDRESS = 0x02
ILLEGAL_VALUE = 0x03
SERVER_DEVICE_FAILURE = 0x04


# =========================================================
# Print Hex
# =========================================================

def print_hex(data):

    print("HEX : " + " ".join(f"{b:02X}" for b in data))


# =========================================================
# Exception Response
# =========================================================

def exception_response(transaction,
                        unit,
                        function,
                        exception_code):

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        3,
        unit,
        function | 0x80,
        exception_code
    )

    return response


# =========================================================
# FC01 - Read Coils
# =========================================================

def read_coils(transaction,
               unit,
               start,
               quantity):

    if start + quantity > len(COILS):

        return exception_response(
            transaction,
            unit,
            1,
            ILLEGAL_ADDRESS
        )

    byte_count = (quantity + 7) // 8

    data = bytearray(byte_count)

    for i in range(quantity):

        if COILS[start + i]:

            data[i // 8] |= (1 << (i % 8))

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        byte_count + 3,
        unit,
        1,
        byte_count
    )

    response += data

    return response


# =========================================================
# FC02 - Read Discrete Inputs
# =========================================================

def read_discrete_inputs(transaction,
                         unit,
                         start,
                         quantity):

    if start + quantity > len(INPUTS):

        return exception_response(
            transaction,
            unit,
            2,
            ILLEGAL_ADDRESS
        )

    byte_count = (quantity + 7) // 8

    data = bytearray(byte_count)

    for i in range(quantity):

        if INPUTS[start + i]:

            data[i // 8] |= (1 << (i % 8))

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        byte_count + 3,
        unit,
        2,
        byte_count
    )

    response += data

    return response


# =========================================================
# FC03 - Read Holding Registers
# =========================================================

def read_holding_registers(transaction,
                            unit,
                            start,
                            quantity):

    if start + quantity > len(HOLDING_REGS):

        return exception_response(
            transaction,
            unit,
            3,
            ILLEGAL_ADDRESS
        )

    byte_count = quantity * 2

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        byte_count + 3,
        unit,
        3,
        byte_count
    )

    for i in range(quantity):

        response += struct.pack(
            ">H",
            HOLDING_REGS[start + i]
        )

    return response


# =========================================================
# FC04 - Read Input Registers
# =========================================================

def read_input_registers(transaction,
                         unit,
                         start,
                         quantity):

    if start + quantity > len(INPUT_REGS):

        return exception_response(
            transaction,
            unit,
            4,
            ILLEGAL_ADDRESS
        )

    byte_count = quantity * 2

    response = struct.pack(
        ">HHHBBB",
        transaction,
        0,
        byte_count + 3,
        unit,
        4,
        byte_count
    )

    for i in range(quantity):

        response += struct.pack(
            ">H",
            INPUT_REGS[start + i]
        )

    return response


# =========================================================
# FC05 - Write Single Coil
# =========================================================

def write_single_coil(transaction,
                      unit,
                      address,
                      value):

    if address >= len(COILS):

        return exception_response(
            transaction,
            unit,
            5,
            ILLEGAL_ADDRESS
        )

    if value == 0xFF00:

        new_value = 1

    elif value == 0x0000:

        new_value = 0

    else:

        return exception_response(
            transaction,
            unit,
            5,
            ILLEGAL_VALUE
        )

    old_value = COILS[address]

    COILS[address] = new_value

    print("\r\n*** Coil Changed ***")
    print(f"Address : {address}")
    print(f"Old     : {old_value}")
    print(f"New     : {new_value}")

    response = struct.pack(
        ">HHHBBHH",
        transaction,
        0,
        6,
        unit,
        5,
        address,
        value
    )

    return response


# =========================================================
# FC06 - Write Single Register
# =========================================================

def write_single_register(transaction,
                          unit,
                          address,
                          value):

    if address >= len(HOLDING_REGS):

        return exception_response(
            transaction,
            unit,
            6,
            ILLEGAL_ADDRESS
        )

    old_value = HOLDING_REGS[address]

    HOLDING_REGS[address] = value

    print("\r\n*** Holding Register Changed ***")
    print(f"Address : {address}")
    print(f"Old     : {old_value}")
    print(f"New     : {value}")

    response = struct.pack(
        ">HHHBBHH",
        transaction,
        0,
        6,
        unit,
        6,
        address,
        value
    )

    return response


# =========================================================
# FC15 - Write Multiple Coils
# =========================================================

def write_multiple_coils(transaction,
                         unit,
                         start,
                         quantity,
                         data):

    if start + quantity > len(COILS):

        return exception_response(
            transaction,
            unit,
            15,
            ILLEGAL_ADDRESS
        )

    if quantity == 0:

        return exception_response(
            transaction,
            unit,
            15,
            ILLEGAL_VALUE
        )

    print("\r\n*** Coils Changed ***")
    print("---------------------")

    for i in range(quantity):

        new_value = (data[i // 8] >> (i % 8)) & 0x01

        address = start + i

        old_value = COILS[address]

        COILS[address] = new_value

        print(
            f"Address : {address:03d}  "
            f"Old : {old_value}  "
            f"New : {new_value}"
        )

    response = struct.pack(
        ">HHHBBHH",
        transaction,
        0,
        6,
        unit,
        15,
        start,
        quantity
    )

    return response


# =========================================================
# FC16 - Write Multiple Registers
# =========================================================

def write_multiple_registers(transaction,
                             unit,
                             start,
                             quantity,
                             data):

    if start + quantity > len(HOLDING_REGS):

        return exception_response(
            transaction,
            unit,
            16,
            ILLEGAL_ADDRESS
        )

    if quantity == 0:

        return exception_response(
            transaction,
            unit,
            16,
            ILLEGAL_VALUE
        )

    print("\r\n*** Holding Registers Changed ***")
    print("----------------------------------")

    for i in range(quantity):

        value = struct.unpack(
            ">H",
            data[i * 2:i * 2 + 2]
        )[0]

        address = start + i

        old_value = HOLDING_REGS[address]

        HOLDING_REGS[address] = value

        print(
            f"Address : {address:03d}  "
            f"Old : {old_value}  "
            f"New : {value}"
        )

    response = struct.pack(
        ">HHHBBHH",
        transaction,
        0,
        6,
        unit,
        16,
        start,
        quantity
    )

    return response


# =========================================================
# Main Server
# =========================================================

print("==========================================")
print("        Modbus TCP Test Server")
print("==========================================")
print(f"Listening on {HOST}:{PORT}")
print("Supported Function Codes:")
print("FC01 - Read Coils")
print("FC02 - Read Discrete Inputs")
print("FC03 - Read Holding Registers")
print("FC04 - Read Input Registers")
print("FC05 - Write Single Coil")
print("FC06 - Write Single Register")
print("FC15 - Write Multiple Coils")
print("FC16 - Write Multiple Registers")
print("==========================================")
print()


server = socket.socket(
    socket.AF_INET,
    socket.SOCK_STREAM
)

server.setsockopt(
    socket.SOL_SOCKET,
    socket.SO_REUSEADDR,
    1
)

server.bind((HOST, PORT))

server.listen()


while True:

    client, address = server.accept()

    print("\r\n==========================================")
    print("Client Connected")
    print(f"IP   : {address[0]}")
    print(f"Port : {address[1]}")
    print("==========================================")

    while True:

        request = client.recv(260)

        if not request:

            break

        if len(request) < 8:

            continue

        print("\r\n------------------------------------------")
        print("Request")
        print("------------------------------------------")

        print_hex(request)

        # -------------------------------------------------
        # MBAP Header
        # -------------------------------------------------

        transaction = struct.unpack(
            ">H",
            request[0:2]
        )[0]

        protocol = struct.unpack(
            ">H",
            request[2:4]
        )[0]

        length = struct.unpack(
            ">H",
            request[4:6]
        )[0]

        unit = request[6]

        function = request[7]

        print(f"Transaction ID : {transaction}")
        print(f"Protocol ID    : {protocol}")
        print(f"Length         : {length}")
        print(f"Unit ID        : {unit}")
        print(f"Function Code  : {function}")

        # -------------------------------------------------
        # FC01 / FC02 / FC03 / FC04
        # -------------------------------------------------

        if function in [1, 2, 3, 4]:

            if len(request) < 12:

                continue

            start = struct.unpack(
                ">H",
                request[8:10]
            )[0]

            quantity = struct.unpack(
                ">H",
                request[10:12]
            )[0]

            print(f"Start Address  : {start}")
            print(f"Quantity       : {quantity}")

            if quantity == 0:

                response = exception_response(
                    transaction,
                    unit,
                    function,
                    ILLEGAL_VALUE
                )

            elif function == 1:

                response = read_coils(
                    transaction,
                    unit,
                    start,
                    quantity
                )

            elif function == 2:

                response = read_discrete_inputs(
                    transaction,
                    unit,
                    start,
                    quantity
                )

            elif function == 3:

                response = read_holding_registers(
                    transaction,
                    unit,
                    start,
                    quantity
                )

            else:

                response = read_input_registers(
                    transaction,
                    unit,
                    start,
                    quantity
                )

        # -------------------------------------------------
        # FC05 - Write Single Coil
        # -------------------------------------------------

        elif function == 5:

            if len(request) < 12:

                continue

            address = struct.unpack(
                ">H",
                request[8:10]
            )[0]

            value = struct.unpack(
                ">H",
                request[10:12]
            )[0]

            print(f"Address        : {address}")
            print(f"Value          : 0x{value:04X}")

            response = write_single_coil(
                transaction,
                unit,
                address,
                value
            )

        # -------------------------------------------------
        # FC06 - Write Single Register
        # -------------------------------------------------

        elif function == 6:

            if len(request) < 12:

                continue

            address = struct.unpack(
                ">H",
                request[8:10]
            )[0]

            value = struct.unpack(
                ">H",
                request[10:12]
            )[0]

            print(f"Address        : {address}")
            print(f"Value          : {value}")

            response = write_single_register(
                transaction,
                unit,
                address,
                value
            )

        # -------------------------------------------------
        # FC15 - Write Multiple Coils
        # -------------------------------------------------

        elif function == 15:

            if len(request) < 13:

                continue

            start = struct.unpack(
                ">H",
                request[8:10]
            )[0]

            quantity = struct.unpack(
                ">H",
                request[10:12]
            )[0]

            byte_count = request[12]

            data = request[13:13 + byte_count]

            print(f"Start Address  : {start}")
            print(f"Quantity       : {quantity}")
            print(f"Byte Count     : {byte_count}")

            response = write_multiple_coils(
                transaction,
                unit,
                start,
                quantity,
                data
            )

        # -------------------------------------------------
        # FC16 - Write Multiple Registers
        # -------------------------------------------------

        elif function == 16:

            if len(request) < 13:

                continue

            start = struct.unpack(
                ">H",
                request[8:10]
            )[0]

            quantity = struct.unpack(
                ">H",
                request[10:12]
            )[0]

            byte_count = request[12]

            data = request[13:13 + byte_count]

            print(f"Start Address  : {start}")
            print(f"Quantity       : {quantity}")
            print(f"Byte Count     : {byte_count}")

            response = write_multiple_registers(
                transaction,
                unit,
                start,
                quantity,
                data
            )

        # -------------------------------------------------
        # Unsupported Function
        # -------------------------------------------------

        else:

            print(
                f"Unsupported Function Code : {function}"
            )

            response = exception_response(
                transaction,
                unit,
                function,
                ILLEGAL_FUNCTION
            )

        # -------------------------------------------------
        # Send Response
        # -------------------------------------------------

        print("\r\nResponse")
        print("------------------------------------------")

        print_hex(response)

        client.sendall(response)

    client.close()

    print("\r\nClient Disconnected")