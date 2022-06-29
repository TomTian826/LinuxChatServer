#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>

#include "groupuser.hpp"

class Group{
public:
    Group(int id = -1, std::string groupName = "", std::string groupDesc = "")
    : _id(id), _groupName(groupName), _groupDesc(groupDesc){}

    void setId(int newId){
        _id = newId;
    }
    void setGroupName(std::string newGroupName){
        _groupName = newGroupName;
    }
    void setGroupDesc(std::string newGroupDesc){
        _groupDesc = newGroupDesc;
    }
    

    int getId() const{
        return _id;
    }
    std::string getGroupName() const{
        return _groupName;
    }
    std::string getGroupDesc() const{
        return _groupDesc;
    }
    std::vector<GroupUser>& getGroupUsers() {
        return _groupUsers;
    }

private:
    int _id;
    std::string _groupName;
    std::string _groupDesc;
    std::vector<GroupUser> _groupUsers;
};

#endif