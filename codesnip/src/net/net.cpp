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
static uint64_t auto_increment_value_in_net = 8888;  // 客户端自增1传递服务器，服务器自增1返回到客户端

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
 */
void 
network_area::Client(){
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
	int sockfd;
	socklen_t servlen;
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
	Connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
	// Writen(sockfd, (void*)(&auto_increment_value_in_net), sizeof(auto_increment_value_in_net));
	for(;;){
		sleep(2); // 客户端隔2s发送一次消息
		/// 其实与服务器构成了一个最简单的生产者消费者模型,没有数据可写，会阻塞，但是这里不会阻塞
		Writen(sockfd, (void*)(&auto_increment_value_in_net), sizeof(auto_increment_value_in_net));
		auto_increment_value_in_net++;
	}
	Close(sockfd);
}
/**
 * @brief 并发服务的多进程模型，工作进程的具体功能
 * 接收到客户端的数据流后 decode，再利用提前注册好的消息分发函数进行处理
 * @param  connfd           工作进程的已连接socket
 */
void 
network_area::Doit(int connfd){
	int ret;
	uint64_t buff;
	// ret = Readn(connfd, (void*)&buff, sizeof(buff));
	for(;;){
		/// 没有数据可以读，就会阻塞在read上
		ret = Readn(connfd, (void*)&buff, sizeof(buff));
		if(ret<0){exit(-1);}
		std::cout << "server get num " << buff << " from client" << std::endl;
	}
}

/**
 * @brief 
 * @return struct init_in_pub* @c 
 */
struct init_in_pub*
network_area::InitInServer(){
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
	static struct init_in_pub in_server_pub= {listenfd, connfd, servaddr, cliaddr};
	return &in_server_pub;
}

/**
 * @brief init server, 不同服务器模型其实很多部分还是一致的
 */
struct init_un_pub*
network_area::InitUnServer(){
	int listenfd, connfd;
	struct sockaddr_un servaddr, cliaddr;

	listenfd = Socket(AF_LOCAL, SOCK_STREAM,0);  /// 创建

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;

	Bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr));  /// 绑定

	Listen(listenfd, LISTENQ);  /// 监听固定长度LISTENQ连接队列，凉了就等客户端重传
	static struct init_un_pub un_server_pub= {listenfd, connfd, servaddr, cliaddr};
	return &un_server_pub;
}

/**
 * @brief 多进程模型的网络并发服务器
 */
void network_area::MultiProcessServer(){
	std::cout << "Multi Process Mode Server" << std::endl;
	std::cout << "server process: " << getpid() << std::endl;
	pid_t childpid;
	struct init_un_pub *un = network_area::InitUnServer();
	socklen_t clilen = sizeof(un->servaddr);

	std::cout << "main server ready to accept client's connect" << std::endl;
	for(;;){
		/**
		 * @brief 通常父进程调用accept之后调用fork
		 * connfd就会在父子进程间共享，随后父进程关闭connfd，子进程关闭监听fd，多进程模型可以用作并发服务器
		 */
		un->connfd = Accept(un->listenfd, (sockaddr*)&un->cliaddr, &clilen);  /// 如果没有客户端的连接，这里会被阻塞
		if((childpid = Fork())==0){
			std::cout 
			<< "main server fork child " 
			<< getpid() << " to handle connect from " << un->connfd << std::endl;
			Close(un->listenfd);  /// 子进程关闭监听socket
			network_area::Doit(un->connfd);
			std::cout 
			<< "In multi process mode, sub server process: " << getpid() 
			<< " should quit normally, but this process is child process of main server process," 
			<< " server process never stop, so this process will to be state Z" << std::endl;
			exit(0);  /// 完事后子进程正常退出，短连接
		}
		Close(un->connfd);  ///父进程关闭已连接socket
	}
}

/**
 * @brief select并发的网络服务器
 */
void network_area::SelectServer(){
	std::cout << "Seletc Mode Server" << std::endl;
	std::cout << "server process: " << getpid() << std::endl;

	pid_t childpid;
	struct init_un_pub *un = network_area::InitUnServer();
	socklen_t clilen = sizeof(un->servaddr);
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
		std::cout << "client process: " << getpid() << std::endl;
		network_area::Client();
		std::cout 
		<< "Client process: " << getpid() 
		<< " should quit normally, but client process is child process of server process," 
		<< "server process never stop, so client process will to be state Z" << std::endl;
		exit(0);
	}
	else{
		/// father
		switch (Snip::GetInstance().server_type){
		case MULTI_P:
			network_area::MultiProcessServer();
			break;
		case SINGLE_P_MULTI_T:
			break;
		case SELECT:
			network_area::SelectServer();
			break;
		case POLL:
			break;
		case EPOLL:
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