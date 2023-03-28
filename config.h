#ifndef CONFIG_H
#define CONFIG_H

class Config
{
public:
    Config()=default;
    ~Config()=default;

    void ParseArg(int argc, char*argv[]);

    //端口号
    int port_{9006};

    //日志写入方式，默认同步
    int log_write_{0};

    //触发组合模式,默认listenfd LT + connfd LT
    int trig_mode_{0};

    //listen_fd_触发模式，默认LT
    int listen_trig_mode_{0};

    //connect_fd_触发模式，默认LT
    int connect_mode_{0};

    //优雅关闭链接，默认不使用
    int opt_linger_{0};

    //数据库连接池数量
    int sql_num_{8};

    //线程池内的线程数量
    int thread_num_{8};

    //是否关闭日志,默认不关闭
    int close_log_{0};

    //并发模型选择,默认是proactor
    int actor_model_{0};
};





#endif