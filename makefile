all: loadfile 

loadfile : loadfile.cpp
	g++ -Wall -O2 -std=c++11 -lstdc++ -pthread loadfile.cpp -o loadfile 

server : server.c
	gcc server.c -o server 