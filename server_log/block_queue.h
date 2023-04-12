#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include<mutex>
#include<condition_variable>
#include<memory>
#include<queue>
#include<chrono>


template <class T>
class BlockQueue final
{
public:
    BlockQueue(int max_size = 1000):max_size_ {max_size},std_queue(max_size+24)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        max_size_ = max_size;
    }

    void clear()
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        std_queue.clear();
    }

    ~BlockQueue()
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
    }
    //判断队列是否满了
    bool full() 
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        if (std_queue.size() >=max_size)
        {
            return true;
        }
        return false;
    }
    //判断队列是否为空
    bool empty() 
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        return std_queue.empty();
    }

    //返回队首元素
    T& front() 
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        if (std_queue.empty())
        {
            std::cerr<<"BlockQueue is empty!"<<std::endl;
            return nullptr;
        }
        return std_queue.front();
    }
    //返回队尾元素
    T& back() 
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        if (std_queue.empty())
        {
            std::cerr<<"BlockQueue is empty!"<<std::endl;
            return nullptr;
        }
        return std_queue.front();
    }

    int size() 
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        return std_queue.size();
    }

    int max_size()
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        return max_size_;
    }
    //往队列添加元素，需要将所有使用队列的线程先唤醒
    //当有元素push进队列,相当于生产者生产了一个元素
    //若当前没有线程等待条件变量,则唤醒无意义
    bool push(const T &item)
    {

        std::unique_lock<std::mutex> uni_lock(mutex_);
        if (std_queue.size()>=max_size_)
        {
            cond_.notify_all();
            return false;
        }

        std_queue.push(item);

        cond_.notify_all();
        return true;
    }
    //pop时,如果当前队列没有元素,将会等待条件变量
    bool pop()
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        cond_.wait(uni_lock,[&std_queue]{return !std_queue.empty();});

        std_queue.pop();
        return true;
    }

    //增加了超时处理
    bool pop(T &item, int ms_timeout)
    {
        std::chrono::microseconds time_span(ms_timeout);
        std::unique_lock<std::mutex> uni_lock(mutex_);
        cond_.wait_for(uni_lock,time_span,[&std_queue]{return !std_queue.empty();});
        if(std_queue.empty())
        {
            return false;
        }

        std_queue.pop();
        return true;
    }

    void resize(int new_max_size)
    {
        std::unique_lock<std::mutex> uni_lock(mutex_);
        max_size_=new_max_size;
    }

private:
    std::condition_variable cond_;
    std::mutex mutex_;
    std::queue<T> std_queue;

    int max_size_;
};



#endif