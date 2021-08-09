/**
 * @file wrapsocket.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */
#include "syscallwrap.h"

/* begin process */
/**
 * @brief 
 * @return int @c 
 */
int
Fork(){
    pid_t childpid = fork();
    if(childpid < 0){
        perror("fork error!");
        exit(errno);  // 进程异常退出 errno是每一个task都有的一个全局变量
    }
    return childpid;
}
/* end process */

/* begin socket */
/**
 * @brief 
 * @param  family           协议族 IPv4(AF_INET or PF_INET)或IPv6(PF_INET6)等等
 * @param  type             字节流或报文 SOCK_STREAM (TCP) SOCK_DGRAM (UDP)
 * @param  protocol         一般指定为0，选择与type对应的协议
 * @return int @c           0 means sucess
 */
int
Socket(int family, int type, int protocol){
    /**
     * 如果调用listen成为一个监听socket，那么一个服务器进程中只需要有一个就可以了
     * accept成功之后内核会返回一个全新的socket,称为已连接socket，用来与客户端socket通信，这个是真的一对
     * 所以监听socket存在已连接队列以及未完成连接的队列
     */
    int n = socket(family, type, protocol);
    if(n < 0){
        perror("socket init error!");
        exit(0);
    }
    return n;
}

/**
 * @brief 
 * @param  sockfd           待绑定的socketfd，其实就是给socket赋值
 * @param  myaddr           p+prot的struct,需要自己创建并初始化
 * @param  addrlen          sockaddr_in结构体的长度
 */
void
Bind(int sockfd, const struct sockaddr* myaddr, socklen_t addrlen){
    if(bind(sockfd, myaddr, addrlen)<0){
        perror("bind error!");
        exit(errno);
    }
}

/**
 * @brief  socket创建的是主动的socket，调用listen转为监听socket。socket状态由closed变为listen
 * @param  sockfd           socketfd
 * @param  backlog          已完成连接与未完成连接的的连接总数
 */
void
Listen(int sockfd, int backlog){
    if(listen(sockfd, backlog)<0){
        perror("listen error!");
        exit(errno);
    }
}

/**
 * @brief 取已完成队列中的第一个，返回一个已连接的socket
 * @param  sockfd           监听socket fd
 * @param  cliaddr          服务端创建好用于接收客户端协议地址，这些数据来自于TCP三次握手的第三个ACK
 * @param  addrlen          服务端创建好用于接收客户端协议地址的长度
 * @return int @c           已连接socketfd, 如果为空，进程阻塞sleep，让出CPU
 */
int
Accept(int sockfd, struct sockaddr* cliaddr, socklen_t* addrlen){
    /**
     * @brief accept可能会被可中断的系统调用干掉，所以这里需要优化一下
     */
restart:
    int clisockfd = accept(sockfd, cliaddr, addrlen);
    if(clisockfd < 0){
        if(errno==EINTR){goto restart;}
        perror("accept error!");
        exit(errno);
    }
    return clisockfd;
}

/**
 * @brief 客户端一套就是创建+连接，不必调用bind函数，内核会确定源IP地址并选择一个临时端口
 * 激发三次握手的过程
 * @param  sockfd           客户端创建的socketfd
 * @param  servaddr         目标服务器的ip+端口
 * @param  addrlen          目标服务器地址的长度
 */
void
Connect(int sockfd, const struct sockaddr* servaddr, socklen_t addrlen){
    if(connect(sockfd, servaddr, addrlen)<0){
        perror("connect error!");
        exit(errno);
    }
}
/* end socket */

/* begin unit */
/**
 * @brief Close the file descriptor FD
 * @param  fd               desc
 */
void Close(int fd){
    if(close(fd)<0){
        perror("close error!");
        exit(errno);
    }
}
/* end unit */

/* begin IO */
/**
 * @brief Write N bytes of BUF to FD.  Return the number written, or -1
 * @param  fd               desc
 * @param  ptr              desc
 * @param  nbytes           desc
 */
void
Write(int fd, void *ptr, size_t nbytes){
    if(write(fd, ptr, nbytes)!=nbytes){
        perror("write error!");
        exit(errno);
    }
}
/**
 * @brief Read NBYTES into BUF from FD.  Return the number read, -1 for errors or 0 for EOF
 * @return ssize_t @c 
 */
ssize_t
Read(int fd, void *ptr, size_t nbytes){
    size_t n;
    n=read(fd, ptr, nbytes);
    if(n==-1){
        perror("read erro");
        exit(errno);
    }
    return n;
}

/**
 * @brief 防止read系统调用被中断，执行手动重启
 * @param  fd               desc
 * @param  vptr             desc
 * @param  n                desc
 * @return ssize_t @c 
 */
ssize_t
readn(int fd, void *vptr, size_t n){
    size_t nleft;
    ssize_t nread;
    char *ptr = (char*)vptr;

    nleft = n;
    while(nleft>0){
        nread=read(fd, ptr, nleft);  /// 返回已读取的自己数
        if(nread<0){
            if(errno==EINTR){nread=0;} /// call read again
            else{
                return -1;
            }
        }else if(nread==0){
            /// EOF
            break;
        }
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

/**
 * @brief 
 * @param  fd               desc
 * @param  ptr              desc
 * @param  nbytes           desc
 * @return ssize_t @c 
 */
ssize_t
Readn(int fd, void *ptr, size_t nbytes){
    ssize_t ret;
    ret = readn(fd, ptr, nbytes);
    if(ret < 0){
        perror("read error");
        exit(ret);
    }
    return ret;
}

/**
 * @brief 防止写操作的系统调用被信号中断，执行手动重启
 * @param  fd               desc
 * @param  vptr             desc
 * @param  n                desc
 * @return ssize_t @c 
 */

ssize_t
writen(int fd, void *vptr, ssize_t n){
    size_t nleft;
    ssize_t nwritten;

    const char *ptr;

    nleft = n;
    ptr = (char*)vptr;
    while(nleft){
        nwritten = write(fd, ptr, nleft);
        if(nwritten<0){
            if(errno==EINTR){nwritten=0;}
            else{return -1;}
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n - nleft;
}

/**
 * @brief 
 * @param  fd               desc
 * @param  ptr              desc
 * @param  nbytes           desc
 */
void
Writen(int fd, void *ptr, size_t nbytes){
    if(writen(fd, ptr, nbytes)!=nbytes){
        perror("write error");
        exit(-1);
    }
}
/* end IO */

/* begin IPC */
/**
 * @brief Send signal SIG to process number PID
 * @param  signo            信号
 * @param  target_pid       目标task
 */
void Kill(pid_t target_pid, int signo){
    int ret;
    ret = kill(target_pid, signo);
    if(ret<0){
        perror("kill error");
        exit(errno);
    }
}

/**
 * @brief 
 * @return __sighandler_t @c 
 */
__sighandler_t Signal(int signo, __sighandler_t handler){
    if(signal(signo, handler) == SIG_ERR){
        perror("signal error");
        exit(errno);
    }
}
/* end IPC */