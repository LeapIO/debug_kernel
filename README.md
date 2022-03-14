## kernel_debug
- A linux kernel debug env based on qemu and gdb

### what you need
1. linux kernel src (git clone from git@github.com:torvalds/linux.git)
2. qemu
3. cgdb
4. ...

### steps
1. download linux kernel src and compile it to **vmlinux** (**with debug_info and no any install**)
2. pull debug_kernel and put it in the first level of linux kernel src code
3. `cd debug_kernel` && `make mad` (**details are in Makefile**)
    - gen manual `ramdisk.img` including busybox with the **most basic linux cmd**
4. `cd debug_kernel` && `make dr`
5. **edit gdbinit** and **replace** the path of vmlinux of yours
6. **cgdb -q -x gdbinit**
7. More details are in Makefile, you can choose kinds of blocks (**ahci disk, nemv ssd, nvdimm, etc.**) to debug corresponding driver and all parts in kernel, you can gen **backend storage file** by Makefile

### features (details are in Makefile)
1. debug kernel without blokc dev
2. debug kernel with a **ahci disk device** in `/dev`
3. debug nvdimm with a **nvdimm device** in `/dev`
4. debug nvme ssd with a **nvme ssd device** in `/dev`
5. debug pcie hba by **vfio**

#### To be continued ...