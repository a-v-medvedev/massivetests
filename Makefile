THIRDPARTY ?= thirdparty
MODULE ?= functest
STATIC ?= FALSE

#override CXXFLAGS += -DDEBUG

override CXXFLAGS += -O2 -std=c++17 -Wall -Wextra -pedantic -I$(THIRDPARTY)/argsparser.bin -I$(THIRDPARTY)/yaml-cpp.bin/include -I. -Isrc -D__USE_BSD 
override CXXFLAGS += -DMODULE=$(MODULE) -DMODULESTR=\"$(MODULE)\"
ifeq ($(STATIC),TRUE)
override LDFLAGS += $(THIRDPARTY)/argsparser.bin/libargsparser.a $(THIRDPARTY)/yaml-cpp.bin/lib/libyaml-cpp.a
else
override LDFLAGS += -L$(THIRDPARTY)/yaml-cpp.bin/lib -Wl,-rpath -Wl,$(THIRDPARTY)/yaml-cpp.bin/lib -lyaml-cpp
override LDFLAGS += -L$(THIRDPARTY)/argsparser.bin -Wl,-rpath -Wl,$(THIRDPARTY)/argsparser.bin -largsparser
endif

BINARY = massivetest
OBJS = src/massivetest.o src/helpers.o modules/$(MODULE)/inout.o modules/$(MODULE)/traits.o modules/$(MODULE)/comparator.o

$(BINARY): module.h $(OBJS) 
	g++ -o $@ $(OBJS) $(LDFLAGS)
	rm -f module.h

module.h:
	echo "#include \"modules/$(MODULE)/traits.h\"" > module.h
	echo "#include \"modules/$(MODULE)/inout.h\"" >> module.h

$(OBJS): src/inout_base.h src/process.h src/dispatcher.h src/results.h src/scope.h src/helpers.h Makefile 
$(OBJS): modules/$(MODULE)/traits.h modules/$(MODULE)/inout.h modules/$(MODULE)/comparator.h

clean:
	@rm -f $(BINARY) $(OBJS) module.h

format:
	find src/ -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
	find modules/ -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
