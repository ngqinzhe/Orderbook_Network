#include <iostream>
#include <boost/asio.hpp>
/**
 * IMPLEMENT LIMIT ORDER, MARKET, CANCEL
*/

using namespace boost::asio;
using ip::tcp;

int main(int argc, char* argv[])
{
    // parse input
    std::vector<std::string> result;
    std::string input;
    std::cout << "Input your order details:\n";
    while (std::cin >> input) {
        if (input == "END") break;
        result.push_back(input);
    }

    boost::asio::io_service io_service;

    tcp::socket socket_(io_service);
    // connerrortion to Server
    socket_.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234));

    // request/message from client
    //const std::string message = outputMessage;
    boost::system::error_code error;
    // write message and send vector
    boost::asio::write(socket_, boost::asio::buffer(result), error);
    if (!error) 
        std::cout << "Order sent." << std::endl;
    else 
        std::cout << "Client send failed: " << error.message() << std::endl;

    // getting response from server
    boost::asio::streambuf receive_buffer;
    boost::asio::read(socket_, receive_buffer, boost::asio::transfer_all(), error);
    if (error && error != boost::asio::error::eof) 
        std::cout << "Receive failed: " << error.message() << std::endl;
    else
    {
        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        std::cout << data << std::endl;
    } 

    return 0;

}