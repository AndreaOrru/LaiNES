CXX      = clang++
CXXFLAGS = -std=c++11

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

.PHONY: check-syntax clean

laines: $(OBJ)
	$(CXX) -o $@ $^

check-syntax:
	$(CXX) $(CXXFLAGS) -o /dev/null -S ${CHK_SOURCES}

clean:
	rm -rf src/*.o laines
