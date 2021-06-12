all:
	g++ -std=c++17 -o main main.cpp webpp/server.cpp

run:
	./main