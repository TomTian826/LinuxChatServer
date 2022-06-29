#include <muduo/base/Logging.h>

#include "offlinemsgmodel.hpp"
#include "connectionpool.hpp"

const int MAX_SQL_SIZE = 1024;

bool OfflineMsgModel::insert(int userid, std::string msg){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "insert into offlinemessage values('%d','%s')",
            userid, msg.c_str());

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){
            return true;
        }
    }
    return false;
}

std::vector<std::string> OfflineMsgModel::query(int userid){

    std::vector<std::string> ret;

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "select message from offlinemessage where userid = '%d'", userid);

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        MYSQL_RES* res = curConn->query(sql);
        if(res != nullptr){
            MYSQL_ROW resRow;
            while((resRow = mysql_fetch_row(res)) != nullptr){
                ret.push_back(resRow[0]);
            }
            mysql_free_result(res);
        }
    }
    return ret;
}

bool OfflineMsgModel::erase(int userid){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "delete from offlinemessage where userid = '%d'", userid);

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){
            return true;
        }
    }
    return false;
}



