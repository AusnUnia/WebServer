#ifndef SERVER_LOG_H
#define SERVER_LOG_H

#include<memory>
#include<mutex>

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
};



#endif