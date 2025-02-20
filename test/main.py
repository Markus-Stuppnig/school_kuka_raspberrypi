import socket
from time import sleep

HOST = "10.0.0.1"  # Server address
PORT = 8080          # Server port

# Create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    client_socket.connect((HOST, PORT))
    print("Connected to server")
    print("test")

    client_socket.sendall(b"Hallo Anja")
    sleep(5)
    # Send message
    client_socket.sendall(b"Tschuess Anja")
    print("Message sent")
finally:
    client_socket.close()
    print("Connection closed")
