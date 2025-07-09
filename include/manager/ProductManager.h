#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "product/Product.h"
#include "product/Book.h"
#include "product/Food.h"
#include "product/Clothes.h"

class ProductManager {
private:
    std::vector<std::shared_ptr<Product>> products;
    std::string product_file;
    std::unordered_map<std::string,double> discount_map;   // category -> rate

public:
    explicit ProductManager(const std::string& file_path);

    void AddProduct(std::shared_ptr<Product> product);

    const std::vector<std::shared_ptr<Product>>& GetAllProducts() const;
    std::vector<std::shared_ptr<Product>> SearchByName(const std::string& keyword) const;
    std::vector<std::shared_ptr<Product>> GetProductsBySeller(const std::string& seller) const;

    void ApplyDiscount(const std::string& category,double rate);
    void CancelDiscount(const std::string& category);
    bool  HasDiscount(const std::string& category) const;
    double GetDiscountRate(const std::string& category) const;
    double GetDisplayPrice(const std::shared_ptr<Product>& p) const;

    void LoadFromFile();
    void SaveToFile() const;

    std::shared_ptr<Product> GetProductById(const std::string& id) const;
    std::shared_ptr<Product> GetProductByName(const std::string& name) const;
};
