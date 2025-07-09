#include "product/Clothes.h"

Clothes::Clothes(const std::string& name,
                 const std::string& description,
                 double price,
                 int stock)
    : Product(name,description,price,stock) {}

double Clothes::GetPrice() const { return price; }
std::string Clothes::GetCategory() const { return "Clothes"; }
