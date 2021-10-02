CC = cc -std=c99
CXX = g++ -std=c++11
LD = ld

# path
CUR_DIR = $(shell pwd)
ROOT_DIR = $(CUR_DIR)/../
BIN_DIR = $(CUR_DIR)/../bin/
LIBS_DIR = $(CUR_DIR)/../lib/
# extern headers 
INCLUDES_DIR = $(CUR_DIR)/../include/
# some headers are in src or unit
DIR_SRC = $(CUR_DIR)/../src/
DIR_UNIT = $(DIR_SRC)/unit/
# dirs need to be created automatically
DIRS = $(BIN_DIR) $(LIBS_DIR) $(INCLUDES_DIR)
# sub dirs
# SUB_DIRS = $(shell ls -l | grep ^d | awk '{if($$9!="collect") print $$9}')
# 默认空格分隔符无需-F指定$9就是最后一个
SUB_DIRS := $(shell ls -l | grep ^d | awk '{print $$9}')
# 如果有include文件夹，就清理掉
SUB_DIRS := $(patsubst include,,$(SUB_DIRS))
# path of pub.mk
MKS_DIR = $(CUR_DIR)/../mks/

# some flags
CFLAGS := -g -Wall -O -Wno-reorder
LDFLAGS := -lpthread
# -static
INCLUDEFLAGS := -I$(INCLUDES_DIR) -I$(DIR_SRC) -I$(DIR_UNIT)

BIN := codesnip
# 可以执行文件放到指定目录下
BIN := $(addprefix $(BIN_DIR), $(BIN))
MODULE_PREFIX = -m.o

include $(MKS_DIR)/gensrc.mk

GLOBAL_OBJ := $(OBJS)
GLOBAL_OBJ += $(addsuffix $(MODULE_PREFIX),$(SUB_DIRS))  # 子目录以目录名作为模块名

# 其它变量
RM = rm -rf

# 这个只能单向传递，父进程可以向子进程传递环境变量，但是子进程无法修改父进程的环境变量
export CC CXX LD CFLAGS INCLUDEFLAGS MKS_DIR RM   

# 伪目标文件，伪目标文件在第一个也会被当做最终目标文件去执行 all 这个东西叫 lable
.PHONY:
# 创建目录  递归子目录完成编译 link
all : $(DIRS) $(SUB_DIRS) $(BIN)

# $@ 目标文件
# $^ 所有的依赖文件
# $< 第一个依赖文件
$(DIRS):
	@mkdir -p $@

$(BIN): $(GLOBAL_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

include $(MKS_DIR)/deprule.mk

.PHONY:test
test:
	@echo do check in src

.PHONY:clean
clean:
	@$(RM) *.d *.o *.d.* $(BIN)
	for dir in $(SUB_DIRS);do\
		$(MAKE) -C $$dir clean;\
		done
	@echo src clean done