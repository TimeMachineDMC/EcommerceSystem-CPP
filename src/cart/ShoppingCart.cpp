#include "cart/ShoppingCart.h"

void ShoppingCart::AddItem(std::shared_ptr<Product> p,int qty)
{
    if(!p || qty<=0) return;
    auto it = items.find(p->GetId());
    if(it!=items.end())
        it->second.SetQuantity(it->second.GetQuantity()+qty);
    else
        items.emplace(p->GetId(), CartItem(p,qty));
}

void ShoppingCart::RemoveItem(const std::string& pid){ items.erase(pid); }

void ShoppingCart::UpdateQuantity(const std::string& pid,int q){
    auto it=items.find(pid);
    if(it!=items.end() && q>0) it->second.SetQuantity(q);
}

const std::map<std::string,CartItem>& ShoppingCart::GetItems() const { return items; }

double ShoppingCart::GetTotalAmount() const {
    double sum=0; for(auto&[id,it]:items) sum+=it.GetTotalPrice(); return sum;
}

void ShoppingCart::Clear(){ items.clear(); }
