#include "http_connection.h"


int HttpConnection::epoll_fd_{-1};
int HttpConnection::user_count_{0};