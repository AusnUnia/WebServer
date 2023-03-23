#include "http_connection.h"

#include<mysql/mysql.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include <sys/uio.h>
#include<string.h>
#include<charconv>



const std::string ok_200_title = "OK";
const std::string error_400_title = "Bad Request";
const std::string error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const std::string error_403_title = "Forbidden";
const std::string error_403_form = "You do not have permission to get file form this server.\n";
const std::string error_404_title = "Not Found";
const std::string error_404_form = "The requested file was not found on this server.\n";
const std::string error_500_title = "Internal Error";
const std::string error_500_form = "There was an unusual problem serving the request file.\n";


int HttpConnection::epoll_fd_{-1};
int HttpConnection::user_count_{0};

std::mutex users_mutex;
std::map<std::string,std::string> users;

//初始化该HttpConnection
void HttpConnection::InitMysqlResult(std::shared_ptr<MysqlConnectionPool> connection_pool)
{
    //从连接池中取一个连接,用MysqlConnectionRAII管理这个连接的生命周期,MysqlConnectionRAII析构时连接会回到连接池
    std::shared_ptr<MYSQL> mysql{nullptr};
    MysqlConnectionRAII(mysql,connection_pool);

    //在user表中检索username，passwd数据，浏览器端输入
    if(mysql_query(mysql.get(),"SELECT username,passwd FROM user")!=0)
    {
        std::cerr<<"SELECT error:"<<mysql_error(mysql.get())<<std::endl;
    }

    //从表中检索完整的结果集
    std::shared_ptr<MYSQL_RES> result{mysql_store_result(mysql.get())};

    //返回结果集中的列数
    int fields_num=mysql_num_fields(result.get());

    //返回所有字段结构的数组
    std::shared_ptr<MYSQL_FIELD[]> fields{mysql_fetch_fields(result.get())};

    //
    while(MYSQL_ROW row=mysql_fetch_row(result.get()))
    {
        std::string tmp1(row[0]);
        std::string tmp2(row[1]);
        users[tmp1]=tmp2;
    }

}

//对文件描述符设置非阻塞
int SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void AddFd(int epoll_fd, int fd, bool one_shot, int trig_mode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == trig_mode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonblocking(fd);
}

