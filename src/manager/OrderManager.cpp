#include "manager/OrderManager.h"
#include "common/Utils.h"                 // --- MOD: PadNumber 等工具
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <mutex>

/* --- 本文件工具 --- */
namespace {
std::vector<std::string> Split(const std::string& s,char delim='|')
{
    std::vector<std::string> out;
    std::stringstream ss(s); std::string item;
    while(std::getline(ss,item,delim)) out.push_back(item);
    return out;
}
} // namespace

/* ctor */
OrderManager::OrderManager(ProductManager& pm,UserManager& um)
    : product_mgr(pm), user_mgr(um) {}

/* ---------- CreateOrder ---------- */
std::string OrderManager::CreateOrder(Customer& user,const ShoppingCart& cart)
{
    if(cart.GetItems().empty()) return "";

    /* 库存校验 */
    for(auto&[id,it]:cart.GetItems())
        if(it.GetQuantity()>it.GetProduct()->GetAvailableStock())
            return "";

    /* 冻结库存并做快照 */
    std::vector<CartItem> snap;
    for(auto&[id,it]:cart.GetItems()){
        it.GetProduct()->FreezeStock(it.GetQuantity());
        snap.push_back(it);
    }

    std::ostringstream oss;
    oss<<"ORD-"<<std::setw(6)<<std::setfill('0')<<order_counter++;
    orders.emplace_back(oss.str(), user.GetUsername(), snap);
    return oss.str();
}

/* ---------- PayOrder ---------- */
bool OrderManager::PayOrder(Customer& user,const std::string& oid)
{
    for(auto&o:orders)
        if(o.GetId()==oid && o.GetStatus()==OrderStatus::Unpaid){
            double sum=o.GetTotalAmount();
            if(user.GetBalance()<sum) return false;

            user.DeductBalance(sum);

            for(auto& it:o.GetItems()){
                auto p=it.GetProduct();
                // --- MOD: 卖家加钱用 Recharge，而非 AddBalance
                auto seller=user_mgr.FindByUsername(p->GetOwner());
                if(seller) seller->Recharge(it.GetTotalPrice());

                p->CommitFrozenStock();
            }
            o.SetStatus(OrderStatus::Paid);
            product_mgr.SaveToFile(); user_mgr.SaveToFile();
            // ShoppingCart::ClearCartFile(user.GetUsername());
            return true;
        }
    return false;
}

/* ---------- PayAllUnpaid ---------- */
double OrderManager::PayAllUnpaid(Customer& user)
{
    double need=0;
    for(auto&o:orders)
        if(o.GetCustomer()==user.GetUsername()&&o.GetStatus()==OrderStatus::Unpaid)
            need+=o.GetTotalAmount();

    if(need==0) return 0.0;
    if(user.GetBalance()<need) return -1.0;

    /* --- MOD: 一次性扣，再循环给卖家分账 --- */
    user.DeductBalance(need);

    for(auto&o:orders)
        if(o.GetCustomer()==user.GetUsername()&&o.GetStatus()==OrderStatus::Unpaid){
            for(auto& it:o.GetItems()){
                auto p=it.GetProduct();
                auto seller=user_mgr.FindByUsername(p->GetOwner());
                if(seller) seller->Recharge(it.GetTotalPrice());
                p->CommitFrozenStock();
            }
            o.SetStatus(OrderStatus::Paid);
        }
    product_mgr.SaveToFile(); user_mgr.SaveToFile();
    // ShoppingCart::ClearCartFile(user.GetUsername());
    return need;
}

/* ---------- CancelOrder ---------- */
bool OrderManager::CancelUnpaidOrder(Customer& user, const std::string& order_id)
{
    for (auto& order : orders) {
        if (order.GetId() == order_id && order.GetCustomer() == user.GetUsername() && order.GetStatus() == OrderStatus::Unpaid) {
            order.SetStatus(OrderStatus::Failed); // 取消订单，状态设为 Failed
            // 恢复冻结的库存
            for (auto& item : order.GetItems()) {
                item.GetProduct()->UnfreezeStock(item.GetQuantity());
            }
            product_mgr.SaveToFile(); // 保存商品库存
            user_mgr.SaveToFile();    // 保存用户信息
            return true;
        }
    }
    return false;
}


/* ---------- Save ---------- */
void OrderManager::SaveToFile(const std::string& path) const
{
    std::ofstream fout(path,std::ios::trunc);
    for(const auto&o:orders){
        fout<<o.Serialize()<<'\n';
        for(const auto& it:o.GetItems())
            fout<<"I|"<<it.GetProduct()->GetId()<<'|'<<it.GetQuantity()<<'\n';
    }
}

/* ---------- Load ---------- */
void OrderManager::LoadFromFile(const std::string& path,const ProductManager& pm)
{
    orders.clear(); order_counter=0;

    std::ifstream fin(path);
    if(!fin) return;

    std::string line; std::vector<CartItem> buf; std::string header;

    auto flush=[&](){
        if(!header.empty()){
            Order tmp;
            tmp.Deserialize(header,buf);
            orders.push_back(std::move(tmp));
            buf.clear(); header.clear();
        }
    };

    while(std::getline(fin,line)){
        if(line.rfind("O|",0)==0){ flush(); header=line; }
        else if(line.rfind("I|",0)==0){
            auto parts=Split(line);
            if(parts.size()==3){
                auto p=pm.GetProductById(parts[1]);
                if(p) buf.emplace_back(p,std::stoi(parts[2]));
            }
        }
    }
    flush();
    order_counter = static_cast<int>(orders.size());
}

/* ---------- helper ---------- */
const std::vector<Order>& OrderManager::GetAllOrders() const { return orders; }

/* ---------- 购物车持久化简单实现 ---------- */
void OrderManager::SaveCartToFile(const std::string& user, const ShoppingCart& cart)
{
    if(cart.GetItems().empty()) return;
    std::filesystem::create_directories("data/carts");
    std::ofstream fout("data/carts/"+user+".txt",std::ios::trunc);
    for(auto&[id,it]:cart.GetItems())
        fout<<id<<'|'<<it.GetQuantity()<<'\n';
}
void OrderManager::LoadCartFromFile(const std::string& user,
                                    ProductManager& pm,        // --- FIX
                                    ShoppingCart& out)         // --- FIX
{
    std::ifstream fin("data/carts/"+user+".txt");
    if(!fin) return;
    std::string line;
    while(std::getline(fin,line)){
        auto parts=Split(line);
        if(parts.size()!=2) continue;
        auto p = pm.GetProductById(parts[0]);                   // --- FIX
        if(p) out.AddItem(p,std::stoi(parts[1]));
    }
}
