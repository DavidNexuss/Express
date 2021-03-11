all: build expr

build:
	mkdir -p build

expr: build/expr.cc
	g++ -g $^ -std=c++17 -I . -o $@
build/expr.cc: build/expr.cc.re
	re2c $^ -o $@
build/expr.cc.re: expr.y expression.h
	bison -Wcounterexamples expr.y -o $@

clean:
	rm -rf build
