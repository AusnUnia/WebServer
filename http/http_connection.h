#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <netinet/in.h>
#include <unistd.h> 
#include <fcntl.h>

#include<iostream>
#include<memory>
#include<map>

#include"../mysql_connection_pool/mysql_connection_pool.h"
#include"../lock/my_semaphore.h"
#include"../timer/timer.h"

class HttpConnection
{
public:
    static const int kFileNameLen{200};
    static const int kReadBufferSize{2048};
    static const int kWriteBufferSize{1024};

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

    enum class CheckState
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    enum class HttpCode
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

    enum class LineStatus
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    HttpConnection(){}
    ~HttpConnection(){}

public:
    void Init(int sock_fd, const sockaddr_in &addr, std::string_view root, int trig_mode,
                     int close_log, std::string_view database_user, std::string_view database_password, std::string_view database_name);
    void CloseConnection(bool real_close=true);
    bool ReadOnce();
    bool Write();

    sockaddr_in* get_address()
    {
        return  &address_;
    }

    void InitMysqlResult(std::shared_ptr<MysqlConnectionPool> connection_pool);
    int timer_flag_;
    int improve_;


private:
    void Init();
    HttpCode ProcessRead();
    bool ProcessWrite(HttpCode ret);
    HttpCode parse_request_line(char *text);
    HttpCode parse_headers(char *text);
    HttpCode parse_content(char *text);
    HttpCode do_request();
    char *get_line() { return (read_buffer_+start_line_); };
    LineStatus parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();



    //////////////////////test/////////////////////////////////////
public:
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
    std::shared_ptr<MYSQL> mysql_;
    int state_; //读为0, 写为1

private:
    int sock_fd_;
    sockaddr_in address_;

    char read_buffer_[kReadBufferSize];
    int read_idx_;
    int checked_idx_;
    int start_line_;

    char write_buffer_[kWriteBufferSize];
    int write_idx_;

    CheckState check_state_;
    Method method_;

    char real_file_[kFileNameLen];
    std::string url_;
    std::string version_;
    std::string host_;
    int content_length_;
    bool linger_;
    std::string file_address_;
    struct stat file_stat_;
    struct iovec iovec_[2];
    int iovec_count_;
    int cgi_;        //是否启用的POST
    std::string request_header_; //存储请求头数据
    int bytes_to_send_;
    int bytes_have_send_;
    std::string doc_root_;

    std::map<std::string,std::string> users_;
    int trig_mode_;
    int close_log_;

    std::string database_user_;
    std::string database_password_;
    std::string database_name_;

};



#endif