CC = cc -std=c99
CXX = c++ -std=c++11
LD = ld

# path
CUR_DIR = $(shell pwd)
PROJECT_ROOT = $(CUR_DIR)/../
BIN_DIR = $(CUR_DIR)/../bin/example/
# DIR_OBJ = $(CUR_DIR)/../obj/example/
# 指定.h的路径以及lib的所在路径（非默认路径的话）
INCLUDES_DIR = $(CUR_DIR)/../include/
LIBS_DIR = $(CUR_DIR)/../lib/

# 指定编译参数cflags以及includesfalgs以及link的falgs
# example采用静态编译
CFLAGS := -g -Wall -O -static
LDFLAGS := -lpthread
INCLUDESFLAGS := -I$(INCLUDES_DIR)  # -I与路径之间无空格

# 如果我的输出在BIN文件夹下，而不再当前文件夹下，每次make的时候都会发现当前路径下目标不存在的现象
# 所以每次make尽管目标是不存在的，那么就会重新执行，所以其实每次make都会全部执行，测试不到单独改.h的那种情况了就
SRCS := $(wildcard *.cc)
SRCS += $(wildcard *.cpp)
DEPS := $(SRCS:.cc=.d)  # 主要处理的是头文件的依赖
DEPS := $(DEPS:.cpp=.d)
EXE := $(BIN_DIR)/example
EXE += $(BIN_DIR)/client

# 其它变量
RM = rm -rf

.PHONT:all
all:$(EXE)

# 如果同时包含了 example1.cc example.h，这样的混合形式，.h与.cc会被分开编译，会有两个输出文件, 一个是a.out，一个是example.h.gch
# 所以如果指定了-o就报错了，实际上编译的时候本就无需加.h的
# 如果这个.h在.cc中有被include会被自动找搜索到
# 随之而来的一个问题是，如果.h发生变化，再次make的时候，由于依赖.cc是没有变化的，所以.h中新的变化（比如define常量的改变）
# 差异会体现不到，所以需要下面的解决方案
$(BIN_DIR)/example: example.cc
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) $^ -o $@ $(INCLUDESFLAGS) $(LDFLAGS)

$(BIN_DIR)/client: client.cc
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) $^ -o $@ $(INCLUDESFLAGS) $(LDFLAGS)

# 拷贝bin/example下的可执行文件到initramfs中
# 这个路径是会改变的，要注意调整
# ...
# TARGET_DIR = $(PWD)/../../build_ramdisk/initrmfs/double_D/testcase
# CP = $(shell cp $(EXE) $(TARGET_DIR))
# exp: $(CP)
# 	@echo export successfully

.PHONY:test
test:
	@echo do check in example

.PHONY:clean
clean:
	@$(RM) *.o *.dSYM
	@$(RM) $(EXE)
	@$(RM) $(BIN_DIR)
	@echo example clean done
