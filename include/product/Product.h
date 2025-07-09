#pragma once
#include <string>

// Base class for all products
class Product {
protected:
    std::string id;            // 商品唯一ID
    std::string name;          // 商品名
    std::string description;   // 商品描述
    double price;              // 原价
    int stock;                 // 当前库存
    int frozen_stock = 0;      // 冻结库存（未支付订单占用）
    std::string owner_name;    // 所属商家名

public:
    Product(const std::string& name,
            const std::string& description,
            double price,
            int stock);
    virtual ~Product() = default;

    /* getter */
    std::string GetId()   const;
    std::string GetName() const;
    std::string GetDescription() const;
    int    GetStock()          const;
    int    GetFrozenStock()    const;
    int    GetAvailableStock() const;          // stock - frozen
    double GetRawPrice()       const;
    std::string GetOwner()     const;

    /* setter */
    void SetId(const std::string& id);
    void SetName(const std::string& new_name)        { name = new_name; }
    void SetDescription(const std::string& new_desc) { description = new_desc; }
    void SetRawPrice(double p)                       { price = p; }
    void SetStock(int new_stock);
    void SetPrice(double new_price);
    void SetOwner(const std::string& owner);

    /* 冻结库存相关 */
    void FreezeStock(int quantity);
    void UnfreezeStock(int quantity);
    void CommitFrozenStock();

    /* 抽象接口 */
    virtual double GetPrice() const = 0;
    virtual std::string GetCategory() const = 0;
};
