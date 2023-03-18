#include "http_connection.h"


int HttpConnection::epoll_fd_{-1};
int HttpConnection::user_count_{0};

void HttpConnection::InitMysqlResult(std::shared_ptr<MysqlConnectionPool> connection_pool)
{

}

void HttpConnection::Init(int sock_fd, const sockaddr_in &addr, std::string_view root, int trig_mode,
            int close_log, std::string_view database_user, std::string_view database_password, std::string_view database_name)
{

}