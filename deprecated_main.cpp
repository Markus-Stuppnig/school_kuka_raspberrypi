#define CUSTOM
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include "colorSort.h"
#include "EasyCAT.h"

namespace beast = boost::beast;   
namespace websocket = beast::websocket; 
namespace net = boost::asio;       
using tcp = boost::asio::ip::tcp;   

EasyCAT easycat;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
  WebSocketSession(tcp::socket socket) : ws_(std::move(socket)) {}

  void start() {
    // Perform the WebSocket handshake
    ws_.async_accept(
      beast::bind_front_handler(&WebSocketSession::on_accept, shared_from_this()));
  }

private:
  websocket::stream<tcp::socket> ws_;
  beast::flat_buffer buffer_;

  void on_accept(beast::error_code ec) {
    if(ec) {
      std::cerr << "Error during accept: " << ec.message() << std::endl;
      return;
    }

    std::cout << "Handshake done" << std::endl;
    do_read();
  }

  void do_read() {
    ws_.async_read(
      buffer_,
      beast::bind_front_handler(&WebSocketSession::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    if(ec) {
      std::cerr << "Error during read: " << ec.message() << std::endl;
      return;
    }

    // Convert the buffer data to a string
    std::string message = beast::buffers_to_string(buffer_.data());

    // Handle the received message
    handle_message(message);

    // Clear the buffer after use
    buffer_.consume(bytes_transferred);

    // Continue reading
    do_read();
  }

  void handle_message(const std::string& message) {
    if (message.size() == 2) {
      char first_char = message[0];
      char second_char = message[1];

      // Check the first character
      if(first_char == '0') {
        easycat.BufferIn.Cust.cube = 0;
      }
      else if (first_char == '1') {
        std::cout << "First number: one" << std::endl;  
        easycat.BufferIn.Cust.cube = 1;
      } else if (first_char == '2') {
        std::cout << "First number: two" << std::endl;
        easycat.BufferIn.Cust.cube = 2;
      } else if (first_char == '3') {
        std::cout << "First number: three" << std::endl;
        easycat.BufferIn.Cust.cube = 3;
      } else {
        std::cout << "First number: unknown" << std::endl;
      }

      // Check the second character
      if(second_char == '0') {
        easycat.BufferIn.Cust.position = 0;
      } else if (second_char == '1') {
        std::cout << "Second number: one" << std::endl;
        easycat.BufferIn.Cust.position = 1;
      } else if (second_char == '2') {
        std::cout << "Second number: two" << std::endl;
        easycat.BufferIn.Cust.position = 2;
      } else if (second_char == '3') {
        std::cout << "Second number: three" << std::endl;
        easycat.BufferIn.Cust.position = 3;
      } else {
        std::cout << "Second number: unknown" << std::endl;
      }
    } else {
      std::cout << "Invalid message format" << std::endl;
    }
  }
};

class WebSocketServer {
public:
  WebSocketServer(net::io_context& ioc) : acceptor_(ioc, tcp::endpoint{tcp::v4(), 8080}) {
    do_accept();
  }

private:
  tcp::acceptor acceptor_;

  void do_accept() {
    acceptor_.async_accept(
      [this](beast::error_code ec, tcp::socket socket) {
        if (!ec) {
          std::make_shared<WebSocketSession>(std::move(socket))->start();
        }
        do_accept();
      });
  }
};

// Function for continuous output
void continuous_output() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Adjust the interval as needed
    easycat.MainTask();
  }
}

int main() {
  try {
    easycat.Init();
    easycat.BufferIn.Cust.cube = 0;
    easycat.BufferIn.Cust.position = 0;
    // Create IO context
    net::io_context ioc;

    // Start the WebSocket server
    WebSocketServer server(ioc);

    // Start a thread for continuous output
    std::thread output_thread(continuous_output);

    std::cout << "Server started" << std::endl;

    // Run the I/O service to process asynchronous operations
    ioc.run();

    // Join the output thread when the I/O context is done
    output_thread.join(); 
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
