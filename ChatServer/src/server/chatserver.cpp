#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <string>

#include "chatserver.hpp"
#include "chatservice.hpp"

#include "json.hpp"
using json = nlohmann::json;
//using namespace std;
//using namespace std::placeholders;

/*  固定流程与指令，基本上无需修改什么
1. 组合TcpServer对象
2. 创建EventLoop事件循环对象的指针
3. 确定TcpServer构造函数的参数，从而确定ChatServer的构造函数
4. 在构造函数中，注册处理（用户连接与断开、用户可读写事件）的回调函数
5. 设置合适的服务器线程数量，moduo库会自己划分I/O线程和工作线程
*/


ChatServer::ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr, const std::string& nameArg)
    :_server(loop, listenAddr, nameArg), _loop(loop){
    // 给服务器注册，（用户连接的创建与断开）的回调函数
    _server.setConnectionCallback( std::bind(&ChatServer::onConnection, this, std::placeholders::_1) );     //这里 onConnection 函数是成员函数，隐藏的第一个参数是 this 指针，
                                                                                    //与 setConnectionCallback 所需的参数（函数指针）列表不同
                                                                                    //使用 bind 绑定器可以解决？
                                                                                    //通过指定第一个参数为 this，这样，该成员函数仅需要一个显式参数了

    // 给服务器注册，（用户读写事件）的回调函数
    _server.setMessageCallback( std::bind(&ChatServer::onMessage, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3) );   //疑问？？？参数占位符，需要学习理解

    //设置服务器端的线程数量，如图，一个I/O线程，3个工作线程
    _server.setThreadNum(4);
}
 
ChatServer::~ChatServer(){

}

void ChatServer::start(){
    _server.start();
} 

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn){        //上报给服务层，链接信息
    if(conn->connected()){
        //std::cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online " << std::endl;
    }
    else{
        ChatService::instance()->clientCloseException(conn);        //这玩意算正常退出还是算异常退出？ 应该是异常登陆，因为不是主动发消息向服务器通知下线操作
        conn->shutdown();
    }
}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp time){

    std::string buf = buffer->retrieveAllAsString();

    //消息解析部分
    //客户端与服务器之间的消息类型是提前规定好了的，进行业务标识
    //json数据反序列化
    json js = json::parse(buf);
    
    ChatService::MsgHandler msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);

    //网络与业务解耦
    //面向接口（抽象基类）的编程 或是 基于回调函数
}


