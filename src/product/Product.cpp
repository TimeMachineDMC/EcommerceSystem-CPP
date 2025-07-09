#include "product/Product.h"

Product::Product(const std::string& name,
                 const std::string& description,
                 double price,
                 int stock)
    : name(name), description(description), price(price), stock(stock) {}

void Product::SetId(const std::string& id_) { id = id_; }
std::string Product::GetId()   const { return id; }
std::string Product::GetName() const { return name; }
std::string Product::GetDescription() const { return description; }

int Product::GetStock() const { return stock; }
int Product::GetFrozenStock() const { return frozen_stock; }
int Product::GetAvailableStock() const { return stock - frozen_stock; }

double Product::GetRawPrice() const { return price; }

void Product::SetStock(int v){ if(v>=0) stock = v; }
void Product::SetPrice(double v){ if(v>=0) price = v; }

void Product::SetOwner(const std::string& o){ owner_name = o; }
std::string Product::GetOwner() const { return owner_name; }

/* 冻结 */
void Product::FreezeStock(int q){
    if(q>=0 && q<=GetAvailableStock()) frozen_stock += q;
}
void Product::UnfreezeStock(int q){
    if(q>=0 && q<=frozen_stock) frozen_stock -= q;
}
void Product::CommitFrozenStock(){
    stock -= frozen_stock;
    frozen_stock = 0;
}
