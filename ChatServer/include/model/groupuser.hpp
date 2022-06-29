#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

class GroupUser : public User{
public:
    void setRole(std::string newRole){
        _role = newRole;
    }
    std::string getRole() const{
        return _role;
    }

private:
    std::string _role;
};

#endif

/*
class GroupUser{
public:
    GroupUser(int groupId, int userId, int groupRole)
    : _groupId(groupId), _userId(userId), _groupRole(groupRole){}

    void setGroupId(int newGroupId){
        _groupId = newGroupId;
    }
    void setUserId(int newUserId){
        _userId = newUserId;
    }
    void setGroupRole(int newGroupRole){
        _groupRole = newGroupRole;
    }

    int getGroupId() const{
        return _groupId;
    }
    int getUserId() const{
        return _userId;
    }
    int getGroupRole() const{
        return _groupRole;
    }

private:
    int _groupId;
    int _userId;
    int _groupRole;
};
*/


