#include <signal.h>
#include <iostream>

#include "chatserver.hpp"
#include "chatservice.hpp"

// 处理服务器端 ctrl + c 强行结束进程后，重置现有连接对应用户的 state
void resetHandler(int){
    ChatService::instance()->resetAllState();
    exit(0);
}

int main(int argc, char** argv){
    signal(SIGINT, resetHandler);

    if(argc < 3){
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std:: endl;
        exit(-1);
    }

    char* serverIp = argv[1];
    uint16_t serverPort = atoi(argv[2]);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(serverIp, serverPort);        

    ChatServer myServer(&loop, addr, "projectServer");

    myServer.start();       // listenfd epoll_ctl => epoll
    loop.loop();            //epoll_wait ———— 以阻塞方式等待新用户连接、已连接用户的读写事件

    return 0;
}