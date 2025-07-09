#include "cart/CartItem.h"

CartItem::CartItem(std::shared_ptr<Product> product,int quantity)
    : product(std::move(product)), quantity(quantity) {}

std::shared_ptr<Product> CartItem::GetProduct() const { return product; }
int  CartItem::GetQuantity() const { return quantity; }

void CartItem::SetQuantity(int q){ if(q>=0) quantity=q; }

double CartItem::GetTotalPrice() const { return product->GetPrice()*quantity; }
