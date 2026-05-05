CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
CXXFLAGS += $(shell pkg-config --cflags gtkmm-3.0 epoxy)
LDFLAGS  := $(shell pkg-config --libs   gtkmm-3.0 epoxy)
LDFLAGS  += -lgmp -lmpfr

TARGET   := app

SRCS := main.cpp \
        main_window.cpp \
        gl.cpp \
        gl_shaders_utils.cpp \
        cgal.cpp \
        cgal_shape.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean