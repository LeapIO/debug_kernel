/**
 * @file iovisual.c
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-04-29
 * @copyright Copyright double_D
 * 
 */
#include "entry.h"
#include "iovisual.h"

/* struct pt_regs in linux/arch/x86/include/asm/ptrace.h */

/* 全局变量 默认是局部可用的，想要其它模块可用，必须在使用该变量的模块中extern该变量符号 */
struct kprobe *iopath_kps[IO_DYNAMIC_KPROBE_NUM] = {};

/* up: dynamic tracepoints entry */
/* dwon: dynamic tracepoints entry return point */


/**
 * @brief  open file, a frequent call, but important
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int do_sys_openat2_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    int dfd;
    const char __user *filename;
    struct open_how *how;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    dfd = (int)regs->di;
    filename = (const char *)regs->si;
    how = (struct open_how *)regs->dx;
#endif

    printk(
        KERN_INFO 
            "Function: do_sys_openat2 in linux/fs/open.c\n"
            "   Open request info:\n"
            "       dfd=%d, filename=%s, flags=%lld, mode=%lld\n",
            dfd, filename, how->flags, how->mode);

    return 0;
}

/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int do_sys_openat2_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    unsigned long retval = regs_return_value(regs);
    printk(
        KERN_INFO
        "Return Function: do_sys_openat2\n"
        "   new fd=%ld\n", retval);

    return 0;
}

static struct kretprobe do_sys_openat2_kretp = {
    .kp.symbol_name = "do_sys_openat2",
    .entry_handler	= do_sys_openat2_entry_handler,
    .handler		= do_sys_openat2_ret_handler,
    .maxactive		= 0,
};


/**
 * @brief static ssize_t ext4_file_write_iter(struct kiocb *iocb, struct iov_iter *from)
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int ext4_file_write_iter_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct ext4_write_probe_point _pp;
    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
	_pp.pp_iocb = (struct kiocb *)regs->di;
	_pp.pp_from = (struct iov_iter *)regs->si;
#endif

    /* struct file */
	_pp.pp_file = (struct file *)_pp.pp_iocb->ki_filp;
	_pp.pp_path = (struct path *)(&_pp.pp_file->f_path);
    _pp.pp_flags = _pp.pp_file->f_flags;
	_pp.pp_mode = _pp.pp_file->f_mode;
	_pp.pp_pos = _pp.pp_file->f_pos;

    /* struct dentry */
    _pp.pp_dentry = _pp.pp_path->dentry;
    _pp.pp_name = _pp.pp_dentry->d_name.name;

    /* struct inode */
    _pp.pp_inode = (struct inode *)_pp.pp_file->f_inode;
    _pp.pp_ino = _pp.pp_inode->i_ino;
    _pp.pp_size = _pp.pp_inode->i_size;
    _pp.pp_bytes = _pp.pp_inode->i_bytes;
    _pp.pp_blocks = _pp.pp_inode->i_blocks;

    /* struct sb */
    _pp.pp_sb =  _pp.pp_inode->i_sb;
    _pp.pp_blocksize = _pp.pp_sb->s_blocksize;
    _pp.pp_maxbytes = _pp.pp_sb->s_maxbytes;
    _pp.pp_fs_type = _pp.pp_sb->s_type;
    _pp.pp_root = _pp.pp_sb->s_root;

    printk(
        KERN_INFO
            "Function: ext4_file_write_iter in linux/fs/ext4/file.c\n" 
            "   An ext4 write request!\n" 
            "   VFS info:\n"
            "       fs=%s, fs_blocsize=%lu, fs_root_dentry=%s\n"
            "       inode=%ld, filename=%s\n"
            "       ppos=%lld, write_size=%ld, user_buff=%pK\n", 
            _pp.pp_fs_type->name, _pp.pp_blocksize, _pp.pp_root->d_name.name,
            _pp.pp_ino, _pp.pp_name,
            _pp.pp_iocb->ki_pos, _pp.pp_from->count, _pp.pp_from->iov->iov_base);
    return 0;
}

/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int ext4_file_write_iter_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    unsigned long retval = regs_return_value(regs);
    printk(
        KERN_INFO
        "Return Function: ext4_file_write_iter\n"
        "   Written bytes=%ld\n", retval);

    return 0;
}

static struct kretprobe ext4_file_write_iter_kretp = {
    .kp.symbol_name = "ext4_file_write_iter",
    .entry_handler	= ext4_file_write_iter_entry_handler,
    .handler		= ext4_file_write_iter_ret_handler,
    .maxactive		= 0,
};


/**
 * @brief  const struct address_space_operations *a_ops = mapping->a_ops;
 *  a_ops for ext4 is function ext4_da_aops
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int generic_perform_write_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct file *file;
    struct iov_iter *i;
    loff_t pos;

    unsigned long first_offset;
    unsigned long first_write_bytes;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    file = (struct file *)regs->di;
    i = (struct iov_iter *)regs->si;
    pos = (loff_t)regs->dx;
#endif

    first_offset = (pos & (PAGE_SIZE - 1));
    first_write_bytes = min_t(unsigned long, PAGE_SIZE - first_offset, iov_iter_count(i));

    printk(
        KERN_INFO 
        "Function: generic_perform_write in linux/mm/filemap.c\n" 
        "   Ext4 prepare do buffered write to pagecacheand a_ops is ext4_da_aops\n"
        "   Basic process:\n"
        "       1. lock inode\n"
        "       2. find or create a page for cache\n"
        "       3. do while write to pagecache\n"
        "       4. unlock inode\n"
        "   First offset=%ld (pos & (PAGE_SIZE - 1))\n"
        "   First bytes=%ld (min_t(unsigned long, PAGE_SIZE - first_offset, i->count))\n", 
        first_offset, first_write_bytes);
    return 0;
}

/**
 * @brief 
 * @return int @c 
 */
static int generic_perform_write_ret_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    unsigned long retval = regs_return_value(regs);
    printk(
        KERN_INFO
        "Return Function: generic_perform_write\n"
        "   written bytes=%ld\n", retval);

    return 0;
}

static struct kretprobe generic_perform_write_kretp = {
    .kp.symbol_name = "generic_perform_write",
    .entry_handler	= generic_perform_write_entry_handler,
    .handler		= generic_perform_write_ret_handler,
    .maxactive		= 0,
};


/**
 * @brief Find or create a page at the given pagecache position
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int grab_cache_page_write_begin_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct address_space *mapping;
    pgoff_t index;  // pos >> PAGE_SHIFT
    unsigned flags;

    struct inode *_host;
    unsigned long _nrpages;
    unsigned long _i_ino;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    mapping = (struct address_space *)regs->di;
    index = (pgoff_t)regs->si;
    flags = (unsigned)regs->dx;
#endif

    _host = mapping->host;
    _i_ino = _host->i_ino;
    _nrpages = mapping->nrpages;

    printk(
        KERN_INFO 
        "Function: grab_cache_page_write_begin in linux/mm/filemap.c\n"
        "   Find or create a page at the given pagecache position\n"
        "       ino=%ld, nr pages=%ld, index(pos >> PAGE_SHIFT)=%ld, flags=%d\n",
        _i_ino, _nrpages, index, flags);

    return 0;
}


/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int grab_cache_page_write_begin_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    unsigned long retval = regs_return_value(regs);

    printk(
        KERN_INFO
        "Return Function: grab_cache_page_write_begin\n"
        "   get one page=%pK\n", (struct page *)retval);

    return 0;
}

static struct kretprobe grab_cache_page_write_begin_kretp = {
    .kp.symbol_name = "grab_cache_page_write_begin",
    .entry_handler	= grab_cache_page_write_begin_entry_handler,
    .handler		= grab_cache_page_write_begin_ret_handler,
    .maxactive		= 0,
};


/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int iov_iter_copy_from_user_atomic_entry_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct page *page;
    struct iov_iter *i; 
    unsigned long offset; 
    size_t bytes;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    page = (struct page *)regs->di;
    i = (struct iov_iter *)regs->si;
    offset = (unsigned long)regs->dx;
    bytes = (size_t)regs->cx;
#endif

    printk(
        KERN_INFO 
        "Function: iov_iter_copy_from_user_atomic in linux/lib/iov_iter.c\n"
        "   Copy data from user buf to kernel page cache\n"
        "       page=%pK, offset in one pgae=%ld, bytes=%ld will be written\n",
        page, offset, bytes);
    return 0;
}

/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int iov_iter_copy_from_user_atomic_ret_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    unsigned long retval = regs_return_value(regs);
    printk(
        KERN_INFO
        "Return Function: iov_iter_copy_from_user_atomic\n"
        "   copied bytes=%ld\n", retval);

    return 0;
}

static struct kretprobe iov_iter_copy_from_user_kretp = {
    .kp.symbol_name = "iov_iter_copy_from_user_atomic",
    .entry_handler	= iov_iter_copy_from_user_atomic_entry_handler,
    .handler		= iov_iter_copy_from_user_atomic_ret_handler,
    .maxactive		= 0,
};

/**
 * @brief for betetr output 
 * in linux/include/linux/blk_types.h
 * enum req_opf 
 */
// static char *default_op = "NO_DEFINE";
// static char *blk_req_opf[] = {"REQ_OP_READ", "REQ_OP_WRITE"};

/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int submit_bh_entry_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    int op;
    int op_flags;
    struct buffer_head *bh;

    // unsigned long _b_state;      /* buffer state bitmap (see above) */
    struct page *_b_page;		/* the page this bh is mapped to */    
    sector_t _b_blocknr;         /* start block number */
    size_t _b_size;		        /* size of mapping */	
    // char *_b_data;               /* pointer to data within the page */
    // struct block_device *_b_bdev;

    // sector_t _bd_start_sect;
    // bool _bd_read_only;	/* read-only policy */
    // dev_t _bd_dev;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    op = (int)regs->di;
    op_flags = (int)regs->si;
    bh = (struct buffer_head *)regs->dx;
#endif

    // _b_state = bh->b_state;
    _b_page = bh->b_page;
    _b_blocknr = bh->b_blocknr;
    _b_size = bh->b_size;
    // _b_data = bh->b_data;
    // _b_bdev = bh->b_bdev;

    //"   Device start sect=%lld, read_only=%d, dev_t=%d\n",
    // _bd_start_sect = _b_bdev->bd_start_sect;
    // _bd_read_only = _b_bdev->bd_read_only;
    // _bd_dev = _b_bdev->bd_dev;

    printk(
        KERN_INFO
        "Function: submit_bh in linux/fs/buffer.c\n"
        "   Page is consisted of serveral buffer_head, representing CONTINUOUS logical area in file\n"
        "       op=%d, flags=%d, buffer in page=%pK\n" 
        "       start block number (fs logic block number)=%lld, buffer_head size=%ld(fs block size)\n",
        op, op_flags,
        _b_page,
        _b_blocknr, _b_size);

    return 0;
}

static int submit_bh_ret_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    return 0;
}

/**
 * @brief 
 */
static struct kretprobe submit_bh_kretp = {
    .kp.symbol_name = "submit_bh",
    .entry_handler	= submit_bh_entry_handler,
    .handler		= submit_bh_ret_handler,
    .maxactive		= 0,
};


/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int submit_bio_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    int i_vcnt;
    struct bio *bio;
    struct gendisk *_bi_disk;

    /* in disk it is continuous */
    // struct bvec_iter {
    //     sector_t bi_sector;	/* device address in 512 byte sectors */
    //     unsigned int bi_size;	/* residual(残留的) I/O count */
    //     unsigned int bi_idx;	/* current index into bvl_vec */
    //     unsigned int bi_bvec_done;	/* number of bytes completed in current bvec */
    // };
    struct bvec_iter *_bi_iter;

    /* one bio maybe holds serveral bio_vec */
    // struct bio_vec {
	//     struct page	*bv_page;
	//     unsigned int	bv_len;
	//     unsigned int	bv_offset;
    // };
	unsigned short _bi_vcnt;	/* how many bio_vec's, how many pages */
	unsigned short _bi_max_vecs;	/* max bvl_vecs we can hold */
	struct bio_vec *_bi_io_vec;  /* array, len is bi_vcnt, per page */

    bool has_submit_bio_op;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    bio = (struct bio *)regs->di;
