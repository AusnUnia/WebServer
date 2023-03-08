#include"timer.h"

#include"../http/http_connection.h"

#include<fcntl.h>
#include<sys/epoll.h>
#include<signal.h>
#include<string.h>

#include<assert.h>

//SortedTimerList定义
void SortedTimerList::AddTimer(std::shared_ptr<Timer> timer)
{
    if(!timer)
    {
        return;
    }
    if(timer_list_.empty())
    {
        timer_list_.push_back(timer);
    }
    if(timer->expire_>timer_list_.back()->expire_)
    {
        timer_list_.push_back(timer);
    }    
    if(timer->expire_<timer_list_.front()->expire_)
    {
        timer_list_.push_front(timer);
    }

    InsertTimer(timer);

}

bool SortedTimerList::AdjustTimer(std::shared_ptr<Timer> timer)
{
    if(!timer)
    {
        return false;
    }

    bool result=DeleteTimer(timer);
    AddTimer(timer);

    return result;
}

bool SortedTimerList::DeleteTimer(std::shared_ptr<Timer> timer)
{
    for(auto it=timer_list_.begin();it!=timer_list_.end();it++)
    {
        if((*it).get()==timer.get())
        {
            timer_list_.erase(it);
            return true;
        }
    }

    return false;
}

void SortedTimerList::Tick()
{
    if(timer_list_.empty())
    {
        return;
    }

    time_t time_now=time(nullptr);

    for(auto it=timer_list_.begin();it!=timer_list_.end();it++)
    {
        if(time_now<(*it)->expire_)
        {
            break;
        }
        (*it)->CallBackFunc((*it)->shared_ptr_clientdata_);
        timer_list_.erase(it);
    }
}

void SortedTimerList::InsertTimer(std::shared_ptr<Timer> timer)
{
    for(auto it=timer_list_.begin();it!=timer_list_.end();it++)
    {
        if(timer->expire_ < (*it)->expire_)
        {
            timer_list_.insert(it,timer);
        }
    }
}


//Utils定义

void Utils::Init(int time_slot)
{
    timer_slot_=time_slot;
    return;
}

//对文件描述符fd设置非阻塞
int Utils::SetNonblocking(int fd)
{
    int old_flag=fcntl(fd,F_GETFL);
    int new_flag=old_flag|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_flag);
    return old_flag;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::AddFd(int epoll_fd,int fd,bool one_shot,int trig_mode)
{
    epoll_event event;
    event.data.fd=fd;

    if(trig_mode==1)
        event.events=EPOLLIN|EPOLLET|EPOLLRDHUP;
    else
        event.events=EPOLLIN|EPOLLRDHUP;
    
    if(one_shot)
        event.events|=EPOLLONESHOT;
    
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);

    SetNonblocking(fd);

}

//信号处理函数
void Utils::SignalHandler(int signal)
{
    int old_errno=errno;
    int msg=signal;
    send(pipe_fd_[1],(char*)&msg,1,0);
}

//设置信号函数
void Utils::AddSignal(int signo,void(handler)(int),bool restart)
{
    struct sigaction sig_act;
    memset(&sig_act,'\0',sizeof(sig_act));

    sig_act.sa_handler=handler;

    if(restart)
        sig_act.sa_flags|=SA_RESTART;
    
    sigfillset(&sig_act.sa_mask);

    assert(sigaction(signo,&sig_act,0)!=-1);

}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::TimerHandler()
{
    sorted_timer_list_.Tick();
    alarm(timer_slot_);
}


void Utils::ShowError(int connection_fd,std::string info)
{
    send(connection_fd,info.c_str(),info.size(),0);
    close(connection_fd);
}


std::shared_ptr<int[]> Utils::pipe_fd_{nullptr};
int Utils::epoll_fd_{0};

void CallBackFunc(std::shared_ptr<ClientData> user_data)
{
    epoll_ctl(Utils::epoll_fd_,EPOLL_CTL_DEL,user_data->sock_fd_,0);
    assert(user_data);
    close(user_data->sock_fd_);
    HttpConnection::user_count_--;
}