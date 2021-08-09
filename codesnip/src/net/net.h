/**
 * @file net.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#ifndef _NET_H
#define _NET_H
#include <arpa/inet.h>
#include "syscallwrap.h"
#include "snip.h"

#define SERVER_SOCK_PORT 6666  // 一个随机的网络端口号
#define LISTENQ 8  // TCP监听队列的长度上限为8，超过上限,服务器主动抛弃，且等待客户端的重传
#define LOCAL_IP "127.0.0.1"
#define AUTO_INCREMENT_UP_LIMIT 1 << 48

struct init_in_pub{
    int listenfd;
    int connfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
};

struct init_un_pub{
    int listenfd;
    int connfd;
    struct sockaddr_un servaddr;
    struct sockaddr_un cliaddr;
};

/**
 * @brief 数字到字符串表达式
 * @param  family           desc
 * @param  addrptr          desc
 * @param  strptr           desc
 * @param  len              desc
 * @return const char* @c 
 */
static inline const char*
Inet_ntop(int family, const void* addrptr, char* strptr, size_t nbytes){
    const char* ptr;
    if(strptr == NULL){
        std::cerr 
        << "NULL 3rd argument to inet_ntop" << std::endl;
        exit(-1);
    }
    ptr = inet_ntop(family, addrptr, strptr, nbytes);
    if(ptr == NULL){
        std::cerr 
        << "inet_ntop error" << std::endl;
        exit(-1);
    }
    return(ptr);
}

/**
 * @brief 
 * @param  family           desc
 * @param  strptr           desc
 * @param  addrptr          desc
 */
static inline void
Inet_pton(int family, const char* strptr, void* addrptr){
    int ret;  /// inet_pton 成功返回1
    ret = inet_pton(family, strptr, addrptr);
    if(ret<0){
        std::cerr 
        << "inet_pton error" << strptr << std::endl;
        exit(-1);
    }
    else if(ret==0){
        std::cerr 
        << "inet_pton error, format error" << strptr << std::endl;
        exit(-1);
    }
}

namespace network_area{
    struct init_un_pub* InitUnServer();  // 本地
    struct init_in_pub* InitInServer();  // 网络
    void Client();
    void RunNetwork();
    void Doit(int);
    void MultiProcessServer();
    void SelectServer();
}
#endif
