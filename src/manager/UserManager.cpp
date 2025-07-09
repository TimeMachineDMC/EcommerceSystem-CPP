#include "manager/UserManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

UserManager::UserManager(const std::string& fp) : user_file(fp) {}

bool UserManager::RegisterUser(const std::string& name,
                               const std::string& pwd,
                               const std::string& type)
{
    std::lock_guard<std::mutex> lock(mtx);
    for(const auto& u:users)
        if(u->GetUsername()==name) return false;

    if(type=="Customer")
        users.emplace_back(std::make_shared<Customer>(name,pwd));
    else if(type=="Seller")
        users.emplace_back(std::make_shared<Seller>(name,pwd));
    else return false;

    SaveToFile();  
    return true;
}

std::shared_ptr<User> UserManager::Login(const std::string& name,
                                         const std::string& pwd) const
{
    std::lock_guard<std::mutex> lock(mtx);
    for(const auto& u:users)
        if(u->GetUsername()==name && u->CheckPassword(pwd))
            return u;
    return nullptr;
}

void UserManager::LoadFromFile()
{
    std::lock_guard<std::mutex> lock(mtx);
    users.clear();

    std::ifstream fin(user_file);
    if(!fin) {
        std::cerr << "[UserManager] Failed to open " << user_file << '\n';
        return;
    }

    std::string name,pwd,type;
    double bal;
    int count=0;
    while(fin>>name>>pwd>>bal>>type){
        if(type=="Customer")
            users.emplace_back(std::make_shared<Customer>(name,pwd,bal));
        else if(type=="Seller")
            users.emplace_back(std::make_shared<Seller>(name,pwd,bal));
        ++count;
    }
    std::cerr << "[UserManager] Loaded "<<count<<" users.\n";
}
void UserManager::SaveToFile() const
{
    std::lock_guard<std::mutex> lock(mtx);
    std::ofstream fout(user_file,std::ios::trunc);
    if(!fout) {
        std::cerr << "[UserManager] Failed to save " << user_file << '\n';
        return;
    }
    for(const auto& u:users)
        fout<<u->GetUsername()<<' '
            <<u->GetPassword()<<' '
            <<u->GetBalance()<<' '
            <<u->GetUserType()<<'\n';
}

const std::vector<std::shared_ptr<User>>&
UserManager::GetAllUsers() const { return users; }

std::shared_ptr<User> UserManager::FindByUsername(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(mtx);
    for(const auto& u:users)
        if(u->GetUsername()==name) return u;
    return nullptr;
}
