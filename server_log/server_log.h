#ifndef SERVER_LOG_H
#define SERVER_LOG_H

#include<memory>
#include<mutex>
#include<string>
#include<iostream>
#include<fstream>

#include"./block_queue.h"

class ServerLog
{
private:
    static std::shared_ptr<ServerLog> singleton_log_;
    static std::once_flag singleton_flag_;

public:   
    static std::shared_ptr<ServerLog> GetInstance()
    {
        std::call_once
        (
            singleton_flag_,
            [&]
            {
                singleton_log_=std::make_shared<ServerLog>();
            }
        );
        return singleton_log_;
    }

    static void *FlushLogThreadEntry(void *args)
    {
        ServerLog::GetInstance()->AsyncWriteLog();
    }

    bool Init(std::string file_name, int close_log, int log_buffer_size = 8192, int max_split_lines = 5000000, int max_queue_size = 0);

    void WriteLog(int level, const char *format, ...);

    void Flush(void);

private:
    ServerLog();
    virtual ~ServerLog();
    void *AsyncWriteLog()
    {
        std::string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        single_log=log_queue_.front();
        while (!single_log.empty())
        {
            log_queue_.pop();

            std::unique_lock<std::mutex> uni_lock(log_mutex_);
            fs_<<single_log;
            uni_lock.unlock();

            single_log=log_queue_.front();
        }
    }

private:
    std::string dir_name_; //路径名
    std::string log_name_; //log文件名
    int max_split_lines;  //日志最大行数
    int log_buffer_size_; //日志缓冲区大小
    long long line_count_{0};  //日志行数记录
    int today_;        //因为按天分类,记录当前时间是那一天
    std::ofstream fs_;         //打开log的文件指针
    std::string buffer_;
    BlockQueue<std::string> log_queue_; //阻塞队列
    bool is_async_{false};                  //是否同步标志位
    std::mutex log_mutex_;
    int close_log_; //关闭日志
};

#define LOG_DEBUG(format, ...) if(0 == close_log_) {ServerLog::GetInstance()->WriteLog(0, format, ##__VA_ARGS__); ServerLog::GetInstance()->Flush();}
#define LOG_INFO(format, ...) if(0 == close_log_) {ServerLog::GetInstance()->WriteLog(1, format, ##__VA_ARGS__); ServerLog::GetInstance()->Flush();}
#define LOG_WARN(format, ...) if(0 == close_log_) {ServerLog::GetInstance()->WriteLog(2, format, ##__VA_ARGS__); ServerLog::GetInstance()->Flush();}
#define LOG_ERROR(format, ...) if(0 == close_log_) {ServerLog::GetInstance()->WriteLog(3, format, ##__VA_ARGS__); ServerLog::GetInstance()->Flush();}



#endif