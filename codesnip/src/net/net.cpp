/**
 * @file net.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */
#include "net.h"
using namespace network_area;
using namespace snip_utils;
static uint64_t auto_increment_value_in_net = 8888;  // 客户端自增1传递服务器，服务器自增1返回到客户端

std::mutex mu_;
std::condition_variable con_;
bool serversetupdone=false;

/**
 * @brief 注释式的IO与非阻塞式的IO
 * socket的默认状态是阻塞
 * 		1. 在阻塞socket上调用read类系统调用时(read,readv,recv,recvfrom,recvmsg)
 * 			read copy 内核读buff中的数据到用户空间的buff中，如果读buff为空，调用进程sleep
 * 			如果非阻塞，调用进程立刻返回一个 errno EWOULDBLOCK=11 means operation should blocked
 * 		2. 在阻塞socket上调用write类系统调用时(write,writev,send,sendto,sendmsg)
 * 			write copy 用户空间buff的数据到内核空间的发送缓冲区，如果发送buff满，则调用进程sleep
 * 			如果非阻塞，调用进程返回 errno EWOULDBLOCK=11 means operation should blocked 以及返回当前能够写入到发送缓冲区中的字节数
 * 		3. 接收外来的连接，即accept，没有新连接到达，则调用进程sleep
 * 			非阻塞返回 errno EWOULDBLOCK
 * 		4. 发起connect
 * 			阻塞，直到连接建立后返回
 * 			非阻塞，先返回一个 errno EINPROGRESS means Operation now in progress
 */

/**
 * @brief Client函数会单独跑子进程，一般客户端都会有一个主线程负责UI，再带一个消息处理线程
 * 客户端采用多线程的方式来模拟并发
 * 客户端的端口会被随机指定一个
 */
void 
network_area::SingleClient()
{
	/**
	 * @brief 说一下字节相关的函数
	 * 大小端的问题
	 * 		intel(小端)机器下执行 htons(16)会得到4096的大端结果 htons htonl ntohs ntohl
	 * 			hton means host to net
	 * 			ntoh means net to host
	 * 			s: short 端口
	 * 			l: long 地址
	 * 字符串操纵函数
	 * 		void bzero(void* dst, size_t nbytes);
	 * 		void memset(void* dst, int c, size_t nbytes);
	 * 		int bcopy(const void* src, void* dst, size_t nbytes); 0表示相同
	 * 		void memcpy(void* src, void* dst, size_t nbytes);
	 * 		void bcmp(const void* ptr1, void* ptr2, size_t nbytes);
	 * 		int memcmp(const void* ptr1, void* ptr2, size_t nbytes); 0表示相同
	 * 网路地址转换函数 ascii to net or net to or presention to numeric or numeric to presention
	 * 		int inet_aton(const char* strptr, struct in_addr* addrptr) 会把一个字符串的地址转为uint32_t
	 * 		in_addr_t inet_addr(const char* strptr);  返回字符串对应的二进制地址uint32_t
	 * 		char* inet_ntoa(struct in_addr inaddr);  返回一个点分十进制的字符串
	 * 
	 * 		int inet_pton(int family, const char* strptr, void* addptr);  表达式转数字
	 * 		const char* inet_ntop(int family, const void* addrptr, char* strptr, size_t len); 数字转字符串表达式
	 */
    std::printf(
        "Client: New client %ld start at %lld\n", 
        gettid(), snip_utils::GetCurrentMillisecs());
	int sockfd;
	// socklen_t servlen;
	// struct sockaddr_in servaddr;  /// 网络socket地址
	struct sockaddr_un servaddr;  /// 本地socket地址
	// sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	sockfd = Socket(AF_LOCAL, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	// servaddr.sin_family = AF_INET;
	// servaddr.sin_port = htons(SERVER_SOCK_PORT);
	// Inet_pton(AF_INET, LOCAL_IP, &servaddr.sin_addr);

	/// 连接服务器，客户端连接成功后会随机启用一个端口，源IP就是自己内核的IP，同步阻塞的方式
    /// 本地的这个确实是没有端口的，本质是一种进程间的通信，尽管写法非常的接近
	Connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
	// Writen(sockfd, (void*)(&auto_increment_value_in_net), sizeof(auto_increment_value_in_net));
    int times=3;
	for(;;){
		sleep(2); // 客户端隔2s发送一次消息
		/// 其实与服务器构成了一个最简单的生产者消费者模型,没有数据可写，会阻塞，但是这里不会阻塞
		Writen(sockfd, (void*)(&auto_increment_value_in_net), sizeof(auto_increment_value_in_net));
		auto_increment_value_in_net++;
        times--;
        if(times<0){break;}
	}
    std::printf("Client: Client %ld quiet\n", gettid());
    Close(sockfd);
}

