
mathparse: mathparse.cpp
	clang++ mathparse.cpp -o mathparse -std=c++11 -ggdb

clean:
	rm mathparse
