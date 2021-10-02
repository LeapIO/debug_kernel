/**
 * @file unit.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#ifndef _UNIT_H
#define _UNIT_H
#include "snip.h"
#include <sstream>

#define USER_BUFF_SIZE 4096

namespace snip_utils {
    bool StringIsDigit(const std::string&);
    class Iobuff{
    public:
        char buff[USER_BUFF_SIZE];
        ssize_t p_off;  // 当前buff操作的offset
        Iobuff(){
            p_off = 0;
            bzero(buff, USER_BUFF_SIZE);
        }
    };
    unsigned long long GetCurrentMillisecs();
}
#endif