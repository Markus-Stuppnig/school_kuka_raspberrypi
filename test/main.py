import socket
from time import sleep

HOST = "172.20.10.13"  # Server address
PORT = 8080          # Server port

# Create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    client_socket.connect((HOST, PORT))
    print("Connected to server")

    client_socket.sendall(b"5,10,8")

    while True:
        pass
    # Send message
    print("Message sent")
finally:
    client_socket.close()
    print("Connection closed")
