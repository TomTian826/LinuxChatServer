#include <vector>

#include "friendmodel.hpp"
#include "connectionpool.hpp"

const int MAX_SQL_SIZE = 1024;

bool FriendModel::insert(int userId, int friendId){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "insert into friend values('%d','%d')",        //这不应该是互相是好友吗？
            userId, friendId);
    
    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){
            return true;
        }
    }
    return false;
}

std::vector<User> FriendModel::query(int userId){

    std::vector<User> ret;
    User tempUser;

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid = '%d'", userId);

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        MYSQL_RES* res = curConn->query(sql);
        if(res != nullptr){
            MYSQL_ROW resRow;
            while((resRow = mysql_fetch_row(res)) != nullptr){
                tempUser.setId(atoi(resRow[0]));
                tempUser.setName(resRow[1]);
                tempUser.setState(resRow[2]);

                ret.push_back(tempUser);
            }
            mysql_free_result(res);
        }
    }
    return ret;
}

bool FriendModel::erase(int userId, int friendId){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "delete from friend where userid = '%d' friendid = '%d'",
            userId, friendId);

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){
            return true;
        }
    }
    return false;
}