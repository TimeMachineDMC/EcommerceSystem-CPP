#pragma once
#include <string>

/* 抽象用户基类 */
class User {
protected:
    std::string user_name;      // 用户名
    std::string password;       // 明文密码（示例作业级别）
    double      balance;        // 账户余额

public:
    User(const std::string& user_name,
         const std::string& password,
         double balance);

    virtual ~User() = default;

    /* 基本信息 */
    std::string        GetUsername() const;
    const std::string& GetPassword() const;          // 只读访问
    bool   CheckPassword(const std::string& pwd) const;
    void   SetPassword(const std::string& new_pwd);

    /* 余额相关 */
    double GetBalance() const;
    void   Recharge(double amount);
    bool   Consume(double amount);                   // 余额不足返回 false
    void   AddBalance(double amount);                // 通用入账
    void   DeductBalance(double amount);             // 通用扣款（余额足）

    /* 用户类型（Customer / Seller） */
    virtual std::string GetUserType() const = 0;
};
