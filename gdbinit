# file /home/doubled/double_D/linux/vmlinux
file /usr/src/linux/vmlinux
target remote:1234
# b start_kernel

# open syscall
# b do_sys_open if strcmp(filename,"open")==0  # always mode=0 or mode=436
# b do_sys_open if strstr(filename,"opentest")
# b do_sys_open if mode==438 && flags==32834
# b do_sys_open if strcmp(filename,"/dev/dax0.0")==0
# b do_sys_open if flags=32770
# b ksys_mmap_pgoff
# b acpi_nfit_init
# chmod syscall
# b do_fchmodat
# b ata_scsi_scan_host
# b blk_mq_init_sq_queue
# b nvme_probe
# b nvme_core_init
# b nvme_pci_configure_admin_queue
# b nvme_set_queue_count
# b nvme_probe
# b nvme_alloc_ns
# b nvme_dev_add
c