#endif

    _bi_disk = bio->bi_disk;
    _bi_iter = &bio->bi_iter;

    _bi_vcnt = bio->bi_vcnt;
	_bi_max_vecs = bio->bi_max_vecs;
    _bi_io_vec = bio->bi_io_vec;

    /** 
     * Submit a bio to the block device layer for I/O
     * bio is the main unit of I/O for the block layer and lower layers
     * It describes the relationship between PAGECACHE and DISK
    */

    if(_bi_disk->fops){
        if(_bi_disk->fops->submit_bio)
            has_submit_bio_op = true;
        else
            has_submit_bio_op = false;
    }
    else {
        has_submit_bio_op = false;
    }

    printk(
        KERN_INFO
        "Function: submit_bio in linux/block/blk-core.c\n"
        "   IO still in fs\n"
        "   If bi_disk->fops has submit_bio: %d\n"
        "   bio->bi_iter represents a CONTINUOUS physical area in disk\n"
        "       bi_sector(bh->b_blocknr * (bh->b_size >> 9))(disk sector num)=%lld, total size=%d\n"
        "           total size is ingetral multiple of fs block size\n"
        "   bio holds serveral bio_vec(s)(pages)\n"
        "       nr_bio_vec=%d, max=%d\n"
        "       ...",
        has_submit_bio_op,
        _bi_iter->bi_sector, _bi_iter->bi_size,
        _bi_vcnt, _bi_max_vecs
    );

    for(i_vcnt=0; i_vcnt<_bi_vcnt; ++i_vcnt){
        struct bio_vec *bv = &_bi_io_vec[i_vcnt];
        printk(
            KERN_INFO
            "       io vector %d: page=%pK, offset in page=%d, size=%d, corresponding to bi_iter\n"
            "           size is ingetral multiple of fs block size",
            i_vcnt,
            bv->bv_page, bv->bv_offset, bv->bv_len);
    }

    return 0;
}

/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int submit_bio_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    // unsigned long retval = regs_return_value(regs);
    // printk(
    //     KERN_INFO
    //     "Return Function: submit_bio\n");

    return 0;
}

/**
 * @brief submit a bio to the block device layer for I/O
 */
static struct kretprobe submit_bio_kretp = {
    .kp.symbol_name = "submit_bio",
    .entry_handler	= submit_bio_entry_handler,
    .handler		= submit_bio_ret_handler,
    .maxactive		= 0,
};

/* leave fs, IO enters generic block layer */

/* leave generic block layer, IO enters scsi upper layer */

/**
 * @brief 
 * @param  ri               desc
 * @param  regs             desc
 * @return int @c 
 */
static int scsi_queue_rq_entry_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct blk_mq_hw_ctx *hctx;
    struct blk_mq_queue_data *bd;

    struct request *_req;  /* request */
    struct request_queue *_q;  /* request queue in device */
    struct scsi_device *_sdev;  /* scsi device */
    struct Scsi_Host *_shost;  /* scsi host dev, hostx */
    struct scsi_cmnd *_cmd;  /* scsi cmd */

    struct blk_mq_tag_set *_tag_set;

    if(!kvisual_supported_arch()){
        return -1;
    }

/* for extension */
#ifdef CONFIG_X86
    hctx = (struct blk_mq_hw_ctx *)regs->di;
    bd = (struct blk_mq_queue_data *)regs->si;
#endif

    _req = bd->rq;
	_q = _req->q;
	_sdev = _q->queuedata;
	_shost = _sdev->host;
	_cmd = blk_mq_rq_to_pdu(_req);

    _tag_set = &_shost->tag_set;

    printk(
        KERN_INFO
        "Function: scsi_queue_rq in linux/drivers/scsi/scsi_lib.c\n"
        "   request_queue property:\n"
        "      queue depth=%d, nr hw queues=%d\n"
        "   scsi device property:\n"
        "       scsi queue depth=%d, scsi max queues depth=%d, "
        "scsi id=%d, scsi channel=%d, scsi lun=%lld, scsi sector(byte size)=%d, scsi level=%c\n"
        "   set tag:\n"
        "       nr maps=%d, nr_hw_queues=%d, queue_depth=%d, numa=%d\n",
        _q->queue_depth, _q->nr_hw_queues,
        _sdev->queue_depth, _sdev->max_queue_depth, 
        _sdev->id, _sdev->channel, _sdev->lun, _sdev->sector_size, _sdev->scsi_level,
        _tag_set->nr_maps, _tag_set->nr_hw_queues, _tag_set->queue_depth, _tag_set->numa_node
        );

    return 0;
}

