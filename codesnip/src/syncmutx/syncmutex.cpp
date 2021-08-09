/**
 * @file mm.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#include "syncmutex.h"
using namespace sync_mutex_area;
static int auto_increment_value_in_sm = 0;
static pthread_spinlock_t spin_lock;// = PTHREAD_PROCESS_PRIVATE;
static pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

void *SpinlockAdd(void* scale){
	int i = 0;
    uint64_t start, end;
    // std::cout 
    // << "start auto increment with spinlock, data scale is " << *(uint64_t*)scale << std::endl;
    start = clock();
	while(i < *(uint64_t*)scale){
		pthread_spin_lock(&spin_lock);
		i++;
		auto_increment_value_in_sm++;
		pthread_spin_unlock(&spin_lock);
	}
    end=clock();
    std::cout 
    << "finish auto increment with spinlock, data scale is " << *(uint64_t*)scale 
    << " and time is " << end-start << std::endl;
}

void *MutexAdd(void* scale){
	int i = 0;
    uint64_t start, end;
    // std::cout 
    // << "start auto increment with mutex, data scale is " << *(uint64_t*)scale << std::endl;
    start = clock();
	while(i < *(uint64_t*)scale){
		pthread_mutex_lock(&mutex_lock);
		i++;
		auto_increment_value_in_sm++;
		pthread_mutex_unlock(&mutex_lock);
	}
    end=clock();
    std::cout 
    << "finish auto increment with mutex, data scale is " << *(uint64_t*)scale 
    << " and time is " << end-start << std::endl;
}

/**
 * @brief 
 */
void sync_mutex_area::CompareSpinAndMutex(){
    /// 两个锁在定义的时候完成初始化
    pthread_spin_init(&spin_lock, PTHREAD_PROCESS_PRIVATE);
    pthread_t p1, p2;
    uint64_t small_scale = SMALL_CA;
    uint64_t huge_scale = HUGE_CA;

    std::cout << "small scale mutex" << std::endl;
    pthread_create(&p1, NULL, &MutexAdd, (void*)&small_scale);
    pthread_join(p1,NULL);
    std::cout << "small scale mutex" << std::endl;

    std::cout << "small scale spinlock" << std::endl;
    pthread_create(&p2, NULL, &SpinlockAdd, (void*)&small_scale);
    pthread_join(p2,NULL);
    std::cout << "small scale spinlock" << std::endl;

    std::cout << "huge scale mutex" << std::endl;
    pthread_create(&p1, NULL, &MutexAdd, (void*)&huge_scale);
    pthread_join(p1,NULL);
    std::cout << "huge scale mutex" << std::endl;

    std::cout << "huge scale spinlock" << std::endl;
    pthread_create(&p2, NULL, &SpinlockAdd, (void*)&huge_scale);
    pthread_join(p2,NULL);
    std::cout << "huge scale spinlock" << std::endl << std::endl;
    std::cout << "Maybe spinlock is better than mutex" << std::endl;
}

/**
 * @brief 
 */
void 
SysCall::SyncMutex(){
	sync_mutex_area::CompareSpinAndMutex();
	return;
}

