#include "product/Book.h"

Book::Book(const std::string& name,
           const std::string& description,
           double price,
           int stock)
    : Product(name,description,price,stock) {}

double Book::GetPrice() const { return price; }
std::string Book::GetCategory() const { return "Book"; }
