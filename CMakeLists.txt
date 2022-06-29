cmake_minimum_required(VERSION 3.0)
project(wholeProjectCMake)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 指定搜索的子目录
add_subdirectory(ChatServer)
add_subdirectory(ChatClient)