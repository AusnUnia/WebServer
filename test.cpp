
#include "server.h"
#include "config.h"

int main(int argc,char* argv[])
{
    std::string user = "Ausn";
    std::string passwd = "FXYiloveyou1314@";
    std::string database_name = "mydb";

    Config config;
    config.ParseArg(argc,argv);

    std::cout<<"ParseArg() success."<<std::endl;


    Server server;
    server.Init(config.port_, user, passwd, database_name, config.log_write_, 
            config.opt_linger_, config.trig_mode_,  config.sql_num_,  config.thread_num_, 
            config.close_log_, config.actor_model_);

    std::cout<<"server.Init() success."<<std::endl;

    server.LogWrite();

    std::cout<<"server.LogWrite() success."<<std::endl;

    server.SqlPool();

    std::cout<<"server.SqlPool() success."<<std::endl;

    server.ThreadPoolInit();

    std::cout<<"server.ThreadPoolInit() success."<<std::endl;

    server.TrigMode();

    std::cout<<"server.TrigMode() success."<<std::endl;

    server.EventListen();

    std::cout<<"server.EventListen() success."<<std::endl;

    server.EventLoop();

    std::cout<<"server.EventLoop() success."<<std::endl;

    return 0;
    
}