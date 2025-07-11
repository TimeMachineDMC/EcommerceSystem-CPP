#pragma once
#include "User.h"

class Seller : public User {
public:
    Seller(const std::string& user_name,
           const std::string& password,
           double balance = 0.0);

    std::string GetUserType() const override { return "Seller"; }
};
