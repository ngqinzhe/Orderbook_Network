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

void OB::Orderbook::limitOrderMatch() {
    if (this->bo.size() == 0 || this->so.size() == 0) {
        //std::cout << "0" << std::endl;
        return;
    }
    
    int b_price = this->bo[0]->price;
    int s_price = this->so[0]->price;
    if (b_price >= s_price) {
        int b_qty = this->bo[0]->quantity;
        int s_qty = this->so[0]->quantity;
        if (this->so[0]->display != 0) {
            s_qty = this->so[0]->display;
        }
        if (this->bo[0]->display != 0) {
            b_qty = this->bo[0]->display;
        }
        if (b_qty < s_qty) {
            this->bo.erase(this->bo.begin());
            this->so[0]->quantity -= b_qty;
            std::cout << s_price * b_qty << std::endl;
            return;
        } 
        else if (b_qty > s_qty){
            this->so.erase(this->so.begin());
            this->bo[0]->quantity -= s_qty;
            std::cout << s_qty * s_price << std::endl;
            return;
        }
        else {
            this->bo.erase(this->bo.begin());
            this->so.erase(this->so.begin());
        }
    } 
    //std::cout << "0" << std::endl;
}

void OB::Orderbook::marketOrderMatch(std::shared_ptr<OB::Order> o) {
    //check if opposite sides are empty 
    int total = 0;
    while (o->quantity != 0) {
        if ((this->bo.size() == 0 && o->type == "S") || (this->so.size() == 0 && o->type == "B")) break;
        
        int initial_qty = o->quantity;
        int initial_price = o->type == "B" ? this->so[0]->price : this->bo[0]->price;
        
        if (o->type == "B") {
            if (initial_qty < this->so[0]->quantity) {
                this->so[0]->quantity -= initial_qty;
                total += initial_qty * initial_price;
            }
            else {
                int trade_qty = this->so[0]->quantity;
                this->so.erase(this->so.begin());
                total += trade_qty * initial_price;
            }
        }
        else {
            if (initial_qty < this->bo[0]->quantity) {
                this->bo[0]->quantity -= initial_qty;
                total += initial_qty * initial_price;
            }
            else {
                int trade_qty = this->bo[0]->quantity;
                this->bo.erase(this->bo.begin());
                total += trade_qty * initial_price;
            }
        }
    }
    std::cout << "Trade occured for a total of: $" << total << std::endl;
}

void OB::Orderbook::iocOrderMatch(std::shared_ptr<OB::Order> o) {
    if (o->type == "B" && o->price >= this->so[0]->price) {
        if (o->quantity < this->so[0]->quantity) {
            this->so[0]->quantity -= o->quantity;
            std::cout << o->quantity * this->so[0]->price << std::endl;
        }
        else {
            int trade_qty = this->so[0]->quantity;
            int trade_price = this->so[0]->price;
            this->so.erase(this->so.begin());
            std::cout << trade_qty * trade_price << std::endl;
        }
    
    }
    else if (o->type == "S" && o->price <= this->bo[0]->price) {
        if (o->quantity < this->bo[0]->quantity) {
            this->bo[0]->quantity -= o->quantity;
            std::cout << o->quantity * this->bo[0]->price << std::endl;
        }
        else {
            int trade_qty = this->bo[0]->quantity;
            int trade_price = this->bo[0]->price;
            this->bo.erase(this->bo.begin());
            std::cout << trade_qty * trade_price << std::endl;
        }
    
    }
}

void OB::Orderbook::cancelOrder(std::string orderId) {
    for (int i = 0; i < this->bo.size(); i++) {
        if (this->bo[i]->name == orderId) {
            this->bo.erase(this->bo.begin() + i);
            return;
        }
    }

    for (int i = 0; i < this->so.size(); i++) {
        if (this->so[i]->name == orderId) {
            this->so.erase(this->so.begin() + i);
            return;
        }
    }
}

