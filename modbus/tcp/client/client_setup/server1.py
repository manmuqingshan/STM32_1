import socket

HOST = "0.0.0.0"
PORT = 502

# Create TCP socket
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

server.bind((HOST, PORT))
server.listen(1)

print(f"Modbus TCP Test Server Listening on Port {PORT}...\n")

conn, addr = server.accept()

print(f"Client Connected : {addr[0]}:{addr[1]}")

while True:

    data = conn.recv(260)

    if not data:
        print("Client Disconnected")
        break

    if len(data) < 12:
        print("Invalid Modbus TCP Request")
        continue
        
    print(f"Transaction ID : {data[0]:02X} {data[1]:02X}")
    print(f"Protocol ID    : {data[2]:02X} {data[3]:02X}")
    print(f"Length         : {data[4]:02X} {data[5]:02X}")
    print(f"Unit ID        : {data[6]:02X}")
    print(f"Function Code  : {data[7]:02X}")
    print(f"Start Address  : {data[8]:02X} {data[9]:02X}")
    print(f"Quantity       : {data[10]:02X} {data[11]:02X}")

conn.close()
server.close()