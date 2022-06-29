#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// User 表的数据操作类，直接与数据库进行交互（增删查改）
// 疑问这个不应该也可以设计成单例模式吗？全局共享一个就行了？
// 哦哦，这是 ChatService 这个单例模式里的一个成员变量
class UserModel{
public:
    bool insert(User& newUser);
    bool erase(int id);
    bool updateState(User& changedUser);
    bool resetAllState();
    
    User query(int id);
private:
};

#endif