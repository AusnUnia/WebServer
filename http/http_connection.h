#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <netinet/in.h>
#include <unistd.h>

#include<iostream>
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

    void Init(int sock_fd, const sockaddr_in &addr, std::string_view root, int trig_mode,
                     int close_log, std::string_view database_user, std::string_view database_password, std::string_view database_name);


    //////////////////////test/////////////////////////////////////
    HttpConnection(int sock):sock_(sock)
    {

    }
    std::string header_{"GET"};
    std::string req_{""};
    int sock_;
    void Process()
    {
        char buf[512];

        FILE* read_stream=fdopen(sock_,"r");
        FILE* write_stream=fdopen(dup(sock_),"w");

        fgets(buf,512,read_stream);
        req_=buf;
        std::cout<<req_<<std::endl;

        FILE* html_file=fopen("./html/test.html","r");
        if(html_file==nullptr)
        {
            std::cout<<"open error!"<<std::endl;
            return;
        }

        while(fgets(buf,512,html_file)!=nullptr)
        {
            fputs(buf,write_stream);
            fflush(write_stream);
        }
        fflush(write_stream);
        fclose(write_stream);
        fclose(read_stream);
    }
    ////////////////////////////test////////////////////////////////////

public:
    static int epoll_fd_;
    static int user_count_;
};



#endif