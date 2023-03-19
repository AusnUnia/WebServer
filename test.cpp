#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

#include "thread_pool/thread_pool.h"
#include "mysql_connection_pool/mysql_connection_pool.h"
#include "timer/timer.h"
#include "server.h"

class Task
{
    public:
    Task(int sock):sock_(sock)
    {

    }
    std::string header_{"GET"};
    std::string req_{""};
    int sock_;
    void Process()
    {
        char buf[512];

        FILE* read_stream=fdopen(sock_,"r");
        FILE* write_stream=fdopen(dup(sock_),"w");

        fgets(buf,512,read_stream);
        req_=buf;
        std::cout<<req_<<std::endl;

        FILE* html_file=fopen("./html/test.html","r");
        if(html_file==nullptr)
        {
            std::cout<<"open error!"<<std::endl;
            return;
        }

        while(fgets(buf,512,html_file)!=nullptr)
        {
            fputs(buf,write_stream);
            fflush(write_stream);
        }
        fflush(write_stream);
        fclose(write_stream);
        fclose(read_stream);
    }
};


int main()
{
    
    std::unique_ptr<MysqlConnectionPool> unique{MysqlConnectionPool::GetInstance()};
    Server server;
    server.Init(8966,"root","FXYiloveyou1314","mydb",0,0,0,8,4,0,0);

    SortedTimerList lst;
    auto timer=std::make_shared<Timer>();
    lst.AddTimer(std::make_shared<Timer>());
    lst.AddTimer(timer);
    lst.AdjustTimer(timer);


    int server_sock=0;
    int client_sock=0;

    struct sockaddr_in server_addr,client_addr;

    server_sock=socket(PF_INET,SOCK_STREAM,0);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(9190);
    server_addr.sin_family=AF_INET;

    std::cout<<server_sock<<std::endl;

    if( bind(server_sock,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1 )
    {
        std::cout<<"bind errer!"<<std::endl;
    }

    if(listen(server_sock,20)==-1)
    {
        std::cout<<"lsiten errer!"<<std::endl;
    }


    ThreadPool<Task> pool;

    while(true)
    {
        std::cout<<"listenning..."<<std::endl;
        socklen_t client_addr_size=sizeof(client_addr);
        client_sock=accept(server_sock,(struct sockaddr*)&client_addr,&client_addr_size);

        std::cout<<client_sock<<std::endl;
        
        std::cout<<"connection request : "<<ntohs(client_addr.sin_port)<<std::endl;

        std::shared_ptr<Task> tsk {std::make_shared<Task>(client_sock)};

        pool.AddTask(tsk);

    }

    close(client_sock);

}