void OB::Orderbook::fillorkillOrder(std::shared_ptr<OB::Order> o) {
    int order_qty = o->quantity;
    if (o->type == "B") {
        if (o->price >= this->so[0]->price) {
            if (o->quantity <= this->so[0]->quantity) {
                int trade_price = this->so[0]->price;
                this->so[0]->quantity -= order_qty;
                if (this->so[0]->quantity == 0) {
                    this->so.erase(this->so.begin());
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
                while (o->price >= this->so[i]->price) {
                    if (this->so[i]->quantity >= remainder) {
                        last_trade = remainder;
                        remainder = 0;
                        break;
                    }
                    remainder -= this->so[i]->quantity;
                    i ++;
                }
                if (remainder != 0) {
                    std::cout << 0 << std::endl;
                    return;
                }
                //remove cleared orders and add qty and price to trade_price
                for (int j = 0; j < i; j++) {
                    trade_price += this->so[0]->price * this->so[0]->quantity;
                    this->so.erase(this->so.begin());
                }
                this->so[0]->quantity -= last_trade;
                trade_price += last_trade * this->so[0]->price;
                std::cout << trade_price << std::endl;
            }
        }
    }
    else {
        if (o->price <= this->bo[0]->price) {
            if (o->quantity <= this->bo[0]->quantity) {
                int trade_price = o->price;
                this->bo[0]->quantity -= order_qty;
                if (this->bo[0]->quantity == 0) {
                    this->bo.erase(this->bo.begin());
                }
                std::cout << order_qty * trade_price << std::endl;
                return;
            } else {
                int i = 0;
                int trade_price = 0;
                int remainder = o->quantity;
                int last_trade = 0;
                //while the order price is higher, we will check how many orders we can clear
                while (o->price <= this->bo[i]->price) {
                    if (this->bo[i]->quantity >= remainder) {
                        last_trade = remainder;
                        remainder = 0;
                        break;
                    }
                    remainder -= this->bo[i]->quantity;
                    i ++;
                }
                if (remainder != 0) {
                    std::cout << 0 << std::endl;
                    return;
                }
                //remove cleared orders and add qty and price to trade_price
                for (int j = 0; j < i; j++) {
                    trade_price += this->bo[0]->price * this->bo[0]->quantity;
                    this->bo.erase(this->bo.begin());
                }
                // delete the quantity
                this->bo[0]->quantity -= last_trade;
                trade_price += this->bo[0]->price * last_trade;
                std::cout << trade_price << std::endl;
            }
        }
    }
}

void OB::Orderbook::cancelReplaceOrder(std::string orderId, int quantity, int price) {
    // need to find the order in both orderbooks since only the name is given
    for (int i = 0; i < this->bo.size(); i++) {
        if (this->bo[i]->name == orderId) {
            if (price >= this->so[0]->price) return;
            int original_price = this->bo[i]->price;
            this->bo.erase(this->bo.begin() + i);
            std::shared_ptr<OB::Order> o = std::make_shared<OB::Order>(price, quantity, orderId, "B");
            if (original_price == price) this->bo.insert(this->bo.begin() + i, o);
            else this->insert(o);
            return;
        }
    }

    //check sell side
    for (int i = 0; i < this->so.size(); i++) {
        if (this->so[i]->name == orderId) {
            if (price <= this->bo[0]->price) return;
            int original_price = this->so[i]->price;
            this->so.erase(this->so.begin() + i);
            std::shared_ptr<OB::Order> o = std::make_shared<OB::Order>(price, quantity, orderId, "B");
            if (original_price == price) this->so.insert(this->so.begin() + i, o);
            else this->insert(o);
            return;
        }
    }
}

void OB::Orderbook::icebergOrder(std::shared_ptr<OB::Order> o) {
    std::string orderId = o->name;
    bool isBuy = o->type == "B" ? true : false;
    this->insert(o);
    
    if (isBuy && this->bo[0]->name == orderId && !this->so.empty() && this->bo[0]->price >= this->so[0]->price) {
        OB::Orderbook::limitOrderMatch();
    }
    else if (!isBuy && this->so[0]->name == orderId && !this->bo.empty() && this->so[0]->price <= this->bo[0]->price) {
        OB::Orderbook::limitOrderMatch();
    }

}

