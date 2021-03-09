all: build expr

build:
	mkdir -p build

expr: build/expr.cc
	g++ $^ -std=c++17 -I . -o $@
build/expr.cc.re: expr.y
	bison $^ -o $@
build/expr.cc: build/expr.cc.re
	re2c $^ -o $@

clean:
	rm -rf build
