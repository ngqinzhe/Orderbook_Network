#include <algorithm>
#include <iostream>
#include <sstream>
#include "new_OrderBook.h"

//standard orders
OB::Order::Order(int p, int q, std::string n, std::string t) {
    this->price = p;
    this->quantity = q;
    this->name = n;
    this->type = t;
    this->display = 0;
}
//iceberg order
OB::Order::Order(int p, int q, std::string n, std::string t, int dis) {
    this->price = p;
    this->quantity = q;
    this->name = n;
    this->type = t;
    this->display = dis;
}

std::string OB::Order::getID() const {
    std::string ans;
    ans = std::to_string(this->quantity) + '@' + std::to_string(this->price) + '#' + this->name;
    if (this->display != 0) ans = std::to_string(this->display) + "(" + std::to_string(this->quantity) + ")" + '@' + std::to_string(this->price) + '#' + this->name;
    return ans;
}

OB::Orderbook::Orderbook() {
    bo = std::multimap<int, Order, std::greater<int>>();
    so = std::multimap<int, Order, std::less<int>>();
    mob = std::map<std::string, buyIter>();
    mos = std::map<std::string, sellIter>();
}


void OB::Orderbook::insert(Order& o) {
    // o.isBuy() == 1 if it is a "B" order
    int orderPrice = o.getPrice();
    std::string orderName = o.getName();
    // place an iterator in another map, to ensure o(1) deletion of orders
    if (o.isBuy()) {
        this->bo.insert(std::pair<int, Order>(orderPrice, o));
        auto it = this->bo.find(orderPrice);
        while (it->second.getName() != orderName) it++;
        mob.insert(std::pair<std::string, buyIter>(orderName, it));
    }
    else {
        this->so.insert(std::pair<int, Order>(orderPrice, o));
        auto it = this->so.find(orderPrice);
        while (it->second.getName() != orderName) it++;
        mob.insert(std::pair<std::string, sellIter>(orderName, it));
    }
}

void OB::Orderbook::print() const {
    std::cout << "B: ";
    for (std::multimap<int, Order>::const_iterator it = this->bo.begin(); it != this->bo.end(); it++) {
        std::cout << it->second.getID() << " "; 
    }
    std::cout << std::endl;
    std::cout << "S: ";
    for (std::multimap<int, Order>::const_iterator it = this->so.begin(); it != this->so.end(); it++) {
        std::cout << it->second.getID() << " "; 
    }
    std::cout << std::endl; 
}

void OB::Orderbook::limitOrderMatch() {
    if (this->bo.size() == 0 || this->so.size() == 0) {
        std::cout << "0" << std::endl;
        return;
    }
    
    int b_price = this->bo.begin()->second.getPrice();
    int s_price = this->so.begin()->second.getPrice();
    // Order matching occurs when there is a buy price higher than OR equal to selling price
    if (b_price >= s_price) {
        int b_qty = this->bo.begin()->second.getQuantity();
        int s_qty = this->so.begin()->second.getQuantity();

        // IF the orders have a display price, ICE Order
        if (this->so.begin()->second.getDisplay() != 0) {
            s_qty = this->so.begin()->second.getDisplay();
        }
        if (this->bo.begin()->second.getDisplay() != 0) {
            b_qty = this->bo.begin()->second.getDisplay();
        }

        // MATCH THE LIMIT ORDERS

        // if buy quantity is less than sell quantity, then we erase all buy quantity
        if (b_qty < s_qty) {
            this->bo.erase(this->bo.begin());
            
            this->so.begin()->second.deductQuantity(b_qty);
            std::cout << s_price * b_qty << std::endl;
            return;
        } 
        // if sell quantity is less than buy quantity, we erase all sell quantity
        else if (b_qty > s_qty){
            this->so.erase(so.begin());
            this->bo.begin()->second.deductQuantity(s_qty);
            std::cout << s_qty * s_price << std::endl;
        }
        // IF Sell Quantity == Buy Quantity
        else {
            this->bo.erase(bo.begin());
            this->so.erase(so.begin());
        }
    } 
    std::cout << "0" << std::endl;
}

