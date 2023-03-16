#include "server.h"


#include<unistd.h>
#include<assert.h>
#include<string.h>

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
    //初始化数据库连接池
    sql_pool_=std::move(MysqlConnectionPool::GetInstance());
    sql_pool_->Init("localhost",database_user_,database_password_,database_name_,3306,sql_num_,close_log_);

    user_connections_.get()->InitMysqlResult(sql_pool_);
}

void Server::ThreadPoolInit()
{
    thread_pool_=std::make_shared< ThreadPool<HttpConnection> >();
}

void Server::EventListen()
{
    listen_fd_=socket(PF_INET,SOCK_STREAM,0);
    assert(listen_fd_>0);

    //优雅关闭连接
    if (opt_linger_==0)
    {
        struct linger tmp = {0, 1};
        setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if (opt_linger_==1)
    {
        struct linger tmp = {1, 1};
        setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    //设置服务器监听的地址
    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(port_);

    //让端口释放后立即就可以被再次使用
    int flag=1;
    setsockopt(listen_fd_,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));

    //绑定监听文件描述符和监听地址
    if(bind(listen_fd_,(struct sockaddr*)&address,sizeof(address))!=0)
    {
        std::cerr<<"bind() error!"<<std::endl;
    }

    //开始监听
    if(listen(listen_fd_,8)!=0)
    {
        std::cerr<<"listen() error!"<<std::endl;
    }

    //初始化定时器
    utils_.Init(kTimeSlot);

    //epoll创建内核事件表
    epoll_event events[kMaxEventNumber];
    epoll_fd_=epoll_create(8);
    if(epoll_fd_==-1)
    {
        std::cerr<<"epoll_create() error!"<<std::endl;
    }

    //设置监听文件描述符的相关事件（读事件）的触发模式
    utils_.AddFd(epoll_fd_,listen_fd_,false,listen_trig_mode_);

    //将epoll_fd_和HttpConnection相关联
    HttpConnection::epoll_fd_=epoll_fd_;

    //
}