/**
 * @file iovisual.h
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-04-29
 * @copyright Copyright double_D
 * 
 */
#ifndef _IOVISUAL_H
#define _IOVISUAL_H

#include <linux/fs.h>
#include <linux/uio.h>
#include <linux/dcache.h>
#include <uapi/linux/openat2.h>
#include <linux/blk_types.h>
#include <linux/blk-mq.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <linux/device.h>
#include "../../../fs/ext4/ext4.h"  /* 路径暂时先写按该文件所在的位置改 */
#include <trace/events/ext4.h>
#include <trace/events/block.h>

#define IO_DYNAMIC_KPROBE_NUM 0
#define IO_DYNAMIC_KRETPROBE_NUM 8

extern struct kprobe *iopath_kps[];
extern struct kretprobe *iopath_kretps[];

struct ext4_write_probe_point {
    /* parameters in function ext4_file_write_iter */
    struct kiocb *pp_iocb;
	struct iov_iter *pp_from;

    /* struct file */
	struct file	*pp_file;
	struct path *pp_path;
	unsigned int pp_flags;
	fmode_t pp_mode;
	loff_t pp_pos;

    /* struct dentry */
    struct dentry *pp_dentry;
    const unsigned char *pp_name;  /* dentry name in dentry tree and in hash table */

    /* struct inode */
    struct inode *pp_inode;
    unsigned long pp_ino;  /* indoe num */
    loff_t pp_size;
    unsigned short pp_bytes;
    blkcnt_t pp_blocks;
    struct address_space *pp_mapping;

    /* struct sb */
    struct super_block *pp_sb;
    unsigned long pp_blocksize;
    loff_t pp_maxbytes;
    struct file_system_type	*pp_fs_type;
    struct dentry *pp_root;
};

void register_static_io_trace(void);
void unregister_static_io_trace(void);

#endif