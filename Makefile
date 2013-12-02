CXX      = clang++
CXXFLAGS = -ggdb -std=c++11
LIBS     = -lSDL

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

.PHONY: check-syntax clean

laines: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $^

check-syntax:
	$(CXX) $(CXXFLAGS) -o /dev/null -S ${CHK_SOURCES}

clean:
	rm -rf src/*.o laines