static int scsi_queue_rq_ret_handler(
    struct kretprobe_instance *ri, struct pt_regs *regs)
{
    // unsigned long retval = regs_return_value(regs);
    return 0;
}

static struct kretprobe scsi_queue_rq_kretp = {
    .kp.symbol_name = "scsi_queue_rq",
    .entry_handler	= scsi_queue_rq_entry_handler,
    .handler		= scsi_queue_rq_ret_handler,
    .maxactive		= 0,
};
/* leave scsi upper layer, IO enters scsi low level driver, eg. ata_piix*/

struct kretprobe *iopath_kretps[IO_DYNAMIC_KRETPROBE_NUM] = {
    &do_sys_openat2_kretp,
    &ext4_file_write_iter_kretp,
    &generic_perform_write_kretp,
    &grab_cache_page_write_begin_kretp,
    &iov_iter_copy_from_user_kretp,
    &submit_bh_kretp,
    &submit_bio_kretp,
    /* leave fs, IO enters generic block layer */
    /* leave generic block layer, IO enters scsi upper layer */
    &scsi_queue_rq_kretp
};


/* up: dynamic tracepoints */
/* dwon: static tracepoints */


/**
 * @brief in linux/fs/ext4/inode.c
 * @param  ignore           desc
 * @param  inode            desc
 * @param  pos              desc
 * @param  len              desc
 * @param  copied           desc
 */
static void ext4_da_write_begin_trace(
    void *ignore, 
    struct inode *inode, loff_t pos, unsigned int len, unsigned int flags)
{
    printk(KERN_INFO 
        "Function: ext4_da_write_begin in linux/fs/ext4/inode.c\n"
        "   Begin to write\n"
        "       ino=%ld, pos(offset in file)=%lld, len=%d, flags=%d\n",
        inode->i_ino, pos, len, flags);
    return;
}

static void ext4_da_write_end_trace(
    void *ignore, 
    struct inode *inode, loff_t pos, unsigned int len, unsigned int copied)
{
    printk(KERN_INFO 
        "Function: ext4_da_write_end!\n"
        "   Write has been finished\n"
        "       ino=%ld, pos=%lld, len=%d, copied=%d\n",
        inode->i_ino, pos, len, copied);
    return;
}

/**
 * @brief 
 * @param  ignore           desc
 * @param  bio              desc
 */
static void block_getrq_trace(void *ignore, struct bio *bio)
{
    printk(KERN_INFO 
        "Function: block_getrq!\n"
        "   get a free request entry in queue for block IO operations\n");
    return;
}

/**
 * @brief register static tracepoints
 */
void register_static_io_trace(void)
{
    int ret;

    /* trace_ext4_da_write_begin(inode, pos, len, flags); */
    ret = register_trace_ext4_da_write_begin(ext4_da_write_begin_trace, NULL);
    WARN_ON(ret);

    // /* trace_ext4_da_write_end(inode, pos, len, copied) */
    ret = register_trace_ext4_da_write_end(ext4_da_write_end_trace, NULL);
    WARN_ON(ret);

    /* EXPORT_TRACEPOINT_SYMBOL_GPL(block_getrq); in linux/block/blk-mq.c */
    ret = register_trace_block_getrq(block_getrq_trace, NULL);
	WARN_ON(ret);

    return;
}

/**
 * @brief 
 */
void unregister_static_io_trace(void)
{
    unregister_trace_ext4_da_write_begin(ext4_da_write_begin_trace, NULL);
    unregister_trace_ext4_da_write_end(ext4_da_write_end_trace, NULL);
    unregister_trace_block_getrq(block_getrq_trace, NULL);
}