#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include <thread>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <semaphore.h>
#include <atomic>

#include "user.hpp"
#include "group.hpp"
#include "public.hpp"

#include "json.hpp"
using json = nlohmann::json;

using CmdHandler = std::function<void(int, std::string)>;
//====================================常量定义=============================================
const int MAX_USER_SIZE = 50;
const int MAX_BUFFER_SIZE = 1024;
//====================================函数声明=============================================
std::string getCurTime();

void doLoginResponse(json& loginMsgAck);
void doRegistResponse(json& registMsgAck);
void readTaskHandler(int clientfd);
void mainUserMenu(int clientfd);

void showCurUserData();

void help(int clientfd = 0, std::string input = "");
void logout(int, std::string input = "");

void addFriend(int, std::string);
void addGroup(int, std::string);

void createGroup(int, std::string);

void oneChat(int, std::string);
void groupChat(int, std::string);
//====================================全局变量=============================================
User g_curUser;
std::vector<User> g_curUserFriends;
std::vector<Group> g_curUserGroups;
//记录是否成功登录
std::atomic_bool g_isLoginSuccess{false};

//读写线程之间的通信
sem_t rwsem;  

//记录是否已经有用户登录的状态，用于显示不同层级的目录
//bool curUserUsing;

//把函数名称与 command 指令字符串绑定
std::unordered_map<std::string, CmdHandler> _cmdHandlerMap = {
    {"help", help},
    {"logout", logout},

    {"addFriend", addFriend},
    {"addGroup", addGroup},

    {"createGroup", createGroup},

    {"oneChat", oneChat},
    {"groupChat", groupChat}
};

//指令信息表，感觉是辅助信息，记录了command 如何使用，同时也是记录了对应的 func 名称
std::unordered_map<std::string, std::string> _cmdInfoMap = {
    {"help",         "显示所有支持的命令    格式 help"},
    {"logout",       "注销                 格式 logout"},

    {"addFriend",    "添加好友             格式 addFriend:friendid"},
    {"addGroup",     "加入群组             格式 addGroup:groupid"},

    {"createGroup",  "创建群组             格式 createGroup:\"groupname\":\"groupdesc\""},

    {"oneChat",      "一对一聊天           格式 oneChat:friendid:\"message\""},
    {"groupChat",    "群组聊天             格式 groupChat:groupid:\"message\""}
};
//========================================================================================

//主线程作为发送线程，子线程作为接收线程
int main(int argc, char** argv){

    if(argc < 3){
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std:: endl;
        exit(-1);
    }

    char* serverIp = argv[1];
    uint16_t serverPort = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1){
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);
    server.sin_addr.s_addr = inet_addr(serverIp);
    if(connect(clientfd, (sockaddr*)&server, sizeof(sockaddr_in)) == -1){
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    //menu 循环跳出条件
    //curUserUsing = false;

    //信号量初始化
    sem_init(&rwsem, 0, 0);
    
    //创建监听子线程，负责处理所有来自服务器的信息
    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    for(;;){    //while(true){
        // 显示首页面菜单 登录、注册、退出
        std::cout << "========================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "choice : ";
        int choice = 0;
        std::cin >> choice;
        std::cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
            case 1:{
                int userId = 0;
                char userPassword[MAX_USER_SIZE] = {'\0'};

                std::cout << "userId : ";
                std::cin >> userId;
                std::cin.get();

                //getline(std::cin, userPassword);      //要求类型是 string
                std::cout << "userPassword : ";
                std::cin.getline(userPassword, MAX_USER_SIZE);     //要求类型是 char*

                json loginMsg;
                loginMsg["msgid"] = LOGIN_MSG;
                loginMsg["id"] = userId;
                loginMsg["password"] = userPassword;

                //登录状态初始化
                g_isLoginSuccess = false;

                std::string request = loginMsg.dump();
                int len = send(clientfd, request.c_str(), request.size() + 1, 0);
                if(len == -1){
                    std::cerr << "send login msg error " << request << std::endl;
                }

                sem_wait(&rwsem);

                if(g_isLoginSuccess){
                    //curUserUsing = true;
                    mainUserMenu(clientfd);
                }
            }
            break;

            case 2:{
                char userName[MAX_USER_SIZE] = {'\0'};
                char userPassword[MAX_USER_SIZE] = {'\0'};

                std::cout << "userName : ";
                std::cin.getline(userName, MAX_USER_SIZE);

                std::cout << "userPassword : ";
                std::cin.getline(userPassword, MAX_USER_SIZE);

                json registMsg;
                registMsg["msgid"] = REGIST_MSG;
                registMsg["name"] = userName;
                registMsg["password"] = userPassword;
                std::string request = registMsg.dump();

                int len = send(clientfd, request.c_str(), request.size() + 1, 0);
                if(len == -1){
                    std::cerr << "send regist msg error : " << request << std::endl;
                }

                sem_wait(&rwsem);
            }
            break;

            case 3:{
                close(clientfd);
                sem_destroy(&rwsem); 
                exit(0);
                //就直接结束程序了？服务器那边会根据 conn 中断进行处理是吧
            }

            default:{
                std::cerr << "invalid input! " << std::endl;
            }
        }
    }



}

