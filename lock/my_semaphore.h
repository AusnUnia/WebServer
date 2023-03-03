#ifndef MY_SEMAPHORE_H
#define MY_SEMAPHORE_H

#include<memory>
#include<mutex>
#include<condition_variable>


class Semaphore 
{
    public:
        Semaphore() : count_{0}
        {
        }

        Semaphore(int count) : count_{count}
        {
        }

        void Signal() 
        {
            std::unique_lock<std::mutex> lock(count_mutex_);
            count_++;
            cv_.notify_one();
        }

        void Wait() 
        {
            std::unique_lock<std::mutex> lock(count_mutex_);  //count_mutex_会被线程竞争持有，但是当一个线程持有了它，很快就会由于cv_.wait的条件不满足而释放它，然后进入阻塞状态，或者cv_wait条件满足继续向下执行离开lock的作用域而释放它。
            cv_.wait(lock, [=] { return count_ > 0; });
            count_--;
        }

        void set_count(int count)
        {
            count_=count;
        }

    private:
        std::mutex count_mutex_;        
        std::condition_variable cv_;
        int count_;
};

#endif