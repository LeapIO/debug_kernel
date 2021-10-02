CC = cc -std=c99
CXX = c++ -std=c++11
LD = ld

# path
CUR_DIR = $(shell pwd)
PROJECT_ROOT = $(CUR_DIR)/../
BIN_DIR = $(CUR_DIR)/../bin/test/
INCLUDES_DIR = $(CUR_DIR)/../include/
DIR_SRC = $(CUR_DIR)/../src/
LIBS_DIR = $(CUR_DIR)/../lib/
CFLAGS := -g -Wall -O -Wno-reorder
# -static
LDFLAGS := -lpthread
INCLUDESFLAGS := -I$(INCLUDES_DIR) -I$(DIR_SRC)

SRCS := $(wildcard *.cc)
SRCS += $(wildcard *.cpp)
DEPS := $(SRCS:.cc=.d)
DEPS := $(DEPS:.cpp=.d)

EXE := $(BIN_DIR)/timer

RM = rm -rf

.PHONT:all
all:$(EXE)


$(BIN_DIR)/timer: timer.cc
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) $^ -o $@ $(INCLUDESFLAGS) $(LDFLAGS)

.PHONY:test
test:
	@echo do check in example

.PHONY:clean
clean:
	@$(RM) *.o *.dSYM
	@$(RM) $(EXE)
	@$(RM) $(BIN_DIR)
	@echo example clean done
