#!/bin/bash
# 基本功能包括
#     getattr
#     mkdir
#     unlink
#     rmdir
#     open
#     read
#     write
#     readdir
#     create
#     init
# 主要是测试功能上的不报错，表现基本正常即可

# start
# ./toydfsclient /mnt/toydfsclient

firstdirs=128  # 一级目录的个数
seconddirs=16  # 二级目录的个数
nfiles=3  # 二级目录下的文件数量
maxconcurrent=$(nproc)  # 最大的并发任务数量
waittimes=0

# 限制并发线程的数量
waitforjobs() {
    # 当前任务数量大于$1(所限制的最大后台进程数量)
    # test其实就相当于是if
    # a -ge b a是否大于b
    # wc -l lines/wc -w words
    # wait Wait for job completion and return exit status
    # wait -n means wait for next
    # jobs is a shell builtin 
    while test $(jobs -p | wc -w) -ge "$1"; do 
        waittimes=$(($waittimes+1)) 
        echo "wait for ... $waittimes"
        wait -n;
    done
}

echo "begin to make first dirs ..."
# 创建目录，连续生成多个目录
for i in $(seq 1 $firstdirs)
do
    waitforjobs $maxconcurrent
    mkdir -p "/mnt/toydfsclient/$i" &  # 都是要fork新进程的
done
echo "waiting for first dirs ..."
wait  # 等待一级目录创建完毕，wait等待所有子进程的退出
echo "finish first dirs and begin to mkdir second dirs ..."

for i in $(seq 1 $firstdirs); do
    for ii in $(seq 1 $seconddirs); do
        waitforjobs $maxconcurrent
        # cd "/mnt/toydfsclient/$i" && mkdir -p "$ii" &
        # 一级目录创建完毕后所有的命令都是互不相关的，但是在namei的过程中是会造成并发的
        mkdir -p "/mnt/toydfsclient/$i/$ii" &
    done
done
echo "waiting for second dirs ..."
wait
echo "finish second dirs and begin create files ..."

# for i in $(seq 1 $firstdirs); do
#     for ii in $(seq 1 $seconddirs); do
#         for iii in $(seq 1 $nfiles); do
#             waitforjobs $maxconcurrent
#             # cd "/mnt/toydfsclient/$i/$ii" && touch $iii.log &
#             touch "/mnt/toydfsclient/$i/$ii/$iii.log" &  # 双引号是能够解析$的 
#         done
#     done
# done
# echo "waiting for files ..."
# wait
# echo "finish for files and begin to delete..."

# # 删除目录，随机删除500个目录
# # 函数调用规则如下，注意$1与$2
# random(){
#     min=$1
#     max=$(($2 - $min + 1))
#     num=$(($RANDOM+1000000000)) # 增加一个10位的数再求余
#     echo $(($num%$max + $min))
# }

# for i in $(seq 1 20)
# do
# rm -rf /mnt/toydfsclient/$i
# done

# for i in $(seq 20 40)
# do
# rand_no=$(random 1 10)
# rm -rf /mnt/toydfsclient/$i/$rand_no
# done

# for iiii in $(seq 1 10)
# do
# touch touch /mnt/toydfsclient/$iiii.txt
# done

# 后续直接去执行python脚本测试read/write
# python ptest.py
# python不争气啊，写个IO还没有C写的舒服
# cd /root/6.824/src/toydfs/client/test && ./basetest
# cd /root/6.824/src/toydfs/client/test && ./concurrent
exit 0
