THIRDPARTY ?= thirdparty
MODULE ?= imb_async

override CXXFLAGS += -O2 -std=c++11 -Wall -Wextra -pedantic -I$(THIRDPARTY)/argsparser.bin -I$(THIRDPARTY)/yaml-cpp.bin/include -I. -Isrc -D__USE_BSD 
override LDFLAGS += $(THIRDPARTY)/argsparser.bin/libargsparser.a $(THIRDPARTY)/yaml-cpp.bin/lib/libyaml-cpp.a

BINARY = massivetest
OBJS = src/massivetest.o src/helpers.o modules/$(MODULE)/inout.o modules/$(MODULE)/traits.o

$(BINARY): $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): src/inout_base.h src/process.h src/dispatcher.h src/results.h src/scope.h src/helpers.h Makefile 
$(OBJS): modules/$(MODULE)/traits.h modules/$(MODULE)/inout.h

clean:
	@rm -f $(BINARY) $(OBJS)

format:
	find src/ -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
	find modules/ -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
