#include <gtest/gtest.h>
#include "Orderbook.h"

// Test fixture for the Orderbook class
class OrderbookTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize any objects or variables needed for the tests
        orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
        orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 105, 8));
    }

    // Declare any objects or variables needed for the tests
    Orderbook orderbook;
};

// Test case to verify adding an order
TEST_F(OrderbookTest, AddOrderTest) {
    // Add a new order and verify the size increases
    const OrderId orderId = 3;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 95, 15));
    EXPECT_EQ(orderbook.Size(), 3);
}

// Test case to verify canceling an order
TEST_F(OrderbookTest, CancelOrderTest) {
    // Cancel an existing order and verify the size decreases
    orderbook.CancelOrder(1);
    EXPECT_EQ(orderbook.Size(), 1);
}

// Test case to verify modifying an order
TEST_F(OrderbookTest, ModifyOrderTest) {
    // Modify an existing order and verify the size remains unchanged
    OrderModify modifiedOrder(2, Side::Sell, 110, 5);
    orderbook.ModifyOrder(modifiedOrder);
    EXPECT_EQ(orderbook.Size(), 2);

    // Verify that the modified order is reflected correctly
    const auto& orderInfos = orderbook.GetOrderInfos();
    const auto& asks = orderInfos.GetAsks();
    for (const auto& ask : asks) {
        if (ask.price_ == 110) {
            EXPECT_EQ(ask.quantity_, 5);
            return;
        }
    }
    FAIL() << "Modified order not found in the order book.";
}

// Test case to verify matching orders
TEST_F(OrderbookTest, MatchOrdersTest) {
    // Add more orders to create matching scenarios
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 100, 12));
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 4, Side::Buy, 110, 6));

    // Match orders and verify trades are generated
    auto trades = orderbook.MatchOrders();
    EXPECT_GT(trades.size(), 0);

    // Verify that orders are filled or partially filled
    for (const auto& trade : trades) {
        EXPECT_GT(trade.GetBidTrade().quantity_, 0);
        EXPECT_GT(trade.GetAskTrade().quantity_, 0);
    }
}

// Test case to verify edge cases and error handling
TEST_F(OrderbookTest, EdgeCasesTest) {
    // Try adding orders with invalid parameters and verify appropriate errors are thrown
    EXPECT_THROW(orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 5, Side::Buy, -10, 5)), std::invalid_argument);
    EXPECT_THROW(orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 6, Side::Sell, 120, -5)), std::invalid_argument);

    // Try canceling a non-existent order and verify no changes occur
    EXPECT_EQ(orderbook.Size(), 2);
    orderbook.CancelOrder(100);  // Non-existent order
    EXPECT_EQ(orderbook.Size(), 2);

    // Try modifying a non-existent order and verify no changes occur
    OrderModify modifyNonExistentOrder(100, Side::Buy, 110, 5);
    EXPECT_EQ(orderbook.Size(), 2);
    orderbook.ModifyOrder(modifyNonExistentOrder);
    EXPECT_EQ(orderbook.Size(), 2);

    // Add an order with insufficient funds and verify it's not added
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 7, Side::Buy, 110, 15));
    EXPECT_EQ(orderbook.Size(), 2);
}
