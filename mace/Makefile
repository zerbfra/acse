project = ../bin/mace
objects = machine.o fetch.o getbits.o decode.o main.o
deps = $(objects:.o=.d)
export project objects deps

.PHONY: all clean

all: $(deps)
	make -f Makefile.all

$(project): $(objects)
	gcc -o $@ $^

clean:
	rm -f $(objects) $(deps) $(project) $(project:=.exe)

# Automake dependencies for C/C++ files
define make-dep
@echo "Making dependencies for $<..." 
@$(SHELL) -ec "$(CC) -MM $(CPPFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@"
endef
%.d: %.c
	$(make-dep)