void OB::Orderbook::marketOrderMatch(Order& o) {
    //check if opposite sides are empty 
    int total = 0;

    // we will try to fill all the quantity available in this market order
    while (o.getQuantity() != 0) {
        // no orders to fill, market order ENDS
        if ((this->bo.size() == 0 && !o.isBuy()) || (this->so.size() == 0 && o.isBuy())) break;
        
        int initial_qty = o.getQuantity();
        int initial_price = o.isBuy() ? this->so.begin()->second.getPrice() : this->bo.begin()->second.getPrice();
        
        // if it is a MARKET BUY ORDER
        if (o.isBuy()) {
            // if order quantity is less than the top SELL ORDER
            if (initial_qty < this->so.begin()->second.getQuantity()) {
                this->so.begin()->second.deductQuantity(initial_qty);
                total += initial_qty * initial_price;
            }
            // if order quantity is more than the top SELL ORDER, deduct then loop through again
            else {
                int trade_qty = this->so.begin()->second.getQuantity();
                this->so.erase(so.begin());
                total += trade_qty * initial_price;
            }
        }
        // if it is MARKET SELL ORDER
        else {
            // if order quantity is less than the top BUY ORDER
            if (initial_qty < this->bo.begin()->second.getQuantity()) {
                this->bo.begin()->second.deductQuantity(initial_qty);
                total += initial_qty * initial_price;
            }
            // if order quantity is more than the top BUY ORDER, deduct then loop through again
            else {
                int trade_qty = this->bo.begin()->second.getQuantity();
                this->bo.erase(bo.begin());
                total += trade_qty * initial_price;
            }
        }
    }
    std::cout << total << std::endl;
}

std::string OB::Orderbook::cancelOrder(const std::string& orderId) {
    if (this->mob.find(orderId) == this->mob.end() && this->mos.find(orderId) == this->mos.end()) {
        std::cout << "No existing match for order: " << orderId << " to be canceled." << std::endl;
        return "";
    }
    //check if it is in buy
    if (this->mob.find(orderId) != this->mob.end()) {
        auto it = this->mob.at(orderId);
        this->bo.erase(it);
        return "B";
    }
    else {
        auto it = this->mos.at(orderId);
        this->so.erase(it);
        return "S";
    }
}

void OB::Orderbook::iocOrderMatch(Order& o) {
    // if this is a BUY ORDER, check if the price is higher than the top SELL ORDER
    if (o.isBuy() && o.getPrice() >= this->so.begin()->second.getPrice()) {
        // if order quantity is less than top SELL ORDER quantity, fill all the order
        if (o.getQuantity() < this->so.begin()->second.getQuantity()) {
            this->so.begin()->second.deductQuantity(o.getQuantity());
            std::cout << o.getQuantity() * this->so.begin()->second.getPrice() << std::endl;
        }
        // if order quantity is more than top SELL ORDER quantity, remove the current top SELL ORDER
        else {
            int trade_qty = this->so.begin()->second.getQuantity();
            int trade_price = this->so.begin()->second.getPrice();
            this->so.erase(so.begin());
            std::cout << trade_qty * trade_price << std::endl;
        }
    
    }
    // if this is a SELL ORDER
    else if (!o.isBuy() && o.getPrice() <= this->bo.begin()->second.getPrice()) {
        // if order quantity is less than top BUY ORDER quantity, fill all the order
        if (o.getQuantity() < this->bo.begin()->second.getQuantity()) {
            this->bo.begin()->second.deductQuantity(o.getQuantity());
            std::cout << o.getQuantity() * this->bo.begin()->second.getPrice() << std::endl;
        }
        else {
            int trade_qty = this->bo.begin()->second.getQuantity();
            int trade_price = this->bo.begin()->second.getPrice();
            this->bo.erase(bo.begin());
            std::cout << trade_qty * trade_price << std::endl;
        }
    
    }
}

void OB::Orderbook::fillorkillOrder(Order& o) {
    if (o.isBuy()) FOKBUY(o);
    else FOKSELL(o);
}

