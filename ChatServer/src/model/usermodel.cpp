#include <muduo/base/Logging.h>

#include "usermodel.hpp"
#include "connectionpool.hpp"

const int MAX_SQL_SIZE = 1024;

bool UserModel::insert(User& newUser){

    char sql[MAX_SQL_SIZE] = {'\0'};
    //这是干啥？直接确定这个 SQL 语句？已经默认 newUser 是在业务层处理好的，需要同步到数据库上？
    sprintf(sql, "insert into user(name, password, state) values('%s','%s','%s')",
            newUser.getName().c_str(), newUser.getPassword().c_str(), newUser.getState().c_str());

    LOG_INFO << sql;

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){

            //表的主键 id 是执行SQL语句后，数据库自动生成的，需要将其读出，然后使用 setId() 返回给 newUser 对象
            //完全不理解这个函数的作用 与 其实现细节
            newUser.setId(mysql_insert_id(curConn->getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id){

    User targetUser;

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "select * from user where id = '%d'", id);

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        MYSQL_RES* res = curConn->query(sql);               //这是数据库查询结果
        if(res != nullptr){
            MYSQL_ROW resRow = mysql_fetch_row(res);        //从这个 MYSQL_RES 结果对象中，取出一行数据;可以使用[]按数组访问，元素均是 char* 类型，即数据库查询结果都是char*
            if(resRow != nullptr){
                targetUser.setId(atoi(resRow[0]));
                targetUser.setName(resRow[1]);
                targetUser.setPassword(resRow[2]);
                targetUser.setState(resRow[3]);
                //！！！资源的对应释放
            }
            mysql_free_result(res);
        }
    }
    return targetUser;
    
}

bool UserModel::updateState(User& changedUser){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "update user set state = '%s' where id = '%d' " , changedUser.getState().c_str(), changedUser.getId());

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){
            return true;
        }
    }
    return false;
}

bool UserModel::resetAllState(){
    char sql[MAX_SQL_SIZE] = "update user set state = 'offline' where state = 'online' ";
    
    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if(curConn->update(sql)){
            return true;
        }
    }
    return false;
}