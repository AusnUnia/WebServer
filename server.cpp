#include "server.h"


#include<unistd.h>
#include<sys/signal.h>
#include<assert.h>
#include<string.h>


Server::Server(): user_http_connections_{new std::shared_ptr<HttpConnection>[kMaxFd]()},user_timers_{new std::shared_ptr<ClientData>[kMaxFd]()}
{
    for(int i=0;i<kMaxFd;i++)
    {
        user_http_connections_[i]=std::make_shared<HttpConnection>();
        user_timers_[i]=std::make_shared<ClientData>();
    }
    
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
    sql_pool_=MysqlConnectionPool::GetInstance();
    sql_pool_->Init("localhost",database_user_,database_password_,database_name_,3306,sql_num_,close_log_);
    std::cout<<"sql_pool Init success!"<<std::endl;
    user_http_connections_[0]->InitMysqlResult(sql_pool_);
    std::cout<<"InitMysqlResult!"<<std::endl;
}

void Server::ThreadPoolInit()
{
    thread_pool_=std::make_shared< ThreadPool<HttpConnection> >(0,sql_pool_,20,4);
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
    std::cout<<"listenning..."<<std::endl;

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
    if(socketpair(PF_UNIX,SOCK_STREAM,0,pipe_fd_)!=0)
    {
        std::cerr<<"socketpair() error!"<<std::endl;
        std::cerr<<errno<<std::endl;
    }
    utils_.SetNonblocking(pipe_fd_[1]);
    utils_.AddFd(epoll_fd_,pipe_fd_[0],false,0);

    //注册信号应对函数
    utils_.AddSignal(SIGPIPE,SIG_IGN);
    utils_.AddSignal(SIGALRM,utils_.SignalHandler,false); //注册信号应对函数，这里的SignalHandler会把信号发给pip_fd_[1]，这样就可以从pipe_fd_[0]中读取到该信号了。
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
                std::cout<<"new conn1"<<std::endl;
                bool flag=DealClientData();
                std::cout<<"new conn2"<<std::endl;
                if(flag==false)
                    continue;
            }
            else if(events_[i].events&(EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                std::cout<<"close conn1"<<std::endl;
                std::shared_ptr<Timer> sh_timer=user_timers_[sock_fd]->weak_ptr_timer_.lock();
                DealTimer(sh_timer,sock_fd); //关闭客户的连接，移除相应计时器
                std::cout<<"close conn2"<<std::endl;
            }
            //pipe_fd_[0]有读事件表明有系统信号来了,下面处理信号
            else if( (sock_fd==pipe_fd_[0])&&(events_[i].events&EPOLLIN) )
            {
                std::cout<<"system sig"<<std::endl;
                bool flag=DealWithSignal(time_out,stop_server); //DealWithSignal将根据pip_fd_[0]中读取的信号决定，是否将time_out,stop_server置为1,从而控制服务器的行动。
                if (false == flag)
                    std::cout<<"dealclientdata failure"<<std::endl;
                std::cout<<"stop_server="<<stop_server<<std::endl;
                std::cout<<"time_out="<<time_out<<std::endl;
            }
            //客户来数据了,处理客户连接收到的数据
            else if(events_[i].events&EPOLLIN)
            {
                std::cout<<"read..."<<std::endl;
                DealWithRead(sock_fd); //处理读数据（接收客户发的数据）
            }
            else if(events_[i].events&EPOLLOUT)
            {
                std::cout<<"write..."<<std::endl;
                DealWithWrite(sock_fd); //处理写数据（发送给客户数据）
            }
        }

        if(time_out)
        {
            utils_.TimerHandler();
            std::clog<<"timer tick"<<std::endl;
            time_out=false;
        }
    }
}

