cmake_minimum_required(VERSION 3.10)

project(test)

add_executable(test ./test.cpp ./mysql_connection_pool/mysql_connection_pool.cpp
./timer/timer.cpp ./http/http_connection.cpp ./server.cpp ./config.cpp) 

target_link_libraries(${PROJECT_NAME} mysqlclient)
