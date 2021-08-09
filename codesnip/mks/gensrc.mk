# wildcard 被展开为被空格分开的，已经存在的，匹配模式的 就是源文件的 list
SRCS := $(wildcard *.cc)
SRCS += $(wildcard *.cpp)
# 主要处理的是头文件的依赖，循环 src list，将.cc 替换为 .d
DEPS := $(SRCS:.cc=.d)
DEPS := $(DEPS:.cpp=.d)
# OBJS = $(SRCS:.cc=.o)
OBJS := $(patsubst %.cc, %.o, $(SRCS))
OBJS := $(patsubst %.cpp, %.o, $(OBJS))