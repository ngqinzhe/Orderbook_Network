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
    public:
        std::vector<std::shared_ptr<Order> > bo;
        std::vector<std::shared_ptr<Order> > so;
        Orderbook();
        ~Orderbook();
        std::vector<std::shared_ptr<Order> > buyOrderbook();
        std::vector<std::shared_ptr<Order> > sellOrderbook();
        void insert(std::shared_ptr<Order> o);
        void print();
    };
    void limitOrderMatch(Orderbook &ob);
    void marketOrderMatch(Orderbook &ob, std::shared_ptr<Order> o);
    void iocOrderMatch(Orderbook &ob, std::shared_ptr<Order> o);
    void cancelOrder(Orderbook &ob, std::string orderId);
    void fillorkillOrder(Orderbook &ob, std::shared_ptr<Order> o);
    void cancelReplaceOrder(Orderbook &ob, std::string orderId, int quantity, int price);
    void icebergOrder(Orderbook &ob, std::shared_ptr<Order> o);
}

#endif