#include "server.h"


#include<unistd.h>

Server::Server(): user_connections_{new HttpConnection[kMaxFd]},user_timers_{new ClientData[kMaxFd]}
{
    //设置root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);

    std::cout<<server_path<<std::endl;

    std::string server_path_str{server_path};

    file_root_dir_=server_path_str+"/root";

}

Server::~Server()
{
    close(epoll_fd_);
    close(listen_fd_);
    close(pipe_fd_[1]);
    close(pipe_fd_[0]);
}

void Server::Init(int port , std::string database_user, std::string database_password, std::string database_name,
        int log_write , int opt_linger, int trig_mode, int sql_num,
        int thread_num, int close_log, int actor_model)
{
    port_=port;

    database_user_=database_user;
    database_password_=database_password;
    database_name_=database_name;

    sql_num_=sql_num;

    thread_num_=thread_num;

    log_write_=log_write;

    opt_linger_=opt_linger;

    trig_mode_=trig_mode;

    close_log_=close_log;

    actor_model_=actor_model;
}

void Server::TrigMode()
{
    //LT + LT
    if (trig_mode_==0)
    {
        listen_trig_mode_ = 0;
        connect_trig_mode_ = 0;
    }
    //LT + ET
    else if (trig_mode_==1)
    {
        listen_trig_mode_ = 0;
        connect_trig_mode_ = 1;
    }
    //ET + LT
    else if (trig_mode_==2)
    {
        listen_trig_mode_ = 1;
        connect_trig_mode_ = 0;
    }
    //ET + ET
    else if (trig_mode_==3)
    {
        listen_trig_mode_ = 1;
        connect_trig_mode_ = 1;
    }
}

void Server::LogWrite()
{

}

void Server::SqlPool()
{

}

void Server::ThreadPoolInit()
{
    thread_pool_=std::make_shared< ThreadPool<HttpConnection> >();
}