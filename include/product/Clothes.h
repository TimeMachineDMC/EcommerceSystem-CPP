#pragma once
#include "Product.h"

// 服装类商品
class Clothes : public Product {
public:
    Clothes(const std::string& name,
            const std::string& description,
            double price,
            int stock);

    double      GetPrice()   const override;
    std::string GetCategory() const override;
};
