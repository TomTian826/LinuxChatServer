#ifndef USER_H
#define USER_H

#include <string>

// 匹配 User 表的 ORM 类
class User{
public:
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
    : _id(id), _name(name), _password(password), _state(state){}

    void setId (int newId){
        _id = newId;
    }
    void setName (std::string newName){
        _name = newName;
    }
    void setPassword (std::string newPassword){
        _password = newPassword;
    }
    void setState (std::string newState){
        _state = newState;
    }

    int getId () const {
        return _id;
    }
    std::string getName () const {
        return _name;
    }
    std::string getPassword () const {
        return _password;
    }
    std::string getState () const {
        return _state;
    }


private:
    int _id;             //居然是一个整型？ -2147483648 ~ 21474 83647
    std::string _name;
    std::string _password;
    std::string _state;

    //那么为啥这里不储存一个好友列表？group 就需要保存一个成员列表
};

#endif