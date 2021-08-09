/**
 * @file wrapsock.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#ifndef _WRAPSOCK_H
#define _WRAPSOCK_H

/**
 * @file wrapsocket.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>
#include "snip.h"

#define USER_BUFF_MAX_LEN 4096

#define gettid() syscall(__NR_gettid)

/* begin process */
int Fork();
/* end process */
/* begin socket */
int Socket(int, int, int);
int Accept(int, struct sockaddr*, socklen_t*);
void Listen(int, int);
void Connect(int, const struct sockaddr*, socklen_t);
void Bind(int, const struct sockaddr*, socklen_t);
/* end socket */
/* begin IO */
ssize_t Readn(int, void *, size_t);
void Writen(int, void *, size_t);
/* end IO */
/* begin IPC */
void Kill(pid_t, int);
__sighandler_t Signal(int, __sighandler_t);
/* end IPC */
/* begin unit */
void Close(int);
/* end unit */
#endif
