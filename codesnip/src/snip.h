/**
 * @file snip.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-02-28
 * @copyright Copyright double_D
 * 
 */
#ifndef _SNIP_H
#define _SNIP_H

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <iostream>
#include <vector>
#include <map>

#define ARG_NUM 2

/**
 * @brief c++ 单例的实现
 * 私有化它的构造函数，以防止外界创建单例类的对象
 * 使用一个公有的静态方法获取该实例
 * 
 * 类或类的引用用 .
 * 类的指针用 ->
 * 
 * 引用被创建的时候必须初始化，引用就是一个别名，不是指针也不是拷贝
 * 指针初始化的时候无需attach，但是必须初始化，至少要初始化为null
 * 
 * 静态成员变量，不被算入sizeof，可以通过 类名::成员名 的方式访问，本质是全局变量
 */

typedef void(*SnipFuncPtr)();  /// 为无参数，且返回 None 的函数起了一个别名 SnipFuncPtr
typedef enum {SERVSE_T, BLOCK_IO_T, IPC_T, FS_T} input_t;

/**
 * @brief 核心内容全部在这里，涵盖绝大多数的系统调用
 */
class SysCall {
public:
	static void LWP();  /// 轻量级进程
	static void Net();  ///网络相关
	static void IPC();  // 信号相关
	static void SyncMutex();  // 互斥与同步
	static void FS();  // 文件系统相关的内容
	/// ...
};

/**
 * @brief Snip
 */
class Snip {
public:
	/// 名字应该去对应函数指针
	std::map<std::string, SnipFuncPtr> snip_to_func;

	uint8_t server_type;
	uint8_t blockio_type;
	uint8_t ipc_type;
	uint8_t fsapi_type;

	// static std::map<std::string, std::map<int, input_t>> para_num;
	// para_num["servert="][8] = SERVSE_T;
	std::map<std::string, input_t> para_num;
	/// 用户输入字符解析
	/// 静态函数是全局函数，不能调用非静态的成员变量与成员函数
	static Snip& GetInstance() {
		/**
		 * @brief
		 * 首先 static 申明的局部变量，整个进程只有一份，根据是否初始化保存在 .BSS 或 .TEXE
		 * 只有在第一次执行到申明处被初始化，以后即使有 初始化语句 也不再初始化
		 * 
		 * 此外 Magic Static 特性
		 * 	If control enters the declaration concurrently,
		 *	while the variable is being initialized,
		 * 	the concurrent execution shall wait for completion of the initialization.
		 * 所以是线程安全的，由于是static，也无需清理 delete
		 */
		static Snip snip(0,1,2,0);  /// 定义了一个类的实例，构造函数执行初始化，即使调用多次，也仅仅执行一次
		return snip;
	}
	void ShowUsage();
	void ParaArgs(int argc, char *argv[]);
private:
	/// 不允许外部创建
	Snip(
		uint8_t _server_type, 
		uint8_t _blockio_type, 
		uint8_t _ipc_type,
		uint8_t _fsapi_type
	):
		server_type(_server_type), // 默认多进程的服务器类型
		blockio_type(_blockio_type), // 默认阻塞IO
		ipc_type(_ipc_type), // 默认信号的方式通信  
		fsapi_type(_fsapi_type)  // 默认使用inotify
	{
		/**
		 * @brief 用户输入参数的第一层次
		 */
		snip_to_func.insert(
			std::make_pair<std::string, SnipFuncPtr>("lwp", &SysCall::LWP));
		snip_to_func.insert(
			std::make_pair<std::string, SnipFuncPtr>("net", &SysCall::Net));
		snip_to_func.insert(
			std::make_pair<std::string, SnipFuncPtr>("ipc", &SysCall::IPC));
		snip_to_func.insert(
			std::make_pair<std::string, SnipFuncPtr>("sm", &SysCall::SyncMutex));  // sync and mutex
		snip_to_func.insert(
			std::make_pair<std::string, SnipFuncPtr>("fs", &SysCall::FS));  // fs
		// snip_to_func.insert(
		// 	std::make_pair<std::string, SnipFuncPtr>("ta", &SysCall::TestCase));  // testcase

		/**
		 * @brief
		 */
		para_num["servert="] = SERVSE_T;
		para_num["blockiot="] = BLOCK_IO_T;
		para_num["ipct="] = IPC_T;
		para_num["fsapit="] = FS_T;
	}
};

/**
 * @brief 
 */
class Output {
public:
	std::map<int, std::string> to_servert;
	std::map<int, std::string> to_io;
	std::map<int, std::string> to_ipc;
	std::map<int, std::string> to_fsapi;
	/// 静态函数是全局函数，不能调用非静态的成员变量与成员函数
	static Output& GetInstance() {
		static Output output;  /// 定义了一个类的实例，构造函数执行初始化，即使调用多次，也仅仅执行一次
		return output;
	}
private:
	Output(){
		/**
		 * @brief 
		 */
		to_servert[0] = "multi processes";
		to_servert[1] = "single process multi threads";
		to_servert[2] = "select";
		to_servert[3] = "poll";
		to_servert[4] = "epoll";
		/**
		 * @brief 
		 */
		to_io[0] = "no blocking";
		to_io[1] = "blocking";
		/**
		 * @brief 
		 */
		to_ipc[0] = "pipe";
		to_ipc[1] = "named pipe";
		to_ipc[2] = "semaphore";
		to_ipc[3] = "signal";
		to_ipc[4] = "msg queue";
		to_ipc[5] = "shared memory";
		to_ipc[6] = "sockets";
		/**
		 * @brief 
		 */
		to_fsapi[0] = "inotify";
	}
};

void ErrUse(Snip&);
int AdjustArgs(int, char *[], int, unsigned);
int ProcessOptArgs(int, char *[]);

/**
 * @brief 服务器类型 多进程，单进程多线程，select，以及poll/epoll
 */
typedef enum {MULTI_P, SINGLE_P_MULTI_T, SELECT,POLL,EPOLL} server_t;
/**
 * @brief IO模式
 */
typedef enum {NO_BLOCKING, BLOCKING} io_t;
/**
 * @brief IPC类型 管道，命名管道，信号，消息队列，信号量，共享内存，socket
 */
typedef enum {PIPE, NAMED_PIPE, SIGNAL, MSG_QUEUE, SEMAPHORE, SHARED_MEMORY, SOCKETS} ipc_t;
/**
 * @brief 所测试文件系统的api类型
 */
typedef enum {INOTIFY,} fsapi_t;
#endif