//====================================函数定义=============================================
std::string getCurTime(){
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    struct tm *ptm = localtime(&tt);

    char date[60] = {'\0'};

    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int) ptm->tm_year + 1900, (int) ptm->tm_mon + 1, (int) ptm->tm_mday,
            (int) ptm->tm_hour, (int) ptm->tm_min, (int) ptm->tm_sec);

    return std::string(date);
}

void showCurUserData(){
    std::cout << "======================login user======================" << std::endl;
    std::cout << "current login user => id:" << g_curUser.getId() << " name:" << g_curUser.getName() << std::endl;


    std::cout << "----------------------friend list---------------------" << std::endl;
    if (!g_curUserFriends.empty())
    {
        for (auto& userFriend : g_curUserFriends)
        {
            std::cout << userFriend.getId() << " " << userFriend.getName() << " " << userFriend.getState() << std::endl;
        }
    }


    std::cout << "----------------------group list----------------------" << std::endl;
    if (!g_curUserGroups.empty())
    {
        for (Group &userGroup : g_curUserGroups)
        {
            std::cout << userGroup.getId() << " " << userGroup.getGroupName() << " " << userGroup.getGroupDesc() << std::endl;

            for (GroupUser &groupUser : userGroup.getGroupUsers())
            {
                std::cout << groupUser.getId() << " " << groupUser.getName() << " " << groupUser.getState()
                     << " " << groupUser.getRole() << std::endl;
            }
        }
    }
    std::cout << "======================================================" << std::endl;
}

void doLoginResponse(json& loginMsgAck){
    if (loginMsgAck["errno"].get<int>() != 0){
        std::cerr << loginMsgAck["errno"].get<int>() << std::endl;
        std::cerr << loginMsgAck["errmsg"] << std::endl;
        g_isLoginSuccess = false;
    }
    else{
        g_curUser.setId(loginMsgAck["id"].get<int>());
        g_curUser.setName(loginMsgAck["name"]);

        if (loginMsgAck.contains("friends")){
            g_curUserFriends.clear();

            User tempUser;
            json tempUserJson;
            std::vector<std::string> curUserFriends = loginMsgAck["friends"];

            for (const auto &iter : curUserFriends){
                tempUserJson = json::parse(iter);

                tempUser.setId(tempUserJson["id"].get<int>());
                tempUser.setName(tempUserJson["name"]);
                tempUser.setState(tempUserJson["state"]);

                g_curUserFriends.push_back(tempUser);
            }
        }

        if (loginMsgAck.contains("groups")){
            g_curUserGroups.clear();

            std::vector<std::string> curUserGroups = loginMsgAck["groups"];
            Group tempGroup;
            GroupUser tempGroupUser;
            json tempGroupJson;
            json tempGroupMemberJson;

            for (const auto &iter : curUserGroups){
                tempGroupJson = json::parse(iter);

                tempGroup.setId(tempGroupJson["groupid"].get<int>());
                tempGroup.setGroupName(tempGroupJson["groupname"]);
                tempGroup.setGroupDesc(tempGroupJson["groupdesc"]);
                tempGroup.getGroupUsers().clear();

                std::vector<std::string> curGroupMembersJson = tempGroupJson["groupusers"];
                for (const auto &groupMemberJson : curGroupMembersJson){
                    tempGroupMemberJson = json::parse(groupMemberJson);

                    tempGroupUser.setId(tempGroupMemberJson["id"].get<int>());
                    tempGroupUser.setName(tempGroupMemberJson["name"]);
                    tempGroupUser.setState(tempGroupMemberJson["state"]);
                    tempGroupUser.setRole(tempGroupMemberJson["role"]);

                    tempGroup.getGroupUsers().push_back(tempGroupUser);
                }
                g_curUserGroups.push_back(tempGroup);
            }
        }

        showCurUserData();

        if (loginMsgAck.contains("offlinemsgs")){
            std::vector<std::string> curUserOfflineMsgs = loginMsgAck["offlinemsgs"];
            json curOfflineMsg;

            std::cout << std::endl;
            for (const auto &iter : curUserOfflineMsgs){
                curOfflineMsg = json::parse(iter);
                if (curOfflineMsg["msgid"].get<int>() == ONE_CHAT_MSG){
                    std::cout << curOfflineMsg["time"] << " [ " << curOfflineMsg["from"].get<int>() << " ] "
                              << curOfflineMsg["name"] << " said : " << curOfflineMsg["msg"] << std::endl;
                }
                else{
                    std::cout << curOfflineMsg["time"] << " [ " << curOfflineMsg["from"].get<int>() << " ] "
                              << curOfflineMsg["name"] << " in group "
                              << "[" << curOfflineMsg["groupid"].get<int>() << "]"
                              << " said : " << curOfflineMsg["msg"] << std::endl;
                }
            }
            std::cout << std::endl;
        }
    
        g_isLoginSuccess = true;
    }
}

