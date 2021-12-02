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

std::string OB::Order::get() {
    std::string ans;
    ans = std::to_string(this->quantity) + '@' + std::to_string(this->price) + '#' + this->name;
    if (this->display != 0) ans = std::to_string(this->display) + "(" + std::to_string(this->quantity) + ")" + '@' + std::to_string(this->price) + '#' + this->name;
    return ans;
}

OB::Orderbook::Orderbook() {
    bo = std::vector<std::shared_ptr<OB::Order> >();
    so = std::vector<std::shared_ptr<OB::Order> >();
}

OB::Orderbook::~Orderbook() {}

void OB::Orderbook::insert(std::shared_ptr<OB::Order> o) {
    if (o->type == "B") {
        bo.push_back(o);
        std::sort(bo.begin(), bo.end(), [&](std::shared_ptr<OB::Order> o1, std::shared_ptr<OB::Order> o2) 
        {
            return o1->price > o2->price;
        });
    }
    else {
        so.push_back(o);
        std::sort(so.begin(), so.end(), [&](std::shared_ptr<OB::Order> o1, std::shared_ptr<OB::Order> o2) 
        {
            return o1->price < o2->price;
        });
    }
}

void OB::Orderbook::print() {
    std::cout << "B: ";
    for (std::vector<std::shared_ptr<OB::Order> >::const_iterator it = bo.begin(); it != bo.end(); it++) {
        std::cout << (*it)->get() << " "; 
    }
    std::cout << std::endl;
    std::cout << "S: ";
    for (std::vector<std::shared_ptr<OB::Order> >::const_iterator it = so.begin(); it != so.end(); it++) {
        std::cout << (*it)->get() << " "; 
    }
    std::cout << std::endl; 
}

void OB::limitOrderMatch(OB::Orderbook &ob) {
    if (ob.bo.size() == 0 || ob.so.size() == 0) {
        //std::cout << "0" << std::endl;
        return;
    }
    
    int b_price = ob.bo[0]->price;
    int s_price = ob.so[0]->price;
    if (b_price >= s_price) {
        int b_qty = ob.bo[0]->quantity;
        int s_qty = ob.so[0]->quantity;
        if (ob.so[0]->display != 0) {
            s_qty = ob.so[0]->display;
        }
        if (ob.bo[0]->display != 0) {
            b_qty = ob.bo[0]->display;
        }
        if (b_qty < s_qty) {
            ob.bo.erase(ob.bo.begin());
            ob.so[0]->quantity -= b_qty;
            std::cout << s_price * b_qty << std::endl;
            return;
        } 
        else if (b_qty > s_qty){
            ob.so.erase(ob.so.begin());
            ob.bo[0]->quantity -= s_qty;
            std::cout << s_qty * s_price << std::endl;
            return;
        }
        else {
            ob.bo.erase(ob.bo.begin());
            ob.so.erase(ob.so.begin());
        }
    } 
    //std::cout << "0" << std::endl;
}

void OB::marketOrderMatch(OB::Orderbook &ob, std::shared_ptr<OB::Order> o) {
    //check if opposite sides are empty 
    int total = 0;
    while (o->quantity != 0) {
        if ((ob.bo.size() == 0 && o->type == "S") || (ob.so.size() == 0 && o->type == "B")) break;
        
        int initial_qty = o->quantity;
        int initial_price = o->type == "B" ? ob.so[0]->price : ob.bo[0]->price;
        
        if (o->type == "B") {
            if (initial_qty < ob.so[0]->quantity) {
                ob.so[0]->quantity -= initial_qty;
                total += initial_qty * initial_price;
            }
            else {
                int trade_qty = ob.so[0]->quantity;
                ob.so.erase(ob.so.begin());
                total += trade_qty * initial_price;
            }
        }
        else {
            if (initial_qty < ob.bo[0]->quantity) {
                ob.bo[0]->quantity -= initial_qty;
                total += initial_qty * initial_price;
            }
            else {
                int trade_qty = ob.bo[0]->quantity;
                ob.bo.erase(ob.bo.begin());
                total += trade_qty * initial_price;
            }
        }
    }
    std::cout << "Trade occured for a total of: $" << total << std::endl;
}

void OB::iocOrderMatch(OB::Orderbook &ob, std::shared_ptr<OB::Order> o) {
    if (o->type == "B" && o->price >= ob.so[0]->price) {
        if (o->quantity < ob.so[0]->quantity) {
            ob.so[0]->quantity -= o->quantity;
            std::cout << o->quantity * ob.so[0]->price << std::endl;
        }
        else {
            int trade_qty = ob.so[0]->quantity;
            int trade_price = ob.so[0]->price;
            ob.so.erase(ob.so.begin());
            std::cout << trade_qty * trade_price << std::endl;
        }
    
    }
    else if (o->type == "S" && o->price <= ob.bo[0]->price) {
        if (o->quantity < ob.bo[0]->quantity) {
            ob.bo[0]->quantity -= o->quantity;
            std::cout << o->quantity * ob.bo[0]->price << std::endl;
        }
        else {
            int trade_qty = ob.bo[0]->quantity;
            int trade_price = ob.bo[0]->price;
            ob.bo.erase(ob.bo.begin());
            std::cout << trade_qty * trade_price << std::endl;
        }
    
    }
}

