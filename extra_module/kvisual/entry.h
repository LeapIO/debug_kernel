/**
 * @file init.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-06
 * @copyright Copyright double_D
 * 
 */

#ifndef _ENTRY_H
#define _ENTRY_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>

static inline int kvisual_handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    printk(
        KERN_ERR "fault_handler: p->symbol_name=%s p->addr = 0x%p, trap #%dn", 
        p->symbol_name, p->addr, trapnr);
    return 0;
}

static inline bool kvisual_supported_arch(void)
{
#if !defined(CONFIG_X86) || !defined(CONFIG_X86_64)
	printk(KERN_INFO "Only x86 is supported!\n");
	return false; /* TODO */
#endif
    return true;
}
#endif