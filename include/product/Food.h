#pragma once
#include "Product.h"

// 食品类商品
class Food : public Product {
public:
    Food(const std::string& name,
         const std::string& description,
         double price,
         int stock);

    double      GetPrice()   const override;
    std::string GetCategory() const override;
};