void doRegistResponse(json& registMsgAck){
    if (registMsgAck["errno"].get<int>() != 0){
        std::cerr << "regist error " << std::endl;
    }
    else{
        std::cout << "regist success " << std::endl;

        std::cout << registMsgAck["userName"] << ", here is your userId : " << registMsgAck["id"].get<int>()
                  << " , do not forget" << std::endl;
    }
}

void readTaskHandler(int clientfd){
    for(;;){
        char buffer[MAX_BUFFER_SIZE] = {'\0'};
        int len = recv(clientfd, buffer, MAX_BUFFER_SIZE, 0);
        if(len == -1 || len == 0){
            std::cerr << "recv msg error " << std::endl;
            close(clientfd);
            exit(-1);
        }
        else{
            json recvMsg = json::parse(buffer);
            int msgType = recvMsg["msgid"].get<int>();

            switch (msgType)
            {
                case LOGIN_MSG_ACK:{
                    //提交登录请求后，用来处理登录的反馈信息
                    doLoginResponse(recvMsg);
                    //使用信号量来通知主线程
                    sem_post(&rwsem);
                }
                break;

                case REGIST_MSG_ACK:{
                    //提交登录请求后，用来处理登录的反馈信息
                    doRegistResponse(recvMsg);
                    //使用信号量来通知主线程
                    sem_post(&rwsem);
                }
                break;


                case ONE_CHAT_MSG:{
                    std::cout << "received one on one chat message " << std::endl;
                    std::cout << recvMsg["time"] << " [ " << recvMsg["from"].get<int>() << " ] "
                              << recvMsg["name"] << " said : " << recvMsg["msg"] << std::endl;
                    continue;
                }
                break;

                case GROUP_CHAT_MSG:{
                    std::cout << "received group chat message " << std::endl;
                    std::cout << recvMsg["time"] << " [ " << recvMsg["from"].get<int>() << " ] "
                              << recvMsg["name"] << " in group " << "[" << recvMsg["groupid"].get<int>() <<  "]" 
                              << " said : " << recvMsg["msg"] << std::endl;
                    continue;
                }
                break;
            }
        }
    }
}

void mainUserMenu(int clientfd){
    char buffer[MAX_BUFFER_SIZE] = {'\0'};
    help();

    while(g_isLoginSuccess){
        std::cin.getline(buffer, MAX_BUFFER_SIZE);
        std::string commandBuffer(buffer);      //这里存储了全部的用户输入信息
        std::string command;                    //这里单独是把 command 部分单独截取出来

        int index = commandBuffer.find(':');
        if(index == std::string::npos){
            command = commandBuffer;
        }
        else{
            command = commandBuffer.substr(0, index);
        }

        if(_cmdHandlerMap.count(command)){
            CmdHandler cmdHandler = _cmdHandlerMap[command];
            cmdHandler(clientfd, commandBuffer.substr(index + 1));
        }
        else{
            std::cerr << " invalid input" << std::endl;
            std::cerr << command << std::endl;
            continue;
        }
    }
}

