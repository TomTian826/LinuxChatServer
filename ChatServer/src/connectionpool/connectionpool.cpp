#include "connectionpool.hpp"

const int MAX_LINE_SIZE = 1024;
const std::string CONFIG_FILE_PATH = "../mysql.ini";

ConnectionPool* ConnectionPool::instance(){
    static ConnectionPool pool;
    return &pool;
}

std::shared_ptr<MySQL> ConnectionPool::getConn(){       //感觉有问题，再思考思考
    std::unique_lock<std::mutex> lock(_queueMutex);
    if(_connQueue.empty()){
        if(std::cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(_connTimeout))){
            if(_connQueue.empty()){
                LOG_INFO << "get mysql connection failed, timeout";
                return nullptr;
            }
        }
    }

    std::shared_ptr<MySQL> sp(_connQueue.front(), 
        [&](MySQL* curConn) {
            std::unique_lock<std::mutex> lock(_queueMutex);
            curConn->updateAliveTime();
            _connQueue.push(curConn);
        }
    );

    _connQueue.pop();
    if(_connQueue.empty()){
        _cv.notify_all();
    }

    return sp;
}

ConnectionPool::ConnectionPool(){
    if(!loadConfigFile()){
        return;
    }

    _curConnCnt = 0;
    for(int i = 0; i < _initSize; ++i){
        MySQL* p = new MySQL;
        if(p->connect(_server, _port, _user, _password, _dbname)){
            p->updateAliveTime();
            _connQueue.push(p);
            _curConnCnt++;
        }
    }

    //启动一个新的线程，负责在需要的时候，创建新的可用连接
    std::thread produce(std::bind(&ConnectionPool::produceConn, this));
    produce.detach();

    //启动一个新的线程，负责清理长时间未使用的多余的空闲连接
    std::thread reduce(std::bind(&ConnectionPool::reduceConn, this));
    reduce.detach();
}

void ConnectionPool::produceConn(){
    for(;;){
        std::unique_lock<std::mutex> lock(_queueMutex); 
        while(!_connQueue.empty()){
            _cv.wait(lock);
        }

        if(_curConnCnt < _maxSize){
            MySQL* p = new MySQL;
            if(p->connect(_server, _port, _user, _password, _dbname)){
                p->updateAliveTime();
                _connQueue.push(p);
                _curConnCnt++;
            }
        }

        _cv.notify_all();
    }
}

void ConnectionPool::reduceConn(){
    for(;;){
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));

        std::unique_lock<std::mutex> lock(_queueMutex);
        while(_connQueue.size() > _initSize){
            MySQL* curConn = _connQueue.front();
            if(curConn->getAliveTime() >= (_maxIdleTime * 1000) ){
                _connQueue.pop();
                _curConnCnt--;
                delete curConn;
            }
            else{
                break;
            }
        }
    }
}

bool ConnectionPool::loadConfigFile(){
    FILE* pf = fopen(CONFIG_FILE_PATH.c_str(), "r");

    if(pf == nullptr){
        LOG_INFO << " mysql.cnf is not existed ";
        return false;
    }

    while(!feof(pf)){
        char line[MAX_LINE_SIZE] = {'\0'};
        fgets(line, 1024, pf);
        std::string lineStr = line;

        int separatorIndex = lineStr.find('=', 0);
        if(separatorIndex == -1){
            continue;
        }

        // int endIndex = lineStr.find('\n', separatorIndex);
        std::string key = lineStr.substr(0, separatorIndex);
        std::string val = lineStr.substr(separatorIndex + 1);
        val.pop_back();

        if(key == "server"){
            _server = val;
        }
        else if(key == "port"){
            _port = atoi(val.c_str());
        }
        else if(key == "user"){
            _user = val;
        }
        else if(key == "password"){
            _password = val;
        }
        else if(key == "dbname"){
            _dbname = val;
        }
//--------------------------------------------------------------------------------
        else if(key == "initSize"){
            _initSize = atoi(val.c_str());
        }
        else if(key == "maxSize"){
            _maxSize = atoi(val.c_str());
        }
        else if(key == "maxIdleTime"){
            _maxIdleTime = atoi(val.c_str());
        }
        else if(key == "connTimeout"){
            _connTimeout = atoi(val.c_str());
        }
    }

    return true;
}

