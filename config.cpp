#include"config.h"

#include<unistd.h>
#include<stdlib.h>


void Config::ParseArg(int argc, char* argv[])
{
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                port_ = atoi(optarg);
                break;
            }
            case 'l':
            {
                log_write_ = atoi(optarg);
                break;
            }
            case 'm':
            {
                trig_mode_ = atoi(optarg);
                break;
            }
            case 'o':
            {
                opt_linger_ = atoi(optarg);
                break;
            }
            case 's':
            {
                sql_num_ = atoi(optarg);
                break;
            }
            case 't':
            {
                thread_num_ = atoi(optarg);
                break;
            }
            case 'c':
            {
                close_log_ = atoi(optarg);
                break;
            }
            case 'a':
            {
                actor_model_ = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
}