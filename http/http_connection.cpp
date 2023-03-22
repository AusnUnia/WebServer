#include "http_connection.h"


const std::string ok_200_title = "OK";
const std::string error_400_title = "Bad Request";
const std::string error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const std::string error_403_title = "Forbidden";
const std::string error_403_form = "You do not have permission to get file form this server.\n";
const std::string error_404_title = "Not Found";
const std::string error_404_form = "The requested file was not found on this server.\n";
const std::string error_500_title = "Internal Error";
const std::string error_500_form = "There was an unusual problem serving the request file.\n";


int HttpConnection::epoll_fd_{-1};
int HttpConnection::user_count_{0};

void HttpConnection::InitMysqlResult(std::shared_ptr<MysqlConnectionPool> connection_pool)
{

}

void HttpConnection::Init(int sock_fd, const sockaddr_in &addr, std::string_view root, int trig_mode,
            int close_log, std::string_view database_user, std::string_view database_password, std::string_view database_name)
{

}