void OB::cancelOrder(OB::Orderbook &ob, std::string orderId) {
    for (int i = 0; i < ob.bo.size(); i++) {
        if (ob.bo[i]->name == orderId) {
            ob.bo.erase(ob.bo.begin() + i);
            return;
        }
    }

    for (int i = 0; i < ob.so.size(); i++) {
        if (ob.so[i]->name == orderId) {
            ob.so.erase(ob.so.begin() + i);
            return;
        }
    }
}

void OB::fillorkillOrder(OB::Orderbook &ob, std::shared_ptr<OB::Order> o) {
    int order_qty = o->quantity;
    if (o->type == "B") {
        if (o->price >= ob.so[0]->price) {
            if (o->quantity <= ob.so[0]->quantity) {
                int trade_price = ob.so[0]->price;
                ob.so[0]->quantity -= order_qty;
                if (ob.so[0]->quantity == 0) {
                    ob.so.erase(ob.so.begin());
                }
                std::cout << order_qty * trade_price << std::endl;
                return;
            } else {
                int i = 0;
                int trade_price = 0;
                int remainder = o->quantity;
                int last_trade = 0;
                //while the order price is higher, we will check how many orders we can clear
                //need to make sure remainder can be cleared
                while (o->price >= ob.so[i]->price) {
                    if (ob.so[i]->quantity >= remainder) {
                        last_trade = remainder;
                        remainder = 0;
                        break;
                    }
                    remainder -= ob.so[i]->quantity;
                    i ++;
                }
                if (remainder != 0) {
                    std::cout << 0 << std::endl;
                    return;
                }
                //remove cleared orders and add qty and price to trade_price
                for (int j = 0; j < i; j++) {
                    trade_price += ob.so[0]->price * ob.so[0]->quantity;
                    ob.so.erase(ob.so.begin());
                }
                ob.so[0]->quantity -= last_trade;
                trade_price += last_trade * ob.so[0]->price;
                std::cout << trade_price << std::endl;
            }
        }
    }
    else {
        if (o->price <= ob.bo[0]->price) {
            if (o->quantity <= ob.bo[0]->quantity) {
                int trade_price = o->price;
                ob.bo[0]->quantity -= order_qty;
                if (ob.bo[0]->quantity == 0) {
                    ob.bo.erase(ob.bo.begin());
                }
                std::cout << order_qty * trade_price << std::endl;
                return;
            } else {
                int i = 0;
                int trade_price = 0;
                int remainder = o->quantity;
                int last_trade = 0;
                //while the order price is higher, we will check how many orders we can clear
                while (o->price <= ob.bo[i]->price) {
                    if (ob.bo[i]->quantity >= remainder) {
                        last_trade = remainder;
                        remainder = 0;
                        break;
                    }
                    remainder -= ob.bo[i]->quantity;
                    i ++;
                }
                if (remainder != 0) {
                    std::cout << 0 << std::endl;
                    return;
                }
                //remove cleared orders and add qty and price to trade_price
                for (int j = 0; j < i; j++) {
                    trade_price += ob.bo[0]->price * ob.bo[0]->quantity;
                    ob.bo.erase(ob.bo.begin());
                }
                // delete the quantity
                ob.bo[0]->quantity -= last_trade;
                trade_price += ob.bo[0]->price * last_trade;
                std::cout << trade_price << std::endl;
            }
        }
    }
}

void OB::cancelReplaceOrder(OB::Orderbook &ob, std::string orderId, int quantity, int price) {
    // need to find the order in both orderbooks since only the name is given
    for (int i = 0; i < ob.bo.size(); i++) {
        if (ob.bo[i]->name == orderId) {
            if (price >= ob.so[0]->price) return;
            int original_price = ob.bo[i]->price;
            ob.bo.erase(ob.bo.begin() + i);
            std::shared_ptr<OB::Order> o = std::make_shared<OB::Order>(price, quantity, orderId, "B");
            if (original_price == price) ob.bo.insert(ob.bo.begin() + i, o);
            else ob.insert(o);
            return;
        }
    }

    //check sell side
    for (int i = 0; i < ob.so.size(); i++) {
        if (ob.so[i]->name == orderId) {
            if (price <= ob.bo[0]->price) return;
            int original_price = ob.so[i]->price;
            ob.so.erase(ob.so.begin() + i);
            std::shared_ptr<OB::Order> o = std::make_shared<OB::Order>(price, quantity, orderId, "B");
            if (original_price == price) ob.so.insert(ob.so.begin() + i, o);
            else ob.insert(o);
            return;
        }
    }
}

void OB::icebergOrder(OB::Orderbook &ob, std::shared_ptr<OB::Order> o) {
    std::string orderId = o->name;
    bool isBuy = o->type == "B" ? true : false;
    ob.insert(o);
    
    if (isBuy && ob.bo[0]->name == orderId && !ob.so.empty() && ob.bo[0]->price >= ob.so[0]->price) {
        OB::limitOrderMatch(ob);
    }
    else if (!isBuy && ob.so[0]->name == orderId && !ob.bo.empty() && ob.so[0]->price <= ob.bo[0]->price) {
        OB::limitOrderMatch(ob);
    }

}

