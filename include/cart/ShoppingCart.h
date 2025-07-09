#pragma once
#include <map>
#include <string>
#include "CartItem.h"
#include <filesystem>

class ShoppingCart {
private:
    std::map<std::string,CartItem> items;  

public:
    void AddItem(std::shared_ptr<Product> product,int quantity);
    void RemoveItem(const std::string& product_id);
    void UpdateQuantity(const std::string& product_id,int new_qty);

    const std::map<std::string,CartItem>& GetItems() const;
    double GetTotalAmount() const;          // 原价合计
    void   Clear();

    static void ClearCartFile(const std::string& username) {
        std::filesystem::path cart_file = "data/carts/" + username + ".txt";
        if (std::filesystem::exists(cart_file)) {
            std::filesystem::remove(cart_file);  
        }
    }
};