//从内核时间表删除描述符
void RemoveFd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void ModFd(int epoll_fd, int fd, int ev, int trig_mode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == trig_mode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

//初始化连接,外部调用初始化套接字地址
void HttpConnection::Init(int sock_fd, const sockaddr_in &addr, std::string_view root, int trig_mode,
            int close_log, std::string_view database_user, std::string_view database_password, std::string_view database_name)
{
    sock_fd_=sock_fd;
    address_=addr;

    AddFd(epoll_fd_,sock_fd_,true,trig_mode);
    user_count_++;

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    doc_root_=root;
    trig_mode_=trig_mode;
    close_log_=close_log;

    database_user_=database_user;
    database_password_=database_password;
    database_name_=database_name;

    Init();
}

//初始化新接受的连接
//check_state默认为分析请求行状态
void HttpConnection::Init()
{
    mysql_ = nullptr;
    bytes_to_send_ = 0;
    bytes_have_send_ = 0;
    check_state_ = CheckState::CHECK_STATE_REQUESTLINE;
    linger_ = false;
    method_ = Method::GET;
    url_ = "";
    version_ = "";
    content_length_ = 0;
    host_ = "";
    start_line_ = 0;
    checked_idx_ = 0;
    read_idx_ = 0;
    write_idx_ = 0;
    cgi_ = 0;
    state_ = 0;
    timer_flag_ = 0;
    improve_ = 0;

}

//关闭http连接
void HttpConnection::CloseConnection(bool real_close)
{
    if(real_close&&(sock_fd_!=-1))
    {
        std::cout<<"close: "<<sock_fd_<<std::endl;
        RemoveFd(epoll_fd_,sock_fd_); //从事件表中删除sock_fd_,然后关闭sock_fd_的连接
        sock_fd_=-1;
        user_count_--;
    }
}

//从状态机，用于分析出一行内容
//返回值为行的读取状态，有LINE_OK,LINE_BAD,LINE_OPEN
HttpConnection::LineStatus HttpConnection::ParseLine()
{
    char temp;
    for(;checked_idx_<read_idx_;++checked_idx_)
    {
        temp=read_buffer_[checked_idx_];
        if(temp=='\r')
        {
            if ((checked_idx_ + 1) == read_idx_)
                return LineStatus::LINE_OPEN;
            else if (read_buffer_[checked_idx_ + 1] == '\n')
            {
                read_buffer_[checked_idx_++] = '\0';
                read_buffer_[checked_idx_++] = '\0';
                return LineStatus::LINE_OK;
            }
            return LineStatus::LINE_BAD;
        }
        else if(temp=='\n')
        {
            if (checked_idx_ > 1 && read_buffer_[checked_idx_ - 1] == '\r')
            {
                read_buffer_[checked_idx_-1] = '\0';
                read_buffer_[checked_idx_++] = '\0';
                return LineStatus::LINE_OK;
            }
            return LineStatus::LINE_BAD;
        }
    }
    return LineStatus::LINE_OPEN;
}

//循环读取客户数据，直到无数据可读或对方关闭连接
//非阻塞ET工作模式下，需要一次性将数据读完
bool HttpConnection::ReadOnce()
{
    if (read_idx_ >= kReadBufferSize)
    {
        return false;
    }
    int bytes_read = 0;

    //LT读取数据，水平触发，每次来数据都会触发事件，所以不用一次读完
    if (0 == trig_mode_)
    {
        bytes_read = recv(sock_fd_, read_buffer_.data() + read_idx_, kReadBufferSize-read_idx_, 0);
        read_idx_+= bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        return true;
    }
    //ET读数据，边缘触发，socket上数据从无到有会触发事件一次之后不触发必须一次读完
    else
    {
        while (true)
        {
            bytes_read = recv(sock_fd_, read_buffer_.data() + read_idx_, kReadBufferSize-read_idx_, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            read_idx_+= bytes_read;
        }
        return true;
    }
}

//解析http请求行，获得请求方法，目标url及http版本号
HttpConnection::HttpCode HttpConnection::ParseRequestLine(std::string_view text)
{
    //提取method
    const char* method_tail = strpbrk(text.data(), " \t");
    std::string_view method = text.substr(0,method_tail-text.data());
    if (method=="GET")
        method_ = Method::GET;
    else if (method=="POST")
    {
        method_ = Method::POST;
        cgi_ = 1;
    }
    else
        return HttpCode::BAD_REQUEST;

    //提取url
    text=text.substr(method_tail-text.data()+1,-1);
    const char* url_tail=strpbrk(text.data()," \t");
    std::string_view url=text.substr(0,url_tail-text.data());
    if(url=="")
    {
        return HttpCode::BAD_REQUEST;
    }
    //url_=url;不忙赋值，待会儿还要裁剪url

    //提取http协议版本
    text=text.substr(url_tail-text.data()+1,-1);
    const char* version_tail=strpbrk(text.data()," \t");
    std::string_view version=text.substr(0,url_tail-text.data());
    if(version=="")
    {
        return HttpCode::BAD_REQUEST;
    }
    else if(version!="HTTP/1.1")
    {
        return HttpCode::BAD_REQUEST;
    }
    version_=version;


    //处理url，得到请求的文件路径
    if(url.substr(0,7)=="http://")
    {
        url.remove_prefix(7);
        int start_pos=url.find('/');
        url.remove_prefix(start_pos);
    } 
    else if (url.substr(0,8)=="https://")
    {
        url.remove_prefix(8);
        int start_pos=url.find('/');
        url.remove_prefix(start_pos);
    }

    if (url.empty() || url[0] != '/')
        return HttpCode::BAD_REQUEST;

    url_=url;
    //当url为/时，显示判断界面
    if (url_=="/")
        url_+="test.html";

    check_state_ = CheckState::CHECK_STATE_HEADER; //接下来该处理headers了

    return HttpCode::NO_REQUEST;
}


//解析http请求的一个头部信息
HttpConnection::HttpCode HttpConnection::ParseHeaders(std::string_view text)
{
    if (text[0] == '\0')
    {
        if (content_length_ != 0)
        {
            check_state_ = CheckState::CHECK_STATE_CONTENT;
            return HttpCode::NO_REQUEST;
        }
        return HttpCode::GET_REQUEST;
    }
    else if (text.substr(0,11)=="Connection:")
    {
        text.remove_prefix(11);
        text.remove_prefix(strspn(text.data(), " \t"));
        if (text.substr(0,10)=="keep-alive")
        {
            linger_ = true; //用户想保持连接，那最后要优雅断开连接
        }
    }
    else if (text.substr(15)=="Content-length:")
    {
        text.remove_prefix(15);
        text.remove_prefix(strspn(text.data(), " \t"));

        auto result=std::from_chars(text.data(),text.data()+text.size(),content_length_);
        if(result.ec==std::errc::invalid_argument)
        {
            std::cerr<<"could not convert! from_chars() error!"<<std::endl;
        }
    }
    else if (text.substr(5)=="Host:")
    {
        text.remove_prefix(5);
        text.remove_prefix(strspn(text.data(), " \t"));
        host_ = text;
    }
    else
    {
        std::cerr<<"oop!unknow header: "<<text<<std::endl;
    }

    return HttpCode::NO_REQUEST;
}


HttpConnection::HttpCode HttpConnection::ParseContent(std::string_view text)
{
    if(read_idx_>=content_length_+checked_idx_)
    {
        request_header_=text.substr(0,content_length_);
        return HttpCode::GET_REQUEST;
    }
    return HttpCode::NO_REQUEST;
}


//读入数据后的操作,一行一行地解析
HttpConnection::HttpCode HttpConnection::ProcessRead()
{
    LineStatus line_status = LineStatus::LINE_OK;
    HttpCode ret = HttpCode::NO_REQUEST;
    std::string_view text;

    while ((check_state_ == CheckState::CHECK_STATE_CONTENT && line_status == LineStatus::LINE_OK) || ((line_status = ParseLine()) == LineStatus::LINE_OK))
    {
        text = GetLine();
        start_line_ = checked_idx_;
        std::cout<<text<<std::endl;
        switch (check_state_)
        {
            case CheckState::CHECK_STATE_REQUESTLINE:
            {
                ret = ParseRequestLine(text);
                if (ret == HttpCode::BAD_REQUEST)
                    return HttpCode::BAD_REQUEST;
                break;
            }
            case CheckState::CHECK_STATE_HEADER:
            {
                ret = ParseHeaders(text);
                if (ret == HttpCode::BAD_REQUEST)
                    return HttpCode::BAD_REQUEST;
                else if (ret == HttpCode::GET_REQUEST)
                {
                    return DoRequest();
                }
                break;
            }
            case CheckState::CHECK_STATE_CONTENT:
            {
                ret = ParseContent(text);
                if (ret == HttpCode::GET_REQUEST)
                    return DoRequest();
                line_status = LineStatus::LINE_OPEN;
                break;
            }
            default:
                return HttpCode::INTERNAL_ERROR;
        }
    }
    return HttpCode::NO_REQUEST;
}

HttpConnection::HttpCode HttpConnection::DoRequest()
{
    real_file_=doc_root_;
    int len = doc_root_.size();
    //printf("m_url:%s\n", m_url);
    int last_slash_pos = url_.rfind('/');

    //处理cgi
    if (cgi_ == 1 && (url_[last_slash_pos+1] == '2' || url_[last_slash_pos+1] == '3'))
    {

        //根据标志判断是登录检测还是注册检测
        char flag =url_[1];

        std::string url_real;
        url_real.append("/") ;
        url_real.append(url_.substr(2,-1));
        real_file_.append(url_real);

        //将用户名和密码提取出来
        //user=123&passwd=123
        std::string account_user;
        std::string account_password;
        int and_pos=request_header_.find('&');
        account_user=request_header_.substr(0,and_pos);
        account_password=request_header_.substr(and_pos+1,-1);

        if (url_[last_slash_pos+1] == '3') //注册
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            std::string sql_insert;
            sql_insert="INSERT INTO user(username, passwd) VALUES(";
            sql_insert.append("'");
            sql_insert.append(account_user);
            sql_insert.append("', '");
            sql_insert.append(account_password);
            sql_insert.append("')");

            if (users.find(account_user) == users.end())
            {
                int res;
                {
                std::lock_guard<std::mutex> guard(users_mutex);
                res = mysql_query(mysql_.get(), sql_insert.c_str());
                users.insert(std::pair<std::string, std::string>(account_user, account_password));
                }

                if (!res)
                    url_="/log.html";
                else
                    url_="/registerError.html";
            }
            else
                url_="/registerError.html";
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if (url_[last_slash_pos+1] == '3')
        {
            if (users.find(account_user) != users.end() && users[account_user] == account_password)
                url_="/welcome.html";
            else
                url_="/logError.html";
        }
    }

    if (url_[last_slash_pos+1] == '0')
    {
        std::string url_real;
        url_real="/register.html";
        real_file_.append(url_real);
    }
    else if (url_[last_slash_pos+1] == '1')
    {
        std::string url_real;
        url_real="/log.html";
        real_file_.append(url_real);
    }
    else if (url_[last_slash_pos+1] ==  '5')
    {
        std::string url_real;
        url_real="/picture.html";
        real_file_.append(url_real);
    }
    else if (url_[last_slash_pos+1] ==  '6')
    {
        std::string url_real;
        url_real="/video.html";
        real_file_.append(url_real);
    }
    else if (url_[last_slash_pos+1] ==  '7')
    {
        std::string url_real;
        url_real="/fans.html";
        real_file_.append(url_real);
    }
    else
        real_file_.append(url_);

    if (stat(real_file_.c_str(), &file_stat_) < 0)
        return HttpCode::NO_RESOURCE;

    if (!(file_stat_.st_mode & S_IROTH))
        return HttpCode::FORBIDDEN_REQUEST;

    if (S_ISDIR(file_stat_.st_mode))
        return HttpCode::BAD_REQUEST;


    int fd = open(real_file_.c_str(), O_RDONLY); //fd为即将映射到进程空间的文件描述符
    file_address_ = (char *)mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);//将fd对应的文件从内核空间（硬盘）映射到进程空间，PROT_READ表示进程只能读这块内存
    close(fd);

    return HttpCode::FILE_REQUEST;
}


void HttpConnection::Unmap()
{
    if(!file_address_.empty())
    {
        munmap(file_address_.data(),file_stat_.st_size); //解除内存映射
        file_address_.clear();
    }
}

bool HttpConnection::Write()
{
    int temp = 0;

    //该发的已经发送完了
    if (bytes_to_send_ == 0)
    {
        ModFd(epoll_fd_, sock_fd_, EPOLLIN, trig_mode_);
        Init();
        return true;
    }

    while (1)
    {
        temp = writev(sock_fd_, iovec_, iovec_count_);

        if (temp < 0)
        {
            if (errno == EAGAIN)
            {
                ModFd(epoll_fd_, sock_fd_, EPOLLOUT, trig_mode_);
                return true;
            }
            Unmap();
            return false;
        }

        bytes_have_send_ += temp;
        bytes_to_send_ -= temp;

        if (bytes_have_send_ >= iovec_[0].iov_len)
        {
            iovec_[0].iov_len = 0;
            iovec_[1].iov_base = file_address_.data() + (bytes_have_send_ - write_idx_);
            iovec_[1].iov_len = bytes_to_send_;
        }
        else
        {
            iovec_[0].iov_base = write_buffer_.data() + bytes_have_send_;
            iovec_[0].iov_len = iovec_[0].iov_len - bytes_have_send_;
        }

        if (bytes_to_send_ <= 0)
        {
            Unmap();
            ModFd(epoll_fd_, sock_fd_, EPOLLIN, trig_mode_);

            if (linger_)
            {
                Init();
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}

bool HttpConnection::AddResponse()
{

}

void HttpConnection::Process()
{
    HttpCode read_ret = ProcessRead();
    if (read_ret == HttpCode::NO_REQUEST)
    {
        ModFd(epoll_fd_,sock_fd_, EPOLLIN, trig_mode_);
        return;
    }

    bool write_ret = ProcessWrite(read_ret);
    if (!write_ret)
    {
        CloseConnection();
    }
    ModFd(epoll_fd_, sock_fd_, EPOLLOUT, trig_mode_);
}