#ifndef MYSQL_CONNECTION_POOL_H
#define MYSQL_CONNECTION_POOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <memory>
#include <atomic>

#include "lock/my_semaphore.h"

class MysqlConnectionPool
{
public:
    static std::unique_ptr<MysqlConnectionPool> GetInstance(); //获取一个单例子

    void Init(std::string url,std::string user,std::string pass_word,std::string db_name,int port,int max_connection_num,int close_log);
    
    void Destroy();

private:
    MysqlConnectionPool();
    ~MysqlConnectionPool();

	int max_connection_num_;  //最大连接数
	std::atomic<int> busy_connection_num_{0};  //当前已使用的连接数
	std::atomic<int> free_connection_num_{0}; //当前空闲的连接数
	std::mutex connection_queue_mutex_; //
	std::queue<MYSQL*> connection_queue_; //连接池
	Semaphore sem_;  //用于线程间消息传递

public:
	std::string url_;			 //主机地址
	int port_;		 //数据库端口号
	std::string user_;		 //登陆数据库用户名
	std::string pass_word_;	 //登陆数据库密码
	std::string db_name_; //使用数据库名
	int close_log_;	  //日志开关

};











#endif