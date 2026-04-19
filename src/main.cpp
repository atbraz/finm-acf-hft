#include "feed_parser.h"
#include "market_snapshot.h"
#include "order_manager.h"

#include <iomanip>
#include <iostream>

constexpr double SPREAD_THRESHOLD = 0.10;
constexpr int ORDER_QTY = 10;

int main() {
    auto feed = load_feed("sample_feed.txt");
    MarketSnapshot snapshot;
    OrderManager order_manager;
    int net_position = 0;

    for (const auto& event : feed) {
        event.print();

        if (event.type == FeedType::BID) {
            snapshot.update_bid(event.price, event.quantity);
            const auto* best_bid = snapshot.get_best_bid();
            if (best_bid) {
                std::cout << "[Market] Best Bid: " << std::fixed
                          << std::setprecision(2) << best_bid->price << " x "
                          << best_bid->quantity << "\n";
            }
        } else if (event.type == FeedType::ASK) {
            snapshot.update_ask(event.price, event.quantity);
            const auto* best_ask = snapshot.get_best_ask();
            if (best_ask) {
                std::cout << "[Market] Best Ask: " << std::fixed
                          << std::setprecision(2) << best_ask->price << " x "
                          << best_ask->quantity << "\n";
            }
        } else if (event.type == FeedType::EXECUTION) {
            order_manager.handle_fill(event.order_id, event.quantity);
            continue;
        }

        double spread = snapshot.get_spread();
        if (spread < SPREAD_THRESHOLD && spread > 0) {
            Side side;
            double price;
            if (net_position >= 0) {
                side = Side::Sell;
                price = snapshot.get_best_ask()->price;
            } else {
                side = Side::Buy;
                price = snapshot.get_best_bid()->price;
            }

            int id = order_manager.place_order(side, price, ORDER_QTY);
            std::cout << "[Strategy] Spread " << std::fixed
                      << std::setprecision(2) << spread
                      << " < threshold " << SPREAD_THRESHOLD << ". Placing "
                      << (side == Side::Buy ? "BUY" : "SELL") << " at "
                      << price << " x " << ORDER_QTY
                      << " (ID = " << id << ", pos = " << net_position << ")\n";

            // Update position assuming immediate fill for simplicity.
            // Real fills are tracked via EXECUTION events, but for position
            // tracking purposes we account at placement time since this is
            // a simulation with guaranteed fills.
            net_position += (side == Side::Buy ? ORDER_QTY : -ORDER_QTY);
        }
    }

    std::cout << "\n--- Final State ---\n";
    std::cout << "Net position: " << net_position << "\n";
    order_manager.print_active_orders();

    return 0;
}
