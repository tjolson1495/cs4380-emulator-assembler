CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lgtest -pthread
 
all: emu4380 runTests
 
emu4380: src/main.o src/emu4380.o src/cache.o
	$(CXX) $(CXXFLAGS) src/main.o src/emu4380.o src/cache.o -o emu4380
 
runTests: tests/tests.o src/emu4380.o src/cache.o
	$(CXX) $(CXXFLAGS) tests/tests.o src/emu4380.o src/cache.o -o runTests $(LDFLAGS)
 
src/main.o: src/main.cpp include/emu4380.h
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o
 
src/emu4380.o: src/emu4380.cpp include/emu4380.h include/cache.h
	$(CXX) $(CXXFLAGS) -c src/emu4380.cpp -o src/emu4380.o
 
src/cache.o: src/cache.cpp include/cache.h include/emu4380.h
	$(CXX) $(CXXFLAGS) -c src/cache.cpp -o src/cache.o
 
tests/tests.o: tests/tests.cpp include/emu4380.h
	$(CXX) $(CXXFLAGS) -c tests/tests.cpp -o tests/tests.o
 
cmake:
	cmake -S . -B build
	cmake --build build
 
clean:
	rm -f src/*.o tests/*.o emu4380 runTests
	rm -rf build