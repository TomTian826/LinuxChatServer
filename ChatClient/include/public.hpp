#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType{         //枚举常量只能以标识符形式表示，不能是整型、字符型等文字常量
    LOGIN_MSG = 1,      //1     登录操作    相当于从 1 开始枚举
    LOGIN_MSG_ACK,
    LOGOUT_MSG,         //3     用户退出

    REGIST_MSG,         //4     注册账号
    REGIST_MSG_ACK,

    ONE_CHAT_MSG,       //6     发起聊天
    //UNUSED_MSG,         //保留
    
    ADD_FRIEND_MSG,     //7     好友申请
    ADD_FRIEND_MSG_ACK, //8     好友申请答复

    CREATE_GROUP_MSG,   //9
    ADD_GROUP_MSG,      //10
    GROUP_CHAT_MSG,     //11

    
};

#endif

//{"msgid":3,"name":"RushBcpp","password":"projectTest1"}

//{"msgid":1,"name":"RushBcpp","password":"projectTest1"}

//{"msgid":1,"id":22,"password":"projectTest1"}
//{"msgid":1,"id":19,"password":"123456"}
//{"msgid":5,"from":22,"to":19,"msg":"TestForOneChat"}

//{"msgid":1,"id":22,"password":"projectTest1"}
//{"msgid":5,"from":22,"to":15,"msg":"TestForOffline"}

//{"msgid":1,"id":19,"password":"123456"}