#include "order/Order.h"
#include <sstream>
#include <iomanip>

/* 带参 ctor */
Order::Order(const std::string& id,
             const std::string& customer,
             const std::vector<CartItem>& items)
    : order_id(id), customer_name(customer), items(items),
      status(OrderStatus::Unpaid), created_time(std::time(nullptr))
{
    total_amount = 0.0;
    for(const auto& it:items) total_amount += it.GetTotalPrice();
}

/* getter */
std::string Order::GetId() const { return order_id; }
std::string Order::GetCustomer() const { return customer_name; }
double Order::GetTotalAmount() const { return total_amount; }
OrderStatus Order::GetStatus() const { return status; }
std::time_t Order::GetCreatedTime() const { return created_time; }
const std::vector<CartItem>& Order::GetItems() const { return items; }

/* setter */
void Order::SetStatus(OrderStatus s){ status = s; }

/* 序列化 */
std::string Order::Serialize() const
{
    std::ostringstream oss;
    oss<<"O|"<<order_id<<'|'<<customer_name<<'|'<<static_cast<int>(status)
       <<'|'<<created_time<<'|'<<total_amount;
    return oss.str();
}

/* 反序列化（只恢复头部） */
void Order::Deserialize(const std::string& header,
                        const std::vector<CartItem>& body)
{
    std::stringstream ss(header); std::string tmp;
    std::getline(ss,tmp,'|');                // "O"
    std::getline(ss,order_id,'|');
    std::getline(ss,customer_name,'|');

    std::string status_str,time_str,amount_str;
    std::getline(ss,status_str,'|');
    std::getline(ss,time_str,'|');
    std::getline(ss,amount_str);

    status        = static_cast<OrderStatus>(std::stoi(status_str));
    created_time  = static_cast<std::time_t>(std::stoll(time_str));
    total_amount  = std::stod(amount_str);
    items         = body;
}
