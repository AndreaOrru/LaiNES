CXX      = clang++
CXXFLAGS = -O3 -march=native -std=c++11
LIBS     = -lSDL

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst %.cpp, %.o,   $(SRC))
DEP = $(patsubst %.cpp, %.dep, $(SRC))

.PHONY: check-syntax clean

laines: $(OBJ)
	$(CXX) $(LIBS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $(@:.o=.dep) -c $< -o $@

check-syntax:
	$(CXX) $(CXXFLAGS) -o /dev/null -S ${CHK_SOURCES}

clean:
	rm -rf src/*.dep src/*.o laines

-include $(DEP)
