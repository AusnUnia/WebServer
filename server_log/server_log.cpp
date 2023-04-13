#include"server_log.h"

#include<thread>
#include<fmt/format.h>

#include<sys/time.h>
#include<stdarg.h>

std::shared_ptr<ServerLog> ServerLog::singleton_log_;
std::once_flag ServerLog::singleton_flag_;


ServerLog::ServerLog()
{

}

ServerLog::~ServerLog()
{
    fs_.close();
}
//异步需要设置阻塞队列的长度，同步不需要设置
bool ServerLog::Init(std::string file_name, int close_log, int log_buffer_size, int max_split_lines, int max_queue_size)
{
    //如果设置了max_queue_size,则设置为异步
    if (max_queue_size >= 1)
    {
        is_async_ = true;
        log_queue_.resize(max_queue_size);
        //FlushLogThreadEntry为回调函数,这里表示创建线程异步写日志
        std::thread flush_log_thread(FlushLogThreadEntry);
    }
    
    close_log_ = close_log;
    log_buffer_size_ = log_buffer_size;
    buffer_.resize(log_buffer_size,'\0');
    max_split_lines_ = max_split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    int last_slash_pos=file_name.rfind('/');
    std::string log_full_name;

    if (last_slash_pos == std::string::npos)
    {
        log_full_name=fmt::format("{:d}_{:02d}_{:02d}_{:s}", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    else
    {
        log_name_=file_name.substr(last_slash_pos);
        dir_name_=file_name.substr(0,last_slash_pos+1);
        log_full_name=fmt::format("{:s}{:d}_{:02d}_{:02d}_{:s}", dir_name_, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name_);
    }

    today_ = my_tm.tm_mday;
    
    fs_.open(log_full_name,std::ios::app);

    if (!fs_ )
    {
        return false;
    }

    return true;
}

void ServerLog::WriteLog(int level, std::string format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    std::string level_str;
    
    switch (level)
    {
        case 0:
            level_str="[debug]:";
            break;
        case 1:
            level_str="[info]:";
            break;
        case 2:
            level_str="[warn]:";
            break;
        case 3:
            level_str="[erro]:";
            break;
        default:
            level_str="[info]:";
            break;
    }

    //写入一个log，对m_count++, m_split_lines最大行数
    std::unique_lock<std::mutex> uni_lock(log_mutex_);
    line_count_++;

    if (today_ != my_tm.tm_mday || line_count_ % max_split_lines_ == 0) //每天换新日志文件，超出日志最大行数也要换新日志文件
    {
        std::string new_log;
        fs_.flush();
        fs_.close();
        std::string tail;
       
        tail=fmt::format("{:d}_{:02d}_{:02d}_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
       
        if (today_ != my_tm.tm_mday)
        {
            new_log=fmt::format("{:s}{:s}{:s}", dir_name_, tail, log_name_);
            today_ = my_tm.tm_mday;
            line_count_= 0;
        }
        else
        {
            new_log=fmt::format("{:s}{:s}{:s}.{:d}", dir_name_, tail, log_name_, line_count_ / max_split_lines_);
        }

        fs_.open(new_log,std::ios::app);
    }
 
    uni_lock.unlock();

    va_list valst;
    va_start(valst, format);

    std::string log_str;
    uni_lock.lock();

    //写入的具体时间内容格式
    buffer_=fmt::format("{:d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:06d} {:s} ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, level_str);

    vsnprintf(buffer_.data() + buffer_.size()-1, log_buffer_size_-1, format.c_str(), valst);
    
    buffer_.push_back('\n');

    log_str = buffer_;

    uni_lock.unlock();

    if (is_async_ && !log_queue_.full())
    {
        log_queue_.push(log_str);
    }
    else
    {
        uni_lock.lock();
        fs_<<log_str;
        uni_lock.unlock();
    }

    va_end(valst);
}

void ServerLog::Flush(void)
{
    std::unique_lock<std::mutex> uni_lock(log_mutex_);
    //强制刷新写入流缓冲区
    fs_.flush();
}