CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lgtest -pthread
 
all: emu4380 runTests
 
emu4380: src/main.o src/emu4380.o
	$(CXX) $(CXXFLAGS) src/main.o src/emu4380.o -o emu4380
 
runTests: test/tests.o src/emu4380.o
	$(CXX) $(CXXFLAGS) test/tests.o src/emu4380.o -o runTests $(LDFLAGS)
 
src/main.o: src/main.cpp include/emu4380.h
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o
 
src/emu4380.o: src/emu4380.cpp include/emu4380.h
	$(CXX) $(CXXFLAGS) -c src/emu4380.cpp -o src/emu4380.o
 
test/tests.o: test/tests.cpp include/emu4380.h
	$(CXX) $(CXXFLAGS) -c test/tests.cpp -o test/tests.o
 
cmake:
	cmake -S . -B build
	cmake --build build
 
clean:
	rm -f src/*.o test/*.o emu4380 runTests
	rm -rf build
 
