CC = g++
CFLAGS = -Wall -Werror -std=c++20 -O2 -I/opt/homebrew/Cellar/boost/1.76.0/include

main: server.cpp new_OrderBook.cpp
	$(CC) $(CFLAGS) server.cpp new_OrderBook.cpp -o server