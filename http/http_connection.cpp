#include "http_connection.h"


int HttpConnection::epoll_fd_{-1};
int HttpConnection::user_count_{0};

void HttpConnection::InitMysqlResult(std::shared_ptr<MysqlConnectionPool> connection_pool)
{

}