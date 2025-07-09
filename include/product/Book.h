#pragma once
#include "Product.h"

// 图书类商品
class Book : public Product {
public:
    Book(const std::string& name,
         const std::string& description,
         double price,
         int stock);

    double      GetPrice()   const override;
    std::string GetCategory() const override;
};
