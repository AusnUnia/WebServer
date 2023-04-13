cmake_minimum_required(VERSION 3.10)

project(test)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

add_executable(test ./test.cpp ./mysql_connection_pool/mysql_connection_pool.cpp
./timer/timer.cpp ./http/http_connection.cpp ./server.cpp ./config.cpp
 ./server_log/server_log.cpp) 

target_link_libraries(${PROJECT_NAME} mysqlclient fmt)
