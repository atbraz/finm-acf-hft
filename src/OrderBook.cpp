#include "OrderBook.hpp"

#include <algorithm>

OrderBook::OrderBook() = default;

#if HFT_BOOK_IMPL_FLAT

void OrderBook::insert_sorted(std::vector<Level>& side, OrderT* o, bool descending) {
    auto cmp = [descending](const Level& lvl, double price) {
        return descending ? lvl.price > price : lvl.price < price;
    };
    auto it = std::lower_bound(side.begin(), side.end(), o->price, cmp);
    if (it != side.end() && it->price == o->price) {
        it->orders.push_back(o);
    } else {
        side.insert(it, Level{o->price, {o}});
    }
}

bool OrderBook::erase_from(std::vector<Level>& side, std::uint64_t id) {
    for (auto lit = side.begin(); lit != side.end(); ++lit) {
        auto& q = lit->orders;
        auto oit = std::find_if(q.begin(), q.end(),
                                [id](OrderT* p) { return p && p->id == id; });
        if (oit != q.end()) {
            q.erase(oit);
            if (q.empty()) side.erase(lit);
            return true;
        }
    }
    return false;
}

void OrderBook::add(OrderT* o) {
    if (!o) return;
    if (o->is_buy) insert_sorted(bids_, o, /*descending=*/true);
    else           insert_sorted(asks_, o, /*descending=*/false);
}

void OrderBook::cancel(std::uint64_t id) {
    if (!erase_from(bids_, id)) erase_from(asks_, id);
}

const OrderT* OrderBook::best_bid() const {
    if (bids_.empty() || bids_.front().orders.empty()) return nullptr;
    return bids_.front().orders.front();
}
const OrderT* OrderBook::best_ask() const {
    if (asks_.empty() || asks_.front().orders.empty()) return nullptr;
    return asks_.front().orders.front();
}

OrderT* OrderBook::pop_best_bid() {
    if (bids_.empty() || bids_.front().orders.empty()) return nullptr;
    OrderT* o = bids_.front().orders.front();
    bids_.front().orders.erase(bids_.front().orders.begin());
    if (bids_.front().orders.empty()) bids_.erase(bids_.begin());
    return o;
}
OrderT* OrderBook::pop_best_ask() {
    if (asks_.empty() || asks_.front().orders.empty()) return nullptr;
    OrderT* o = asks_.front().orders.front();
    asks_.front().orders.erase(asks_.front().orders.begin());
    if (asks_.front().orders.empty()) asks_.erase(asks_.begin());
    return o;
}

#else  // multimap variant

void OrderBook::add(OrderT* o) {
    if (!o) return;
    if (o->is_buy) bids_.emplace(o->price, o);
    else           asks_.emplace(o->price, o);
}

void OrderBook::cancel(std::uint64_t id) {
    for (auto it = bids_.begin(); it != bids_.end(); ++it) {
        if (it->second && it->second->id == id) { bids_.erase(it); return; }
    }
    for (auto it = asks_.begin(); it != asks_.end(); ++it) {
        if (it->second && it->second->id == id) { asks_.erase(it); return; }
    }
}

const OrderT* OrderBook::best_bid() const {
    return bids_.empty() ? nullptr : bids_.begin()->second;
}
const OrderT* OrderBook::best_ask() const {
    return asks_.empty() ? nullptr : asks_.begin()->second;
}

OrderT* OrderBook::pop_best_bid() {
    if (bids_.empty()) return nullptr;
    OrderT* o = bids_.begin()->second;
    bids_.erase(bids_.begin());
    return o;
}
OrderT* OrderBook::pop_best_ask() {
    if (asks_.empty()) return nullptr;
    OrderT* o = asks_.begin()->second;
    asks_.erase(asks_.begin());
    return o;
}

#endif
