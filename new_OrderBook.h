#ifndef new_orderbook_h
#define new_orderbook_h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <functional>

namespace trading {
    class Order {
    private:
        int price;
        int quantity;
        std::string name;
        std::string type;
        int display;

    public:
        Order(int p, int q, std::string n, std::string t);
        Order(int p, int q, std::string n, std::string t, int dis);

        // ORDER FUNCTIONALITIES
        int getPrice() const {return this->price;}
        int getQuantity() const {return this->quantity;}
        std::string getName() const {return this->name;}
        bool isBuy() const {return this->type == "B";}
        bool isIceberg() const {return this->display != 0;}
        int getDisplay() const {return this->display;}
        void setDisplay(int _display) {this->display = _display;}
        void setQuantity(int _quantity) {this->quantity = _quantity;}
        void deductQuantity(int change) {this->quantity -= change;}
        std::string getID() const;
    };

    class Orderbook {
    private:
        // TYPE DEFINITIONS
        typedef std::multimap<int, Order, std::greater<int>>::iterator buyIter;
        typedef std::multimap<int, Order, std::less<int>>::iterator sellIter;
        typedef std::map<std::string, buyIter> mobIter;
        typedef std::map<std::string, sellIter> mosIter;
        
        // CONTAINERS -> DATA STRUCTURES
        std::multimap<int, Order, std::greater<int>> bo;
        std::multimap<int, Order, std::less<int>> so;
        std::map<std::string, buyIter> mob;
        std::map<std::string, sellIter> mos;

        // HELPER FUNCTIONS
        void FOKBUY(Order& o);
        void FOKSELL(Order& o);
        void replaceHelper(const std::string& orderId, int quantity, int price, bool side);
        void ICEBUY(Order& o);
        void ICESELL(Order& o);
        void ICELIMIT(int bQty, int sQty);

    public:
        Orderbook();
        void insert(Order& o);
        void print() const;
        void limitOrderMatch();
        void marketOrderMatch(Order& o);
        void iocOrderMatch(Order& o);
        std::string cancelOrder(const std::string& orderId);
        void fillorkillOrder(Order& o);
        void cancelReplaceOrder(const std::string& orderId, int quantity, int price);
        void icebergOrder(Order& o);
    };
}
#endif