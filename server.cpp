#include "server.h"


#include<unistd.h>
#include<sys/signal.h>
#include<assert.h>
#include<string.h>

#include<memory>

Server::Server(): user_http_connections_{new HttpConnection[kMaxFd]},user_timers_{new ClientData[kMaxFd]}
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

    user_http_connections_.get()->InitMysqlResult(sql_pool_);
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
    if(socketpair(PF_UNIX,SOCK_STREAM,0,pipe_fd_.get())!=0)
    {
        std::cerr<<"socketpair() error!"<<std::endl;
    }
    utils_.SetNonblocking(pipe_fd_[1]);
    utils_.AddFd(epoll_fd_,pipe_fd_[0],false,0);

    //注册信号应对函数
    utils_.AddSignal(SIGPIPE,SIG_IGN);
    utils_.AddSignal(SIGALRM,utils_.SignalHandler,false);
    utils_.AddSignal(SIGTERM,utils_.SignalHandler,false);

    alarm(kTimeSlot);

    //将epoll_fd_,pip_fd_和Utils相关联
    Utils::pipe_fd_=pipe_fd_;
    Utils::epoll_fd_=epoll_fd_;
}

void Server::EventLoop()
{
    bool time_out=false;
    bool stop_server=false;

    while(!stop_server)
    {
        int trigered_events_number=epoll_wait(epoll_fd_,events_,kMaxEventNumber,-1); //一直等，直到有事件触发
        if(trigered_events_number<0&&errno!=EINTR)
        {
            std::cerr<<"epoll failure"<<std::endl;
            break;
        }

        for(int i=0;i<trigered_events_number;i++)
        {
            int sock_fd=events_[i].data.fd;

            //有新客户连接
            if(sock_fd==listen_fd_)
            {
                bool flag=DealClientData();
                if(flag==false)
                    continue;
            }
            else if(events_[i].events&(EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {

            }
        }
    }
}

bool Server::DealClientData()
{
    struct sockaddr_in client_address;
    socklen_t client_address_len=sizeof(client_address);

    if(listen_trig_mode_==0) //边缘触发
    {
        int connection_fd=accept(listen_fd_,(struct sockaddr*)&client_address,&client_address_len);
        if(connection_fd<0)
        {
            std::cerr<<"accept() errer! errno = "+errno<<std::endl;
            return false;
        }

        //客户太多
        if(HttpConnection::user_count_>=kMaxFd)
        {
            utils_.ShowError(connection_fd,"Internal server busy!");
            std::cerr<<"Internal server busy!"<<std::endl;
            return false;
        }

        TimerInit(connection_fd,client_address); //每个客户的连接都上个计时器，防止占用资源不用
    }
    else //水平触发
    {
        while(true)
        {
            int connection_fd=accept(listen_fd_,(struct sockaddr*)&client_address,&client_address_len);
            int connection_fd=accept(listen_fd_,(struct sockaddr*)&client_address,&client_address_len);
            if(connection_fd<0)
            {
                std::cerr<<"accept() errer! errno = "+errno<<std::endl;
                break;
            }

            //客户太多
            if(HttpConnection::user_count_>=kMaxFd)
            {
                utils_.ShowError(connection_fd,"Internal server busy!");
                std::cerr<<"Internal server busy!"<<std::endl;
                break;
            }

            TimerInit(connection_fd,client_address); //每个客户的连接都上个计时器，防止占用资源不用
        }
    }
    return true;
}

void Server::TimerInit(int connection_fd, struct sockaddr_in client_address)
{
    //初始化HttpConnection
    user_http_connections_[connection_fd].Init(connection_fd,client_address,file_root_dir_,connect_trig_mode_,close_log_,database_user_,database_password_,database_name_);

    //初始化ClientData
    user_timers_[connection_fd].address_=client_address;
    user_timers_[connection_fd].sock_fd_=connection_fd;

    //设置计时器
    std::shared_ptr<Timer> sh_timer=std::make_shared<Timer>();
    sh_timer->shared_ptr_clientdata_=std::shared_ptr<ClientData>(&user_timers_[connection_fd]);//绑定用户数据
    sh_timer->CallBackFunc=CallBackFunc;//设置回调函数
    time_t now=time(nullptr);
    sh_timer->expire_=now+3*kTimeSlot;//设置超时时间

    user_timers_[connection_fd].weak_ptr_timer_=sh_timer; //关联用户数据和计时器

    utils_.sorted_timer_list_.AddTimer(sh_timer); //把新的计时器加入链表
}