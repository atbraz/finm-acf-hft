#include <catch2/catch_test_macros.hpp>
#include "OrderBook.hpp"

TEST_CASE("addOrder inserts into both maps", "[orderbook]") {
    OrderBook book;
    book.addOrder("ORD001", 50.10, 100, true);
    REQUIRE(book.contains("ORD001"));
    REQUIRE(book.size() == 1);
}

TEST_CASE("modifyOrder keeps id, updates state", "[orderbook]") {
    OrderBook book;
    book.addOrder("ORD001", 50.10, 100, true);
    book.modifyOrder("ORD001", 51.00, 50);
    REQUIRE(book.contains("ORD001"));
    REQUIRE(book.size() == 1);
}

TEST_CASE("deleteOrder removes order", "[orderbook]") {
    OrderBook book;
    book.addOrder("ORD001", 50.10, 100, true);
    book.deleteOrder("ORD001");
    REQUIRE_FALSE(book.contains("ORD001"));
    REQUIRE(book.size() == 0);
}

TEST_CASE("modify and delete on missing id are no-ops", "[orderbook]") {
    OrderBook book;
    book.modifyOrder("MISSING", 1.0, 1);
    book.deleteOrder("MISSING");
    REQUIRE(book.size() == 0);
    REQUIRE_FALSE(book.contains("MISSING"));
}

TEST_CASE("multiple orders at same price coexist", "[orderbook]") {
    OrderBook book;
    book.addOrder("A", 100.0, 1, true);
    book.addOrder("B", 100.0, 2, true);
    REQUIRE(book.size() == 2);
    book.deleteOrder("A");
    REQUIRE(book.contains("B"));
    REQUIRE(book.size() == 1);
}

TEST_CASE("re-add with same id overwrites in place", "[orderbook]") {
    OrderBook book;
    book.addOrder("X", 10.0, 1, true);
    book.addOrder("X", 10.0, 2, true);
    REQUIRE(book.size() == 1);
}
