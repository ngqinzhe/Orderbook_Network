#ifndef new_orderbook_h
#define new_orderbook_h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace OB {
    struct Order {
        int price;
        int quantity;
        std::string name;
        std::string type;
        int display;

        Order(int p, int q, std::string n, std::string t);
        Order(int p, int q, std::string n, std::string t, int dis);
        std::string get();
    };
    class Orderbook {
    private:
        std::vector<std::shared_ptr<Order> > bo;
        std::vector<std::shared_ptr<Order> > so;
    public:
        Orderbook();
        ~Orderbook();
        std::vector<std::shared_ptr<Order> > buyOrderbook();
        std::vector<std::shared_ptr<Order> > sellOrderbook();
        void insert(std::shared_ptr<Order> o);
        void print();
        void limitOrderMatch();
        void marketOrderMatch(std::shared_ptr<Order> o);
        void iocOrderMatch(std::shared_ptr<Order> o);
        void cancelOrder(std::string orderId);
        void fillorkillOrder(std::shared_ptr<Order> o);
        void cancelReplaceOrder(std::string orderId, int quantity, int price);
        void icebergOrder(std::shared_ptr<Order> o);
    };
    
}

#endif