bool Server::DealClientData()
{
    std::cout<<"Server::DealClientData()"<<std::endl;
    
    struct sockaddr_in client_address;
    socklen_t client_address_len=sizeof(client_address);

    if(listen_trig_mode_==0) //边缘触发
    {
        int connection_fd=accept(listen_fd_,(struct sockaddr*)&client_address,&client_address_len);
        std::cout<<"connection_fd="<<connection_fd<<std::endl;
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
        std::cout<<"HttpConnection::user_count_="<<HttpConnection::user_count_<<std::endl;
        TimerInit(connection_fd,client_address); //每个客户的连接都上个计时器，防止占用资源不用
    }
    else //水平触发
    {
        while(true)
        {
            int connection_fd=accept(listen_fd_,(struct sockaddr*)&client_address,&client_address_len);
            std::cout<<"connection_fd="<<connection_fd<<std::endl;
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
    std::cout<<"Server::TimerInit()"<<std::endl;
    //初始化HttpConnection
    user_http_connections_[connection_fd]->Init(connection_fd,client_address,file_root_dir_,connect_trig_mode_,close_log_,database_user_,database_password_,database_name_);

    //初始化ClientData
    user_timers_[connection_fd]->address_=client_address;
    user_timers_[connection_fd]->sock_fd_=connection_fd;

    //设置计时器
    std::shared_ptr<Timer> sh_timer=std::make_shared<Timer>();
    sh_timer->shared_ptr_clientdata_=std::shared_ptr<ClientData>(user_timers_[connection_fd]);//绑定用户数据
    sh_timer->CallBackFunc=CallBackFunc;//设置回调函数
    time_t now=time(nullptr);
    sh_timer->expire_=now+3*kTimeSlot;//设置超时时间

    user_timers_[connection_fd]->weak_ptr_timer_=sh_timer; //关联用户数据和计时器

    utils_.sorted_timer_list_.AddTimer(sh_timer); //把新的计时器加入链表
}

void Server::DealTimer(std::shared_ptr<Timer> timer, int sock_fd)
{
    timer->CallBackFunc(user_timers_[sock_fd]); //从epoll事件表删除sock_fd对应客户的事件
    if(timer)
    {
        bool delete_result=utils_.sorted_timer_list_.DeleteTimer(timer);
        if(!delete_result)
        {
            std::cerr<<"DeleteTimer() failed. Timer of "<<sock_fd<<std::endl;
        }
    }
    std::cout<<"close fd: "<<user_timers_[sock_fd]->sock_fd_<<std::endl;
}

bool Server::DealWithSignal(bool& time_out, bool& stop_server)
{
    std::cout<<"Server::DealWithSignal()"<<std::endl;
    char signal_buf[1024];

    int ret=recv(pipe_fd_[0],signal_buf,sizeof(signal_buf),0);
    if(ret==-1)
    {
        return false;
    }
    else if(ret==0)
    {
        return false;
    }
    else
    {
        for(int i=0;i<ret;i++)
        {
            if(signal_buf[i]==SIGALRM) //超时了
            {
                std::cout<<"SIGALRM time_out"<<std::endl;
                time_out=true;
                break;
            }
            if(signal_buf[i]==SIGTERM) //中断信来了
            {
                std::cout<<"SIGTERM stop_server"<<std::endl;
                stop_server=true;
                break;
            }
        }
    }
    return true;
}

void Server::DealWithRead(int sock_fd)
{
    std::cout<<"DealWithRead()"<<std::endl;
    std::shared_ptr<Timer> timer=user_timers_[sock_fd]->weak_ptr_timer_.lock();

    //reactor
    if(actor_model_==1)
    {
        if(timer)
        {
            AdjustTimer(timer);
        }

        thread_pool_->AddTask(user_http_connections_[sock_fd]);

        while (true)
        {
            if (1 == user_http_connections_[sock_fd]->improve_)
            {
                if (1 == user_http_connections_[sock_fd]->timer_flag_)
                {
                    DealTimer(timer, sock_fd);
                    user_http_connections_[sock_fd]->timer_flag_ = 0;
                }
                user_http_connections_[sock_fd]->improve_ = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (user_http_connections_[sock_fd]->ReadOnce())
        {
            std::cout<<"deal with the client : "<<inet_ntoa(user_http_connections_[sock_fd]->get_address()->sin_addr)<<std::endl;;

            //若监测到读事件，将该事件放入请求队列
            thread_pool_->AddTask(user_http_connections_[sock_fd]);

            if (timer)
            {
                AdjustTimer(timer);
            }
        }
        else
        {
            DealTimer(timer, sock_fd);
        }
    }
}

void Server::AdjustTimer(std::shared_ptr<Timer> timer)
{
    std::cout<<"Server::AdjustTimer()"<<std::endl;
    time_t now=time(nullptr);
    timer->expire_=now+3*kTimeSlot;
    utils_.sorted_timer_list_.AdjustTimer(timer);
}

void Server::DealWithWrite(int sock_fd)
{
    std::cout<<"Server::DealWithWrite()"<<std::endl;
    std::shared_ptr<Timer> timer = user_timers_[sock_fd]->weak_ptr_timer_.lock();
    //reactor
    if (1 == actor_model_)
    {
        if (timer)
        {
            AdjustTimer(timer);
        }

        thread_pool_->AddTask(user_http_connections_[sock_fd]);

        while (true)
        {
            if (1 == user_http_connections_[sock_fd]->improve_)
            {
                if (1 == user_http_connections_[sock_fd]->timer_flag_)
                {
                    DealTimer(timer, sock_fd);
                    user_http_connections_[sock_fd]->timer_flag_ = 0;
                }
                user_http_connections_[sock_fd]->improve_ = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (user_http_connections_[sock_fd]->Write())
        {
            std::cout<<"send data to the client : "<<inet_ntoa(user_http_connections_[sock_fd]->get_address()->sin_addr)<<std::endl;

            if (timer)
            {
                AdjustTimer(timer);
            }
        }
        else
        {
            DealTimer(timer, sock_fd);
        }
    }
}