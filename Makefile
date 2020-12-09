override CXXFLAGS += -O2 -std=c++11 -Wall -Wextra -pedantic -Ithirdparty/argsparser.bin -Ithirdparty/yaml-cpp.bin/include -I. -Isrc -D__USE_BSD 
override LDFLAGS += thirdparty/argsparser.bin/libargsparser.a thirdparty/yaml-cpp.bin/lib/libyaml-cpp.a

BINARY = massivetest
OBJS = src/massivetest.o src/helpers.o modules/imb_async/inout.o modules/imb_async/traits.o

$(BINARY): $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): src/inout_base.h src/process.h src/dispatcher.h src/results.h src/scope.h src/helpers.h Makefile modules/imb_async/traits.h modules/imb_async/inout.h

clean:
	@rm -f $(BINARY) $(OBJS)

format:
	find src/ -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
	find modules/ -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