void help(int clientfd, std::string input){
    std::cout << " ---------------------------------------------------------------- " << std::endl;
    for(const auto& iter : _cmdInfoMap){
        std::cout << iter.first << " " << iter.second << std::endl;
    }
    std::cout << std::endl;
}

void logout(int clientfd, std::string){
    std::string buffer;

    json request;
    request["msgid"] = LOGOUT_MSG;
    request["id"] = g_curUser.getId();
    buffer = request.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size() + 1, 0);
    if(len == -1){
        std::cerr << " send logout msg error " << std::endl;
    }
    else{
        //User g_curUser;
        //g_curUserFriends.clear();
        //g_curUserGroups.clear();
        g_isLoginSuccess = false;
        //curUserUsing = false;
    }
}

void addFriend(int clientfd, std::string input){
    int friendId = atoi(input.c_str());
    std::string buffer;

    json request;
    request["msgid"] = ADD_FRIEND_MSG;
    request["id"] = g_curUser.getId();
    request["friendid"] = friendId;
    buffer = request.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size() + 1, 0);
    if(len == -1){
        std::cerr << " send addFriend msg error " << std::endl;
    }
}

void addGroup(int clientfd, std::string input){
    int groupId = atoi(input.c_str());
    std::string buffer;

    json request;
    request["msgid"] = ADD_GROUP_MSG;
    request["id"] = g_curUser.getId();
    request["groupid"] = groupId;
    buffer = request.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size() + 1, 0);
    if(len == -1){
        std::cerr << " send addGroup msg error " << std::endl;
    }
}

void createGroup(int clientfd, std::string input){
    int index = input.find(':');
    if(index == std::string::npos){
        std::cerr << " invalid input for createGroup service " << std::endl;
    }
    else{
        std::string groupName = input.substr(0, index);
        std::string groupDesc = input.substr(index + 1);
        std::string buffer;

        json request;
        request["msgid"] = CREATE_GROUP_MSG;
        request["id"] = g_curUser.getId();
        request["groupname"] = groupName;
        request["groupdesc"] = groupDesc;
        buffer = request.dump();

        int len = send(clientfd, buffer.c_str(), buffer.size() + 1, 0);
        if(len == -1){
            std::cerr << " send createGroup msg error " << std::endl;
        }
    }
}

void oneChat(int clientfd, std::string input){
    
    int index = input.find(':');
    if(index == std::string::npos){
        std::cerr << " invalid input for oneChat service " << std::endl;
    }
    else{
        int friendId = atoi(input.substr(0, index).c_str());
        std::string message = input.substr(index + 1);
        std::string buffer;

        json request;
        request["msgid"] = ONE_CHAT_MSG;
        request["from"] = g_curUser.getId();
        request["name"] = g_curUser.getName();
        request["to"] = friendId;
        request["msg"] = message;
        request["time"] = getCurTime();
        buffer = request.dump();
        
        int len = send(clientfd, buffer.c_str(), buffer.size() + 1, 0);
        if(len == -1){
            std::cerr << " send oneChat msg error " << std::endl;
        }
    }
}

void groupChat(int clientfd, std::string input){
    int index = input.find(':');
    if(index == std::string::npos){
        std::cerr << " invalid input for groupChat service " << std::endl;
    }
    else{
        int groupId = atoi(input.substr(0, index).c_str());
        std::string message = input.substr(index + 1);
        std::string buffer;

        json request;
        request["msgid"] = GROUP_CHAT_MSG;
        request["from"] = g_curUser.getId();
        request["name"] = g_curUser.getName();
        request["groupid"] = groupId;
        request["time"] = getCurTime();
        request["msg"] = message;
        buffer = request.dump();

        int len = send(clientfd, buffer.c_str(), buffer.size() + 1, 0);
        if(len == -1){
            std::cerr << " send groupChat msg error " << std::endl;
        }
    }
}
