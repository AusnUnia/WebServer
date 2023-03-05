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
void SortedTimerList::AdjustTimer(std::shared_ptr<Timer> timer)
{
    if(!timer)
    {
        return;
    }

}
void SortedTimerList::DeleteTimer(std::shared_ptr<Timer> timer)
{

}
void SortedTimerList::Tick()
{

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