MODULE = syncmutx-m.o
PWD = $(shell pwd)
PRE_DIR = $(PWD)/../

SUB_DIRS = $(shell ls -l | grep ^d | awk '{print $$9}')
SUB_DIRS := $(patsubst include,,$(SUB_DIRS))

include $(MKS_DIR)/gensrc.mk

# 源码下会写a.cpp对应的a.h头文件
INCLUDEFLAGS += -I$(shell pwd)
# $(info $(INCLUDEFLAGS))

all: $(SUB_DIRS) $(MODULE)

$(MODULE): $(OBJS)
	$(LD) -r $^ -o $(PRE_DIR)/$@

include $(MKS_DIR)/deprule.mk

.PHONY:test
test:
	@echo do check in src

.PHONY:clean
clean:
	-@$(RM) *.d *.o *.d.*
	for dir in $(SUB_DIRS);do\
		$(MAKE) -C $$dir clean;\
		done
	@echo src clean done