void
network_area::Client()
{
    // 32个线程模拟并发访问
    int thread_n=8;
    std::thread threads[thread_n];
    for(int i=0;i<thread_n;++i){
        threads[i] = std::thread(SingleClient);
    }
    for(auto &t:threads){t.join();}
    std::printf("Client: All clients quit\n");
    return;
}

/**
 * @brief 并发服务的多进程模型，工作进程的具体功能
 * 接收到客户端的数据流后 decode，再利用提前注册好的消息分发函数进行处理
 * @param  connfd           工作进程的已连接socket
 */
void 
network_area::Doit(int connfd)
{
	int ret;
	uint64_t buff;
	// ret = Readn(connfd, (void*)&buff, sizeof(buff));
	for(;;){
		/// 没有数据可以读，就会阻塞在read上
		ret = Readn(connfd, (void*)&buff, sizeof(buff));
		if(ret<0){exit(-1);}
        std::printf("Server: server get num=%ld and ret=%d\n", buff, ret);
        if(ret==0){break;}  // 读取到的数据量是0，直接GG就可以了，读到0表示连接关闭
	}
}

/**
 * @brief 
 * @return struct init_in_pub* @c 
 */
struct init_in_pub*
network_area::InitInServer()
{
	int listenfd, connfd;
	/// socketadd_in是IPv4的socket地址结构
	struct sockaddr_in servaddr, cliaddr;
	socklen_t servlen, clilen;
	servlen = clilen = sizeof(servaddr);

	listenfd = Socket(AF_INET,SOCK_STREAM,0);  /// 创建 AF_INET 是网络套接字
	/// 初始化 servaddr
	/// memset(&servaddr,0,servlen);
	bzero(&servaddr, servlen);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_SOCK_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  ///服务器可以在任意的网络接口上接收客户连接
	/// sockaddr是通用套接字地址结构

	Bind(listenfd, (sockaddr*)&servaddr, servlen);  /// 绑定

	Listen(listenfd, LISTENQ);  /// 监听固定长度LISTENQ连接队列，凉了就等客户端重传

    bzero(&cliaddr, servlen);
    connfd=-1;
	static struct init_in_pub in_server_pub= {listenfd, connfd, servaddr, cliaddr};
	return &in_server_pub;
}

/**
 * @brief init server, 不同服务器模型其实很多部分还是一致的
 */
struct init_un_pub*
network_area::InitUnServer()
{
	int listenfd, connfd;
	struct sockaddr_un servaddr, cliaddr;

	listenfd = Socket(AF_LOCAL, SOCK_STREAM,0);  /// 创建

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;

	Bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr));  /// 绑定

	Listen(listenfd, LISTENQ);  /// 监听固定长度LISTENQ连接队列，凉了就等客户端重传

    bzero(&cliaddr, sizeof(cliaddr));
    connfd=-1;
	static struct init_un_pub un_server_pub= {listenfd, connfd, servaddr, cliaddr};
	return &un_server_pub;
}

/**
 * @brief 多进程模型的网络并发服务器
 */
