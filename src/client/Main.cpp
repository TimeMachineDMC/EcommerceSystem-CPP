#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>  
#include <fstream> 
#include "network/SocketStream.h"
#include "../../third_party/json.hpp"
using json = nlohmann::json;

bool IsUserLoggedIn(const std::string& username) {
    std::filesystem::path lock_file = "data/sessions/" + username + ".lock";
    return std::filesystem::exists(lock_file);
}

void LockUserSession(const std::string& username) {
    std::filesystem::create_directories("data/sessions");
    std::ofstream lock_file("data/sessions/" + username + ".lock");
    if (lock_file.is_open()) {
        lock_file << "LOCKED" << std::endl;
        lock_file.close();
    }
}

void UnlockUserSession(const std::string& username) {
    std::filesystem::remove("data/sessions/" + username + ".lock");
}

static std::string inl(){ std::string s; std::getline(std::cin,s); return s; }
static void showP(const json& a){
    for(auto&p:a)
        std::cout << '[' << std::setw(7) << p["cat"] << "] "
                  << '"' << p["name"] << "\": \"" << p["desc"] << "\""
                  << ", Price: " << p["price"]
                  << ", Stock: " << p["stock"]
                  << ", Owner: \"" << p["owner"] << "\"\n";
}
static uint16_t read_port(int argc,char*argv[]){
    if(argc<3) return 5555;
    try{
        int p=std::stoi(argv[2]);
        if(p<1||p>65535) throw std::out_of_range("bad");
        return static_cast<uint16_t>(p);
    }catch(...){
        std::cout<<"Invalid port, fallback 5555\n"; return 5555;
    }
}

