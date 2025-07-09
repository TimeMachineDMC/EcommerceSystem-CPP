#pragma once
#include <memory>
#include "product/Product.h"

class CartItem {
private:
    std::shared_ptr<Product> product;
    int quantity;

public:
    CartItem(std::shared_ptr<Product> product,int quantity);

    std::shared_ptr<Product> GetProduct() const;
    int    GetQuantity() const;
    void   SetQuantity(int quantity);

    double GetTotalPrice() const;                 // 原价 * 数量
};
