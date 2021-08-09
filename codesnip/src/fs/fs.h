/**
 * @file fs.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#ifndef _FS_H
#define _FS_H
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/inotify.h>
#include "snip.h"

struct pthread_para{
    int flag;
    const char *path;
};

namespace fs_area {
    void RunFs();
    void Inotify();
}
#endif
