src = $(notdir $(wildcard src/*.cpp))
obj = $(src:.cpp=.o)
ifdebug ?= n
libname ?= libtest
importlib_flags = -ldl -lboost_filesystem

parser: $(obj)
	g++ -g -o $@ $^ $(importlib_flags)

$(libname).o:
	g++ -fpic -g -c src/$(libname).cpp -o $(libname).o
	g++ -shared -Wl,-soname,$(libname).so.1 -o $(libname).so.1.0.1 $(libname).o -lc

%.o: src/%.cpp
	g++ -g -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *.so* parser
