import socket
from time import sleep

HOST = "127.0.0.1"  # Server address
PORT = 8080          # Server port

# Create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    client_socket.connect((HOST, PORT))
    print("Connected to server")


    sleep(5)
    # Send message
    client_socket.sendall(b"hello")
    print("Message sent")
finally:
    client_socket.close()
    print("Connection closed")
