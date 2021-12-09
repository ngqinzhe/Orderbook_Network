#include "new_OrderBook.h"


//standard orders
trading::Order::Order(int p, int q, std::string n, std::string t) : price(p), quantity(q), name(n), type(t), display(0) {}
//iceberg order
trading::Order::Order(int p, int q, std::string n, std::string t, int dis) : price(p), quantity(q), name(n), type(t), display(dis) {}

std::string trading::Order::getID() const {
    std::string ans;
    ans = std::to_string(this->quantity) + '@' + std::to_string(this->price) + '#' + this->name;
    if (this->display != 0) ans = std::to_string(this->display) + "(" + std::to_string(this->quantity) + ")" + '@' + std::to_string(this->price) + '#' + this->name;
    return ans;
}

trading::Orderbook::Orderbook() {
    bo = std::multimap<int, Order, std::greater<int>>();
    so = std::multimap<int, Order, std::less<int>>();
    mob = std::map<std::string, buyIter>();
    mos = std::map<std::string, sellIter>();
}


void trading::Orderbook::insert(Order& o) {
    // o.isBuy() == 1 if it is a "B" order
    int orderPrice = o.getPrice();
    std::string orderName = o.getName();
    // place an iterator in another map, to ensure o(1) deletion of orders, and access of orders is o(1)
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

void trading::Orderbook::print() const {
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

void trading::Orderbook::limitOrderMatch() {
    if (this->bo.size() == 0 || this->so.size() == 0) {
        std::cout << "0" << std::endl;
        return;
    }
    bool buyIsICE = false;
    bool sellIsICE = false;
    int b_price = this->bo.begin()->second.getPrice();
    int s_price = this->so.begin()->second.getPrice();
    // Order matching occurs when there is a buy price higher than OR equal to selling price
    if (b_price >= s_price) {
        int b_qty = this->bo.begin()->second.getQuantity();
        int s_qty = this->so.begin()->second.getQuantity();

        // IF the orders have a display price, ICE Order
        if (this->so.begin()->second.getDisplay() != 0) {
            s_qty = this->so.begin()->second.getDisplay();
            sellIsICE = true;
        }
        if (this->bo.begin()->second.getDisplay() != 0) {
            b_qty = this->bo.begin()->second.getDisplay();
            buyIsICE = true;
        }

        // MATCH THE LIMIT ORDERS

        // if buy quantity is less than sell quantity, then we erase all buy quantity
        if (b_qty < s_qty) {
            if (!buyIsICE) this->bo.erase(this->bo.begin());
            else {
                ICELIMIT(b_qty, s_qty);
            }
            
            this->so.begin()->second.deductQuantity(b_qty);
            std::cout << s_price * b_qty << std::endl;
            return;
        } 
        // if sell quantity is less than buy quantity, we erase all sell quantity
        else if (b_qty > s_qty){
            if (!sellIsICE) this->so.erase(so.begin());
            else {
                ICELIMIT(b_qty, s_qty);
            }
            this->bo.begin()->second.deductQuantity(s_qty);
            std::cout << s_qty * s_price << std::endl;
            return;
        }
        // IF Sell Quantity == Buy Quantity
        else {
            if (!buyIsICE && !sellIsICE) {
                this->bo.erase(bo.begin());
                this->so.erase(so.begin());
            }
            else if (buyIsICE) {
                this->bo.begin()->second.deductQuantity(b_qty);
                if (!this->bo.begin()->second.getQuantity()) this->bo.erase(this->bo.begin());
                this->so.erase(so.begin());
            }
            else if (sellIsICE) {
                this->so.begin()->second.deductQuantity(s_qty);
                if (!this->so.begin()->second.getQuantity()) this->so.erase(this->so.begin());
                this->bo.erase(bo.begin());
            }
        }
    } 
    std::cout << "0" << std::endl;
}

void trading::Orderbook::ICELIMIT(int bQty, int sQty) {
    if (bQty < sQty) {
        this->bo.begin()->second.deductQuantity(bQty);
        // check if the quantity is less than display
        if (this->bo.begin()->second.getQuantity() < this->bo.begin()->second.getDisplay()){
            // set the display amount to total quantity
            if (!this->bo.begin()->second.getQuantity()) this->bo.erase(this->bo.begin());
            else this->bo.begin()->second.setDisplay(this->bo.begin()->second.getQuantity());
        }
    }
    else if (bQty > sQty) {
        this->so.begin()->second.deductQuantity(sQty);
            // check if the quantity is less than display
            if (this->so.begin()->second.getQuantity() < this->so.begin()->second.getDisplay()){
                // set the display amount to total quantity
                if (!this->so.begin()->second.getQuantity()) this->so.erase(this->so.begin());
                else this->so.begin()->second.setDisplay(this->so.begin()->second.getQuantity());
            }
    }
}

void trading::Orderbook::marketOrderMatch(Order& o) {
    //check if opposite sides are empty 
    int total = 0;
    //store used ICE ORDERS, matched ICE orders will be added in lower priority
    std::vector<trading::Order> checkedOrders;

    // we will try to fill all the quantity available in this market order
    while (o.getQuantity() != 0) {
        // no orders to fill, market order ENDS
        if ((this->bo.size() == 0 && !o.isBuy()) || (this->so.size() == 0 && o.isBuy())) break;
        
        int initial_qty = o.getQuantity();
        int initial_price = o.isBuy() ? this->so.begin()->second.getPrice() : this->bo.begin()->second.getPrice();
        
        // if it is a MARKET BUY ORDER
        if (o.isBuy()) {
            int trade_qty = this->bo.begin()->second.isIceberg() ? this->bo.begin()->second.getDisplay() : this->bo.begin()->second.getQuantity();
            // if order quantity is less than the top SELL ORDER
            if (initial_qty < trade_qty) {
                this->so.begin()->second.deductQuantity(initial_qty);
                total += initial_qty * initial_price;
            }
            // if order quantity is more than the top SELL ORDER, deduct then loop through again
            else {
                // add ice order to vector for insert later, deduct the display qty
                if (this->so.begin()->second.isIceberg()) {
                    this->so.begin()->second.deductQuantity(trade_qty);
                    checkedOrders.push_back(this->so.begin()->second);
                }
                this->so.erase(so.begin());
                total += trade_qty * initial_price;
            }
        }
        // if it is MARKET SELL ORDER
        else {
            int trade_qty = this->bo.begin()->second.isIceberg() ? this->bo.begin()->second.getDisplay() : this->bo.begin()->second.getQuantity();
            // if order quantity is less than the top BUY ORDER
            if (initial_qty < trade_qty) {
                this->bo.begin()->second.deductQuantity(initial_qty);
                o.deductQuantity(initial_qty);
                total += initial_qty * initial_price;
            }
            // if order quantity is more than or equal to the top BUY ORDER, deduct then loop through again
            else {
                if (this->bo.begin()->second.isIceberg()) {
                    this->bo.begin()->second.deductQuantity(trade_qty);
                    checkedOrders.push_back(this->bo.begin()->second);
                }
                o.deductQuantity(trade_qty);
                this->bo.erase(bo.begin());
                total += trade_qty * initial_price;
            }
        }
    }
    // insert those removed orders
    for (auto& orders : checkedOrders) 
        this->insert(orders);
    std::cout << total << std::endl;
}

std::string trading::Orderbook::cancelOrder(const std::string& orderId) {
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

void trading::Orderbook::iocOrderMatch(Order& o) {
    // if this is a BUY ORDER, check if the price is higher than the top SELL ORDER
    std::vector<trading::Order> checkedOrders;
    int totalSum = 0;
    if (o.isBuy()) {
        while (o.getPrice() >= this->so.begin()->second.getPrice() && this->so.size() != 0) {
            int tradeQty = this->so.begin()->second.isIceberg() ? this->so.begin()->second.getDisplay() : this->so.begin()->second.getQuantity();
            // if order quantity is less than top SELL ORDER quantity, fill all the order
            if (o.getQuantity() < tradeQty) {
                this->so.begin()->second.deductQuantity(o.getQuantity());
                totalSum += tradeQty * this->so.begin()->second.getPrice();
                std::cout << totalSum << std::endl;
                return;
            }
            // if order quantity is more than top SELL ORDER quantity, remove the current top SELL ORDER
            else {
                if (this->so.begin()->second.isIceberg()) {
                    this->so.begin()->second.deductQuantity(tradeQty);
                    checkedOrders.push_back(this->so.begin()->second);
                }
                o.deductQuantity(tradeQty);
                totalSum += tradeQty * this->so.begin()->second.getPrice();
                this->so.erase(so.begin());
            }
        }
    }
    // if this is a SELL ORDER
    else if (!o.isBuy()) {
        // if order quantity is less than top BUY ORDER quantity, fill all the order
        while (o.getPrice() <= this->bo.begin()->second.getPrice() && this->bo.size() != 0) {
            int tradeQty = this->bo.begin()->second.isIceberg() ? this->bo.begin()->second.getDisplay() : this->bo.begin()->second.getQuantity();
            // if order quantity is less than top SELL ORDER quantity, fill all the order
            if (o.getQuantity() < tradeQty) {
                this->bo.begin()->second.deductQuantity(o.getQuantity());
                totalSum += tradeQty * this->bo.begin()->second.getPrice();
                std::cout << totalSum << std::endl;
                return;
            }
            // if order quantity is more than top SELL ORDER quantity, remove the current top SELL ORDER
            else {
                if (this->bo.begin()->second.isIceberg()) {
                    this->bo.begin()->second.deductQuantity(tradeQty);
                    checkedOrders.push_back(this->bo.begin()->second);
                }
                o.deductQuantity(tradeQty);
                totalSum += tradeQty * this->bo.begin()->second.getPrice();
                this->bo.erase(bo.begin());
            }
        }
    }
    for (auto& orders : checkedOrders)
        this->insert(orders);
    std::cout << totalSum << std::endl;
}

void trading::Orderbook::fillorkillOrder(Order& o) {
    if (o.isBuy()) FOKBUY(o);
    else FOKSELL(o);
}

void trading::Orderbook::FOKBUY(Order& o) {
    int orderPrice = o.getPrice();
    int totalSum = 0;
    int orderInitialQuantity = o.getQuantity();
    std::vector<trading::Order> checkedOrders;
    // check whether full quantity can be fufilled
    for (auto it = this->so.begin(); it != so.end(); it++) {
        int tradeQty = this->so.begin()->second.isIceberg() ? this->so.begin()->second.getDisplay() : this->so.begin()->second.getQuantity();
        if (it->second.getPrice() <= orderPrice) orderInitialQuantity -= tradeQty;
    }
    if (orderInitialQuantity > 0) {
        std::cout << "0" << std::endl;
        return;
    }
    // BUY PRICE must be more than the current SELL ORDER BOOK price, else order will just kill
    while (orderPrice >= this->so.begin()->second.getPrice()) {
        // if the order quantity < orderbook quantity, then deduct and end
        int tradeQty = this->so.begin()->second.isIceberg() ? this->so.begin()->second.getDisplay() : this->so.begin()->second.getQuantity();
        if (o.getQuantity() < tradeQty) {
            totalSum += o.getQuantity() * this->so.begin()->second.getPrice();
            this->so.begin()->second.deductQuantity(o.getQuantity());
            break;
        } 
        // order quantity is larger than orderbook quantity
        else {
            totalSum += tradeQty * this->so.begin()->second.getPrice();
            o.deductQuantity(tradeQty);
            if (this->so.begin()->second.isIceberg()) {
                this->so.begin()->second.deductQuantity(tradeQty);
                checkedOrders.push_back(this->so.begin()->second);
            }
            this->so.erase(this->so.begin());
            if (o.getQuantity() == 0) break;
        }
    }
    std::cout << totalSum << std::endl;
}

void trading::Orderbook::FOKSELL(Order& o) {
    int orderPrice = o.getPrice();
    int totalSum = 0;
    int orderInitialQuantity = o.getQuantity();
    std::vector<trading::Order> checkedOrders;
    // check whether full quantity can be fufilled
    for (auto it = this->bo.begin(); it != bo.end(); it++) {
        int tradeQty = this->bo.begin()->second.isIceberg() ? this->bo.begin()->second.getDisplay() : this->bo.begin()->second.getQuantity();
        if (it->second.getPrice() >= orderPrice) orderInitialQuantity -= tradeQty;
    }
    if (orderInitialQuantity > 0) {
        std::cout << "0" << std::endl;
        return;
    }

    // SELL PRICE must be less than the current BUY ORDER BOOK price, else order will just kill
    while (orderPrice <= this->bo.begin()->second.getPrice()) {
        // if the order quantity < orderbook quantity, then deduct and end
        int tradeQty = this->bo.begin()->second.isIceberg() ? this->bo.begin()->second.getDisplay() : this->bo.begin()->second.getQuantity();
        if (o.getQuantity() < tradeQty) {
            totalSum += o.getQuantity() * this->bo.begin()->second.getPrice();
            this->bo.begin()->second.deductQuantity(o.getQuantity());
            break;
        } 
        // order quantity is larger than orderbook quantity
        else {
            totalSum += tradeQty * this->bo.begin()->second.getPrice();
            o.deductQuantity(tradeQty);
            if (this->bo.begin()->second.isIceberg()) {
                this->bo.begin()->second.deductQuantity(tradeQty);
                checkedOrders.push_back(this->bo.begin()->second);
            }
            this->bo.erase(this->bo.begin());
            if (o.getQuantity() == 0) break;
        }
    }

    for (auto& orders : checkedOrders) 
        this->insert(orders);
    std::cout << totalSum << std::endl;
}

void trading::Orderbook::cancelReplaceOrder(const std::string& orderId, int quantity, int price) {
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

void trading::Orderbook::replaceHelper(const std::string& orderId, int quantity, int price, bool side) {
    auto it = side ? this->mob.at(orderId) : this->mos.at(orderId);    
        // if no price change, don't cancel
    if (it->second.getPrice() == price) {
        it->second.setQuantity(quantity);
        return;
    }
    // cancel then insert
    else {
        std::string side = this->cancelOrder(orderId);
        trading::Order order(price, quantity, orderId, side);
        std::pair<int, Order> newOrder = std::pair<int, Order>(price, order);
        this->insert(order);
        return;
    }
}

void trading::Orderbook::icebergOrder(Order& o) {
    if (o.isBuy()) ICEBUY(o);
    else ICESELL(o);
}

void trading::Orderbook::ICEBUY(Order& o) {
    int orderPrice = o.getPrice();
    int totalSum = 0;
    while (orderPrice >= this->so.begin()->second.getPrice() && this->so.size() != 0) {
        // if the order quantity < orderbook quantity, then deduct and end
        if (o.getQuantity() < this->so.begin()->second.getQuantity()) {
            totalSum += o.getQuantity() * this->so.begin()->second.getPrice();
            this->so.begin()->second.deductQuantity(o.getQuantity());
            // order quantity is completely gone
            o.deductQuantity(o.getQuantity());
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
    if (o.getQuantity()) this->insert(o);
}

void trading::Orderbook::ICESELL(Order& o) {
    int orderPrice = o.getPrice();
    int totalSum = 0;
    while (orderPrice <= this->bo.begin()->second.getPrice() && this->bo.size() != 0) {
        // if the order quantity < orderbook quantity, then deduct and end
        if (o.getQuantity() < this->bo.begin()->second.getQuantity()) {
            totalSum += o.getQuantity() * this->bo.begin()->second.getPrice();
            this->bo.begin()->second.deductQuantity(o.getQuantity());
            // order quantity is completely gone
            o.deductQuantity(o.getQuantity());
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
    if (o.getQuantity()) this->insert(o);
}