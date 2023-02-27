cmake_minimum_required(VERSION 3.10)

project(test)

add_executable(test ./thread_pool/thread_pool.cc ./test.cc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)