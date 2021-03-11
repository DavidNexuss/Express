all: build expr

build:
	mkdir -p build

build/%.o : %.cc
	g++ -O2 $^ -std=c++17 -I . -c -o $@

build/expr.o: build/expr.cc
	g++ -O2 $^ -std=c++17 -I . -c -o $@

expr: build/expr.o build/expression_util.o build/scope.o
	g++ -O2 $^ -std=c++17 -I . -o $@

build/expr.cc: build/expr.cc.re
	re2c $^ -o $@
build/expr.cc.re: expr.y expression.h
	bison -Wcounterexamples expr.y -o $@

clean:
	rm -rf build
