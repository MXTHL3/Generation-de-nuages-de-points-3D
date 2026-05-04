CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 `pkg-config --cflags gtkmm-3.0`
LIBS = `pkg-config --libs gtkmm-3.0`

TARGET = app

SRCS = main.cpp main_window.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all