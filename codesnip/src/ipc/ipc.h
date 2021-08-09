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
#include <signal.h>
#include "snip.h"
#include "syscallwrap.h"

#define NO_SIGNAL_1 32
#define NO_SIGNAL_2 33

namespace ipc_area{
    void LookPipe();
    void LookNamedPipe();
    void LookSignal();
    void LookMsgQueue();
    void LookSemaphore();
    void LookSharedMemory();
    void LookSockets();
}
void SignalReceiver();
void SignalSender(pid_t);
void UserSignalHandler(int);
#endif