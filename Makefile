CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -Ilib
LDFLAGS := -pthread

.PHONY: all clean run test

all: main service

main: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

service: service.cpp $(wildcard lib/*.hpp)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_service: test_service.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_queue: test/queue.cpp $(wildcard lib/*.hpp)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
test_ratelimiter: test/ratelimiter.cpp $(wildcard lib/*.hpp)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test: test_ratelimiter test_queue
	./test_queue
	# ./test_ratelimiter \

run: main
	./main

clean:
	rm -f main service test_service test_ratelimiter
