CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra
INCLUDES = -Isrc
LIBS = $(shell pkg-config --libs raylib 2>/dev/null || echo "-lraylib -lm -lpthread -ldl")

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = metro_grid

all: $(TARGET) coaster_grid

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

coaster_grid: $(TARGET)
	ln -sf $(TARGET) coaster_grid

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
