#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<queue>
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

        bool AddTask(T* task);
    private:
        static ThreadPool<T>* worker_thread_entry_(ThreadPool<T>*); //worker线程的入口函数，必须申明为static，因为初始化的时候就会调用该函数。
        void run();
    private:
        int max_task_num_; //task_queue_中能存在的最大任务数量
        std::queue<T *> task_queue_; //等待被线程执行的任务队列
        int thread_num_; //线程数量
        Semaphore sem_;  //用于线程间消息传递
        std::vector<std::thread*> threads_; //装有所有线程的vector
        std::mutex task_queue_mutex_; //用来保护task_queue_lock的互斥量
};


#endif