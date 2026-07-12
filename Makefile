CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -Iinclude

all: server client

server:
	$(CXX) $(CXXFLAGS) src/kvstore.cpp src/server.cpp -o server

client:
	$(CXX) $(CXXFLAGS) src/client.cpp -o client

test:
	$(CXX) $(CXXFLAGS) src/kvstore.cpp tests/test_kvstore.cpp -o testkvstore && ./testkvstore

clean:
	rm -f server client testkvstore

.PHONY: all server client test clean