void 
network_area::MultiProcessServer()
{
	std::cout << "Multi Process Mode Server" << std::endl;
	std::cout << "Server Process: " << getpid() << std::endl;
	pid_t childpid;
	struct init_un_pub *un = network_area::InitUnServer();
	socklen_t clilen = sizeof(un->servaddr);

	std::cout
        << "main server ready to accept client's connect" << std::endl;
    // /// 客户端与服务器是不同的进程啊，这个不对
    // {
    //     std::lock_guard<std::mutex> lock(mu_);
    //     serversetupdone=true;
    //     con_.notify_all();

    // }

	for(;;){
		/**
		 * @brief 通常父进程调用accept之后调用fork
		 * connfd就会在父子进程间共享，随后父进程关闭connfd，子进程关闭监听fd，多进程模型可以用作并发服务器
         * 如果没有客户端的连接，这里会被阻塞
		 */
		un->connfd = Accept(un->listenfd, (sockaddr*)&un->cliaddr, &clilen);
		if((childpid = Fork())==0){
			std::cout 
			    << "main server fork child " 
			    << getpid() << " to handle connect from " << un->connfd << std::endl;
			Close(un->listenfd);  /// 子进程关闭监听socket
			network_area::Doit(un->connfd);
			// std::cout 
			//     << "In multi process mode, sub server process: " << getpid() 
			//     << " should quit normally, but this process is child process of main server process," 
			//     << " server process never stop, so this process will to be state Z" << std::endl;
			std::printf("Server %ld quit\n", gettid());
            exit(0);  /// 完事后子进程正常退出，短连接
		}
		Close(un->connfd);  ///父进程关闭已连接socket
	}
}

/**
 * @brief 单进程多线程的网络服务器
 * 利用线程池实现
 */
void
network_area::ThreadPools()
{
	std::cout << "Multi Threads with Thread Pool" << std::endl;
	std::cout << "Server Process: " << getpid() << std::endl;
    // 完成创建绑定与监听
	struct init_un_pub *un = network_area::InitUnServer();
	socklen_t clilen = sizeof(un->servaddr);
	std::cout 
        << "main server ready to accept client's connect" << std::endl;
    
    /**
     * @brief 创建线程池
     */
    ThreadPool pool(8);
    // Initialize pool
    pool.Init();
    std::printf("thread pool has finished init\n");

    // accept客户端的连接
    for(;;){
        un->connfd = Accept(un->listenfd, (sockaddr*)&un->cliaddr, &clilen);
        std::printf("a connet is comming\n");
        // 如果执行到这一步说明有客户端的连接，所有的连接都给到线程池中了主线程不会被阻塞
        // 新连接对应的socket就是un->connfd，之前的数据如果写入则之后确实是可以读出来的
        // 但是确实如果数量超过线程池的数量的话是有问题的
        pool.Submit(Doit, un->connfd);
    }
}

/**
 * @brief select并发的网络服务器
 */
