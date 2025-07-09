/**
 * Phase-3 服务器入口
 *  支持：Register / Login / Balance / Recharge / ChangePassword /
 *        List / Search / CartAdd / CartRemove / CartList /
 *        OrderCreate / Pay / AddProduct / ApplyDiscount /
 *        SellerStock / OrderList / Logout
 */
#include <iostream>
#include <thread>
#include <iomanip>
#include <memory>
#include "network/SocketStream.h"
#include "manager/UserManager.h"
#include "manager/ProductManager.h"
#include "manager/OrderManager.h"
#include "cart/ShoppingCart.h"
#include "user/Customer.h"
#include "user/Seller.h"
#include "../../third_party/json.hpp"
using json = nlohmann::json;

/* ───────── 每个客户端线程 ───────── */
static void Handle(socket_t s,
                   UserManager& um,
                   ProductManager& pm,
                   OrderManager& om)
{
    try {
        SocketStream ss{s};
        std::shared_ptr<User> cur;         // 当前登录用户
        ShoppingCart          cart;        // 会话购物车

        auto ok  =[&](const json& d){ ss.SendLine(json{{"ok",true},{"data",d}}.dump()); };
        auto ok0 =[&]{ ss.SendLine(R"({"ok":true})"); };
        auto err =[&](const std::string& m){ ss.SendLine(json{{"ok",false},{"msg",m}}.dump()); };

        while (true) {
            std::string ln = ss.RecvLine();
            if (ln.empty()) break;
            json req = json::parse(ln);
            const std::string tp = req["type"];

            /* ---------- Register ---------- */
            if (tp == "Register") {
                const auto& d = req["data"];
                bool res = um.RegisterUser(d["username"], d["password"], d["userType"]);
                if (res) { um.SaveToFile(); ok0(); }
                else     { err("User exists or bad type"); }
            }

            /* ---------- Login ---------- */
            else if (tp == "Login") {
                const auto& d = req["data"];
                cur = um.Login(d["username"], d["password"]);
                if (!cur) { err("Invalid credential"); continue; }
                OrderManager::LoadCartFromFile(                       // --- FIX ①
                cur->GetUsername(), pm, cart); 
                ok({{"userType", cur->GetUserType()}});
            }

            /* ---------- ChangePassword ---------- */
            else if (tp == "ChangePassword") {
                if (!cur) { err("Not login"); continue; }
                const auto& d = req["data"];
                if (!cur->CheckPassword(d["old"])) { err("Old password wrong"); continue; }
                cur->SetPassword(d["new"]); um.SaveToFile(); ok0();
            }

            /* ---------- Balance / Recharge ---------- */
            else if (tp == "Balance") {
                if (cur) ok(cur->GetBalance()); else err("Not login");
            }
            else if (tp == "Recharge") {
                if (!cur) { err("Not login"); continue; }
                double amt = req["data"]["amount"];
                cur->Recharge(amt); um.SaveToFile(); ok(cur->GetBalance());
            }

            /*──── List / Search ────*/
            else if(tp=="List"||tp=="Search"){
                auto vec = tp=="List"
                         ? pm.GetAllProducts()
                         : pm.SearchByName(req["data"]["keyword"]);
                json arr=json::array();
                for(auto&p:vec)
                    arr.push_back({{"cat",p->GetCategory()},
                                   {"name",p->GetName()},
                                   {"desc",p->GetDescription()},
                                   {"price",pm.GetDisplayPrice(p)},
                                   {"stock",p->GetStock()},
                                   {"owner",p->GetOwner()}});
                ok(arr);
            }
            /* ---------- Cart ---------- */
            else if (tp == "CartAdd") {
                auto cust = std::dynamic_pointer_cast<Customer>(cur);
                if (!cust) { err("Only customer"); continue; }
                const auto& d=req["data"];
                auto p = pm.GetProductByName(d["name"]);
                if (!p) { err("Product not found"); continue; }
                if (p->GetAvailableStock() < d["qty"]) { err("Stock not enough"); continue; }
                cart.AddItem(p, d["qty"]); ok0();
            }
            else if (tp == "CartRemove") {
                auto p = pm.GetProductByName(req["data"]["name"]);
                if (p) cart.RemoveItem(p->GetId());
                ok0();
            }
            else if (tp == "CartList") {
                json arr=json::array(); double sum=0;
                for (auto&[id,it]:cart.GetItems()){
                    auto p=it.GetProduct();
                    double pr=pm.GetDisplayPrice(p);
                    sum+=pr*it.GetQuantity();
                    arr.push_back({{"name",p->GetName()},
                                   {"qty",it.GetQuantity()},
                                   {"price",pr}});
                }
                ok({{"items",arr},{"sum",sum}});
            }

            /* ---------- OrderCreate ---------- */
            else if (tp == "OrderCreate") {
                auto cust = std::dynamic_pointer_cast<Customer>(cur);
                if (!cust) { err("Only customer"); continue; }
                if (cart.GetItems().empty()) { err("Cart empty"); continue; }

                std::string oid = om.CreateOrder(*cust, cart);
                if (oid.empty()) { err("Create failed"); continue; }

                cart.Clear(); om.SaveToFile("data/orders.txt");
                ok({{"orderId",oid},{"msg","Order created"}});
            }

            /* ---------- Pay ---------- */
            else if (tp == "Pay") {
                auto cust = std::dynamic_pointer_cast<Customer>(cur);
                if (!cust) { err("Only customer"); continue; }

                double paid = om.PayAllUnpaid(*cust);
                if (paid < 0)               { err("Insufficient balance"); continue; }
                if (paid == 0)              { ok({{"msg","No unpaid orders"}}); continue; }   // --- MOD

                um.SaveToFile(); om.SaveToFile("data/orders.txt");
                ok({{"msg","Payment success"},{"paid",paid}});
            }

            /* ---------- SellerStock ---------- */
            else if (tp == "SellerStock") {
                auto sel = std::dynamic_pointer_cast<Seller>(cur);
                if (!sel) { err("Only seller"); continue; }
                const auto& d=req["data"];

                if (d.contains("name")) {          // modify
                    auto p = pm.GetProductByName(d["name"]);
                    if (!p || p->GetOwner()!=cur->GetUsername()){ err("Not found"); continue;}
                    p->SetStock(d["stock"]); pm.SaveToFile(); ok0();
                } else {                           // list
                    json arr=json::array();
                    for(auto& p:pm.GetProductsBySeller(cur->GetUsername()))
                        arr.push_back({{"name",p->GetName()},{"stock",p->GetStock()}});
                    ok(arr);
                }
            }

            /* ---------- AddProduct ---------- */
            else if (tp == "AddProduct") {
                auto sel = std::dynamic_pointer_cast<Seller>(cur);
                if (!sel) { err("Only seller"); continue; }
                const auto& d=req["data"];
                std::shared_ptr<Product> p;
                std::string cat=d["category"];
                double price = d["price"];
                int stock = d["stock"];
                if (price < 0) { err("Price cannot be negative"); continue; }
                if (stock < 0) { err("Stock cannot be negative"); continue; }
                if      (cat=="Book")    p=std::make_shared<Book>(d["name"],d["desc"],d["price"],d["stock"]);
                else if (cat=="Food")    p=std::make_shared<Food>(d["name"],d["desc"],d["price"],d["stock"]);
                else if (cat=="Clothes") p=std::make_shared<Clothes>(d["name"],d["desc"],d["price"],d["stock"]);
                else { err("Bad category"); continue; }

                p->SetOwner(sel->GetUsername());
                p->SetId(cat + "-" + std::to_string(pm.GetAllProducts().size()));
                pm.AddProduct(p); pm.SaveToFile(); ok0();
            }

            /* ---------- ApplyDiscount ---------- */
            else if (tp == "ApplyDiscount") {
                auto sel = std::dynamic_pointer_cast<Seller>(cur);
                if (!sel) { err("Only seller"); continue; }
                double rate = req["data"]["rate"];                   // --- MOD
                if(rate<=0||rate>=1){ err("Rate must be >0 && <1"); continue; }
                pm.ApplyDiscount(req["data"]["category"], req["data"]["rate"]);
                pm.SaveToFile(); ok0();
            }

            /* ---------- OrderList ---------- */
            else if (tp == "OrderList") {
                json arr = json::array();

                // 如果是顾客，只显示属于该顾客的订单
                if (cur) {
                    std::string uname = cur->GetUsername();
                    // 商家可以看到所有订单
                    if (cur->GetUserType() == "Seller") {
                        for (const auto& o : om.GetAllOrders()) {
                            std::string stat = (o.GetStatus() == OrderStatus::Paid ? "Paid" :
                                                o.GetStatus() == OrderStatus::Unpaid ? "Unpaid" : "Failed");
                            arr.push_back({{"id", o.GetId()},
                                        {"amount", o.GetTotalAmount()},
                                        {"status", stat},
                                        {"customer", o.GetCustomer()}});
                        }
                    }
                    else { // 顾客只看到自己的订单
                        for (const auto& o : om.GetAllOrders()) {
                            if (o.GetCustomer() == uname) {
                                std::string stat = (o.GetStatus() == OrderStatus::Paid ? "Paid" :
                                                    o.GetStatus() == OrderStatus::Unpaid ? "Unpaid" : "Failed");
                                arr.push_back({{"id", o.GetId()},
                                            {"amount", o.GetTotalAmount()},
                                            {"status", stat}});
                            }
                        }
                    }
                }

                ok(arr);  // 返回订单列表
            }
            
            else if (tp == "CancelUnpaidOrder") {
                auto cust = std::dynamic_pointer_cast<Customer>(cur);
                if (!cust) { err("Only customer can cancel unpaid orders"); continue; }

                const auto& d = req["data"];
                std::string order_id = d["orderId"];

                if (om.CancelUnpaidOrder(*cust, order_id)) {
                    ok0();
                } else {
                    err("Failed to cancel order or order is already paid");
                }
            }

            /* ---------- Logout ---------- */
            else if (tp == "Logout") {
                if(cur) OrderManager::SaveCartToFile(cur->GetUsername(), cart); // --- MOD
                cur.reset(); cart.Clear(); ok0();
            }

            else err("Unsupported");
        }
    } catch (const std::exception& e) {
        std::cerr << "[Server] " << e.what() << '\n';
    }
}

/* ───────── main ───────── */
int main(int argc,char* argv[])
{
    uint16_t port = (argc>=2)? static_cast<uint16_t>(std::stoi(argv[1])) : 5555;
    SocketStream::InitWSA();

    socket_t lis = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (::bind(lis,(sockaddr*)&addr,sizeof(addr)) < 0 ||
        ::listen(lis,16) < 0) {
        perror("bind"); return 1;
    }

    UserManager    um("data/users.txt");    um.LoadFromFile();
    ProductManager pm("data/products.txt"); pm.LoadFromFile();
    OrderManager   om(pm, um);              om.LoadFromFile("data/orders.txt", pm);

    std::cout << "Server listening on port " << port << " ...\n";
    while (true) {
        socket_t s = ::accept(lis,nullptr,nullptr);
        if (s == -1 || s == INVALID_SOCKET) continue;
        std::thread(Handle, s, std::ref(um), std::ref(pm), std::ref(om)).detach();
    }
}
