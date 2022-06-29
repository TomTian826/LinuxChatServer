#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <vector>

#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

#include "public.hpp"

#include "json.hpp"
using json = nlohmann::json;

// 处理不同类型的消息回调函数
//using MsgHandler = std::function<void (const muduo::net::TcpConnectionPtr& conn, json &js, muduo::Timestamp time)>;

// 聊天服务器业务类，无需多个实例，单例模式实现即可
class ChatService{
public:
    using MsgHandler = std::function<void (const muduo::net::TcpConnectionPtr& conn, json &js, muduo::Timestamp time)>;

    static ChatService* instance();
    MsgHandler getHandler(int msgId);

    void login(const muduo::net::TcpConnectionPtr& conn, json &js, muduo::Timestamp time);
    void logout(const muduo::net::TcpConnectionPtr& conn, json &js, muduo::Timestamp time);
    void regist(const muduo::net::TcpConnectionPtr& conn, json &js, muduo::Timestamp time);

    void clientCloseException(const muduo::net::TcpConnectionPtr& conn);
    void resetAllState();   // 服务器异常退出后，业务 重置方法

    void oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
    void addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void groupChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
    void createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
    void addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void handleRedisSubscribeMessage(int, std::string);

private:
    ChatService();  //构造函数私有化 ———— 单例模式

    std::unordered_map<int, MsgHandler> _msgHandlerMap;     //存储消息类型与对应的业务处理方法
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap;     //存储用户 id 与其建立的链接
    //std::unordered_map<muduo::net::TcpConnectionPtr, int> _userConnMapReverse;     先存疑，不知道这种数据结构怎么 hash

    std::mutex _connMutex;                                  //保证用户连接表的线程安全  

    UserModel _userModel;       //数据操作类对象，相当于服务类中包含了一个对象，这个对象专门用来与数据库中的 user 表进行交互，也就是说提供了各种函数接口
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    Redis _redis;
};

#endif