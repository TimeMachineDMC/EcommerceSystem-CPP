#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>

#include "order/Order.h"                    // --- MOD: 别忘头文件
#include "cart/ShoppingCart.h"
#include "user/Customer.h"
#include "user/Seller.h"
#include "manager/ProductManager.h"
#include "manager/UserManager.h"

class OrderManager {
private:
    std::vector<Order> orders;
    ProductManager& product_mgr;
    UserManager&    user_mgr;
    int order_counter = 0;

public:
    OrderManager(ProductManager& pm, UserManager& um);

    std::string CreateOrder(Customer& user,const ShoppingCart& cart);
    bool   PayOrder(Customer& user,const std::string& order_id);
    double PayAllUnpaid(Customer& user);            // --- MOD: 已修复卖家收款
    bool   CancelUnpaidOrder(Customer& user,const std::string& order_id);

    const std::vector<Order>& GetAllOrders() const;

    void SaveToFile(const std::string& path) const;
    void LoadFromFile(const std::string& path,const ProductManager& pm);

    /* 购物车持久化（简单文本） */
    static void SaveCartToFile(const std::string& user, const ShoppingCart&);
    // --- FIX ↓ 额外把 ProductManager 传进来
    static void LoadCartFromFile(const std::string& user,
                                 ProductManager& pm,           // <-- 新参数
                                 ShoppingCart& out);           // <--
};