int main(int argc,char*argv[])
{
    if(argc<2){ std::cout<<"Usage: client <ip> [port]\n"; return 0; }
    uint16_t port = read_port(argc,argv);
    SocketStream ss{argv[1], port};
    std::cout<<"Connected to "<<argv[1]<<':'<<port<<'\n';

    while(true){
        std::cout<<"\n1. Register  2. Login  3. List all products  0. Exit\n> ";
        std::string ch=inl();
        if(ch=="0") return 0;

        if(ch=="3"){
            ss.SendLine(R"({"type":"List","data":{}})");
            showP(json::parse(ss.RecvLine())["data"]);
            continue;
        }
        if(ch=="1"){
            std::string u,p,t;
            std::cout<<"username: "; u=inl();
            std::cout<<"password: "; p=inl();
            std::cout<<"type Customer/Seller: "; t=inl();
            ss.SendLine(json{{"type","Register"},
                     {"data",{{"username",u},{"password",p},{"userType",t}}}}.dump());
            std::cout<<(json::parse(ss.RecvLine())["ok"]?"Register OK\n":"Register fail\n");
            continue;
        }
        if(ch!="2"){ std::cout<<"Invalid.\n"; continue;}

        std::string u,p;
        std::cout<<"username: "; u=inl();
        std::cout<<"password: "; p=inl();

        if (IsUserLoggedIn(u)) {
            std::cout << "Login failed: User is already logged in elsewhere.\n";
            continue;
        }

        ss.SendLine(json{{"type","Login"},
                 {"data",{{"username",u},{"password",p}}}}.dump());
        auto log=json::parse(ss.RecvLine());
        if(!log["ok"]||log["data"]["userType"].is_null()){ std::cout<<"Login fail\n"; continue; }
        std::string role=log["data"]["userType"];
        std::cout<<"Login successful. Welcome, "<<role<<' '<<u<<".\n";

        LockUserSession(u);

        ss.SendLine(R"({"type":"List","data":{}})");
        showP(json::parse(ss.RecvLine())["data"]);

        while(true){
            std::cout<<"\n--- Menu ---\n"
                     <<"1. View balance\n2. Recharge\n3. List all products\n"
                     <<"4. Search product\n5. Add product (only Seller)\n"
                     <<"6. Change password\n7. Manage my products (only Seller)\n"
                     <<"8. Apply discount (only Seller)\n"
                     <<"9. Manage shopping cart (only Customer)\n"
                     <<"10. Pay for order (only Customer)\n"
                     <<"11. View all orders\n"
                     <<"12. Cancel orders\n"
                     <<"13. Logout\n> ";
            std::string c=inl();

            if(c=="1"){
                ss.SendLine(R"({"type":"Balance","data":{}})");
                std::cout<<"Balance: $"<<json::parse(ss.RecvLine())["data"]<<'\n';
            }
            else if(c=="2"){
                std::cout<<"Amount: "; std::string amt=inl();
                try{
                    double v=std::stod(amt);
                    ss.SendLine(json{{"type","Recharge"},{"data",{{"amount",v}}}}.dump());
                    std::cout<<"New balance: $"<<json::parse(ss.RecvLine())["data"]<<'\n';
                }catch(...){ std::cout<<"Invalid amount\n"; }
            }

            else if(c=="3"){ ss.SendLine(R"({"type":"List","data":{}})"); showP(json::parse(ss.RecvLine())["data"]); }
            else if(c=="4"){
                std::cout<<"Keyword: "; std::string k=inl();
                ss.SendLine(json{{"type","Search"},{"data",{{"keyword",k}}}}.dump());
                showP(json::parse(ss.RecvLine())["data"]);
            }

            else if(c=="5" && role=="Seller"){
                json d;
                
                std::cout << "Category(Book/Food/Clothes): "; 
                d["category"] = inl();

                std::cout << "Name: "; 
                d["name"] = inl();

                std::cout << "Description: "; 
                d["desc"] = inl();

                double price = 0;
                while (true) {
                    std::cout << "Price: "; 
                    try {
                        price = std::stod(inl());
                        if (price < 0) {
                            std::cout << "Price cannot be negative. Please enter a valid price.\n";
                        } else {
                            break;
                        }
                    } catch (...) {
                        std::cout << "Invalid price format. Please enter a valid price.\n";
                    }
                }
                d["price"] = price;

                int stock = 0;
                while (true) {
                    std::cout << "Stock: "; 
                    try {
                        stock = std::stoi(inl());
                        if (stock < 0) {
                            std::cout << "Stock cannot be negative. Please enter a valid stock.\n";
                        } else {
                            break;
                        }
                    } catch (...) {
                        std::cout << "Invalid stock format. Please enter a valid stock.\n";
                    }
                }
                d["stock"] = stock;

                ss.SendLine(json{{"type", "AddProduct"}, {"data", d}}.dump());

                std::cout << (json::parse(ss.RecvLine())["ok"] ? "Added\n" : "Fail\n");
            }

            else if(c=="7" && role=="Seller"){
                ss.SendLine(R"({"type":"SellerStock","data":{}})");
                auto lst=json::parse(ss.RecvLine())["data"];
                for(auto& e:lst) std::cout<<e["name"]<<"  Stock:"<<e["stock"]<<'\n';
                std::cout<<"Modify? (y/n) "; if(inl()=="y"){
                    std::string pn; std::cout<<"Product name: "; pn=inl();
                    std::cout<<"New stock: "; std::string st=inl();
                    ss.SendLine(json{{"type","SellerStock"},
                             {"data",{{"name",pn},{"stock",std::stoi(st)}}}}.dump());
                    std::cout<<(json::parse(ss.RecvLine())["ok"]?"Updated\n":"Fail\n");
                }
            }
            else if(c=="8" && role=="Seller"){
                std::string cat; double r;
                std::cout<<"Category: "; cat=inl(); std::cout<<"Rate(0.9=10% off): "; r=std::stod(inl());
                ss.SendLine(json{{"type","ApplyDiscount"},{"data",{{"category",cat},{"rate",r}}}}.dump());
                std::cout<<(json::parse(ss.RecvLine())["ok"]?"Applied\n":"Fail\n");
            }

            else if(c=="6"){
                std::string o,n; std::cout<<"Old pwd: "; o=inl(); std::cout<<"New pwd: "; n=inl();
                ss.SendLine(json{{"type","ChangePassword"},
                         {"data",{{"old",o},{"new",n}}}}.dump());
                std::cout<<(json::parse(ss.RecvLine())["ok"]?"Changed\n":"Fail\n");
            }

            else if(c=="9" && role=="Customer"){
                ss.SendLine(R"({"type":"CartList","data":{}})"); 
                auto cart0=json::parse(ss.RecvLine())["data"];   
                std::cout << "\n--- Shopping Cart ---\n";        
                for(auto& it:cart0["items"])                     
                    std::cout<<"- "<<it["name"]<<" x"<<it["qty"] 
                            <<" = "<<it["price"].get<double>()* 
                            it["qty"].get<int>()<<'\n';         
                std::cout<<"Total: "<<cart0["sum"]<<'\n';        
                while(true){
                    std::cout<<"\n--- Shopping Cart ---\n"
                            <<"Options: add / remove / update / order / clear / done\n"
                            <<"(cart)> ";
                    std::string op=inl();
                    if(op=="done") break;

                    bool action = false; 

                    if(op=="add"){
                        std::cout<<"\n--- Available Products ---\n";
                        ss.SendLine(R"({"type":"List","data":{}})");
                        showP(json::parse(ss.RecvLine())["data"]);
                        std::cout<<"Product Name: "; std::string pn=inl();
                        std::cout<<"Quantity: "; std::string qt=inl();
                        ss.SendLine(json{{"type","CartAdd"},{"data",{{"name",pn},{"qty",std::stoi(qt)}}}}.dump());
                        std::cout<<(json::parse(ss.RecvLine())["ok"]?"Added\n":"Fail\n");
                        action = true;
                    }
                    else if(op=="remove"){
                        std::cout<<"Product Name: "; std::string pn=inl();
                        ss.SendLine(json{{"type","CartRemove"},{"data",{{"name",pn}}}}.dump());
                        std::cout<<(json::parse(ss.RecvLine())["ok"]?"Removed\n":"Fail\n");
                        action = true;
                    }
                    else if(op=="update"){
                        std::cout<<"Product Name: "; std::string pn=inl();
                        std::cout<<"New Qty: "; std::string qt=inl();
                        ss.SendLine(json{{"type","CartRemove"},{"data",{{"name",pn}}}}.dump()); ss.RecvLine();
                        ss.SendLine(json{{"type","CartAdd"},{"data",{{"name",pn},{"qty",std::stoi(qt)}}}}.dump());
                        std::cout<<(json::parse(ss.RecvLine())["ok"]?"Updated\n":"Fail\n");
                        action = true;
                    }
                    else if(op=="order"){                         // 下单但不付款
                        ss.SendLine(R"({"type":"OrderCreate","data":{}})");
                        auto r=json::parse(ss.RecvLine());
                        std::cout<<(r["ok"]?r["data"]["msg"]:r["msg"])<<'\n';
                        action = true;
                    }
                    else if(op=="clear"){
                        ss.SendLine(R"({"type":"CartList","data":{}})");
                        auto items=json::parse(ss.RecvLine())["data"]["items"];
                        for(auto&e:items){
                            ss.SendLine(json{{"type","CartRemove"},{"data",{{"name",e["name"]}}}}.dump());
                            ss.RecvLine();
                        }
                        std::cout<<"Cleared.\n";
                        action = true;
                    }

                    if(action){
                        ss.SendLine(R"({"type":"CartList","data":{}})");
                        auto cart=json::parse(ss.RecvLine())["data"];
                        std::cout << "--- Shopping Cart ---\n";
                        for(auto& it:cart["items"])
                            std::cout<<"- "<<it["name"]<<" x"<<it["qty"]<<" = "
                                    <<it["price"].get<double>()*it["qty"].get<int>()<<'\n';
                        std::cout<<"Total: "<<cart["sum"]<<'\n';
                    }
                }
            }

            else if(c=="10" && role=="Customer"){
                // 先拉取自己的订单
                ss.SendLine(R"({"type":"OrderList","data":{}})");
                auto list=json::parse(ss.RecvLine())["data"];

                json unpaid=json::array();
                for(auto&o:list) if(o["status"]=="Unpaid") unpaid.push_back(o);

                if(unpaid.empty()){                                     
                    std::cout<<"No unpaid orders.\n";
                    continue;
                }
                std::cout<<"=== Unpaid Orders ===\n";
                for(auto&o:unpaid){
                    std::cout<<o["id"]<<"  $" << o["amount"] << '\n';
                    for(auto& it:o["items"])
                        std::cout<<"   - "<<it["name"]<<" x"<<it["qty"]
                                <<" @"<<it["price"]<<'\n';
                }
                std::cout<<"Pay all above orders? (y/n) ";
                if(inl()!="y"){ std::cout<<"Payment canceled.\n"; continue; }

                ss.SendLine(R"({"type":"Pay","data":{}})");
                auto r=json::parse(ss.RecvLine());
                if(r["ok"])
                    std::cout<<"Payment success. Total $" << r["data"]["paid"] << '\n';
                else
                    std::cout<<"Payment failed: "<<r["msg"]<<'\n';
            }

            else if(c=="11"){
                ss.SendLine(R"({"type":"OrderList","data":{}})");
                auto list = json::parse(ss.RecvLine())["data"];
                if (list.empty()) {
                    std::cout << "No orders found.\n";
                } else {
                    for(auto&o:list)
                        std::cout << o["id"] << "  "
                                << o["status"] << "  $"
                                << o["amount"] << '\n';
                }
            }

            else if(c=="13"){
                ss.SendLine(R"({"type":"Logout","data":{}})");
                ss.RecvLine();        
                UnlockUserSession(u); 
                break;
            }

            else if (c == "12" && role == "Customer") {
                std::cout << "Enter order ID to cancel: ";
                std::string order_id;
                std::cin >> order_id;

                ss.SendLine(json{{"type", "CancelUnpaidOrder"}, {"data", {{"orderId", order_id}}}}.dump());
                auto res = json::parse(ss.RecvLine());
                
                if (res["ok"]) {
                    std::cout << "Order " << order_id << " has been successfully canceled.\n";
                } else {
                    std::cout << "Failed to cancel order " << order_id << ".\n";
                }
            }


            else std::cout<<"Invalid or permission denied.\n";
        }
    }
}
