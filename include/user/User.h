#pragma once
#include <string>
class User {
protected:
    std::string user_name;      
    std::string password;       // 明文密码
    double      balance;      

public:
    User(const std::string& user_name,
         const std::string& password,
         double balance);

    virtual ~User() = default;

    std::string        GetUsername() const;
    const std::string& GetPassword() const;        
    bool   CheckPassword(const std::string& pwd) const;
    void   SetPassword(const std::string& new_pwd);

    double GetBalance() const;
    void   Recharge(double amount);
    bool   Consume(double amount);                   
    void   AddBalance(double amount);                
    void   DeductBalance(double amount);           

    virtual std::string GetUserType() const = 0;
};
