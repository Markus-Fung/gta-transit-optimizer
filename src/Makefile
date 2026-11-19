CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g
TARGET = router
SRCS = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRCS) src/graph.hpp src/router.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) $(TARGET).exe