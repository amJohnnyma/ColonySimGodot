# cd extensions/world/bin
# rm -rf *
# cmake .. -DCMAKE_BUILD_TYPE=editor
# make -j8


CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -I. 

SRCS = core_test.cpp world.cpp entity.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = core_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean core_test

clean:
	rm -f $(OBJS) $(TARGET)

core_test: $(TARGET)
	./$(TARGET)

