expr: expr.cc
	g++ expr.cc -std=c++17 -o expr

expr.cc.re: expr.y
	bison expr.y -o expr.cc.re
expr.cc: expr.cc.re
	re2c expr.cc.re -o expr.cc


clean:
	rm expr.cc.re expr.cc
