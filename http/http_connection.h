#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include<memory>

#include"../mysql_connection_pool/mysql_connection_pool.h"

class HttpConnection
{
public:

    enum class Method
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };

    enum CheckState
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    enum HttpCode
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

public:
    void InitMysqlResult(std::shared_ptr<MysqlConnectionPool> connection_pool);

public:
    static int epoll_fd_;
    static int user_count_;
};



#endif