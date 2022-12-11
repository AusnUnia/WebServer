#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<queue>
#include<thread>
#include<mutex>

template<class T>
class ThreadPool
{
    public:
        ThreadPool();
        ~ThreadPool();
    private:
        ThreadPool<T>* worker_thread_entry_(ThreadPool<T>*); //the start function of worker thread.
        void run();
    private:
        int max_task_num_; //the maximum number of tasks in task_queue_.
        std::queue<T> task_queue_; //a queue where tasks waiting to be executed by a thread.
        int thread_num_; //the number of threads.
        std::thread* threads_; //a pointer which points to the array of threads.
        std::mutex task_queue_lock_; //a lock to protect task_queue_.
};

template<class T>
ThreadPool<T>::ThreadPool(): max_task_num_(20), task_queue_(20), thread_num_(4)
{
    threads_=new std::thread[thread_num_];
    for(int i=0;i<thread_num_;i++)
    {
        threads_[i]=std::move(std::thread(worker_thread_entry_,this));
        threads_[i].detach();
    }
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
        std::lock_guard<std::mutex> tmp_lock_guard(task_queue_lock_);
        if(task_queue_.empty())
        {
            tmp_lock_guard.~lock_guard();
            continue;
        }
    }
}

#endif