#pragma once
#include <vector>
#include <string>
#include <ctime>
#include "cart/CartItem.h"

enum class OrderStatus { Unpaid, Paid, Failed };

class Order {
private:
    std::string            order_id;
    std::string            customer_name;
    std::vector<CartItem>  items;
    double                 total_amount = 0.0;
    OrderStatus            status       = OrderStatus::Unpaid;
    std::time_t            created_time = 0;

public:
    Order() = default;  

    Order(const std::string& id,
          const std::string& customer,
          const std::vector<CartItem>& items);

    std::string  GetId()       const;
    std::string  GetCustomer() const;
    double       GetTotalAmount() const;
    OrderStatus  GetStatus()   const;
    std::time_t  GetCreatedTime() const;
    const std::vector<CartItem>& GetItems() const;

    void SetStatus(OrderStatus s);

    std::string Serialize() const;
    void Deserialize(const std::string& header,
                     const std::vector<CartItem>& body);
};
