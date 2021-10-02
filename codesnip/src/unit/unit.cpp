/**
 * @file unit.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-02
 * @copyright Copyright double_D
 * 
 */

#include "unit.h"

/**
 * @brief ...
 */
bool snip_utils::StringIsDigit(const std::string& str){
	std::stringstream sin(str);
	double d;
	char c;
	if(!(sin >> d)) return false;
	if (sin >> c) return false;
	return true;
}

unsigned long long snip_utils::GetCurrentMillisecs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
}