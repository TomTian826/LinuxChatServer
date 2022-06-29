#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_h

#include <vector>
#include <string>

class OfflineMsgModel{
public:
    bool insert(int userId, std::string msg);
    bool erase(int userId);

    std::vector<std::string> query(int userId);
private:
};


#endif

