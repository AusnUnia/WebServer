#ifndef MYSQL_CONNECTION_POOL_H
#define MYSQL_CONNECTION_POOL_H

#include <mysql/mysql.h>
#include <string>
#include <list>
#include <memory>
#include <atomic>

#include "lock/my_semaphore.h"

class MysqlConnectionPool
{
public:
	std::shared_ptr<MYSQL> GetConnection();				 //获取数据库连接
	bool ReleaseConnection(std::shared_ptr<MYSQL> conn); //释放连接
	int get_free_connection_num();					 //获取连接
	void DestroyPool();					 //销毁所有连接


    static std::unique_ptr<MysqlConnectionPool> GetInstance(); //获取一个单例

    void Init(std::string url,std::string user,std::string pass_word,std::string db_name,int port,int max_connection_num,int close_log);

private:
    MysqlConnectionPool();
    ~MysqlConnectionPool();

	int max_connection_num_;  //最大连接数
	std::atomic<int> busy_connection_num_{0};  //当前已使用的连接数
	std::atomic<int> free_connection_num_{0}; //当前空闲的连接数
	std::mutex connection_list_mutex_; //保护connection_list_的互斥量
	std::list<std::shared_ptr<MYSQL>> connection_list_; //连接池
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