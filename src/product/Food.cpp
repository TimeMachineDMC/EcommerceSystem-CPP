#include "product/Food.h"

Food::Food(const std::string& name,
           const std::string& description,
           double price,
           int stock)
    : Product(name,description,price,stock) {}

double Food::GetPrice() const { return price; }
std::string Food::GetCategory() const { return "Food"; }
