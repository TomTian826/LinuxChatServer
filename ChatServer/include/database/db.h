#ifndef DB_H
#define DB_H

#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <string>
#include <ctime>

class MySQL{
public:
    MySQL();
    ~MySQL();
    //bool connect();
    bool connect(std::string server, unsigned short port, std::string user, std::string password, std::string dbname);
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);
    MYSQL* getConnection();

    void updateAliveTime();
    clock_t getAliveTime() const;

private:
    MYSQL* _conn;
    clock_t _aliveTime;         //记录其在连接池中，成为空闲连接的起始时间
};

#endif