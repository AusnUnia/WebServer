#include"thread_pool.h"
#include<iostream>

template<class T>
ThreadPool<T>::ThreadPool()
: max_task_num_(20), task_queue_(20), thread_num_(4), threads_(thread_num_)
{
    for(int i=0;i<thread_num_;i++)
    {
        threads_[i]=new std::thread(worker_thread_entry_,this);
        threads_[i]->detach();
    }
}

template<class T>
ThreadPool<T>::ThreadPool(int max_task_num, int thread_num)
:max_task_num_(max_task_num),thread_num_(thread_num)
{
    if(thread_num_<=0||max_task_num_<=0)
    {
        std::cerr<<"Thread number and task number must be larger than 0\n";
        throw std::exception();
    }
    
    threads_.resize(thread_num_);

    for(int i=0;i<thread_num_;i++)
    {
        threads_[i]=new std::thread(worker_thread_entry_,this);
        threads_[i]->detach();
    }
}

template<class T>
ThreadPool<T>::~ThreadPool()
{

}

template<class T>
bool ThreadPool<T>::AddTask(T* task)
{
    std::lock_guard<std::mutex> guard(task_queue_mutex_);
    if(task_queue_.size()>=max_task_num_)
    {
        std::cerr<<"Task_queue_ is full\n";
        return false;
    }

    task_queue_.push(task);
    return true;
}


template<class T>
ThreadPool<T>* ThreadPool<T>::worker_thread_entry_(ThreadPool<T>* this_of_thread_pool)
{
    this_of_thread_pool->run();
    return this_of_thread_pool;
}

template<class T>
void ThreadPool<T>::run()
{
    while(1)
    {
        std::lock_guard<std::mutex> tmp_lock_guard(task_queue_mutex_);
        if(task_queue_.empty())
        {
            continue;
        }
    }
}


int main()
{
    class task
    {
        
    };
    ThreadPool<task> tp(1,-1);
}