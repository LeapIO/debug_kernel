#!/bin/bash
# 这是一个半自动的 git 提交脚本
home=$HOME
_pwd=`pwd`
# echo $home
# echo $_pwd
ctime=`date +"%Y-%m-%d %H:%M:%S​"`
# echo $ctime
if [ ! $1 ]; then
    commitlog=$ctime
else
    commitlog="$1: $ctime"
fi
# echo $commitlog

git add .
git commit -m "$commitlog"  # commitlog 本身是带空格的，所以必须用双引号引起来
git push