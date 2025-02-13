#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return 1;
    }

    // Set non-blocking mode
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    while (true) {
        // Accept connection (non-blocking)
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd >= 0) {
            std::cout << "Client connected." << std::endl;
            fcntl(client_fd, F_SETFL, O_NONBLOCK);
            break;
        }
    }
    int i = 0;
    while (true) {
        std::cout << i++ << std::endl;
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            std::cout << "Received: " << buffer << std::endl;
            memset(buffer, 0, BUFFER_SIZE);
        } else if (bytes_read == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
