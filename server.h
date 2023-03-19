#ifndef SERVER_H
#define SERVER_H


#include<string>
#include<memory>
#include<array>

#include<sys/epoll.h>

#include "./thread_pool/thread_pool.h"
#include "./http/http_connection.h"
#include "./mysql_connection_pool/mysql_connection_pool.h"
#include "./timer/timer.h"


const int kMaxFd = 65536;           //最大文件描述符
const int kMaxEventNumber=10000; //最大事件数
const int kTimeSlot = 5;             //最小超时单位


class Server
{
public:
    Server();
    ~Server();

    void Init(int port , std::string user, std::string password, std::string database_name,
              int log_write , int opt_linger, int trig_mode, int sql_num,
              int thread_num, int close_log, int actor_model);

    void ThreadPoolInit();
    void SqlPool();
    void LogWrite();
    void TrigMode();
    void EventListen();
    void EventLoop();
    void TimerInit(int connfd, struct sockaddr_in client_address);
    void AdjustTimer(std::shared_ptr<Timer> timer);
    void DealTimer(std::shared_ptr<Timer> timer, int sock_fd); //关闭客户的连接，移除相应计时器
    bool DealClientData();
    bool DealWithSignal(bool& timeout, bool& stop_server); //DealWithSignal将根据pip_fd_[0]中读取的信号决定，是否将time_out,stop_server置为1,从而控制服务器的行动。
    void DealWithRead(int sock_fd);
    void DealWithWrite(int sock_fd);

public:
    //基础
    int port_; //服务器监听的端口号
    std::string file_root_dir_; //服务器文件的根目录
    int log_write_;
    int close_log_; 
    int actor_model_; 

    std::shared_ptr<int [2]> pipe_fd_; //往pipe_fd_[1]中写入系统信号，从pipe_fd_[0]中读取系统信号
    int epoll_fd_;
    std::shared_ptr< std::shared_ptr<HttpConnection>[] > user_http_connections_; //HttpConnection类对象的数组

    //数据库相关
    std::shared_ptr<MysqlConnectionPool> sql_pool_; //mysql数据库连接池
    std::string database_user_;         //登陆数据库用户名
    std::string database_password_;     //登陆数据库密码
    std::string database_name_; //使用数据库名
    int sql_num_;

    //线程池相关
    std::shared_ptr< ThreadPool<HttpConnection> > thread_pool_;
    int thread_num_;

    //epoll_event相关
    epoll_event events_[kMaxEventNumber];

    int listen_fd_;
    int opt_linger_; //opt_linger=0为优雅关闭连接，opt_linger=1时会延时关闭
    int trig_mode_;  //用于设置listen_trig_mode_和connect_trig_mode_
    int listen_trig_mode_; //控制监听文件描述符的相关事件的触发模式
    int connect_trig_mode_;

    //定时器相关
    std::shared_ptr< std::shared_ptr<ClientData>[] > user_timers_; //user_timers_指向一个 保存有客户数据的共享指针 的数组
    Utils utils_;
};


#endif