CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
LDFLAGS := -pthread

.PHONY: all clean run

all: main service

main: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

service: service.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_service: test_service.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

run: main
	./main

clean:
	rm -f main service
