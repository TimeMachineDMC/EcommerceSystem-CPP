#pragma once
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include "user/User.h"
#include "user/Customer.h"
#include "user/Seller.h"

/* 线程安全的用户管理器 */
class UserManager {
private:
    std::vector<std::shared_ptr<User>> users;
    std::string user_file;
    mutable std::mutex mtx;                 // 保护 users

public:
    explicit UserManager(const std::string& file_path);

    bool RegisterUser(const std::string& name,
                      const std::string& pwd,
                      const std::string& type);

    std::shared_ptr<User> Login(const std::string& name,
                                const std::string& pwd) const;

    void LoadFromFile();
    void SaveToFile() const;

    const std::vector<std::shared_ptr<User>>& GetAllUsers() const;
    std::shared_ptr<User> FindByUsername(const std::string& name) const;
};
