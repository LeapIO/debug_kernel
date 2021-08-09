/**
 * @file fs.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */
#include "ipc.h"

static int sig_cnt[NSIG];  // NSIG is 65
static volatile sig_atomic_t get_SIGINT = 0;

void UserSignalHandler(int signo){
    if(signo == SIGINT){get_SIGINT = 1;}
    sig_cnt[signo]++;
}

/**
 * @brief 子进程，信号的接收者
 */
void SignalReceiver(){
    /// 子进程
    std::cout << "singal receievr pid: " << getpid() <<  std::endl;
    int i = 0;
    /**
     * @brief 除了信号 SIGKILL与SIGSTOP，其它信号均捕获
     */
    for(int i=1; i<NSIG;i++){
        if(i==SIGKILL || i==SIGSTOP || i==NO_SIGNAL_1 || i==NO_SIGNAL_2){continue;}
        // std::cout << i << std::endl;
        Signal(i, &UserSignalHandler);
    }
    std::cout << "singal receievr finish set user signal handler" << std::endl;

    while(!get_SIGINT){continue;}
    for(i=0; i<NSIG; i++){
        if(sig_cnt[i]!=0){
            std::cout 
            << "signal: " << i << " be caught " << sig_cnt[i] << " times!" << std::endl;
        }
    }
    return;
}

/**
 * @brief 父进程，信号的发送者
 * @param  recieve_pid      desc
 */
void SignalSender(pid_t recieve_pid){
    std::cout << "signal sender pid: " << getpid() << std::endl;
    sleep(2);  /// 直接发子进程观察不到
    /// 父进程给子进程发送信号 SIGINT
    uint8_t times = 1;
    for(int i=0;i<times;++i){
        std::cout << "signal sender sends SIGINT" << std::endl;
        Kill(recieve_pid, SIGINT);  // 给子进程发送ctrl+c信号
    }
    waitpid(recieve_pid, NULL, 0);  /// 父进程等待子进程退出
    return;
}

/**
 * @brief
 */
void ipc_area::LookSignal(){
    pid_t pid = Fork();
    if(pid==0){SignalReceiver();}
    else {SignalSender(pid);}
}

/**
 * @brief 
 */
void ipc_area::LookPipe(){;}

/**
 * @brief 
 */
void ipc_area::LookNamedPipe(){;}

/**
 * @brief 
 */
void ipc_area::LookMsgQueue(){;}

/**
 * @brief 
 */
void ipc_area::LookSemaphore(){;}

/**
 * @brief 
 */
void ipc_area::LookSharedMemory(){;}

/**
 * @brief 
 */
void ipc_area::LookSockets(){;}

/**
 * @brief 
 */
void
SysCall::IPC(){
    switch(Snip::GetInstance().ipc_type){
    case PIPE:
        ipc_area::LookPipe();
        break;
    case NAMED_PIPE:
        ipc_area::LookNamedPipe();
        break;
    case SIGNAL:
        ipc_area::LookSignal();
        break;
    case MSG_QUEUE:
        ipc_area::LookMsgQueue();
        break;
    case SEMAPHORE:
        ipc_area::LookSemaphore();
        break;
    case SHARED_MEMORY:
        ipc_area::LookSharedMemory();
        break;
    case SOCKETS:
        ipc_area::LookSockets();
        break;
    default:
        std::cerr 
        << "ipc type error" << std::endl;
        break;
    }
	return;
}