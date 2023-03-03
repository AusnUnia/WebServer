#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<list>
#include<thread>
#include<mutex>
#include<vector>
#include<exception>

#include"../lock/my_semaphore.h"

template<class T>
class ThreadPool
{
    public:
        ThreadPool();
        ThreadPool(int max_task_num, int thread_num);
        ~ThreadPool();

        bool AddTask(std::weak_ptr<T> task);
    private:
        static ThreadPool<T>* worker_thread_entry_(ThreadPool<T>*); //worker线程的入口函数，必须申明为static，因为初始化的时候就会调用该函数。
        void Run();
    private:
        int max_task_num_; //task_list_中能存在的最大任务数量
        std::list< std::weak_ptr<T> > task_list_; //等待被线程执行的任务队列,用weak_ptr管理任务，避免该处影响到任务本身的生命周期。
        int thread_num_; //线程数量
        Semaphore sem_;  //用于线程间消息传递
        std::vector<std::thread*> threads_; //装有所有线程的vector
        std::mutex task_list_mutex_;  //用来保护task_list_lock的互斥量
};


//实现
#include<iostream>
#include<stdlib.h>

template<class T>
ThreadPool<T>::ThreadPool()
: max_task_num_{10}, thread_num_{2}, threads_(thread_num_)
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
    task_list_mutex_.lock();
    if(task_list_.size()>=max_task_num_)
    {
        task_list_mutex_.unlock();
        sem_.Signal();
        std::cerr<<"task_list_ is full\n";
        return false;
    }
    task_list_.push_back(task);
    task_list_mutex_.unlock();

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
        task_list_mutex_.lock();
        if(task_list_.empty())  //在sem_.Wait()中虽然已经判断过count_>0了，但是从sem_.Wait()到task_list_mutex_lock()并非原子操作，中途可能有其他线程插手修改了task_list_,所以必须再次检查task_list_是否为空
        {
            task_list_mutex_.unlock();
            continue;
        }

        std::weak_ptr<T>& weak_ptr_task=task_list_.front(); //取出队列最前面的任务
        task_list_.pop_front();
        task_list_mutex_.unlock(); 
        
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


#endif