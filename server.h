#ifndef SERVER_H
#define SERVER_H


#include<string>

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
    Server();

    void Init(int port , std::string user, std::string passWord, std::string databaseName,
              int log_write , int opt_linger, int trig_mode, int sql_num,
              int thread_num, int close_log, int actor_model);

    void ThreadPoolInit();
    void SqlPool();
    void LogWrite();
    void TrigMode();
    void EventListen();
    void EventLoop();
    void TimerInit(int connfd, struct sockaddr_in client_address);
    void AdjustTimer(Timer *timer);
    void DealTimer(Timer *timer, int sockfd);
    bool DealClientData();
    bool DealWithSignal(bool& timeout, bool& stop_server);
    void DealWithRead(int sockfd);
    void DealWithWrite(int sockfd);

public:
    //基础
    int port_;
    std::string root_;
    int log_write_;
    int close_log_;
    int actor_model_;

    int pipe_fd_[2];
    int epoll_fd_;
    HttpConnection *users;

    //数据库相关
    std::shared_ptr<MysqlConnectionPool> sql_pool_;
    std::string database_user_;         //登陆数据库用户名
    std::string database_password_;     //登陆数据库密码
    std::string database_name_; //使用数据库名
    int sql_num_;

    //线程池相关
    std::shared_ptr< ThreadPool<HttpConnection> > *thread_pool_;
    int thread_num_;

    //epoll_event相关
    epoll_event events[kMaxEventNumber];

    int listen_fd_;
    int OPT_LINGER_;
    int TRIGMode_;
    int LISTENTrigmode_;
    int CONNTrigmode_;

    //定时器相关
    ClientData *users_timer_;
    Utils utils_;
};


#endif