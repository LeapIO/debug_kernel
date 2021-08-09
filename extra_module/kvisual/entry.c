/**
 * @file k_visual.c
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-03-06
 * @copyright Copyright double_D
 * kernel visual
 */
#include "entry.h"
#include "iovisual.h"


/**
 * iopath_kretps includes:
 *		do_sys_openat2
 *		ext4_file_write_iter
 *		generic_perform_write
 *		grab_cache_page_write_begin
 *		iov_iter_copy_from_user_atomic
 *		submit_bh_kretp
 *		submit_bio_kretp
 *		scsi_queue_rq
 */


static int __init kvisual_init(void)
{
	int ret;

	/* iopath_kps is a static group including all kprobes about io path */
	// ret = register_kprobes(iopath_kps, IO_DYNAMIC_KPROBE_NUM);
	// if (ret < 0) {
	// 	printk(KERN_DEBUG "register_kprobe failed, returned %d\n", ret);
	// 	return ret;
	// }

	/* kret probe */
	ret = register_kretprobes(iopath_kretps, IO_DYNAMIC_KRETPROBE_NUM);
	if (ret < 0) {
		pr_err("register_kretprobe failed, returned %d\n", ret);
		return ret;
	}

	/* register static trace */
	register_static_io_trace();

	printk(KERN_INFO "Welcome to Kernel Visualization!\n");
	return 0;
}

static void __exit kvisual_exit(void)
{
	unregister_kprobes(iopath_kps, IO_DYNAMIC_KPROBE_NUM);
	unregister_static_io_trace();
	printk("Kernel Visualization exit!\n");
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("double_D");
module_init(kvisual_init);
module_exit(kvisual_exit);