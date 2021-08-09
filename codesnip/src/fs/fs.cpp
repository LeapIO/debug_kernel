/**
 * @file fs.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */
#include "fs.h"
using namespace fs_area;

/**
 * @brief 利用inotify来监听文件系统的事件
 * @param  arg              desc
 * @return void* @c 
 */
void*
DataProcessThread(void *arg)
{
    struct pthread_para *para = (struct pthread_para *)arg;
    const char *pwd = para->path;

    int fd, poll_num, wd;
    nfds_t nfds;
    struct pollfd fds[1];

    /* Create the file descriptor for accessing the inotify API. */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    /* Mark directories for events when file was created */
    wd = inotify_add_watch(fd, pwd, IN_CREATE);
    if (wd < 0) {
        fprintf(stderr, "Cannot watch '%s': %s\n", pwd, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Prepare for polling. */
    nfds = 1;

    fds[0].fd = fd;                 /* Inotify input */
    fds[0].events = POLLIN;

    /* Wait for events and/or terminal input. */
    std::cout << "Listening for events" << std::endl;
    while (1) {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1) {
            if (errno == EINTR)
                continue;
            perror("poll");
            exit(EXIT_FAILURE);
        }

        /* Inotify events are available. */
        if (poll_num > 0 && (fds[0].revents & POLLIN)) {
            // HandleEvents(fd);   
        }
    }

    std::cout << "Listening for events stopped" << std::endl;
    close(fd);  /* Close inotify file descriptor. */
    exit(EXIT_SUCCESS);
}

/**
 * @brief 主线程loop
    开一个work thread 监听目标文件夹下的变化，读取最新的数据处理
 */
void
fs_area::Inotify(){
    int err;
    pthread_t p;

    /* pthread传多个参数需要结构体，否则直接取参数地址即可 */
    struct pthread_para para;

    std::string pwd = get_current_dir_name();
    pwd += "/inotifytarget";

    para.flag = 1;
    para.path = pwd.c_str();

recreateinotify:
    if(mkdir(pwd.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)<0){
        if(errno==EEXIST){
            if(remove(pwd.c_str())<0){
                perror("remove exit inotifytarget failure!");
                exit(errno);
            }
            goto recreateinotify;
        }else{
            perror("mkdir inotifytarget failure!");
            exit(errno);
        } 
    }

    if(pwd.back()!='/')
        pwd += '/';

    if(access(pwd.c_str(), F_OK | R_OK) < 0){
        std::cout << "target path " << pwd << " can not be access!" << std::endl;
        exit(-1);
    }

    /* &DataProcessThread is same with DataProcessThread */
    err = pthread_create(&p, NULL, &DataProcessThread, &para);
    if(err<0){
        perror("pthread_2 error!");
        exit(errno);
    }

    std::cout << "Main thread wait the end of data process thread!" << std::endl;
    pthread_join(p, NULL);
    return;
}

void
fs_area::RunFs(){
    switch (Snip::GetInstance().fsapi_type){
    case INOTIFY:
        fs_area::Inotify();
        break;
    default:
        std::cerr
        << "net type error" << std::endl;
    }
}

/**
 * @brief 
 */
void
SysCall::FS(){
    fs_area::RunFs();
	return;
}