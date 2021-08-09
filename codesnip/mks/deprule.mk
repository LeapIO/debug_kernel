# make -C 进入目录执行makefile
# 在makefile中可以预先使用一个未定义的变量M， 这个变量可以在make执行时传递给它
# make -C $@ M=$(CUR_DIR) 可以用更简单的export直接传递，这里采用export
$(SUB_DIRS): ECHO
	$(MAKE) -C $@
ECHO:
	# TODO i do not understand
	@echo Compiling $(SUB_DIRS)

%.o:%.cc
	$(CXX) -c $< -o $@ $(CFLAGS) $(INCLUDEFLAGS)

%.o:%.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(INCLUDEFLAGS)

# snipcode.d 的正则处理过程缺路径导致的 make 报错,.d是当前路径下的，但是.o不是
# 第四行的正则表达式就是将 snipCode.o: snipCode.cc snipCode.h 变为 snipCode.o snipCode.d: snipCode.cc snipCode.h
%.d:%.cc
	@set -e; \
	$(RM) $@; \
	$(CXX) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

%.d:%.cpp
	@set -e; \
	$(RM) $@; \
	$(CXX) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$
include $(DEPS)