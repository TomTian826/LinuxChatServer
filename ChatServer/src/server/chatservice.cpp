#include "chatservice.hpp"

//获取单例对象的接口函数
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

//注册消息以及对应的Handler回调操作
ChatService::ChatService(){
    //instance();
    _msgHandlerMap.insert( { LOGIN_MSG, std::bind( &ChatService::login, this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { LOGOUT_MSG, std::bind(&ChatService::logout, this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { REGIST_MSG, std::bind( &ChatService::regist, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { ONE_CHAT_MSG, std::bind( &ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { ADD_FRIEND_MSG, std::bind( &ChatService::addFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { GROUP_CHAT_MSG, std::bind( &ChatService::groupChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { CREATE_GROUP_MSG, std::bind( &ChatService::createGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert( { ADD_GROUP_MSG, std::bind( &ChatService::addGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    if(_redis.connect()){
        _redis.init_notify_handler(std::bind( &ChatService::handleRedisSubscribeMessage , this, std::placeholders::_1, std::placeholders::_2));
    }
}

ChatService::MsgHandler ChatService::getHandler(int msgId){
    
    std::unordered_map<int, MsgHandler>::iterator targetHandler = _msgHandlerMap.find(msgId);

    if(targetHandler == _msgHandlerMap.end()){
        //std::string errstr = "msgId: " + std::to_string(msgId) + " can not find corresponding handler! ";
        // 这里原打算抛出异常，但会需要对应的异常处理
        // 改为返回一个默认的、进行空操作的 Handler
        return [=](const muduo::net::TcpConnectionPtr& conn, json &js, muduo::Timestamp time){
            LOG_ERROR << "msgId: " << msgId << " can not find corresponding handler! ";
        };
    }

    else{
        return (*targetHandler).second;
    }
}

void ChatService::login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    LOG_INFO << "Do login Service";

    int id = js["id"].get<int>();
    std::string password = js["password"];
    
    json responce;
    responce["msgid"] = LOGIN_MSG_ACK;

    User targetAccount = _userModel.query(id);        //存疑，不知道这一步能不能直接有效地查询到用户想要登录的账号id，即不知道数据库中是否有对应的id数据

    if(targetAccount.getId() == id && targetAccount.getPassword() == password){     //这里肯定有密码的加密储存，数据库中不储存密码的明码
        if(targetAccount.getState() == "online"){
            responce["errno"] = 3;          //该账户已是在线状态，是否需要额外处理？
            responce["errmsg"] = "this account is online";
        }
        else{
            //这里会存在线程安全隐患，只使用必要的锁来保证数据的一致性
            {
                std::lock_guard<std::mutex> lock(_connMutex);
                _userConnMap[id] = conn;        
            }

            _redis.subscribe(id);       //已经在某个服务器上保存了连接，将该用户注册到消息队列中

            responce["errno"] = 0;
            responce["id"] = targetAccount.getId();
            responce["name"] = targetAccount.getName();

            targetAccount.setState("online");
            _userModel.updateState(targetAccount);      //未考虑是否更新成功，要相信 MySQL 服务器提供的数据同步（不人为管理锁）

            //查询该用户是否有离线消息需要读取
            LOG_INFO << "Do login OfflineMsgs Service";
            std::vector<std::string> offlineMsgs = _offlineMsgModel.query(id);
            if(!offlineMsgs.empty()){
                responce["offlinemsgs"] = offlineMsgs;      
                _offlineMsgModel.erase(id);
            }

            //查询该用户的好友信息
            LOG_INFO << "Do login Friends Service";
            std::vector<User> friends = _friendModel.query(id);
            if(!friends.empty()){
                std::vector<std::string> friendsJsonString;
                json tempFriendJson;
                for(const auto& iter : friends){
                    tempFriendJson["id"] = iter.getId();
                    tempFriendJson["name"] = iter.getName();
                    tempFriendJson["state"] = iter.getState();

                    friendsJsonString.push_back(tempFriendJson.dump());
                }

                responce["friends"] = friendsJsonString;
            }

            //查询该用户的群组消息
            LOG_INFO << "Do login Groups Service";
            std::vector<Group> groups = _groupModel.queryMyGroups(id);
            if(!groups.empty()){
                std::vector<std::string> groupsJsonString;
                std::vector<std::string> groupsUsersJsonString;
                json tempGroupJson;
                json tempGroupUserJson;
                for(auto& iter : groups){                                   //疑问，iter 应该是 const Group&
                    tempGroupJson["groupid"] = iter.getId();
                    tempGroupJson["groupname"] = iter.getGroupName();
                    tempGroupJson["groupdesc"] = iter.getGroupDesc();

                    groupsUsersJsonString.clear();
                    for(const auto& groupUser : iter.getGroupUsers()){      //那这里如果将 getGroupUsers() 定义成一个 const 方法，但仍无法调用，所以不知道是 const 方法定义错了，还是逻辑上出什么问题了
                        tempGroupUserJson["id"] = groupUser.getId();
                        tempGroupUserJson["name"] = groupUser.getName();
                        tempGroupUserJson["state"] = groupUser.getState();
                        tempGroupUserJson["role"] = groupUser.getRole();

                        groupsUsersJsonString.push_back(tempGroupUserJson.dump());
                    }
                    tempGroupJson["groupusers"] = groupsUsersJsonString;

                    groupsJsonString.push_back(tempGroupJson.dump());
                }

                responce["groups"] = groupsJsonString;
            }
        }
    }
    else{
        if(targetAccount.getId() == id){
            responce["errno"] = 1;          //密码错误
            responce["errmsg"] = "wrong id or password";
        }
        else{
            responce["errno"] = 2;          //该用户id不存在
            responce["errmsg"] = "this id is not exist";
        }
    }

    conn->send(responce.dump());
}

void ChatService::logout(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userId = js["id"].get<int>();
    //if(userId != -1 && _userConnMap.count(userId)){
    if(_userConnMap.count(userId)){
        {
            std::lock_guard<std::mutex> lock(_connMutex);
            LOG_INFO << "find disconnected link";
            _userConnMap.erase(userId);
        }
    }

    _redis.unsubscribe(userId);
    
    User tempUser(userId, "", "", "offline");
    _userModel.updateState(tempUser);

    LOG_INFO << "changed disconnected user state";
}

void ChatService::regist(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){

    LOG_INFO << "Do regist Service";

    std::string userName = js["name"];
    std::string userPassword = js["password"];

    User newUser(-1, userName, userPassword);
    json responce;
    responce["msgid"] = REGIST_MSG_ACK;

    if(_userModel.insert(newUser)){
        LOG_INFO << "Do regist right";
        responce["errno"] = 0;
        responce["id"] = newUser.getId();
        responce["userName"] = userName;
    }
    else{
        LOG_INFO << "Do regist wrong";
        responce["errno"] = 1;
    }

    conn->send(responce.dump());
}

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn){

    LOG_INFO << "Do clientCloseException Service";

    int userId = -1;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for(const auto& iter : _userConnMap){
            if(iter.second == conn){
                userId = iter.first;
                _userConnMap.erase(userId);

                LOG_INFO << "find disconnected link";
                break;
            }
        }
    }

    _redis.unsubscribe(userId);

    if(userId != -1){
        User targetUser(userId, "", "", "offline");
        _userModel.updateState(targetUser);

        LOG_INFO << "changed disconnected user state";
    }
}

void ChatService::resetAllState(){
    LOG_INFO << "Do reset Service";
    _userModel.resetAllState();

    //有问题，服务器异常退出时，应该有相应的处理
}

void ChatService::oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){

    muduo::net::TcpConnectionPtr targetConn;
    int targetId = js["to"].get<int>();

    {
        std::lock_guard<std::mutex> lock(_connMutex);
        if(_userConnMap.count(targetId)){       //这里需要用锁吗？查找对应链接会有多线程安全隐患吗？
            targetConn = _userConnMap[targetId];
            targetConn->send(js.dump());        //这里不能出锁，因为如果在锁外边，进行通信过程中，不能一直保证连接的有效性
            return;
        }
    }

    //如果没有在 if 里 return，表示没有查到目标 id 的有效连接，说明对方并未连接到这一台服务器上，需要查看目标的状态信息
    //目标在线，只是在其余服务器上建立了连接
    User targetUser = _userModel.query(targetId);
    if(targetUser.getState() == "online"){
        _redis.publish(targetId, js.dump());
        return;
    }
    
    //进行离线信息处理
    if(_offlineMsgModel.insert(targetId, js.dump())){
        LOG_INFO << "message was send, friend is not online";
    }
    else{
        LOG_INFO << "send message error";
    }
}

void ChatService::addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){

    int userId = js["id"].get<int>();
    int friendId = js["friendid"].get<int>();

    json responce;

    if(_friendModel.insert(userId, friendId)){
        LOG_INFO << "add friend success";
    }
    else{
        LOG_INFO << "add friend error";
    }
}

void ChatService::groupChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    
    int userId = js["from"].get<int>();
    int groupId = js["groupid"].get<int>();

    std::vector<int> groupOtherUsers = _groupModel.queryThisGroup(userId, groupId);

    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for(const auto& curId : groupOtherUsers){
            if(_userConnMap.count(curId)){          //在线直接转发
                _userConnMap[curId]->send(js.dump());
            }
            else{
                User targetUser = _userModel.query(curId);
                if(targetUser.getState() == "online"){
                    _redis.publish(curId, js.dump())
;                }
                else{
                    _offlineMsgModel.insert(curId, js.dump());      //存储离线消息
                }
            }
        }
    }
}

void ChatService::createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){

    int userId = js["id"].get<int>();
    std::string groupName = js["groupname"];
    std::string groupDesc = js["groupdesc"];

    Group newGroup(-1, groupName, groupDesc);
    if(_groupModel.createGroup(newGroup)){
        _groupModel.addToGroup(userId, newGroup.getId(), "creator");
    }
}

void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){

    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();

    _groupModel.addToGroup(userId, groupId, "normal");
}

void ChatService::handleRedisSubscribeMessage(int userId, std::string msg){
    json js = json::parse(msg.c_str());
    muduo::net::TcpConnectionPtr targetConn;

    {
        std::lock_guard<std::mutex> lock(_connMutex);
        if(_userConnMap.count(userId)){
            targetConn = _userConnMap[userId];
            targetConn->send(js.dump());
            return;
        }  
    }

    _offlineMsgModel.insert(userId, msg);
    return;
}