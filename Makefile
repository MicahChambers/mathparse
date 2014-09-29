all: mathexpression 

mathexpression: mathexpression.cpp
	g++ -Wall main.cpp mathparse.cpp -o mathparse -std=c++11 -ggdb

clean:
	rm mathexpression 
