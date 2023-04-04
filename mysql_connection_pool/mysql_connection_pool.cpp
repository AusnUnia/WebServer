#include "mysql_connection_pool.h"

#include <iostream>

std::shared_ptr<MYSQL> MysqlConnectionPool::GetConnection()
{
	std::unique_lock<std::mutex> uni_lock(connection_list_mutex_);

	if(connection_list_.size()==0)
	{
		uni_lock.unlock();
		return nullptr;
	}

	std::shared_ptr<MYSQL> sh_conn {connection_list_.front()};
	connection_list_.pop_front();
	uni_lock.unlock();

	return sh_conn;
}

bool MysqlConnectionPool::ReleaseConnection(std::shared_ptr<MYSQL> sh_conn)
{
	if(!sh_conn)
	{
		return false;
	}

	std::unique_lock<std::mutex> connection_list_unique_lock(connection_list_mutex_);

	connection_list_.push_back(sh_conn);
	free_connection_num_++;
	busy_connection_num_--;

	connection_list_unique_lock.unlock();

	sem_.Signal();

	return true;

}

int MysqlConnectionPool::get_free_connection_num()
{
	return free_connection_num_;
}

void MysqlConnectionPool::DestroyPool()
{
	connection_list_mutex_.lock();
	
	if(connection_list_.size()>0)
	{
		for(auto it=connection_list_.begin();it!=connection_list_.end();it++)
		{
			std::shared_ptr<MYSQL> sh_conn=std::move(*it);
			mysql_close(sh_conn.get());
		}
	}

	connection_list_.clear();

	connection_list_mutex_.unlock();

	return;

}


MysqlConnectionPool::MysqlConnectionPool()
{
	
}

MysqlConnectionPool::~MysqlConnectionPool()
{
	DestroyPool();
}


std::once_flag MysqlConnectionPool::singleton_flag_;
std::shared_ptr<MysqlConnectionPool> MysqlConnectionPool::singleton_pool_{nullptr};
std::shared_ptr<MysqlConnectionPool> MysqlConnectionPool::GetInstance()
{
    std::call_once
	(
		singleton_flag_,
		[&]
		{
			singleton_pool_=std::make_shared<MysqlConnectionPool>();
		}
	);
    return singleton_pool_;
}

void MysqlConnectionPool::Init(std::string url,std::string user,std::string pass_word,std::string db_name,int port,int max_connection_num,int close_log)
{
	if(max_connection_num<=0)
    {
        std::cout<<"max_connection_num must be larger than 0\n";
        return;
    }

    
    url_=url;
    user_=user;
    pass_word_=pass_word;
    db_name_=db_name;
    port_=port;
    close_log_=close_log;

    for(int i=0;i<max_connection_num;i++)
    {

		MYSQL *con = NULL;
		MYSQL *ret = NULL;

		ret = mysql_init(con);

		if (ret == NULL)
		{
			std::cout<<"MySQL Error: mysql_init() return NULL\n";
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
			std::cout<<"MySQL Error: mysql_init() return NULL\n";
			exit(1);
		}
		else
		{
			con=ret;
		}

		std::shared_ptr<MYSQL> sh_con(con);

		connection_list_.push_back(sh_con);

		free_connection_num_++;
    }


    max_connection_num_=free_connection_num_;

    sem_.set_count(max_connection_num_);

    return;

}


MysqlConnectionRAII::MysqlConnectionRAII(std::shared_ptr<MYSQL>& connection, std::shared_ptr<MysqlConnectionPool> connection_pool)
{
	connection=connection_pool->GetConnection();

	connection_=connection;
	connection_pool_=connection_pool;
}
MysqlConnectionRAII::~MysqlConnectionRAII()
{
	connection_pool_->ReleaseConnection(connection_);
}