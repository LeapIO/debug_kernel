# add your self vmlinux path here
# file /home/doubled/double_D/linux/vmlinux
# file /usr/src/linux/vmlinux
file /home/founder/hba/linux/linux/vmlinux
# source /home/founder/hba/linux/linux/vmlinux-gdb.py
target remote:1236
#b arch/x86/boot/compressed/head_64.S:172
b arch/x86/boot/compressed/head_64.S:262
# b arch/x86/kernel/head_64.S:65
# b arch/x86/kernel/head_64.S:117
# b arch/x86/kernel/head_64.S:283
# b start_kernel
# b leapio_xdma_isr
# b leapio_xdma_engine_service_work
# b ahci_do_hardreset
# b drivers/ata/libata-eh.c:2691
# b drivers/ata/libata-eh.c:2705
# b ata_scsi_error
# b leapio_init_one
# b leapio_xdma_map_bars
# b ahci_scr_read
# b ahci_qc_issue
# b ahci_handle_port_interrupt
# b ata_qc_issue
# b drivers/ata/libata-core.c:1932
# b ata_qc_complete
# b drivers/leapioata/leapioata.c:1621
# b leapiosas_probe
# b drivers/scsi/leapiosas/leapiosas_init.c:38
# b aac_probe_one
# b leapio_sas_exec_task
# b drivers/scsi/leapiosas/leapiosas.c:564
# b leapio_sas_core_reset
# b leapio_sas_probe
# b leapio_sas_handle_iaf
# b leapio_sas_port_event
# b leapio_sas_dump_task_file
# b xpdma_init
b simple_operation
b xpdma_write
c
# /home/founder/hba/linux/linux/drivers/scsi/leapiosas/leapiosas.c