void OB::Orderbook::FOKBUY(Order& o) {
    int orderPrice = o.getPrice();
    int totalSum = 0;
    int orderInitialQuantity = o.getQuantity();
    // check whether full quantity can be fufilled
    for (auto it = this->so.begin(); it != so.end(); it++) {
        if (it->second.getPrice() <= orderPrice) orderInitialQuantity -= it->second.getQuantity();
    }
    if (orderInitialQuantity > 0) {
        std::cout << "0" << std::endl;
        return;
    }
    // BUY PRICE must be more than the current SELL ORDER BOOK price, else order will just kill
    while (orderPrice >= this->so.begin()->second.getPrice()) {
        // if the order quantity < orderbook quantity, then deduct and end
        if (o.getQuantity() < this->so.begin()->second.getQuantity()) {
            totalSum += o.getQuantity() * this->so.begin()->second.getPrice();
            this->so.begin()->second.deductQuantity(o.getQuantity());
            break;
        } 
        // order quantity is larger than orderbook quantity
        else {
            totalSum += this->so.begin()->second.getQuantity() * this->so.begin()->second.getPrice();
            o.deductQuantity(this->so.begin()->second.getQuantity());
            this->so.erase(this->so.begin());
            if (o.getQuantity() == 0) break;
        }
    }
    std::cout << totalSum << std::endl;
}

void OB::Orderbook::FOKSELL(Order& o) {
    int orderPrice = o.getPrice();
    int totalSum = 0;
    int orderInitialQuantity = o.getQuantity();
    // check whether full quantity can be fufilled
    for (auto it = this->bo.begin(); it != bo.end(); it++) {
        if (it->second.getPrice() >= orderPrice) orderInitialQuantity -= it->second.getQuantity();
    }
    if (orderInitialQuantity > 0) {
        std::cout << "0" << std::endl;
        return;
    }

    // SELL PRICE must be less than the current BUY ORDER BOOK price, else order will just kill
    while (orderPrice <= this->bo.begin()->second.getPrice()) {
        // if the order quantity < orderbook quantity, then deduct and end
        if (o.getQuantity() < this->bo.begin()->second.getQuantity()) {
            totalSum += o.getQuantity() * this->bo.begin()->second.getPrice();
            this->bo.begin()->second.deductQuantity(o.getQuantity());
            break;
        } 
        // order quantity is larger than orderbook quantity
        else {
            totalSum += this->bo.begin()->second.getQuantity() * this->bo.begin()->second.getPrice();
            o.deductQuantity(this->bo.begin()->second.getQuantity());
            this->bo.erase(this->bo.begin());
            if (o.getQuantity() == 0) break;
        }
    }
    std::cout << totalSum << std::endl;
}

void OB::Orderbook::cancelReplaceOrder(const std::string& orderId, int quantity, int price) {
    if (this->mob.find(orderId) == this->mob.end() && this->mos.find(orderId) == this->mos.end()) {
        std::cout << "No existing match for order: " << orderId << " to be replaced." << std::endl;
        return;
    }

    //check if it is in buy
    if (this->mob.find(orderId) != this->mob.end()) {
        replaceHelper(orderId, quantity, price, 1);
    }
    // if it is in sell
    else {
        replaceHelper(orderId, quantity, price, 0);
    }
}

void OB::Orderbook::replaceHelper(const std::string& orderId, int quantity, int price, bool side) {
    auto it = side ? this->mob.at(orderId) : this->mos.at(orderId);    
        // if no price change, don't cancel
    if (it->second.getPrice() == price) {
        it->second.setQuantity(quantity);
        return;
    }
    // cancel then insert
    else {
        std::string side = this->cancelOrder(orderId);
        OB::Order order(price, quantity, orderId, side);
        std::pair<int, Order> newOrder = std::pair<int, Order>(price, order);
        this->insert(order);
        return;
    }
}

void OB::Orderbook::icebergOrder(Order& o) {
    std::string orderId = o.getName();
    this->insert(o);

    int orderPrice = o.getPrice();
    
    if (o.isBuy() && this->bo.begin()->second.getName() == orderId && this->so.size() != 0 && orderPrice >= this->so.begin()->second.getPrice()) {
        limitOrderMatch();
    }
    else if (!o.isBuy() && this->so.begin()->second.getName() == orderId && this->bo.size() != 0 && orderPrice <= this->bo.begin()->second.getPrice()) {
        limitOrderMatch();
    }

}

