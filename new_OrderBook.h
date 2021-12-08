#ifndef new_orderbook_h
#define new_orderbook_h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace OB {
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

        // bool operator< (int& x, const Order& rhs) const {return x < rhs.price;}
        // bool operator> (int& x, const Order& rhs) const {return x > rhs.price;}

        // get price
        int getPrice() const {return this->price;}
        int getQuantity() const {return this->quantity;}
        std::string getName() const {return this->name;}
        bool isBuy() const {return this->type == "B";}
        int getDisplay() const {return this->display;}
        void setQuantity(int _quantity) {this->quantity = _quantity;}

        void deductQuantity(int change) {this->quantity -= change;}
        std::string getID() const;
    };

    class Orderbook {
    private:

        std::multimap<int, Order, std::greater<int>> bo;
        std::multimap<int, Order, std::less<int>> so;

        typedef std::multimap<int, Order, std::greater<int>>::iterator buyIter;
        typedef std::multimap<int, Order, std::less<int>>::iterator sellIter;
        
        std::map<std::string, buyIter> mob;
        std::map<std::string, sellIter> mos;

        typedef std::map<std::string, buyIter> mobIter;
        typedef std::map<std::string, sellIter> mosIter;

        void FOKBUY(Order& o);
        void FOKSELL(Order& o);
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
        void replaceHelper(const std::string& orderId, int quantity, int price, bool side);
        void icebergOrder(Order& o);
    };

    

}
#endif