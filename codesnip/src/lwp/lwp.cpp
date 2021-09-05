/**
 * @file pm.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */
#include "lwp.h"

int global_var = 100;
static unsigned long add_var = 0;

namespace process_thread_area 
{
void*
ThreadFunc(void *arg)
{
    std::cout 
    << "curr pid is " << getpid() 
    << " curr tid is " << gettid() 
    << " from syscall(__NR_gettid)" << std::endl 
    << "get tid from std::this_thread::get_id() is " 
    << std::this_thread::get_id() << std::endl << std::endl;

    int j;
    for(j=0; j<1000; ++j){
        add_var++;
    }
    std::cout << gettid() << " finish loop" << std::endl;
    return NULL;
}

void 
CreateThread(){
    int err;
    pthread_t p1, p2;
    err = pthread_create(&p1, NULL, ThreadFunc, NULL);
    if(err!=0){
        // snip_utils::EncounterError();
        std::cout<<"error creation for p1!" << std::endl;
    }
    err = pthread_create(&p2, NULL, ThreadFunc, NULL);
    if(err!=0){
        // snip_utils::EncounterError();
        std::cout<<"error creation for p2!" << std::endl;
    }

    std::cout 
    << "child has created two threads, if child threads do not call join to wait main thread"  << std::endl 
    << "call join block here" << std::endl << std::endl;
    pthread_join(p1, NULL); /// 主线称等待子线程结束后才结束
    pthread_join(p2, NULL);
    return;
}

void 
ForkProAndCreateT(){
    pid_t pid = fork();
    if(pid==-1){
        // snip_utils::EncounterError();
    }
    if(pid==0){
        global_var = 200;
        std::cout 
        << "i am child " << getpid() 
        << " i will sleep 1 seconds " << std::endl
        << "child process changes a glbal var from 100 to 200, chekc it the value is " << global_var 
        << std::endl << std::endl << std::endl;
        sleep(1); /// 子进程睡眠 5s
        CreateThread();  /// 子进程创建2个线程
        std::cout << std::endl
        << "the final add_var is " << add_var 
        << " the value is not identical because the thread condition!" << std::endl;
        /// 子进程再fork一个新的子进程
        pid_t pid_again = fork();
        if(pid_again<0){
            // snip_utils::EncounterError();
        }
        if(pid_again==0){
            const char* pathname = "/bin/ls";
            char* const argv[] = {(char*)"ls", (char*)"-al", (char*)"/", NULL};
            char* const envp[ ]={(char*)"PATH=/bin", NULL};
            int result = execve(pathname, argv, envp);
            if(result==-1){
                // snip_utils::EncounterError();
            }
        }
        else{
            waitpid(pid, NULL, 0);  /// 父进程等待子进程退出
        }
    }
    else if(pid > 0){
        std::cout 
        << "i am father " << getpid() 
        << " i am waiting for child exit" << std::endl;
        waitpid(pid, NULL, 0); /// 父进程阻塞直到子进程退出
        /// waitpid(pid, NULL, WNOHANG); 父进程不阻塞，子进程的父进程变为1号进程
        
        std::cout << std::endl 
        << "father will exit and the global var is " << global_var << std::endl;
        
        std::cout 
        << "if call waitpid like waitpid(pid, NULL, WNOHANG)"
        << " father will not be blocked "
        << " and then, the child's father changes to 1 process /sbin/init" << std::endl;
    }
    return;
}

int
RunThreadPool()
{
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    return 0;
}
}

/**
 * @brief 
 */
void 
SysCall::LWP(){
	process_thread_area::ForkProAndCreateT();
    process_thread_area::RunThreadPool();
	return;
}