#include "redis.hpp"
#include <iostream>

Redis::Redis()
    : _publish_context(nullptr), _subscribe_context(nullptr)
{}

Redis::~Redis(){
    if(_publish_context != nullptr){
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr){
        redisFree(_subscribe_context);
    }
}

bool Redis::connect(){
    _publish_context = redisConnect("127.0.0.1", 6379);
    if(_publish_context == nullptr){
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }

    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if(_subscribe_context == nullptr){
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }

    std::thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    std::cout << "connect redis_server success" << std::endl;
    return true;
}
    
bool Redis::publish(int channel, std::string message){
    redisReply* reply = (redisReply*)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if(reply == nullptr){
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}


bool Redis::subscribe(int channel){

    //因为 SUBSCRIBE 是使线程阻塞地等待目标信道里出现消息，所以不使用 redisCommand + SUBSCRIBE
    //而是只负责进行信道的订阅，消息的处理则是由 observer_channel_message()负责
    //也就是另外创建一个线程来负责处理消息

    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel)){
        std::cerr << "subscribe command failed" << std::endl;
        return false;
    }

    int done = 0;

    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done)){
            std::cerr << "subscribe command failed" << std::endl;
            return false;
        }
    }

    return true;
}

bool Redis::unsubscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel)){
        std::cerr << "unsubscribe command failed" << std::endl;
        return false;
    }

    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done)){
            std::cerr << "unsubscribe command failed" << std::endl;
            return false;
        }
    }

    return true;
}

void Redis::observer_channel_message(){
    redisReply* reply = nullptr;
    while(REDIS_OK == redisGetReply(this->_subscribe_context, (void**)&reply)){
        if( reply != nullptr && 
            reply->element[2] != nullptr && 
            reply->element[2]->str != nullptr){
            
            _notify_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
    }
}

void Redis::init_notify_handler(std::function<void(int, std::string)> fn){
    this->_notify_handler = fn;
}