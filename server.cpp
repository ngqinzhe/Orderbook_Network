#include <iostream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "new_OrderBook.h"


using namespace boost::asio;
using ip::tcp;



class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
private:
    tcp::socket sock;
    std::string message = "FROM SERVER: Server received your order.\n";
    enum { max_length = 1024};
    //char data[max_length];
    std::vector<std::string> orderbuf = std::vector<std::string>(7);
    OB::Orderbook* _orderBook;

public:
    typedef boost::shared_ptr<ConnectionHandler> pointer;
    ConnectionHandler(boost::asio::io_context& io_context, OB::Orderbook& orderBook) : sock(io_context) 
    {
        _orderBook = &orderBook;
    }
    
    // pointer creation
    static pointer create(boost::asio::io_context& io_context, OB::Orderbook& orderBook)
    {
        return pointer(new ConnectionHandler(io_context, orderBook));
    }

    //socket creation
    tcp::socket& socket()
    {
        return sock;
    }

    void start()
    {
        sock.async_read_some(
            boost::asio::buffer(orderbuf),
            boost::bind(&ConnectionHandler::handle_read,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
        );

        sock.async_write_some(
            boost::asio::buffer(message, max_length),
            boost::bind(&ConnectionHandler::handle_write,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
        );
    }

    void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) 
        {
            std::cout << "FROM CLIENT: ";
            for (const std::string x : orderbuf)
                std::cout << x << " ";
            std::cout << std::endl;

            for (int i = 0; i < orderbuf.size(); ++i) 
            {
                if (orderbuf[i] == "CXL") {
                    std::string orderId = orderbuf[i + 1];
                    OB::cancelOrder((*_orderBook), orderId);
                    i += 1;
                    continue;
                }
                if (orderbuf[i] == "SUB") {
                    std::string orderType = orderbuf[i + 1];
                    std::string t = orderbuf[i+2];
                    std::string n = orderbuf[i + 3];
                    int q = std::stoi(orderbuf[i + 4]);
                    if (orderType == "LO") {
                        int p = std::stoi(orderbuf[i + 5]);
                        std::shared_ptr<OB::Order> order = std::make_shared<OB::Order>(p, q, n, t);
                        if (t == "B") {
                            _orderBook->insert(order);
                            OB::limitOrderMatch((*_orderBook));
                        }
                        else {
                            _orderBook->insert(order);
                            OB::limitOrderMatch((*_orderBook));
                        }
                        i += 5;
                        continue;
                    }
                    else if (orderType == "MO") {
                        int p = 0;
                        std::shared_ptr<OB::Order> order = std::make_shared<OB::Order>(p, q, n, t);
                        OB::marketOrderMatch((*_orderBook), order);
                        i += 4;
                        continue;
                    }
                }
            }
            _orderBook->print();
            std::cout << "Orderbook updated." << std::endl;
        }
        else 
        {
            std::cerr << "Error: " << err.message() << std::endl;
            sock.close();
        }
    }

    void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) 
        {
            
        }
        else 
        {
            std::cerr << "Error: " << err.message() << std::endl;
            sock.close();
        }
    }
};

class Server 
{
private:
    tcp::acceptor acceptor_;
    OB::Orderbook ob;

private:    
    void start_accept()
    {
        //socket
        ConnectionHandler::pointer connection = ConnectionHandler::create((boost::asio::io_context&) acceptor_.get_executor().context(), ob);

        acceptor_.async_accept(connection->socket(),
        boost::bind(&Server::handle_accept, this, connection, 
        boost::asio::placeholders::error));
    }

public:
    Server(boost::asio::io_context& io_context) : acceptor_(io_context, tcp::endpoint(tcp::v4(), 1234))
    {
        ob = OB::Orderbook();
        start_accept();
    }

    void handle_accept(ConnectionHandler::pointer connection, const boost::system::error_code& err)
    {
        if (!err)
            connection->start();
        start_accept();
    }
};


int main()
{
    try
    {
        boost::asio::io_context io_context;
        Server server(io_context);
        io_context.run();   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}