#pragma once
#include <string>

// Base class for all products
class Product {
protected:
    std::string id;           
    std::string name;         
    std::string description; 
    double price;              
    int stock;                 
    int frozen_stock = 0;      
    std::string owner_name;   

public:
    Product(const std::string& name,
            const std::string& description,
            double price,
            int stock);
    virtual ~Product() = default;

    std::string GetId()   const;
    std::string GetName() const;
    std::string GetDescription() const;
    int    GetStock()          const;
    int    GetFrozenStock()    const;
    int    GetAvailableStock() const;        
    double GetRawPrice()       const;
    std::string GetOwner()     const;

    void SetId(const std::string& id);
    void SetName(const std::string& new_name)        { name = new_name; }
    void SetDescription(const std::string& new_desc) { description = new_desc; }
    void SetRawPrice(double p)                       { price = p; }
    void SetStock(int new_stock);
    void SetPrice(double new_price);
    void SetOwner(const std::string& owner);

    void FreezeStock(int quantity);
    void UnfreezeStock(int quantity);
    void CommitFrozenStock();

    virtual double GetPrice() const = 0;
    virtual std::string GetCategory() const = 0;
};
