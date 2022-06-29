#include "db.h"

//说这个建立连接的相关文本是从配置文件中读取出来的，所以到底是在哪进行这些变量的声明？
//或者说，这段代码就是只进行，以固定的用户访问固定的数据库，感觉可以进一步实现代码复用，但这个业务中确实只会用到有限的数据库，和其中有限的表

/*
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "projectMySQL98!";
static std::string dbname = "chat";
// */

MySQL::MySQL(){
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL(){
    if(_conn != nullptr){
        mysql_close(_conn);
    }
}

//有些不解，为啥返回值是 bool，而看上去是返回了一个 MYSQL* （MYSQL对象指针）
//c 语言中没有 bool 类型，是 int 的延伸 —— 0（NULL） 与 非 0

//bool MySQL::connect(){
bool MySQL::connect(std::string server, unsigned short port, std::string user, std::string password, std::string dbname){
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if(p != nullptr){
        mysql_query(_conn, "set names gbk");
        //编码设置，默认是 ASCII

        LOG_INFO << "connect to mysql successed";
    }
    else{
        LOG_INFO << "connect to mysql failed";
    }
    return p;
}

bool MySQL::update(std::string sql){
    if(mysql_query(_conn, sql.c_str())){
        LOG_INFO << __FILE__ << ": " << __LINE__ << ": " << sql << " update error";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(std::string sql){
    if(mysql_query(_conn, sql.c_str())){
        LOG_INFO << __FILE__ << ": " << __LINE__ << ": " << sql << " query error";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection(){
    return _conn;
}

void MySQL::updateAliveTime(){
    _aliveTime = clock();
}

clock_t MySQL::getAliveTime() const {
    return clock() - _aliveTime;
}