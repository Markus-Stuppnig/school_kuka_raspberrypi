#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CUSTOM
#include "EasyCAT.h"

#define PORT 8080
#define BUFFER_SIZE 1024

EasyCAT myEasyCAT;

int i = 0;

void send_coordinates(uint8_t x, uint8_t y, uint8_t index) {
    std::cout << "Sending coordinates: x=" << x << ", y=" << y << ", index=" << index << std::endl;
    myEasyCAT.BufferIn.Cust.x = x;
    myEasyCAT.BufferIn.Cust.y = y;
    myEasyCAT.BufferIn.Cust.index = index;
    std::cout << "Coordinates sent: x=" << x << ", y=" << y << ", index=" << index << std::endl;
}

void initEasyCat() {
    if (myEasyCAT.Init() == true) {
        std::cout << "EasyCAT Initialization Successful" << std::endl;
    } else {
        std::cout << "EasyCAT Initialization Failed" << std::endl;
    }
}

void mainEasyCat() {
    #ifdef DEBUG
        std::cout << "Running Main Task: " << i << std::endl;
    #endif
    myEasyCAT.MainTask();
}

void parse_and_send(const char* buffer) {
    unsigned int x, y, index;  // Use unsigned int for sscanf, as it doesn't directly support uint8_t

    if (sscanf(buffer, "%u,%u,%u", &x, &y, &index) == 3 && x <= 255 && y <= 255 && index <= 255) {
        uint8_t x8 = static_cast<uint8_t>(x);
        uint8_t y8 = static_cast<uint8_t>(y);
        uint8_t index8 = static_cast<uint8_t>(index);

        std::cout << "Parsed values - x: " << static_cast<int>(x8)
                  << ", y: " << static_cast<int>(y8)
                  << ", index: " << static_cast<int>(index8) << std::endl;

        send_coordinates(x8, y8, index8);
    } else {
        std::cout << "Invalid format received" << std::endl;
    }
}

int main() {
    #ifdef DEBUG
        std::cout << "Debug mode is ON" << std::endl;
    #endif

    initEasyCat();

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
        // EasyCat
        mainEasyCat();

        // Accept connection (non-blocking)
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd >= 0) {
            std::cout << "Client connected." << std::endl;
            fcntl(client_fd, F_SETFL, O_NONBLOCK);
            break;
        }
    }
    // int i = 0;
    while (true) {

        // EasyCat
        mainEasyCat();

        // Read data from client
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            std::cout << "Received: " << buffer << std::endl;
            parse_and_send(buffer);
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
