# Top level makefile, the real shit is at src/Makefile

# 找不到all，所以只能执行.DEFAULT这个lable
default: all

.DEFAULT:
	@cd src && $(MAKE) $@
	@cd example && $(MAKE) $@

.PHONY:example
example:
	# @cd example && $(MAKE) $@
	$(MAKE) -C $@

.PHONY:src
src:
	# @cd src && $(MAKE) $@
	$(MAKE) -C $@

.PHONY:clean
clean:
	@cd src && $(MAKE) clean $@
	@cd example && $(MAKE) clean $@
	@echo all clean done

# $(MAKE)本身也是GNU的宏
# $(info $(MAKE))
# $(warning $(MAKE))
.PHONY:test
test:
	@echo do check in top level