void 
network_area::SelectServer()
{
	std::cout << "Seletc Mode Server" << std::endl;
	std::cout << "server process: " << getpid() << std::endl;

    int nready;
    int sockfd;
    ssize_t n;
    int i;
    int maxfd; 
    int maxi = -1;

	struct init_un_pub *un = network_area::InitUnServer();
	socklen_t clilen = sizeof(un->servaddr);
    // 这个监听socket要设置成非阻塞的
    ioctl(un->listenfd, FIONBIO, 1);  //1:非阻塞 0:阻塞

    // 3. 初始化fd_set
    int client[FD_SETSIZE]; // FD_SETSIZE is 256 保存已连接fd 
    for (i = 0; i < FD_SETSIZE; ++i) { client[i] = -1; }

    maxfd = un->listenfd;
    char buf[1024];

    // fd_set代表的是描述符集，本质上是一个整数, 根据操作系统的不同有可能是64位或者32位，通过每位的0或者1来判断fd是否就绪
    // 有新连接到来的时候，这些socket会增加的
    fd_set rset, allset; 
    FD_ZERO(&allset); // 清零
    FD_SET(un->listenfd, &allset); // 把listenfd加入到allset
    printf("Server: select ready\n");

    // int listenfd;
    // int connfd;
    // struct sockaddr_un servaddr;
    // struct sockaddr_un cliaddr;

    for(;;){
        rset = allset;
        // 函数签名int select(int maxfdpl, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout)
        // 返回值表示fd_set里就绪的元素总个数,包括读,写,异常fd_set
        // 第一个参数表示所有文件描述符的最大值
        // 中间仨代表要监听的读set, 写set和异常set
        // 最后一个参数代表select每个fd超时的时间
        // rset即关心socket是否可读
        // nready返回的是就绪的描述符
        // 监听socket确实是就绪的，可以直接读
        nready = Select(maxfd+1, &rset, NULL, NULL, NULL);  // 监听fd可读
        // 只有connect与写操作能够使程序继续执行
        std::printf(
            "Server: pass select, ready num=%d at %lld\n", 
            nready, snip_utils::GetCurrentMillisecs());

        // FD_ISSET 如果fd在set中则为真
		if(FD_ISSET(un->listenfd, &rset))
        {
            /* new client connection */
			clilen = sizeof(un->cliaddr);
            // un->connfd 代表对应新客户端的socket
            // 服务器启动后，首先是阻塞在这里的，即accept，直到第一个connect到来
            // 这个accept如果是阻塞的，则导致无法处理其它已经就绪的描述符，所以得修改成非阻塞的
			un->connfd = Accept(un->listenfd, (sockaddr*)&(un->cliaddr), &clilen);
			std::printf(
                "Server: New client connect in select at %lld\n", snip_utils::GetCurrentMillisecs());
			for (i = 0; i < FD_SETSIZE; i++){
				if (client[i] < 0) {
					client[i] = un->connfd;	/* save descriptor */
					break;
				}
            }

			if (i==FD_SETSIZE) {std::printf("Server: Too many clients");}
            /* add new descriptor to set */
			FD_SET(un->connfd, &allset);
            ioctl(un->connfd, FIONBIO, 1);
            /* for select */
			if (un->connfd > maxfd) {maxfd = un->connfd;}
			/* max index in client[] array */
			if (i > maxi) {maxi = i;} 
            /* no more readable descriptors */
			if (--nready <= 0) {continue;}				
		}

        /* check all clients for data */
		for (i=0; i<=maxi; i++)
        {
            std::printf(
                "Server: in select loop i=%d,fd=%d at %lld\n", 
                i, client[i], snip_utils::GetCurrentMillisecs());
            sockfd = client[i];
			if(sockfd < 0) {continue;}
			if(FD_ISSET(sockfd, &rset)) {
                // std::printf("Server: i=%d,fd=%d is ready\n", i, sockfd);
                // 这个地方的读必须是非阻塞的才可以，之前的两种服务模型都是单独的进程或线程在处理
                // 所以阻塞读是没有问题的，但是这里必须设置为非阻塞的
                // sockfd 是非阻塞的fd
                n = Read(sockfd, buf, 1024);
                // std::printf("Server: i=%d,fd=%d, readbytes=%d\n", i, sockfd, n);
                if(n==0) {
                    // 将数据读取到buffer中
                    std::printf(
                        "Server: Client close at %lld\n", snip_utils::GetCurrentMillisecs());
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else {
                    // buff中有数据
                    // Doit(sockfd);
                    uint64_t* p=reinterpret_cast<uint64_t*>(&buf);
                    std::printf(
                        "Server: server get num=%d, n=%d in select at %lld\n", 
                        static_cast<int>(*p), static_cast<int>(n), snip_utils::GetCurrentMillisecs());
                }

                /* no more readable descriptors */
				if (--nready<=0) {break;}	
			}
            // std::printf("Server: i=%d,fd=%d is not ready\n", i, sockfd);
		}
    }
}

/**
 * @brief poll
 */
void
network_area::PollServer()
{
    // int listenfd;
    // int connfd;
    // struct sockaddr_un servaddr;
    // struct sockaddr_un cliaddr;

	std::cout << "Poll Mode Server" << std::endl;
	std::cout << "server process: " << getpid() << std::endl;

	struct init_un_pub *un = network_area::InitUnServer();
    // 这个监听socket要设置成非阻塞的
    socklen_t clilen=sizeof(un->cliaddr);
    ioctl(un->listenfd, FIONBIO, 1);  //1:非阻塞 0:阻塞

	int i, maxi, sockfd;
	int nready;
	ssize_t n;
	char buf[1024];
	struct pollfd client[OPEN_MAX]; 

	client[0].fd = un->listenfd;
	client[0].events = POLLRDNORM;  // POLLRDNORM 普通数据可读
    // 这个监听描述符只看是否有普通数据可读
    /* -1 indicates available entry */
	for (i=1; i<OPEN_MAX; i++) {client[i].fd = -1;}
    /* max index into client[] array */
	maxi=0;

	for(;;){
		nready = Poll(client, maxi+1, INFTIM);
        /* new client connection */
		if (client[0].revents & POLLRDNORM) 
        {
			clilen = sizeof(un->cliaddr);
			un->connfd = Accept(un->listenfd, (sockaddr*)&(un->cliaddr), &clilen);
			std::printf("Server: new client\n");

            /* save descriptor */
			for (i=1; i<OPEN_MAX; i++){
				if (client[i].fd < 0) {
					client[i].fd = un->connfd;	
					break;
				}
            }

			if (i == OPEN_MAX) {std::printf("Server: too many clients\n");}
			client[i].events = POLLRDNORM;
            ioctl(un->connfd, FIONBIO, 1);
            /* max index in client[] array */
			if (i > maxi){maxi = i;}
            /* no more readable descriptors */
			if (--nready <= 0){continue;}
		}

        /* check all clients for data */
		for (i = 1; i <= maxi; i++) 
        {
            sockfd = client[i].fd;
            if(sockfd<0){continue;}				
			if (client[i].revents & (POLLRDNORM | POLLERR)) 
            {
                n=Read(sockfd, buf, 1024);  // 同样非阻塞的读操作
				if(n<0)
                {
					if(errno==ECONNRESET){
						std::printf("Server: client[%d] aborted connection\n", i);
						Close(sockfd);
						client[i].fd = -1;
					} else {std::printf("Server: read error\n");}
				} else if(n==0) {
					/*connection closed by client */
					std::printf("Server: client[%d] closed connection\n", i);
					Close(sockfd);
					client[i].fd = -1;
				} else {
                    uint64_t* p=reinterpret_cast<uint64_t*>(&buf);
                    std::printf(
                        "Server: server get num=%d, n=%d in select at %lld\n", 
                        static_cast<int>(*p), static_cast<int>(n), snip_utils::GetCurrentMillisecs());
                }
                /* no more readable descriptors */
				if (--nready<=0){break;}
			}
		}
	}
    return;
}

/**
 * @brief 实际需要一个额外的线程来处理分包与粘包
 * @param  fd               desc
 */
void
network_area::DoUseFd(int fd)
{
    int n;
    char buf[1024];
    n=Read(fd, buf, 1024);  // 同样非阻塞的读操作
    uint64_t* p=reinterpret_cast<uint64_t*>(&buf);
    std::printf(
        "Server: server get num=%d, n=%d in select at %lld\n", 
        static_cast<int>(*p), 
        static_cast<int>(n), snip_utils::GetCurrentMillisecs());
    return;
}

/**
 * @brief epoll
 * 直接参考man手册中的内容
 */
void
network_area::EPollServer()
{
    // int listenfd;
    // int connfd;
    // struct sockaddr_un servaddr;
    // struct sockaddr_un cliaddr;

	std::cout << "EPoll Mode Server" << std::endl;
	std::cout << "server process: " << getpid() << std::endl;

	struct init_un_pub *un = network_area::InitUnServer();
    // 这个监听socket要设置成非阻塞的
    socklen_t clilen=sizeof(un->cliaddr);
    ioctl(un->listenfd, FIONBIO, 1);  //1:非阻塞 0:阻塞

    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd;
    int n;

    ThreadPool pool(16);
    // Initialize pool
    pool.Init();
    std::printf("Server: in epoll mode, thread pool has finished init\n");

    // 动态变动大小
    epollfd = epoll_create1(0);
    if(epollfd == -1){
        perror("Failed to create epoll file descriptor\n");
        exit(EXIT_FAILURE);
    }

    ev.data.fd = un->listenfd;
    ev.events = EPOLLIN;  // 表示对应的文件描述符可以读（包括对端SOCKET正常关闭）
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, un->listenfd, &ev) < 0) {
        perror("epoll_ctl EPOLL_CTL_ADD\n");
        exit(EXIT_FAILURE);
    }

    for(;;){
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        // 有就绪的才会被唤醒
        if (nfds == -1) {
            perror("epoll_wait\n");
            exit(EXIT_FAILURE);
        }

        for (n=0; n<nfds; ++n) 
        {
            if (events[n].data.fd == un->listenfd) {
                un->connfd = Accept(un->listenfd, (sockaddr*)&(un->cliaddr), &clilen);
                std::printf("Server: epoll new connect\n");
                if (un->connfd == -1) {
                    perror("accept\n");
                    exit(EXIT_FAILURE);
                }
                ioctl(un->connfd, FIONBIO, 1);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = un->connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, un->connfd, &ev) == -1) {
                    perror("epoll_ctl: conn_sock\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                // int _fd = events[n].data.fd;
                // n=Read(_fd, buf, 1024);  // 同样非阻塞的读操作
                // uint64_t* p=reinterpret_cast<uint64_t*>(&buf);
                // std::printf(
                //     "Server: server get num=%d, n=%d in select at %lld\n", 
                //     static_cast<int>(*p), 
                //     static_cast<int>(n), snip_utils::GetCurrentMillisecs());
                /**
                 * @brief 用消息队列用另一个线程做一样的事
                 * 保证主线程的IO的高效性
                 */
                int _fd = events[n].data.fd;
                pool.Submit(DoUseFd, _fd);
            }
        }
    }

    if(close(epollfd))
    {
        perror("Failed to close epoll file descriptor\n");
        exit(EXIT_FAILURE);
    }
    return;
}

