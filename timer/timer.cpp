#include"timer.h"


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