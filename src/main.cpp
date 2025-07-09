#include <iostream>
#include <limits>
#include <sstream>

#include "manager/UserManager.h"
#include "manager/ProductManager.h"
#include "user/Customer.h"
#include "user/Seller.h"
#include "common/Utils.h"
#include "manager/OrderManager.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* 工具函数 */
void ShowProduct(const std::shared_ptr<Product>& p, const ProductManager& pm)
{
    double price = pm.GetDisplayPrice(p);
    std::cout << "[" << p->GetCategory() << "] "
              << p->GetName() << ": " << p->GetDescription()
              << ", Price: " << price;

    if (pm.HasDiscount(p->GetCategory())) {
        double rate = pm.GetDiscountRate(p->GetCategory());
        int percent = static_cast<int>((1.0 - rate) * 100 + 0.5);
        std::cout << " (in " << percent << "% discount)";
    }

    std::cout << ", Stock: " << p->GetStock()
              << ", Owner: " << p->GetOwner() << '\n';
}

bool ReadInput(std::string& result)
{
    if (!(std::cin >> result)) return false;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (result == "q" || result == "quit") return false;
    return true;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    UserManager    user_manager("data/users.txt");
    ProductManager product_manager("data/products.txt");
    const std::string ORDER_FILE = "data/orders.txt";
    OrderManager order_manager(product_manager, user_manager);  

    user_manager.LoadFromFile();
    product_manager.LoadFromFile();
    order_manager.LoadFromFile(ORDER_FILE, product_manager); 

    std::shared_ptr<User> current_user = nullptr;
    std::string choice;

    std::cout << "Welcome to the E-Commerce Platform (Phase 1)\n";

    while (true) {

        /* 未登录  */
        if (!current_user) {
            std::cout << "\n1. Register  2. Login  3. List all products  0. Exit\n> ";
            std::cin  >> choice;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            /* 1. Register */
            if (choice == "1") {
                std::string name, pwd, type;
                std::cout << "username (or q for quit): ";
                if (!ReadInput(name)) continue;
                std::cout << "password (or q for quit): ";
                if (!ReadInput(pwd)) continue;
                std::cout << "type Customer/Seller (or q for quit): ";
                if (!ReadInput(type)) continue;

                if (user_manager.RegisterUser(name, pwd, type)) {
                    std::cout << "Registration successful.\n";
                    user_manager.SaveToFile();
                } else std::cout << "Registration failed.\n";

            /* 2. Login */
            } else if (choice == "2") {
                std::string name, pwd;
                std::cout << "username (or q for quit): ";
                if (!ReadInput(name)) continue;
                std::cout << "password (or q for quit): ";
                if (!ReadInput(pwd)) continue;

                current_user = user_manager.Login(name, pwd);
                if (current_user) {
                    std::cout << "Login successful. Welcome, "
                              << current_user->GetUserType() << ' '
                              << current_user->GetUsername() << ".\n\n";
                    for (const auto& p : product_manager.GetAllProducts())
                        ShowProduct(p, product_manager);
                } else std::cout << "Login failed.\n";

            /* 3. List all products */
            } else if (choice == "3") {
                if (product_manager.GetAllProducts().empty()) {
                    std::cout << "No products available.\n";
                } else {
                    for (const auto& p : product_manager.GetAllProducts())
                        ShowProduct(p, product_manager);
                }

            /* 0. Exit */
            } else if (choice == "0" || choice == "q" || choice == "quit") {
                break;

            } else {
                std::cout << "Invalid choice.\n";
            }

        /* 已登录 */
        } else {
            std::cout << "\n--- Menu ---\n"
                    << "1. View balance\n2. Recharge\n3. List all products\n"
                    << "4. Search product\n5. Add product (only Seller)\n"
                    << "6. Change password\n7. Manage my products (only Seller)\n"
                    << "8. Apply discount (only Seller)\n"
                    << "9. Manage shopping cart (only Customer)\n"
                    << "10. Pay for order (only Customer)\n"
                    << "11. View all orders\n"
                    << "12. Logout\n> ";
            std::cin >> choice;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            /* 1. balance */
            if (choice == "1") {
                std::cout << "Balance: " << current_user->GetBalance() << '\n';

            /* 2. recharge */
            } else if (choice == "2") {
                std::string input;
                std::cout << "Amount (or q for quit): ";
                if (!ReadInput(input)) continue;
                std::stringstream ss(input);
                double amt;
                if (!(ss >> amt)) { std::cout << "Invalid amount.\n"; continue; }
                if (amt <= 0)     { std::cout << "Amount must be positive.\n"; continue; }
                current_user->Recharge(amt);
                user_manager.SaveToFile();
                std::cout << "New balance: " << current_user->GetBalance() << '\n';

            /* 3. list all */
            } else if (choice == "3") {
                for (const auto& p : product_manager.GetAllProducts())
                    ShowProduct(p, product_manager);

            /* 4. search */
            } else if (choice == "4") {
                std::string key;
                std::cout << "Keyword (or q for quit): ";
                if (!ReadInput(key)) continue;

                auto result = product_manager.SearchByName(key);
                if (result.empty()) {
                    std::cout << "No product matches \"" << key << "\".\n";
                } else {
                    for (const auto& p : result)
                        ShowProduct(p, product_manager);
                }

            /* 5. add product (seller) */
            } else if (choice == "5" && current_user->GetUserType() == "Seller") {
                std::string type, name, desc;
                double price = 0;
                int stock = 0;

                std::cout << "Type(Book/Food/Clothes) or q for quit to cancel: ";
                if (!ReadInput(type)) continue;
                if (type != "Book" && type != "Food" && type != "Clothes") {
                    std::cout << "Invalid type. Add-product aborted.\n"; continue;
                }

                std::cout << "Name (or q for quit): ";
                if (!ReadInput(name)) continue;
                std::cout << "Description (or q for quit): ";
                std::getline(std::cin >> std::ws, desc);
                if (desc == "q" || desc == "quit") continue;

                std::string line;
                std::cout << "Price (or q for quit): ";
                if (!std::getline(std::cin >> std::ws, line) || line == "q" || line == "quit") continue;
                std::istringstream price_ss(line);
                if (!(price_ss >> price)) { std::cout << "Invalid price.\n"; continue; }

                std::cout << "Stock (or q for quit): ";
                if (!std::getline(std::cin >> std::ws, line) || line == "q" || line == "quit") continue;
                std::istringstream stock_ss(line);
                if (!(stock_ss >> stock)) { std::cout << "Invalid stock.\n"; continue; }

                if (price <= 0 || stock < 0) {
                    std::cout << "Price/stock out of range.\n"; continue;
                }

                std::shared_ptr<Product> pr;
                if      (type == "Book")    pr = std::make_shared<Book>(name, desc, price, stock);
                else if (type == "Food")    pr = std::make_shared<Food>(name, desc, price, stock);
                else                        pr = std::make_shared<Clothes>(name, desc, price, stock);

                pr->SetOwner(current_user->GetUsername());
                product_manager.AddProduct(pr);
                product_manager.SaveToFile();
                std::cout << "Product added successfully.\n";

            /* 6. change password */
            } else if (choice == "6") {
                std::string oldp, newp;
                std::cout << "Current password (or q for quit): ";
                if (!ReadInput(oldp)) continue;
                if (!current_user->CheckPassword(oldp)) { std::cout << "Incorrect.\n"; continue; }

                std::cout << "New password (or q for quit): ";
                if (!ReadInput(newp)) continue;

                current_user->SetPassword(newp);
                user_manager.SaveToFile();
                std::cout << "Password changed.\n";

            /* 7. manage own products */
            } else if (choice == "7" && current_user->GetUserType() == "Seller") {
                auto mine = product_manager.GetProductsBySeller(current_user->GetUsername());
                if (mine.empty()) { std::cout << "You have no products.\n"; continue; }

                for (size_t i = 0; i < mine.size(); ++i) {
                    auto p = mine[i];
                    std::cout << i + 1 << ". ";
                    ShowProduct(p, product_manager);
                }

                std::string sel_input;
                std::cout << "Index (0 cancel, or q for quit): ";
                if (!ReadInput(sel_input)) continue;

                int sel = 0;
                try { sel = std::stoi(sel_input); } catch (...) { std::cout << "Invalid index.\n"; continue; }
                if (sel < 1 || sel > static_cast<int>(mine.size())) { std::cout << "Index out of range.\n"; continue; }

                auto p = mine[sel - 1];
                std::cout << "Fields: name / desc / price / stock / show / done\n";

                std::string cmd;
                while (true) {
                    std::cout << "(edit)> ";
                    std::cin >> cmd;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    if (cmd == "name") {
                        std::string v;
                        std::cout << "New name: ";
                        if (!ReadInput(v)) break;
                        p->SetName(v);

                    } else if (cmd == "desc") {
                        std::string v;
                        std::cout << "New description: ";
                        std::getline(std::cin, v);
                        if (v.empty()) std::getline(std::cin, v);
                        p->SetDescription(v);

                    } else if (cmd == "price") {
                        std::string v;
                        std::cout << "New price (original price): ";
                        if (!ReadInput(v)) break;
                        try {
                            double new_price = std::stod(v);
                            p->SetRawPrice(new_price);     // 只改原价
                        } catch (...) {
                            std::cout << "Invalid.\n";
                        }
                    } else if (cmd == "stock") {
                        std::string v;
                        std::cout << "New stock: ";
                        if (!ReadInput(v)) break;
                        try { p->SetStock(std::stoi(v)); } catch (...) { std::cout << "Invalid stock.\n"; }

                    } else if (cmd == "show") {
                        ShowProduct(p, product_manager);

                    } else if (cmd == "done") {
                        product_manager.SaveToFile();
                        std::cout << "Changes saved.\n";
                        break;

                    } else {
                        std::cout << "Unknown command.\n";
                    }
                }

            /* 8. apply discount (seller) */
            } else if (choice == "8" && current_user->GetUserType() == "Seller") {
                std::cout << "Current discount status:\n";
                for (const std::string& cat : {"Book", "Food", "Clothes"}) {
                    if (product_manager.HasDiscount(cat)) {
                        double rate = product_manager.GetDiscountRate(cat);
                        int percent = static_cast<int>((1.0 - rate) * 100 + 0.5);
                        std::cout << " - " << cat << ": " << percent << "% off\n";
                    } else {
                        std::cout << " - " << cat << ": no discount\n";
                    }
                }

                std::string action;
                std::cout << "Enter 'set' to apply discount, 'cancel' to remove discount: ";
                if (!ReadInput(action)) continue;
                if (action != "set" && action != "cancel") { std::cout << "Unknown command.\n"; continue; }

                std::string category;
                std::cout << "Category to target (Book/Food/Clothes): ";
                if (!ReadInput(category)) continue;
                if (category != "Book" && category != "Food" && category != "Clothes") {
                    std::cout << "Invalid category.\n"; continue;
                }

                if (action == "set") {
                    std::string rate_input;
                    std::cout << "Discount rate (e.g., 0.8 for 20% off): ";
                    if (!ReadInput(rate_input)) continue;
                    try {
                        double rate = std::stod(rate_input);
                        if (rate <= 0 || rate >= 1) { std::cout << "Rate must be > 0 and < 1.\n"; continue; }
                        product_manager.ApplyDiscount(category, rate);
                        std::cout << "Discount applied to " << category << ".\n";
                    } catch (...) { std::cout << "Invalid number.\n"; }

                } else { /* cancel */
                    if (!product_manager.HasDiscount(category)) {
                        std::cout << "No discount applied to " << category << ".\n"; continue;
                    }
                    product_manager.CancelDiscount(category);
                    std::cout << "Discount canceled for " << category << ".\n";
                }
            
            // 9. Manage shopping cart
            } else if (choice == "9" && current_user->GetUserType() == "Customer") {
                static ShoppingCart cart;
                std::cout << "\n--- Shopping Cart ---\n";
                for (const auto& [id, item] : cart.GetItems()) {
                    std::cout << "- " << item.GetProduct()->GetName()
                              << " x" << item.GetQuantity()
                              << " = " << item.GetTotalPrice() << '\n';
                }
                std::cout << "Options: add / remove / update / order / clear / done\n";
                std::string cmd;
                while (true) {
                    std::cout << "(cart)> ";
                    std::cin >> cmd;
                    if (cmd == "add") {
                    std::cout << "\n--- Available Products ---\n";
                    for (const auto& p : product_manager.GetAllProducts())
                        ShowProduct(p, product_manager);

                    std::cout << "Please enter Product Name (e.g., Milk, C++ Primer):\n";

                    std::string pname;
                    std::cout << "Product Name: ";
                    std::getline(std::cin >> std::ws, pname);

                    std::string qty_input;
                    std::cout << "Quantity: ";
                    std::cin >> qty_input;

                    std::stringstream ss(qty_input);
                    int qty = 0;
                    if (!(ss >> qty) || qty <= 0) {
                        std::cout << "Invalid quantity. Must be a positive integer.\n";
                        continue;
                    }

                    auto p = product_manager.GetProductByName(pname);
                    if (p) cart.AddItem(p, qty);
                    else std::cout << "Product not found.\n";

                    } else if (cmd == "remove") {
                        std::string pid;
                        std::cout << "Product ID: "; std::cin >> pid;
                        cart.RemoveItem(pid);
                    } else if (cmd == "update") {
                        std::string pid;
                        int qty = 0;
                        std::cout << "Product ID: "; std::cin >> pid;
                        std::cout << "New Quantity: "; std::cin >> qty;
                        cart.UpdateQuantity(pid, qty);
                    } else if (cmd == "order") {
                        std::string oid = order_manager.CreateOrder(*dynamic_cast<Customer*>(current_user.get()), cart);
                        if (!oid.empty()) {
                            std::cout << "Order created: " << oid << '\n';
                            cart.Clear();
                        }
                    } else if (cmd == "clear") {
                        cart.Clear();
                    } else if (cmd == "done") break;
                    else std::cout << "Unknown command.\n";
                }
            }

            // 10. Pay order
            else if (choice == "10" && current_user->GetUserType() == "Customer") {
                std::string oid;
                std::cout << "Order ID to pay: ";
                std::cin >> oid;

                std::cout << "Action: pay / cancel\n> ";
                std::string action;
                std::cin >> action;

                if (action == "pay") {
                    bool ok = order_manager.PayOrder(*dynamic_cast<Customer*>(current_user.get()), oid);
                    if (ok) std::cout << "Payment success. Balance updated, inventory updated.\n";
                    else std::cout << "Payment failed.\n";
                } else if (action == "cancel") {
                    bool ok = order_manager.CancelOrder(*dynamic_cast<Customer*>(current_user.get()), oid);
                    if (ok) std::cout << "Order canceled. Frozen stock restored.\n";
                    else std::cout << "Cancel failed.\n";
                } else {
                    std::cout << "Unknown action.\n";
                }
            }

            // 由于 User 是一个基类，而当前用户可能是 Customer 或 Seller，所以将 User* 强制转换为 Customer*，
            // 前提是当前用户确实是 Customer 类型。如果不是，转换将返回 nullptr

            // 11. View all orders
            else if (choice == "11") {
                std::cout << "\nAll Orders:\n";
                for (const auto& order : order_manager.GetAllOrders()) {
                    std::cout << "- " << order.GetId()
                              << " [" << (order.GetStatus() == OrderStatus::Paid ? "Paid" :
                                         order.GetStatus() == OrderStatus::Unpaid ? "Unpaid" : "Failed")
                              << "] Amount: " << order.GetTotalAmount() << '\n'; 
                }
            /* 12. logout */
            } else if (choice == "12") {
                current_user.reset();
            }

            else {
                std::cout << "Invalid choice.\n";
            }
        }
    }

    user_manager.SaveToFile();
    product_manager.SaveToFile();
    order_manager.SaveToFile(ORDER_FILE); 
    return 0;
}
