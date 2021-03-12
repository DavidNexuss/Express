
CFLAGS=""
DEBUG=-g
RELEASE=-O2

release: CFLAGS = $(RELEASE)
release: all

debug: CFLAGS = $(DEBUG)
debug: all

all: build dist/expr dist/expr.a

build:
	mkdir -p build dist

OBJECTS= build/expr.o build/expression_util.o build/scope.o build/register_types.o

dist/expr: $(OBJECTS)
	g++ $(CFLAGS) $^ -std=c++17 -I . -o $@

dist/expr.a: $(OBJECTS)
	ar rvs $@ $^

build/expr.cc: build/expr.cc.re
	re2c $^ -o $@
build/expr.cc.re: expr.y expression.h
	bison -Wcounterexamples expr.y -o $@

build/%.o : %.cc
	g++ $(CFLAGS) $^ -std=c++17 -I . -c -o $@

build/expr.o: build/expr.cc
	g++ $(CFLAGS) $^ -std=c++17 -I . -c -o $@

clean:
	rm -rf build
