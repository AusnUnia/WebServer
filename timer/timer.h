#ifndef TIMER_H
#define TIMER_H

#include<list>
#include<memory>
#include<string>

#include<arpa/inet.h>
#include<netinet/in.h>
#include<time.h>


class Timer;

class ClientData
{
public:
    sockaddr_in address_;
    int sock_fd_;
    std::weak_ptr<Timer> weak_ptr_timer_;

};


class Timer
{
public:
    time_t expire_;
    std::shared_ptr<ClientData> shared_ptr_clientdata_;
    void (*CallBackFunc)(std::shared_ptr<ClientData> user_data);
    
};

class SortedTimerList
{
public:
    void AddTimer(std::shared_ptr<Timer> timer);
    bool AdjustTimer(std::shared_ptr<Timer> timer);
    bool DeleteTimer(std::shared_ptr<Timer> timer);
    void Tick();

private:
    void InsertTimer(std::shared_ptr<Timer> timer);

    std::list<std::shared_ptr<Timer>> timer_list_; //expire_小的在前（靠近head）,大的在后（靠近tail）
};

class Utils
{
public:
    void Init(int time_slot);

    int SetNonblocking(int fd); //对文件描述符fd设置非阻塞

    void AddFd(int epoll_fd,int fd,bool one_shot,int trig_mode); //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT

    static void SignalHandler(int signal); //信号处理函数

    void AddSignal(int signo,void(handler)(int),bool restart=true); //设置信号函数

    void TimerHandler(); //定时处理任务，重新定时以不断触发SIGALRM信号

    void ShowError(int connection_fd,std::string  info);

public:
    int timer_slot_;
    static std::shared_ptr<int[]> pipe_fd_;
    static int epoll_fd_;
    SortedTimerList sorted_timer_list_;
};

void CallBackFunc(std::shared_ptr<ClientData> user_data);

#endif