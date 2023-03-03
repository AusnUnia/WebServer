#include "mysql_connection_pool.h"
#include <iostream>

MysqlConnectionPool::MysqlConnectionPool()
{

}

std::unique_ptr<MysqlConnectionPool> MysqlConnectionPool::GetInstance()
{
    static MysqlConnectionPool ret;
    return std::make_unique<MysqlConnectionPool> (&ret);
}

void MysqlConnectionPool::Init(std::string url,std::string user,std::string pass_word,std::string db_name,int port,int max_connection_num,int close_log)
{
    if(max_connection_num<=0)
    {
        std::cerr<<"max_connection_num must be larger than 0\n";
        return;
    }
    
    url_=url;
    user_=user;
    pass_word_=pass_word;
    db_name_=db_name;
    max_connection_num_=max_connection_num;
    port_=port;
    close_log_=close_log;

    for(int i=0;i<max_connection_num_;i++)
    {
		MYSQL *con = NULL;
		MYSQL *ret = NULL;

		ret = mysql_init(con);

		if (ret == NULL)
		{
			std::cerr<<"MySQL Error: mysql_init() return NULL\n";
			exit(1);
		}
		else
		{
			con=ret;
		}

		ret = mysql_real_connect(con, url_.c_str(), user_.c_str(), pass_word_.c_str(), db_name_.c_str(), port_, nullptr, 0);

		if (ret == NULL)
		{
			std::string err_info(mysql_error(con));
			err_info=(std::string("MySQL Error[Error=")+std::to_string(mysql_errno(con))+std::string("]: ")+err_info);
			std::cerr<<"MySQL Error: mysql_init() return NULL\n";
			exit(1);
		}
		else
		{
			con=ret;
		}
        
		connection_queue_.push(con);
		free_connection_num_++;
    }

    return;

}