cmake_minimum_required(VERSION 3.10)

project(test)
find_path(MYSQL_ROOT_DIR mysql)

add_executable(test ./test.cpp ./mysql_connection_pool/mysql_connection_pool.cpp) 
include_directories(/usr/include/mysql)
target_include_directories(${PROJECT_NAME} PRIVATE ${MYSQL_ROOT_DIR}/include)
target_link_directories(${PROJECT_NAME} PRIVATE ${MYSQL_ROOT_DIR}/lib)
target_link_libraries(${PROJECT_NAME} mysqlclient)
