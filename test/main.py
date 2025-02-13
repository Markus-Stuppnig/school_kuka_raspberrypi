import socket
from time import sleep

HOST = "172.20.10.13"  # Server address
PORT = 8080          # Server port

# Create socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    client_socket.connect((HOST, PORT))
    print("Connected to server")


    sleep(5)
    # Send message
    client_socket.sendall(b"17,68,1")
    print("Message sent")
finally:
    client_socket.close()
    print("Connection closed")
