CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
LDFLAGS := -pthread

.PHONY: all clean run

all: main

main: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

run: main
	./main

clean:
	rm -f main
