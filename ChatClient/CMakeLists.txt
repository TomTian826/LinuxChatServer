cmake_minimum_required(VERSION 3.0)
project(ChatClientCMake)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

include_directories(${PROJECT_SOURCE_DIR}/include)
# include_directories(${PROJECT_SOURCE_DIR}/include/model)
# include_directories(${PROJECT_SOURCE_DIR}/include/client)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_DIR)

add_executable(ChatClient ${SRC_DIR})

target_link_libraries(ChatClient pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)