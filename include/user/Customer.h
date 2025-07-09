#pragma once
#include "User.h"

class Customer : public User {
public:
    Customer(const std::string& user_name,
             const std::string& password,
             double balance = 0.0);

    std::string GetUserType() const override { return "Customer"; }
};
