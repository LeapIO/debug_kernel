/**
 * @file codesnip.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-02-28
 * @copyright Copyright double_D
 * 
 */

#include <iostream>
#include "snip.h"

int 
main(int argc, char* argv[])
{
	Snip& snip = Snip::GetInstance();  /// 静态函数通过 类名::函数名的方式访问
	snip.ParaArgs(argc, argv);
	return 0;
}