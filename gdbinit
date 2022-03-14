# add your self vmlinux path here
# file /home/doubled/double_D/linux/vmlinux
# file /usr/src/linux/vmlinux
file /home/founder/hba/linux/linux/vmlinux
# source /home/founder/hba/linux/linux/vmlinux-gdb.py
target remote:1234
b drivers/zcahci/zcahci.c:152
c