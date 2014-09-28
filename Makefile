all: mathparse regex_test

mathparse: mathparse.cpp
	g++ -Wall mathparse.cpp -o mathparse -std=c++11 -ggdb

regex_test: regex_test.cpp
	g++ -Wall regex_test.cpp -o regex_test -std=c++11 -ggdb

clean:
	rm mathparse
