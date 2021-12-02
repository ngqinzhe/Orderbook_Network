# Orderbook with Networking using Boost ASIO 

This is built on top of my [OrderMatchingEngine](https://github.com/ngqinzhe/Order_Matching_Engine) which introduces networking where the client side can send orders to the server, and the server will have the Orderbook updated. The server runs asynchronously and can receive orders from multiple clients.

This supports `BUY` and `SELL` sides, and the main orders namely `LIMIT`, `MARKET` and `CANCEL` orders.

Orders in the order book are represented by '[Quantity]@[Price]#[OrderID]'. E.g. a BUY order represented as '350@9#qu82' is an order where the user is willing to buy 350 units of the stock at $9.

## Installing Packages
Head over to install [Boost](https://www.boost.org/) and [ASIO](https://think-async.com/). If you are using mac and have homebrew installed:
```
brew install boost
```
```
brew install asio
```

## Compiling the program
Since C++ is platform dependent, it is recommended that you compile it on your local machine before trying to run it.

Locate your boost and ASIO lib folders, and add them after the `-I` include flag as such to compile the programs. For me, the containing folder is /opt/homebrew/Cellar/boost/1.76.0/include. 

You need to compile both the server and client.
```
g++ -std=c++20 -I/opt/homebrew/Cellar/boost/1.76.0/include server.cpp new_OrderBook.cpp -o server
```
```
g++ -std=c++20 -I/opt/homebrew/Cellar/boost/1.76.0/include client.cpp -o client
```

## Running the program
After compilation, simply open two terminals, one for server and client. Run the server application first, to start up the server. For simplicity, we are assuming that the server is running on localhost 127.0.0.1 on port: 1234

```
./server
```

After starting up the server, headover to another terminal tab and run the client.
```
./client
```

After running the client, input the orders into the terminal. Currently it only supports one order per client, and typing `END` will send the order over to the server.

Server will also update the Orderbook and print out the current orders in the order book whenever it receives a new order from a client. 


#### SAMPLE LIMIT ORDER
```
SUB LO B Ffuj 200 13
END
```

#### SAMPLE MARKET ORDER
```
SUB MO B IZLO 250
END
```

### SAMPLE CANCEL ORDER
```
CXL Ffuj
END
```

