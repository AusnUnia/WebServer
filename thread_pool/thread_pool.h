#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<list>
#include<thread>
#include<mutex>
#include<vector>
#include<exception>

#include"../lock/my_semaphore.h"
#include"../mysql_connection_pool/mysql_connection_pool.h"

template<class T>
class ThreadPool
{
    public:
        ThreadPool();
        ThreadPool(int actor_model, int max_task_num, int thread_num);
        ~ThreadPool();

        bool AddTask(std::weak_ptr<T> task);
    private:
        static ThreadPool<T>* worker_thread_entry_(ThreadPool<T>*); //worker线程的入口函数，必须申明为static，因为初始化的时候就会调用该函数。
        void Run();
    private:
        int max_task_num_; //task_list_中能存在的最大任务数量
        mutable std::mutex task_list_mutex_;  //用来保护task_list_lock的互斥量,必须通过unique_lock或lock_guard来管理使用。
        std::list< std::weak_ptr<T> > task_list_; //等待被线程执行的任务队列,用weak_ptr管理任务，避免该处影响到任务本身的生命周期。
        
        int thread_num_; //线程数量
        mutable Semaphore sem_;  //用于线程间消息传递
        std::vector<std::unique_ptr<std::thread>> threads_; //装有所有线程的vector

        std::shared_ptr<MysqlConnectionPool> connection_pool_;  //mysql数据库连接池

        int actor_model_;    //模型切换
};


//实现
#include<iostream>
#include<stdlib.h>

template<class T>
ThreadPool<T>::ThreadPool()
: max_task_num_{10}, thread_num_{4}, threads_(thread_num_), actor_model_{0}
{
    for(int i=0;i<thread_num_;i++)
    {
        threads_[i]=std::move( std::make_unique<std::thread>(worker_thread_entry_,this) );
        threads_[i]->detach();
    }
}

template<class T>
ThreadPool<T>::ThreadPool(int actor_model,int max_task_num, int thread_num)
:actor_model_{actor_model}, max_task_num_{max_task_num},thread_num_{thread_num}
{
    if(thread_num_<=0||max_task_num_<=0)
    {
        std::cerr<<"Thread number and task number must be larger than 0\n";
        throw std::exception();
    }
    
    threads_.resize(thread_num_);

    for(int i=0;i<thread_num_;i++)
    {
        threads_[i]=std::move( std::make_unique<std::thread>(worker_thread_entry_,this) );
        threads_[i]->detach();
    }
}

template<class T>
ThreadPool<T>::~ThreadPool()
{

}

template<class T>
bool ThreadPool<T>::AddTask(std::weak_ptr<T> task)
{
    std::cout<<"ThreadPool<T>::AddTask()"<<std::endl;
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
        std::unique_lock<std::mutex> task_list_unique_lock(task_list_mutex_);
        if(task_list_.empty())  //在sem_.Wait()中虽然已经判断过count_>0了，但是从sem_.Wait()到task_list_mutex_lock()并非原子操作，中途可能有其他线程插手修改了task_list_,所以必须再次检查task_list_是否为空
        {
            task_list_unique_lock.unlock();
            continue;
        }

        std::weak_ptr<T>& weak_ptr_task=task_list_.front(); //取出队列最前面的任务
        task_list_.pop_front();
        task_list_unique_lock.unlock(); 
        
        std::cout<<std::this_thread::get_id()<<" thread ready to do sth.\n";
        std::shared_ptr<T> shared_ptr_task=weak_ptr_task.lock();
        std::cout<<"ThreadPool<T>::Run() 1"<<std::endl;
        if(!shared_ptr_task)
        {
            continue;
        }
        std::cout<<"ThreadPool<T>::Run() 2"<<std::endl;
        if(actor_model_==1)
        {
            if(shared_ptr_task->state_==0)//读入
            {
                if(shared_ptr_task->ReadOnce())
                {
                    shared_ptr_task->improve_=1;
                    MysqlConnectionRAII mysql_connection(shared_ptr_task->mysql_,connection_pool_);
                    shared_ptr_task->Process();
                }
                else
                {
                    shared_ptr_task->improve_=1;
                    shared_ptr_task->timer_flag_=1;
                }
            }
            else //写入
            {
                if(shared_ptr_task->Write())
                {
                    shared_ptr_task->improve_=1;
                }
                else
                {
                    shared_ptr_task->improve_=1;
                    shared_ptr_task->timer_flag_=1;
                }
            }
        }
        else
        {
            std::cout<<"ThreadPool<T>::Run() 3"<<std::endl;
            MysqlConnectionRAII mysql_connection(shared_ptr_task->mysql_,connection_pool_);
            std::cout<<"ThreadPool<T>::Run() 4"<<std::endl;
            shared_ptr_task->Process();
            std::cout<<"ThreadPool<T>::Run() 5"<<std::endl;
        }

        
        std::cout<<std::this_thread::get_id()<<"  doing sth.\n";

    }
}


#endif