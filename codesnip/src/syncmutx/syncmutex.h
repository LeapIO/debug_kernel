/**
 * @file mm.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#ifndef _MM_H
#define _MM_H
#include "snip.h"
#include "common/rwlock.h"

#define SMALL_CA 1000  // 小规模临界区代码行数
#define HUGE_CA 100000000  // 大规模临界区代码行数

namespace sync_mutex_area{
    void CompareSpinAndMutex();
}
#endif
