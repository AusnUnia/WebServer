#ifndef TIMER_H
#define TIMER_H

#include<list>
#include<memory>
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
    
};

class SortedTimerList
{
public:
    void AddTimer(std::shared_ptr<Timer> timer);
    void AdjustTimer(std::shared_ptr<Timer> timer);
    void DeleteTimer(std::shared_ptr<Timer> timer);
    void Tick();

private:
    void InsertTimer(std::shared_ptr<Timer> timer);

    std::list<std::shared_ptr<Timer>> timer_list_; //expire_小的在前（靠近head）,大的在后（靠近tail）
};





#endif