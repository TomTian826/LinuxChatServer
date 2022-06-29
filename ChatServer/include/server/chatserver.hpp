#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>

class ChatServer{
public:
    ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr, const std::string& nameArg);  //事件循环  IP + port  服务器名称，用于初始化服务器对象
    ~ChatServer();

    void start();   //服务器启动服务

private:
    muduo::net::TcpServer _server;
    muduo::net::EventLoop* _loop;

    void onConnection(const muduo::net::TcpConnectionPtr&);     //疑问？？？TcpConnectionPtr 这个类型的作用是什么
    void onMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
};

#endif

