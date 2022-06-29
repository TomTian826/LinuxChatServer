cmake_minimum_required(VERSION 3.0)
project(ChatServerCMake)

# 配置编译选项 -- 2
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
# set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
# set(BUILD_SHARED_LIBS)

# 配置头文件搜索路径 -- 4
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/model)
include_directories(${PROJECT_SOURCE_DIR}/include/database)
include_directories(${PROJECT_SOURCE_DIR}/include/redis)
include_directories(${PROJECT_SOURCE_DIR}/include/connectionpool)
# 配置库文件搜索路径 -- 5
link_directories(/usr/lib64/mysql)
link_directories(/usr/local/lib)


# 可以设置需要生成的源文件列表
# set(SRC_LIST ./formalTestModuo.cpp) 
# 把 （.指定路径）下的所有源文件名称，放入到变量名SRC_LIST中
aux_source_directory(${PROJECT_SOURCE_DIR}/src/model MODEL_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/server SERVER_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/db DB_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/redis REDIS_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/connectionpool CONNECTIONPOOL_LIST)


# 生成可执行文件，表示projectCMakeServer1这个程序，由SRC_LIST变量所定义的源文件编译而来
add_executable(ChatServer ${SERVER_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST} ${CONNECTIONPOOL_LIST}) 

# 添加链接库文件，表示projectCMakeServer1这个程序，需要链接这些库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread hiredis)


# 设置可执行文件最终存储到的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(EXECUTABLE_OUTPUT_PATH /home/admin/project/formalTestModuo/bin)