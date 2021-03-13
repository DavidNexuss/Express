CFLAGS=-std=c++17
DEBUG=-g
RELEASE=-O2

release: CFLAGS += $(RELEASE)
release: all
	strip dist/expr

debug: CFLAGS += $(DEBUG)
debug: all

all: build dist/expr dist/expr.a

build:
	mkdir -p /tmp/expr_build dist
	ln -s /tmp/expr_build build

OBJECTS= build/expression_util.o build/scope.o build/register_types.o

dist/expr: $(OBJECTS) build/expr_main.o
	g++ $(CFLAGS) $^ -o $@

dist/expr.a: $(OBJECTS) build/expr.o
	ar rvs $@ $^

build/expr_main.cc.re: expr.y expression.h
	cat expr.y expr_main.y | bison -Wcounterexamples /dev/stdin -o $@
build/expr_main.cc: build/expr_main.cc.re
	re2c $^ -o $@
build/expr_main.o: build/expr_main.cc
	g++ $(CFLAGS) $^ -I . -c -o $@

build/expr.cc.re: expr.y expression.h
	bison -Wcounterexamples expr.y -o $@
build/expr.cc: build/expr.cc.re
	re2c $^ -o $@
build/expr.o: build/expr.cc
	g++ $(CFLAGS) $^ -I . -c -o $@

build/%.o : %.cc
	g++ $(CFLAGS) $^ -c -o $@

clean:
	rm -rf /tmp/expr_build
	rm build
