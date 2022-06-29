#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>

#include "user.hpp"

class FriendModel{
public:
    bool insert(int userId, int friendId);
    bool erase(int userId, int friendId);

    std::vector<User> query(int userId);
private:
};
#endif