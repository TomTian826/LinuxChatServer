#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <muduo/base/Logging.h>

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>
#include <chrono>

#include "db.h"

class ConnectionPool{

public:
    static ConnectionPool* instance();
    //选择使用智能指针，重载智能指针的删除器，出作用域时自动归还至连接池中
    std::shared_ptr<MySQL> getConn();

private:
    ConnectionPool();
    
    //单独的线程,用于生产可用的连接
    void produceConn();

    //单独的线程,用于回收长时间未被使用的空闲的连接
    void reduceConn();

    bool loadConfigFile();

    std::string _server;
    unsigned short _port;
    std::string _user;
    std::string _password;
    std::string _dbname;

/*
    std::string _server = "127.0.0.1";
    unsigned short _port = 3306;
    std::string _user = "root";
    std::string _password = "projectMySQL98!";
    std::string _dbname = "chat";
*/

    int _initSize;          //连接池的初始连接量
    int _maxSize;           //连接池的最大连接量
    int _maxIdleTime;       //连接池的多余连接的最大空闲时间
    int _connTimeout;       //连接池的获得连接的超时时间

    std::queue<MySQL*> _connQueue;      //多线程安全连接池（生产者消费者模型）
    std::atomic_int _curConnCnt;        //这个为啥要单独一个变量？或者说查询 _connQueue.size() 会有问题？
    std::mutex _queueMutex;
    std::condition_variable _cv;         //设置条件变量，用于连接生产线程以及连接消费线程的通信
};


#endif