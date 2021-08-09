/**
 * @file example.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-02-28
 * @copyright Copyright double_D
 * 
 */
#ifndef _EXAMPLE_H
#define _EXAMPLE_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <iostream>

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))
#endif
