file ../vmlinux
source ../vmlinux-gdb.py
target remote:1234
# b start_kernel
# b scsi_probe_and_add_lun
# b leapio_sas_exec_task
# # b leapio_sas_prep_task
# b leapio_sas_complete_task
# b scsi_probe_lun
# b scsi_add_lun
# b scsi_mode_sense
# b drivers/scsi/scsi_scan.c:738
# b sasi_finish
# b drivers/scsi/libsas/sas_init.c:153
# b sas_form_port
# b leapio_sas_port_formed
c