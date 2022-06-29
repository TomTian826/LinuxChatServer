#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"

class GroupModel{
public:
    bool createGroup(Group& newGroup);
    bool addToGroup(int userId, int groupId, std::string groupRole);
    std::vector<Group> queryMyGroups(int userId);
    std::vector<int> queryThisGroup(int userId, int groupId);

private:
};

#endif