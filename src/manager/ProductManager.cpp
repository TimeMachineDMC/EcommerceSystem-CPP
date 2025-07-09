#include "manager/ProductManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

/* ----------- 工具：字符串切割 ----------- */
namespace {
std::vector<std::string> Split(const std::string& s,char delim='|')
{
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss,item,delim)) out.push_back(item);
    return out;
}
} // namespace

/* ----------- ctor ----------- */
ProductManager::ProductManager(const std::string& path)
    : product_file(path) {}

/* ----------- 基本查询 ----------- */
void ProductManager::AddProduct(std::shared_ptr<Product> p){ products.push_back(std::move(p)); }

const std::vector<std::shared_ptr<Product>>&
ProductManager::GetAllProducts() const { return products; }

std::vector<std::shared_ptr<Product>>
ProductManager::SearchByName(const std::string& k) const
{
    std::vector<std::shared_ptr<Product>> res;
    for(auto&p:products)
        if(p->GetName().find(k)!=std::string::npos) res.push_back(p);
    return res;
}

std::vector<std::shared_ptr<Product>>
ProductManager::GetProductsBySeller(const std::string& seller) const
{
    std::vector<std::shared_ptr<Product>> res;
    for(auto&p:products)
        if(p->GetOwner()==seller) res.push_back(p);
    return res;
}

/* ----------- 折扣 ----------- */
void ProductManager::ApplyDiscount(const std::string& cat,double r){ discount_map[cat]=r; }
void ProductManager::CancelDiscount(const std::string& cat){ discount_map.erase(cat); }
bool  ProductManager::HasDiscount(const std::string& cat) const{ return discount_map.count(cat); }
double ProductManager::GetDiscountRate(const std::string& cat) const{
    auto it=discount_map.find(cat); return it==discount_map.end()?1.0:it->second;
}
double ProductManager::GetDisplayPrice(const std::shared_ptr<Product>& p) const{
    return p->GetRawPrice()*GetDiscountRate(p->GetCategory());
}

/* ----------- Save ----------- */
void ProductManager::SaveToFile() const
{
    if(auto parent=std::filesystem::path(product_file).parent_path(); !parent.empty())
        std::filesystem::create_directories(parent);

    std::ofstream fout(product_file,std::ios::trunc);
    if(!fout) throw std::runtime_error("Open "+product_file+" fail");

    /* 折扣行：D|category|rate */
    for(auto&[cat,r]:discount_map)
        fout<<"D|"<<cat<<'|'<<r<<'\n';

    /* 商品行：P|Cat|Name|Desc|Price|Stock|Owner|Id */
    for(auto&p:products)
        fout<<"P|"<<p->GetCategory()<<'|'
            <<p->GetName()       <<'|'
            <<p->GetDescription()<<'|'
            <<p->GetRawPrice()   <<'|'
            <<p->GetStock()      <<'|'
            <<p->GetOwner()      <<'|'
            <<p->GetId()         <<'\n';
}

/* ----------- Load ----------- */
void ProductManager::LoadFromFile()
{
    products.clear(); discount_map.clear();

    std::ifstream fin(product_file);
    if(!fin) return;

    std::string line; std::unordered_map<std::string,int> id_cnt;
    int lineno = 0;
    while(std::getline(fin,line)){
        lineno++;
        if(line.empty()) continue;
        auto parts=Split(line);
        try {
            if(parts[0]=="D" && parts.size()==3){
                discount_map[parts[1]]=std::stod(parts[2]);
            }
            else if(parts[0]=="P" && parts.size()>=7){
                const std::string& cat  = parts[1];
                const std::string& name = parts[2];
                const std::string& desc = parts[3];
                double price   = std::stod(parts[4]);
                int    stock   = std::stoi(parts[5]);
                const std::string& owner = parts[6];

                std::shared_ptr<Product> p;
                if      (cat=="Book")    p=std::make_shared<Book>(name,desc,price,stock);
                else if (cat=="Food")    p=std::make_shared<Food>(name,desc,price,stock);
                else if (cat=="Clothes") p=std::make_shared<Clothes>(name,desc,price,stock);
                else continue;

                p->SetOwner(owner);

                if(parts.size()>=8) p->SetId(parts[7]);
                else               p->SetId(cat+"-"+std::to_string(id_cnt[cat]++));

                products.push_back(std::move(p));
            }
        } catch(const std::exception& e){
            std::cerr << "Product file parse error on line "<<lineno<<": " << line << "\n";
            std::cerr << "    what(): " << e.what() << '\n';
        }
    }
}


/* ----------- helpers ----------- */
std::shared_ptr<Product> ProductManager::GetProductById(const std::string& id) const{
    for(auto& p:products) if(p->GetId()==id) return p; return nullptr;
}
std::shared_ptr<Product> ProductManager::GetProductByName(const std::string& n) const{
    for(auto& p:products) if(p->GetName()==n) return p; return nullptr;
}
