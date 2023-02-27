#include"thread_pool.h"
#include<iostream>
#include<stdlib.h>

template<class T>
ThreadPool<T>::ThreadPool()
: max_task_num_{20}, thread_num_{4}, threads_(thread_num_)
{
    for(int i=0;i<thread_num_;i++)
    {
        threads_[i]=new std::thread(worker_thread_entry_,this);
        threads_[i]->detach();
    }
}

template<class T>
ThreadPool<T>::ThreadPool(int max_task_num, int thread_num)
:max_task_num_{max_task_num},thread_num_{thread_num}
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
    for(int i=0;i<thread_num_;i++)
    {
        delete threads_[i];
    }
}

template<class T>
bool ThreadPool<T>::AddTask(std::weak_ptr<T> task)
{
    task_queue_mutex_.lock();
    if(task_queue_.size()>=max_task_num_)
    {
        task_queue_mutex_.unlock();
        sem_.Signal();
        std::cerr<<"task_queue_ is full\n";
        return false;
    }
    task_queue_.push(task);
    task_queue_mutex_.unlock();

    sem_.Signal();  //告诉一个线程来新任务了

    return true;
}


template<class T>
ThreadPool<T>* ThreadPool<T>::worker_thread_entry_(ThreadPool<T>* this_of_thread_pool)
{
    this_of_thread_pool->Run();
    return this_of_thread_pool;
}

template<class T>
void ThreadPool<T>::Run()
{
    while(1)
    {
        sem_.Wait();
        task_queue_mutex_.lock();
        if(task_queue_.empty())  //在sem_.Wait()中虽然已经判断过count_>0了，但是从sem_.Wait()到task_queue_mutex_lock()并非原子操作，中途可能有其他线程插手修改了task_queue_,所以必须再次检查task_queue_是否为空
        {
            task_queue_mutex_.unlock();
            continue;
        }

        std::weak_ptr<T>& weak_ptr_task=task_queue_.front(); //取出队列最前面的任务
        task_queue_.pop();
        task_queue_mutex_.unlock(); 
        
        std::cout<<std::this_thread::get_id()<<"  ready sth.\n";
        std::shared_ptr<T> shared_ptr_task=weak_ptr_task.lock();
        if(!shared_ptr_task)
        {
            continue;
        }
        shared_ptr_task->Process();
        
        std::cout<<std::this_thread::get_id()<<"  doing sth.\n";

    }
}