void
network_area::RunNetwork(){
	/**
	 * @brief 一个简单的通信模型，尝试展示一下TCP的建立与close
	 * fork一个子进程出来作为连接的客户端
	 */

	pid_t pid = Fork();
	if(pid==0){
		/// child
        // // 客户端等待服务器起来之后再启动，客户端与服务器是不同的进程啊，这个不对
        // std::unique_lock<std::mutex> lock(mu_);
        // while(!serversetupdone){
        //     con_.wait(lock);
        // }
        /// 先简单sleep一下
        sleep(5);
		network_area::Client();
		// std::cout 
		//     << "Client process: " << getpid() 
		//     << " should quit normally, but client process is child process of server process," 
		//     << "server process never stop, so client process will to be state Z" << std::endl;
		exit(0);
	}
	else{
		/// father
		switch (Snip::GetInstance().server_type){
		case MULTI_P:
			network_area::MultiProcessServer();
			break;
		case SINGLE_P_MULTI_T:
            network_area::ThreadPools();
			break;
		case SELECT:
			network_area::SelectServer();
			break;
		case POLL:
            network_area::PollServer();
			break;
		case EPOLL:
            network_area::EPollServer();
			break;
		default:
			std::cerr 
			<< "net type error" << std::endl;
		}
		/// waitpid(pid, NULL, 0); 网络服务器不停机的，所以不会走到这里来
	}
}

/**
 * @brief 
 */
void 
SysCall::Net(){
	network_area::RunNetwork();
	return;
}