#include "user/User.h"

User::User(const std::string& user_name,
           const std::string& password,
           double balance)
    : user_name(user_name), password(password), balance(balance) {}

std::string User::GetUsername() const { return user_name; }
const std::string& User::GetPassword() const { return password; }

bool User::CheckPassword(const std::string& pwd) const { return pwd == password; }
void User::SetPassword(const std::string& new_pwd)     { password = new_pwd; }

/* 余额 */
double User::GetBalance() const { return balance; }

void User::Recharge(double amount){ if(amount>0) balance += amount; }
bool User::Consume(double amount){
    if(amount>balance) return false;
    balance -= amount; return true;
}
/* 通用 + / - */
void User::AddBalance(double amount){ if(amount>0) balance += amount; }
void User::DeductBalance(double amount){ if(amount>=0 && amount<=balance) balance -= amount; }
